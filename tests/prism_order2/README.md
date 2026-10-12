# Second-order prism functions

`run_tests.py` checks `BF_Edge_3FT_a/b`, `BF_Edge_3FQ_a/b/c`, `BF_Edge_3VZ` and the prism
`BF_Node_2F` / `BF_GradNode_2F` on meshes it generates itself (structured prism blocks
extruded along different axes, split tetrahedra on top of prisms), under random node
numberings:

- curl-curl nullspace and linear independence of the spaces on single elements;
- DOF counts (the facet filter: quadrangle-only and triangle-only facet functions);
- exact projections of fields in the discrete spaces, with and without boundary terms and
  `Assign 0` on quadrangles, which require conforming facet functions on every shared facet;
- divergence-free linear fields are curls of the edge + facet + 3VZ space, and are not all
  curls without 3VZ;
- `BF_Edge_3FT_a/b` give the same results as `BF_Edge_3F_a/b` on tetrahedra;
- pyramids in the support of the facet-filtered functions are an error;
- the new quadrature rules (prism tensor products, quadrangle n x n) and second-order
  convergence on smoothly mapped prisms.

It needs numpy; the CENOS backend Python has it. From PowerShell, with the GetDP runtime
DLLs on `PATH`:

```powershell
$env:PATH = "D:\source\cenos\backend\bin;D:\source\cenos\backend\bin\Library\bin;" + $env:PATH
D:\source\cenos\backend\bin\python.exe tests\prism_order2\run_tests.py --getdp build\getdp.exe
```
