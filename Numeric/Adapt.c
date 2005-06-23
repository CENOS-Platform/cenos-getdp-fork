#define RCSID "$Id: Adapt.c,v 1.15 2005-06-23 01:45:02 geuzaine Exp $"
/*
 * Copyright (C) 1997-2005 P. Dular, C. Geuzaine
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
 *   Jean-Francois Remacle
 */

#include "GetDP.h"
#include "Data_Element.h"
#include "Adapt.h"
#include "Numeric.h"

#define TOL     1.e-08
#define MAXDEG  999

static int     NN ;
static double  MINH , *ERR , *HH , *PP , E0, DIM ;

/* ------------------------------------------------------------------------ */
/*  f XXX                                                                   */
/* ------------------------------------------------------------------------ */

/* h-type version 1 : minimize the number of elements while keeping a
   given global error */

double fH1 (double l){
  int i;
  double val1 = 0.0, val2 = 0.0;

  GetDP_Begin("fH1");

  for(i = 1 ; i <= NN ; i++){
    val1 += pow(2.*l*DSQR(ERR[i])*PP[i]/DIM, DIM/(2.*PP[i]+DIM));
    val2 += DSQR(ERR[i]) * pow(2.*l*DSQR(ERR[i])*PP[i]/DIM, -2.*PP[i]/(2.*PP[i]+DIM));
  }

  GetDP_Return( -(val1 + l * (val2 - DSQR(E0))) );
}

/* h-type version 2 : minimize the error while keeping a given number
   of elements */

double fH2 (double l){
  int i;
  double val1 = 0.0, val2 = 0.0, qi;

  GetDP_Begin("fH2");

  for(i = 1 ; i <= NN ; i++){
    qi = pow(DIM*l/(2.*PP[i]*DSQR(ERR[i])), -DIM/(DIM+2.*PP[i]));
    val1 += DSQR(ERR[i]) * pow(qi, -2.*PP[i]/DIM);
    val2 += qi;
  }

  GetDP_Return( -(val1 + l * (val2 - E0)) );
}

/* p-type : minimize error by modifying the interpolation order vector */

double fP1 (double l){
  int i;
  double val1 = 0.0, val2 = 0.0, qi, e;

  GetDP_Begin("fP1");

  for(i = 1 ; i <= NN ; i++){
    e = ERR[i];
    if(e == 0.0) e=1.e-12;
    qi = - log(2.*l*log(HH[i]/MINH)*DSQR(e)) / log(HH[i]/MINH);
    val1 -= 0.5 * qi;
    val2 += pow(HH[i]/MINH, qi) * DSQR(e);
  }

  GetDP_Return( -(val1 + l * (val2 - DSQR(E0))) );
}


/* ------------------------------------------------------------------------ */
/*  A d a p t                                                               */
/* ------------------------------------------------------------------------ */

double min1d (int method, double (*funct)(double), double *xmin){
  double xx, fx, fb, fa, bx, ax;

  GetDP_Begin("min1d");

  switch(method){
  case H1: 
  case P1: ax=1.e-12; xx=1.e2; break;
  default: ax=1.e-15; xx=1.e-12; break;
  }    
  mnbrak(&ax,&xx,&bx,&fa,&fx,&fb,funct);

  GetDP_Return( brent(ax,xx,bx,funct,TOL,xmin) );
}

/* Adapt return the constraint (N0 ou e0) for the optimzed problem */

double Adapt (int N,        /* Number of elements */
	      int method,   /* H1, H2, P1 or P2 */
	      int dim,      /* 2 or 3 */
	      double *err,  /* elementary errors */
	      double *h,    /* elementary mesh sizes */
	      double *p,    /* elementary exponents */
	      double e0     /* prescribed error or number of elements */
	      ){
  int i;
  double contr=0., pivrai, lambda, minf, qi, ri, pi, obj, obj2, minri=0., maxri=0.;
  double errmin=0., errmax=0.;

  GetDP_Begin("Adapt");

  h[N+1] = 1.0;
  p[N+1] = 1.0;

  NN  = N;
  ERR = err;
  HH  = h;
  PP  = p;
  E0  = e0;
  DIM = (double)dim;
  
  for(i = 1 ; i <= N ; i++){
    if(i == 1) 
      errmin = errmax = err[i];
    else{
      errmin = DMIN(errmin, err[i]);
      errmax = DMAX(errmax, err[i]);
    }
  }

  switch (method) {

  case H1 :
    minf = min1d (method, fH1, &lambda);
    obj = 0.0;
    for(i = 1 ; i <= N ; i++){
      qi = pow(2.*lambda*DSQR(err[i])*p[i]/DIM, DIM/(2.*p[i]+DIM));
      ri = pow(qi,1./DIM);
      if(i==1) minri = maxri = ri;
      if(err[i] == 0.0) ri = .5;
      minri = DMIN(minri, ri);
      maxri = DMAX(maxri, ri);
      obj += DSQR(err[i]) * pow(ri, -2.*p[i]) ; 
      h[i] = sqrt(2.) * h[i]/ri;
      p[i] = ri;
    }
    contr = fabs(minf);

    Msg(INFO, "H-Refinement 1, Error %g=>%g, Objective %g, Reduction Factor %g->%g",
	e0, sqrt(obj), -minf, minri, maxri);
    break;

  case H2 :
    minf = min1d (method, fH2, &lambda);
    obj = 0.0;
    for(i = 1 ; i <= N ; i++){
      qi = pow((DIM*lambda)/(2.*DSQR(err[i])*p[i]), -DIM/(DIM+2.*p[i]));
      ri = pow(qi, 1./DIM);
      if(i == 1) minri = maxri = ri;
      minri = DMIN(minri, ri);
      maxri = DMAX(maxri, ri);
      obj += pow(ri,DIM) ; 
      h[i] = h[i]/ri;
      p[i] = p[i];
    }
    contr = sqrt(fabs(minf));

    Msg(INFO, "H-Refinement 2, Elements %g=>%g, Objective %g, Reduction Factor %g->%g",
	e0, obj, 100. * sqrt(fabs(minf)), minri, maxri);
    break;

  case P1 :
    MINH = h[1];
    for(i = 1 ; i <= N ; i++) MINH =DMIN(h[i], MINH);
    MINH /= 2.;

    minf = min1d (method, fP1, &lambda);
    obj = obj2 = 0.0;
    for(i = 1 ; i <= N ; i++){
      qi = -log(2.*lambda*DSQR(err[i])*log(h[i]/MINH)) / log(h[i]/MINH);
      pi = p[i] - .5 * qi;
      pivrai = DMIN(DMAX(1., (double)(int)(pi+.99)), MAXDEG);
      obj2 += pow(h[i]/MINH, 2.*(p[i]-pivrai)) * DSQR(err[i]);
      obj += DSQR(err[i]) * pow(h[i]/MINH, qi); 
      h[i] = h[i];
      p[i] = pi;
    }
    contr = fabs(minf);

    Msg(INFO, "P-Refinement, Error %g=%g=>%g, Objective %g",
	e0, sqrt(obj), sqrt(obj2), minf);
    break;

  case P2 :
    minf = min1d (method, fH1, &lambda);
    break;

  default :
    Msg(ERROR, "Unknown adaptation method");
  }

  GetDP_Return(contr) ;
}
