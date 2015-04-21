
Jacobian {
  { Name JVol ;
    Case {
      { Region All ; Jacobian Vol ; }
    }
  }
  { Name JSur ; Case { { Region All ; Jacobian Sur; } } }
}

Integration {
  { Name I ; Case {
      { Type Gauss ;
        Case {
          { GeoElement Triangle    ; NumberOfPoints  6 ; }
          { GeoElement Quadrangle  ; NumberOfPoints  7 ; }
          { GeoElement Tetrahedron ; NumberOfPoints  4 ; }
        }
      }
    }
  }
}

Constraint {

  { Name MVP  ; Type Assign ;
    Case {
      { Region SurfacesDirichletBC ; Value 0. ; }
    }
  }

  { Name Current_2D  ; Type Assign ; // Zero net current in each lamination
    Case {
      For k In {1:N}
        { Region Iron~{k} ; Value 0. ; }
      EndFor
    }
  }

  { Name Voltage_2D  ; Type Assign ;
    Case {
    }
  }

}

FunctionSpace {
  { Name Hcurl_a ; Type Form1P ;
    BasisFunction {
      { Name se  ; NameOfCoef ae  ; Function BF_PerpendicularEdge ;
        Support Domain ; Entity NodesOf[All] ; }
     }
    Constraint {
      { NameOfCoef ae ; EntityType NodesOf ; NameOfConstraint MVP ; }
    }
  }
  // Gradient of Electric scalar potential (2D)
  //===========================================
  { Name Hregion_u_2D ; Type Form1P ;
    BasisFunction {
      { Name sr ; NameOfCoef ur ; Function BF_RegionZ ;
        Support DomainC ; Entity DomainC ; }
    }
    GlobalQuantity {
      { Name U ; Type AliasOf        ; NameOfCoef ur ; }
      { Name I ; Type AssociatedWith ; NameOfCoef ur ; }
    }
    Constraint {
      { NameOfCoef U ; EntityType Region ; NameOfConstraint Voltage_2D ; }
      { NameOfCoef I ; EntityType Region ; NameOfConstraint Current_2D ; }
    }
  }
}


Formulation {
  // ====================================================================
  // NONLINEAR - single value b-h curve
  // ====================================================================

  { Name MagStaDyn_a_ref; Type FemEquation ;
    Quantity {
      { Name a  ; Type Local  ; NameOfSpace Hcurl_a ; }
      { Name ur ; Type Local  ; NameOfSpace Hregion_u_2D ; }
      { Name I  ; Type Global ; NameOfSpace Hregion_u_2D [I] ; }
      { Name U  ; Type Global ; NameOfSpace Hregion_u_2D [U] ; }
    }
    Equation {
      Galerkin { [ nu[{d a}] * Dof{d a} , {d a} ]  ; In Domain ; Jacobian JVol ; Integration I ; }
      Galerkin { JacNL[ dhdb_NL[{d a}]* Dof{d a} , {d a} ] ; In DomainNL ; Jacobian JVol ; Integration I ; }

      Galerkin { DtDof[ sigma[] * Dof{a} , {a} ]   ; In DomainC ; Jacobian JVol ; Integration I ; }
      Galerkin { [      sigma[] * Dof{ur}, {a} ]   ; In DomainC ; Jacobian JVol ; Integration I ; }

      Galerkin { DtDof[ sigma[] * Dof{a} , {ur} ]  ; In DomainC ; Jacobian JVol ; Integration I ; }
      Galerkin { [      sigma[] * Dof{ur}, {ur} ]  ; In DomainC ; Jacobian JVol ; Integration I ; }
      GlobalTerm { [ Dof{I} , {U} ]                ; In DomainC ; }

      Galerkin { [ -js[] , {a} ] ; In DomainS0     ; Jacobian JVol; Integration I ; }
    }
  }
}

Resolution {
  { Name MagStaDyn_a_ref ;
    System {
      { Name A ; NameOfFormulation MagStaDyn_a_ref; }
    }
    Operation {
      CreateDirectory[Dir_Ref];
      InitSolution[A] ; SaveSolution[A] ;
      TimeLoopTheta[ time0, timemax, dtime, theta_value]{
        IterativeLoop[Nb_max_iter, reltol, relaxation_factor[]] {
          GenerateJac[A]; SolveJac[A];
        }
        SaveSolution[A];
      }
    }
  }
}


PostProcessing {
  //=================================================
  // NONLINEAR
  //=================================================
  { Name MagStaDyn_a_ref ; NameOfFormulation MagStaDyn_a_ref ;
    PostQuantity {
      { Name a ; Value { Local { [ {a} ]; In Domain ; Jacobian JVol; } } }
      { Name az; Value { Local { [ CompZ[{a}] ]; In Domain ; Jacobian JVol; } } }
      { Name b ; Value { Local { [ {d a} ]; In Domain ; Jacobian JVol; } } }
      { Name h ; Value { Local { [ nu[ {d a} ] * {d a} ]; In Domain  ; Jacobian JVol; } } }
      { Name j ; Value { Local { [ -sigma[] * ( Dt[ {a} ] + {ur} ) ]; In DomainC ; Jacobian JVol; } } }
      { Name jz; Value { Local { [ CompZ[ -sigma[] * ( Dt[ {a} ] + {ur} )] ]; In DomainC ; Jacobian JVol; } } }
      { Name js; Value { Local { [ js[] ]; In DomainS0 ; Jacobian JVol; } } }
      { Name U ; Value { Term { [ {U} ]; In DomainC ; } } }
      { Name I ; Value { Term { [ {I} ]; In DomainC ; } } }
      { Name JouleLossesMap ; Value { Local { [ sigma[] * SquNorm[Dt[ {a} ] + {ur} ] ]; In DomainC ; Jacobian JVol ; } } }
      { Name JouleLosses    ; Value { Integral { [ sigma[] * SquNorm[Dt[{a}] + {ur} ] ]; In DomainC ; Jacobian JVol ; Integration I ; } } }
      { Name MagEnergy      ; Value { Integral { [ nu[ {d a} ] * {d a} * Dt[ {d a} ] ]; In Domain ; Jacobian JVol ; Integration I ; } } } 
    }
  }
}

ncuts = n_smc;
ndeltacuts_x = 3;
numPtsDiscret = 2000;
PostOperation {
  { Name maps ; NameOfPostProcessing MagStaDyn_a_ref ;
    Operation {
      Print[ az, OnElementsOf Domain,  File StrCat[Dir_Ref,StrCat["az",ExtGmsh] ] ];
      Print[ a,  OnElementsOf Domain,  File StrCat[Dir_Ref,StrCat["a" ,ExtGmsh] ] ];
      Print[ j,  OnElementsOf DomainC, File StrCat[Dir_Ref,StrCat["j" ,ExtGmsh] ] ];
      Print[ b,  OnElementsOf Domain,  File StrCat[Dir_Ref,StrCat["b" ,ExtGmsh] ] ];
      Print[ h,  OnElementsOf Domain , File StrCat[Dir_Ref,StrCat["h" ,ExtGmsh] ] ];
      Print[ JouleLossesMap,OnElementsOf DomainC , File StrCat[Dir_Ref,StrCat["JLMap",ExtGmsh] ] ];
    }
  }
  { Name globalquantities ; NameOfPostProcessing MagStaDyn_a_ref ;
    Operation {
      Print[ MagEnergy[Domain], OnGlobal, Format TimeTable, File StrCat[Dir_Ref, Sprintf("MagEnergy_nl%g_f%g.dat", Flag_NL, Freq) ] ] ;
      Print[ JouleLosses[Iron], OnGlobal, Format TimeTable, File StrCat[Dir_Ref, Sprintf("JL_nl%g_f%g.dat"       , Flag_NL, Freq) ] ] ;
    }
  }

  { Name cuts; NameOfPostProcessing MagStaDyn_a_ref ;    
    Operation {
      For iTS In {1:nTS}
      TS = listOfTS~{iTS};
      For i In {1:ncuts:ndeltacuts_x}
      Print[ az, OnLine{ {(i-0.5)*e , 0., 0.}{(i-0.5)*e , 500e-6, 0.} } {numPtsDiscret}, Format Table, File StrCat[Dir_Ref, StrCat[Sprintf["az_ref_cut%g_TS%g", i, TS], ExtData ] ], TimeStep{TS}];
      Print[ a, OnLine{ {(i-0.5)*e , 0., 0.}{(i-0.5)*e , 500e-6, 0.} } {numPtsDiscret}, Format Table, File StrCat[Dir_Ref, StrCat[Sprintf["a_ref_cut%g_TS%g", i, TS], ExtData ] ], TimeStep{TS}];
      Print[ b, OnLine{ {(i-0.5)*e , 0., 0.}{(i-0.5)*e , 500e-6, 0.} } {numPtsDiscret}, Format Table, File StrCat[Dir_Ref, StrCat[Sprintf["b_ref_cut%g_TS%g", i, TS], ExtData ] ], TimeStep{TS}];
      Print[ h, OnLine{ {(i-0.5)*e , 0., 0.}{(i-0.5)*e , 500e-6, 0.} } {numPtsDiscret}, Format Table, File StrCat[Dir_Ref, StrCat[Sprintf["h_ref_cut%g_TS%g", i, TS], ExtData ] ], TimeStep{TS}];
      EndFor
      EndFor
    }
  }
}
