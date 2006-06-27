#define RCSID "$Id: F_Math.c,v 1.14 2006-06-27 22:53:19 geuzaine Exp $"
/*
 * Copyright (C) 1997-2006 P. Dular, C. Geuzaine
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 * Please report all bugs and problems to <getdp@geuz.org>.
 */

#include "GetDP.h"
#include "Data_DefineE.h"
#include "F_Function.h"
#include "GeoData.h"
#include "Get_Geometry.h"
#include "Cal_Value.h" 
#include "CurrentData.h"
#include "Numeric.h"

/* ------------------------------------------------------------------------ */
/*  Warning: the pointers A and V can be identical. You must                */
/*           use temporary variables in your computations: you can only     */
/*           affect to V at the very last time (when you're sure you will   */
/*           not use A any more).                                           */
/* ------------------------------------------------------------------------ */

#define F_ARG    struct Function * Fct, struct Value * A, struct Value * V


/* ------------------------------------------------------------------------ */
/*  C math functions (scalar, 1 argument, imaginary part set to zero)       */
/* ------------------------------------------------------------------------ */

#define scalar_real_1_arg(func,string)					\
  int     k;								\
									\
  GetDP_Begin("F_" string);						\
									\
  if(A->Type != SCALAR)							\
    Msg(GERROR, "Non scalar argument for function '" string "'");	\
									\
  V->Val[0] = func(A->Val[0]) ;						\
  if (Current.NbrHar != 1){						\
    V->Val[MAX_DIM] = 0. ;						\
    for (k = 2 ; k < Current.NbrHar ; k += 2)				\
      V->Val[MAX_DIM*k] = V->Val[MAX_DIM*(k+1)] = 0. ;			\
  }									\
  V->Type = SCALAR;							\
									\
  GetDP_End ;

void  F_Exp   (F_ARG) { scalar_real_1_arg (exp,  "Exp")   }
void  F_Log   (F_ARG) { scalar_real_1_arg (log,  "Log")   }
void  F_Log10 (F_ARG) { scalar_real_1_arg (log10,"Log10") }
void  F_Sqrt  (F_ARG) { scalar_real_1_arg (sqrt, "Sqrt")  }
void  F_Sin   (F_ARG) { scalar_real_1_arg (sin,  "Sin")   }
void  F_Asin  (F_ARG) { scalar_real_1_arg (asin, "Asin")  }
void  F_Cos   (F_ARG) { scalar_real_1_arg (cos,  "Cos" )  }
void  F_Acos  (F_ARG) { scalar_real_1_arg (acos, "Acos")  }
void  F_Tan   (F_ARG) { scalar_real_1_arg (tan,  "Tan")   }
void  F_Atan  (F_ARG) { scalar_real_1_arg (atan, "Atan")  }
void  F_Sinh  (F_ARG) { scalar_real_1_arg (sinh, "Sinh")  }
void  F_Cosh  (F_ARG) { scalar_real_1_arg (cosh, "Cosh")  }
void  F_Tanh  (F_ARG) { scalar_real_1_arg (tanh, "Tanh")  }
void  F_Fabs  (F_ARG) { scalar_real_1_arg (fabs, "Fabs")  }
void  F_Floor (F_ARG) { scalar_real_1_arg (floor,"Floor") }
void  F_Ceil  (F_ARG) { scalar_real_1_arg (ceil, "Ceil")  }

#undef scalar_real_1_arg


/* ------------------------------------------------------------------------ */
/*  C math functions (scalar, 2 arguments, imaginary part set to zero)      */
/* ------------------------------------------------------------------------ */

#define scalar_real_2_arg(func,string)					\
  int     k;								\
									\
  GetDP_Begin("F_" string);						\
									\
  if(A->Type != SCALAR || (A+1)->Type != SCALAR)			\
    Msg(GERROR, "Non scalar argument(s) for function '" string "'");	\
									\
  V->Val[0] = func(A->Val[0], (A+1)->Val[0]) ;				\
  if (Current.NbrHar != 1){						\
    V->Val[MAX_DIM] = 0. ;						\
    for (k = 2 ; k < Current.NbrHar ; k += 2)				\
      V->Val[MAX_DIM*k] = V->Val[MAX_DIM*(k+1)] = 0. ;			\
  }									\
  V->Type = SCALAR;							\
									\
  GetDP_End ;

void  F_Atan2 (F_ARG) { scalar_real_2_arg (atan2, "Atan2")   }
void  F_Fmod  (F_ARG) { scalar_real_2_arg (fmod, "Fmod")     }

#undef scalar_real_2_arg

/* ------------------------------------------------------------------------ */
/*  Sign function                                                           */
/* ------------------------------------------------------------------------ */

void  F_Sign (F_ARG) { 
  int     k, n;
  double  x;   

  GetDP_Begin("Sign");
  if(A->Type != SCALAR)
    Msg(GERROR, "Non scalar argument for function 'Sign'");
  x = A->Val[0];

  if(x > 0)
    V->Val[0] = 1.;
  else if(x < 0)
    V->Val[0] = -1.;
  else
    V->Val[0] = 0.;

  if (Current.NbrHar != 1){
    V->Val[MAX_DIM] = 0. ;
    for (k = 2 ; k < Current.NbrHar ; k += 2)
      V->Val[MAX_DIM*k] = V->Val[MAX_DIM*(k+1)] = 0. ;
  }
  V->Type = SCALAR;
  GetDP_End ;
}

/* ------------------------------------------------------------------------ */
/*  Bessel functions jn, yn and their derivatives                           */
/* ------------------------------------------------------------------------ */

void  F_Jn (F_ARG) { 
  int     k, n;
  double  x;   

  GetDP_Begin("Jn");
  if(A->Type != SCALAR || (A+1)->Type != SCALAR)
    Msg(GERROR, "Non scalar argument(s) for function 'Jn'");
  n = (int)A->Val[0];
  x = (A+1)->Val[0];

  V->Val[0] = jn(n, x);

  if (Current.NbrHar != 1){
    V->Val[MAX_DIM] = 0. ;
    for (k = 2 ; k < Current.NbrHar ; k += 2)
      V->Val[MAX_DIM*k] = V->Val[MAX_DIM*(k+1)] = 0. ;
  }
  V->Type = SCALAR;
  GetDP_End ;
}

void  F_Yn (F_ARG) { 
  int     k, n;
  double  x;   

  GetDP_Begin("Yn");
  if(A->Type != SCALAR || (A+1)->Type != SCALAR)
    Msg(GERROR, "Non scalar argument(s) for function 'Yn'");
  n = (int)A->Val[0];
  x = (A+1)->Val[0];

  V->Val[0] = yn(n, x);

  if (Current.NbrHar != 1){
    V->Val[MAX_DIM] = 0. ;
    for (k = 2 ; k < Current.NbrHar ; k += 2)
      V->Val[MAX_DIM*k] = V->Val[MAX_DIM*(k+1)] = 0. ;
  }
  V->Type = SCALAR;
  GetDP_End ;
}

double dBessel(double *tab, int n, double x){
  if(n == 0){
    return - tab[1];
  }
  else{
    return tab[n-1] - (double)n/x * tab[n];
  }
}

void  F_dJn (F_ARG) {
  int     k, n;
  double  x, *jntab;   

  GetDP_Begin("dJn");
  if(A->Type != SCALAR || (A+1)->Type != SCALAR)
    Msg(GERROR, "Non scalar argument(s) for function 'dJn'");
  n = (int)A->Val[0];
  x = (A+1)->Val[0];

  jntab = (double*)Malloc((n + 2) * sizeof(double));
  for(k = 0; k < n + 2; k++){
    jntab[k] = jn(k, x);
  }
  V->Val[0] = dBessel(jntab, n, x);
  Free(jntab);

  if (Current.NbrHar != 1){
    V->Val[MAX_DIM] = 0. ;
    for (k = 2 ; k < Current.NbrHar ; k += 2)
      V->Val[MAX_DIM*k] = V->Val[MAX_DIM*(k+1)] = 0. ;
  }
  V->Type = SCALAR;
  GetDP_End ;
}

void  F_dYn (F_ARG) { 
  int     k, n;
  double  x, *yntab;   

  GetDP_Begin("dYn");
  if(A->Type != SCALAR || (A+1)->Type != SCALAR)
    Msg(GERROR, "Non scalar argument(s) for function 'dYn'");
  n = (int)A->Val[0];
  x = (A+1)->Val[0];

  yntab = (double*)Malloc((n + 2) * sizeof(double));
  for(k = 0; k < n + 2; k++){
    yntab[k] = yn(k, x);
  }
  V->Val[0] = dBessel(yntab, n, x);
  Free(yntab);

  if (Current.NbrHar != 1){
    V->Val[MAX_DIM] = 0. ;
    for (k = 2 ; k < Current.NbrHar ; k += 2)
      V->Val[MAX_DIM*k] = V->Val[MAX_DIM*(k+1)] = 0. ;
  }
  V->Type = SCALAR;
  GetDP_End ;
}

#undef F_ARG

