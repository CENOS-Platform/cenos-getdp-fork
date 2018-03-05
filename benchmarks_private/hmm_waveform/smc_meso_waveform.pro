Include "smc_data.pro";

Group {
  GammaCornerFix = Region[ {GAMMA_POINT } ];

  GammaLeft      = Region[ {GAMMA_LEFT } ];
  GammaRight     = Region[ {GAMMA_RIGHT} ];
  GammaUp        = Region[ {GAMMA_UP} ];
  GammaDown      = Region[ {GAMMA_DOWN} ];

  If (Flag_Geometry == Half_Geometry)
    GammaLeft_NJ   = Region[ {GAMMA_LEFT_NJ } ];
    GammaLeft_TH   = Region[ {GAMMA_LEFT_TH } ];
  EndIf

  If (Flag_Geometry == Quarter_Geometry)
    GammaLeft_NJ   = Region[ {GAMMA_LEFT_NJ } ];
    GammaLeft_TH   = Region[ {GAMMA_LEFT_TH } ];
    GammaDown_NJ   = Region[ {GAMMA_DOWN_NJ } ];
    GammaDown_TH   = Region[ {GAMMA_DOWN_TH } ];
  EndIf
  Skin_Omega_C   = Region[ {SKIN_COND} ];

  Omega_NL       = Region[ {IRON} ];
  Omega_L        = Region[ {INSULATION} ];
  Omega_C        = Region[ {Omega_NL} ];
  Omega_CC       = Region[ {Omega_L} ];
  Omega          = Region[ {Omega_NL, Omega_L} ];
}

Function {
  mu0            = 4.e-7 * Pi;
  nu0            = 1/mu0;

  DefineConstant[ ELENUM=0, QPINDEX=0, t_0=0, t_end=0, dt_Macro=0, Flag_WR_meso=1,
                  Flag_meso_comp=0, which_time_window, which_wr_iteration, len_time, which_time_step, which_time];

  If(!Flag_WR_meso)
    bM[]   = Vector[BX, BY, BZ];
    /*
    Nbr_SubProblems = 1;

    BX_TIME[] = InterpolationAkima[$Time]{ListAlt[TIMEVEC, BX]};  
    BY_TIME[] = InterpolationAkima[$Time]{ListAlt[TIMEVEC, BY]};  
    BZ_TIME[] = InterpolationAkima[$Time]{ListAlt[TIMEVEC, BZ]};
    dBX_TIME[] = dInterpolationAkima[$Time]{ListAlt[TIMEVEC, BX]};  
    dBY_TIME[] = dInterpolationAkima[$Time]{ListAlt[TIMEVEC, BY]};  
    dBZ_TIME[] = dInterpolationAkima[$Time]{ListAlt[TIMEVEC, BZ]};  
    bM[]   = Vector[BX_TIME[], BY_TIME[], BZ_TIME[]];
    dbM[]   = Vector[dBX_TIME[], dBY_TIME[], dBZ_TIME[]];
    */
  EndIf

  If(Flag_WR_meso)
    AX_TIME[] = InterpolationAkima[$Time]{ListAlt[TIMEVEC, AX] };  
    AY_TIME[] = InterpolationAkima[$Time]{ListAlt[TIMEVEC, AY] };  
    AZ_TIME[] = InterpolationAkima[$Time]{ListAlt[TIMEVEC, AZ] };
    dAX_TIME[] = dInterpolationAkima[$Time]{ListAlt[TIMEVEC, AX] };  
    dAY_TIME[] = dInterpolationAkima[$Time]{ListAlt[TIMEVEC, AY] };  
    dAZ_TIME[] = dInterpolationAkima[$Time]{ListAlt[TIMEVEC, AZ] };  
    aM[]   = Vector[AX_TIME[], AY_TIME[], AZ_TIME[]];
    daM[]   = Vector[dAX_TIME[], dAY_TIME[], dAZ_TIME[]];  
    
    BX_TIME[] = InterpolationAkima[$Time]{ListAlt[TIMEVEC, BX]};  
    BY_TIME[] = InterpolationAkima[$Time]{ListAlt[TIMEVEC, BY]};  
    BZ_TIME[] = InterpolationAkima[$Time]{ListAlt[TIMEVEC, BZ]};
    dBX_TIME[] = dInterpolationAkima[$Time]{ListAlt[TIMEVEC, BX]};  
    dBY_TIME[] = dInterpolationAkima[$Time]{ListAlt[TIMEVEC, BY]};  
    dBZ_TIME[] = dInterpolationAkima[$Time]{ListAlt[TIMEVEC, BZ]};  
    bM[]   = Vector[BX_TIME[], BY_TIME[], BZ_TIME[]];
    dbM[]   = Vector[dBX_TIME[], dBY_TIME[], dBZ_TIME[]];
    
    DTAX_TIME[] = InterpolationAkima[$Time]{ListAlt[TIMEVEC, DTAX]};  
    DTAY_TIME[] = InterpolationAkima[$Time]{ListAlt[TIMEVEC, DTAY]};  
    DTAZ_TIME[] = InterpolationAkima[$Time]{ListAlt[TIMEVEC, DTAZ]};
    dDTAX_TIME[] = dInterpolationAkima[$Time]{ListAlt[TIMEVEC, DTAX]};  
    dDTAY_TIME[] = dInterpolationAkima[$Time]{ListAlt[TIMEVEC, DTAY]};  
    dDTAZ_TIME[] = dInterpolationAkima[$Time]{ListAlt[TIMEVEC, DTAZ]};  
    //eM[]   = -Vector[DTAX_TIME[], DTAY_TIME[], DTAZ_TIME[]]; // derivative of the interpolated function
    eM[]   = -Vector[dAX_TIME[], dAY_TIME[], dAZ_TIME[]]; // interpolation of the derivative values
    deM[]   = -Vector[dDTAX_TIME[], dDTAY_TIME[], dDTAZ_TIME[]];
      
    DTBX_TIME[] = InterpolationAkima[$Time]{ListAlt[TIMEVEC, DTBX]};  
    DTBY_TIME[] = InterpolationAkima[$Time]{ListAlt[TIMEVEC, DTBY]};  
    DTBZ_TIME[] = InterpolationAkima[$Time]{ListAlt[TIMEVEC, DTBZ]};
    dDTBX_TIME[] = dInterpolationAkima[$Time]{ListAlt[TIMEVEC, DTBX]};
    dDTBY_TIME[] = dInterpolationAkima[$Time]{ListAlt[TIMEVEC, DTBY]};  
    dDTBZ_TIME[] = dInterpolationAkima[$Time]{ListAlt[TIMEVEC, DTBZ]};  
    //dt_bM[]   = Vector[DTBX_TIME[], DTBY_TIME[], DTBZ_TIME[]]; // derivative of the interpolated function
    dt_bM[]   =  Vector[dBX_TIME[], dBY_TIME[], dBZ_TIME[]]; // interpolation of the derivative values
    dt_dbM[]   = Vector[dDTBX_TIME[], dDTBY_TIME[], dDTBZ_TIME[]];

  EndIf


  NbrMaxIter     = 5;
  Eps            = 1e-6;
  Relax          = 1.0;
  T               = 1.0/Freq;
  time0           = t_0;
  timemax         = t_end;
  ti              = time0;

  Printf("Done in the mesoscale resolution. t_0 = %g, t_end = %g and Nbr_SubProblems=%g. Which_time_step = %g Which_time = %g",
         t_0, t_end, Nbr_SubProblems, which_time_step, which_time);

  ratio_dtM_dtm   = 5;
  dt_Meso         = dt_Macro/ratio_dtM_dtm;
  theta_value     = 1;
  tf              = timemax;
  
  // Parameters for the electric linear law
  sigmaIron = 5.e6;
  sigma[Omega_NL] = sigmaIron;
  sigma[Omega_L] = 1.e-12 * sigmaIron;

  //Parameters of the Brauer nonlinear constitutive law
  nu[Omega_L] = nu0 * TensorDiag[1., 1., 1.];
  dhdb[Omega_L] = nu[SquNorm[$1]];
  If(!Flag_NL)
    nu[Omega_NL] = nu0 * TensorDiag[1., 1., 1.];
    dhdb[Omega_NL] = nu[SquNorm[$1]];
    dhdb_NL[Omega_NL] = nu0 * TensorDiag[0., 0., 0.];
  EndIf
  If(Flag_NL)
    gamma = 388.;
    alpha = 0.3774;
    beta = 2.97;
    nu_ML[Omega_NL] = gamma + alpha * Exp[beta*$1];
    dnudb2[] = alpha * beta* Exp[beta*$1];
    nu[Omega_NL] = nu_ML[SquNorm[$1]];
    dhdb[Omega_NL] = nu[SquNorm[$1]] * TensorDiag[1., 1., 1.] + 2 * dnudb2[SquNorm[$1]] * SquDyadicProduct[$1];
    dhdb_NL[Omega_NL] = 2 * dnudb2[SquNorm[$1]] * SquDyadicProduct[$1];
  EndIf
  If (!Flag_WR_meso)
    a_tprevious[] = Vector[0., 0., ScalarField[XYZ[], $1]{0}];
  EndIf
  If (Flag_WR_meso)
    a_tprevious[]   = (which_time_window == 1) ? Vector[0.,0.,0.] : Vector[0., 0., ScalarField[XYZ[]]{0}];
  EndIf

    Printf("Running computations for time step %g", (which_time_step - (which_wr_iteration * len_time)) );
    
}

Constraint {
  { Name a_Meso;
    Case {
      { Region GammaRight; Type Link; RegionRef GammaLeft;
        Coefficient 1.; Function Vector[$X-lx, $Y, $Z]; }
      { Region GammaUp; Type Link; RegionRef GammaDown;
        Coefficient 1.; Function Vector[$X, $Y-ly, $Z]; }
      { Region GammaCornerFix; Type Assign; Value 0.0; }
    }
  }

  { Name a_Meso_WR ;
    Case {
      If (!Flag_WR_meso)
        { Region Omega ; Type Init ; Value Field[XYZ[] ]{0} ; }
        //{ Region Omega ; Type Init ; Value Field[XYZ[]]{0} ; }
      EndIf
    }
  }  

  
  { Name a_Meso_Init;
    Case {
      If(Flag_Dynamic && Flag_WR_meso && (which_time_window > 1) ) 
        //{ Region Omega ; Type Init ; Value Field[XYZ[]]{0} ; }
        { Type InitFromResolution; Region Omega; NameOfResolution a_Init; }
      EndIf
    }
  }

 { Name Current_2D;
    Case {
      // forcing net current in each lamination to be equal to zero
      { Region Omega_C; Type Assign; Value 0.0; }
    }
  }
 { Name Voltage_2D;
    Case {
    }
  }
}

If(Flag_WR_meso)
  Include "MagStaDyn_a_meso_waveform.pro"
EndIf
If(!Flag_WR_meso)
  Include "MagStaDyn_a_meso_waveform_1.pro"
EndIf
