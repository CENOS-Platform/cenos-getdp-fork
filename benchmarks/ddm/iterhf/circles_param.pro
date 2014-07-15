Include "freq.pro";
Include "runParams.pro";

solverTol = 1e-6;
MAXIT = 100;
RESTART = MAXIT;
RENEW_PHASE = 0;

SWEEP_FORWARD = 0;
SWEEP_BACKWARD = 0;

// SOLVER = "gmres_pcright" ;
SOLVER = "gmres" ;

coarseMesh = 1; // set 0 for uniform (fine) mesh
coarseSigma = 1; // set to 0 to finely mesh Sigma

// ratiolc1lc0 = coarseMesh ? 3 : 1 ;
lcExp = 3 ;


// nlambda = 30 ;
// nlambda_ref = 60 ;


Flag_Field = 1 ;
Flag_Quinconce = 0;
k = k0 ;
R0 = 1;

If (!Flag_Quinconce)
Nbr_SubProblems = Nh*Nv ;
EndIf
If (Flag_Quinconce)
  Nbr_SubProblems = Nv%2 ? (Nh-1)*(Nv+1)/2 + Nh*(Nv-1)/2 :(Nh-1)*Nv/2 + Nh*Nv/2 ;
EndIf
Printf("Nbr Subproblems: %g", Nbr_SubProblems);


disty = R0;
distx = R0;
dist = (distx>disty)?distx:disty;

MaxN = (Nh>Nv)?Nh:Nv;
Ro = 0.65*MaxN*(2*R0+dist) ;
//Ro = MaxN*(R0+dist/2)-dist/2 + R0 ;

lambda = 2*Pi/k ;
scalefactor = (k<2) ? 2./k : 1.;

lc0 = lambda/nlambda/scalefactor ;
lc1 = lambda/nlambda/scalefactor * ratiolc1lc0 ;

// lc_ref = lambda/nlambda_ref ;
