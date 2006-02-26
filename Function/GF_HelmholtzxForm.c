#define RCSID "$Id: GF_HelmholtzxForm.c,v 1.8 2006-02-26 00:42:53 geuzaine Exp $"
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
 *
 * Contributor(s):
 *   Ruth Sabariego
 */

#include "GetDP.h" 
#include "Data_Active.h"
#include "BF_Function.h"
#include "CurrentData.h"
#include "Data_DefineE.h"
#include "Numeric.h"
#include "Numeric_F.h"
#include "Quadrature.h"


#define F_ARG2						\
  struct Element * Element, struct Function * Fct,	\
  void  (*xFunctionBF) (), int NumEntity, 		\
  double x, double y, double z, struct Value * Val

#define CAST  void(*)()

#define MAX_NODES        6 
#define EPSILON          1.e-10
#define EPSILON2         1.e-20

/* this is a hack... */
#define RADIUS           0.154797 

/* ------------------------------------------------------------------------ */
/*  G F _ Helmholtzx F o r m                                           */
/* ------------------------------------------------------------------------ */

void GF_HelmholtzxForm (F_ARG2) {
  
  double   xs[MAX_NODES], ys[MAX_NODES], zs[MAX_NODES], u[3], v[3], n[3], l2[2], xl, yl, zl ;
  int      i, j = 1 ;
  int      i_IntPoint, NGT_Points = 7;
  double   a, b, c, d, us, vs, ws, usi, vsi, wsi, Ri, wghti;
  double   s0m, s0p, s1m, s1p, s2m, s2p, t00, t10, t20, t0m, t0p, t1p, r00, r10, r20, r0p, r0m, r1p, f0, f1, f2, B0, B1, B2 ;
  double   IS, valr, vali ;

  GetDP_Begin("GF_HelmholtzxForm");
  
  switch ((int)Fct->Para[0]) {

  case _3D :
    
    switch (Element->ElementSource->Type) {
      
    case TRIANGLE :
    case QUADRANGLE :  
      xs[0] = Element->ElementSource->x[0] ; ys[0] = Element->ElementSource->y[0] ;
      zs[0] = Element->ElementSource->z[0] ;
      xs[1] = Element->ElementSource->x[1] ; ys[1] = Element->ElementSource->y[1] ;
      zs[1] = Element->ElementSource->z[1] ;
      xs[2] = Element->ElementSource->x[2] ; ys[2] = Element->ElementSource->y[2] ;
      zs[2] = Element->ElementSource->z[2] ;
      
      valr = 0. ;
      vali = 0. ;
      IS = 0. ;
      printf("hola \n");

      if (Element->ElementSource->Type ==  QUADRANGLE) {
	     xs[3] = Element->ElementSource->x[3] ; ys[3] = Element->ElementSource->y[3] ;
	     zs[3] = Element->ElementSource->z[3] ;
	     j = 0 ;
       };
      
      for(i = j; i < 2; i++){      
	    /* triangle side lengths */
	    a = sqrt(SQU(xs[1]-xs[0]) + SQU(ys[1]-ys[0]) +  SQU(zs[1]-zs[0]));
	    b = sqrt(SQU(xs[2]-xs[1]) + SQU(ys[2]-ys[1]) +  SQU(zs[2]-zs[1]));
	    c = sqrt(SQU(xs[2]-xs[0]) + SQU(ys[2]-ys[0]) +  SQU(zs[2]-zs[0]));
	    
	    /* local system (u,v,w) centered at (xs[0],ys[0],zs[0]) */
	    u[0] = (xs[1]-xs[0])/a; u[1] = (ys[1]-ys[0])/a; u[2] = (zs[1]-zs[0])/a;
      
	    /* triangle normal */
	    Geo_CreateNormal(Element->ElementSource->Type,xs,ys,zs,n);
                  
	    v[0] = n[1]*u[2]-n[2]*u[1]; v[1] = n[2]*u[0]-n[0]*u[2]; v[2] = n[0]*u[1]-n[1]*u[0];

	    l2[0] = (xs[2]-xs[0])*u[0] + (ys[2]-ys[0])*u[1] + (zs[2]-zs[0])*u[2]; /*u2 coordinate*/
	    l2[1] = (xs[2]-xs[0])*v[0] + (ys[2]-ys[0])*v[1] + (zs[2]-zs[0])*v[2];  /*triangle height, v2 coordinate*/
     
	    /* local coordinates of the observation point (xl, yl, zl)*/ 
	    xl = u[0] * (x-xs[0]) +  u[1] * (y-ys[0]) + u[2] * (z-zs[0]);
	    yl = v[0] * (x-xs[0]) +  v[1] * (y-ys[0]) + v[2] * (z-zs[0]);
	    zl = n[0] * (x-xs[0]) +  n[1] * (y-ys[0]) + n[2] * (z-zs[0]);
	    
	    s0m = -( (a-xl) * (a-l2[0]) + yl*l2[1] ) / b;   s0p = s0m + b;
	    s1p = ( xl * l2[0] + yl * l2[1] ) / c;   s1m = s1p - c; s2m = - xl; s2p = a - xl;
    
	    /*distance observation point projection on triangle plane to triangle local vertices*/
	    t00 = (yl * (l2[0]-a) + l2[1] * (a-xl)) / b;
	    t10 = (xl * l2[1] - yl * l2[0]) / c;
	    t20 = yl;
	    t0m = sqrt((a-xl)*(a-xl) + yl*yl); 
	    t0p = sqrt((l2[0]-xl)*(l2[0]-xl) + (l2[1]-yl)*(l2[1]-yl)); 
	    t1p = sqrt(xl*xl + yl*yl);

	    /* minimum distances from the observation point to each triangle side*/
	    r00 = sqrt(SQU(t00) + SQU(zl));
	    r10 = sqrt(SQU(t10) + SQU(zl));
	    r20 = sqrt(SQU(t20) + SQU(zl));
	    
	    /* distances from observation point to the vertices*/
	    r0p = sqrt(SQU(t0p) + SQU(zl));      
	    r0m = sqrt(SQU(t0m) + SQU(zl));
	    r1p = sqrt(SQU(t1p) + SQU(zl));

	    /* intermediate functions */
           	   	    
	    f0 = r00 <= EPSILON ? 0 : log((r0p + s0p) / (r0m + s0m));
	    f1 = r10 <= EPSILON ? 0 : log((r1p + s1p) / (r0p + s1m));
	    f2 = r20 <= EPSILON ? 0 : log((r0m + s2p) / (r1p + s2m));
	    
	    B0 = fabs(t00) <= EPSILON ? 0 : atan(t00*s0p/(SQU(r00)+fabs(zl)*r0p))-atan(t00*s0m/(SQU(r00)+fabs(zl)*r0m));
	    B1 = fabs(t10) <= EPSILON ? 0 : atan(t10*s1p/(SQU(r10)+fabs(zl)*r1p))-atan(t10*s1m/(SQU(r10)+fabs(zl)*r0p));
	    B2 = fabs(t20) <= EPSILON ? 0 : atan(t20*s2p/(SQU(r20)+fabs(zl)*r0m))-atan(t20*s2m/(SQU(r20)+fabs(zl)*r1p));
	    d = a * l2[1]; /* Double aire a cause de normalization */
	   
	    IS +=  ONE_OVER_TWO_PI * (-fabs(zl)*(B0+B1+B2) + t00*f0+t10*f1+t20*f2)/d; /* 1/r integral solution*/
	    
	    /* Gauss Numerical Integration of (exp(Fct->Para[1]*R)-1)/R */
	    for (i_IntPoint = 1; i_IntPoint <= NGT_Points; i_IntPoint++){
	      Gauss_Triangle(NGT_Points,i_IntPoint,&us,&vs,&ws,&wghti);
	      usi = u[0]*us + u[1]*vs + u[2]*ws ;
	      vsi = v[0]*us + v[1]*vs + v[2]*ws ;
	      wsi = n[0]*us + n[1]*vs + n[2]*ws ;
	      Ri = sqrt( SQU(xl-usi) + SQU(yl-vsi) + SQU(zl-wsi) ) ;
	    
	      valr += Ri > EPSILON ? (wghti*(cos(Fct->Para[1]*Ri)-1)/Ri): 0 ;
	      vali += Ri > EPSILON ? (-wghti*sin(Fct->Para[1]*Ri)/Ri): (-wghti * Fct->Para[1]); 
	    }
		
	    valr = d * valr/2;
	    vali = d * vali/2;

	    Val->Val[0] = IS + valr ;
	    Val->Val[MAX_DIM] = vali ; /* Imaginary part. Numerical integral */
	    
	    if (j == 0){
	      xs[1] = xs[2]; ys[1] = ys[2]; zs[1] = zs[2];
	      xs[2] = xs[3]; ys[2] = ys[3]; zs[2] = zs[3];
	    }
      }
      if (j == 0){ Val->Val[0] = (Val->Val[0])/2; }
      Val->Type = SCALAR;
      break ;
    
      default :
	Msg(GERROR, "Unknown Element Type (%s) for 'GF_HelmholtzxForm'",
	    Get_StringForDefine(Element_Type, Element->ElementSource->Type));
      }
      break ;

    default :
      Msg(GERROR, "Unknown Dimension (%d) for 'GF_HelmholtzxForm'", 
	  (int)Fct->Para[0]);
    
    }
    GetDP_End ;
}

#undef F_ARG2
#undef CAST









