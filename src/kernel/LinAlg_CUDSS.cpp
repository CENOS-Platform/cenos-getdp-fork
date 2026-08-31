// GetDP - Copyright (C) 1997-2026 P. Dular and C. Geuzaine, University of Liege
//
// See the LICENSE.txt file for license information. Please report all
// issues on https://gitlab.onelab.info/getdp/getdp/issues.
//
// GPU-accelerated direct sparse solve via NVIDIA cuDSS, called as an
// alternative to PETSc's own (CPU-only, MUMPS-based) direct solve.
//
// Deliberately built WITHOUT nvcc: cuDSS is a host-API call into a prebuilt
// NVIDIA library (it dispatches its own GPU kernels internally), so this
// file only needs cuda_runtime.h + cudss.h, both plain C-compatible headers,
// and links directly against cudart.lib/cudss.lib via the same mingw
// toolchain used for the rest of GetDP. This sidesteps every problem that
// PETSc's own --with-cuda=1 build path runs into (nvcc/cl.exe host-compiler
// requirements, CUDA-toolkit-vs-Linux directory layout mismatches,
// Thrust/CCCL, PETSc's __attribute__-decorated headers) — none of PETSc's
// own source is ever compiled by nvcc.
//
// Note: cudart.lib's static loader references MSVC's /GS stack-cookie
// runtime symbols, which mingw does not define. See gs_stub.c in this same
// directory for the (standard, well-known) fix.
//
// This file extracts CSR arrays from an already CPU-assembled PETSc Mat via
// ordinary PETSc CPU API (no CUDA involved in that step at all), uploads to
// the GPU, factors + solves via cuDSS, downloads the result. Matrix type is
// treated as general/nonsymmetric by default (safe for any FEM system;
// symmetric-specific optimization can be added later once this path is
// proven).

#include <cuda_runtime.h>
#include <cuComplex.h>
#include <cudss.h>
#include <cstdio>
#include <cstring>

#include "LinAlg_CUDSS.h"

#define CUDA_CHECK(call, msg)                                                 \
  do {                                                                        \
    cudaError_t _e = (call);                                                  \
    if(_e != cudaSuccess) {                                                   \
      fprintf(stderr, "[cuDSS] CUDA error in %s: %s\n", msg,                  \
             cudaGetErrorString(_e));                                        \
      goto cleanup;                                                           \
    }                                                                         \
  } while(0)

#define CUDSS_CHECK(call, msg)                                                \
  do {                                                                        \
    cudssStatus_t _s = (call);                                                \
    if(_s != CUDSS_STATUS_SUCCESS) {                                          \
      fprintf(stderr, "[cuDSS] cuDSS error in %s: status=%d\n", msg, (int)_s);\
      goto cleanup;                                                           \
    }                                                                         \
  } while(0)

// n            : number of rows/cols (square system)
// nnz          : number of nonzeros in the CSR matrix
// rowPtr       : CSR row offsets, length n+1, 0-based
// colInd       : CSR column indices, length nnz, 0-based
// values       : CSR values, length nnz, interleaved (re,im) pairs -
//                binary-compatible with cuDoubleComplex / std::complex<double>
// rhs          : right-hand side, length n, interleaved (re,im) pairs
// solution     : OUT, length n, interleaved (re,im) pairs - caller-allocated
//
// Returns 0 on success, nonzero on any failure (CUDA or cuDSS error, or no
// GPU present) - caller should fall back to the normal CPU direct solve.
int GetDP_CUDSS_SolveComplex(int n, int nnz, const int *rowPtr,
                             const int *colInd, const double *values,
                             const double *rhs, double *solution)
{
  int result = 1;

  int *d_rowPtr = nullptr, *d_colInd = nullptr;
  cuDoubleComplex *d_values = nullptr, *d_rhs = nullptr, *d_x = nullptr;
  cudaStream_t stream = nullptr;

  cudssHandle_t handle = nullptr;
  cudssConfig_t solverConfig = nullptr;
  cudssData_t solverData = nullptr;
  cudssMatrix_t A = nullptr, x = nullptr, b = nullptr;
  bool haveHandle = false, haveConfig = false, haveData = false;
  bool haveA = false, haveX = false, haveB = false;

  CUDA_CHECK(cudaMalloc((void **)&d_rowPtr, (size_t)(n + 1) * sizeof(int)),
            "cudaMalloc rowPtr");
  CUDA_CHECK(cudaMalloc((void **)&d_colInd, (size_t)nnz * sizeof(int)),
            "cudaMalloc colInd");
  CUDA_CHECK(cudaMalloc((void **)&d_values, (size_t)nnz * sizeof(cuDoubleComplex)),
            "cudaMalloc values");
  CUDA_CHECK(cudaMalloc((void **)&d_rhs, (size_t)n * sizeof(cuDoubleComplex)),
            "cudaMalloc rhs");
  CUDA_CHECK(cudaMalloc((void **)&d_x, (size_t)n * sizeof(cuDoubleComplex)),
            "cudaMalloc x");

  CUDA_CHECK(cudaMemcpy(d_rowPtr, rowPtr, (size_t)(n + 1) * sizeof(int),
                       cudaMemcpyHostToDevice), "memcpy rowPtr");
  CUDA_CHECK(cudaMemcpy(d_colInd, colInd, (size_t)nnz * sizeof(int),
                       cudaMemcpyHostToDevice), "memcpy colInd");
  CUDA_CHECK(cudaMemcpy(d_values, values, (size_t)nnz * sizeof(cuDoubleComplex),
                       cudaMemcpyHostToDevice), "memcpy values");
  CUDA_CHECK(cudaMemcpy(d_rhs, rhs, (size_t)n * sizeof(cuDoubleComplex),
                       cudaMemcpyHostToDevice), "memcpy rhs");

  CUDA_CHECK(cudaStreamCreate(&stream), "cudaStreamCreate");

  CUDSS_CHECK(cudssCreate(&handle), "cudssCreate");
  haveHandle = true;
  CUDSS_CHECK(cudssSetStream(handle, stream), "cudssSetStream");

  CUDSS_CHECK(cudssConfigCreate(&solverConfig), "cudssConfigCreate");
  haveConfig = true;
  CUDSS_CHECK(cudssDataCreate(handle, &solverData), "cudssDataCreate");
  haveData = true;

  {
    int64_t nn = n;
    CUDSS_CHECK(cudssMatrixCreateDn(&b, nn, 1, nn, d_rhs, CUDSS_C_64F,
                                    CUDSS_LAYOUT_COL_MAJOR),
               "cudssMatrixCreateDn b");
    haveB = true;
    CUDSS_CHECK(cudssMatrixCreateDn(&x, nn, 1, nn, d_x, CUDSS_C_64F,
                                    CUDSS_LAYOUT_COL_MAJOR),
               "cudssMatrixCreateDn x");
    haveX = true;

    // General/nonsymmetric by default - correct for any FEM system,
    // including symmetric ones (just without exploiting symmetry for
    // extra speed/memory). Revisit with CUDSS_MTYPE_SYMMETRIC/SPD once a
    // caller can reliably tell us the matrix is known-symmetric.
    CUDSS_CHECK(
      cudssMatrixCreateCsr(&A, nn, nn, nnz, d_rowPtr, nullptr, d_colInd,
                           d_values, CUDSS_R_32I, CUDSS_R_32I, CUDSS_C_64F,
                           CUDSS_MTYPE_GENERAL, CUDSS_MVIEW_FULL,
                           CUDSS_BASE_ZERO),
      "cudssMatrixCreateCsr");
    haveA = true;
  }

  CUDSS_CHECK(
    cudssExecute(handle, CUDSS_PHASE_ANALYSIS, solverConfig, solverData, A, x, b),
    "cudssExecute analysis");
  CUDSS_CHECK(
    cudssExecute(handle, CUDSS_PHASE_FACTORIZATION, solverConfig, solverData, A, x, b),
    "cudssExecute factorization");
  CUDSS_CHECK(
    cudssExecute(handle, CUDSS_PHASE_SOLVE, solverConfig, solverData, A, x, b),
    "cudssExecute solve");

  CUDA_CHECK(cudaStreamSynchronize(stream), "cudaStreamSynchronize");
  CUDA_CHECK(cudaMemcpy(solution, d_x, (size_t)n * sizeof(cuDoubleComplex),
                       cudaMemcpyDeviceToHost), "memcpy solution");

  result = 0;

cleanup:
  if(haveA) cudssMatrixDestroy(A);
  if(haveB) cudssMatrixDestroy(b);
  if(haveX) cudssMatrixDestroy(x);
  if(haveData) cudssDataDestroy(handle, solverData);
  if(haveConfig) cudssConfigDestroy(solverConfig);
  if(haveHandle) cudssDestroy(handle);
  if(stream) cudaStreamDestroy(stream);
  if(d_rowPtr) cudaFree(d_rowPtr);
  if(d_colInd) cudaFree(d_colInd);
  if(d_values) cudaFree(d_values);
  if(d_rhs) cudaFree(d_rhs);
  if(d_x) cudaFree(d_x);

  return result;
}
