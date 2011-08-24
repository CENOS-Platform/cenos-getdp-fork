// GetDP - Copyright (C) 1997-2011 P. Dular, C. Geuzaine
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <getdp@geuz.org>.

#include <math.h>
#include "GetDPConfig.h"
#include "Message.h"

#if defined(HAVE_GSL)

#include <gsl/gsl_version.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_linalg.h>

void new_handler(const char *reason, const char *file, int line,
                 int gsl_errno)
{
  Message::Error("GSL: %s (%s, line %d)", reason, file, line);
}

int check_gsl()
{
  // check version
  int major, minor;
  if(!sscanf(gsl_version, "%d.%d", &major, &minor)){
    Message::Fatal("Cannot retreive GSL version");
    return 0;
  }
  if(major < 1 || (major == 1 && minor < 2)) {
    Message::Error("Your GSL version (%d.%d.X) has a bug in the singular value",
        major, minor);
    Message::Error("decomposition code. Please upgrade to version 1.2 or above.");
    Message::Fatal("You can download the GSL from http://sources.redhat.com/gsl/");
    return 0;
  }
  // set new error handler
  gsl_set_error_handler(&new_handler);
  return 1;
}

#else

#include "nrutil.h"

void dsvdcmp(double **a, int m, int n, double w[], double **v);

int check_gsl()
{
  return 1;
}

#endif

#if !defined(HAVE_GMSH)

void invert_singular_matrix3x3(double MM[3][3], double II[3][3])
{
  int i, j, k, n = 3;
  double TT[3][3];

  for(i = 1; i <= n; i++) {
    for(j = 1; j <= n; j++) {
      II[i - 1][j - 1] = 0.0;
      TT[i - 1][j - 1] = 0.0;
    }
  }

#if defined(HAVE_GSL)
  gsl_matrix *M = gsl_matrix_alloc(3, 3);
  gsl_matrix *V = gsl_matrix_alloc(3, 3);
  gsl_vector *W = gsl_vector_alloc(3);
  gsl_vector *TMPVEC = gsl_vector_alloc(3);
  for(i = 1; i <= n; i++) {
    for(j = 1; j <= n; j++) {
      gsl_matrix_set(M, i - 1, j - 1, MM[i - 1][j - 1]);
    }
  }
  gsl_linalg_SV_decomp(M, V, W, TMPVEC);
  for(i = 1; i <= n; i++) {
    for(j = 1; j <= n; j++) {
      double ww = gsl_vector_get(W, i - 1);
      if(fabs(ww) > 1.e-16) {   //singular value precision
        TT[i - 1][j - 1] += gsl_matrix_get(M, j - 1, i - 1) / ww;
      }
    }
  }
  for(i = 1; i <= n; i++) {
    for(j = 1; j <= n; j++) {
      for(k = 1; k <= n; k++) {
        II[i - 1][j - 1] +=
          gsl_matrix_get(V, i - 1, k - 1) * TT[k - 1][j - 1];
      }
    }
  }
  gsl_matrix_free(M);
  gsl_matrix_free(V);
  gsl_vector_free(W);
  gsl_vector_free(TMPVEC);
#else
  double **M = dmatrix(1, 3, 1, 3);
  double **V = dmatrix(1, 3, 1, 3);
  double *W = dvector(1, 3);
  for(i = 1; i <= n; i++) {
    for(j = 1; j <= n; j++) {
      M[i][j] = MM[i - 1][j - 1];
    }
  }
  dsvdcmp(M, n, n, W, V);
  for(i = 1; i <= n; i++) {
    for(j = 1; j <= n; j++) {
      if(fabs(W[i]) > 1.e-16) { //singular value precision
        TT[i - 1][j - 1] += M[j][i] / W[i];
      }
    }
  }
  for(i = 1; i <= n; i++) {
    for(j = 1; j <= n; j++) {
      for(k = 1; k <= n; k++) {
        II[i - 1][j - 1] += V[i][k] * TT[k - 1][j - 1];
      }
    }
  }
  free_dmatrix(M, 1, n, 1, n);
  free_dmatrix(V, 1, n, 1, n);
  free_dvector(W, 1, n);
#endif
}

#endif
