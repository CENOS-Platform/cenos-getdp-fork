// Functions
// ==========
//PML function
Function{
  k[] = #10; // registre number 10 is booked !

  Dist_XF_Boundary = Sqrt[(XF - Xmax)^2];
  Dist_YF_Boundary = Sqrt[(YF - Ymax)^2];
  // Distance between a point (X,Y,Z) and the centre of the domain (XF,YF,ZF)
  RF_X[] = Sqrt[(X[] - XF)^2];
  RF_Y[] = Sqrt[(Y[] - YF)^2];

  DampingProfileX[] = 1/(Dist_XF_Boundary + SizePMLX - Fabs[RF_X[]]) - 1/(SizePMLX);
  DampingProfileY[] = 1/(Dist_YF_Boundary + SizePMLY - Fabs[RF_Y[]]) - 1/(SizePMLY);
  //Take Max(0, DampingProfile)
  SigmaX[] = 0.5*(DampingProfileX[] + Fabs[DampingProfileX[]]);
  SigmaY[] = 0.5*(DampingProfileY[] + Fabs[DampingProfileY[]]);
  
  Kx[] = Complex[1, SigmaX[]/k[]];
  Ky[] = Complex[1, SigmaY[]/k[]];
  D[] = TensorDiag[Ky[]/Kx[], Kx[]/Ky[], 0.];
  S_PML[] = Kx[]*Ky[];
  
}

Function {
  I[] = Complex[0., 1.] ;

  // Distance between a point (X,Y,Z) and the source (XS,YS,ZS):
  R[]= Sqrt[(X[] - XS)^2 + (Y[] - YS)^2 + (Z[] - ZS)^2];
  KR[] = k[]*R[];

  // Green2D[] = i/4*Hankel_0^{(1)}(kR[])
  Green2D[] = 0.25*Complex[-Yn[0,KR[]],Jn[0,KR[]]];
  // Green2D[] conjugated:
  GreenConjug[] = -0.25*Complex[Yn[0,KR[]],Jn[0,KR[]]];
}

// FORMULATION
// ============
Formulation {
  //Emission (if approx. Green)
  If(CLUTTER)
    { Name Emission; Type FemEquation;
      Quantity{
	{Name Ue ; Type Local; NameOfSpace EspUforw;}
      }
      Equation{
	Galerkin{[D[]*Dof{Grad Ue}, {Grad Ue}];
	  In AllDomains; Jacobian JVol; Integration I1;}
	Galerkin{[-k[]^2*n[]^2*Kx[]*Ky[]*Dof{Ue}, {Ue}];
	  In AllDomains; Jacobian JVol; Integration I1;}
	// Approx. Dirac
	Galerkin{[-Dirac[], {Ue}];
	  In SourceInt; Jacobian JVol; Integration I1;}
      }
    }
  EndIf

  //Back propagation
  { Name BackProp; Type FemEquation;
    Quantity{
      If(CLUTTER)
	{Name Ue ; Type Local; NameOfSpace EspUforw;}
      EndIf
      {Name Uback ; Type Local; NameOfSpace EspUback;}
    }
    Equation{
      Galerkin{[D[]*Dof{Grad Uback}, {Grad Uback}];
	In AllDomains; Jacobian JVol; Integration I1;}
      Galerkin{[-k[]^2*n[]^2*Kx[]*Ky[]*Dof{Uback}, {Uback}];
	In AllDomains; Jacobian JVol; Integration I1;}
      // Source (conjugated)
      If(!CLUTTER)
	Galerkin{[-GreenConjug[], {Uback}];
	  In TRM; Jacobian JVol; Integration I1;}
      EndIf
      If(CLUTTER)
	Galerkin{[-Conj[{Ue}], {Uback}];
	  In TRM; Jacobian JVol; Integration I1;}	
      EndIf
    }
  }
}//End Formulation

// Resolution
// ==========
Resolution{
  {Name TR;
    System{
      If(CLUTTER)
	{Name Emission; NameOfFormulation Emission; Type Complex; }
      EndIf
      {Name BackProp; NameOfFormulation BackProp; Type Complex; }
    }
    Operation{
      For ik In {0:nk-1}
	Evaluate[(k_min+ik*stepK) #10];
	If(CLUTTER)
	  Generate[Emission]; Solve[Emission]; 
	EndIf
	Generate[BackProp]; Solve[BackProp]; 
	If(ik == 0)
	  PostOperation[InitField];
	EndIf
	If(ik>0)
	  PostOperation[SaveUback];
	  PostOperation[StackUback];
	EndIf
      EndFor
    }
  }
  // Empty resolution (to display functions for example).
  // ============================================
  {Name Empty;
    System{
      {Name Direct; NameOfFormulation BackProp; Type Complex; }
    }
    Operation{
    }
  }
}

//Postprocessing
// =============
PostProcessing{
  {Name InitField; NameOfFormulation BackProp;
    Quantity {
      {Name Uback; Value {Local { [{Uback}] ; In Propagation_Domain; Jacobian JVol; }}}
    }
  }

  {Name SaveUback; NameOfFormulation BackProp;
    Quantity {
      {Name OldUback; Value {Local { [ComplexScalarField[XYZ[]]{0}] ; In Propagation_Domain; Jacobian JVol; }}}
    }
  }

  {Name StackUback; NameOfFormulation BackProp;
    Quantity {
      {Name Uback; Value {Local { [{Uback} + ComplexScalarField[XYZ[]]{1}] ; In Propagation_Domain; Jacobian JVol; }}}
    }
  }
  
  {Name Uback; NameOfFormulation BackProp;
    Quantity {
      {Name Uback; Value {Local { [ComplexScalarField[XYZ[]]{0}] ; In Propagation_Domain; Jacobian JVol; }}}
      {Name Uback_abs; Value {Local { [Norm[ComplexScalarField[XYZ[]]{0}]] ; In Propagation_Domain; Jacobian JVol; }}}
    }
  }

  // Functions (associated with Empty resolution)
  {Name Functions; NameOfFormulation BackProp;
    Quantity {
      {Name Green2D; Value {Local { [Green2D[]] ; In AllDomains; Jacobian JVol; }}} 
      {Name Green2DNorm; Value {Local { [Norm[Green2D[]]] ; In AllDomains; Jacobian JVol; }}} 
      {Name SigmaX; Value {Local { [Norm[SigmaX[]]] ; In AllDomains; Jacobian JVol; }}} 
      {Name SigmaY; Value {Local { [Norm[SigmaY[]]] ; In AllDomains; Jacobian JVol; }}} 
    }
  }
} // End Postprocessing.

// Post operation
// ===============
PostOperation{
  {Name InitField; NameOfPostProcessing InitField ;
    Operation {
      Print [Uback, OnElementsOf Propagation_Domain, StoreInField 0];
    }
  }

  {Name SaveUback; NameOfPostProcessing SaveUback ;
    Operation {
      Print [OldUback, OnElementsOf Propagation_Domain, StoreInField 1];
    }
  }

  {Name StackUback; NameOfPostProcessing StackUback ;
    Operation {
      Print [Uback, OnElementsOf Propagation_Domain, StoreInField 0];
    }
  }

  {Name Uback; NameOfPostProcessing Uback ;
    Operation {
      Print [Uback, OnElementsOf Propagation_Domain, File "Uback.pos"];
      Print [Uback_abs, OnElementsOf Propagation_Domain, File "Uback_abs.pos"];
    }
  }

  // Display functions.
  {Name Functions; NameOfPostProcessing Functions ;
    Operation {
      Print [Green2D, OnElementsOf Propagation_Domain, File "fun_Green2D.pos"];
      Print [Green2DNorm, OnElementsOf Propagation_Domain, File "fun_Green2DNorm.pos"];
      Print [SigmaX, OnElementsOf AllDomains, File "fun_SigmaX.pos"];
      Print [SigmaY, OnElementsOf AllDomains, File "fun_SigmaY.pos"];
    }
  }
  
}
