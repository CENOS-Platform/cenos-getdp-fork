// Optimization problem specification

// performance function type
NO_PERF = 0;
BFIELD_ERROR = 1;
TORQUE_VAR = 2;
IRON_LOSSES = 3;
COMPLIANCE = 4;
VOLUME = 5;
TORQUE = 6;

// postpro views tag
TORQUE_VAR_FIELD = 20;
VELOCITY_FIELD = 7;//pour que ça marche avec -gmshread il faut mettre à 0

DefineConstant[
  pInOpt = "Input/OptParam/",

  // Optimization 
  Flag_opt = {1,
    Name StrCat[pInOpt,"opt"], Label "Optimization",
    Choices {0,1}, Visible 1},

  Flag_topopt = {0, 
    Name StrCat[pInOpt,"optType"],Label "TopOpt?",
    Choices {0,1}, Visible (Flag_opt == 1)},

  // Design variables -> FIXME: write in py toolkit!
  lm = {2.352*mm , 
    Name StrCat[pInOpt,"x_0"], Visible (Flag_opt == 1), Closed 1},  
  Th_magnet = {32.67 *deg, 
    Name StrCat[pInOpt,"x_1"], Visible (Flag_opt == 1), Closed 1},
  AxialLength = {35*mm,
    Name StrCat[pp,"Axial length [m]"],Visible (Flag_opt == 1),Closed 1},
  Gap = {(26.02-25.6)*mm,
    Name StrCat[pp,"Airgap width [m]"],Visible (Flag_opt==1), Closed 1},

  // Velocity field (Mesh perturbation)
  PerturbMesh = {0, Choices{0,1},
    Name "Sensitivity/Compute perturbation velocity field", Visible (Flag_opt == 1)},
  Perturbation = {1e-6,
    Name "Sensitivity/Perturbation value", Visible (Flag_opt == 1)},
  SensitivityParameter = { StrCat[pInOpt, "x_0"],
    Choices{
      StrCat[pInOpt, "x_0"],
      StrCat[pInOpt, "x_1"],
      StrCat[pInOpt, "x_2"],
      StrCat[pInOpt, "x_3"],
      StrCat[pInOpt, "x_4"],
      StrCat[pInOpt, "x_5"],
      StrCat[pInOpt, "x_6"]
    },
    Name "Sensitivity/Parameter to perturb", Visible (Flag_opt == 1) }

  // sensitivity analysis 
  Flag_PerfType = {TORQUE, 
    Choices {
      NO_PERF="No performance function",  
      BFIELD_ERROR="air gap B field error",
      TORQUE_VAR="torque variance",
      IRON_LOSSES="losses",
      COMPLIANCE="compliance",
      TORQUE="torque"
    },
   Name "Input/OptParam/PerfType",Label "performance function type", Visible 1},

  Tnom = {90.0, 
    Name "Input/OptParam/TorqueNominal", Label "Nominal desired torque",
    Visible (Flag_PerfType==TORQUE_VAR)},

  regionVar = {0, Name "Input/OptParam/regionVar",
                  Label "Region of design variables", 
                  Choices {0="Rotor Fe",1="Stator Fe",2="Rotor/Stator Fe"},
                  Visible (Flag_topopt)},

  Flag_InterpLaw = {0, Name "Input/OptParam/MaterialInterpLaw",
                       Label "material interpolation law",
                       Choices {0="SIMP",1="RAMP"},Visible (Flag_topopt)},

  degree_SIMP = {3.0, Name "Input/OptParam/SimpPenalDegree",
                      Label "Degree SIMP", Visible (Flag_topopt)}

] ;

