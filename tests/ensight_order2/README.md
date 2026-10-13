# PrintExternal `Order 2` (quadratic EnSight Gold) checks

`run_tests.py` meshes small domains with the gmsh Python API, Galerkin-projects analytic
fields with GetDP and checks the `Format ENSIGHT, Order 2` output:

- node and element counts against the mesh (vertices + unique edges);
- mid-edge node positions (edge midpoints; the mesh's own mid-nodes on a curved
  second-order mesh) and cell orientation;
- nodal values against the analytic field;
- read-back through VTK's `vtkEnSightGoldBinaryReader` (cell types, interpolation at
  the parametric centre, orientation);
- `Order 1` and no `Order` give byte-identical files (and identical to `--baseline-getdp`);
- error messages for unsupported requests.

It needs gmsh, numpy and (optionally) vtk; the CENOS backend Python has all three. From
PowerShell, with the GetDP runtime DLLs on `PATH`:

```powershell
$env:PATH = "D:\source\cenos\backend\bin;D:\source\cenos\backend\bin\Library\bin;" + $env:PATH
D:\source\cenos\backend\bin\python.exe tests\ensight_order2\run_tests.py --getdp build\getdp.exe `
    --baseline-getdp <getdp.exe built before the change>
```
