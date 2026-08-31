// GetDP - Copyright (C) 1997-2026 P. Dular and C. Geuzaine, University of Liege
//
// See the LICENSE.txt file for license information. Please report all
// issues on https://gitlab.onelab.info/getdp/getdp/issues.

#ifndef LIN_ALG_CUDSS_H
#define LIN_ALG_CUDSS_H

#ifdef __cplusplus
extern "C" {
#endif

// GPU direct sparse solve of A*x = rhs via NVIDIA cuDSS, double-complex
// scalars (matching this build's PetscScalar). A is given as 0-based CSR.
// values/rhs/solution are interleaved (re,im) pairs, i.e. binary-compatible
// with cuDoubleComplex / std::complex<double> / PETSc's PetscScalar in a
// complex build - callers can pass such arrays directly, no repacking.
//
// Returns 0 on success, nonzero if anything failed (no GPU, driver error,
// cuDSS error) - caller should fall back to the normal CPU solve path.
int GetDP_CUDSS_SolveComplex(int n, int nnz, const int *rowPtr,
                             const int *colInd, const double *values,
                             const double *rhs, double *solution);

#ifdef __cplusplus
}
#endif

#endif
