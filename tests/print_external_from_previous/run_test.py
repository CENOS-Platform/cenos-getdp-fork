"""Regression test for PrintExternal_FromPrevious (CENOS complex motion continues EnSight results with it).

    python run_test.py --getdp path/to/getdp.exe

Writes an EnSight case with two earlier time steps, solves a one-triangle problem at t = 0.2
and continues the case with PrintExternal_FromPrevious + PrintExternal. Checks that GetDP exits
cleanly, the case lists all three times and the new step is written as step 2. Runs with the
background EnSight writer and with GETDP_SYNC_WRITE/GETDP_EAGER_GEOM, which CENOS sets.
"""

import argparse
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path

MESH = """$MeshFormat
2.2 0 8
$EndMeshFormat
$Nodes
3
1 0 0 0
2 1 0 0
3 0 1 0
$EndNodes
$Elements
1
1 2 2 1 1 1 2 3
$EndElements
"""

PRO = """Group { Dom = Region[1]; }
Jacobian { { Name J; Case { { Region All; Jacobian Vol; } } } }
Integration {
  { Name I; Case { { Type Gauss;
      Case { { GeoElement Triangle; NumberOfPoints 3; } } } } }
}
FunctionSpace {
  { Name S; Type Form0;
    BasisFunction {
      { Name sn; NameOfCoef un; Function BF_Node; Support Dom; Entity NodesOf[All]; }
    }
  }
}
Formulation {
  { Name F; Type FemEquation;
    Quantity { { Name u; Type Local; NameOfSpace S; } }
    Equation {
      Galerkin { [ Dof{u} , {u} ]; In Dom; Jacobian J; Integration I; }
      Galerkin { [ -1 , {u} ]; In Dom; Jacobian J; Integration I; }
    }
  }
}
Resolution {
  { Name fp;
    System { { Name A; NameOfFormulation F; } }
    Operation { SetTime[0.2]; Generate[A]; Solve[A]; SaveSolution[A]; PostOperation[fp]; }
  }
}
PostProcessing {
  { Name P; NameOfFormulation F;
    Quantity { { Name u; Value { Local { [ {u} ]; In Dom; Jacobian J; } } } }
  }
}
PostOperation {
  { Name fp; NameOfPostProcessing P;
    Operation {
      PrintExternal_FromPrevious[File "results/resFile.0.case"];
      PrintExternal[ PointData { u }, OnElementsOf Dom, File "results/", Format ENSIGHT,
                     SetFrequencyScale "1", LastTimeStepOnly ];
    }
  }
}
"""

PREVIOUS_CASE = """FORMAT
type: ensight gold

GEOMETRY
model: 1 resFileGlobal.0.*****.geo

VARIABLE
scalar per node: 1 u resFile.0.*****_n.u

TIME
time set: 1
number of steps: 2
filename start number: 00000
filename increment: 00001
time values:
0.000000
0.100000
"""


def run_case(exe: str, environment: dict[str, str]) -> list[str]:
    failures = []
    with tempfile.TemporaryDirectory() as tmp:
        work = Path(tmp)
        (work / "fp.msh").write_text(MESH)
        (work / "fp.pro").write_text(PRO)
        (work / "results").mkdir()
        case_file = work / "results" / "resFile.0.case"
        case_file.write_text(PREVIOUS_CASE)
        env = {k: v for k, v in os.environ.items() if not k.startswith("GETDP_")}
        env.update(environment)
        proc = subprocess.run([exe, "fp.pro", "-msh", "fp.msh", "-solve", "fp"], cwd=work, env=env,
                              capture_output=True, text=True, errors="replace")
        if proc.returncode != 0:
            failures.append(f"getdp exit code 0x{proc.returncode & 0xFFFFFFFF:08X}")
            failures += [line for line in (proc.stdout + proc.stderr).splitlines()[-5:]]
            return failures
        text = case_file.read_text()
        steps = re.search(r"number of steps:\s*(\d+)", text)
        times = text.split("time values:")[-1].split()
        if not steps or steps.group(1) != "3":
            failures.append(f"case lists {steps.group(1) if steps else 'no'} steps, expected 3")
        if [float(t) for t in times] != [0.0, 0.1, 0.2]:
            failures.append(f"case time values {times}, expected 0 0.1 0.2")
        for name in ("resFile.0.00002_n.u", "resFileGlobal.0.00002.geo"):
            if not (work / "results" / name).is_file():
                failures.append(f"{name} not written")
    return failures


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--getdp", required=True)
    exe = str(Path(parser.parse_args().getdp).resolve())
    variants = {
        "background writer": {},
        "GETDP_SYNC_WRITE + GETDP_EAGER_GEOM": {"GETDP_SYNC_WRITE": "1", "GETDP_EAGER_GEOM": "1"},
    }
    failed = 0
    for name, environment in variants.items():
        failures = run_case(exe, environment)
        print(f"{'FAIL' if failures else 'ok  '} {name}")
        for failure in failures:
            print(f"     {failure}")
        failed += bool(failures)
    print(f"{len(variants) - failed} of {len(variants)} passed")
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
