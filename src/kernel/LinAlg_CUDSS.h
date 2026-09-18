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
//
// Consecutive calls with the same sparsity pattern reuse the symbolic
// factorization and the device buffers behind it, so a nonlinear loop pays for
// the analysis once instead of once per iteration. A different pattern rebuilds
// the context, releasing the previous one first, so only one system's worth of
// device memory is ever held.
int GetDP_CUDSS_SolveComplex(int n, int nnz, const int *rowPtr,
                             const int *colInd, const double *values,
                             const double *rhs, double *solution);

// Release that retained context, device allocations included. Must be called
// while CUDA is still up - GetDP does so from LinAlg_FinalizeSolver(). Safe
// when nothing was ever solved, and safe to call more than once.
void GetDP_CUDSS_Cleanup(void);

#ifdef __cplusplus
}
#endif

#endif
