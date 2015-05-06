DefineConstant[GenerateVolFlag = 0];
DefineConstant[GenerateSurFlag_0 = 0, GenerateSurFlag_1 = 0];

Macro SolveSubdomains
  // work on own cpu
  SetCommSelf;
  For ii In {0: #ListOfDom()-1}
    idom = ListOfDom(ii);
    // update Dirichlet constraints (only actually necessary when #10
    // changes for Helmholtz)
    UpdateConstraint[Vol~{idom}, GammaD~{idom}, Assign];
    // solve the volume PDE on each subdomain
    If(GenerateVolFlag == 0)
      Generate[Vol~{idom}] ;
      Solve[Vol~{idom}] ;
      GenerateVolFlag = 1 ;
    EndIf
    If(GenerateVolFlag)
      GenerateRHSGroup[Vol~{idom}, Region[{Sigma~{idom}, TrOmegaGammaD~{idom}}] ] ;
      SolveAgain[Vol~{idom}] ;
    EndIf
    // solve the surface PDE on the boundaries of each subdomain
    For iSide In {0:1}
      If(NbrRegions[Sigma~{idom}~{iSide}])
        If(GenerateSurFlag~{iSide} == 0)
          Generate[Sur~{idom}~{iSide}] ;
          Solve[Sur~{idom}~{iSide}] ;
          GenerateSurFlag~{iSide} = 1 ;
        EndIf
        If(GenerateSurFlag~{iSide})
          GenerateRHSGroup[Sur~{idom}~{iSide},
			   Region[{Sigma~{idom}~{iSide}, TrPmlSigma~{idom}~{iSide}}]] ;
          SolveAgain[Sur~{idom}~{iSide}] ;
        EndIf
      EndIf
    EndFor
  EndFor
  // go back to parallel mode
  SetCommWorld;
Return

Macro UpdateGonSurfaces
  SetCommSelf;
  // compute g_in for next iteration, (must be
  // done after all resolutions)
  For ii In {0: #ListOfDom()-1}
    idom = ListOfDom(ii);
      For iSide In {0:1}
        PostOperation[g_out~{idom}~{iSide}] ;
      EndFor
  EndFor
  SetCommWorld;
Return

Macro SaveVolumeSolutions
  SetCommSelf;
  // compute the volume solution
  For ii In {0: #ListOfDom()-1}
    idom = ListOfDom(ii);
    PostOperation[DDM~{idom}] ;
  EndFor
  SetCommWorld;
Return

Macro EnablePhysicalSourcesOnly
  Evaluate[1. #10]; Evaluate[0. #11]; Evaluate[0. #12];
Return
Macro EnableArtificialSourcesOnly
  Evaluate[0. #10]; Evaluate[1. #11]; Evaluate[1. #12];
Return
Macro EnableAllSources
  Evaluate[1. #10]; Evaluate[1. #11]; Evaluate[1. #12];
Return

Resolution {
  { Name DDM ;
    System {
      For ii In {0: #ListOfDom()-1}
        idom = ListOfDom(ii);
        { Name Vol~{idom} ; NameOfFormulation Vol~{idom} ;
          Type Complex; NameOfMesh Sprintf(StrCat[MSH_NAME, "%g.msh"],idom) ; }
        For iSide In {0:1}
          { Name Sur~{idom}~{iSide} ; NameOfFormulation Sur~{idom}~{iSide} ;
            Type Complex; NameOfMesh Sprintf(StrCat[MSH_NAME, "%g.msh"],idom) ; }
        EndFor
      EndFor
    }
    Operation {
      If (MPI_Rank == 0)
        Printf[StrCat["Starting ", StrChoice[ANALYSIS == 0, "Helmholtz", "Maxwell"],
            " DDM with %g subdomains / %g processes"], N_DOM, MPI_Size];
        If(TC_TYPE == 0)
          Printf[StrCat["Using 0-th order (", StrChoice[ANALYSIS == 0, "Sommerfeld/EMDA",
                "Silver-Muller"], ") transmission conditions"]];
        EndIf
        If(TC_TYPE == 1)
          Printf[StrCat["Using 2-nd order (", StrChoice[ANALYSIS == 0, "OO2",
                "J-F. Lee"], ") transmission conditions"]];
        EndIf
        If(TC_TYPE == 2)
          Printf["Using %g-th order Pade (OSRC) transmission conditions", NP_OSRC];
        EndIf
        If(TC_TYPE == 3)
          Printf["Using PML transmission conditions"];
        EndIf
        Printf["Relative iterative solver tolerance = %g", TOL];
      EndIf

      // increase preallocation for Maxwell formulation
      If(ANALYSIS == 1)
        SetGlobalSolverOptions["-petsc_prealloc 200"];
      EndIf

      // compute rhs for Krylov solver on own cpu using physical sources only
      Call EnablePhysicalSourcesOnly;
      Call SolveSubdomains;
      Call UpdateGonSurfaces;

      // launch Krylov solver in parallel on all cpus
      IterativeLinearSolver["I-A", SOLVER, TOL, MAXIT, RESTART,
                            {ListOfField()}, {ListOfNeighborField()}, {}]
      {
        // solve each subproblem on own cpu using articifial sources only
	Call EnableArtificialSourcesOnly;
	Call SolveSubdomains;
	Call UpdateGonSurfaces;
      }

      //DeleteFile[ "/tmp/kspiter.txt" ];
      //Print[ {$KSPIts} , File "/tmp/kspiter.txt"];

      // build final volume solution after convergence on own cpu, using both
      // physical and artificial sources
      Call EnableAllSources;
      Call SolveSubdomains;
      Call SaveVolumeSolutions;
    }
  }
}
