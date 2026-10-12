"""Checks PrintExternal[..., Format ENSIGHT, Order 2] against analytic fields.

Needs a Python with gmsh and numpy (vtk optional), e.g. the CENOS backend Python:

    python run_tests.py --getdp path/to/getdp.exe [--baseline-getdp old/getdp.exe]
                        [--dll-path dir1;dir2] [--keep]

Each case meshes a small domain, Galerkin-projects f = 1+x+2y-z+x^2+yz (in the
discrete space, so reproduced exactly) and exports it with Order 2. The checks
are written against the EnSight Gold node ordering and the mesh itself, not
against the writer's own tables.
"""

import argparse
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

import gmsh
import numpy as np

sys.path.insert(0, str(Path(__file__).parent))
import ensight_gold  # noqa: E402

# EnSight Gold node layout as VTK's EnSight reader/writer use it (ParaView):
# (corner slots, [(mid slot, corner slot a, corner slot b)]).
def _corners_first(nv, edges):
    return list(range(nv)), [(nv + k, a, b) for k, (a, b) in enumerate(edges)]


ENSIGHT_LAYOUT = {
    "bar3": ([0, 2], [(1, 0, 2)]),
    "tria6": _corners_first(3, [(0, 1), (1, 2), (2, 0)]),
    "quad8": _corners_first(4, [(0, 1), (1, 2), (2, 3), (3, 0)]),
    "tetra10": _corners_first(4, [(0, 1), (1, 2), (2, 0), (0, 3), (1, 3), (2, 3)]),
    "pyramid13": _corners_first(5, [(0, 1), (1, 2), (2, 3), (3, 0), (0, 4), (1, 4), (2, 4), (3, 4)]),
    "penta15": _corners_first(6, [(0, 1), (1, 2), (2, 0), (3, 4), (4, 5), (5, 3), (0, 3), (1, 4), (2, 5)]),
    "hexa20": _corners_first(
        8, [(0, 1), (1, 2), (2, 3), (3, 0), (4, 5), (5, 6), (6, 7), (7, 4), (0, 4), (1, 5), (2, 6), (3, 7)]
    ),
}
NV = {t: len(c) for t, (c, _) in ENSIGHT_LAYOUT.items()}
LINEAR = {"bar3": "bar2", "tria6": "tria3", "quad8": "quad4", "tetra10": "tetra4",
          "pyramid13": "pyramid5", "penta15": "penta6", "hexa20": "hexa8"}
# Gmsh element type -> (EnSight order-2 type, Gmsh edge table)
GMSH_TYPES = {
    1: ("bar3", [(0, 1)]), 8: ("bar3", [(0, 1)]),
    2: ("tria6", [(0, 1), (1, 2), (2, 0)]), 9: ("tria6", [(0, 1), (1, 2), (2, 0)]),
    3: ("quad8", [(0, 1), (1, 2), (2, 3), (3, 0)]),
    4: ("tetra10", [(0, 1), (1, 2), (2, 0), (3, 0), (3, 2), (3, 1)]),
    11: ("tetra10", [(0, 1), (1, 2), (2, 0), (3, 0), (3, 2), (3, 1)]),
    5: ("hexa20", [(0, 1), (0, 3), (0, 4), (1, 2), (1, 5), (2, 3), (2, 6), (3, 7), (4, 5), (4, 7), (5, 6), (6, 7)]),
    6: ("penta15", [(0, 1), (0, 2), (0, 3), (1, 2), (1, 4), (2, 5), (3, 4), (3, 5), (4, 5)]),
    7: ("pyramid13", [(0, 1), (0, 3), (0, 4), (1, 2), (1, 4), (2, 3), (2, 4), (3, 4)]),
}


def f(p):
    x, y, z = p
    return 1 + x + 2 * y - z + x * x + y * z


def flin(p):
    x, y, z = p
    return 1 + x + 2 * y - z


def vlin(p):
    x, y, z = p
    return (1 + y, x - z, y + 2 * z)


# --------------------------------------------------------------------------- meshes


def _write(path, order=1):
    if order > 1:
        gmsh.model.mesh.setOrder(order)
    gmsh.option.setNumber("Mesh.MshFileVersion", 2.2)
    gmsh.write(str(path))


def mesh_tet(path):
    gmsh.model.add("tet")
    gmsh.model.occ.addBox(0, 0, 0, 1, 1, 1)
    gmsh.model.occ.synchronize()
    gmsh.model.addPhysicalGroup(3, [1], 1)
    gmsh.model.addPhysicalGroup(2, [s for _, s in gmsh.model.getEntities(2)], 2)
    gmsh.option.setNumber("Mesh.MeshSizeMax", 0.4)
    gmsh.model.mesh.generate(3)
    _write(path)


def mesh_mixed2d(path):
    gmsh.model.add("mixed2d")
    geo = gmsh.model.geo
    p = [geo.addPoint(x, y, 0, 0.3) for x, y in [(0, 0), (1, 0), (2, 0), (2, 1), (1, 1), (0, 1)]]
    l1, l2, l3, l4, l5, l6 = (geo.addLine(p[i], p[(i + 1) % 6]) for i in range(6))
    lm = geo.addLine(p[1], p[4])
    s1 = geo.addPlaneSurface([geo.addCurveLoop([l1, lm, l5, l6])])
    s2 = geo.addPlaneSurface([geo.addCurveLoop([l2, l3, l4, -lm])])
    geo.synchronize()
    for c in (l2, l3, l4, lm):
        gmsh.model.mesh.setTransfiniteCurve(c, 4)
    gmsh.model.mesh.setTransfiniteSurface(s2)
    gmsh.model.mesh.setRecombine(2, s2)
    gmsh.model.addPhysicalGroup(2, [s1, s2], 1)
    gmsh.model.addPhysicalGroup(1, [l1, l2, l3, l4, l5, l6], 2)
    gmsh.model.mesh.generate(2)
    _write(path)


def _extruded_block(name, recombine_base):
    gmsh.model.add(name)
    geo = gmsh.model.geo
    p = [geo.addPoint(x, y, 0, 0.35) for x, y in [(0, 0), (1, 0), (1, 1), (0, 1)]]
    lines = [geo.addLine(p[i], p[(i + 1) % 4]) for i in range(4)]
    s = geo.addPlaneSurface([geo.addCurveLoop(lines)])
    if recombine_base:
        # through the geo kernel: the extruded layers copy the base surface's mesh
        for c in lines:
            geo.mesh.setTransfiniteCurve(c, 4)
        geo.mesh.setTransfiniteSurface(s)
        geo.mesh.setRecombine(2, s)
    out = geo.extrude([(2, s)], 0, 0, 1, numElements=[3], recombine=True)
    geo.synchronize()
    vol = [t for d, t in out if d == 3]
    gmsh.model.addPhysicalGroup(3, vol, 1)
    gmsh.model.addPhysicalGroup(2, [t for _, t in gmsh.model.getBoundary([(3, v) for v in vol])], 2)
    gmsh.model.mesh.generate(3)


def mesh_hex(path):
    _extruded_block("hex", True)
    _write(path)


def mesh_prism(path):
    _extruded_block("prism", False)
    _write(path)


def mesh_pyramid(path):
    nodes = [(0, 0, 0), (1, 0, 0), (1, 1, 0), (0, 1, 0), (0, 0, 1), (1, 0, 1), (1, 1, 1), (0, 1, 1), (0.5, 0.5, 0.5)]
    # every base is ordered so that its right-hand normal points to the apex (node 9)
    pyramids = [(1, 2, 3, 4), (5, 8, 7, 6), (1, 5, 6, 2), (2, 6, 7, 3), (3, 7, 8, 4), (1, 4, 8, 5)]
    lines = ["$MeshFormat", "2.2 0 8", "$EndMeshFormat", "$Nodes", str(len(nodes))]
    lines += [f"{i + 1} {x} {y} {z}" for i, (x, y, z) in enumerate(nodes)]
    lines += ["$EndNodes", "$Elements", str(len(pyramids))]
    lines += [f"{i + 1} 7 2 1 1 {' '.join(map(str, b))} 9" for i, b in enumerate(pyramids)]
    lines += ["$EndElements"]
    Path(path).write_text("\n".join(lines) + "\n")


def mesh_tet2_curved(path, order=2):
    gmsh.model.add("tet2")
    gmsh.model.occ.addCylinder(0, 0, 0, 0, 0, 1, 0.5)
    gmsh.model.occ.synchronize()
    gmsh.model.addPhysicalGroup(3, [1], 1)
    gmsh.model.addPhysicalGroup(2, [s for _, s in gmsh.model.getEntities(2)], 2)
    gmsh.option.setNumber("Mesh.MeshSizeMax", 0.3)
    gmsh.option.setNumber("Mesh.HighOrderOptimize", 0)
    gmsh.model.mesh.generate(3)
    _write(path, order)


def mesh_line3(path):
    """Two linear triangles (Vol) and one third-order 4-node line (Bnd) on y=0."""
    nodes = [(0, 0), (1, 0), (1, 1), (0, 1), (1 / 3, 0), (2 / 3, 0)]
    lines = ["$MeshFormat", "2.2 0 8", "$EndMeshFormat", "$Nodes", str(len(nodes))]
    lines += [f"{i + 1} {x} {y} 0" for i, (x, y) in enumerate(nodes)]
    lines += ["$EndNodes", "$Elements", "3",
              "1 2 2 1 1 1 2 3", "2 2 2 1 1 1 3 4", "3 26 2 2 2 1 2 5 6", "$EndElements"]
    Path(path).write_text("\n".join(lines) + "\n")


# --------------------------------------------------------------------------- .pro

PRO = """
Group {{
  Vol = Region[{{1}}];
  Bnd = Region[{{2}}];
  Dom = Region[{{Vol, Bnd}}];
}}
Function {{
  f[] = 1 + X[] + 2*Y[] - Z[] + X[]^2 + Y[]*Z[];
  flin[] = 1 + X[] + 2*Y[] - Z[];
  vlin[] = Vector[1 + Y[], X[] - Z[], Y[] + 2*Z[]];
}}
Jacobian {{ {{ Name Jac; Case {{ {{ Region Bnd; Jacobian {bnd_jac}; }} {{ Region All; Jacobian Vol; }} }} }} }}
Integration {{ {{ Name Int; Case {{ {{ Type {integ}; Case {{
  {integ_cases}
}} }} }} }} }}
FunctionSpace {{
  {{ Name H; Type Form0; BasisFunction {{
    {{ Name sn; NameOfCoef un; Function BF_Node; Support Dom; Entity NodesOf[All]; }}
    {h2}
  }} }}
  {hcurl_space}
}}
Formulation {{ {{ Name Proj; Type FemEquation;
  Quantity {{ {{ Name u; Type Local; NameOfSpace H; }} {hcurl_quantity} }}
  Equation {{
    Galerkin {{ [ Dof{{u}}, {{u}} ]; In Vol; Jacobian Jac; Integration Int; }}
    Galerkin {{ [ -{fu}[], {{u}} ]; In Vol; Jacobian Jac; Integration Int; }}
    {hcurl_equation}
  }}
}} }}
Resolution {{ {{ Name Proj; System {{ {{ Name S; NameOfFormulation Proj; }} }}
  Operation {{ Generate[S]; Solve[S]; SaveSolution[S]; }} }} }}
PostProcessing {{ {{ Name Proj; NameOfFormulation Proj; Quantity {{
  {{ Name u; Value {{ Local {{ [ {{u}} ]; In Dom; Jacobian Jac; }} }} }}
  {{ Name fa; Value {{ Local {{ [ f[] ]; In Dom; Jacobian Jac; }} }} }}
  {{ Name fl; Value {{ Local {{ [ flin[] ]; In Dom; Jacobian Jac; }} }} }}
  {{ Name err2; Value {{ Integral {{ [ ({{u}} - {fu}[])^2 ]; In Vol; Jacobian Jac; Integration Int; }} }} }}
  {hcurl_post}
}} }} }}
PostOperation {{ {{ Name Out; NameOfPostProcessing Proj; Operation {{
  CreateDir["{out}"];
  {prints}
  Print[ err2[Vol], OnGlobal, Format Table, File "{out}err2.txt" ];
  {hcurl_print}
  {extra}
}} }} }}
"""

H_EXTRA = {
    "2E": "{ Name se; NameOfCoef ue; Function BF_Node_2E; Support Dom; Entity EdgesOf[All]; }",
    # quadratic completeness on quads/hexes: GetDP's 2E functions on them are
    # products of nodal functions and do not span x^2 alone
    "2F": "{ Name sf; NameOfCoef uf; Function BF_Node_2F; Support Dom; Entity FacetsOf[All]; }",
    "2V": "{ Name sv; NameOfCoef uv; Function BF_Node_2V; Support Dom; Entity VolumesOf[All]; }",
}
# Type Gauss has at most 7 points on quadrangles, fewer than the 8-9 local
# second-order dofs: the discrete mass matrix would be singular
INTEGRATION = {
    "Gauss": {"Point": 1, "Line": 4, "Line2": 4, "Line3": 4, "Triangle": 12, "Triangle2": 12, "Triangle3": 12,
              "Tetrahedron": 15, "Tetrahedron2": 15, "Prism": 21, "Pyramid": 8},
    "GaussLegendre": {"Point": 1, "Line": 4, "Line2": 4, "Triangle": 16, "Quadrangle": 16, "Tetrahedron": 64,
                      "Hexahedron": 27, "Prism": 21, "Pyramid": 8},
}
HCURL_SPACE = """{ Name E; Type Form1; BasisFunction {
    { Name we; NameOfCoef ae; Function BF_Edge; Support Dom; Entity EdgesOf[All]; }
    { Name we2; NameOfCoef ae2; Function BF_Edge_2E; Support Dom; Entity EdgesOf[All]; }
    { Name wfa; NameOfCoef afa; Function BF_Edge_3F_a; Support Dom; Entity FacetsOf[All]; }
    { Name wfb; NameOfCoef afb; Function BF_Edge_3F_b; Support Dom; Entity FacetsOf[All]; }
  } }"""
HCURL_QUANTITY = "{ Name a; Type Local; NameOfSpace E; }"
HCURL_EQUATION = """Galerkin { [ Dof{a}, {a} ]; In Vol; Jacobian Jac; Integration Int; }
    Galerkin { [ -vlin[], {a} ]; In Vol; Jacobian Jac; Integration Int; }"""
HCURL_POST = """{ Name a; Value { Local { [ {a} ]; In Dom; Jacobian Jac; } } }
  { Name errA2; Value { Integral { [ SquNorm[{a} - vlin[]] ]; In Vol; Jacobian Jac; Integration Int; } } }"""


def write_pro(path, case, out, order, smoothing, fmt="ENSIGHT", extra=""):
    quantities = "u, fa, fl" + (", a" if case["hcurl"] else "")
    opts = f'Format {fmt}, SetFrequencyScale "1", LastTimeStepOnly, Smoothing {smoothing}'
    if order is not None:
        opts += f", Order {order}"
    groups = ["Vol"] + (["Bnd"] if case["bnd"] else [])
    prints = "\n  ".join(
        f'PrintExternal[ PointData {{ {quantities} }}, OnElementsOf {g}, File "{out}", {opts} ];' for g in groups
    )
    Path(path).write_text(
        PRO.format(
            bnd_jac=case["bnd_jac"],
            integ=case["integ"],
            integ_cases="\n  ".join(
                f"{{ GeoElement {g}; NumberOfPoints {n}; }}" for g, n in INTEGRATION[case["integ"]].items()
            ),
            h2="\n    ".join(H_EXTRA[k] for k in case["h"]),
            fu=case["fu"],
            hcurl_space=HCURL_SPACE if case["hcurl"] else "",
            hcurl_quantity=HCURL_QUANTITY if case["hcurl"] else "",
            hcurl_equation=HCURL_EQUATION if case["hcurl"] else "",
            hcurl_post=HCURL_POST if case["hcurl"] else "",
            hcurl_print=f'Print[ errA2[Vol], OnGlobal, Format Table, File "{out}errA2.txt" ];' if case["hcurl"] else "",
            out=out,
            prints=prints,
            extra=extra,
        )
    )


def run_getdp(exe, workdir, pro="proj.pro", expect_ok=True):
    cmd = [str(exe), pro, "-msh", "mesh.msh", "-solve", "Proj", "-pos", "Out", "-v", "3"]
    r = subprocess.run(cmd, cwd=workdir, capture_output=True, text=True, timeout=600)
    log = r.stdout + r.stderr
    if expect_ok and (r.returncode != 0 or "Error" in log):
        raise RuntimeError(f"getdp failed ({r.returncode}) in {workdir}:\n{log[-3000:]}")
    return r.returncode, log


# --------------------------------------------------------------------------- checks


class Checker:
    def __init__(self):
        self.failures = []
        self.passed = 0

    def check(self, ok, msg):
        if ok:
            self.passed += 1
        else:
            self.failures.append(msg)
            print("  FAIL:", msg)


def read_out(outdir, case):
    outdir = Path(outdir)
    geo = ensight_gold.read_geometry(outdir / "resFileGlobal.0.00000.geo")
    vars_ = {"u": 1, "fa": 1, "fl": 1}
    if case["hcurl"]:
        vars_["a"] = 3
    vals = {n: ensight_gold.read_variable(outdir / f"resFile.0.00000_n.{n}", geo, c) for n, c in vars_.items()}
    return geo, vals


def mesh_reference(mesh_path):
    """Per physical tag: element counts per EnSight order-2 type and vertex/edge counts.
    Also the mesh node coordinates and, for second-order meshes, the mesh's own
    mid-edge node per sorted corner-tag pair."""
    gmsh.model.add("ref")
    gmsh.merge(str(mesh_path))
    tags, xyz, _ = gmsh.model.mesh.getNodes()
    xyz = np.asarray(xyz).reshape(-1, 3)
    coords = {int(t): xyz[i] for i, t in enumerate(tags)}
    ref = {"tags": np.asarray(tags, dtype=int), "xyz": xyz, "mid": {}}
    for dim, ptag in gmsh.model.getPhysicalGroups():
        counts, verts, edges = {}, set(), set()
        for ent in gmsh.model.getEntitiesForPhysicalGroup(dim, ptag):
            etypes, _, enodes = gmsh.model.mesh.getElements(dim, ent)
            for et, en in zip(etypes, enodes):
                etype, gedges = GMSH_TYPES[int(et)]
                npe = gmsh.model.mesh.getElementProperties(int(et))[3]
                nv = NV[etype]
                for k in range(len(en) // npe):
                    nodes = [int(n) for n in en[k * npe : (k + 1) * npe]]
                    counts[etype] = counts.get(etype, 0) + 1
                    verts.update(nodes[:nv])
                    for e, (a, b) in enumerate(gedges):
                        key = tuple(sorted((nodes[a], nodes[b])))
                        edges.add(key)
                        if npe > nv:
                            ref["mid"][key] = coords[nodes[nv + e]]
        ref[ptag] = {"counts": counts, "nverts": len(verts), "nedges": len(edges)}
    gmsh.model.remove()
    return ref


def nearest_tags(ref, pts):
    """Mesh node tag of the closest mesh node for every point (exported corners)."""
    out = np.empty(len(pts), dtype=int)
    for s in range(0, len(pts), 512):
        d = np.linalg.norm(pts[s : s + 512, None, :] - ref["xyz"][None, :, :], axis=2)
        out[s : s + 512] = ref["tags"][d.argmin(axis=1)]
    return out


def signed_volume(etype, c):
    c = np.asarray(c)
    if etype in ("tetra10", "penta15"):
        return np.linalg.det([c[1] - c[0], c[2] - c[0], c[3] - c[0]])
    if etype in ("hexa20", "pyramid13"):
        return np.linalg.det([c[1] - c[0], c[3] - c[0], c[4] - c[0]])
    return None


def check_case(chk, name, case, workdir, ref):
    o2 = workdir / "out_o2s1/"
    geo, vals = read_out(o2, case)
    geo1, _ = read_out(workdir / "out_o1/", case)
    allxyz = np.array([p for part in geo.values() for p in part["xyz"]])
    L = float(np.linalg.norm(allxyz.max(0) - allxyz.min(0)))
    fmax = max(abs(f(p)) for p in allxyz)

    for pnum, part in geo.items():
        ptag = 1 if part["name"] == "Vol" else 2
        r = ref[ptag]
        xyz = np.array(part["xyz"])
        # 1. counts
        exp_nodes = r["nverts"] + r["nedges"]
        chk.check(len(xyz) == exp_nodes, f"{name}/{part['name']}: {len(xyz)} nodes, expected {exp_nodes}")
        got_counts = {t: len(b[0]) for t, b in part["blocks"].items()}
        chk.check(got_counts == r["counts"], f"{name}/{part['name']}: element counts {got_counts} != {r['counts']}")
        # corners of the Order 1 export, by element id
        p1 = next(p for p in geo1.values() if p["name"] == part["name"])
        corners1 = {}
        for t1, (ids1, conn1) in p1["blocks"].items():
            for eid, cn in zip(ids1, conn1):
                corners1[eid] = (t1, np.array([p1["xyz"][i] for i in cn]))
        node_tag = nearest_tags(ref, xyz) if case["curved"] else None
        nbad_mid = nbad_vol = nbad_corner = 0
        for etype, (ids, conn) in part["blocks"].items():
            corner_slots, mids = ENSIGHT_LAYOUT[etype]
            for eid, cn in zip(ids, conn):
                c = xyz[cn]
                # 2. mid-edge geometry
                for m, a, b in mids:
                    if case["curved"]:
                        key = tuple(sorted((node_tag[cn[a]], node_tag[cn[b]])))
                        expected = ref["mid"].get(key, np.full(3, np.inf))
                    else:
                        expected = 0.5 * (c[a] + c[b])
                    if np.linalg.norm(c[m] - expected) > 1e-6 * L:
                        nbad_mid += 1
                # 3. orientation and corners vs Order 1
                vol = signed_volume(etype, c[corner_slots])
                if vol is not None and vol <= 0:
                    nbad_vol += 1
                t1, c1 = corners1.get(eid, (None, None))
                if t1 != LINEAR[etype] or np.abs(c1 - c[corner_slots]).max() > 1e-9 * L:
                    nbad_corner += 1
        chk.check(nbad_mid == 0, f"{name}/{part['name']}: {nbad_mid} mid-edge nodes off their edge/mesh node")
        chk.check(nbad_vol == 0, f"{name}/{part['name']}: {nbad_vol} cells with non-positive corner volume")
        chk.check(nbad_corner == 0, f"{name}/{part['name']}: {nbad_corner} cells whose corners differ from Order 1")
        # 4. values
        fa = np.array([v[0] for v in vals["fa"][pnum]])
        fexact = np.array([f(p) for p in xyz])
        chk.check(np.abs(fa - fexact).max() <= 1e-5 * fmax, f"{name}/{part['name']}: analytic f differs at nodes")
        if case["exact_u"]:
            u = np.array([v[0] for v in vals["u"][pnum]])
            fu = fexact if case["fu"] == "f" else np.array([flin(p) for p in xyz])
            err = np.abs(u - fu).max()
            chk.check(err <= 1e-5 * fmax, f"{name}/{part['name']}: projected u differs from f by {err:g}")
        if case["hcurl"] and part["name"] == "Vol":
            a = np.array(vals["a"][pnum])
            va = np.array([vlin(p) for p in xyz])
            err = np.abs(a - va).max()
            chk.check(err <= 1e-5 * np.abs(va).max(), f"{name}/{part['name']}: projected a differs from v by {err:g}")

    # premise of the exactness checks
    if case["exact_u"]:
        err2 = float((workdir / "out_o2s1" / "err2.txt").read_text().split()[-1])
        chk.check(err2 < 1e-20, f"{name}: projection residual err2={err2:g} (premise)")
    # 5. smoothing on/off agree for continuous fields
    geo0, vals0 = read_out(workdir / "out_o2s0/", case)
    for pnum in geo:
        for n in ("u", "fa"):
            d = np.abs(np.array(vals[n][pnum]) - np.array(vals0[n][pnum])).max()
            chk.check(d <= 1e-6 * fmax, f"{name}: Smoothing 0/1 differ for {n} by {d:g}")


def check_vtk(chk, name, outdir, curved):
    """Curved cells are neither affine nor exact for P2 fields: only type and
    orientation are checked for them."""
    try:
        import vtk
    except ImportError:
        print("  (vtk not available, VTK checks skipped)")
        return
    from vtk.util.numpy_support import vtk_to_numpy

    reader = vtk.vtkEnSightGoldBinaryReader()
    reader.SetCaseFileName(str(Path(outdir) / "resFile.0.case"))
    reader.Update()
    mb = reader.GetOutput()
    quadratic = {vtk.VTK_QUADRATIC_EDGE: (vtk.vtkLine, "linear"), vtk.VTK_QUADRATIC_TRIANGLE: (vtk.vtkTriangle, 3),
                 vtk.VTK_QUADRATIC_QUAD: (vtk.vtkQuad, 4), vtk.VTK_QUADRATIC_TETRA: (vtk.vtkTetra, 4),
                 vtk.VTK_QUADRATIC_HEXAHEDRON: (vtk.vtkHexahedron, 8), vtk.VTK_QUADRATIC_WEDGE: (vtk.vtkWedge, 6),
                 vtk.VTK_QUADRATIC_PYRAMID: (vtk.vtkPyramid, 5)}
    nbad_type = nbad_val = nbad_geo = nbad_orient = ncells = 0
    it = mb.NewIterator()
    it.InitTraversal()
    while not it.IsDoneWithTraversal():
        grid = it.GetCurrentDataObject()
        fa = vtk_to_numpy(grid.GetPointData().GetArray("fa"))
        fl = vtk_to_numpy(grid.GetPointData().GetArray("fl"))
        for ic in range(grid.GetNumberOfCells()):
            cell = grid.GetCell(ic)
            ncells += 1
            ct = cell.GetCellType()
            if ct not in quadratic:
                nbad_type += 1
                continue
            npts = cell.GetNumberOfPoints()
            ids = [cell.GetPointId(i) for i in range(npts)]
            pc = [0.0, 0.0, 0.0]
            cell.GetParametricCenter(pc)
            x = [0.0, 0.0, 0.0]
            w = [0.0] * npts
            cell.EvaluateLocation(vtk.mutable(0), pc, x, w)
            # pyramid13 does not reproduce P2: check the linear field there
            field, fn = (fl, flin) if ct == vtk.VTK_QUADRATIC_PYRAMID else (fa, f)
            val = sum(wi * field[i] for wi, i in zip(w, ids))
            if not curved and abs(val - fn(x)) > 1e-5 * max(1.0, abs(fn(x))):
                nbad_val += 1
            lin_cls, _ = quadratic[ct]
            lin = lin_cls()
            nc = lin.GetNumberOfPoints()
            for i in range(nc):
                lin.GetPoints().SetPoint(i, grid.GetPoint(ids[i]))
                lin.GetPointIds().SetId(i, i)
            xl = [0.0, 0.0, 0.0]
            wl = [0.0] * nc
            lin.EvaluateLocation(vtk.mutable(0), pc, xl, wl)
            if not curved and np.linalg.norm(np.subtract(x, xl)) > 1e-6:
                nbad_geo += 1
            if lin.GetCellDimension() == 3:
                c = np.array([grid.GetPoint(ids[i]) for i in range(nc)])
                if ct == vtk.VTK_QUADRATIC_WEDGE:  # VTK wedge: base normal points away from the top
                    ok = np.linalg.det([c[1] - c[0], c[2] - c[0], c[3] - c[0]]) < 0
                elif ct == vtk.VTK_QUADRATIC_TETRA:
                    ok = np.linalg.det([c[1] - c[0], c[2] - c[0], c[3] - c[0]]) > 0
                else:
                    ok = np.linalg.det([c[1] - c[0], c[3] - c[0], c[4] - c[0]]) > 0
                nbad_orient += not ok
        it.GoToNextItem()
    chk.check(ncells > 0, f"{name}: VTK read no cells")
    chk.check(nbad_type == 0, f"{name}: VTK: {nbad_type} cells not read as quadratic")
    chk.check(nbad_val == 0, f"{name}: VTK: {nbad_val} cells interpolate f wrongly at the parametric centre")
    chk.check(nbad_geo == 0, f"{name}: VTK: {nbad_geo} cells with a non-affine (mis-ordered) geometry")
    chk.check(nbad_orient == 0, f"{name}: VTK: {nbad_orient} cells inverted in VTK convention")


def check_bytes(chk, name, dirs):
    base = dirs[0]
    for other in dirs[1:]:
        files = sorted(p.name for p in base.iterdir() if p.name.startswith("resFile"))
        same = files == sorted(p.name for p in other.iterdir() if p.name.startswith("resFile"))
        diff = [fn for fn in files if same and (base / fn).read_bytes() != (other / fn).read_bytes()]
        chk.check(same and not diff, f"{name}: {base.name} vs {other.name} differ: {diff or 'file lists'}")


# --------------------------------------------------------------------------- main

_BASE = dict(bnd=True, bnd_jac="Sur", integ="Gauss", h=["2E"], fu="f", hcurl=False, exact_u=True, curved=False)
CASES = {
    "tet": dict(_BASE, mesh=mesh_tet, hcurl=True),
    "mixed2d": dict(_BASE, mesh=mesh_mixed2d, integ="GaussLegendre", h=["2E", "2F"]),
    "hex": dict(_BASE, mesh=mesh_hex, integ="GaussLegendre", h=["2E", "2F", "2V"]),
    # the linear field: baselines older than the prism BF_Node_2F (checked in
    # tests/prism_order2) do not span P2 on prisms (still exported through penta15)
    "prism": dict(_BASE, mesh=mesh_prism, fu="flin"),
    # BF_Node_2E has no pyramid implementation
    "pyramid": dict(_BASE, mesh=mesh_pyramid, bnd=False, h=[], fu="flin"),
    "tet2curved": dict(_BASE, mesh=mesh_tet2_curved, exact_u=False, curved=True),
}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--getdp", required=True)
    ap.add_argument("--baseline-getdp")
    ap.add_argument("--dll-path", default="")
    ap.add_argument("--keep", action="store_true")
    ap.add_argument("--cases", default=",".join(CASES))
    args = ap.parse_args()
    if args.dll_path:
        os.environ["PATH"] = args.dll_path + os.pathsep + os.environ["PATH"]

    root = Path(tempfile.mkdtemp(prefix="ensight_order2_"))
    print("work dir:", root)
    chk = Checker()
    gmsh.initialize()
    gmsh.option.setNumber("General.Terminal", 0)
    try:
        for name in args.cases.split(","):
            case = CASES[name]
            print(f"[{name}]")
            wd = root / name
            wd.mkdir()
            gmsh.clear()
            case["mesh"](wd / "mesh.msh")
            gmsh.clear()
            ref = mesh_reference(wd / "mesh.msh")
            variants = [("out_o2s1/", 2, 1, args.getdp), ("out_o2s0/", 2, 0, args.getdp),
                        ("out_o1/", 1, 1, args.getdp), ("out_none/", None, 1, args.getdp)]
            if args.baseline_getdp and not case["curved"]:
                variants.append(("out_base/", None, 1, args.baseline_getdp))
            for out, order, sm, exe in variants:
                write_pro(wd / "proj.pro", case, out, order, sm)
                run_getdp(exe, wd)
            check_case(chk, name, case, wd, ref)
            check_vtk(chk, name, wd / "out_o2s1", case["curved"])
            dirs = [wd / "out_none", wd / "out_o1"] + ([wd / "out_base"] if (wd / "out_base").exists() else [])
            check_bytes(chk, name, dirs)
            if case["curved"]:
                # second-order mesh, Order 1: linear cells on the vertices only
                g1, _ = read_out(wd / "out_o1/", case)
                types = sorted({t for p in g1.values() for t in p["blocks"]})
                chk.check(types == ["tetra4", "tria3"], f"{name}: Order 1 on a P2 mesh wrote {types}")
                nn = sum(len(p["node_ids"]) for p in g1.values())
                chk.check(nn == ref[1]["nverts"] + ref[2]["nverts"], f"{name}: Order 1 node count {nn}")

        print("[errors]")
        wd = root / "errors"
        wd.mkdir()
        gmsh.clear()
        mesh_tet(wd / "mesh.msh")
        for label, order, fmt, needle in [("Order 3", 3, "ENSIGHT", "Order must be 1 or 2"),
                                          ("Order 2 + VTU", 2, "VTU", "only available with Format ENSIGHT")]:
            write_pro(wd / "proj.pro", CASES["tet"], "out/", order, 1, fmt=fmt)
            rc, log = run_getdp(args.getdp, wd, expect_ok=False)
            chk.check(needle in log and abs(rc) < 256, f"errors: {label}: rc={rc}, message missing:\n{log[-800:]}")
        mesh_line3(wd / "mesh.msh")
        case = dict(_BASE, h=[], fu="flin", exact_u=False)
        write_pro(wd / "proj.pro", case, "out/", 2, 1)
        rc, log = run_getdp(args.getdp, wd, expect_ok=False)
        # an error after an earlier PrintExternal queued a write ends in
        # std::terminate (writer thread still joinable at exit): only a clean
        # message and no access violation are required here
        chk.check("does not support elements of type" in log and rc != 0 and rc != 0xC0000005,
                  f"errors: third-order mesh: rc={rc}, message missing:\n{log[-800:]}")
    finally:
        gmsh.finalize()
        if not args.keep:
            shutil.rmtree(root, ignore_errors=True)

    print(f"\n{chk.passed} checks passed, {len(chk.failures)} failed")
    return 1 if chk.failures else 0


if __name__ == "__main__":
    sys.exit(main())
