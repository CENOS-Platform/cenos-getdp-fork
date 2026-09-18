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
// the GPU, factors + solves via cuDSS, downloads the result.
//
// Two things it does beyond a plain one-shot cuDSS call, both of which came
// out of benchmarking the em3d induction-heating cases against MKL PARDISO:
//
//  * It exploits symmetry. Time-harmonic eddy-current matrices are complex
//    SYMMETRIC (A == A^T, not Hermitian). Handing cuDSS the full matrix as
//    CUDSS_MTYPE_GENERAL asks it for an LU, which is about twice the flops
//    and twice the factor storage of the LDL^T the matrix actually needs.
//    The storage half mattered most: the largest measured case wanted
//    ~19 GB of factor on a 16 GB card, and WDDM silently paged the overflow
//    to host memory instead of failing - one solve went from 31 s to 465 s.
//    Symmetry is DETECTED, never assumed: structure is checked when the
//    pattern is first seen and the values are re-checked on every solve, so
//    a formulation that is not symmetric (or stops being symmetric) falls
//    back to the general path on its own.
//
//  * It keeps the factorization context alive between solves. A nonlinear
//    run calls this hundreds of times with the same sparsity pattern and new
//    values; recreating the handle and redoing CUDSS_PHASE_ANALYSIS - the
//    reordering, which is host-side work - every time cost ~0.5-0.9 s per
//    solve on the smallest case, while PETSc's own path reuses its symbolic
//    factorization across the same loop. The retained context is rebuilt
//    whenever the pattern changes, so it holds device memory for exactly one
//    system at a time, the same peak as the one-shot version.
//
// The caller (_cudssTrySolve() in LinAlg_PETSC.cpp) still checks ||Ax-b||
// against the ORIGINAL full matrix on every solve and falls back to the CPU
// if it does not hold, so neither of the above can turn a wrong answer into
// an accepted one.

#include <cuda_runtime.h>
#include <cuComplex.h>
#include <cudss.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

#include "LinAlg_CUDSS.h"

// cuDSS's C API changed shape between 0.7.x and 0.8: 0.8 introduced its own
// cudssDataType_t enum (CUDSS_R_32I/CUDSS_C_64F/...) and split
// cudssMatrixCreateCsr()'s single "indexType" into separate
// "offsetType"/"indexType" parameters (one more argument than 0.7.x); 0.7.x
// instead takes the standard CUDA cudaDataType_t enum (CUDA_R_32I/
// CUDA_C_64F/..., from <library_types.h>, already pulled in transitively)
// and a single index type covering both row offsets and column indices.
// Support both so this file builds against either an NVIDIA-installer cuDSS
// (0.8, as verified on the original dev machine) or the pip-distributed
// nvidia-cudss-cu13 package (0.7.1.6, as actually shipped alongside the RF
// app) without needing two copies of this file.
#if CUDSS_VERSION >= 800
#define GETDP_CUDSS_INDEX_TYPE CUDSS_R_32I
#define GETDP_CUDSS_VALUE_TYPE CUDSS_C_64F
#else
#define GETDP_CUDSS_INDEX_TYPE CUDA_R_32I
#define GETDP_CUDSS_VALUE_TYPE CUDA_C_64F
#endif

// All cuDSS/CUDA-related diagnostics from this file share the "GETDP-CUDSS:"
// prefix (including the two below) so a caller (a wrapping script, a CI log
// scanner, whatever actually invokes getdp.exe) can reliably find out
// whether the GPU path was actually used by grepping for that one fixed
// string, without depending on GetDP's own -v verbosity level - Message::
// Info/Warning are gated by verbosity (off by default below -v 4 / -v 2
// respectively) and are NOT used for any of this, deliberately: these lines
// always print, regardless of -v. All go to stderr.
//
// These two report the failure and bail out of the enclosing helper; the
// caller then drops the retained context and returns failure, so the next
// solve starts from a clean state rather than reusing whatever the failed
// call left behind.
//
// They also record WHICH library said no. cuDSS refusing a call is the one
// failure worth retrying differently: if it will not take this matrix as
// symmetric, the same matrix as general is still worth a try. A CUDA failure -
// no device, out of memory - says nothing about the formulation, and retrying
// it would only print the same error twice per solve.
namespace {
bool lastFailureFromCudss = false;
}

#define CUDA_CHECK(call, msg)                                                 \
  do {                                                                        \
    cudaError_t _e = (call);                                                   \
    if(_e != cudaSuccess) {                                                    \
      lastFailureFromCudss = false;                                            \
      fprintf(stderr, "GETDP-CUDSS: CUDA error in %s: %s\n", msg,              \
              cudaGetErrorString(_e));                                         \
      return false;                                                            \
    }                                                                          \
  } while(0)

#define CUDSS_CHECK(call, msg)                                                \
  do {                                                                        \
    cudssStatus_t _s = (call);                                                 \
    if(_s != CUDSS_STATUS_SUCCESS) {                                           \
      lastFailureFromCudss = true;                                             \
      fprintf(stderr, "GETDP-CUDSS: cuDSS error in %s: status=%d\n", msg,      \
              (int)_s);                                                        \
      return false;                                                            \
    }                                                                          \
  } while(0)

namespace {

// ---------------------------------------------------------------------------
// Retained state
// ---------------------------------------------------------------------------

struct Context {
  bool valid = false;

  // The pattern exactly as the caller handed it over (full matrix, both
  // triangles), kept so the next call can tell whether it is the same system.
  // Comparing the pattern rather than remembering which Mat it came from is
  // deliberate: two different matrices that share a pattern share a valid
  // symbolic factorization too, and that is the only thing being reused.
  int n = 0;
  int nnz = 0;
  std::vector<int> rowPtr;
  std::vector<int> colInd;

  // What actually goes to the device. In symmetric mode that is the upper
  // triangle only, gathered out of the caller's full-matrix value array.
  bool symmetric = false;
  int upNnz = 0;
  std::vector<int> gather; // upNnz: CSR index of each uploaded value
  std::vector<int> mirror; // upNnz: CSR index of (j,i), -1 on the diagonal
  std::vector<double> stage; // 2*upNnz interleaved (re,im)

  int *dRowPtr = nullptr;
  int *dColInd = nullptr;
  cuDoubleComplex *dValues = nullptr;
  cuDoubleComplex *dRhs = nullptr;
  cuDoubleComplex *dX = nullptr;
  cudaStream_t stream = nullptr;

  cudssHandle_t handle = nullptr;
  cudssConfig_t config = nullptr;
  cudssData_t data = nullptr;
  cudssMatrix_t matA = nullptr, matX = nullptr, matB = nullptr;
  bool haveHandle = false, haveConfig = false, haveData = false;
  bool haveA = false, haveX = false, haveB = false;
};

Context ctx;

void destroyContext()
{
  if(ctx.haveA) cudssMatrixDestroy(ctx.matA);
  if(ctx.haveB) cudssMatrixDestroy(ctx.matB);
  if(ctx.haveX) cudssMatrixDestroy(ctx.matX);
  if(ctx.haveData) cudssDataDestroy(ctx.handle, ctx.data);
  if(ctx.haveConfig) cudssConfigDestroy(ctx.config);
  if(ctx.haveHandle) cudssDestroy(ctx.handle);
  if(ctx.stream) cudaStreamDestroy(ctx.stream);
  if(ctx.dRowPtr) cudaFree(ctx.dRowPtr);
  if(ctx.dColInd) cudaFree(ctx.dColInd);
  if(ctx.dValues) cudaFree(ctx.dValues);
  if(ctx.dRhs) cudaFree(ctx.dRhs);
  if(ctx.dX) cudaFree(ctx.dX);
  ctx = Context();
}

bool patternMatches(int n, int nnz, const int *rowPtr, const int *colInd)
{
  if(!ctx.valid || ctx.n != n || ctx.nnz != nnz) return false;
  if(memcmp(ctx.rowPtr.data(), rowPtr, sizeof(int) * (size_t)(n + 1)) != 0)
    return false;
  if(nnz > 0 && memcmp(ctx.colInd.data(), colInd, sizeof(int) * (size_t)nnz) != 0)
    return false;
  return true;
}

// ---------------------------------------------------------------------------
// Symmetry
// ---------------------------------------------------------------------------

// Works out, in one pass over the nonzeros, whether the pattern is
// structurally symmetric with a full diagonal, and where each upper-triangle
// entry and its transposed partner live in the caller's CSR arrays.
//
// O(nnz), not O(nnz log n): the column indices within a PETSc SeqAIJ row are
// sorted, and the mirror of (i,j) is hunted with a per-row cursor that only
// ever moves forward - rows are visited in increasing i, so for a given row j
// the column being looked for also increases monotonically.
//
// Returns false the moment the matrix cannot be described by its upper
// triangle alone, which is exactly the condition for sending it to cuDSS as
// CUDSS_MTYPE_SYMMETRIC / CUDSS_MVIEW_UPPER.
bool buildUpperPattern(int n, int nnz, const int *rowPtr, const int *colInd,
                       std::vector<int> &uRowPtr, std::vector<int> &uColInd,
                       std::vector<int> &gather, std::vector<int> &mirror)
{
  const size_t guess = (size_t)nnz / 2 + (size_t)n;
  uRowPtr.assign((size_t)n + 1, 0);
  uColInd.clear(); uColInd.reserve(guess);
  gather.clear(); gather.reserve(guess);
  mirror.clear(); mirror.reserve(guess);

  std::vector<int> cursor(rowPtr, rowPtr + n);
  long long nLower = 0;

  for(int i = 0; i < n; i++) {
    bool haveDiagonal = false;
    for(int k = rowPtr[i]; k < rowPtr[i + 1]; k++) {
      const int j = colInd[k];
      if(j < i) { // matched later from its upper partner
        nLower++;
        continue;
      }
      if(j == i) {
        haveDiagonal = true;
        uColInd.push_back(j);
        gather.push_back(k);
        mirror.push_back(-1);
        continue;
      }
      // j > i: the transposed entry (j,i) must exist, in row j, at column i
      int c = cursor[j];
      while(c < rowPtr[j + 1] && colInd[c] < i) c++;
      if(c >= rowPtr[j + 1] || colInd[c] != i) return false;
      cursor[j] = c + 1;
      uColInd.push_back(j);
      gather.push_back(k);
      mirror.push_back(c);
    }
    // cuDSS wants the diagonal present in the triangle it is given; a row
    // without one cannot be described this way even if the rest matches.
    if(!haveDiagonal) return false;
    uRowPtr[(size_t)i + 1] = (int)uColInd.size();
  }

  // Every lower entry must have been claimed by exactly one upper entry,
  // otherwise there is something down there the upper triangle does not
  // describe. (uColInd holds one diagonal per row plus the off-diagonals.)
  return nLower == (long long)uColInd.size() - (long long)n;
}

// Gathers the upper triangle into the staging buffer and, on the way, checks
// that the two triangles still agree numerically.
//
// The structure is fixed for the life of the context; the values are not, so
// this runs on every solve. It is one pass over the nonzeros against a
// factorization that costs orders of magnitude more, and it is what stops a
// wrong symmetry assumption from ever reaching cuDSS - rather than finding out
// afterwards, from a residual check, having already paid for the factorization.
bool stageSymmetricValues(const double *values)
{
  const int m = ctx.upNnz;

  // Scale for the "this is just numerical dust" floor below. Taken from the
  // matrix itself because these systems are not scaled to O(1): entries run
  // over many orders of magnitude, and a fixed absolute tolerance would either
  // reject every matrix or accept anything.
  double maxMag = 0.0;
  for(int u = 0; u < m; u++) {
    const int k = ctx.gather[u];
    const double mag = std::fabs(values[2 * k]) + std::fabs(values[2 * k + 1]);
    if(mag > maxMag) maxMag = mag;
  }
  const double rtol = 1e-9;
  const double atol = 1e-12 * maxMag;

  for(int u = 0; u < m; u++) {
    const int k = ctx.gather[u];
    const double re = values[2 * k], im = values[2 * k + 1];
    ctx.stage[2 * (size_t)u] = re;
    ctx.stage[2 * (size_t)u + 1] = im;

    const int t = ctx.mirror[u];
    if(t < 0) continue; // diagonal
    const double tre = values[2 * t], tim = values[2 * t + 1];
    const double diff = std::fabs(re - tre) + std::fabs(im - tim);
    if(diff <= atol) continue;
    const double scale =
      std::fabs(re) + std::fabs(im) + std::fabs(tre) + std::fabs(tim);
    if(diff > rtol * scale) return false;
  }
  return true;
}

// ---------------------------------------------------------------------------
// Context construction: everything that only depends on the pattern
// ---------------------------------------------------------------------------

bool buildContext(int n, int nnz, const int *rowPtr, const int *colInd,
                  const double *values, bool allowSymmetric)
{
  ctx.n = n;
  ctx.nnz = nnz;
  ctx.rowPtr.assign(rowPtr, rowPtr + n + 1);
  ctx.colInd.assign(colInd, colInd + nnz);

  // Host-side first: decide the mode before allocating anything on the device,
  // so falling back to general mode costs nothing but this pass.
  std::vector<int> uRowPtr, uColInd;
  ctx.symmetric = false;
  if(allowSymmetric &&
     buildUpperPattern(n, nnz, rowPtr, colInd, uRowPtr, uColInd, ctx.gather,
                       ctx.mirror)) {
    ctx.symmetric = true;
    ctx.upNnz = (int)uColInd.size();
    ctx.stage.assign(2 * (size_t)ctx.upNnz, 0.0);
    if(!stageSymmetricValues(values)) {
      fprintf(stderr, "GETDP-CUDSS: pattern is symmetric but the values are "
                      "not - using general mode\n");
      ctx.symmetric = false;
    }
  }
  if(!ctx.symmetric) {
    ctx.upNnz = nnz;
    ctx.gather.clear();
    ctx.mirror.clear();
    ctx.stage.clear();
    ctx.stage.shrink_to_fit();
  }

  const int *hRowPtr = ctx.symmetric ? uRowPtr.data() : rowPtr;
  const int *hColInd = ctx.symmetric ? uColInd.data() : colInd;

  CUDA_CHECK(cudaMalloc((void **)&ctx.dRowPtr, (size_t)(n + 1) * sizeof(int)),
             "cudaMalloc rowPtr");
  CUDA_CHECK(cudaMalloc((void **)&ctx.dColInd, (size_t)ctx.upNnz * sizeof(int)),
             "cudaMalloc colInd");
  CUDA_CHECK(cudaMalloc((void **)&ctx.dValues,
                        (size_t)ctx.upNnz * sizeof(cuDoubleComplex)),
             "cudaMalloc values");
  CUDA_CHECK(cudaMalloc((void **)&ctx.dRhs, (size_t)n * sizeof(cuDoubleComplex)),
             "cudaMalloc rhs");
  CUDA_CHECK(cudaMalloc((void **)&ctx.dX, (size_t)n * sizeof(cuDoubleComplex)),
             "cudaMalloc x");

  // Indices are uploaded once per pattern - that is the point of the context.
  CUDA_CHECK(cudaMemcpy(ctx.dRowPtr, hRowPtr, (size_t)(n + 1) * sizeof(int),
                        cudaMemcpyHostToDevice),
             "memcpy rowPtr");
  CUDA_CHECK(cudaMemcpy(ctx.dColInd, hColInd, (size_t)ctx.upNnz * sizeof(int),
                        cudaMemcpyHostToDevice),
             "memcpy colInd");
  // The analysis phase is given a matrix that already has its values, so it
  // never reads uninitialised device memory. factorAndSolve() uploads them
  // again straight after - once per pattern, against an analysis that costs far
  // more, and it keeps the per-solve path identical whether the context was
  // just built or reused.
  CUDA_CHECK(cudaMemcpy(ctx.dValues,
                        ctx.symmetric ? ctx.stage.data() : values,
                        (size_t)ctx.upNnz * sizeof(cuDoubleComplex),
                        cudaMemcpyHostToDevice),
             "memcpy values");

  CUDA_CHECK(cudaStreamCreate(&ctx.stream), "cudaStreamCreate");

  CUDSS_CHECK(cudssCreate(&ctx.handle), "cudssCreate");
  ctx.haveHandle = true;

  // Guard against a real danger, not a hypothetical one: cuDSS's C exports
  // aren't name-mangled with their signature, so Windows will happily
  // resolve e.g. cudssMatrixCreateCsr() against a *loaded* cudss64_0.dll
  // whose actual argument list doesn't match what this file was *compiled*
  // against - no load-time error, just undefined behavior at the first real
  // call (a crash, or worse, a silently wrong solve). This has already
  // happened once in practice (0.7.x vs 0.8 - see the CUDSS_VERSION gate
  // below cudssMatrixCreateCsr()). cudssGetProperty() is identical across
  // both versions, so it's safe to call unconditionally to ask the *loaded*
  // library what it actually is, and compare against what was compiled in
  // (CUDSS_VERSION_MAJOR/MINOR, from cudss.h): a MAJOR.MINOR mismatch means
  // "don't trust this", so bail out to the normal MUMPS fallback instead of
  // risking the mismatched call. PATCH-level differences are left alone
  // (bugfix releases, not API changes).
  {
    int loadedMajor = -1, loadedMinor = -1, loadedPatch = -1;
    CUDSS_CHECK(cudssGetProperty(MAJOR_VERSION, &loadedMajor),
                "cudssGetProperty MAJOR_VERSION");
    CUDSS_CHECK(cudssGetProperty(MINOR_VERSION, &loadedMinor),
                "cudssGetProperty MINOR_VERSION");
    CUDSS_CHECK(cudssGetProperty(PATCH_LEVEL, &loadedPatch),
                "cudssGetProperty PATCH_LEVEL");
    if(loadedMajor != CUDSS_VERSION_MAJOR || loadedMinor != CUDSS_VERSION_MINOR) {
      fprintf(stderr,
              "GETDP-CUDSS: version-mismatch built=%d.%d.%d loaded=%d.%d.%d "
              "- falling back to MUMPS\n",
              CUDSS_VERSION_MAJOR, CUDSS_VERSION_MINOR, CUDSS_VERSION_PATCH,
              loadedMajor, loadedMinor, loadedPatch);
      return false;
    }
  }

  CUDSS_CHECK(cudssSetStream(ctx.handle, ctx.stream), "cudssSetStream");

  CUDSS_CHECK(cudssConfigCreate(&ctx.config), "cudssConfigCreate");
  ctx.haveConfig = true;
  CUDSS_CHECK(cudssDataCreate(ctx.handle, &ctx.data), "cudssDataCreate");
  ctx.haveData = true;

  {
    const int64_t nn = n;
    CUDSS_CHECK(cudssMatrixCreateDn(&ctx.matB, nn, 1, nn, ctx.dRhs,
                                    GETDP_CUDSS_VALUE_TYPE,
                                    CUDSS_LAYOUT_COL_MAJOR),
                "cudssMatrixCreateDn b");
    ctx.haveB = true;
    CUDSS_CHECK(cudssMatrixCreateDn(&ctx.matX, nn, 1, nn, ctx.dX,
                                    GETDP_CUDSS_VALUE_TYPE,
                                    CUDSS_LAYOUT_COL_MAJOR),
                "cudssMatrixCreateDn x");
    ctx.haveX = true;

    // CUDSS_MTYPE_SYMMETRIC means complex symmetric (A == A^T) here, NOT
    // Hermitian - which is what a time-harmonic eddy-current formulation
    // produces, and why CUDSS_MTYPE_HERMITIAN would be the wrong answer even
    // though the matrix is complex. The view says which triangle was
    // uploaded; everything below the diagonal was dropped by
    // buildUpperPattern() after checking it is redundant.
    const cudssMatrixType_t mtype =
      ctx.symmetric ? CUDSS_MTYPE_SYMMETRIC : CUDSS_MTYPE_GENERAL;
    const cudssMatrixViewType_t mview =
      ctx.symmetric ? CUDSS_MVIEW_UPPER : CUDSS_MVIEW_FULL;
    const int64_t upnnz = ctx.upNnz;
#if CUDSS_VERSION >= 800
    // 0.8: separate offsetType (row pointers) / indexType (column indices).
    CUDSS_CHECK(cudssMatrixCreateCsr(&ctx.matA, nn, nn, upnnz, ctx.dRowPtr,
                                     nullptr, ctx.dColInd, ctx.dValues,
                                     GETDP_CUDSS_INDEX_TYPE,
                                     GETDP_CUDSS_INDEX_TYPE,
                                     GETDP_CUDSS_VALUE_TYPE, mtype, mview,
                                     CUDSS_BASE_ZERO),
                "cudssMatrixCreateCsr");
#else
    // < 0.8: a single indexType covers both row offsets and column indices.
    CUDSS_CHECK(cudssMatrixCreateCsr(&ctx.matA, nn, nn, upnnz, ctx.dRowPtr,
                                     nullptr, ctx.dColInd, ctx.dValues,
                                     GETDP_CUDSS_INDEX_TYPE,
                                     GETDP_CUDSS_VALUE_TYPE, mtype, mview,
                                     CUDSS_BASE_ZERO),
                "cudssMatrixCreateCsr");
#endif
    ctx.haveA = true;
  }

  // The expensive, host-side, pattern-only phase. Everything above exists so
  // that this runs once per system instead of once per solve.
  CUDSS_CHECK(cudssExecute(ctx.handle, CUDSS_PHASE_ANALYSIS, ctx.config,
                           ctx.data, ctx.matA, ctx.matX, ctx.matB),
              "cudssExecute analysis");

  ctx.valid = true;
  fprintf(stderr,
          "GETDP-CUDSS: analysed n=%d nnz=%d mode=%s uploaded-nnz=%d\n", n, nnz,
          ctx.symmetric ? "symmetric-upper" : "general", ctx.upNnz);
  return true;
}

// Numeric factorization + solve, on a context whose analysis is already done.
bool factorAndSolve(const double *values, const double *rhs, double *solution)
{
  CUDA_CHECK(cudaMemcpy(ctx.dValues, ctx.symmetric ? ctx.stage.data() : values,
                        (size_t)ctx.upNnz * sizeof(cuDoubleComplex),
                        cudaMemcpyHostToDevice),
             "memcpy values");
  CUDA_CHECK(cudaMemcpy(ctx.dRhs, rhs, (size_t)ctx.n * sizeof(cuDoubleComplex),
                        cudaMemcpyHostToDevice),
             "memcpy rhs");

  CUDSS_CHECK(cudssExecute(ctx.handle, CUDSS_PHASE_FACTORIZATION, ctx.config,
                           ctx.data, ctx.matA, ctx.matX, ctx.matB),
              "cudssExecute factorization");
  CUDSS_CHECK(cudssExecute(ctx.handle, CUDSS_PHASE_SOLVE, ctx.config, ctx.data,
                           ctx.matA, ctx.matX, ctx.matB),
              "cudssExecute solve");

  CUDA_CHECK(cudaStreamSynchronize(ctx.stream), "cudaStreamSynchronize");
  CUDA_CHECK(cudaMemcpy(solution, ctx.dX,
                        (size_t)ctx.n * sizeof(cuDoubleComplex),
                        cudaMemcpyDeviceToHost),
             "memcpy solution");
  return true;
}

} // namespace

// n            : number of rows/cols (square system)
// nnz          : number of nonzeros in the CSR matrix
// rowPtr       : CSR row offsets, length n+1, 0-based
// colInd       : CSR column indices, length nnz, 0-based
// values       : CSR values, length nnz, interleaved (re,im) pairs -
//                binary-compatible with cuDoubleComplex / std::complex<double>
// rhs          : right-hand side, length n, interleaved (re,im) pairs
// solution     : OUT, length n, interleaved (re,im) pairs - caller-allocated
//
// The matrix is always given in full (both triangles); whether only half of it
// is sent to the GPU is decided here.
//
// Returns 0 on success, nonzero on any failure (CUDA or cuDSS error, or no
// GPU present) - caller should fall back to the normal CPU direct solve.
int GetDP_CUDSS_SolveComplex(int n, int nnz, const int *rowPtr,
                             const int *colInd, const double *values,
                             const double *rhs, double *solution)
{
  if(n <= 0 || nnz <= 0) return 1;

  // At most two passes: the second one only ever happens to demote a symmetric
  // context to a general one, which is why nothing here can loop.
  bool allowSymmetric = true;
  for(int attempt = 0; attempt < 2; attempt++) {
    if(!patternMatches(n, nnz, rowPtr, colInd)) {
      destroyContext();
      if(!buildContext(n, nnz, rowPtr, colInd, values, allowSymmetric)) {
        const bool retryAsGeneral = ctx.symmetric && lastFailureFromCudss;
        destroyContext();
        if(retryAsGeneral) {
          fprintf(stderr, "GETDP-CUDSS: cuDSS would not take this matrix as "
                          "symmetric - retrying as general\n");
          allowSymmetric = false;
          continue;
        }
        return 1;
      }
    }
    else if(ctx.symmetric && !stageSymmetricValues(values)) {
      // Same pattern, but this iteration's values no longer mirror. Rebuild in
      // general mode rather than hand cuDSS half a matrix that is not the whole
      // story - and stay there, because the next iteration is unlikely to
      // become symmetric again.
      fprintf(stderr, "GETDP-CUDSS: values stopped being symmetric - rebuilding "
                      "in general mode\n");
      destroyContext();
      allowSymmetric = false;
      continue;
    }

    const bool wasSymmetric = ctx.symmetric;
    if(factorAndSolve(values, rhs, solution)) break;

    // Drop the context: after a CUDA or cuDSS error its device state cannot be
    // trusted for the next solve.
    const bool retryAsGeneral = wasSymmetric && lastFailureFromCudss;
    destroyContext();
    if(!retryAsGeneral) return 1;
    fprintf(stderr, "GETDP-CUDSS: symmetric factorization failed - retrying as "
                    "general\n");
    allowSymmetric = false;
  }

  if(!ctx.valid) return 1;

  // Only reports that cuDSS's own factor+solve succeeded - the caller
  // (_cudssTrySolve() in LinAlg_PETSC.cpp) still runs its own residual
  // sanity check before actually accepting this solution, and prints its
  // own "GETDP-CUDSS: accepted"/"rejected" line reflecting that final
  // decision. This line alone is not proof the result was used.
  fprintf(stderr, "GETDP-CUDSS: solve-succeeded version=%d.%d.%d\n",
          CUDSS_VERSION_MAJOR, CUDSS_VERSION_MINOR, CUDSS_VERSION_PATCH);

  return 0;
}

// Release the retained factorization context and every device allocation
// behind it. Called from LinAlg_FinalizeSolver() before PetscFinalize(), i.e.
// while the CUDA context is still alive - leaving this to static destruction
// order would mean freeing device memory after the runtime may already have
// torn itself down.
void GetDP_CUDSS_Cleanup(void) { destroyContext(); }
