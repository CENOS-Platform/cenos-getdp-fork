//-------------
// Frequencies
//-------------
// k_geo : "k" used to build geometry (gmsh)
// k_dis : "k" used for the discretization
// k_w : "true wavenumber", used in the Helmholtz equation (getdp)
MENU_WAVENUMBER = "Wavenumbers/";
//actual wavenumber (in simulation)
DefineConstant[
  k = {10., Min 1., Max 100., Step 0.1, Label "Actual wavenumber k", Name Str[MENU_WAVENUMBER, "0"]}
  //reference to build geometry
  k_geo = {10., Min 1., Max 100., Step 0.1, Label "k_geo", Name Str[MENU_WAVENUMBER, "1"]}
  //reference for discretization
  //(if band of frequencies for example)
  k_dis = {10., Min 1., Max 100., Step 0.1, Label "k_dis", Name Str[MENU_WAVENUMBER, "2"]}
];
lambda = 2*Pi/k;
lambda_geo = 2*Pi/k_geo;
lambda_dis = 2*Pi/k_dis;


/*======================================================
Parameters of the TRM
L : distance from the mirror to the source (x-direction)
Dx : Size in the x-direction 
Dy : Size in the y-direction 
Dz : Size in the z-direction
======================================================*/
MENU_TRM = "TRM/";

DefineConstant[
  nL = {20., Min 1., Max 100., Step 0.1, Label "Distance to source (in lambda_geo)", Name Str[MENU_TRM, "0"]}
  nDx = {0.2, Min 0.1, Max 10., Step 0.01, Label "Thickness (in lambda_geo)", Name Str[MENU_TRM, "1"]}
  nDy = {15., Min 0.1, Max 100., Step 0.1, Label "Aperture (in lambda_geo)", Name Str[MENU_TRM, "2"]}
];

L = lambda_geo*nL;
Dx = lambda_geo*nDx;
Dy = lambda_geo*nDy;
Dz = lambda_geo*0;

MENU_SOURCE = "Source/";
DefineConstant[
  //Coordinate of the source :
  YS = {0., Min -Dy/2, Max Dy/2, Step 0.1, Label "YS", Name Str[MENU_SOURCE, "0"]}
];
XS = 0;
ZS = 0;


// TRM
//=========
X_TRM_min = XS + L;
X_TRM_max = XS + L + Dx;
Y_TRM_min = -Dy/2;
Y_TRM_max = Dy/2;

MENU_PROP = "Propagation domain/";
// Domaine d'etude
//=========
DefineConstant[
  n_LS = {nL + nDx, Min 1., Max 30., Step 0.1, Label "Distance behind the source", Name Str[MENU_PROP, "0"]}
  n_LTRM = {Dy/4, Min Dy/10, Max 100., Step 0.1, Label "Distance behind the TRM", Name Str[MENU_PROP, "1"]}
  n_LY = {Dy/5, Min Dy/10, Max 100., Step 0.1, Label "Distance on top/bottom of the TRM", Name Str[MENU_PROP, "2"]}
];
LbehindS = n_LS*lambda_geo;
LbehindTRM = n_LTRM*lambda_geo;
LUpDownTRM = n_LY*lambda_geo;

X_Omega_min = XS - LbehindS;
X_Omega_max = X_TRM_max + Dy/4;
Y_Omega_min = Y_TRM_min - LUpDownTRM;
Y_Omega_max = Y_TRM_max + LUpDownTRM;
  
MENU_LC = "Step discretization in nb of points per lambda_dis/";
DefineConstant[
  //TRM
  nlcTRM = {15., Min 1., Max 30., Step 0.1, Label "TRM", Name Str[MENU_LC, "0"]}
  //Source (small open subset surrounding the source)
  nlcSourceExt = {15., Min 1., Max 30., Step 0.1, Label "Around the source", Name Str[MENU_LC, "1"]}
  //Interne boundary separating propagation and absorbing domains
  nlcIntern_Bound = {10., Min 1., Max 30., Step 0.1, Label "Boundary with PML", Name Str[MENU_LC, "2"]}
  //Extern boundary of the PML (truncation)
  nlcExtern_Bound = {10., Min 1., Max 30., Step 0.1, Label "End of PML", Name Str[MENU_LC, "2"]}
  //source
];
lcTRM = lambda_dis/nlcTRM;
lcSourceExt = lambda_dis/nlcSourceExt;
lcIntern_Bound = lambda_dis/nlcIntern_Bound;
lcExtern_Bound = lambda_dis/nlcExtern_Bound;
lcSourceInt = lcIntern_Bound;

//constant parameters

// PML
//====

// Max value of absorbing coefficient
// AlphaX = k_w + i SigmaX
SigmaXmax = 100;
SigmaYmax = 100;

SizeInteriorDomainX = X_Omega_max - X_Omega_min;
SizeInteriorDomainY = Y_Omega_max - Y_Omega_min;

// Coordinate of the center of the domain
//XF = (XS + 1.15*L)/2;
XF = (X_Omega_max + X_Omega_min)/2;
YF = (Y_Omega_max + Y_Omega_min)/2;
ZF = ZS;

// Width and Height of the absorbing layer
MENU_PML = "PML parameters/";
DefineConstant[
  nSizePMLX = {10., Min 1., Max 30., Step 0.1, Label "x-size (in nb. element)", Name Str[MENU_PML, "0"]}
  nSizePMLY = {10., Min 1., Max 30., Step 0.1, Label "y-size (in nb. element)", Name Str[MENU_PML, "1"]}
];
SizePMLX = nSizePMLX*lcIntern_Bound;
SizePMLY = nSizePMLY*lcIntern_Bound;




