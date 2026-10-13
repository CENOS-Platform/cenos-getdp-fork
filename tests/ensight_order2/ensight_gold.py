"""Minimal reader for the EnSight Gold C Binary files written by PrintExternal."""

import re
import struct
from pathlib import Path

NODES_PER_ELEMENT = {
    "point": 1,
    "bar2": 2,
    "bar3": 3,
    "tria3": 3,
    "tria6": 6,
    "quad4": 4,
    "quad8": 8,
    "tetra4": 4,
    "tetra10": 10,
    "pyramid5": 5,
    "pyramid13": 13,
    "penta6": 6,
    "penta15": 15,
    "hexa8": 8,
    "hexa20": 20,
}


class _Stream:
    def __init__(self, data: bytes):
        self.data = data
        self.pos = 0

    def eof(self) -> bool:
        return self.pos >= len(self.data)

    def string(self) -> str:
        s = self.data[self.pos : self.pos + 80].split(b"\0")[0].decode().strip()
        self.pos += 80
        return s

    def ints(self, n: int) -> list:
        v = list(struct.unpack_from(f"<{n}i", self.data, self.pos))
        self.pos += 4 * n
        return v

    def floats(self, n: int) -> list:
        v = list(struct.unpack_from(f"<{n}f", self.data, self.pos))
        self.pos += 4 * n
        return v


def read_geometry(path: Path) -> dict:
    """Returns {part_number: {"name", "node_ids", "xyz", "blocks": {type: (ids, conn)}}}.

    Connectivity is 0-based into the part's node list, in EnSight slot order."""
    s = _Stream(Path(path).read_bytes())
    assert s.string() == "C Binary"
    s.string()
    s.string()
    assert s.string() == "node id given"
    assert s.string() == "element id given"
    parts = {}
    kw = s.string()
    while kw == "part":
        number = s.ints(1)[0]
        name = s.string()
        assert s.string() == "coordinates"
        nn = s.ints(1)[0]
        node_ids = s.ints(nn)
        x, y, z = s.floats(nn), s.floats(nn), s.floats(nn)
        part = {"name": name, "node_ids": node_ids, "xyz": list(zip(x, y, z)), "blocks": {}}
        kw = "" if s.eof() else s.string()
        while kw in NODES_PER_ELEMENT:
            ne = s.ints(1)[0]
            ids = s.ints(ne)
            npe = NODES_PER_ELEMENT[kw]
            flat = s.ints(ne * npe)
            conn = [[c - 1 for c in flat[i * npe : (i + 1) * npe]] for i in range(ne)]
            part["blocks"][kw] = (ids, conn)
            kw = "" if s.eof() else s.string()
        parts[number] = part
    return parts


def read_variable(path: Path, parts: dict, ncomp: int) -> dict:
    """Returns {part_number: [value tuple per node]}."""
    s = _Stream(Path(path).read_bytes())
    s.string()
    values = {}
    while not s.eof():
        assert s.string() == "part"
        number = s.ints(1)[0]
        assert s.string() == "coordinates"
        nn = len(parts[number]["node_ids"])
        comps = [s.floats(nn) for _ in range(ncomp)]
        values[number] = list(zip(*comps))
    return values


def read_case(path: Path) -> dict:
    """Returns {variable_name: (kind, file_pattern)} from a .case file."""
    out = {}
    for line in Path(path).read_text().splitlines():
        m = re.match(r"\s*(scalar|vector|tensor \w+) per node:\s*1\s+(\S+)\s+(\S+)", line)
        if m:
            out[m.group(2)] = (m.group(1), m.group(3))
    return out
