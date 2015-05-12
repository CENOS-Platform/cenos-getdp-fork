//========================================================
// Benchmark "Academic eigenvalues problems"
// File: GMSH geometry (choice of the cavity)
//========================================================

Include "main.dat" ;

Include Str[LinkGeo] ;

Solver.AutoShowLastStep = 0;
//Solver.AutoCheck = 0;
Solver.AutoShowViews = 2;
Solver.AutoMesh = 2;

General.ScaleX = 1;
General.ScaleY = 1;
General.ScaleZ = 1;
General.TrackballQuaternion0 = 0;
General.TrackballQuaternion1 = 0;
General.TrackballQuaternion2 = 0;
General.TrackballQuaternion3 = 1;
