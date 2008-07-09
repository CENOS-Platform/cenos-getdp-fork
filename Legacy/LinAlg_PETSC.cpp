// GetDP - Copyright (C) 1997-2008 P. Dular, C. Geuzaine
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <getdp@geuz.org>.
//
// Contributor(s):
//   David Colignon
//   Ruth Sabariego
//   Jose Geraldo A. Brito Neto
//

#include "LinAlg.h"
#include "MallocUtils.h"
#include "Message.h"

#if defined(HAVE_PETSC)

/* 
   Options for PETSc can be provided on the command line, or in the file
   ~/.petscrc. 

   By default, we use the following options (GMRES iterative solver,
   1.e-8 relative tolerance with ILU(6) preconditioner and RCMK
   renumbering):

   -pc_type ilu
   -pc_ilu_levels 6 (version 2.3.0) or -pc_factor_levels 6 (version 2.3.1)
   -pc_ilu_mat_ordering_type rcm (version 2.3.0) or -pc_factor_mat_ordering rcm (version 2.3.1)
   -ksp_rtol 1.e-8

   Other useful options include:

   -ksp_gmres_restart 100
   -ksp_monitor
   ...

   To use a direct solver (a sparse lu) instead of an iterative
   solver, use

   -ksp_type preonly -pc_type lu

   When PETSc is compiled with external solvers (UMFPACK, SuperLU,
   etc.), they can be accessed simply by changing the matrix type. For
   example, with umfpack:

   -mat_type umfpack -ksp_type preonly -pc_type lu 
*/

#define MYCHECK(ierr) CHKERRABORT(PETSC_COMM_WORLD,ierr)
static int  ierr, SolverInitialized = 0;

/* Initialize */

void LinAlg_Initialize(int* argc, char*** argv)
{
  MPI_Init(argc, argv);
}

void LinAlg_InitializeSolver(int* argc, char*** argv)
{
  /* This function detects if MPI is initialized */
  PetscInitialize(argc, argv, PETSC_NULL, PETSC_NULL);
  SolverInitialized = 1;
}

/* Finalize */

void LinAlg_Finalize()
{
  int flag;

  MPI_Initialized(&flag);
  if(flag) MPI_Finalize();
}

void LinAlg_FinalizeSolver()
{
  if(SolverInitialized) PetscFinalize();
}

/* Barrier */

void LinAlg_Barrier()
{
  MPI_Barrier(PETSC_COMM_WORLD);
}

/* Create */

void LinAlg_CreateSolver(gSolver *Solver, char * SolverDataFileName)
{
  Solver->ksp = NULL;
}

void LinAlg_CreateVector(gVector *V, gSolver *Solver, int n)
{
  ierr = VecCreate(PETSC_COMM_WORLD, &V->V); MYCHECK(ierr);
  ierr = VecSetSizes(V->V, PETSC_DECIDE, n); MYCHECK(ierr);

  /* override the default options with the ones from the option
     database (if any) */
  ierr = VecSetFromOptions(V->V); MYCHECK(ierr);
}

void LinAlg_CreateMatrix(gMatrix *M, gSolver *Solver, int n, int m)
{
  ierr = MatCreate(PETSC_COMM_WORLD, &M->M); MYCHECK(ierr); 
  ierr = MatSetSizes(M->M, PETSC_DECIDE, PETSC_DECIDE, n, m); MYCHECK(ierr); 

  /* override the default options with the ones from the option database (if any) */
  ierr = MatSetFromOptions(M->M); MYCHECK(ierr);

  /* preallocation option must be set after other options */
  ierr = MatSeqAIJSetPreallocation(M->M, 50, PETSC_NULL); MYCHECK(ierr); 
}

/* Destroy */

void LinAlg_DestroySolver(gSolver *Solver)
{
  ierr = KSPDestroy(Solver->ksp); MYCHECK(ierr);
  Solver->ksp = NULL;
}

void LinAlg_DestroyVector(gVector *V)
{
  ierr = VecDestroy(V->V); MYCHECK(ierr);
}

void LinAlg_DestroyMatrix(gMatrix *M)
{
  ierr = MatDestroy(M->M); MYCHECK(ierr);
}

/* Copy */

void LinAlg_CopyScalar(gScalar *S1, gScalar *S2)
{
  S1->s = S2->s;
}

void LinAlg_CopyVector(gVector *V1, gVector *V2)
{
  ierr = VecCopy(V1->V, V2->V); MYCHECK(ierr);
}

void LinAlg_CopyMatrix(gMatrix *M1, gMatrix *M2)
{
  ierr = MatCopy(M1->M, M2->M, DIFFERENT_NONZERO_PATTERN); MYCHECK(ierr);
}

/* Zero */

void LinAlg_ZeroScalar(gScalar *S)
{
  S->s = 0.;
}

void LinAlg_ZeroVector(gVector *V)
{
  PetscScalar zero = 0.0;

  ierr = VecSet(V->V, zero); MYCHECK(ierr);
}

void LinAlg_ZeroMatrix(gMatrix *M)
{
  ierr = MatZeroEntries(M->M); MYCHECK(ierr);
}

/* Scan */

void LinAlg_ScanScalar(FILE *file, gScalar *S)
{
#if PETSC_USE_COMPLEX
  double a, b;
  fscanf(file, "%lf %lf", &a, &b);
  S->s = a + PETSC_i * b;
#else
  fscanf(file, "%lf", &S->s);
#endif
}

void LinAlg_ScanVector(FILE *file, gVector *V)
{
  int i, n;
  PetscScalar tmp;
  double a, b;  
  
  ierr = VecGetSize(V->V, &n); MYCHECK(ierr);
  for(i=0; i<n; i++){
#if PETSC_USE_COMPLEX
    fscanf(file, "%lf %lf", &a, &b);
    tmp = a + PETSC_i * b;
#else
    fscanf(file, "%lf", &a);
    tmp = a;
#endif
    ierr = VecSetValues(V->V, 1, &i, &tmp, INSERT_VALUES); MYCHECK(ierr);
  }
}

void LinAlg_ScanMatrix(FILE *file, gMatrix *M)
{
  Msg::Error("'LinAlg_ScanMatrix' not yet implemented");  
}

/* Read */

void LinAlg_ReadScalar(FILE *file, gScalar *S)
{
  Msg::Error("'LinAlg_ReadScalar' not yet implemented");
}

void LinAlg_ReadVector(FILE *file, gVector *V)
{
  int i, n;
  PetscScalar *tmp;

  ierr = VecGetSize(V->V, &n); MYCHECK(ierr);
  tmp = (PetscScalar*)Malloc(n*sizeof(PetscScalar));
  fread(tmp, sizeof(PetscScalar), n, file);

  for(i=0; i<n; i++){
    ierr = VecSetValues(V->V, 1, &i, &tmp[i], INSERT_VALUES); MYCHECK(ierr);
  }

  Free(tmp);
}

void LinAlg_ReadMatrix(FILE *file, gMatrix *M)
{
  Msg::Error("'LinAlg_ReadMatrix' not yet implemented");  
}

/* Print */

void LinAlg_PrintScalar(FILE *file, gScalar *S)
{
#if PETSC_USE_COMPLEX
  fprintf(file, "%.16g %.16g", real(S->s), imag(S->s));
#else
  fprintf(file, "%.16g", S->s);
#endif
}

void LinAlg_PrintVector(FILE *file, gVector *V)
{
  PetscScalar *tmp;
  int     i, n;

  ierr = VecGetLocalSize(V->V, &n); MYCHECK(ierr);
  ierr = VecGetArray(V->V, &tmp); MYCHECK(ierr);

  for (i=0; i<n; i++){
#if PETSC_USE_COMPLEX
    fprintf(file, "%.16g %.16g\n", real(tmp[i]), imag(tmp[i]));
#else
    fprintf(file, "%.16g\n", tmp[i]);
#endif
  }
  fflush(file);

  ierr = VecRestoreArray(V->V, &tmp); MYCHECK(ierr);
} 

void LinAlg_PrintMatrix(FILE *file, gMatrix *M)
{
  PetscViewer fd;
  
  PetscViewerASCIIOpen(PETSC_COMM_WORLD, "matrix.m", &fd); MYCHECK(ierr);
  ierr = PetscViewerSetFormat(fd, PETSC_VIEWER_ASCII_MATLAB); MYCHECK(ierr);
  ierr = MatView(M->M, fd); MYCHECK(ierr);
  ierr = PetscViewerDestroy(fd); MYCHECK(ierr);
}

/* Write */

void LinAlg_WriteScalar(FILE *file, gScalar *S)
{
  Msg::Error("'LinAlg_WriteScalar' not yet implemented");
}

void LinAlg_WriteVector(FILE *file, gVector *V)
{
  PetscScalar *tmp;
  int n;

  ierr = VecGetLocalSize(V->V, &n); MYCHECK(ierr);
  ierr = VecGetArray(V->V, &tmp); MYCHECK(ierr);
  fwrite(tmp, sizeof(PetscScalar), n, file);
  fprintf(file, "\n");
  ierr = VecRestoreArray(V->V, &tmp); MYCHECK(ierr);
}

void LinAlg_WriteMatrix(FILE *file, gMatrix *M)
{
  Msg::Error("'LinAlg_WriteMatrix' not yet implemented");  
}

/* Get */

void LinAlg_GetVectorSize(gVector *V, int *i)
{
  ierr = VecGetSize(V->V, i); MYCHECK(ierr);
}

void LinAlg_GetLocalVectorRange(gVector *V, int *low, int *high)
{
  ierr = VecGetOwnershipRange(V->V, low, high); MYCHECK(ierr);
}

void LinAlg_GetMatrixSize(gMatrix *M, int *i, int *j)
{
  ierr = MatGetSize(M->M, i, j); MYCHECK(ierr);
}

void LinAlg_GetLocalMatrixRange(gMatrix *M, int *low, int *high)
{
  ierr = MatGetOwnershipRange(M->M, low, high); MYCHECK(ierr);
}

void LinAlg_GetDoubleInScalar(double *d, gScalar *S)
{
#if PETSC_USE_COMPLEX
  *d = real(S->s);
#else
  *d = S->s;
#endif
}

void LinAlg_GetComplexInScalar(double *d1, double *d2, gScalar *S)
{
#if PETSC_USE_COMPLEX
  *d1 = real(S->s);
  *d2 = imag(S->s);
#else
  Msg::Error("'LinAlg_GetComplexInScalar' not available with this Solver");  
#endif
}

void LinAlg_GetScalarInVector(gScalar *S, gVector *V, int i)
{
  PetscScalar *tmp;

  ierr = VecGetArray(V->V, &tmp); MYCHECK(ierr);
  S->s = tmp[i];
  ierr = VecRestoreArray(V->V, &tmp); MYCHECK(ierr);
}

void LinAlg_GetDoubleInVector(double *d, gVector *V, int i)
{
  PetscScalar *tmp;

  ierr = VecGetArray(V->V, &tmp); MYCHECK(ierr);
#if PETSC_USE_COMPLEX
  *d = real(tmp[i]);
#else
  *d = tmp[i];
#endif
  ierr = VecRestoreArray(V->V, &tmp); MYCHECK(ierr);
}

void LinAlg_GetAbsDoubleInVector(double *d, gVector *V, int i)
{
  PetscScalar *tmp;

  ierr = VecGetArray(V->V, &tmp); MYCHECK(ierr);
#if PETSC_USE_COMPLEX
  *d = fabs(real(tmp[i]));
#else
  *d = fabs(tmp[i]);
#endif
  ierr = VecRestoreArray(V->V, &tmp); MYCHECK(ierr);
}

void LinAlg_GetComplexInVector(double *d1, double *d2, gVector *V, int i, int j)
{
  PetscScalar *tmp;

  ierr = VecGetArray(V->V, &tmp); MYCHECK(ierr);
#if PETSC_USE_COMPLEX
  *d1 = real(tmp[i]);
  *d2 = imag(tmp[i]);
#else
  *d1 = (double)tmp[i];
  *d2 = (double)tmp[j];
#endif
  ierr = VecRestoreArray(V->V, &tmp); MYCHECK(ierr);
}

void LinAlg_GetScalarInMatrix(gScalar *S, gMatrix *M, int i, int j)
{
  Msg::Error("'LinAlg_GetScalarInMatrix' not yet implemented");  
}

void LinAlg_GetDoubleInMatrix(double *d, gMatrix *M, int i, int j)
{
  Msg::Error("'LinAlg_GetDoubleInMatrix' not yet implemented");  
}

void LinAlg_GetComplexInMatrix(double *d1, double *d2, gMatrix *M, int i, int j, int k, int l)
{
  Msg::Error("'LinAlg_GetComplexInMatrix' not yet implemented");  
}

void LinAlg_GetColumnInMatrix(gMatrix *M, int col, gVector *V1)
{
  Msg::Error("'LinAlg_GetColumnInMatrix' not yet implemented");  
}

/* Set */

void LinAlg_SetScalar(gScalar *S, double *d)
{
#if PETSC_USE_COMPLEX
  S->s = d[0] + (PETSC_i * d[1]);
#else
  S->s = d[0];
#endif
}

void LinAlg_SetVector(gVector *V, double *v)
{
  PetscScalar tmp = *v;

  ierr = VecSet(V->V, tmp); MYCHECK(ierr);
}

void LinAlg_SetScalarInVector(gScalar *S, gVector *V, int i)
{
  ierr = VecSetValues(V->V, 1, &i, &S->s, INSERT_VALUES); MYCHECK(ierr);
}

void LinAlg_SetDoubleInVector(double d, gVector *V, int i)
{
  PetscScalar tmp;

  tmp = d;
  ierr = VecSetValues(V->V, 1, &i, &tmp, INSERT_VALUES); MYCHECK(ierr);
}

void LinAlg_SetComplexInVector(double d1, double d2, gVector *V, int i, int j)
{
  PetscScalar tmp;

#if PETSC_USE_COMPLEX
  tmp = d1 + PETSC_i * d2;
  ierr = VecSetValues(V->V, 1, &i, &tmp, INSERT_VALUES); MYCHECK(ierr);
#else
  tmp = d1;
  ierr = VecSetValues(V->V, 1, &i, &tmp, INSERT_VALUES); MYCHECK(ierr);
  tmp = d2;
  ierr = VecSetValues(V->V, 1, &j, &tmp, INSERT_VALUES); MYCHECK(ierr);
#endif
}

void LinAlg_SetScalarInMatrix(gScalar *S, gMatrix *M, int i, int j)
{
  ierr = MatSetValues(M->M, 1, &i, 1, &j, &S->s, INSERT_VALUES); MYCHECK(ierr);
}

void LinAlg_SetDoubleInMatrix(double d, gMatrix *M, int i, int j)
{
  Msg::Error("'LinAlg_SetDoubleInMatrix' not yet implemented");  
}

void LinAlg_SetComplexInMatrix(double d1, double d2, gMatrix *M, int i, int j, int k, int l)
{
  Msg::Error("'LinAlg_SetComplexInMatrix' not yet implemented");  
}

/* Add */

void LinAlg_AddScalarScalar(gScalar *S1, gScalar *S2, gScalar *S3)
{
  S3->s = S1->s + S2->s;
}

void LinAlg_DummyVector(gVector *V)
{
  Msg::Error("'LinAlg_DummyVector' not yet implemented");  
}

void LinAlg_AddScalarInVector(gScalar *S, gVector *V, int i)
{
  ierr = VecSetValues(V->V, 1, &i, &S->s, ADD_VALUES); MYCHECK(ierr);
}

void LinAlg_AddDoubleInVector(double d, gVector *V, int i)
{
  PetscScalar tmp;
  tmp = d;
  ierr = VecSetValues(V->V, 1, &i, &tmp, ADD_VALUES); MYCHECK(ierr);
}

void LinAlg_AddComplexInVector(double d1, double d2, gVector *V, int i, int j)
{
  PetscScalar tmp;

#if PETSC_USE_COMPLEX
  tmp = d1 + PETSC_i * d2;
  ierr = VecSetValues(V->V, 1, &i, &tmp, ADD_VALUES); MYCHECK(ierr);
#else
  tmp = d1;
  ierr = VecSetValues(V->V, 1, &i, &tmp, ADD_VALUES); MYCHECK(ierr);
  tmp = d2;
  ierr = VecSetValues(V->V, 1, &j, &tmp, ADD_VALUES); MYCHECK(ierr);
#endif
}

void LinAlg_AddScalarInMatrix(gScalar *S, gMatrix *M, int i, int j)
{
  ierr = MatSetValues(M->M, 1, &i, 1, &j, &S->s, ADD_VALUES); MYCHECK(ierr);
}

void LinAlg_AddDoubleInMatrix(double d, gMatrix *M, int i, int j)
{
  PetscScalar tmp;

  tmp = d;
  ierr = MatSetValues(M->M, 1, &i, 1, &j, &tmp, ADD_VALUES); MYCHECK(ierr);
}

void LinAlg_AddComplexInMatrix(double d1, double d2, gMatrix *M, int i, int j, int k, int l)
{
  PetscScalar tmp;

#if PETSC_USE_COMPLEX
  tmp = d1 + PETSC_i * d2;
  ierr = MatSetValues(M->M, 1, &i, 1, &j, &tmp, ADD_VALUES); MYCHECK(ierr);
#else
  if(d1){
    tmp = d1;
    ierr = MatSetValues(M->M, 1, &i, 1, &j, &tmp, ADD_VALUES); MYCHECK(ierr);
    ierr = MatSetValues(M->M, 1, &k, 1, &l, &tmp, ADD_VALUES); MYCHECK(ierr);
  }
  if(d2){
    tmp = -d2;
    ierr = MatSetValues(M->M, 1, &i, 1, &l, &tmp, ADD_VALUES); MYCHECK(ierr);
    tmp = d2;
    ierr = MatSetValues(M->M, 1, &k, 1, &j, &tmp, ADD_VALUES); MYCHECK(ierr);
  }
#endif
}

void LinAlg_AddVectorVector(gVector *V1, gVector *V2, gVector *V3)
{
  PetscScalar tmp = 1.0;

  if(V3 == V1){
    ierr = VecAXPY(V1->V, tmp, V2->V); MYCHECK(ierr);
  }
  else if(V3 == V2){
    ierr = VecAXPY(V2->V, tmp, V1->V); MYCHECK(ierr);
  }
  else
    Msg::Error("Wrong arguments in 'LinAlg_AddVectorVector'");  
}

void LinAlg_AddVectorProdVectorDouble(gVector *V1, gVector *V2, double d, gVector *V3)
{
  PetscScalar tmp = d;

  if(V3 == V1){
    ierr = VecAXPY(V1->V, tmp, V2->V); MYCHECK(ierr);
  }
  else if(V3 == V2){
    ierr = VecAYPX(V2->V, tmp, V1->V); MYCHECK(ierr);
  }
  else
    Msg::Error("Wrong arguments in 'LinAlg_AddVectorProdVectorDouble'");  
}

void LinAlg_AddMatrixMatrix(gMatrix *M1, gMatrix *M2, gMatrix *M3)
{
  PetscScalar tmp = 1.0;

  if(M3 == M1){
    ierr = MatAXPY(M1->M, tmp, M2->M, DIFFERENT_NONZERO_PATTERN); MYCHECK(ierr);
  }
  else if(M3 == M2){
    ierr = MatAXPY(M2->M, tmp, M1->M, DIFFERENT_NONZERO_PATTERN); MYCHECK(ierr);
  }
  else
    Msg::Error("Wrong arguments in 'LinAlg_AddMatrixMatrix'");  
}

void LinAlg_AddMatrixProdMatrixDouble(gMatrix *M1, gMatrix *M2, double d, gMatrix *M3)
{
  PetscScalar tmp = d;

  if(M3 == M1){
    ierr = MatAXPY(M1->M, tmp, M2->M, DIFFERENT_NONZERO_PATTERN); MYCHECK(ierr);
  }
  else if(M3 == M2){
#if (PETSC_VERSION_MAJOR == 2) && (PETSC_VERSION_MINOR == 3) && (PETSC_VERSION_SUBMINOR < 2)
    ierr = MatAYPX(M2->M, tmp, M1->M); MYCHECK(ierr);
#else
    ierr = MatAYPX(M2->M, tmp, M1->M, DIFFERENT_NONZERO_PATTERN); MYCHECK(ierr);
#endif
  }
  else
    Msg::Error("Wrong arguments in 'LinAlg_AddMatrixProdMatrixDouble'");
}

/* Sub */

void LinAlg_SubScalarScalar(gScalar *S1, gScalar *S2, gScalar *S3)
{
  S3->s = S1->s - S2->s;
}

void LinAlg_SubVectorVector(gVector *V1, gVector *V2, gVector *V3)
{
  PetscScalar tmp = -1.0;

  if(V3 == V1){
    ierr = VecAXPY(V1->V, tmp, V2->V); MYCHECK(ierr);
  }
  else if(V3 == V2){
    ierr = VecAYPX(V2->V, tmp, V1->V); MYCHECK(ierr);
  }
  else
    Msg::Error("Wrong arguments in 'LinAlg_SubVectorVector'");  
}

void LinAlg_SubMatrixMatrix(gMatrix *M1, gMatrix *M2, gMatrix *M3)
{
  Msg::Error("'LinAlg_SubMatrixMatrix' not yet implemented");    
}

/* Prod */

void LinAlg_ProdScalarScalar(gScalar *S1, gScalar *S2, gScalar *S3)
{
  S3->s = S1->s * S2->s;
}

void LinAlg_ProdScalarDouble(gScalar *S1, double d, gScalar *S2)
{
  S2->s = S1->s * d;
}

void LinAlg_ProdScalarComplex(gScalar *S, double d1, double d2, double *d3, double *d4)
{
#if PETSC_USE_COMPLEX
  PetscScalar tmp;
#endif

#if PETSC_USE_COMPLEX
  tmp = S->s * (d1 + PETSC_i * d2);
  *d3 = real(tmp);
  *d4 = imag(tmp);
#else
  *d3 = S->s * d1;
  *d4 = S->s * d2;
#endif
}

void LinAlg_ProdVectorScalar(gVector *V1, gScalar *S, gVector *V2)
{ 
  if(V2 == V1){
    ierr = VecScale(V1->V, S->s); MYCHECK(ierr);
  }
  else
    Msg::Error("Wrong arguments in 'LinAlg_ProdVectorScalar'");
}

void LinAlg_ProdVectorDouble(gVector *V1, double d, gVector *V2)
{
  PetscScalar tmp;
  tmp = d;

  if(V2 == V1){
    ierr = VecScale(V1->V, tmp); MYCHECK(ierr);
  }
  else
    Msg::Error("Wrong arguments in 'LinAlg_ProdVectorDouble'");
}

void LinAlg_ProdVectorComplex(gVector *V1, double d1, double d2, gVector *V2)
{
  Msg::Error("'LinAlg_ProdVectorComplex' not yet implemented");
}

void LinAlg_ProdVectorVector(gVector *V1, gVector *V2, double *d)
{
  PetscScalar tmp;

  ierr = VecDot(V1->V, V2->V, &tmp); MYCHECK(ierr);
#if PETSC_USE_COMPLEX
  *d = real(tmp);
#else
  *d = tmp;
#endif
}

void LinAlg_ProdMatrixVector(gMatrix *M, gVector *V1, gVector *V2)
{
  if(V2==V1)
    Msg::Error("Wrong arguments in 'LinAlg_ProdMatrixVector'");
  else
    ierr = MatMult(M->M, V1->V, V2->V); MYCHECK(ierr);
}

void LinAlg_ProdMatrixScalar(gMatrix *M1, gScalar *S, gMatrix *M2)
{
  if(M2 == M1){
    ierr = MatScale(M1->M, S->s); MYCHECK(ierr);
  }
  else
    Msg::Error("Wrong arguments in 'LinAlg_ProdMatrixScalar'");
}

void LinAlg_ProdMatrixDouble(gMatrix *M1, double d, gMatrix *M2)
{
  PetscScalar tmp;
  tmp = d;

  if(M2 == M1){
    ierr = MatScale(M1->M, tmp); MYCHECK(ierr);
  }
  else
    Msg::Error("Wrong arguments in 'LinAlg_ProdMatrixDouble'");
}

void LinAlg_ProdMatrixComplex(gMatrix *M1, double d1, double d2, gMatrix *M2)
{
#if PETSC_USE_COMPLEX
  if(M2 == M1){
    PetscScalar tmp = d1 + (PETSC_i * d2);
    ierr = MatScale(M1->M, tmp); MYCHECK(ierr);
  }
  else
    Msg::Error("Wrong arguments in 'LinAlg_ProdMatrixDouble'");
#else
  Msg::Error("'LinAlg_ProdMatrixComplex' not yet implemented");
#endif
}

/* Div */

void LinAlg_DivScalarScalar(gScalar *S1, gScalar *S2, gScalar *S3)
{
  S3->s = S1->s / S2->s;
}

void LinAlg_DivScalarDouble(gScalar *S1, double d, gScalar *S2)
{
  S2->s = S1->s / d;
}

/* Norm */

void LinAlg_VectorNorm2(gVector *V1, double *norm)
{
  PetscReal tmp;
  
  VecNorm(V1->V, NORM_2, &tmp);
  *norm = tmp;
}

void LinAlg_VectorNormInf(gVector *V1, double *norm)
{
  PetscReal tmp;
  
  VecNorm(V1->V, NORM_INFINITY, &tmp);
  *norm = tmp;
}

/* Assemble */

void LinAlg_AssembleMatrix(gMatrix *M)
{
  ierr = MatAssemblyBegin(M->M, MAT_FINAL_ASSEMBLY); MYCHECK(ierr);
  ierr = MatAssemblyEnd(M->M, MAT_FINAL_ASSEMBLY); MYCHECK(ierr);  
}

void LinAlg_AssembleVector(gVector *V)
{
  ierr = VecAssemblyBegin(V->V); MYCHECK(ierr);
  ierr = VecAssemblyEnd(V->V); MYCHECK(ierr);
}

/* Solve */

#if defined(HAVE_ZITSOL)

extern "C" {
  int getdp_zitsol(int n, int nnz, int *row, int *col, double *valr, double *vali, 
		   double *rhsr, double *rhsi, double *solr, double *soli,
		   int precond, int lfil, double tol0, double tol, int im, int maxits);
}

static void _zitsol(gMatrix *A, gVector *B, gVector *X)
{
  MatInfo info;
  PetscInt ncols;
  PetscTruth set;
  PetscScalar *b, *x, d;
  const PetscInt *cols;
  const PetscScalar *vals;
  int i, j, k, n, nnz, *row, *col;
  double *valr, *vali, *rhsr, *rhsi, *solr, *soli;

  int precond = 1, lfil = 30, im = 100, maxits = 200;
  double tol0 = 0.01, tol = 1e-10;

  PetscOptionsGetInt(PETSC_NULL, "-precond", &precond, &set);
  PetscOptionsGetInt(PETSC_NULL, "-lfil", &lfil, &set);
  PetscOptionsGetReal(PETSC_NULL, "-tol0", &tol0, &set);

  Msg::Info("precond = %d lfil = %d tol0 = %g", precond, lfil, tol0);

  ierr = MatGetInfo(A->M, MAT_LOCAL, &info);
  n = info.rows_local;
  nnz = info.nz_used;

  row = (int*)Malloc(nnz * sizeof(int));
  col = (int*)Malloc(nnz * sizeof(int));
  valr = (double*)Malloc(nnz * sizeof(double));
  vali = (double*)Malloc(nnz * sizeof(double));
  rhsr = (double*)Malloc(n * sizeof(double));
  rhsi = (double*)Malloc(n * sizeof(double));
  solr = (double*)Malloc(n * sizeof(double));
  soli = (double*)Malloc(n * sizeof(double));

  k = 0;
  for(i = 0; i < n; i++){
    ierr = MatGetRow(A->M, i, &ncols, &cols, &vals);
    for(j = 0; j < ncols; j++){
      if(k >= nnz){
	Msg::Error("something wrong in nnz: %d >= %d", k, nnz);
	return;
      }
      row[k] = i;
      col[k] = cols[j];
      Msg::Debug("A[%d][%d] = ", row[k], col[k]);
#if PETSC_USE_COMPLEX
      valr[k] = real(vals[j]);
      vali[k] = imag(vals[j]);
      Msg::Debug("%g+i*%g", valr[k], vali[k]);
#else
      valr[k] = vals[j];
      vali[k] = 0.;
      Msg::Debug("%g", valr[k]);
#endif
      k++;
    }
    ierr = MatRestoreRow(A->M, i, &ncols, &cols, &vals);
  }

  Msg::Info("n = %d, nnz = %d (check k = %d)", n, nnz, k);

  ierr = VecGetArray(B->V, &b); MYCHECK(ierr);
  ierr = VecGetArray(X->V, &x); MYCHECK(ierr);
  for(i = 0; i < n; i++){
#if PETSC_USE_COMPLEX
    rhsr[i] = real(b[i]);
    rhsi[i] = imag(b[i]);
    solr[i] = real(x[i]);
    soli[i] = imag(x[i]);
#else
    rhsr[i] = b[i];
    rhsi[i] = 0.;
    solr[i] = x[i];
    soli[i] = 0.;
#endif
  }
  ierr = VecRestoreArray(B->V, &b); MYCHECK(ierr);
  ierr = VecRestoreArray(X->V, &x); MYCHECK(ierr);
  
  if(getdp_zitsol(n, nnz, row, col, valr, vali, rhsr, rhsi, solr, soli,
		  precond, lfil, tol0, tol, im, maxits) >= maxits){
    Msg::Error("Did not converge in %d iterations", maxits);
  }

  for(i = 0; i < n; i++){
#if PETSC_USE_COMPLEX
    d = solr[i] + PETSC_i * soli[i];
#else
    d = solr[i];
#endif
    ierr = VecSetValues(X->V, 1, &i, &d, INSERT_VALUES); MYCHECK(ierr);
  }
  
  Free(row);
  Free(col);
  Free(valr);
  Free(vali);
  Free(rhsr);
  Free(rhsi);
  Free(solr);
  Free(soli);
}

#endif

static void _solve(gMatrix *A, gVector *B, gSolver *Solver, gVector *X, int precond)
{
  int its, RankCpu, i, j, view = 0;

#if defined(HAVE_ZITSOL)
  /* testing Yousef's new preconditioners and solvers */
  PetscTruth zitsol = PETSC_FALSE, set;
  PetscOptionsGetTruth(PETSC_NULL, "-zitsol", &zitsol, &set);
  if(zitsol){ _zitsol(A, B, X); return; }
#endif

  MPI_Comm_rank(PETSC_COMM_WORLD, &RankCpu);

  if(!Solver->ksp && Msg::GetVerbosity() > 0) view = 1;

  if(view && !RankCpu){
    ierr = MatGetSize(A->M, &i, &j); MYCHECK(ierr);
    Msg::Info("N: %d", i);
  }

  if(!Solver->ksp) {
    ierr = KSPCreate(PETSC_COMM_WORLD, &Solver->ksp); MYCHECK(ierr);
    ierr = KSPSetOperators(Solver->ksp, A->M, A->M, DIFFERENT_NONZERO_PATTERN); MYCHECK(ierr);
    ierr = KSPGetPC(Solver->ksp, &Solver->pc); MYCHECK(ierr);
    /* set some default options */
    ierr = PCSetType(Solver->pc, PCILU); MYCHECK(ierr);
#if (PETSC_VERSION_MAJOR == 2) && (PETSC_VERSION_MINOR == 3) && (PETSC_VERSION_SUBMINOR == 0)
    ierr = PCILUSetMatOrdering(Solver->pc, MATORDERING_RCM); MYCHECK(ierr);
    ierr = PCILUSetLevels(Solver->pc, 6); MYCHECK(ierr);
#elif (PETSC_VERSION_MAJOR == 2) && (PETSC_VERSION_MINOR == 3) && (PETSC_VERSION_SUBMINOR < 3)
    ierr = PCFactorSetMatOrdering(Solver->pc, MATORDERING_RCM); MYCHECK(ierr);
    ierr = PCFactorSetLevels(Solver->pc, 6); MYCHECK(ierr);
#else
    ierr = PCFactorSetMatOrderingType(Solver->pc, MATORDERING_RCM); MYCHECK(ierr);
    ierr = PCFactorSetLevels(Solver->pc, 6); MYCHECK(ierr);
#endif
    ierr = KSPSetTolerances(Solver->ksp, 1.e-8, PETSC_DEFAULT, PETSC_DEFAULT, 
			    PETSC_DEFAULT); MYCHECK(ierr);
    
    /* override the default options with the ones from the option
       database (if any) */
    ierr = KSPSetFromOptions(Solver->ksp); MYCHECK(ierr);
  }
  else if(precond){
    ierr = KSPSetOperators(Solver->ksp, A->M, A->M, DIFFERENT_NONZERO_PATTERN); MYCHECK(ierr);
  }
  
  ierr = KSPSolve(Solver->ksp, B->V, X->V); MYCHECK(ierr);

    if(view){
      ierr = KSPView(Solver->ksp,PETSC_VIEWER_STDOUT_SELF); MYCHECK(ierr);
    }

  if(!RankCpu){
    ierr = KSPGetIterationNumber(Solver->ksp, &its); MYCHECK(ierr);
    Msg::Info("%d iterations", its);
  }
}

void LinAlg_Solve(gMatrix *A, gVector *B, gSolver *Solver, gVector *X)
{
  _solve(A, B, Solver, X, 1);
}

void LinAlg_SolveAgain(gMatrix *A, gVector *B, gSolver *Solver, gVector *X)
{
  _solve(A, B, Solver, X, 0);
}

#endif
