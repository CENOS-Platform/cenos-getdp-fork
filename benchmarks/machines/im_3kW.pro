// Authors - J. Gyselinck, R.V. Sabariego (2013)
//
// Induction motor
//

Include "im_3kW_data.geo" ;

DefineConstant[
  Flag_AnalysisType = {2,  Choices{0="Static",  1="Time domain",  2="Frequency domain"},
    Label "Type of analysis",  Path "Input/29", Highlight "Blue", Visible 1,
    Help Str["- Use 'Static' to compute static fields created in the machine",
      "- Use 'Time domain' to compute the dynamic response of the machine",
      "- Use 'Frequency domain' to compute steady-state phasors depending on the slip"]} ,
  Flag_SrcType_Stator = { 2, Choices{1="Current", 2="Voltage"},
    Label "Source type in stator", Path "Input/41", Highlight "Blue", Visible 1},
  Flag_Cir_RotorCage = { (Flag_SrcType_Stator==2), Choices{0,1},
    Label "Use circuit in rotor cage", Path "Input/40", ReadOnly (Flag_SrcType_Stator==1), Visible 1}
  slip = { 0, Min 0., Max 1, Step 0.05, Loop (Flag_AnalysisType == 2),
    Label "slip", Path "Input/30", Highlight "AliceBlue", Visible (Flag_AnalysisType == 2)}
];

Flag_Cir = (Flag_SrcType_Stator==2);

// this not necessary anymore (thanks to the new behavior of Visibility)
If(Flag_AnalysisType!=2)
  UndefineConstant[ "Input/30slip" ];
EndIf
variableFrequencyLoop = slip;

DefineConstant[
  Flag_NL = { (Flag_AnalysisType==2)?0:1, Choices{0,1},
    Label "Nonlinear BH-curve", Path "Input/60", ReadOnly (Flag_AnalysisType==2)?1:0, Visible 1},
    // FIXME: nonlinear law in frequency domain not yet implemented
  Flag_NL_law_Type = { 2, Choices{0="Analytical", 1="Interpolated",
      2="Analytical VH800-65D", 3="Interpolated VH800-65D"},
    Label "BH-curve", Path "Input/61", Highlight "Blue", Visible Flag_NL}
] ;

If(Flag_AnalysisType==2)
  UndefineConstant["Input/61Flag_NL_law"];
EndIf


Group{
  DefineGroup[ Stator_Al, Stator_Cu ];
  DefineGroup[ Rotor_Al, Rotor_Cu ];
}

Group{
  Stator_Fe     = #STATOR_FE ;
  Stator_Air    = #STATOR_SLOTOPENING ;
  Stator_Airgap = #STATOR_AIRGAP ;

  Stator_Bnd_A0 = #STATOR_BND_A0 ;
  Stator_Bnd_A1 = #STATOR_BND_A1 ;

  If(Flag_OpenRotor)
    Rotor_Fe     = #{ROTOR_FE} ;
    Rotor_Air    = #{ROTOR_SLOTOPENING} ;
  EndIf
  If(!Flag_OpenRotor)
    Rotor_Fe     = #{ROTOR_FE, ROTOR_SLOTOPENING} ;
    Rotor_Air    = #{} ;
  EndIf

  Rotor_Airgap = #ROTOR_AIRGAP ;

  nbRotorBars = (Flag_Symmetry) ? NbrPolesInModel*NbrSectTot/NbrPolesTot : NbrSectTot ;
  For k In {1:nbRotorBars}
    Rotor_Bar~{k} = Region[ (ROTOR_BAR+k) ];
    Rotor_Bars += Region[ Rotor_Bar~{k} ];
  EndFor

  Rotor_Bnd_MB = #ROTOR_BND_MOVING_BAND ;
  Rotor_Bnd_A0 = #ROTOR_BND_A0 ;
  Rotor_Bnd_A1 = #ROTOR_BND_A1 ;

  MovingBand_PhysicalNb = #MOVING_BAND ;  // Fictitious number for moving band, not in the geo file
  Surf_Inf = #SURF_EXT ;
  Surf_bn0 = #SURF_INT ;
  Surf_cutA0 = #{STATOR_BND_A0, ROTOR_BND_A0};
  Surf_cutA1 = #{STATOR_BND_A1, ROTOR_BND_A1};

  Stator_Ind_Ap = #{STATOR_IND_AP}; Stator_Ind_Am = #{};
  Stator_Ind_Bp = #{STATOR_IND_BP}; Stator_Ind_Bm = #{};
  Stator_Ind_Cp = #{}             ; Stator_Ind_Cm = #{STATOR_IND_CM};
  If(NbrPolesInModel > 1)
    Stator_Ind_Am += #STATOR_IND_AM;
    Stator_Ind_Bm += #STATOR_IND_BM;
    Stator_Ind_Cp += #STATOR_IND_CP;
  EndIf

  PhaseA = Region[{ Stator_Ind_Ap, Stator_Ind_Am }];
  PhaseB = Region[{ Stator_Ind_Bp, Stator_Ind_Bm }];
  PhaseC = Region[{ Stator_Ind_Cp, Stator_Ind_Cm }];

  // FIXME: Just one physical region for nice graph in Onelab
  PhaseA_pos = Region[{ Stator_Ind_Ap }];
  PhaseB_pos = Region[{ Stator_Ind_Bp }];
  PhaseC_pos = Region[{ Stator_Ind_Cm }];

  Stator_IndsP = Region[{ Stator_Ind_Ap, Stator_Ind_Bp, Stator_Ind_Cp }];
  Stator_IndsN = Region[{ Stator_Ind_Am, Stator_Ind_Bm, Stator_Ind_Cm }];

  Stator_Inds = Region[ {PhaseA, PhaseB, PhaseC} ] ;
  Rotor_Inds  = Region[ {} ] ;

  StatorC  = Region[{ }] ;
  StatorCC = Region[{ Stator_Fe }] ;
  RotorC   = Region[{ Rotor_Bars }] ;
  RotorCC  = Region[{ Rotor_Fe   }] ;


  // Moving band:  with or without symmetry, the BND line of the rotor must be complete
  Stator_Bnd_MB = #STATOR_BND_MOVING_BAND;
  For k In {1:NbrPolesTot/NbrPolesInModel}
    Rotor_Bnd_MB~{k} = Region[ (ROTOR_BND_MOVING_BAND+k-1) ];
    Rotor_Bnd_MB += Region[ Rotor_Bnd_MB~{k} ];
  EndFor
  For k In {2:NbrPolesTot/NbrPolesInModel}
    Rotor_Bnd_MBaux  += Region[ Rotor_Bnd_MB~{k} ] ;
  EndFor

  Dummy = #NICEPOS;   // boundary between different materials, used for animation
}

Function{
  NbrPolePairs = NbrPolesTot/2 ;

  Freq = 50  ;
  Period = 1/Freq ; // Fundamental period in s

  DefineConstant[
    Flag_ImposedSpeed = { 0, Choices{0="None", 1="Synchronous speed (no load)",
        2="Choose speed"}, Label "Imposed rotor speed [rpm]", Highlight "Blue",
      Path "Input/30", Visible Flag_AnalysisType!=2},
    myrpm = { rpm_nominal, Label "Speed [rpm]", Path "Input/31",
      Highlight "AliceBlue", ReadOnlyRange 1, Visible (Flag_ImposedSpeed==2 && Flag_AnalysisType!=2)},
    Tmec = { 0, Label "Mechanical torque [Nm]", Path "Input/32",
      Highlight "AliceBlue", Visible (!Flag_ImposedSpeed && Flag_AnalysisType!=2) },
    Frict = { 0, Label "Friction torque [Nm]", Path "Input/33",
      Highlight "AliceBlue", Visible (!Flag_ImposedSpeed && Flag_AnalysisType!=2) },
    NbrPeriod = {10, Label "Total number of periods", Path "Input/40",
      Highlight "AliceBlue", Visible (Flag_AnalysisType==1)},
    NbSteps = {100, Label "Number of time steps per period", Path "Input/41",
      Highlight "AliceBlue", Visible (Flag_AnalysisType==1)}
  ];


  // relaxation of applied voltage, for reducing the transient
  NbTrelax = 2 ; // Number of periods while relaxation is applied
  Trelax = NbTrelax*Period;
  Frelax[] = (!Flag_NL || Flag_AnalysisType==0 || $Time>Trelax) ? 1. :
             0.5*(1.-Cos[Pi*$Time/Trelax]) ; // smooth step function

  rpm_syn = 60*Freq/NbrPolePairs ;

  rpm = (Flag_ImposedSpeed==0) ? 0.:
        ((Flag_ImposedSpeed==1) ? rpm_syn : myrpm) ;

  //slip = (rpm_syn-rpm)/rpm_syn ; // slip = 1 ; ==> blocked rotor

  wr = (Flag_AnalysisType==2) ? (1-slip)*2*Pi*Freq/NbrPolePairs : rpm/60*2*Pi ; // angular rotor speed in rad_mec/s

  // imposed movement with fixed speed wr
  delta_time = Period/NbSteps; // time step in s
  delta_theta[] = (Flag_ImposedSpeed) ? (delta_time*wr) : (#77-#66); // angle step (in rad)
  time0 = 0.;                 // initial time in s
  timemax = NbrPeriod*Period;  // final time  in s

  sigma[ Rotor_Bars ] = (Flag_AnalysisType==2 ? slip : 1.)*sigma_bars ;

  Stator_PhaseArea[] = SurfaceArea[]{STATOR_IND_AP} + SurfaceArea[]{STATOR_IND_AM};
  NbWires[]  = 2*Ns*NbrPolesInModel/NbrPolesTot; // Number of wires in series per phase
  SurfCoil[] = Stator_PhaseArea[];

  pA =  0 ;
  pB =  4*Pi/3 ;
  pC =  2*Pi/3 ;

  DefineConstant[
    Irms = { IA, Path "Input/50", Label "Stator current (rms)",
      Highlight "AliceBlue", Visible (Flag_SrcType_Stator==1)},
    Vrms = { VA, Path "Input/50", Label "Stator voltage (rms)",
      Highlight "AliceBlue", Visible (Flag_SrcType_Stator==2)}
  ] ;
  VV = Vrms * Sqrt[2] ;
  II = Irms * Sqrt[2] ;

  Friction[] = Frict ;
  Torque_mec[] = Tmec ;
  Inertia = inertia_fe ;

}

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------


If(Flag_Cir)
  Include "im_3kW_circuit.pro" ;
EndIf
Include "machine_magstadyn_a.pro" ;


