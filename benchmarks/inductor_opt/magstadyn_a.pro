Group {
   DefineGroup[
     DomainM, DomainB, DomainS, DomainInf,
     DomainL, DomainNL,
     Surf_bn0, Surf_Inf
  ];
}

DefineConstant[
  //-------------------------------------------------------------
  // optimization stuff ...
  //-------------------------------------------------------------
  ResId = "",
  ResDir = StrCat["res/", ResId],

  Flag_SolveStateVar = {0, Name "Input/OptParam/SolveStateVar",
                           Label "Get State Variable", Choices {0,1}, Visible 1},

  Flag_SolveAdjointVar = {0, Name "Input/OptParam/SolveAdjointVar",
                             Label "Get Adjoint Variable",Choices {0,1}, Visible 1},

  Flag_PerfType = {COMPLIANCE, Name "Input/OptParam/PerfType",
                               Label "performance function type",
                               Choices {NO_PERF="No performance function",
                                        BFIELD_ERROR="air gap B field error",
                                        TORQUE_VAR="torque variance",
				        IRON_LOSSES="losses",
                                        COMPLIANCE="compliance",
                                        TORQUE="torque"},Visible Flag_SolveAdjointVar},
  // Sensitivity analysis method --> replace by 1 variable!!!
  Flag_SemiAnalyticAvmSens = {0, Name "Input/OptParam/SemiAnalyticAvmSensQuantitys",
                                 Label "Semi analytic quantitys (avm)",
                                 Choices {0,1}, Visible 0},

  Flag_SemiAnalyticDirSens = {0, Name "Input/OptParam/SemiAnalyticDirQuantitys",
                                 Label "Semi analytic quantitys (direct)", 
                                 Choices {0,1}, Visible 0},

  Flag_AvmFixedDomSens = {0, Name "Input/OptParam/AdjointMethodSensFixedDom",
                             Label "fixed domain derivative (avm)", 
                             Choices {0,1}, Visible 1},

  Flag_DirFixedDomSens = {0, Name "Input/OptParam/DirMethodSensFixedDom",
                            Label "fixed domain derivative (direct)", 
                            Choices {0,1}, Visible 0},

  Flag_AvmVarDomSens = {0, Name "Input/OptParam/AdjointMethodSensVarDom",
                           Label "continuum derivative (avm)",
                           Choices {0,1}, Visible 0},

  Flag_DirVarDomSens = {0, Name "Input/OptParam/DirectMethodSensVarDom",
                           Label "continuum derivative (direct)",
                           Choices {0,1}, Visible 0},
  
  NbSubDom = {2, Name "Input/OptParam/NbDomain", Label "Nbr of regions", Visible 0},

  //Tnom = {90, Name "Input/OptParam/TorqueNominal", Visible 1},
  Tnom = {90.0, Name "Input/OptParam/TorqueNominal",
                Label "Nominal desired torque",
                Visible (Flag_PerfType==TORQUE_VAR)},
  // Filter
  Flag_filterSensitivity = {0, Name "Input/OptParam/filterSens", 
                               Label "Filter Derivatives?",
                               Choices {0, 1}, Visible (Flag_topopt==1)}, 

  Rmin = {0.001*10, Name "Input/OptParam/RadiusSensFilter",
                    Label "Sensitivity Filter Radius", 
                    Visible (Flag_filterSensitivity==1)},

  Flag_filterMeshCoordinates = {0, Name "Input/OptParam/filterMeshCoord", 
                                   Label "Filter mesh nodes coordinates?", 
                                   Choices {0, 1}, Visible 0}, 

  regionVar = {0, Name "Input/OptParam/regionVar",
                     Label "Region of design variables", 
                     Choices {0="E-core",1="E+I-core"},
                     Visible (Flag_topopt)},

  Flag_InterpLaw = {0, Name "Input/OptParam/MaterialInterpLaw",
                       Label "material interpolation law",
                       Choices {0="SIMP",1="RAMP"},Visible (Flag_topopt==1)},

  degree_SIMP = {3.0, Name "Input/OptParam/SimpPenalDegree",
                      Label "Degree SIMP", Visible (Flag_topopt==1)}
  //-------------------------------------------------------------

];

Function{
 DefineConstant[
    Val_Rint, Val_Rext,
    Lz = 1,
    SymmetryFactor = 1,
    Nb_max_iter = 30,
    relaxation_factor = 1,
    stop_criterion = 1e-5,
    reltol = 1e-7,
    abstol = 1e-5,
    T = 1/Freq, // Fundamental period in s
    time0 = 0,
    NbT = 1,
    timemax = NbT*T,
    NbSteps = 100,
    delta_time = T/NbSteps,
    FillFactor_Winding = {1, Name "Input/4Coil Parameters/3Fill factor",
      Highlight "AliceBlue", Visible 1},
    Factor_R_3DEffects = {1, Name "Input/4Coil Parameters/9fact", Label "3D factor",
      Highlight "AliceBlue", Visible 1},
    // Increasing the resistance by 50% == 1.5
    II, VV,
    Flag_NL = 0,
    Flag_NL_Newton_Raphson = {1, Choices{0,1}, Name "Input/41Newton-Raphson iteration",
      Visible Flag_NL},
    po = "Output/"
  ];

  DefineFunction[
    dhdb_NL, br, js0,
    rmin2,prod_x_dC,designVar,nu_prime
  ];

}

Include "BH.pro"; // nonlinear BH caracteristic of magnetic material

Group {
  DomainB   = Region[ {Inds} ];

  If(Flag_Infinity)
    DomainInf = Region[ {AirInf} ];
  EndIf

  DomainCC = Region[ {Air/*, AirInf*/, Inds, Core} ];
  DomainC  = Region[ { } ];
  Domain  = Region[ {DomainCC, DomainC} ];

  If(Flag_NL)
    DomainNL = Region[ {Core} ];
    DomainL  = Region[ {Domain,-DomainNL} ];
  EndIf
  DomainDummy = #12345 ; // Dummy region number for postpro with functions
  // ----------------------------------------------------------
  DomCompl = Region[{AIRGAP}];
  If(Flag_topopt)
    If(regionVar == 0)
      DomainOpt = Region[{ECORE}];
    EndIf
    If(regionVar == 1)
      DomainOpt = Region[{ECORE,ICORE}];
    EndIf
  EndIf
  If(!Flag_topopt)
    DomainOpt = Region[{}];
  EndIf
  // ----------------------------------------------------------
}

Function {
 
  nu0 = 1./mu0;
  nu [#{DomainCC,-Core}]  = 1./mu0 ;
  //nu [#{Air/*, AirInf*/, Inds}]  = 1./mu0 ;

  If(!Flag_topopt)
    If(!Flag_NL)
      nu [#{Core}]  = 1/(mur_fe*mu0) ;
    EndIf
    If(Flag_NL)
      nu [ DomainNL ] = nu_EIcore[$1] ;
      dhdb_NL [ DomainNL ] = dhdb_EIcore_NL[$1];
    EndIf
  EndIf
  If(Flag_topopt)
    p = degree_SIMP;
    designVar[#DomainOpt]  = ScalarField[XYZ[],0,1]{DES_VAR_FIELD};
    If(!Flag_NL)
      nu [#{Core,-DomainOpt}]  = 1/(mur_fe*mu0) ;
      nu[#DomainOpt]  = (1 / (mur_fe * mu0) - nu0)*designVar[]^p + nu0; 
      nu_prime[#DomainOpt]  = p*(1 / (mur_fe * mu0) - nu0)*designVar[]^(p-1.0); 
    EndIf
    If(Flag_NL)
      If(Flag_NL_Curve == 0) //EIcore
        nu[#{Core,-DomainOpt}] = nu_EIcore[$1] ;
        nu[DomainOpt] = nu0*(1.0 + (nu_EIcore[$1]/nu0 - 1.0)*designVar[]^p);
        dhdb_NL[DomainOpt] = dhdb_EIcore_NL[$1]*designVar[]^p;
        dhdb_NL[#{Core,-DomainOpt}] = dhdb_EIcore_NL[$1];
        nu_prime[DomainOpt] = p*nu0*(nu_EIcore[$1]/nu0 - 1.0)*designVar[]^(p-1.0);
      EndIf
      If(Flag_NL_Curve == 1) //Park(article topology optimization)
        nu[#{Core,-DomainOpt}] = nu_Park[$1]; //nu_EIcore[$1] ;
        nu[DomainOpt] = nu0*(1.0 + (nu_Park[$1]/nu0 - 1.0)*designVar[]^p);
        dhdb_NL[DomainOpt] = dhdb_Park_NL[$1]*designVar[]^p;
        dhdb_NL[#{Core,-DomainOpt}] = dhdb_Park_NL[$1];
        nu_prime[DomainOpt] = p*nu0*(nu_Park[$1]/nu0 - 1.0)*designVar[]^(p-1.0);
      EndIf
    EndIf
  EndIf

  sigma[#{Inds}] = sigma_coil ;
  rho[] = 1/sigma[] ;

  Rb[] = Factor_R_3DEffects*FillFactor_Winding*Lz*NbWires[]^2/SurfCoil[]/sigma[] ;
  Resistance[#{Inds}] = Rb[] ;
  // ----------------------------------------------------------
  // optimization
  // ----------------------------------------------------------
  If(Flag_filterSensitivity)
    rmin2[] = Rmin*Rmin;
    prod_x_dC[] = ScalarField[XYZ[],0,1]{SENS_FIELD};
  EndIf
  volDensity[#{DomainOpt}] = 7874; //acier
  //volDensity[#{Rotor_Fe,Stator_Fe}] = 7874; //acier
  //volDensity[#{DomainM}] = 7400; //PM

  T_max[] = ( SquDyadicProduct[$1] - SquNorm[$1] * TensorDiag[0.5, 0.5, 0.5] ) / mu0 ;
}

//-------------------------------------------------------------------------------------

Jacobian {
  { Name Vol;
    Case {
      { Region DomainInf ; Jacobian VolSphShell{Val_Rint, Val_Rext} ; }
      { Region All ; Jacobian Vol; }
    }
  }
}

Integration {
  { Name I1 ; Case {
      { Type Gauss ;
        Case {
          { GeoElement Triangle   ; NumberOfPoints  6 ; }
	  { GeoElement Quadrangle ; NumberOfPoints  4 ; }
	  { GeoElement Line       ; NumberOfPoints  13 ; }
        }
      }
    }
  }
}

//-------------------------------------------------------------------------------------

Constraint {

  { Name MVP_2D ;
    Case {
      { Region Surf_Inf ; Type Assign ; Value 0. ; }
      //{ Region Surf_bn0 ; Type Assign ; Value 0. ; }
    }
  }

  { Name Current_2D ;
    Case {
      { Region Inds ; Value II*Idir[] ; TimeFunction IA[]; }
    }
  }

  { Name Voltage_2D ;
    Case {
    }
  }

}

//-----------------------------------------------------------------------------------------------

FunctionSpace {
  // Magnetic Vector Potential
  { Name Hcurl_a_2D ; Type Form1P ;
    BasisFunction {
      { Name se1 ; NameOfCoef ae1 ; Function BF_PerpendicularEdge ;
        Support Region[{Domain}] ; Entity NodesOf [ All ] ; }
   }
    Constraint {
      { NameOfCoef ae1 ; EntityType NodesOf ; NameOfConstraint MVP_2D ; }
    }
  }
  //-----------------------------------------------------------------
  // Gradient of Electric scalar potential (2D)
  { Name Hregion_u_Mag_2D ; Type Form1P ;
    BasisFunction {
      { Name sr ; NameOfCoef ur ; Function BF_RegionZ ;
        Support DomainC ; Entity DomainC ; }
    }
    GlobalQuantity {
      { Name U ; Type AliasOf        ; NameOfCoef ur ; }
      { Name I ; Type AssociatedWith ; NameOfCoef ur ; }
    }
    Constraint {
      { NameOfCoef U ; EntityType GroupsOfNodesOf ; NameOfConstraint Voltage_2D ; }
      { NameOfCoef I ; EntityType GroupsOfNodesOf ; NameOfConstraint Current_2D ; }
    }
  }
  //-----------------------------------------------------------------
  { Name Hregion_i_Mag_2D ; Type Vector ;
    BasisFunction {
      { Name sr ; NameOfCoef ir ; Function BF_RegionZ ;
        Support DomainB ; Entity DomainB ; }
    }
    GlobalQuantity {
      { Name Ib ; Type AliasOf        ; NameOfCoef ir ; }
      { Name Ub ; Type AssociatedWith ; NameOfCoef ir ; }
    }
    Constraint {
      { NameOfCoef Ub ; EntityType Region ; NameOfConstraint Voltage_2D ; }
      { NameOfCoef Ib ; EntityType Region ; NameOfConstraint Current_2D ; }
    }
  }
  //-----------------------------------------------------------------
  //  Adjoint var --> same constraints as A
  { Name Hcurl_lambda_2D ; Type Form1P ;
    BasisFunction {
      { Name se1 ; NameOfCoef ae1 ; Function BF_PerpendicularEdge ;
        Support Region[{Domain}] ; Entity NodesOf [ All ] ; }
   }
    Constraint {
      { NameOfCoef ae1 ; EntityType NodesOf ; NameOfConstraint MVP_2D ; }
    }
  }
  //-----------------------------------------------------------------
  // Filtered sensitivity
  { Name H_psi ; Type Form0 ;
    BasisFunction {
      { Name sPsi ; NameOfCoef psi ; Function BF_Node ;
        Support Domain ; Entity NodesOf[ All ] ; }
    }
  }
}
//----------------------------------------------------------------------------------------

Formulation {

  { Name MagStaDyn_a_2D ; Type FemEquation ;
    Quantity {
      { Name a  ; Type Local  ; NameOfSpace Hcurl_a_2D ; }
      { Name ur ; Type Local  ; NameOfSpace Hregion_u_Mag_2D ; }
      { Name I  ; Type Global ; NameOfSpace Hregion_u_Mag_2D [I] ; }
      { Name U  ; Type Global ; NameOfSpace Hregion_u_Mag_2D [U] ; }

      { Name ir ; Type Local  ; NameOfSpace Hregion_i_Mag_2D ; }
      { Name Ub ; Type Global ; NameOfSpace Hregion_i_Mag_2D [Ub] ; }
      { Name Ib ; Type Global ; NameOfSpace Hregion_i_Mag_2D [Ib] ; }
    }

    Equation {
      Galerkin { [ nu[{d a}] * Dof{d a}  , {d a} ] ;
        In Domain ; Jacobian Vol ; Integration I1 ; }
      If(Flag_NL_Newton_Raphson)
      Galerkin { JacNL [ dhdb_NL[{d a}] * Dof{d a} , {d a} ] ;
        In DomainNL ; Jacobian Vol ; Integration I1 ; }
      EndIf
      Galerkin { [ -nu[] * br[] , {d a} ] ;
        In DomainM ; Jacobian Vol ; Integration I1 ; }

      Galerkin { DtDof[ sigma[] * Dof{a} , {a} ] ;
        In DomainC ; Jacobian Vol ; Integration I1 ; }
      Galerkin { [ sigma[] * Dof{ur}, {a} ] ;
        In DomainC ; Jacobian Vol ; Integration I1 ; }

      Galerkin { [ -js0[] , {a} ] ;
        In DomainS ; Jacobian Vol ; Integration I1 ; }

      Galerkin { DtDof[ sigma[] * Dof{a} , {ur} ] ;
        In DomainC ; Jacobian Vol ; Integration I1 ; }
      Galerkin { [ sigma[] * Dof{ur} , {ur} ] ;
        In DomainC ; Jacobian Vol ; Integration I1 ; }
      GlobalTerm { [ Dof{I} , {U} ] ; In DomainC ; }

      Galerkin { [ -NbWires[]/SurfCoil[] * Dof{ir} , {a} ] ;
        In DomainB ; Jacobian Vol ; Integration I1 ; }
      Galerkin { DtDof [ Lz * NbWires[]/SurfCoil[] * Dof{a} , {ir} ] ;
        In DomainB ; Jacobian Vol ; Integration I1 ; }
      GlobalTerm { [ Dof{Ub}/SymmetryFactor , {Ib} ] ; In DomainB ; }
      Galerkin { [ Rb[]/SurfCoil[]* Dof{ir} , {ir} ] ;
        In DomainB ; Jacobian Vol ; Integration I1 ; } // Resistance term

      // GlobalTerm { [ Resistance[]  * Dof{Ib} , {Ib} ] ; In DomainB ; }
      // The above term can replace the resistance term:
      // if we have an estimation of the resistance of DomainB, via e.g. measurements
      // which is better to account for the end windings...

    }
  }
  //-----------------------------------------------------------------
  // Adjoint Formulation
  //-----------------------------------------------------------------
 { Name AdjointFormulation ; Type FemEquation ;
    Quantity {
      { Name a  ; Type Local  ; NameOfSpace Hcurl_a_2D; }
      { Name lambda ; Type Local  ; NameOfSpace Hcurl_lambda_2D; }
    }
    Equation {
      // bilinear(Lambda,Lambda')
      Galerkin { [ nu[ {d a} ]* Dof{d lambda}  , {d lambda} ] ;
        In Domain ; Jacobian Vol ; Integration I1 ; }
      Galerkin { [ dhdb_NL[ {d a} ] * Dof{d lambda} , {d lambda} ] ; 
        In DomainNL ; Jacobian Vol ; Integration I1 ; }

      // Adjoint load
      If(Flag_PerfType == COMPLIANCE) // F = Int_DO{nu*curl(A)^2}
        Printf["-- Compliance Pseudoload --"];
        Galerkin { [ -2.0*nu[{d a}]*{d a}, {d lambda} ] ;
         In DomCompl ; Jacobian Vol ; Integration I1 ; }
      EndIf
   }
  }
  //-----------------------------------------------------------------
  // Sensitivity filtering (for topology optimization)
  //-----------------------------------------------------------------
 { Name Filt_sens ; Type FemEquation ;
    Quantity {
       { Name psi ; Type Local ; NameOfSpace H_psi;}
    }
    Equation {
       Galerkin { [ rmin2[] * Dof{d psi}, {d psi} ] ;
                   In DomainOpt; Jacobian Vol ; Integration I1 ; }

       Galerkin { [ Dof{psi}, {psi} ] ;
                   In DomainOpt; Jacobian Vol; Integration I1; }

       Galerkin { [ -prod_x_dC[], {psi} ] ;
                   In DomainOpt; Jacobian Vol; Integration I1; }
      }
 }
}
//--------------------------------------------------------------------------------------
Resolution {

  { Name Analysis ;
    System {
      If(Flag_AnalysisType==2)
        { Name A;NameOfFormulation MagStaDyn_a_2D;Type ComplexValue;Frequency Freq; }
      EndIf
      If(Flag_AnalysisType<2)
        { Name A ; NameOfFormulation MagStaDyn_a_2D ; }
      EndIf
    }
    Operation {
      CreateDir["res/"];

      InitSolution[A] ;

      PostOperation[Get_Analytical] ; // Values from magnetic circuit

      If(Flag_AnalysisType==0 || Flag_AnalysisType==2)
        If(!Flag_NL)
          Generate[A] ; Solve[A] ;
        EndIf
        If(Flag_NL)
          IterativeLoop[Nb_max_iter, stop_criterion, relaxation_factor]{
            GenerateJac[A] ; SolveJac[A] ; }
        EndIf
        SaveSolution[A] ;

        PostOperation[Get_LocalFields] ;
        PostOperation[Get_GlobalQuantities] ;
      EndIf // End Flag_AnalysisType==0 (Static) Flag_AnalysisType==2 (Frequency)

      If(Flag_AnalysisType==1)
        TimeLoopTheta[time0, timemax, delta_time, 1.]{ // Euler implicit (1) -- Crank-Nicolson (0.5)
          If(!Flag_NL)
            Generate[A]; Solve[A];
          EndIf
          If(Flag_NL)
            IterativeLoop[Nb_max_iter, stop_criterion, relaxation_factor] {
              GenerateJac[A] ; SolveJac[A] ; }
          EndIf
          SaveSolution[A];

          PostOperation[Get_LocalFields] ;
          Test[ $TimeStep > 1 ]{
            PostOperation[Get_GlobalQuantities];
          }
        }
      EndIf // End Flag_AnalysisType==1 (Time domain)
    }
  }
  //===================================================================
  // Optimization 
  //===================================================================
  { Name OptimStep ;
    System {
        { Name A ; NameOfFormulation MagStaDyn_a_2D ; } //get state variable
        { Name B ; NameOfFormulation AdjointFormulation ; } //get adjoint variable
        { Name D ; NameOfFormulation Filt_sens ; }
    }
    Operation {
     CreateDir[ResDir];
     //-------------------------------------------------------------------
     If(Flag_SolveStateVar) //Solve for A ?
       Printf["-- Compute State Variable --"];
       If(Flag_AnalysisType==0) // --- stationnary ---
         If(Flag_topopt)
           GmshRead["res/designVariable.pos",DES_VAR_FIELD]; 
         EndIf
         InitSolution[A];
         PostOperation[Get_Analytical2] ; // Values from magnetic circuit
         If(!Flag_NL)
	    Printf["-- Linear system --"];
            Generate[A]; Solve[A];
          EndIf
         If(Flag_NL)
           Printf["-- NL system --"];
           IterativeLoop[Nb_max_iter, stop_criterion, relaxation_factor]{
             GenerateJac[A] ; SolveJac[A] ; }
         EndIf
         SaveSolution[A];
         PostOperation[Get_PrimalSystem];
      EndIf
     EndIf
     //-------------------------------------------------------------------
     If(Flag_SolveAdjointVar) 
       Printf["-- Compute Adjoint Variable --"];
       // Compute adjoint variable (independent of design variables) 
       // --> choose this method if #PerfFunc << #var 
       // --> Same system for shape and topology optimization
       // --> 1 system per performance function
       ReadSolution[A]; //Load state variable (potential vector)
       If(Flag_topopt)
         GmshRead["res/designVariable.pos",DES_VAR_FIELD]; 
       EndIf
       InitSolution[B]; Generate[B]; Solve[B]; 
       SaveSolution[A]; SaveSolution[B]; 
       PostOperation[Get_AdjointSystem];
     EndIf
     //-------------------------------------------------------------------
     If(Flag_AvmFixedDomSens) // adjoint method sens. - fixed mesh
       Printf["-- Compute AVM sensitivity analysis (fixed domain) --"];
       ReadSolution[A];ReadSolution[B];//A and Lambda   
       GmshRead["res/designVariable.pos",DES_VAR_FIELD];
       PostOperation[Get_AvmFixedDomSens];
     EndIf
     //-------------------------------------------------------------------
     If(Flag_filterSensitivity) // Filter sensitivity (only if TO)
       Printf["-- Filter Sensitivity --"];
       GmshRead["res/Sensitivity_DesVar.pos", SENS_FIELD]; 
       Generate[D]; Solve[D]; SaveSolution[D];
       PostOperation[Get_FilteredSens];
     EndIf
    }
  }
}
//-----------------------------------------------------------------------------------------------

PostProcessing {

  { Name MagStaDyn_a_2D ; NameOfFormulation MagStaDyn_a_2D ;
    PostQuantity {
      { Name a  ; Value { Term { [ {a} ] ; In Domain ; Jacobian Vol ; } } }
      { Name az ; Value { Term { [ CompZ[{a}] ] ; In Domain ; Jacobian Vol ; } } }

      { Name b  ; Value { Term { [ {d a} ] ; In Domain ; Jacobian Vol ; } } }
      { Name nb  ; Value { Term { [ Norm[{d a}] ] ; In Domain ; Jacobian Vol ; } } }
      { Name br ; Value { Term { [ br[] ] ; In DomainM ; Jacobian Vol ; } } }

      { Name h ; Value { Term { [ nu[{d a}] * {d a} ] ; In Domain ; Jacobian Vol ; } } }

      { Name js0 ; Value { Term { [ js0[] ] ; In DomainS ; Jacobian Vol ; } } }

      { Name j  ; Value {
          Term { [ -sigma[]*(Dt[{a}]+{ur}) ]; In DomainC ; Jacobian Vol ; }
        }
      }

      { Name ir ; Value { Term { [ {ir} ] ; In Inds ; Jacobian Vol ; } } }

      { Name jz ; Value {
          Term { [ CompZ[-sigma[]*(Dt[{a}]+{ur})] ]       ; In DomainC ; Jacobian Vol ; }
        }
      }

      { Name rhoj2 ;
        Value {
          Term { [ sigma[]*SquNorm[ Dt[{a}]+{ur}] ] ; In Region[{DomainC}] ; Jacobian Vol ; }
          Term { [ 1./sigma[]*SquNorm[ IA[]*{ir} ] ] ; In Inds  ; Jacobian Vol ; }
        }
      }

      { Name JouleLosses ;
        Value {
          Integral { [ SymmetryFactor*Lz*sigma[] * SquNorm[ Dt[{a}]+{ur} ] ];
            In Region[{DomainC}] ; Jacobian Vol ; Integration I1 ; }
          Integral { [ SymmetryFactor*Lz*1./sigma[]*SquNorm[ IA[]*{ir} ] ];
            In Inds  ; Jacobian Vol ; Integration I1 ; }
        }
      }

      { Name MagEnergy ; Value {
          Integral { [ SymmetryFactor*Lz* 1/2 *nu[{d a}]*{d a}*{d a} ] ;
            In Domain ; Jacobian Vol ; Integration I1 ; } } }

      { Name Flux ; Value {
          Integral { [ SymmetryFactor*Lz*Idir[]*NbWires[]/SurfCoil[]* CompZ[{a}] ] ;
            In Inds  ; Jacobian Vol ; Integration I1 ; }
        }
      }

      { Name ComplexPower ; // S = P + i*Q
        Value {
          Integral { [ Complex[ sigma[]*SquNorm[Dt[{a}]+{ur}], nu[]*SquNorm[{d a}] ] ] ;
            In Region[{DomainC}] ; Jacobian Vol ; Integration I1 ; }
        }
      }

      { Name U ; Value {
          Term { [ {U} ]   ; In DomainC ; }
          Term { [ {Ub} ]  ; In DomainB ; }
        }
      }

      { Name I ; Value {
          Term { [ {I} ]   ; In DomainC ; }
          Term { [ {Ib} ]  ; In DomainB ; }
        }
      }

      { Name S ; Value {
          Term { [ {U}*Conj[{I}] ]    ; In DomainC ; }
          Term { [ {Ub}*Conj[{Ib}] ]  ; In DomainB ; }
        }
      }

      // Getting the value of some functions
     For k In {0:NbAvailableMagCircuits-1}
       { Name Reluctance~{k} ; Value { Term { Type Global; [ Reluctance~{k}[] ] ; In DomainDummy ; } } }
       { Name Inductance~{k} ; Value { Term { Type Global; [ Inductance~{k}[] ] ; In DomainDummy ; } } }
     EndFor

      { Name Inductance_from_Flux ; Value { Term { Type Global; [ #11*1e3/II ] ; In DomainDummy ; } } } // Flux stored in register #11
      { Name Inductance_from_MagEnergy ; Value { Term { Type Global; [ 2*#22*1e3/(II*II) ] ; In DomainDummy ; } } } // MagEnergy stored in register #22

    }//PostQuantity
  }// MagStaDyn_a_2D
}// PostProcessing


//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------

PostOperation Get_LocalFields UsingPost MagStaDyn_a_2D {
  Print[ ir, OnElementsOf Inds,   File StrCat[Dir,"ir",ExtGmsh], LastTimeStepOnly ] ;
  Print[ b,  OnElementsOf Domain, File StrCat[Dir,"b",ExtGmsh], LastTimeStepOnly ] ;
  Print[ nb,  OnElementsOf Domain, File StrCat[Dir,"nb",ExtGmsh], LastTimeStepOnly ] ;
  Print[ az, OnElementsOf Domain, File StrCat[Dir,"a",ExtGmsh], LastTimeStepOnly ];
}

PostOperation Get_Analytical UsingPost MagStaDyn_a_2D {
  For k In {0:NbAvailableMagCircuits-1}
    Print[ Reluctance~{k}, OnRegion DomainDummy, Format Table, LastTimeStepOnly,
      File StrCat[Dir,Sprintf("Reluctance%g",k),ExtGnuplot] ];
    Print[ Inductance~{k}, OnRegion DomainDummy, Format Table, LastTimeStepOnly,
      File StrCat[Dir, Sprintf("Inductance%g",k),ExtGnuplot],
      SendToServer StrCat[po,Sprintf("6%gInductance Magnetic Circuit %g [mH]", k, k)], Color "LightYellow" ];
  EndFor
}

PostOperation Get_GlobalQuantities UsingPost MagStaDyn_a_2D {
  Print[ I, OnRegion Ind_1, Format Table,
    File > StrCat[Dir,"I",ExtGnuplot], LastTimeStepOnly,
    SendToServer StrCat[po,"20I [A]"], Color "LightYellow" ];
  Print[ U, OnRegion Ind_1, Format Table,
    File > StrCat[Dir,"U",ExtGnuplot], LastTimeStepOnly,
    SendToServer StrCat[po,"30U [V]"], Color "LightYellow" ];

  Print[ Flux[Inds], OnGlobal, Format TimeTable,
    File > StrCat[Dir,"Flux",ExtGnuplot], LastTimeStepOnly, Store 11,
    SendToServer StrCat[po,"40Flux [Wb]"],  Color "LightYellow" ];

  Print[ MagEnergy[Domain], OnGlobal, Format TimeTable,
    File > StrCat[Dir,"ME",ExtGnuplot], LastTimeStepOnly, Store 22,
    SendToServer StrCat[po,"41Magnetic Energy [W]"],  Color "LightYellow" ];

  Print[ Inductance_from_Flux, OnRegion DomainDummy, Format Table, LastTimeStepOnly,
    File StrCat[Dir,"Inductance",ExtGnuplot],
    SendToServer StrCat[po,"50Inductance from Flux [mH]"], Color "LightYellow" ];
  Print[ Inductance_from_MagEnergy, OnRegion DomainDummy, Format Table, LastTimeStepOnly,
    File StrCat[Dir,"Inductance",ExtGnuplot],
    SendToServer StrCat[po,"51Inductance from Magnetic Energy [mH]"], Color "LightYellow" ];
}


DefineConstant[
//  R_ = {"Analysis", Name "GetDP/1ResolutionChoices", Visible 0},
//  C_ = {"-solve -v2", Name "GetDP/9ComputeCommand", Visible 0},
//  P_ = {"", Name "GetDP/2PostOperationChoices", Visible 0}
  R_ = {"OptimStep", Name "GetDP/1ResolutionChoices",
	Choices {"Analysis", "OptimStep"}, Visible 0},
  C_ = {"-solve -v 5 -v2", Name "GetDP/9ComputeCommand", Visible 0},
  P_ = {"", Name "GetDP/2PostOperationChoices", Visible 0}
];
