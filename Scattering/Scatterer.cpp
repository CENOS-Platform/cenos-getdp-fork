// $Id: Scatterer.cpp,v 1.8 2002-05-02 01:13:02 geuzaine Exp $

#include "Utils.h"
#include "Tools.h"
#include "Scatterer.h"
#include "Function.h"

int fcmp_CPoint(const void * a, const void * b) {
  double cmp ;
  
  cmp = ((CPoint*)a)->val - ((CPoint*)b)->val ;
  if      (cmp > 1.e-16)  return  1 ;
  else if (cmp < -1.e-16) return -1 ;
  else                    return  0 ;
}

// Parametric definition of the scatterers. This is awfull c++, I
// know, thanks :-)

void Scatterer::x(double u, double *x){
  switch(type){
  case CIRCLE :
    x[0] = cos(u); 
    x[1] = sin(u); 
    x[2] = 0.; 
    break;
  case ELLIPSE :
    x[0] = a*cos(u); 
    x[1] = b*sin(u); 
    x[2] = 0.; 
    break;
  case DROP :
    x[0] = 2.*sin(u/2.); 
    x[1] = -sin(u); 
    x[2] = 0.; 
    break;
  }
}

void Scatterer::dx(double u, double *x){ 
  switch(type){
  case CIRCLE :
    x[0] = -sin(u); 
    x[1] = cos(u); 
    x[2] = 0.; 
    break;
  case ELLIPSE :
    x[0] = -a*sin(u); 
    x[1] = b*cos(u); 
    x[2] = 0.; 
    break;
  case DROP :
    x[0] = cos(u/2.); 
    x[1] = -cos(u); 
    x[2] = 0.; 
    break;
  }
}

void Scatterer::polar(double u, double *r, double *dr){
  double cart[3], dcart[3];
  x(u,cart);
  *r = sqrt(cart[0]*cart[0]+cart[1]*cart[1]);
  dx(u,dcart);
  *dr = (cart[0]*dcart[0]+cart[1]*dcart[1]) / *r;
}

// Compute (hum!) the target point

void Scatterer::singularPoint(double t0, List_T *pts){
  CPoint pt;

  pt.degree = 1;
  pt.val = t0;
  List_Insert(pts, &pt, fcmp_CPoint);
}


//  Compute all critical points (i.e. for which gradient of the total
//  phase of the integral equation vanishes).

void newt (double x[], int n, int *check, void (*vecfunc) (int, double[], double[]));

static Scatterer *TheScat;

void phaseGradient(int n, double *in, double *out){
  out[1] = cos(10*in[1]); return;

  double theta0 = TheScat->targetPoint, theta = in[1];
  double r0, dr0, r, dr, phip;

  TheScat->polar(theta,&r,&dr);
  TheScat->polar(theta0,&r0,&dr0);

  phip = (r*dr - r0*dr*cos(theta-theta0) + r0*r*sin(theta-theta0)) /
    sqrt(r0*r0 + r*r - 2*r0*r*cos(theta-theta0)) +
    dr*cos(theta) - r*sin(theta) ;

  out[1] = phip;

  //printf("in=%g   out=%g\n", in[1], out[1]);

}

void Scatterer::criticalPoints(double t0, double k[3], List_T *pts){
  int i, n, check;
  CPoint pt;
  double tmp[2], theta;

  pt.degree = 2;

  switch(type){

  case CIRCLE :
    // in the case of a circle, they are given in closed form by, for
    // an integer n:
    //
    //  0 <= t-t0 = Pi - 2*t0 + 4*n*PI <= 2*PI
    //  0 <= t-t0 = (PI-2*t0)/3 + 4/3*PI*n

    if(k[1] || k[2])
      Msg(ERROR, "Critical point computation not done in the general case");

    for(n=-2 ; n<=2 ; n++){
      pt.val = PI-t0+4.*n*PI;
      if((pt.val-t0>=0) && (pt.val-t0<=TWO_PI)){
	while(pt.val > TWO_PI) pt.val-=TWO_PI;
	List_Insert(pts, &pt, fcmp_CPoint);
      }
      pt.val = (PI+t0)/3.+4./3.*n*PI;
      if((pt.val-t0>=0) && (pt.val-t0<=TWO_PI)){
	while(pt.val > TWO_PI) pt.val-=TWO_PI;
	List_Insert(pts, &pt, fcmp_CPoint);
      }
    }
    break;

  default :
    // store the critical points during the first gmres iteration

    theta = 0.;
    targetPoint = t0;

    TheScat = this;

    for(i=0; i<1000; i++){
      tmp[1] = theta;
      newt(tmp, 1, &check, phaseGradient);

      tmp[1] = GetInInterval(tmp[1], 0., TWO_PI);

      printf("theta = %g sol = %.16g check = %d\n", theta, tmp[1], check);

      theta += TWO_PI/1000.;
    }
    Msg(ERROR, "Unknown type of scatterer for critical point computation");
    break;

  }
}


void Scatterer::shadowingPoints(double t, double shift, double k[3], List_T *pts){
  CPoint pt;

  pt.degree = 3;
   
  switch(type){

  case CIRCLE :
    if(k[1] || k[2])
      Msg(ERROR, "Shadowing point computation not done in the general case");

    pt.val = PI/2. + shift ;
    List_Insert(pts, &pt, fcmp_CPoint);

    pt.val = 3*PI/2. - shift ;
    List_Insert(pts, &pt, fcmp_CPoint);
    break;

  default :
    Msg(ERROR, "Unknown type of scatterer for shadowing point computation");
    break;

  }
}
  

void Scatterer::printPoints(double t, List_T *pts){
  static int first=1;
  static FILE *fp;
  double coord[3];
  CPoint pt;
  int i;

  if(first){
    first = 0;
    fp = fopen("points.pos", "w");
    fprintf(fp, "View.PointSize = 10;\n");
  }

  fprintf(fp, "View \"target=%g\" {\n", t);
  for(i=0; i<List_Nbr(pts); i++){
    List_Read(pts,i,&pt);
    x(pt.val,coord);
    fprintf(fp, "SP(%g,%g,%g){%d};\n",coord[0],coord[1],coord[2],
	    pt.degree);
  }
  fprintf(fp, "};\n");

}
