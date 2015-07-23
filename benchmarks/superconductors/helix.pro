Include "helix_data.pro";

Group {
  Air = Region[ AIR ];
  Matrix = Region[ {MATRIX} ];
  BndMatrix = Region[ BND_MATRIX ];
  Filaments = Region[{}];
  Terminals = Region[{ (BND_MATRIX + 1), (BND_MATRIX + 2) }];
  For i In {1:NumLayers}
    For j In {1:NumFilaments~{i}}
      Filaments += Region[ (FILAMENT + 1000 * i + j) ];
      Terminals += Region[ (BND_FILAMENT + 1000 * i + j) ];
      Terminals += Region[ (BND_FILAMENT + 1100 * i + j) ];
    EndFor
  EndFor

  OmegaC = Region[{Matrix, Filaments}];
  OmegaCC = Region[{Air}];
  Omega = Region[{OmegaC, OmegaCC}];
  BndOmegaC = Region[{BndMatrix}];

  Cut1AV = Region[ (AIR + 1) ];
  Cut1TO = Region[ (AIR + 2) ];

  BndOmega = Region[ {(BND_AIR),  (BND_AIR+1),  (BND_AIR+2), Terminals} ];
}

Function {

  DefineConstant[
    sigmaMatrix = {6e7,
      Name "Input/4Materials/Matrix conductivity [Sm⁻¹]"},
    Itot = {800/2,
      Name "Input/3Source/Total current [A]"},
    Ec = {1e-4,
      Name "Input/4Materials/Critical electric field [Vm⁻¹]"},
    Jc = {5e8,
      Name "Input/4Materials/Critical current density [Am⁻²]"},
    n = {30, Min 3, Max 40, Step 1, Highlight "LightYellow",
      Name "Input/4Materials/Exponent (n) value"},
    Freq = {50,
      Name "Input/3Source/Frequency [Hz]"},
    periods = {0.25,
      Name "Input/Solver/Periods to simulate [s]"},
    time0 = 0, // initial time
    time1 = periods * (1 / Freq), // final time
    // n = 10 -> 20, dt = 5e-5
    // n = 30, dt = 5e-6
    dt = {5e-5, Min 5e-7, Max 5e-4, Step 1e-6,
      Name "Input/Solver/Time step [s]"}
    theta = 1, // implicit Euler
    tol_abs = {1e-6,
      Name "Input/Solver/{Absolute tolerance on nonlinear residual"},
    tol_rel = {1e-3,
      Name "Input/Solver/{Relative tolerance on nonlinear residual"}
  ];

  mu[Omega] = 4*Pi*1e-7;

  rho[Matrix] = 1 / sigmaMatrix;

  // power law E(J) = rho(J) * J
  rho[Filaments] = Ec / Jc^n * (Norm[$1])^(n - 1);
  dEdJ[Filaments] =
    Ec / Jc^n * (Norm[$1])^(n - 1) * TensorDiag[1.,1.,1.] +
    Ec / Jc^n * (n-1) * (Norm[$1])^(n - 3) *
      Tensor[CompX[$1] * CompX[$1], CompX[$1] * CompY[$1], CompX[$1] * CompZ[$1],
             CompY[$1] * CompX[$1], CompY[$1] * CompY[$1], CompY[$1] * CompZ[$1],
             CompZ[$1] * CompX[$1], CompZ[$1] * CompY[$1], CompZ[$1] * CompZ[$1]];
}

Jacobian {
  { Name Vol ;
    Case { { Region All ; Jacobian Vol ; } }
  }
  { Name Sur ;
    Case { { Region All ; Jacobian Sur ; } }
  }
  { Name Lin ;
    Case { { Region All ; Jacobian Lin ; } }
  }
}

Integration {
  { Name Int ;
    Case { { Type Gauss ;
	Case {
	  { GeoElement Point       ; NumberOfPoints  1 ; }
	  { GeoElement Line        ; NumberOfPoints  3 ; }
	  { GeoElement Triangle    ; NumberOfPoints  4 ; }
	  { GeoElement Quadrangle  ; NumberOfPoints  4 ; }
	  { GeoElement Tetrahedron ; NumberOfPoints  5 ; }
	  { GeoElement Hexahedron  ; NumberOfPoints  6 ; }
	  { GeoElement Prism       ; NumberOfPoints  6 ; }
	}
      } }
  }
}

Constraint {
  { Name VoltageTO ;
    Case {
    }
  }
  { Name CurrentTO ;
    Case {
      { Region Cut1TO; Value Itot ; TimeFunction Sin_wt_p[]{2*Pi*Freq, 0.} ; }
    }
  }
}

FunctionSpace {
  { Name HSpace; Type Form1;
    BasisFunction {
      { Name sn; NameOfCoef phi; Function BF_GradNode;
        Support Omega; Entity NodesOf[OmegaCC]; }
      { Name se; NameOfCoef t; Function BF_Edge;
        Support OmegaC; Entity EdgesOf[All, Not BndOmegaC]; }
      { Name sc1; NameOfCoef I1; Function BF_GroupOfEdges;
        Support Omega; Entity GroupsOfEdgesOf[Cut1TO]; }
    }
    GlobalQuantity {
      { Name Current1 ; Type AliasOf        ; NameOfCoef I1 ; }
      { Name Voltage1 ; Type AssociatedWith ; NameOfCoef I1 ; }
    }
    Constraint {
      { NameOfCoef Current1 ;
        EntityType GroupsOfEdgesOf ; NameOfConstraint CurrentTO ; }
      { NameOfCoef Voltage1 ;
        EntityType GroupsOfEdgesOf ; NameOfConstraint VoltageTO ; }
    }
  }
}

Formulation {
  { Name MagDynTO; Type FemEquation;
    Quantity {
      { Name t; Type Local; NameOfSpace HSpace; }
      { Name I1; Type Global; NameOfSpace HSpace[Current1]; }
      { Name V1; Type Global; NameOfSpace HSpace[Voltage1]; }
    }
    Equation {
      // Nonlinear weak form: Find t_k such that
      //
      //   \partial_t (\mu t_k, t') + (\rho(curl t_k) curl t_k, curl t') = 0,
      //
      // for all t' in Hspace.
      //
      // Linearization:
      //
      //   E(J_k) \approx E(J_k-1) + (dE/dJ)_k-1 * (J_k - J_k-1)
      //
      // i.e.
      //
      //   (\rho(curl t_k) curl t_k, curl t') \approx
      //      (\rho(curl t_k-1) curl t_k-1, curl t')
      //    + (dEdJ(curl t_k-1) curl t_k, curl t')
      //    - (dEdJ(curl t_k-1) curl t_k-1, curl t')
      //
      Galerkin { DtDof [ mu[] * Dof{t} , {t} ];
        In Omega; Integration Int; Jacobian Vol;  }

      Galerkin { [ rho[] * Dof{d t} , {d t} ];
        In Matrix; Integration Int; Jacobian Vol;  }

      Galerkin { [ rho[{d t}] * {d t} , {d t} ];
        In Filaments; Integration Int; Jacobian Vol;  }
      Galerkin { [ dEdJ[{d t}] * Dof{d t} , {d t} ];
        In Filaments; Integration Int; Jacobian Vol;  }
      Galerkin { [ - dEdJ[{d t}] * {d t} , {d t} ];
        In Filaments ; Integration Int; Jacobian Vol;  }

      GlobalTerm { [ -Dof{V1} , {I1} ] ; In Cut1TO ; }
    }
  }
}

Resolution {
  { Name MagDynTOTime;
    System {
      { Name A; NameOfFormulation MagDynTO; }
    }
    Operation {
      SetGlobalSolverOptions["-mat_mumps_icntl_14 500"];
      CreateDirectory["res"];
      InitSolution[A];
      TimeLoopTheta[time0, time1, dt, theta] {
        Generate[A]; Solve[A];
        Generate[A]; GetResidual[A, $res0]; Evaluate[ $res = $res0, $it = 0 ];
        Print[{$it, $res, $res / $res0}, Format "Residual %03g: abs %14.12e rel %14.12e"];
        While[$res > tol_abs && $res / $res0 > tol_rel]{
          Solve[A];
          Generate[A]; GetResidual[A, $res]; Evaluate[ $it = $it + 1 ];
          Print[{$it, $res, $res / $res0}, Format "Residual %03g: abs %14.12e rel %14.12e"];
        }
        SaveSolution[A];
        PostOperation[MagDynTO];
      }
    }
  }

  { Name MagDynTOComplex;
    System {
      { Name A; NameOfFormulation MagDynTO;
        Type ComplexValue; Frequency Freq;}
    }
    Operation {
      CreateDirectory["res"];
      Generate[A]; Solve[A]; SaveSolution[A];
    }
  }
}

PostProcessing {
  { Name MagDynTO; NameOfFormulation MagDynTO;
    Quantity {
      { Name phi; Value{ Local{ [ {dInv t} ] ;
	    In Omega; Jacobian Vol; } } }
      { Name t; Value{ Local{ [ {t} ] ;
	    In Omega; Jacobian Vol; } } }
      { Name h; Value{ Local{ [ {t} ] ;
	    In Omega; Jacobian Vol; } } }
      { Name j; Value{ Local{ [ {d t} ] ;
	    In OmegaC; Jacobian Vol; } } }
      { Name q; Value{ Local{ [ rho[{d t}]*( Re[{d t}]*Re[{d t}] + Im[{d t}]*Im[{d t}]) ] ;
	    In OmegaC; Jacobian Vol; } } }
      { Name b; Value{ Local{ [ mu[]*({t}) ] ;
            In Omega; Jacobian Vol; } } }
      { Name dtb; Value{ Local{ [ mu[]*(Dt [{t}]) ] ;
            In Omega; Jacobian Vol; } } }
      { Name I1 ; Value { Term { [ {I1} ] ; In Cut1TO ; } } }
      { Name V1 ; Value { Term { [ {V1} ] ; In Cut1TO ; } } }
      { Name Z1 ; Value { Term { [ {V1}/{I1} ] ; In Cut1TO ; } } }
    }
  }
}

PostOperation {
  { Name MagDynTO ; NameOfPostProcessing MagDynTO ; LastTimeStepOnly ;
    Operation {
      Print[ h, OnElementsOf Omega , File "res/hTO.pos" ];
      Print[ j, OnElementsOf OmegaC , File "res/jTO.pos" ];
      //Print[ q, OnElementsOf OmegaC , File "res/qTO.pos"] ;
      //Print[I1, OnRegion Cut1TO, File "res/I1TO.pos"];
      //Print[V1, OnRegion Cut1TO, File "res/V1TO.pos"];
      //Print[Z1, OnRegion Cut1TO, File "res/Z1TO.pos"];
    }
  }

}

DefineConstant[
  R_ = {"MagDynTOTime", Name "GetDP/1ResolutionChoices", Visible 0},
  C_ = {"-solve -bin -v 3 -v2", Name "GetDP/9ComputeCommand", Visible 0},
  P_ = {"", Name "GetDP/2PostOperationChoices", Visible 0}
];
