"""Checks the second-order prism functions BF_Edge_3FT_a/b, BF_Edge_3FQ_a/b/c,
BF_Edge_3VZ and the prism BF_Node_2F / BF_GradNode_2F.

Needs numpy (the CENOS backend Python has it):

    python run_tests.py --getdp path/to/getdp.exe [--dll-path dir1;dir2] [--keep]

Meshes are generated here (structured blocks, written as MSH 2.2), so that
prisms extruded along different axes, prism|tet interfaces and node
numberings are under control. Fields lying in the discrete spaces are
projected and must be reproduced to round-off; this only holds if the facet
functions are conforming across every shared facet, for every numbering.

As with BF_Edge + BF_Edge_3F_a/b on tetrahedra, the A-space (edge, facet and
3VZ functions) has constant tangential traces on edges, so it does not contain
P1^3: it is meant to be complete in curl (every divergence-free linear field
is a curl of it), and complete together with the gradients of the
second-order nodal space (BF_GradNode_2E/2F).
"""

import argparse
import itertools
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

import numpy as np

GMSH_TRIANGLE, GMSH_QUADRANGLE, GMSH_TETRAHEDRON, GMSH_PRISM, GMSH_PYRAMID = 2, 3, 4, 6, 7
VOL, BND, SIDE = 1, 2, 3

# --------------------------------------------------------------------------- meshes


class Mesh:
    def __init__(self):
        self.xyz = np.zeros((0, 3))
        self.volumes = []  # (gmsh type, [node indices])

    def add_nodes(self, pts, tol=1e-9):
        """Adds points, merging them with existing ones; returns their indices."""
        index = {tuple(np.round(p / tol).astype(np.int64)): i for i, p in enumerate(self.xyz)}
        ids, new = [], []
        for p in pts:
            key = tuple(np.round(np.asarray(p) / tol).astype(np.int64))
            if key not in index:
                index[key] = len(self.xyz) + len(new)
                new.append(p)
            ids.append(index[key])
        if new:
            self.xyz = np.vstack([self.xyz, np.asarray(new, dtype=float)])
        return ids

    def faces(self):
        """Every facet as (sorted node tuple) -> [cyclic node list, number of owners]."""
        table = {
            GMSH_TETRAHEDRON: [(0, 2, 1), (0, 1, 3), (0, 3, 2), (1, 2, 3)],
            GMSH_PRISM: [(0, 2, 1), (3, 4, 5), (0, 1, 4, 3), (0, 3, 5, 2), (1, 2, 5, 4)],
            GMSH_PYRAMID: [(0, 3, 2, 1), (0, 1, 4), (1, 2, 4), (2, 3, 4), (3, 0, 4)],
        }
        faces = {}
        for t, nodes in self.volumes:
            for f in table[t]:
                cyc = [nodes[i] for i in f]
                key = tuple(sorted(cyc))
                if key in faces:
                    faces[key][1] += 1
                else:
                    faces[key] = [cyc, 1]
        return faces

    def edges(self):
        table = {
            GMSH_TETRAHEDRON: [(0, 1), (1, 2), (2, 0), (0, 3), (1, 3), (2, 3)],
            GMSH_PRISM: [(0, 1), (1, 2), (2, 0), (3, 4), (4, 5), (5, 3), (0, 3), (1, 4), (2, 5)],
            GMSH_PYRAMID: [(0, 1), (1, 2), (2, 3), (3, 0), (0, 4), (1, 4), (2, 4), (3, 4)],
        }
        return {tuple(sorted((n[a], n[b]))) for t, n in self.volumes for a, b in table[t]}

    def counts(self):
        faces = self.faces()
        return {
            "nodes": len({i for _, n in self.volumes for i in n}),
            "edges": len(self.edges()),
            "triangles": sum(1 for k in faces if len(k) == 3),
            "quadrangles": sum(1 for k in faces if len(k) == 4),
            "prisms": sum(1 for t, _ in self.volumes if t == GMSH_PRISM),
        }

    def transform(self, fn):
        self.xyz = np.array([fn(p) for p in self.xyz])

    def write(self, path, side=None, perm=None):
        """MSH 2.2: volumes (tag 1), boundary facets (tag 2, or 3 where side(centroid)).
        perm renumbers the nodes, which changes every rank-based facet function choice."""
        n = len(self.xyz)
        perm = np.arange(n) if perm is None else np.asarray(perm)
        elements = [(t, VOL, nodes) for t, nodes in self.volumes]
        for cyc, owners in self.faces().values():
            if owners == 1:
                tag = SIDE if side is not None and side(self.xyz[cyc].mean(axis=0)) else BND
                elements.append((GMSH_TRIANGLE if len(cyc) == 3 else GMSH_QUADRANGLE, tag, cyc))
        lines = ["$MeshFormat", "2.2 0 8", "$EndMeshFormat", "$Nodes", str(n)]
        lines += [f"{perm[i] + 1} {x:.17g} {y:.17g} {z:.17g}" for i, (x, y, z) in enumerate(self.xyz)]
        lines += ["$EndNodes", "$Elements", str(len(elements))]
        lines += [
            f"{k + 1} {t} 2 {tag} {tag} {' '.join(str(perm[i] + 1) for i in nodes)}"
            for k, (t, tag, nodes) in enumerate(elements)
        ]
        lines.append("$EndElements")
        Path(path).write_text("\n".join(lines) + "\n")


def prism_block(mesh, divisions, lo, hi, axis="z", diagonals="alternate", rng=None):
    """Structured block of prisms between the corners lo and hi, extruded along axis.
    divisions = (n_a, n_b, n_c) in the block's own (base, base, extrusion) axes."""
    na, nb, nc = divisions
    # cyclic permutations keep the prisms positively oriented
    order = {"z": (0, 1, 2), "x": (1, 2, 0), "y": (2, 0, 1)}[axis]
    lo, hi = np.asarray(lo, float), np.asarray(hi, float)

    def point(i, j, k):
        p = np.zeros(3)
        for local, n, idx in zip(order, (na, nb, nc), (i, j, k)):
            p[local] = lo[local] + (hi[local] - lo[local]) * idx / n
        return p

    grid = {}
    pts = [point(i, j, k) for i in range(na + 1) for j in range(nb + 1) for k in range(nc + 1)]
    ids = mesh.add_nodes(pts)
    for (i, j, k), nid in zip(itertools.product(range(na + 1), range(nb + 1), range(nc + 1)), ids):
        grid[i, j, k] = nid
    prisms = []
    for i in range(na):
        for j in range(nb):
            flip = (i + j) % 2 if diagonals == "alternate" else rng.integers(2)
            q = [(i, j), (i + 1, j), (i + 1, j + 1), (i, j + 1)]
            tris = [(q[0], q[1], q[2]), (q[0], q[2], q[3])] if not flip else [(q[0], q[1], q[3]), (q[1], q[2], q[3])]
            for tri in tris:
                for k in range(nc):
                    prisms.append([grid[a, b, k] for a, b in tri] + [grid[a, b, k + 1] for a, b in tri])
    return prisms


def lowest(mesh_ids, corners):
    return min(corners, key=lambda c: mesh_ids[c])


def prism_to_tets(nodes):
    """Lowest-index split (Dompierre et al.), conforming between neighbours."""
    relabel = [[0, 1, 2, 3, 4, 5], [1, 2, 0, 4, 5, 3], [2, 0, 1, 5, 3, 4],
               [3, 5, 4, 0, 2, 1], [4, 3, 5, 1, 0, 2], [5, 4, 3, 2, 1, 0]]
    r = relabel[lowest(nodes, range(6))]
    n = [nodes[i] for i in r]
    opposite = lowest(n, [1, 2, 4, 5])
    tets = [[0, 1, 2, 5], [0, 1, 5, 4], [0, 4, 5, 3]] if opposite in (1, 5) else [[0, 1, 2, 4], [0, 4, 2, 5], [0, 4, 5, 3]]
    return [[n[c] for c in t] for t in tets]


def orient_tets(mesh):
    out = []
    for t, nodes in mesh.volumes:
        if t == GMSH_TETRAHEDRON:
            p = mesh.xyz[nodes]
            if np.linalg.det(np.array([p[1] - p[0], p[2] - p[0], p[3] - p[0]])) < 0:
                nodes = [nodes[0], nodes[1], nodes[3], nodes[2]]
        out.append((t, nodes))
    mesh.volumes = out


def mesh_mixed():
    """Prisms extruded along z (z < 0.5) under split tetrahedra (z > 0.5), plus a
    block of prisms extruded along x sharing quadrangular facets with the first
    one, so that horizontal and vertical facet edges swap roles across it."""
    m = Mesh()
    m.volumes += [(GMSH_PRISM, p) for p in prism_block(m, (3, 3, 2), (0, 0, 0), (1, 1, 0.5), "z")]
    for p in prism_block(m, (3, 3, 2), (0, 0, 0.5), (1, 1, 1), "z"):
        m.volumes += [(GMSH_TETRAHEDRON, t) for t in prism_to_tets(p)]
    # base (y, z) with 3 x 2 divisions, extruded along x in 3 layers: matches the
    # first block on the plane y = 1
    m.volumes += [(GMSH_PRISM, p) for p in prism_block(m, (3, 2, 3), (0, 1, 0), (1, 2, 0.5), "x")]
    orient_tets(m)
    return m


def mesh_extruded(n=3, nz=3, diagonals="alternate", rng=None):
    m = Mesh()
    m.volumes += [(GMSH_PRISM, p) for p in prism_block(m, (n, n, nz), (0, 0, 0), (1, 1, 1), "z", diagonals, rng)]
    return m


def mesh_tets(n=3):
    m = Mesh()
    for p in prism_block(m, (n, n, n), (0, 0, 0), (1, 1, 1), "z"):
        m.volumes += [(GMSH_TETRAHEDRON, t) for t in prism_to_tets(p)]
    orient_tets(m)
    return m


def single(kind):
    m = Mesh()
    if kind == "prism":
        ids = m.add_nodes([(0, 0, 0), (1, 0, 0), (0.2, 1.1, 0), (0.1, 0.05, 0.7), (1.1, 0.05, 0.7), (0.3, 1.15, 0.7)])
        m.volumes = [(GMSH_PRISM, ids)]
    else:
        ids = m.add_nodes([(0, 0, 0), (1, 0, 0), (0.2, 1.1, 0), (0.3, 0.2, 0.9)])
        m.volumes = [(GMSH_TETRAHEDRON, ids)]
    return m


def mesh_with_pyramid():
    m = Mesh()
    ids = m.add_nodes([(0, 0, 0), (1, 0, 0), (1, 1, 0), (0, 1, 0), (0.5, 0.5, 0.8)])
    m.volumes = [(GMSH_PYRAMID, ids)]
    return m


AFFINE = np.array([[1.1, 0.2, -0.15], [-0.1, 0.9, 0.25], [0.3, -0.2, 1.2]])


def affine(p):
    return AFFINE @ p + np.array([0.3, -0.2, 0.1])


def smooth(p):
    x, y, z = p
    return np.array([x + 0.08 * np.sin(np.pi * y), y + 0.08 * np.sin(np.pi * z), z + 0.08 * np.sin(np.pi * x)])


# --------------------------------------------------------------------------- .pro

FIELDS = """
  // P1^3
  F1[] = Vector[1 + 2*X[] - Y[] + 0.5*Z[], -1 + X[] + 3*Y[] - 2*Z[], 0.5 - X[] + Y[] + Z[]];
  curlF1[] = Vector[3, 1.5, 2];
  // gradient of P2
  G2[] = Vector[2*X[] + Y[] + 1, X[] - 2*Z[], -2*Y[] + Z[] - 1];
  // [P1(x,y)]^2 x P2(z) + P1(x,y) x P1(z), exact on prisms extruded along z
  T1[] = Vector[(1 + X[] - 2*Y[])*(1 + Z[] - Z[]^2), (2 - X[] + Y[])*(Z[]^2 - 0.5),
                (1 + X[] + Y[])*(1 - Z[])];
  curlT1[] = Vector[(1 - Z[]) - (2 - X[] + Y[])*2*Z[],
                    (1 + X[] - 2*Y[])*(1 - 2*Z[]) - (1 - Z[]),
                    -(Z[]^2 - 0.5) + 2*(1 + Z[] - Z[]^2)];
  // gradient of P2(x,y) x P2(z)
  GT2[] = Vector[(2*X[] + Y[])*(1 + Z[]^2), (X[] + 1)*(1 + Z[]^2), (X[]^2 + X[]*Y[] + Y[])*2*Z[]];
  // tangential trace zero on the plane x = 0: P1^3 plus the gradient of x(1+y+z)
  S1[] = Vector[1 + Y[] + Z[], X[], 2*X[]] + Vector[1 + Y[] + Z[], X[], X[]];
  // smooth
  Fs[] = Vector[Sin[2*Y[] + Z[]], Cos[X[] - Z[]], Sin[X[] + 2*Y[]]];
  curlFs[] = Vector[2*Cos[X[] + 2*Y[]] - Sin[X[] - Z[]], Cos[2*Y[] + Z[]] - Cos[X[] + 2*Y[]],
                    -Sin[X[] - Z[]] - 2*Cos[2*Y[] + Z[]]];
  // scalars: P2 and P2(x,y) x P2(z)
  f2[] = 1 + X[] - 2*Y[] + 0.5*Z[] + X[]^2 - X[]*Y[] + 2*Y[]*Z[] - Z[]^2;
  ft2[] = (1 + X[] + Y[]^2 - X[]*Y[])*(1 - Z[] + 2*Z[]^2);
  // divergence-free P1, with a vertical component linear in x and y
  C1[] = Vector[1 + Y[] - 2*Z[] + 0.5*X[], 2 + 3*X[] + Z[] - 0.25*Y[], -1 + X[] - Y[] - 0.25*Z[]];
"""

BF = {
    "edge": "BF_Edge",
    "edge2": "BF_GradNode_2E",
    "facet2": "BF_GradNode_2F",
    "node": "BF_GradNode",
    "ft_a": "BF_Edge_3FT_a",
    "ft_b": "BF_Edge_3FT_b",
    "f_a": "BF_Edge_3F_a",
    "f_b": "BF_Edge_3F_b",
    "fq_a": "BF_Edge_3FQ_a",
    "fq_b": "BF_Edge_3FQ_b",
    "fq_c": "BF_Edge_3FQ_c",
    "vz": "BF_Edge_3VZ",
}
ENTITY = {"edge": "EdgesOf", "edge2": "EdgesOf", "node": "NodesOf", "vz": "VolumesOf"}
A_SPACE = ["edge", "ft_a", "ft_b", "fq_a", "fq_b", "fq_c", "vz"]
FULL_SPACE = A_SPACE + ["edge2", "facet2"]
H_SPACE = {"node": "BF_Node", "edge2": "BF_Node_2E", "facet2": "BF_Node_2F"}

PRO = """
Group {{
  Vol = Region[{{1}}];
  Bnd = Region[{{2, 3}}];
  Side = Region[{{3}}];
  Dom = Region[{{Vol, Bnd}}];
}}
Function {{
{fields}
}}
Constraint {{ {{ Name fix; Case {{ {{ Region Side; Value 0.; }} }} }}
  {{ Name gauge; Case {{ {{ Region Vol; SubRegion Bnd; Value 0.; }} }} }} }}
Jacobian {{ {{ Name Jac; Case {{ {{ Region Bnd; Jacobian Sur; }} {{ Region All; Jacobian Vol; }} }} }} }}
Integration {{ {{ Name Int; Case {{ {{ Type Gauss; Case {{
  {{ GeoElement Triangle; NumberOfPoints 12; }} {{ GeoElement Quadrangle; NumberOfPoints {quad}; }}
  {{ GeoElement Tetrahedron; NumberOfPoints 15; }} {{ GeoElement Prism; NumberOfPoints {prism}; }}
  {{ GeoElement Pyramid; NumberOfPoints 8; }}
}} }} }} }} }}
FunctionSpace {{
  {{ Name S; Type {form}; BasisFunction {{
{basis}
  }}
  Constraint {{
{constraints}
  }} }}
}}
Formulation {{ {{ Name Proj; Type FemEquation;
  Quantity {{ {{ Name a; Type Local; NameOfSpace S; }} }}
  Equation {{
{equations}
  }}
}} }}
Resolution {{ {{ Name Proj; System {{ {{ Name A; NameOfFormulation Proj; }} }}
  Operation {{ {operations} }} }} }}
PostProcessing {{ {{ Name Proj; NameOfFormulation Proj; Quantity {{
{post}
}} }} }}
PostOperation {{ {{ Name Out; NameOfPostProcessing Proj; Operation {{
{prints}
}} }} }}
"""


def write_pro(path, form, basis_keys, equations, post, operations="Generate[A]; Solve[A];", constrained=(),
              quad=9, prism=36, h1=False, print_in="Vol", gauge=False):
    lines, cons = [], []
    if gauge:
        cons.append("    { NameOfCoef c_edge; EntityType EdgesOfTreeIn; EntitySubType StartingOn; NameOfConstraint gauge; }")
    for key in basis_keys:
        fn = H_SPACE[key] if h1 else BF[key]
        entity = ENTITY.get(key, "FacetsOf")
        lines.append(f"    {{ Name w_{key}; NameOfCoef c_{key}; Function {fn}; Support Dom; Entity {entity}[All]; }}")
        if key in constrained:
            cons.append(f"    {{ NameOfCoef c_{key}; EntityType {entity}; NameOfConstraint fix; }}")
    post_lines = "\n".join(f"  {{ Name {n}; Value {{ {v} }} }}" for n, v in post.items())
    prints = "\n".join(
        f'  Print[ {n}[{print_in}], OnGlobal, Format Table, File "{n}.txt" ];' for n in post
    )
    Path(path).write_text(
        PRO.format(
            fields=FIELDS,
            quad=quad,
            prism=prism,
            form=form,
            basis="\n".join(lines),
            constraints="\n".join(cons),
            equations="\n".join(f"    {e}" for e in equations),
            operations=operations,
            post=post_lines,
            prints=prints,
        )
    )


def integral(expr):
    return f"Integral {{ [ {expr} ]; In Vol; Jacobian Jac; Integration Int; }}"


def run_getdp(exe, workdir, post=True, expect_ok=True):
    # -sparsity: exact preallocation, GetDP's default reallocates for second-order spaces
    cmd = [str(exe), "proj.pro", "-msh", "mesh.msh", "-solve", "Proj", "-sparsity", "-v", "4"]
    cmd += ["-pos", "Out"] if post else []
    r = subprocess.run(cmd, cwd=workdir, capture_output=True, text=True, timeout=900)
    log = r.stdout + r.stderr
    if expect_ok and (r.returncode != 0 or re.search(r"\bError\b", log)):
        raise RuntimeError(f"getdp failed ({r.returncode}) in {workdir}:\n{log[-3000:]}")
    return r.returncode, log


def read_global(workdir, name):
    text = (Path(workdir) / f"{name}.txt").read_text().split()
    return float(text[-1]) if len(text) > 1 else float(text[0])


def ndofs(log):
    m = re.findall(r"(\d+)\s+[Dd]ofs?\b", log)
    if not m:
        m = re.findall(r"N:\s*(\d+)", log)
    return int(m[-1]) if m else None


def read_matlab_matrix(path):
    """PETSc ASCII MATLAB output of a (complex) sparse matrix."""
    text = Path(path).read_text()
    size = re.search(r"Size\s*=\s*(\d+)\s+(\d+)", text)
    n, m = int(size.group(1)), int(size.group(2))
    body = text[text.index("zzz = [") + len("zzz = ["): text.index("];")]
    M = np.zeros((n, m), dtype=complex)
    for line in body.strip().splitlines():
        v = line.replace("i", "").split()
        if len(v) < 3:
            continue
        val = float(v[2]) + (float(v[3]) * 1j if len(v) > 3 else 0)
        M[int(v[0]) - 1, int(v[1]) - 1] += val
    return M


def nullity(M, rtol=1e-9):
    s = np.linalg.svd(M, compute_uv=False)
    return int(np.sum(s <= rtol * s.max())), len(s)


# --------------------------------------------------------------------------- checks


class Checker:
    def __init__(self):
        self.failures, self.passed = [], 0

    def check(self, ok, msg):
        if ok:
            self.passed += 1
        else:
            self.failures.append(msg)
            print("  FAIL:", msg)


def l2_projection(exe, workdir, mesh, keys, field, curl=None, perm=None, h1=False, boundary=False,
                  constrained=(), side=None, quad=9, prism=36):
    """Relative L2 errors of the projection of the expression field (and of its curl)."""
    workdir.mkdir(parents=True, exist_ok=True)
    mesh.write(workdir / "mesh.msh", side=side, perm=perm)
    gal = "Galerkin {{ [ {} ]; In {}; Jacobian Jac; Integration Int; }}"
    eq = [gal.format("Dof{a}, {a}", "Vol"), gal.format(f"-({field}), {{a}}", "Vol")]
    if boundary:
        eq += [gal.format("Dof{a}, {a}", "Bnd"),
               gal.format("-Normal[] * (Normal[] * Dof{a}), {a}", "Bnd"),
               gal.format(f"-(({field}) - Normal[] * (Normal[] * ({field}))), {{a}}", "Bnd")]
    if curl:
        eq += [gal.format("Dof{d a}, {d a}", "Vol"), gal.format(f"-({curl}), {{d a}}", "Vol")]
    norm = (lambda e: f"({e})^2") if h1 else (lambda e: f"SquNorm[{e}]")
    post = {"err": integral(norm(f"{{a}} - ({field})")), "ref": integral(norm(field))}
    if curl:
        post["errc"] = integral(norm(f"{{d a}} - ({curl})"))
        post["refc"] = integral(norm(curl))
    write_pro(workdir / "proj.pro", "Form0" if h1 else "Form1", keys, eq, post, constrained=constrained, h1=h1,
              quad=quad, prism=prism)
    _, log = run_getdp(exe, workdir)
    out = {"err": np.sqrt(read_global(workdir, "err") / read_global(workdir, "ref")), "dofs": ndofs(log)}
    if curl:
        out["errc"] = np.sqrt(read_global(workdir, "errc") / read_global(workdir, "refc"))
    return out


def curl_range(exe, workdir, mesh, keys, target, perm=None):
    """Relative error of the best curl of the space to the expression target,
    from a tree-gauged curl-curl solve: zero iff target is a curl of the space."""
    workdir.mkdir(parents=True, exist_ok=True)
    mesh.write(workdir / "mesh.msh", perm=perm)
    gal = "Galerkin {{ [ {} ]; In Vol; Jacobian Jac; Integration Int; }}"
    eq = [gal.format("Dof{d a}, {d a}"), gal.format(f"-({target}), {{d a}}")]
    post = {"errc": integral(f"SquNorm[{{d a}} - ({target})]"), "refc": integral(f"SquNorm[{target}]")}
    write_pro(workdir / "proj.pro", "Form1", keys, eq, post, gauge=True)
    run_getdp(exe, workdir)
    return np.sqrt(read_global(workdir, "errc") / read_global(workdir, "refc"))


def matrix(exe, workdir, mesh, keys, curl, perm=None):
    """Assembled curl-curl (curl=True) or mass matrix of the space."""
    workdir.mkdir(parents=True, exist_ok=True)
    mesh.write(workdir / "mesh.msh", perm=perm)
    q = "d a" if curl else "a"
    eq = [f"Galerkin {{ [ Dof{{{q}}}, {{{q}}} ]; In Vol; Jacobian Jac; Integration Int; }}"]
    write_pro(workdir / "proj.pro", "Form1", keys, eq, {}, operations="Generate[A]; Print[A];")
    run_getdp(exe, workdir, post=False)
    return read_matlab_matrix(workdir / "file_mat_A.m")


def test_single_element(exe, tmp, chk, rng, trials):
    print("single elements: curl-curl nullspace and independence")
    for t in range(trials):
        perm = rng.permutation(6)
        K = matrix(exe, tmp / f"single_prism_cc_{t}", single("prism"), A_SPACE, True, perm)
        null, n = nullity(K)
        chk.check(n == 23 and null == 5, f"prism A-space: {n} functions, curl-curl nullity {null} (want 23, 5), perm {perm}")
        M = matrix(exe, tmp / f"single_prism_mass_{t}", single("prism"), FULL_SPACE, False, perm)
        null, n = nullity(M)
        chk.check(n == 35 and null == 0, f"prism A + grad bubbles: {n} functions, mass nullity {null} (want 35, 0), perm {perm}")
        M = matrix(exe, tmp / f"single_prism_av_{t}", single("prism"), A_SPACE + ["node", "edge2", "facet2"], False, perm)
        null, n = nullity(M)
        chk.check(n == 41 and n - null == 35, f"prism A + grad v: rank {n - null} of {n} (want 35 of 41), perm {perm}")
    for t in range(min(trials, 24)):
        perm = rng.permutation(4)
        K = matrix(exe, tmp / f"single_tet_cc_{t}", single("tet"), ["edge", "ft_a", "ft_b"], True, perm)
        null, n = nullity(K)
        chk.check(n == 14 and null == 3, f"tet Whitney + 3FT: {n} functions, curl-curl nullity {null} (want 14, 3)")


def expected_dofs(mesh, keys, h1=False):
    c = mesh.counts()
    per = {"node": c["nodes"], "edge": c["edges"], "edge2": c["edges"], "facet2": c["quadrangles"],
           "ft_a": c["triangles"], "ft_b": c["triangles"], "fq_a": c["quadrangles"], "fq_b": c["quadrangles"],
           "fq_c": c["quadrangles"], "vz": c["prisms"]}
    return sum(per[k] for k in keys)


def test_exactness(exe, tmp, chk, rng, trials, tol=1e-9):
    print("exact projections on mixed meshes under random numberings")
    mixed = mesh_mixed()
    mixed.transform(affine)
    n = len(mixed.xyz)
    h1 = ["node", "edge2", "facet2"]
    for t in range(trials):
        perm = rng.permutation(n)
        e = curl_range(exe, tmp / f"mixed_curl_{t}", mixed, A_SPACE, "C1[]", perm)
        chk.check(e < tol, f"mixed A-space, divergence-free P1 curl: {e:.2e}")
        r = l2_projection(exe, tmp / f"mixed_full_{t}", mixed, FULL_SPACE, "F1[] + G2[]", "curlF1[]", perm)
        chk.check(r["err"] < tol and r["errc"] < tol, f"mixed full space, P1^3 + grad P2: {r['err']:.2e} / curl {r['errc']:.2e}")
        r = l2_projection(exe, tmp / f"mixed_bnd_{t}", mixed, FULL_SPACE, "F1[] + G2[]", None, perm, boundary=True)
        chk.check(r["err"] < tol, f"mixed full space with boundary term: {r['err']:.2e}")
        r = l2_projection(exe, tmp / f"mixed_h1_{t}", mixed, h1, "f2[]", None, perm, h1=True)
        chk.check(r["err"] < tol, f"mixed H1, P2: {r['err']:.2e}")
        if t == 0:
            want = expected_dofs(mixed, h1)
            chk.check(r["dofs"] == want, f"mixed H1 dofs {r['dofs']} (want {want})")
            r = l2_projection(exe, tmp / "mixed_dofs_full", mixed, FULL_SPACE, "F1[]", None, perm)
            want = expected_dofs(mixed, FULL_SPACE)
            chk.check(r["dofs"] == want, f"mixed full space dofs {r['dofs']} (want {want})")
            r = l2_projection(exe, tmp / "mixed_dofs_A", mixed, A_SPACE, "F1[]", None, perm)
            want = expected_dofs(mixed, A_SPACE)
            chk.check(r["dofs"] == want, f"mixed A-space dofs {r['dofs']} (want {want})")

    print("exact projections of tensor fields on prisms extruded along z")
    for t in range(trials):
        m = mesh_extruded(3, 3, "random", rng)
        perm = rng.permutation(len(m.xyz))
        e = curl_range(exe, tmp / f"ext_curl_{t}", m, A_SPACE, "C1[]", perm)
        chk.check(e < tol, f"extruded A-space, divergence-free P1 curl: {e:.2e}")
        e = curl_range(exe, tmp / f"ext_curl_noZ_{t}", m, [k for k in A_SPACE if k != "vz"], "C1[]", perm)
        chk.check(e > 1e-6, f"extruded A-space without 3VZ should miss some linear curls: {e:.2e}")
        r = l2_projection(exe, tmp / f"ext_full_{t}", m, FULL_SPACE, "T1[] + GT2[]", "curlT1[]", perm, boundary=True)
        chk.check(r["err"] < tol and r["errc"] < tol,
                  f"extruded full space, tensor field + grad P2xP2: {r['err']:.2e} / curl {r['errc']:.2e}")
        r = l2_projection(exe, tmp / f"ext_h1_{t}", m, h1, "ft2[]", None, perm, h1=True)
        chk.check(r["err"] < tol, f"extruded H1, P2xP2: {r['err']:.2e}")

    print("Assign 0 on a boundary made of quadrangles")
    for t in range(max(1, trials // 2)):
        m = mesh_extruded(3, 3, "random", rng)
        perm = rng.permutation(len(m.xyz))
        r = l2_projection(exe, tmp / f"fix_{t}", m, FULL_SPACE, "S1[]", None, perm,
                          constrained=["edge", "edge2", "facet2", "ft_a", "ft_b", "fq_a", "fq_b", "fq_c"],
                          side=lambda c: abs(c[0]) < 1e-9)
        chk.check(r["err"] < tol, f"constrained projection: {r['err']:.2e}")


def test_quadrature(exe, tmp, chk):
    print("quadrature rules")
    m = mesh_extruded(2, 2)
    # x^2 z^4 + y^3: degree (2..3 in x,y) x 4 in z; exact integral over the unit cube
    exact = 1 / 3 * 1 / 5 + 1 / 4
    for npts in (18, 21, 36, 42, 48, 64):
        wd = tmp / f"quad_prism_{npts}"
        wd.mkdir(parents=True, exist_ok=True)
        m.write(wd / "mesh.msh")
        write_pro(wd / "proj.pro", "Form0", ["node"],
                  ["Galerkin { [ Dof{a}, {a} ]; In Vol; Jacobian Jac; Integration Int; }"],
                  {"I": integral("X[]^2*Z[]^4 + Y[]^3")}, prism=npts, h1=True)
        run_getdp(exe, wd)
        val = read_global(wd, "I")
        tol = 1e-12 if npts in (18, 36, 48, 64) else 1e-9
        chk.check(abs(val - exact) < tol, f"Prism {npts} points: {val!r} vs {exact!r}")
    for npts in (9, 16):
        wd = tmp / f"quad_quadrangle_{npts}"
        wd.mkdir(parents=True, exist_ok=True)
        # the four vertical sides of the unit cube are quadrangles
        m.write(wd / "mesh.msh", side=lambda c: 1e-9 < c[2] < 1 - 1e-9)
        write_pro(wd / "proj.pro", "Form0", ["node"],
                  ["Galerkin { [ Dof{a}, {a} ]; In Vol; Jacobian Jac; Integration Int; }"],
                  {"I": "Integral { [ Z[]^4 * (X[]^2 + Y[]^2) ]; In Side; Jacobian Jac; Integration Int; }"},
                  quad=npts, h1=True, print_in="Side")
        run_getdp(exe, wd)
        val = read_global(wd, "I")
        # x=0: y^2, x=1: 1+y^2, y=0: x^2, y=1: 1+x^2, each times int z^4 = 1/5
        exact_q = (1 / 3 + 4 / 3 + 1 / 3 + 4 / 3) / 5
        chk.check(abs(val - exact_q) < 1e-12, f"Quadrangle {npts} points: {val!r} vs {exact_q!r}")


def test_tet_regression(exe, tmp, chk, rng):
    print("tetrahedra: BF_Edge_3FT_a/b are BF_Edge_3F_a/b")
    m = mesh_tets(3)
    m.transform(smooth)
    perm = rng.permutation(len(m.xyz))
    a = l2_projection(exe, tmp / "tet_3F", m, ["edge", "f_a", "f_b"], "Fs[]", "curlFs[]", perm)
    b = l2_projection(exe, tmp / "tet_3FT", m, ["edge", "ft_a", "ft_b"], "Fs[]", "curlFs[]", perm)
    chk.check(abs(a["err"] - b["err"]) <= 1e-12 * a["err"] and abs(a["errc"] - b["errc"]) <= 1e-12 * a["errc"],
              f"3F vs 3FT errors: {a['err']!r}/{b['err']!r}, curl {a['errc']!r}/{b['errc']!r}")
    chk.check(a["dofs"] == b["dofs"], f"3F vs 3FT dofs: {a['dofs']} / {b['dofs']}")


def test_pyramid_guard(exe, tmp, chk):
    print("pyramids are rejected by the facet-filtered functions")
    wd = tmp / "pyramid"
    wd.mkdir(parents=True, exist_ok=True)
    mesh_with_pyramid().write(wd / "mesh.msh")
    write_pro(wd / "proj.pro", "Form1", A_SPACE,
              ["Galerkin { [ Dof{a}, {a} ]; In Vol; Jacobian Jac; Integration Int; }",
               "Galerkin { [ -F1[], {a} ]; In Vol; Jacobian Jac; Integration Int; }"],
              {})
    rc, log = run_getdp(exe, wd, post=False, expect_ok=False)
    chk.check("is not defined on Pyramid elements" in log, f"pyramid guard message missing:\n{log[-1500:]}")


def test_convergence(exe, tmp, chk, rng):
    print("convergence on smoothly mapped prisms under random numberings")

    def rates(label, solve):
        errs = []
        for n in (2, 4, 8, 16):
            m = mesh_extruded(n, n, "random", np.random.default_rng(n))
            m.transform(smooth)
            perm = np.random.default_rng(100 + n).permutation(len(m.xyz))
            errs.append(solve(tmp / f"conv_{label}_{n}", m, perm))
        e = np.array(errs).reshape(len(errs), -1)
        r = np.log2(e[:-1] / e[1:])
        print(f"  {label}: errors {e.T.tolist()} rates {r.T.tolist()}")
        return r[-1]

    noz = [k for k in A_SPACE if k != "vz"]
    r = rates("curl_A", lambda wd, m, p: curl_range(exe, wd, m, A_SPACE, "curlFs[]", p))
    chk.check(r[0] > 1.8, f"curl of the A-space: rate {r[0]:.2f} (want 2)")
    r = rates("curl_A_noZ", lambda wd, m, p: curl_range(exe, wd, m, noz, "curlFs[]", p))
    chk.check(r[0] < 1.5, f"curl of the A-space without 3VZ: rate {r[0]:.2f} (expected to degrade)")

    def full(wd, m, p):
        out = l2_projection(exe, wd, m, FULL_SPACE, "Fs[]", "curlFs[]", p)
        return out["err"], out["errc"]

    r = rates("full", full)
    chk.check(r[0] > 1.8 and r[1] > 1.8, f"full space, field and curl: rates {r[0]:.2f}, {r[1]:.2f} (want 2)")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--getdp", required=True)
    ap.add_argument("--dll-path", default="")
    ap.add_argument("--keep", action="store_true")
    ap.add_argument("--trials", type=int, default=6)
    args = ap.parse_args()
    if args.dll_path:
        os.environ["PATH"] = args.dll_path + os.pathsep + os.environ["PATH"]
    exe = Path(args.getdp).resolve()
    tmp = Path(tempfile.mkdtemp(prefix="prism_order2_"))
    rng = np.random.default_rng(12345)
    chk = Checker()
    try:
        test_quadrature(exe, tmp, chk)
        test_single_element(exe, tmp, chk, rng, args.trials * 4)
        test_exactness(exe, tmp, chk, rng, args.trials)
        test_tet_regression(exe, tmp, chk, rng)
        test_pyramid_guard(exe, tmp, chk)
        test_convergence(exe, tmp, chk, rng)
    finally:
        if args.keep:
            print("kept", tmp)
        else:
            shutil.rmtree(tmp, ignore_errors=True)
    print(f"\n{chk.passed} checks passed, {len(chk.failures)} failed")
    return 1 if chk.failures else 0


if __name__ == "__main__":
    sys.exit(main())
