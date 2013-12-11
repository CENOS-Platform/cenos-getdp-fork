// Multiple scattering by ellipsoidal cylinders in dimension 2

Printf('=================New Geometry=================');

Include "scattering_data.pro";
Include "CreateEllipses.geo"; // function which constructs automatically the boundary of the scatterers (=ellipses)

//Center point
PF = newp; Point(PF)={0, 0, 0, lcScat};

//Scatterers
//==========
//CentreX[]={0,0,2};
//CentreY[]={0,3,0};
//RadiusX[] = {2,0.5,0.5};
//RadiusY[] = {2,0.5,0.5};

//pre-variable, before verifying the geometry is ok
CentreX_pre[] = {}; CentreY_pre[] = {};
RadiusX_pre[] = {}; RadiusY_pre[] = {};

For ns In {0:NMAX-1}
  DefineConstant[
    x~{ns}={0., Min -10000, Max 10000, Step 0.1, Label "X-coord", Name Str[MENU_OBST, Sprintf("/Obst. %g/1x", ns+1)], Visible (ns < N_scat_to_create)}
    y~{ns}={0., Min -10000, Max 10000, Step 0.1, Label "Y-coord", Name Str[MENU_OBST, Sprintf("/Obst. %g/1y", ns+1)], Visible (ns < N_scat_to_create)}
    rx~{ns}={1., Min 0.1, Max Xmax, Step 0.1, Label "X-radius", Name Str[MENU_OBST, Sprintf("/Obst. %g/2rx", ns+1)], Visible (ns < N_scat_to_create)}
    ry~{ns}={1., Min 0.1, Max Ymax, Step 0.1, Label "Y-radius", Name Str[MENU_OBST, Sprintf("/Obst. %g/2ry", ns+1)], Visible (ns < N_scat_to_create)}
  ];
  CentreX_pre[] += x~{ns};
  CentreY_pre[] += y~{ns};
  RadiusX_pre[] += rx~{ns};
  RadiusY_pre[] += ry~{ns};
EndFor

// Initialization of the list of "Line Loop" (LL_scat) and of "Line" (Line_Scat)
LL_scat[] = {};
Line_Scat[] = {};
S_scat[] = {};
CentreX[] = {}; CentreY[] = {};
RadiusX[] = {}; RadiusY[] = {};

//Creation of the circular scatterers
N_scat = 0;
Call CreateEllipses;
DefineConstant[
  N_scat2 = {N_scat, Name Str[MENU_OBST,"/0Nscat"], Label "Nb. of placed obstacles", ReadOnly 1 }
];
For pCreate In {0:(N_scat_to_create-1)}
  //plot result for user
  DefineConstant[
    _NotFit~{pCreate} = {1, Choices{0, 1}, Name Str[MENU_OBST, Sprintf("/Obst. %g/00OK", pCreate+1)], Label "Cannot be placed", Visible (pCreate < N_scat_to_create && !(_ItsOK~{pCreate})), ReadOnly 1, Highlight "Red"}
  ];
EndFor

/*If(Type_PROBLEM == PENETRABLE)
  Physical Surface(Ind_Scat) = {S_scat[]};
EndIf*/

// Fictitious Boundary
//====================
If(Type_Truncation == ABC)
  PABC1 = newp; Point(PABC1) = {Xmax, 0, 0, lc};
  PABC2 = newp; Point(PABC2) = {0, Ymax, 0, lc};
  PABC3 = newp; Point(PABC3) = {XBoundmin, 0, 0, lc};
  PABC4 = newp; Point(PABC4) = {0, YBoundmin, 0, lc};
  
  LineABC1 = newl; Ellipse(LineABC1) = {PABC1, PF, PABC2, PABC2};
  LineABC2 = newl; Ellipse(LineABC2) = {PABC2, PF, PABC3, PABC3};
  LineABC3 = newl; Ellipse(LineABC3) = {PABC3, PF, PABC4, PABC4};
  LineABC4 = newl; Ellipse(LineABC4) = {PABC4, PF, PABC1, PABC1};
  
  LLABC = newll; Line Loop(LLABC) = {LineABC1, LineABC2, LineABC3, LineABC4};

  // Surfaces
  //=========
  
  SurfPropagation_Domain = news; Plane Surface(SurfPropagation_Domain) = {LL_scat[],LLABC};
  
  Physical Line(Ind_GammaInf) = {LineABC1, LineABC2, LineABC3, LineABC4};  
EndIf

// Fictitious Boundary
//====================
If(Type_Truncation == PML)
    If(Type_SHAPE == DOM_SQUARE)
    p_int_DR = newp; Point(p_int_DR) = {Xmax, -Ymax, 0, lc};
    p_int_UR = newp; Point(p_int_UR) = {Xmax, Ymax, 0, lc};
    p_int_UL = newp; Point(p_int_UL) = {-Xmax, Ymax, 0, lc};
    p_int_DL = newp; Point(p_int_DL) = {-Xmax, -Ymax, 0, lc};
  
    Line_int_R = newl; Line(Line_int_R) = {p_int_DR, p_int_UR};
    Line_int_U = newl; Line(Line_int_U) = {p_int_UR, p_int_UL};
    Line_int_L = newl; Line(Line_int_L) = {p_int_UL, p_int_DL};
    Line_int_D = newl; Line(Line_int_D) = {p_int_DL, p_int_DR};
    
    LL_int = newll; Line Loop(LL_int) = {Line_int_R, Line_int_U, Line_int_L, Line_int_D};
    //truncation of the PML
    p_ext_DR = newp; Point(p_ext_DR) = {Xmax + SizePMLX, -Ymax - SizePMLY, 0, lc};
    p_ext_UR = newp; Point(p_ext_UR) = {Xmax + SizePMLX, Ymax + SizePMLY, 0, lc};
    p_ext_UL = newp; Point(p_ext_UL) = {-Xmax - SizePMLX, Ymax + SizePMLY, 0, lc};
    p_ext_DL = newp; Point(p_ext_DL) = {-Xmax - SizePMLX, -Ymax - SizePMLY, 0, lc};
    
    Line_ext_R = newl; Line(Line_ext_R) = {p_ext_DR, p_ext_UR};
    Line_ext_U = newl; Line(Line_ext_U) = {p_ext_UR, p_ext_UL};
    Line_ext_L = newl; Line(Line_ext_L) = {p_ext_UL, p_ext_DL};
    Line_ext_D = newl; Line(Line_ext_D) = {p_ext_DL, p_ext_DR};

    LL_ext = newll; Line Loop(LL_ext) = {Line_ext_R, Line_ext_U, Line_ext_L, Line_ext_D};
    Physical Line(Ind_PML_Bound) = {Line_ext_R, Line_ext_U, Line_ext_L, Line_ext_D};  
  EndIf
  If(Type_SHAPE == DOM_CIRCULAR)
    p_int_R = newp; Point(p_int_R) = {Xmax, 0, 0, lc};
    p_int_U = newp; Point(p_int_U) = {0, Ymax, 0, lc};
    p_int_L = newp; Point(p_int_L) = {-Xmax, 0, 0, lc};
    p_int_D = newp; Point(p_int_D) = {0, -Ymax, 0, lc};
  
    Line_int_RU = newl; Ellipse(Line_int_RU) = {p_int_R, PF, p_int_U, p_int_U};
    Line_int_UL = newl; Ellipse(Line_int_UL) = {p_int_U, PF, p_int_L, p_int_L};
    Line_int_LD = newl; Ellipse(Line_int_LD) = {p_int_L, PF, p_int_D, p_int_D};
    Line_int_DR = newl; Ellipse(Line_int_DR) = {p_int_D,PF, p_int_R, p_int_R};

    LL_int = newll; Line Loop(LL_int) = {Line_int_RU, Line_int_UL, Line_int_LD, Line_int_DR};

    //truncation of the PML
    p_ext_R = newp; Point(p_ext_R) = {Xmax + SizePMLX, 0, 0, lc};
    p_ext_U = newp; Point(p_ext_U) = {0, Ymax + SizePMLY, 0, lc};
    p_ext_L = newp; Point(p_ext_L) = {-Xmax - SizePMLX, 0, 0, lc};
    p_ext_D = newp; Point(p_ext_D) = {0, -Ymax - SizePMLY, 0, lc};
    
    Line_ext_RU = newl; Ellipse(Line_ext_RU) = {p_ext_R, PF, p_ext_U, p_ext_U};
    Line_ext_UL = newl; Ellipse(Line_ext_UL) = {p_ext_U, PF, p_ext_L, p_ext_L};
    Line_ext_LD = newl; Ellipse(Line_ext_LD) = {p_ext_L, PF, p_ext_D, p_ext_D};
    Line_ext_DR = newl; Ellipse(Line_ext_DR) = {p_ext_D, PF, p_ext_R, p_ext_R};

    LL_ext = newll; Line Loop(LL_ext) = {Line_ext_RU, Line_ext_UL, Line_ext_LD, Line_ext_DR};
    Physical Line(Ind_PML_Bound) = {Line_ext_RU, Line_ext_UL, Line_ext_LD, Line_ext_DR};
  EndIf

  // Surfaces
  //=========
  SurfPropagation_Domain = news; Plane Surface(SurfPropagation_Domain) = {LL_scat[],LL_int};
  SurfPML = news; Plane Surface(SurfPML) = {LL_int,LL_ext};
  
  Physical Surface(Ind_PML) = {SurfPML};
  
EndIf

//  Physical entities
//===================
Physical Surface(Ind_Propagation_Domain) = {SurfPropagation_Domain};  
// Boundaries
//Physical Line(Ind_Scat_Bound) = {Line_Scat[]};

For j In {0:N_scat-1}
  Physical Line(100+j) = {Line_Scat[4*j], Line_Scat[4*j+1], Line_Scat[4*j+2], Line_Scat[4*j+3]};
EndFor

//Just to plot the point source
If(INCIDENT_WAVE == POINTSOURCE && PLOT_POINT_SOURCE)
  Include "scattering_data_opt.pro";
  X_source = r_source*Cos[theta_source];
  Y_source = r_source*Sin[theta_source];
  ps = newp; Point(ps) = {X_source, Y_source, 0, lc};
EndIf

