

PostProcessing {

  // --------------------------------------------------------------------
  // Adjoint variable
  // --------------------------------------------------------------------
  { Name AdjointFormulation ; NameOfFormulation AdjointFormulation;
    PostQuantity {
      // Field quantities
      { Name lambdaZ ; Value { Term {[ CompZ[{lambda}] ] ; In Domain ; Jacobian Vol;}}}
      { Name lambda ; Value { Term {[ {lambda} ] ; In Domain ; Jacobian Vol;}}}

      { Name lambda_K_A; // Int(nu*rotA*rotLambda)->lambda*K*A
          Value {
      	     Integral { [ ( nu[{d a}] * {d a} ) * {d lambda} ];
	               In Domain ; Jacobian Vol  ; Integration I1; }   
          }
      }

      { Name lambda_g; // Sum_i{lambda*g_i}
       	Value {
           Integral { [( nu[{d a}] * br[] )*{d lambda}];
	              In DomainM ; Jacobian Vol  ; Integration I1; }      	
     	   Integral { [ js[] * {lambda}];
	              In DomainS ; Jacobian Vol  ; Integration I1; }      	
      	}
      }

      { Name Sensitivity_AdjointMethod; 
        Value { 
                 Integral { [(-1.0*nu_prime[{d a}]*{d a})*{d lambda}]; 
                 In Domain ; Jacobian Vol  ; Integration I1; }
        }
      }

      { Name v ; Value { Term { [ velocityField[] ] ; In Domain ; Jacobian Vol ; }}}

      { Name rho_d_bilin_NL ; 
        Value{Term{[d_bilin_lie_NL[ {d a}, {d lambda} ]]; In Domain;Jacobian Vol ;}}
      }

      { Name sensF ; 
          Value { 
              Integral{[dF_adjoint_lie[{d a}]]; // d{f}/d{tau}(phi)
                In Domain ; Jacobian Vol ; Integration I1;}
          }
      }

      { Name sensK ; 
          Value { 
              Integral{[ d_bilin_lie[{d a},{d lambda}] ];//d{a}/d{tau}(A,lambda)
                In Domain;Jacobian Vol ; Integration I1;}
          }
      }

      { Name sensK_NL ; 
          Value { 
              Integral{[ d_bilin_lie_NL[{d a},{d lambda}] ];
                In DomainNL; Jacobian Vol ; Integration I1;}
          }
      }

      { Name sensM ; 
          Value { 
              Integral{[ d_M_lie[ {d a}, {d lambda} ] ];//d{l}/d{tau}(A,lambda)
                In Domain ; Jacobian Vol ; Integration I1;}
          }
      }

      { Name sensJ ; 
          Value { 
              Integral{[ d_J_lie[ {lambda} ] ];//d{l}/d{tau}(A,lambda)
                In Domain ; Jacobian Vol ; Integration I1;}
        }
      }
      { Name AvmVarDomSens; 
        Value { 
            Integral { [ dF_adjoint_lie[ {d a} ] ];  // d{f}/d{tau}(phi)
              In DomainFunc ; Jacobian Vol ; Integration I1 ;}
            Integral { [ -d_bilin_lie[ {d a}, {d lambda} ]];//d{a}/d{tau}(phi,lambda)
              In Domain ; Jacobian Vol ; Integration I1 ; }
            Integral { [ -d_bilin_lie_NL[ {d a}, {d lambda} ] ];
              In DomainNL; Jacobian Vol ; Integration I1;}
            Integral { [ d_M_lie[ {d a}, {d lambda} ] ];//d{l}/d{tau}(phi,lambda)
              In DomainM ; Jacobian Vol ; Integration I1 ; }
            Integral { [ d_J_lie[ {lambda} ] ];//d{l}/d{tau}(A,lambda)
              In DomainS ; Jacobian Vol ; Integration I1;}
          } 
        }
      }
  }
  // --------------------------------------------------------------------
  // Direct Lie derivative
  // --------------------------------------------------------------------
  { Name SolveDirectSystem ; NameOfFormulation DirectFormulation;
    PostQuantity {    
        { Name v ; Value { Term { [ velocityField[] ] ; In Domain ; Jacobian Vol ; }}}
 
        { Name d_aZ ; Value { Term { [ CompZ[{d_a}] ] ; In Domain ; Jacobian Vol ; }}}

        { Name dF_direct_lie ; 
          Value { 
              Integral{[ dF_direct_lie[{d a}, {d d_a}] ]; // d{f}/d{tau}(phi)
                In DomainFunc; Jacobian Vol; Integration I1; }
          }
        }
    }
  }
  // --------------------------------------------------------------------
  // Filter sensitivity (fixed optimization domain)
  // --------------------------------------------------------------------
  { Name FilterSensitivity; NameOfFormulation FilterSensitivity;
    PostQuantity {
      // Field quantities
      { Name psi0_moy; 
        Value { 
           Integral { [ prod_x_dC[]/ElementVol[] ] ; 
             In DomainOpt ; Jacobian Vol ; Integration I1; }
        }
      }
      { Name psi0; Value { Term { [ prod_x_dC[] ] ; In DomainOpt ; Jacobian Vol ; }}}
      { Name psi; Value { Term { [ {psi} ] ; In DomainOpt ; Jacobian Vol ; }}}
      { Name perfFuncSensFiltDesVar;Value{Term{[{psi}];In DomainOpt;Jacobian Vol;}}}
      { Name perfFuncSensFilt; 
        Value { Term { [ {psi}/designVar[] ] ; In DomainOpt ; Jacobian Vol ; }}
      }
      { Name dV; Value { Term { [ designVar[] ] ; In DomainOpt ; Jacobian Vol ; }}}
    }
  }
}

PostOperation {

 { Name Get_PrimalSystem; NameOfPostProcessing PostOptim;
   Operation{
     Print[ rho_js, OnElementsOf DomainS,
	    File StrCat[ResDir, StrCat["rho_js",ExtGmsh]], LastTimeStepOnly] ;

     Print[ Flux[PhaseA], OnGlobal, Format TimeTable,
	 File > StrCat[ResDir, StrCat["Flux_a",ExtGnuplot]], LastTimeStepOnly, Store 11,
	 SendToServer StrCat[poF,"A"],  Color "Pink" ];
     Print[ Flux[PhaseB], OnGlobal, Format TimeTable,
 	 File > StrCat[ResDir, StrCat["Flux_b",ExtGnuplot]], LastTimeStepOnly, Store 22,
	 SendToServer StrCat[poF,"B"],  Color "Yellow" ];
     Print[ Flux[PhaseC], OnGlobal, Format TimeTable,
	 File > StrCat[ResDir, StrCat["Flux_c",ExtGnuplot]], LastTimeStepOnly, Store 33,
	 SendToServer StrCat[poF,"C"], Color "LightGreen"];

     Print[ I, OnRegion PhaseA_pos, Format Table,
	 File > StrCat[ResDir, StrCat["Ia",ExtGnuplot]], LastTimeStepOnly,
	 SendToServer StrCat[poI,"A"], Color "Pink" ];
     Print[ I, OnRegion PhaseB_pos, Format Table,
	 File > StrCat[ResDir, StrCat["Ib",ExtGnuplot]], LastTimeStepOnly,
	 SendToServer StrCat[poI,"B"], Color "Yellow" ];
     Print[ I, OnRegion PhaseC_pos, Format Table,
	 File > StrCat[ResDir, StrCat["Ic",ExtGnuplot]], LastTimeStepOnly,
	 SendToServer StrCat[poI,"C"], Color "LightGreen" ];

     Print[ RotorPosition_deg, OnRegion DomainDummy, Format Table, LastTimeStepOnly,
   	 File StrCat[ResDir, StrCat["rotorPosition",ExtGnuplot]],Color "LightYellow",
   	 SendToServer StrCat[po_min,"Rotor position [mech. deg]"]  ];

     // Volume and mass
     Print[ Volume[DomainOpt], OnGlobal, Format TimeTable, 
                 File StrCat[ResDir,"Volume",ExtGmsh], LastTimeStepOnly, 
                 SendToServer StrCat[po_min,"Volume"], Color "LightYellow"] ;

     Print[ Volume, OnElementsOf DomainOpt, 
                 File StrCat[ResDir,"ElementVolume",ExtGmsh], LastTimeStepOnly] ;

     Print[ surfacePM[Rotor_Magnets], OnGlobal, Format TimeTable, 
         File > StrCat[ResDir, StrCat["surf_PM",ExtGmsh]], LastTimeStepOnly, 
         SendToServer StrCat[po_min, "surfacePM"], Color "LightYellow" ];

     // optim quantitys
     Print[ BradError[DomainFunc], OnGlobal, Format Table, 
         File StrCat[ResDir, StrCat["BradialErrorInt",ExtGnuplot]], LastTimeStepOnly, 
         SendToServer StrCat[po_min,"BradialErrorInt"], Color "LightYellow" ];

     Print[ComplianceELM[DomainFunc], OnGlobal, Format Table,
	 File StrCat[ResDir, StrCat["ComplianceElm",ExtGnuplot]], LastTimeStepOnly,
	 SendToServer StrCat[po_min,"ComplianceElm"], Color "LightYellow" ];

     Print[ComplianceELM[DomainFunc], OnGlobal, Format Table,//change this!!!
	 File StrCat[ResDir, StrCat["ComplianceElast",ExtGnuplot]], LastTimeStepOnly,
	 SendToServer StrCat[po_min,"ComplianceElast"], Color "LightYellow" ];

     Print[Torque_simple[DomainFunc], OnGlobal, Format Table,
	 Store 58,File StrCat[ResDir, StrCat["Torque",ExtGnuplot]], LastTimeStepOnly,
	 SendToServer StrCat[po_min,"Torque"], Color "LightYellow" ];

     Print[Torque_var[DomainFunc], OnGlobal, Format Table,LastTimeStepOnly,
	 Store 60, File StrCat[ResDir, StrCat["TorqueVariance",ExtGnuplot]], 
	 SendToServer StrCat[po_min,"TorqueVariance"], Color "LightYellow" ];

     Print[Torque_var, OnElementsOf Domain,
	 File StrCat[ResDir, StrCat["TorqueVarianceAllDom",ExtGmsh]], LastTimeStepOnly];

     Print[Torque_var_square, OnRegion Rotor_Airgap, Format Table,LastTimeStepOnly,
	 Store 61, File StrCat[ResDir, StrCat["TorqueVarianceSquare",ExtGnuplot]], 
	 SendToServer StrCat[po_min,"TorqueVarianceSquare"], Color "LightYellow" ];

    Print[ Iron_Loss_Density,  OnElementsOf Stator_Fe,
	 File StrCat[ResDir, StrCat["LossDensity",ExtGmsh]], LastTimeStepOnly] ;

    Print[Iron_Losses[Stator_Fe], OnGlobal, Format Table,
	 File StrCat[ResDir, StrCat["IronLoss",ExtGnuplot]], LastTimeStepOnly,
	 SendToServer StrCat[po_min,"IronLoss"], Color "LightYellow" ];

    Print[ mur,  OnElementsOf DomainFe, 
           File StrCat[ResDir, StrCat["mur",ExtGmsh]], LastTimeStepOnly];

    If(!StrCmp(Flag_optType,"topology"))
      Print[ designVarPlot,  OnElementsOf DomainOpt, LastTimeStepOnly,
            File StrCat[ResDir, StrCat["designVariablePlot",ExtGmsh]]];
    EndIf

    Print[ boundary, OnElementsOf DomainDummy,  
         File StrCat[ResDir, StrCat["DomainBoundary",ExtGmsh]], LastTimeStepOnly] ;

    Print[ az, OnElementsOf Domain,
 	 File StrCat[ResDir, StrCat["aZ",ExtGmsh]], LastTimeStepOnly] ;

    Print[ a, OnElementsOf Domain,
	 File StrCat[ResDir, StrCat["a",ExtGmsh]], LastTimeStepOnly] ;

    Print[ br, OnElementsOf DomainM,
	 File StrCat[ResDir, StrCat["br",ExtGmsh]], LastTimeStepOnly] ;

    Print[ b, OnElementsOf Domain,
	 File StrCat[ResDir, StrCat["b",ExtGmsh]], LastTimeStepOnly] ;
 
    r_Bradial = Drotor/2.+Ef/10; 
    r_Bradial_end = Drotor/2.+Ef-Ef/10; 
    dr_Bradial = (r_Bradial_end - r_Bradial)/3.; 
    r = r_Bradial_end;
    theta_Bradial = 45 * deg;
    nb_Bradial = 501;

    Print[ b_radialFund_airgap, 
           OnGrid {$A*Cos[$B], $A*Sin[$B], $C} 
                  { r, 0:theta_Bradial:(theta_Bradial)/nb_Bradial, 0.},    
           File StrCat[ResDir, StrCat["b_radialFund_airgap",ExtGnuplot]],
           Format Gnuplot ];

    Print[ b_radialFund_airgap,
           OnGrid {$A*Cos[$B], $A*Sin[$B], $C}  
                  { r, 0:theta_Bradial:(theta_Bradial)/nb_Bradial, 0.}, 
           File StrCat[ResDir, StrCat["b_radialFund_airgap",ExtGmsh]],
           LastTimeStepOnly, Format Gmsh];

    Print[ b_radial_airgap, 
           OnGrid {$A*Cos[$B], $A*Sin[$B], $C} 
                  { r, 0:theta_Bradial:(theta_Bradial)/nb_Bradial, 0.},    
           File StrCat[ResDir, StrCat["b_radial_airgap",ExtGnuplot]], Format Gnuplot] ;

    Print[ b_radial_airgap, 
           OnGrid {$A*Cos[$B], $A*Sin[$B], $C} 
                  { r, 0:theta_Bradial:(theta_Bradial)/nb_Bradial, 0.}, 
         File StrCat[ResDir, StrCat["b_radial_airgap",ExtGmsh]],
         LastTimeStepOnly, Format Gmsh];
   }
 }

 { Name SolveAdjointSystem; NameOfPostProcessing AdjointFormulation;
   Operation{
     Print[ lambdaZ, OnElementsOf Domain,
     	 File StrCat[ResDir, StrCat["lambdaZ",ExtGmsh]], LastTimeStepOnly] ;
   }
 }

 { Name GetSemiAdjointSens; NameOfPostProcessing AdjointFormulation;
   Operation{
     Print[ lambda_K_A[Domain], OnGlobal, Format Table,
  	   File StrCat[ResDir, StrCat["lambda_K_A",ExtGnuplot]], LastTimeStepOnly,
	   SendToServer StrCat[po_min,"lambda_K_A"], Color "LightYellow" ];
     Print[ lambda_g[Domain],OnGlobal, Format Table,
	   File StrCat[ResDir, StrCat["lambda_g",ExtGnuplot]], LastTimeStepOnly,
	   SendToServer StrCat[po_min,"lambda_g"], Color "LightYellow" ];
   }
 }
  // --------------------------------------------------------------------
  // Sensitivity (adjoint variable) with Lie approach 
  // --------------------------------------------------------------------
  { Name GetTopOptAdjointSens; NameOfPostProcessing AdjointFormulation;
    Operation{
      Print[ Sensitivity_AdjointMethod, OnElementsOf DomainOpt,LastTimeStepOnly,
	   File StrCat[ResDir, StrCat["SensPerfAvmFixedDom",ExtGmsh]]] ;
    }
  }
  { Name GetShapeOptAdjointSens; NameOfPostProcessing AdjointFormulation;
    Operation{
//       Print[ v, OnElementsOf Domain,
//	      File StrCat[ResDir, StrCat["velocity",ExtGmsh]], LastTimeStepOnly] ;
////       Print[ rho_d_bilin_NL, OnElementsOf DomainNL,
////	      File StrCat[ResDir, StrCat["rho_d_bilin_NL",ExtGmsh]], LastTimeStepOnly] ;
//       Print[ sensF[DomainFunc], OnGlobal, Format Table, 
//              File StrCat[ResDir, StrCat["d_func", ExtGnuplot]], 
//	      SendToServer StrCat[po_min,"d_func"], LastTimeStepOnly];
//       Print[ sensK[Domain], OnGlobal, Format Table,
//              File StrCat[ResDir, StrCat["d_bilin", ExtGnuplot]],  
//              SendToServer StrCat[po_min,"d_bilin"], LastTimeStepOnly];
//       Print[ sensK_NL[DomainNL], OnGlobal, Format Table,
//              File StrCat[ResDir, StrCat["d_bilin_NL", ExtGnuplot]],  
//              SendToServer StrCat[po_min,"d_bilin_NL"], LastTimeStepOnly];
//       Print[ sensM[DomainM], OnGlobal, Format Table,
//              File StrCat[ResDir, StrCat["d_M", ExtGnuplot]], 
//              SendToServer StrCat[po_min,"d_M"], LastTimeStepOnly];
//       Print[ sensJ[DomainS], OnGlobal, Format Table,
//              File StrCat[ResDir, StrCat["d_J", ExtGnuplot]], 
//              SendToServer StrCat[po_min,"d_J"], LastTimeStepOnly];
       Print[ AvmVarDomSens[Domain], OnGlobal, Format Table,
              File StrCat[ResDir, StrCat["AvmVarDomSens", ExtGnuplot]], 
              SendToServer StrCat[po_min,"AvmVarDomSens"], LastTimeStepOnly];
    }
  } 
  // --------------------------------------------------------------------
  // Sensitivity (direct method) with Lie approach 
  // --------------------------------------------------------------------
  { Name SolveDirectSystem; NameOfPostProcessing SolveDirectSystem;
    Operation{
       Print[ d_aZ, OnElementsOf Domain,
	      File StrCat[ResDir, StrCat["d_aZ",ExtGmsh]], LastTimeStepOnly] ;
    }
  } 
  { Name GetShapeOptDirectSens; NameOfPostProcessing SolveDirectSystem;
    Operation{
       Print[ dF_direct_lie[DomainFunc], OnGlobal, Format Table,
              File StrCat[ResDir, StrCat["sensDirectLie", ExtGnuplot]], 
              SendToServer StrCat[po_min,"sensDirectLie"], LastTimeStepOnly];
    }
  } 
  // --------------------------------------------------------------------
  // Filter sensitivity 
  // --------------------------------------------------------------------
  { Name FilterSens; NameOfPostProcessing FilterSensitivity;
    Operation {
	  Print [ perfFuncSensFiltDesVar, OnElementsOf DomainOpt, 
                  File StrCat[ResDir,"Sensitivity_DesVar_Filtered",".pos"] ];
    }
  }

}
