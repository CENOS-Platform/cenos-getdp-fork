// $Id: FFT.cpp,v 1.14 2002-09-19 01:17:17 geuzaine Exp $

#include "Utils.h"
#include "FFT.h"
#include "Patch.h"

#define DEBUG_FFT 0

// packing order for F: [0,1...n/2-1,-n/2,...,-1]

#if 0 // Slow DFT

FFT::FFT(int n, Patch *patch){
  N = Nexp = n;
  expansionFactor = 1;
  fourierCoefs = new Complex[n];
}

FFT::~FFT(){
  delete[] fourierCoefs; 
}

void FFT::forward(Complex *f, Complex *F){
  int k, n;
  for(n=0 ; n<N ; n++){
    F[n] = 0.;
    for(k=0 ; k<N ; k++){
      F[n] += f[k] * (cos(-TWO_PI*n*k/(double)N)+I*sin(-TWO_PI*n*k/(double)N));
    }
  }

#if DEBUG_FFT
  printf("t = [ \n");
  for(k=0 ; k<N ; k++) printf("%.15e + (%.15ei)\n", f[k].real(), f[k].imag());
  printf("]; \n");

  printf("f = [ \n");
  for(n=0 ; n<N ; n++) printf("%.15e + (%.15ei)\n", F[n].real(), F[n].imag());
  printf("]; \n");
#endif
  
}

void FFT::backward(Complex *F, Complex *f){
  int k, n;
  for(k=0 ; k<N ; k++){
    f[k] = 0.;
    for(n=0 ; n<N ; n++){
      f[k] += 1./(double)N * F[n] * (cos(TWO_PI*n*k/(double)N)+I*sin(TWO_PI*n*k/(double)N));
    }
  }
}

void FFT::init(Complex *f){
  forward(f,fourierCoefs);
}

Complex FFT::eval(double t){ // t has to be in [0,2*Pi]
  int i, j;
  Complex sum;

  sum = 0.;
  for(i=0 ; i<N ; i++){
    if(i<=N/2) j = i;
    else j = -N+i;
    sum += (cos(j*t)+I*sin(j*t)) * fourierCoefs[i] / (double)N;
  }

#if DEBUG_FFT
  printf("eval(%.16g) = %.16g %.16g\n", t, sum.real(), sum.imag());
#endif

  return sum;
}

#else

FFT::FFT(int n, Patch *patch){
  int i;

  N = n;
  expansionFactor = 128;
  Nexp = expansionFactor*N;

  forwardPlan = fftw_create_plan(N,FFTW_FORWARD,FFTW_ESTIMATE);
  backwardPlan = fftw_create_plan(Nexp,FFTW_BACKWARD,FFTW_ESTIMATE);
  tmp1 = new fftw_complex[Nexp]; // partially overkill
  tmp2 = new fftw_complex[Nexp]; // partially overkill
  fourierCoefs = new Complex[Nexp];
  expandedVals = new Complex[Nexp+1]; // add endpoint

  double *nodes = new double[Nexp+1]; // add endpoint
  double a=patch->part->center - patch->part->epsilon;
  double b=patch->part->center + patch->part->epsilon;
  for(i=0; i<Nexp+1; i++){
    nodes[i] = patch->changeOfVars(a + i*(b-a)/(double)(Nexp), 0);
  }

#if DEBUG_FFT
  printf("nodes_orig = [\n");
  for(i=0; i<patch->nbdof ; i++){
    printf("%.16g\n", patch->nodes[i]);
  }
  printf("];");
  printf("nodes_interpol = [\n");
  for(i=0; i<Nexp+1 ; i++){
    printf("%.16g\n", nodes[i]);
  }
  printf("];");
#endif

  spline = new Spline(Nexp+1,nodes);
  delete [] nodes;
}

FFT::~FFT(){
  delete[] tmp1; 
  delete[] tmp2; 
  delete[] fourierCoefs; 
  delete[] expandedVals; 
  fftw_destroy_plan(forwardPlan);
  fftw_destroy_plan(backwardPlan);
}

void FFT::forward(Complex *f, Complex *F){
  int i;

  for(i=0; i<N; i++){
    tmp1[i].re = f[i].real();
    tmp1[i].im = f[i].imag();
  }

  fftw_one(forwardPlan,tmp1,tmp2);

  // padding with zeros in the expanded case
  // [0,  1...N/2-1,0,...0,  0,...0,-N/2,...,-1]

  for(i=0; i<Nexp; i++){
    if(i<N/2+1)
      F[i] = Complex(tmp2[i].re,tmp2[i].im);
    else if(i<Nexp-N/2)
      F[i] = 0.;
    else
      F[i] = Complex(tmp2[i-(Nexp-N)].re,tmp2[i-(Nexp-N)].im);
  }      

}

void FFT::backward(Complex *F, Complex *f){
  int i;
  for(i=0; i<Nexp; i++){
    tmp1[i].re = F[i].real()/(double)N;
    tmp1[i].im = F[i].imag()/(double)N;
  }
  fftw_one(backwardPlan,tmp1,tmp2);
  for(i=0; i<Nexp; i++){
    f[i] = Complex(tmp2[i].re,tmp2[i].im);
  }
}

void FFT::init(Complex *f){
  forward(f,fourierCoefs);
  backward(fourierCoefs,expandedVals);
  expandedVals[Nexp] = expandedVals[0]; // add end point

#if DEBUG_FFT
  int k;
  printf("t1 = [ \n");
  for(k=0 ; k<N ; k++) 
    printf("%.15e + (%.15ei)\n", f[k].real(), f[k].imag());
  printf("]; \n");

  printf("f1 = [ \n");
  for(k=0 ; k<Nexp ; k++) 
    printf("%.15e + (%.15ei)\n", fourierCoefs[k].real(), fourierCoefs[k].imag());
  printf("]; \n");

  printf("t2 = [ \n");
  for(k=0 ; k<Nexp+1 ; k++)
    printf("%.15e + (%.15ei)\n", expandedVals[k].real(), expandedVals[k].imag());
  printf("]; \n");
#endif

  spline->init(expandedVals);

#if DEBUG_FFT
  printf("t2reinterpol = [ \n");
  for(k=0; k<200; k++){
    double t=k*TWO_PI/200.;
    Complex sum = spline->eval(t);
    printf("%.15e + (%.15ei)\n", sum.real(), sum.imag());
  }
  printf("]; \n");
#endif

}

Complex FFT::eval(double t){
  Complex sum;

#define DIRECT_EVAL 0

#if DIRECT_EVAL // just for testing...
  int i, j;
  sum = 0.;
  for(i=0 ; i<Nexp ; i++){
    if(i<=Nexp/2) j = i;
    else j = -Nexp+i;
    sum += (cos(j*t)+I*sin(j*t)) * fourierCoefs[i] / (double)Nexp;
  }
#else
  sum = spline->eval(t);
#endif

#if DEBUG_FFT
  printf("eval(%.16g) = %.16g %.16g\n", t, sum.real(), sum.imag());
#endif

  return sum;
}

#endif
