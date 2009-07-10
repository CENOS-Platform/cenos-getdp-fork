// GetDP - Copyright (C) 1997-2008 P. Dular, C. Geuzaine
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <getdp@geuz.org>.
//
// Contributor(s):
//   Johan Gyselinck
//   Ruth Sabariego
//

#include <map>
#include <complex>
#include <string.h>
#include <math.h>
#include "GetDPVersion.h"
#include "ProData.h"
#include "DofData.h"
#include "ExtendedGroup.h"
#include "ListUtils.h"
#include "TreeUtils.h"
#include "MallocUtils.h"
#include "Message.h"

#define TWO_PI             6.2831853071795865

extern struct Problem Problem_S ;
extern struct CurrentData Current ;

FILE  * File_PRE, * File_RES, * File_TMP ;

struct DofData  * CurrentDofData ;

int fcmp_Dof(const void * a, const void * b)
{
  int Result ;

  if ((Result = ((struct Dof *)a)->NumType  - ((struct Dof *)b)->NumType) != 0)
    return Result ;
  if ((Result = ((struct Dof *)a)->Entity   - ((struct Dof *)b)->Entity)  != 0)
    return Result ;
  return        ((struct Dof *)a)->Harmonic - ((struct Dof *)b)->Harmonic ;
}

/* ------------------------------------------------------------------------ */
/*  D o f _ I n i t D o f D a t a                                           */
/* ------------------------------------------------------------------------ */

void Dof_InitDofData(struct DofData * DofData_P, int Num,
		     int ResolutionIndex, int SystemIndex,
		     char * Name_SolverDataFile)
{
  int  Index ;

  DofData_P->Num = Num ;

  DofData_P->ResolutionIndex = ResolutionIndex ;
  DofData_P->SystemIndex     = SystemIndex ;

  DofData_P->FunctionSpaceIndex = NULL ;
  DofData_P->TimeFunctionIndex  = List_Create(10, 5, sizeof(int)) ;
  Index = 0 ;  List_Add(DofData_P->TimeFunctionIndex, &Index) ;
  DofData_P->Pulsation = NULL ;  DofData_P->Val_Pulsation = NULL ;
  DofData_P->NbrHar = 1 ;

  DofData_P->NbrAnyDof = 0 ;  DofData_P->NbrDof = 0 ;
  DofData_P->DofTree = Tree_Create(sizeof(struct Dof), fcmp_Dof) ;
  DofData_P->DofList = NULL ;

  DofData_P->SolverDataFileName = Name_SolverDataFile ;
  DofData_P->Flag_Init[0] = 0 ;
  DofData_P->Flag_Init[1] = 0 ;
  DofData_P->Flag_Init[2] = 0 ;
  DofData_P->Flag_Init[3] = 0 ;

  DofData_P->Flag_Only = 0 ;
  DofData_P->Flag_InitOnly[0] = 0 ;
  DofData_P->Flag_InitOnly[1] = 0 ;
  DofData_P->Flag_InitOnly[2] = 0 ;

  DofData_P->OnlyTheseMatrices = NULL;

  DofData_P->Solutions = NULL ;
  DofData_P->CurrentSolution = NULL ;

  DofData_P->CorrectionSolutions.Flag = 0;
  DofData_P->CorrectionSolutions.AllSolutions = NULL;

  DofData_P->DummyDof = NULL ;
}

/* ------------------------------------------------------------------------ */
/*  D o f _ S e t C u r r e n t D o f D a t a                               */
/* ------------------------------------------------------------------------ */

void Dof_SetCurrentDofData(struct DofData * DofData_P)
{
  CurrentDofData = DofData_P ;
}

/* ------------------------------------------------------------------------ */
/*  F i l e s   . . .                                                       */
/* ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------ */
/*  D o f _ O p e n F i l e                                                 */
/* ------------------------------------------------------------------------ */

void Dof_OpenFile(int Type, char * Name, const char * Mode) 
{
  const char  * Extension;
  char    FileName[256] ;
  FILE  * File_X ;

  switch (Type) {
  case DOF_PRE :  Extension = ".pre" ;  break ;
  case DOF_RES :  Extension = ""     ;  break ;
  case DOF_TMP :  Extension = ""     ;  break ;
  default      :  Extension = ".pre" ;  break ;
  }

  strcpy(FileName, Name) ; strcat(FileName, Extension) ;

  if (!(File_X = fopen(FileName, Mode)))
    Msg::Error("Unable to open file '%s'", FileName) ;

  switch (Type) {
  case DOF_PRE :  File_PRE = File_X ;  break ;
  case DOF_RES :  File_RES = File_X ;  break ;
  case DOF_TMP :  File_TMP = File_RES ; File_RES = File_X ;  break ;
  default      :  break ;
  }
}

/* ------------------------------------------------------------------------ */
/*  D o f _ C l o s e F i l e                                               */
/* ------------------------------------------------------------------------ */

void Dof_CloseFile(int Type)
{
  switch (Type) {
  case DOF_PRE :  fclose(File_PRE) ;  break ;
  case DOF_RES :  fclose(File_RES) ;  break ;
  case DOF_TMP :  fclose(File_RES) ;  File_RES = File_TMP ; break ;
  }
}

/* ------------------------------------------------------------------------ */
/*  D o f _ F l u s h F i l e                                               */
/* ------------------------------------------------------------------------ */

void Dof_FlushFile(int Type)
{
  switch (Type) {
  case DOF_PRE :  fflush(File_PRE) ;  break ;
  case DOF_RES :  fflush(File_RES) ;  break ;
  }
}

/* ------------------------------------------------------------------------ */
/*  D o f _ W r i t e F i l e P R E 0                                       */
/* ------------------------------------------------------------------------ */

void Dof_WriteFilePRE0(int Num_Resolution, char * Name_Resolution,
		       int Nbr_DofData)
{
  fprintf(File_PRE, "$Resolution /* '%s' */\n", Name_Resolution) ;
  fprintf(File_PRE, "%d %d\n", Num_Resolution, Nbr_DofData) ;
  fprintf(File_PRE, "$EndResolution\n") ;
}

/* ------------------------------------------------------------------------ */
/*  D o f _ R e a d F i l e P R E 0                                         */
/* ------------------------------------------------------------------------ */

void Dof_ReadFilePRE0(int * Num_Resolution, int * Nbr_DofData)
{
  char  String[256] ;

  do { 
    fgets(String, sizeof(String), File_PRE) ; 
    if (feof(File_PRE))  break ;
  } while (String[0] != '$') ;

  if (feof(File_PRE)) Msg::Error("$Resolution field not found in file");

  if (!strncmp(&String[1], "Resolution", 10)) {
    fscanf(File_PRE, "%d %d", Num_Resolution, Nbr_DofData) ;
  }

  do {
    fgets(String, sizeof(String), File_PRE) ;
    if (feof(File_PRE)) Msg::Error("Prematured end of file");
  } while (String[0] != '$') ;
}

/* ------------------------------------------------------------------------ */
/*  D o f _ W r i t e F i l e P R E                                         */
/* ------------------------------------------------------------------------ */

void Dof_WriteFilePRE(struct DofData * DofData_P)
{
  int  i, Nbr_Index ;
  struct Dof  * Dof_P0 ;

  fprintf(File_PRE, "$DofData /* #%d */\n", DofData_P->Num) ;

  fprintf(File_PRE, "%d %d\n",
	  DofData_P->ResolutionIndex, DofData_P->SystemIndex) ;

  Nbr_Index = List_Nbr(DofData_P->FunctionSpaceIndex) ;
  fprintf(File_PRE, "%d", Nbr_Index) ;
  for (i = 0 ; i < Nbr_Index ; i++)
    fprintf(File_PRE, " %d",
	    *((int *)List_Pointer(DofData_P->FunctionSpaceIndex, i))) ;
  fprintf(File_PRE, "\n") ;

  Nbr_Index = List_Nbr(DofData_P->TimeFunctionIndex) ;
  fprintf(File_PRE, "%d", Nbr_Index) ;
  for (i = 0 ; i < Nbr_Index ; i++)
    fprintf(File_PRE, " %d",
	    *((int *)List_Pointer(DofData_P->TimeFunctionIndex, i))) ;
  fprintf(File_PRE, "\n") ;

  fprintf(File_PRE, "%d", 1) ;
  for(i = 0 ; i < 1 ; i++)
    fprintf(File_PRE, " %d", 0) ;
  fprintf(File_PRE, "\n") ;

  fprintf(File_PRE, "%d %d\n",
	  (DofData_P->DofTree)? Tree_Nbr(DofData_P->DofTree) : DofData_P->NbrAnyDof,
	  DofData_P->NbrDof) ;

  if (DofData_P->DofTree)
    Tree_Action(DofData_P->DofTree, Dof_WriteDofPRE) ;
  else {
    if (DofData_P->NbrAnyDof){
      Dof_P0 = (struct Dof *)List_Pointer(DofData_P->DofList, 0) ;
      for (i = 0 ; i < DofData_P->NbrAnyDof ; i++)
	Dof_WriteDofPRE(Dof_P0 + i, NULL) ;
    }
  }
  fprintf(File_PRE, "$EndDofData\n") ;
  fflush(File_PRE) ;
}

/* ------------------------------- */
/*  D o f _ W r i t e D o f P R E  */
/* ------------------------------- */

void Dof_WriteDofPRE(void * a, void * b)
{
  struct Dof  * Dof_P ;

  Dof_P = (struct Dof *) a ;

  fprintf(File_PRE, "%d %d %d %d ",
	  Dof_P->NumType, Dof_P->Entity, Dof_P->Harmonic, Dof_P->Type) ;

  switch (Dof_P->Type) {
  case DOF_UNKNOWN :
    fprintf(File_PRE, "%d %d\n", Dof_P->Case.Unknown.NumDof, 1) ;
    break ;
  case DOF_FIXEDWITHASSOCIATE :
    fprintf(File_PRE, "%d ", Dof_P->Case.FixedAssociate.NumDof) ;
    LinAlg_PrintScalar(File_PRE, &Dof_P->Val);
    fprintf(File_PRE, " %d\n", Dof_P->Case.FixedAssociate.TimeFunctionIndex) ;
    break ;
  case DOF_FIXED :
    LinAlg_PrintScalar(File_PRE, &Dof_P->Val);
    fprintf(File_PRE, " %d\n", Dof_P->Case.FixedAssociate.TimeFunctionIndex) ;
    break ;
  case DOF_FIXED_SOLVE :
    fprintf(File_PRE, "%d\n", Dof_P->Case.FixedAssociate.TimeFunctionIndex) ;
    break ;
  case DOF_UNKNOWN_INIT :
    fprintf(File_PRE, "%d ", Dof_P->Case.Unknown.NumDof) ;
    LinAlg_PrintScalar(File_PRE, &Dof_P->Val);
    fprintf(File_PRE, " %d\n", 1) ;
    break ;
  case DOF_LINK :
    fprintf(File_PRE, "%.16g %d\n",
	    Dof_P->Case.Link.Coef, Dof_P->Case.Link.EntityRef) ;
    break ;
  case DOF_LINKCPLX :
    fprintf(File_PRE, "%.16g %.16g %d\n",
	    Dof_P->Case.Link.Coef, Dof_P->Case.Link.Coef2, 
	    Dof_P->Case.Link.EntityRef) ;
    break ;
  }
}

/* ------------------------------------------------------------------------ */
/*  D o f _ R e a d F i l e P R E                                           */
/* ------------------------------------------------------------------------ */

void Dof_ReadFilePRE(struct DofData * DofData_P)
{
  int         i, Nbr_Index, Int, Dummy ;
  struct Dof  Dof ;
  char        String[256] ;

  do { 
    fgets(String, sizeof(String), File_PRE) ; 
    if (feof(File_PRE))  break ;
  } while (String[0] != '$') ;

  if (feof(File_PRE)) Msg::Error("$DofData field not found in file");

  if (!strncmp(&String[1], "DofData", 7)) {

    fscanf(File_PRE, "%d %d",
	   &DofData_P->ResolutionIndex, &DofData_P->SystemIndex) ;

    fscanf(File_PRE, "%d", &Nbr_Index) ;
    DofData_P->FunctionSpaceIndex = List_Create(Nbr_Index, 1, sizeof(int)) ;
    for (i = 0 ; i < Nbr_Index ; i++) {
      fscanf(File_PRE, "%d", &Int) ;
      List_Add(DofData_P->FunctionSpaceIndex, &Int) ;
    }

    fscanf(File_PRE, "%d", &Nbr_Index) ;
    DofData_P->TimeFunctionIndex = List_Create(Nbr_Index, 1, sizeof(int)) ;
    for (i = 0 ; i < Nbr_Index ; i++) {
      fscanf(File_PRE, "%d", &Int) ;
      List_Add(DofData_P->TimeFunctionIndex, &Int) ;
    }

    fscanf(File_PRE, "%d", &Dummy) ;
    for(i = 0 ; i < 1 ; i++)
      fscanf(File_PRE, "%d", &Dummy) ;

    fscanf(File_PRE, "%d %d", &DofData_P->NbrAnyDof, &DofData_P->NbrDof) ;

    DofData_P->DofList = List_Create(DofData_P->NbrAnyDof, 1, sizeof(struct Dof)) ;

    for (i = 0 ; i < DofData_P->NbrAnyDof ; i++) {

      fscanf(File_PRE, "%d %d %d %d",
	     &Dof.NumType, &Dof.Entity, &Dof.Harmonic, &Dof.Type) ;

      switch (Dof.Type) {
      case DOF_UNKNOWN :
	fscanf(File_PRE, "%d", &Dof.Case.Unknown.NumDof) ;
	fscanf(File_PRE, "%d", &Dummy) ;
	break ;
      case DOF_FIXEDWITHASSOCIATE :
	fscanf(File_PRE, "%d", &Dof.Case.FixedAssociate.NumDof) ;
	LinAlg_ScanScalar(File_PRE, &Dof.Val) ;
	fscanf(File_PRE, "%d", &Dof.Case.FixedAssociate.TimeFunctionIndex) ;
	break ;
      case DOF_FIXED :
	LinAlg_ScanScalar(File_PRE, &Dof.Val) ;
	fscanf(File_PRE, "%d", &Dof.Case.FixedAssociate.TimeFunctionIndex) ;
	break ;
      case DOF_FIXED_SOLVE :
	fscanf(File_PRE, "%d", &Dof.Case.FixedAssociate.TimeFunctionIndex) ;
	break ;
      case DOF_UNKNOWN_INIT :
	fscanf(File_PRE, "%d", &Dof.Case.Unknown.NumDof) ;
	LinAlg_ScanScalar(File_PRE, &Dof.Val) ;
	fscanf(File_PRE, "%d", &Dummy) ;
	break ;
      case DOF_LINK :
	fscanf(File_PRE, "%lf %d",
	       &Dof.Case.Link.Coef, &Dof.Case.Link.EntityRef) ;
	Dof.Case.Link.Dof = NULL ;
	break ;
      case DOF_LINKCPLX :
	fscanf(File_PRE, "%lf %lf %d",
	       &Dof.Case.Link.Coef, &Dof.Case.Link.Coef2, &Dof.Case.Link.EntityRef) ;
	Dof.Case.Link.Dof = NULL ;
	break ;
      }

      List_Add(DofData_P->DofList, &Dof) ;
    }
  }

  do {
    fgets(String, sizeof(String), File_PRE) ;
    if (feof(File_PRE)) Msg::Error("Prematured end of file");
  } while (String[0] != '$') ;

  Dof_InitDofType(DofData_P) ;
}

/* ------------------------------------------------------------------------ */
/*  D o f _ W r i t e F i l e R E S 0                                       */
/* ------------------------------------------------------------------------ */

void Dof_WriteFileRES0(char * Name_File, int Format)
{
  Dof_OpenFile(DOF_RES, Name_File, (char*)(Format ? "wb" : "w")) ;
  fprintf(File_RES, "$ResFormat /* GetDP %s, %s */\n", GETDP_VERSION, 
	  Format ? "binary" : "ascii") ;
  fprintf(File_RES, "1.1 %d\n", Format) ;
  fprintf(File_RES, "$EndResFormat\n") ;
  Dof_CloseFile(DOF_RES) ;
}

/* ------------------------------------------------------------------------ */
/*  D o f _ W r i t e F i l e R E S _ E x t e n d M H                       */
/* ------------------------------------------------------------------------ */

void Dof_WriteFileRES_ExtendMH(char * Name_File, struct DofData * DofData_P,
			       int Format, int NbrH)
{
  gVector x;
  double d;
  int i, inew;

  Dof_OpenFile(DOF_RES, Name_File, (char*)(Format ? "ab" : "a")) ;

  fprintf(File_RES, "$Solution  /* DofData #%d */\n", DofData_P->Num) ;
  fprintf(File_RES, "%d 0 0 0 \n", DofData_P->Num) ;

  LinAlg_CreateVector(&x, &DofData_P->Solver, (DofData_P->NbrDof / Current.NbrHar) * NbrH) ;

  LinAlg_ZeroVector(&x) ;

  for (i=0 ; i<DofData_P->NbrDof ; i++){
    LinAlg_GetDoubleInVector(&d, &DofData_P->CurrentSolution->x, i);
    inew = (i / Current.NbrHar) * NbrH + i % Current.NbrHar;
    LinAlg_SetDoubleInVector(d, &x, inew);
  }

  Format ? 
    LinAlg_WriteVector(File_RES,&x) :
    LinAlg_PrintVector(File_RES,&x) ;

  fprintf(File_RES, "$EndSolution\n") ;

  Dof_CloseFile(DOF_RES) ;

  LinAlg_DestroyVector(&x) ;
}

void Dof_WriteFileRES_MHtoTime(char * Name_File, struct DofData * DofData_P, 
			       int Format, List_T * Time_L) 
{
  gVector x;
  double Time, d1, d2, d, *Pulsation;
  int iT, i, j, k;

  Dof_OpenFile(DOF_RES, Name_File, (char*)(Format ? "ab" : "a")) ;


  for(iT=0 ; iT<List_Nbr(Time_L) ; iT++){

    List_Read(Time_L, iT, &Time);

    fprintf(File_RES, "$Solution  /* DofData #%d */\n", DofData_P->Num) ;
    fprintf(File_RES, "%d %e 0 %d \n", DofData_P->Num,Time,iT) ;

    Pulsation = DofData_P->Val_Pulsation ; 
    
    LinAlg_CreateVector(&x, &DofData_P->Solver, DofData_P->NbrDof/Current.NbrHar) ;
    
    LinAlg_ZeroVector(&x) ;
    
    for (i=0 ; i<DofData_P->NbrDof/Current.NbrHar ; i++) {
      d = 0;
      for (k=0 ; k<Current.NbrHar/2 ; k++) {
	j = i * Current.NbrHar + 2*k ; 
	LinAlg_GetDoubleInVector(&d1, &DofData_P->CurrentSolution->x, j) ;
	LinAlg_GetDoubleInVector(&d2, &DofData_P->CurrentSolution->x, j+1) ;
	d += d1 * cos(Pulsation[k]*Time) - d2 * sin(Pulsation[k]*Time) ; 
      }
      LinAlg_SetDoubleInVector(d, &x, i) ;
    }
    
    Format ? 
      LinAlg_WriteVector(File_RES,&x) :
      LinAlg_PrintVector(File_RES,&x) ;
    
    fprintf(File_RES, "$EndSolution\n") ;
  }


  Dof_CloseFile(DOF_RES) ;

  LinAlg_DestroyVector(&x) ;
}

/* ------------------------------------------------------------------------ */
/*  D o f _ W r i t e F i l e R E S                                         */
/* ------------------------------------------------------------------------ */

void Dof_WriteFileRES(char * Name_File, struct DofData * DofData_P, int Format,
		      double Val_Time, double Val_TimeImag, int Val_TimeStep) 
{
  Dof_OpenFile(DOF_RES, Name_File, (char*)(Format ? "ab" : "a")) ;

  fprintf(File_RES, "$Solution  /* DofData #%d */\n", DofData_P->Num) ;
  fprintf(File_RES, "%d %.16g %.16g %d\n", DofData_P->Num, Val_Time, 
	  Val_TimeImag, Val_TimeStep) ;

  Format ? 
    LinAlg_WriteVector(File_RES, &DofData_P->CurrentSolution->x) :
    LinAlg_PrintVector(File_RES, &DofData_P->CurrentSolution->x) ;

  fprintf(File_RES, "$EndSolution\n") ;

  Dof_CloseFile(DOF_RES) ;
}

/* ------------------------------------------------------------------------ */
/*  D o f _ W r i t e F i l e R E S _ W i t h E n t i t y N u m             */
/* ------------------------------------------------------------------------ */

void Dof_WriteFileRES_WithEntityNum(char * Name_File, struct DofData * DofData_P,
                                    struct Group *Group_P, bool saveFixed) 
{
  FILE *fp = fopen(Name_File, "w");
  if(!fp){
    Msg::Error("Unable to open file '%s'", Name_File) ;
    return;
  }

  std::map<int, std::complex<double> > unknowns;

  List_T *l = !DofData_P->DofList ? Tree2List(DofData_P->DofTree) : 0;
  int N = l ? List_Nbr(l) : List_Nbr(DofData_P->DofList);
  for(int i = 0; i < N; i++){
    Dof *dof;
    if(l)
      List_Read(l, i, &dof);
    else
      dof = (Dof*)List_Pointer(DofData_P->DofList, i);
    if(dof->Type == DOF_UNKNOWN){
      gScalar s;
      LinAlg_GetScalarInVector(&s, &DofData_P->CurrentSolution->x, 
                               dof->Case.Unknown.NumDof - 1);
      unknowns[dof->Entity] = s.s;
    }
    if(saveFixed && dof->Type == DOF_FIXED){
      unknowns[dof->Entity] = dof->Val.s;
    }
  }

  if(!Group_P){
    for(std::map<int, std::complex<double> >::iterator it = unknowns.begin();
        it != unknowns.end(); it++)
      fprintf(fp, "%d %g %g\n", it->first, it->second.real(), it->second.imag());
  }
  else{
    Msg::Info("Writing solution for all entities in group '%s'", Group_P->Name) ;
    if(!Group_P->ExtendedList) Generate_ExtendedGroup(Group_P) ;
    for(int i = 0; i < List_Nbr(Group_P->ExtendedList); i++){
      int num;
      List_Read(Group_P->ExtendedList, i, &num);
      if(unknowns.count(num)){
        std::complex<double> s = unknowns[num];
        fprintf(fp, "%d %g %g\n", num, s.real(), s.imag());
      }
      else{
        // yes, write zero: that's on purpose for the iterative schemes
        fprintf(fp, "%d 0 0\n", num);
      }
    }
  }

  if(l) List_Delete(l);
  
  fclose(fp);
}

/* ------------------------------------------------------------------------ */
/*  D o f _ R e a d F i l e R E S                                           */
/* ------------------------------------------------------------------------ */

void Dof_ReadFileRES(List_T * DofData_L, struct DofData * Read_DofData_P,
		     int Read_DofData, double *Time, double *TimeImag,
		     double *TimeStep) 
{
  int             Num_DofData, Val_TimeStep, Format = 0, Read ;
  double          Val_Time, Val_TimeImag = 0., Version = 0.;
  struct DofData  * DofData_P = NULL ;
  struct Solution Solution_S ;
  char            String[256] ;

  while (1) {

    do { 
      fgets(String, sizeof(String), File_RES) ; 
      if (feof(File_RES))  break ;
    } while (String[0] != '$') ;  

    if (feof(File_RES))  break ;

    /*  F o r m a t  */

    if (!strncmp(&String[1], "ResFormat", 9)) {
      fscanf(File_RES, "%lf %d\n", &Version, &Format) ;
    }

    /*  S o l u t i o n  */

    if (!strncmp(&String[1], "Solution", 8)) {

      /* don't use fscanf directly on the stream here: the data that
	 follows can be binary, and the first character could be
	 e.g. 0x0d, which would cause fscanf to eat-up one character
	 too much, leading to an offset in fread */
      fgets(String, sizeof(String), File_RES) ; 
      if(Version <= 1.0)
	sscanf(String, "%d %lf %d", &Num_DofData, &Val_Time, &Val_TimeStep) ;
      else
	sscanf(String, "%d %lf %lf %d", &Num_DofData, &Val_Time, &Val_TimeImag, 
	       &Val_TimeStep) ;

      if (Read_DofData < 0){
	Read = 1 ; DofData_P = (struct DofData*)List_Pointer(DofData_L, Num_DofData) ;
      }
      else if (Num_DofData == Read_DofData) {
	Read = 1 ; DofData_P = Read_DofData_P ; 
      }
      else {
	Read = 0 ;
      }

      if(Read){
	Solution_S.Time = Val_Time ;
	Solution_S.TimeImag = Val_TimeImag ;
	Solution_S.TimeStep = Val_TimeStep ;
	Solution_S.SolutionExist = 1 ;
	Solution_S.TimeFunctionValues = NULL ;

	LinAlg_CreateVector(&Solution_S.x, &DofData_P->Solver, DofData_P->NbrDof) ;
	Format ? 
	  LinAlg_ReadVector(File_RES, &Solution_S.x) :
	  LinAlg_ScanVector(File_RES, &Solution_S.x) ;

	if (DofData_P->Solutions == NULL)
	  DofData_P->Solutions = List_Create( 20, 20, sizeof(struct Solution)) ;
	List_Add(DofData_P->Solutions, &Solution_S) ;
      }
    }

    do {
      fgets(String, sizeof(String), File_RES) ;
      if (feof(File_RES)) 
	Msg::Warning("Prematured end of file (Time Step %d)", Val_TimeStep);
    } while (String[0] != '$') ;

  }   /* while 1 ... */

  *Time = Val_Time ;
  *TimeImag = Val_TimeImag ;
  *TimeStep = (double)Val_TimeStep ;
}

/* ------------------------------------------------------------------------ */
/*  D o f _ T r a n s f e r D o f T r e e T o L i s t                       */
/* ------------------------------------------------------------------------ */

void Dof_TransferDofTreeToList(struct DofData * DofData_P) 
{
  if (DofData_P->DofTree) {
    DofData_P->DofList = Tree2List(DofData_P->DofTree) ;
    Tree_Delete(DofData_P->DofTree) ;
    DofData_P->DofTree = NULL ;
    DofData_P->NbrAnyDof = List_Nbr(DofData_P->DofList) ;
  }

  Dof_InitDofType(DofData_P) ;
}

/* ------------------------------------------------------------------------ */
/*  D o f _ I n i t D o f T y p e                                           */
/* ------------------------------------------------------------------------ */

void Dof_InitDofType(struct DofData * DofData_P) 
{
  struct Dof  * Dof_P, * Dof_P0 ;
  int  i ;

  if (!DofData_P->NbrAnyDof){
    return;
  }

  Dof_P0 = (struct Dof *)List_Pointer(DofData_P->DofList, 0) ;

  for (i = 0 ; i < DofData_P->NbrAnyDof ; i++) {
    Dof_P = Dof_P0 + i ;

    switch (Dof_P->Type) {
    case DOF_LINK :
    case DOF_LINKCPLX :
      Dof_P->Case.Link.Dof =
	Dof_GetDofStruct(DofData_P, Dof_P->NumType,
			 Dof_P->Case.Link.EntityRef, Dof_P->Harmonic) ;
      if (Dof_P->Case.Link.Dof == NULL) {
	Dof_P->Case.Link.Dof = /* Attention: bricolage ... */
	  Dof_GetDofStruct(DofData_P, Dof_P->NumType-1,
			   Dof_P->Case.Link.EntityRef, Dof_P->Harmonic) ;
	if (Dof_P->Case.Link.Dof == NULL)
	  Msg::Error("Wrong Link Constraint: reference Dof (%d %d %d) does not exist",
		     Dof_P->NumType, Dof_P->Case.Link.EntityRef, Dof_P->Harmonic);
      }
      /*
      if (Dof_P->Case.Link.Dof == NULL)
	Msg::Error("Wrong Link Constraint: reference Dof (%d %d %d) does not exist",
	           Dof_P->NumType, Dof_P->Case.Link.EntityRef, Dof_P->Harmonic);
      */
      break ;
    default :
      break ;
    }

  }
}

/* ------------------------------------------------------------------------ */
/*  P R E P R O C E S S I N G   ( C o d e s   i n   T r e e )               */
/* ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------ */
/*  D o f _ A d d F u n c t i o n S p a c e I n d e x                       */
/* ------------------------------------------------------------------------ */

void Dof_AddFunctionSpaceIndex(int Index_FunctionSpace)
{
  if (CurrentDofData->FunctionSpaceIndex == NULL)
    CurrentDofData->FunctionSpaceIndex = List_Create(10, 5, sizeof(int)) ;

  if (List_PQuery
      (CurrentDofData->FunctionSpaceIndex, &Index_FunctionSpace, fcmp_int) == NULL) {
    List_Add(CurrentDofData->FunctionSpaceIndex, &Index_FunctionSpace) ;
    List_Sort(CurrentDofData->FunctionSpaceIndex, fcmp_int) ;
  }
}

/* ------------------------------------------------------------------------ */
/*  D o f _ A d d T i m e F u n c t i o n I n d e x                         */
/* ------------------------------------------------------------------------ */

void Dof_AddTimeFunctionIndex(int Index_TimeFunction)
{
  if (List_PQuery
      (CurrentDofData->TimeFunctionIndex, &Index_TimeFunction, fcmp_int) == NULL) {
    List_Add(CurrentDofData->TimeFunctionIndex, &Index_TimeFunction) ;
    List_Sort(CurrentDofData->TimeFunctionIndex, fcmp_int) ;
  }
}

/* ------------------------------------------------------------------------ */
/*  D o f _ A d d P u l s a t i o n                                         */
/* ------------------------------------------------------------------------ */

void Dof_AddPulsation(struct DofData * DofData_P, double Val_Pulsation)
{
  if (DofData_P->Pulsation == NULL)
    DofData_P->Pulsation = List_Create(1, 2, sizeof(double)) ;

  if (List_PQuery
      (DofData_P->Pulsation, &Val_Pulsation, fcmp_double) == NULL) {
    List_Add(DofData_P->Pulsation, &Val_Pulsation) ;
    List_Sort(DofData_P->Pulsation, fcmp_double) ;
  }
}

/* ------------------------------------------------------------------------ */
/*  D o f _ D e f i n e A s s i g n F i x e d D o f                         */
/* ------------------------------------------------------------------------ */

void Dof_DefineAssignFixedDof(int D1, int D2, int NbrHar, double *Val, 
			      int Index_TimeFunction)
{
  struct Dof  Dof, * Dof_P ;
  int         k ;

  Dof.NumType = D1 ;  Dof.Entity = D2 ;  
  
  for(k=0 ; k<NbrHar ; k+=gSCALAR_SIZE){
    Dof.Harmonic = k ;
    if (!(Dof_P = (struct Dof *)Tree_PQuery(CurrentDofData->DofTree, &Dof))) {
      Dof.Type = DOF_FIXED ;
      LinAlg_SetScalar(&Dof.Val, &Val[k]) ;
      Dof.Case.FixedAssociate.TimeFunctionIndex = Index_TimeFunction + 1 ;
      Dof_AddTimeFunctionIndex(Index_TimeFunction + 1) ;
      Tree_Add(CurrentDofData->DofTree, &Dof) ;
    }
    else if(Dof_P->Type == DOF_UNKNOWN) {
      if(Msg::GetVerbosity() == 10)
	Msg::Info("Overriding unknown Dof with fixed Dof");
      Dof_P->Type = DOF_FIXED ;
      LinAlg_SetScalar(&Dof_P->Val, &Val[k]) ;
      Dof_P->Case.FixedAssociate.TimeFunctionIndex = Index_TimeFunction + 1 ;
      Dof_AddTimeFunctionIndex(Index_TimeFunction + 1) ;
    }
  }
}

/* ------------------------------------------------------------------------ */
/*  D o f _ D e f i n e A s s i g n S o l v e D o f                         */
/* ------------------------------------------------------------------------ */

void Dof_DefineAssignSolveDof(int D1, int D2, int NbrHar, int Index_TimeFunction)
{
  struct Dof  Dof ;
  int         k ;
  
  Dof.NumType = D1 ;  Dof.Entity = D2 ;  

  for(k=0 ; k<NbrHar ; k+=gSCALAR_SIZE){
    Dof.Harmonic = k ;
    if (!Tree_PQuery(CurrentDofData->DofTree, &Dof)) {
      Dof.Type = DOF_FIXED_SOLVE ;
      Dof.Case.FixedAssociate.TimeFunctionIndex = Index_TimeFunction + 1 ;
      Dof_AddTimeFunctionIndex(Index_TimeFunction + 1) ; 
      Tree_Add(CurrentDofData->DofTree, &Dof) ;
    }
  }
}

/* ------------------------------------------------------------------------ */
/*  D o f _ D e f i n e I n i t F i x e d D o f                             */
/* ------------------------------------------------------------------------ */

void Dof_DefineInitFixedDof(int D1, int D2, int NbrHar, double *Val)
{
  struct Dof  Dof ;
  int         k ;

  Dof.NumType = D1 ;  Dof.Entity = D2 ;

  for(k=0 ; k<NbrHar ; k+=gSCALAR_SIZE){
    Dof.Harmonic = k ;
    if (!Tree_PQuery(CurrentDofData->DofTree, &Dof)) {
      Dof.Type = DOF_UNKNOWN_INIT ;
      LinAlg_SetScalar(&Dof.Val, &Val[k]) ;
      Dof.Case.Unknown.NumDof = ++(CurrentDofData->NbrDof) ;
      Tree_Add(CurrentDofData->DofTree, &Dof) ;
    }
  }
}

/* ------------------------------------------------------------------------ */
/*  D o f _ D e f i n e I n i t S o l v e D o f                             */
/* ------------------------------------------------------------------------ */

void Dof_DefineInitSolveDof(int D1, int D2, int NbrHar)
{
  struct Dof  Dof ;
  int         k ;

  Dof.NumType = D1 ;  Dof.Entity = D2 ; 

  for(k=0 ; k<NbrHar ; k+=gSCALAR_SIZE){
    Dof.Harmonic = k ;
    if (!Tree_PQuery(CurrentDofData->DofTree, &Dof)) {
      Dof.Type = DOF_UNKNOWN_INIT ;
      Dof.Case.Unknown.NumDof = ++(CurrentDofData->NbrDof) ;
      Tree_Add(CurrentDofData->DofTree, &Dof) ;
    }
  }
}

/* ------------------------------------------------------------------------ */
/*  D o f _ D e f i n e L i n k D o f                                       */
/* ------------------------------------------------------------------------ */

void Dof_DefineLinkDof(int D1, int D2, int NbrHar, double Value[], int D2_Link)
{
  struct Dof  Dof ;
  int         k ;

  Dof.NumType = D1 ;  Dof.Entity = D2 ;  
  
  for(k=0 ; k<NbrHar ; k+=gSCALAR_SIZE){
    Dof.Harmonic = k ;
    if (!Tree_PQuery(CurrentDofData->DofTree, &Dof)) {
      Dof.Type = DOF_LINK ;
      Dof.Case.Link.Coef = Value[0] ;
      Dof.Case.Link.EntityRef = D2_Link ;
      Dof.Case.Link.Dof = NULL ;
      Tree_Add(CurrentDofData->DofTree, &Dof) ;
    }
  }
}

void Dof_DefineLinkCplxDof(int D1, int D2, int NbrHar, double Value[], int D2_Link)
{
  struct Dof  Dof ;
  int         k ;

  Dof.NumType = D1 ;  Dof.Entity = D2 ;  
  
  for(k=0 ; k<NbrHar ; k+=gSCALAR_SIZE){
    Dof.Harmonic = k ;
    if (!Tree_PQuery(CurrentDofData->DofTree, &Dof)) {
      Dof.Type = DOF_LINKCPLX ;
      Dof.Case.Link.Coef = Value[0] ;
      Dof.Case.Link.Coef2 = Value[1] ;
      Dof.Case.Link.EntityRef = D2_Link ;
      Dof.Case.Link.Dof = NULL ;
      Tree_Add(CurrentDofData->DofTree, &Dof) ;
    }
  }
}

/* ------------------------------------------------------------------------ */
/*  D o f _ D e f i n e U n k n o w n D o f                                 */
/* ------------------------------------------------------------------------ */

void Dof_DefineUnknownDof(int D1, int D2, int NbrHar)
{
  struct Dof  Dof ;
  int         k ;

  Dof.NumType = D1 ;  Dof.Entity = D2 ;

  for(k=0 ; k<NbrHar ; k+=gSCALAR_SIZE){
    Dof.Harmonic = k ;
    if (!Tree_PQuery(CurrentDofData->DofTree, &Dof)) {
      Dof.Type = DOF_UNKNOWN ;
      Dof.Case.Unknown.NumDof = -1 ;
      /* Dof.Case.Unknown.NumDof = ++(CurrentDofData->NbrDof) ; */
      Tree_Add(CurrentDofData->DofTree, &Dof) ;
    }
  }
}

void NumberUnknownDof (void *a, void *b)
{
  struct Dof * Dof_P ;
  
  Dof_P = (struct Dof *)a ;

  if(Dof_P->Type == DOF_UNKNOWN && Dof_P->Case.Unknown.NumDof == -1)
    Dof_P->Case.Unknown.NumDof = ++(CurrentDofData->NbrDof) ;
}

void Dof_NumberUnknownDof(void)
{
  if(CurrentDofData->DofTree)
    Tree_Action(CurrentDofData->DofTree, NumberUnknownDof) ;
  else
    List_Action(CurrentDofData->DofList, NumberUnknownDof) ;
}

/* ------------------------------------------------------------------------ */
/*  D o f _ D e f i n e A s s o c i a t e D o f                             */
/* ------------------------------------------------------------------------ */

void Dof_DefineAssociateDof(int E1, int E2, int D1, int D2, int NbrHar)
{
  struct Dof  Dof, Equ, * Equ_P ;
  int         k ;

  Equ.NumType = E1 ;  Equ.Entity = E2 ;

  for(k=0 ; k<NbrHar ; k+=gSCALAR_SIZE){
    Equ.Harmonic = k ;
    if ((Equ_P = (struct Dof*)Tree_PQuery(CurrentDofData->DofTree, &Equ))) {
      switch (Equ_P->Type) {
      case DOF_FIXED :
	Equ_P->Type = DOF_FIXEDWITHASSOCIATE ;
	Equ_P->Case.FixedAssociate.NumDof = ++(CurrentDofData->NbrDof) ;
	Dof.NumType = D1 ; Dof.Entity = D2 ; Dof.Harmonic = k ;
	if (!Tree_PQuery(CurrentDofData->DofTree, &Dof)) {
	  Dof.Type = DOF_UNKNOWN ;
	  Dof.Case.Unknown.NumDof = CurrentDofData->NbrDof ;
	  Tree_Add(CurrentDofData->DofTree, &Dof) ;
	}
	break ;
      case DOF_FIXED_SOLVE :
	Equ_P->Type = DOF_FIXEDWITHASSOCIATE_SOLVE ;
	Equ_P->Case.FixedAssociate.NumDof = ++(CurrentDofData->NbrDof) ;
	Dof.NumType = D1 ; Dof.Entity = D2 ; Dof.Harmonic = k ;
	if (!Tree_PQuery(CurrentDofData->DofTree, &Dof)) {
	  Dof.Type = DOF_UNKNOWN ;
	  Dof.Case.Unknown.NumDof = CurrentDofData->NbrDof ;
	  Tree_Add(CurrentDofData->DofTree, &Dof) ;
	}
	break ;
      case DOF_UNKNOWN :  case DOF_UNKNOWN_INIT :
	Dof_DefineUnknownDof(D1, D2, NbrHar) ;
	break ;
      }
    }
  }
}

void Dof_GetDof_Four(struct DofData * DofData_P, struct DofData * DofData2_P)
{
  int NbrHar2, NbrDof2, i ;
  struct Dof * Dof_P ;

  NbrHar2 = DofData2_P->NbrHar ;
  NbrDof2 = List_Nbr(DofData2_P->DofList) ;

  for (i=0 ; i<NbrDof2 ; i+=NbrHar2) {

    Dof_P = (struct Dof *) List_Pointer(DofData2_P->DofList, i) ;

    printf("i %d, dof %d \n", i/NbrHar2, List_ISearch(DofData_P->DofList, Dof_P, fcmp_Dof)) ; 
	  
  }
}

/* ------------------------------------------------------------------------ */
/*  P R O C E S S I N G   ( C o d e s   i n   L i s t )                     */
/* ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------ */
/*  D o f _ G e t D o f S t r u c t                                         */
/* ------------------------------------------------------------------------ */

struct Dof *Dof_GetDofStruct(struct DofData * DofData_P, int D1, int D2, int D3)
{
  struct Dof  Dof ;

  Dof.NumType = D1 ;  Dof.Entity = D2 ;  Dof.Harmonic = D3 ;
  
  return (struct Dof *)List_PQuery(DofData_P->DofList, &Dof, fcmp_Dof);
}

/* ------------------------------------------------------------------------ */
/*  D o f _ U p d a t e A s s i g n F i x e d D o f                         */
/* ------------------------------------------------------------------------ */

void Dof_UpdateAssignFixedDof(struct Dof *Dof_P, int NbrHar, double *Val)
{
  int k ;

  for(k=0 ; k<NbrHar ; k+=gSCALAR_SIZE){
    LinAlg_SetScalar(&Dof_P->Val, &Val[k]) ;
  }
}

/* ------------------------------------------------------------------------ */
/*  D o f _ U p d a t e L i n k D o f                                       */
/* ------------------------------------------------------------------------ */

void Dof_UpdateLinkDof(struct Dof *Dof_P, int NbrHar, double Value[], int D2_Link)
{
  int         k ;

  if (Dof_P->Type == DOF_LINK || Dof_P->Type == DOF_LINKCPLX) {
    /*
    fprintf(stderr,"===> %d %d %.16g\n", Dof_P->NumType, Dof_P->Entity, Value[0]) ;
    */
    for(k=0 ; k<NbrHar ; k+=gSCALAR_SIZE){
      Dof_P->Case.Link.Coef = Value[0] ;

      if (Dof_P->Type == DOF_LINKCPLX)
	Dof_P->Case.Link.Coef2 = Value[1] ;

      Dof_P->Case.Link.EntityRef = D2_Link ;
      Dof_P->Case.Link.Dof = NULL ;
    }

  }
}

/* ------------------------------------------------------------------------ */
/*  D o f _ A s s e m b l e I n M a t                                       */
/* ------------------------------------------------------------------------ */

void Dof_AssembleInMat(struct Dof * Equ_P, struct Dof * Dof_P, int NbrHar, 
		       double * Val, gMatrix * Mat, gVector * Vec)
{
  gScalar tmp, tmp2 ;
  double  valtmp[2], d1, d2 ;

  switch (Equ_P->Type) {
  case DOF_UNKNOWN :
  case DOF_FIXEDWITHASSOCIATE :

    switch (Dof_P->Type) {
      
    case DOF_UNKNOWN :
      if(NbrHar==1){
	if(Val[0])
	  LinAlg_AddDoubleInMatrix
	    (Val[0], Mat, 
	     Equ_P->Case.Unknown.NumDof-1, Dof_P->Case.Unknown.NumDof-1) ;
      }
      else
	LinAlg_AddComplexInMatrix
	  (Val[0], Val[1], Mat,
	   Equ_P->Case.Unknown.NumDof-1, Dof_P->Case.Unknown.NumDof-1,
	   (gSCALAR_SIZE==1)?((Equ_P+1)->Case.Unknown.NumDof-1):-1,
	   (gSCALAR_SIZE==1)?((Dof_P+1)->Case.Unknown.NumDof-1):-1) ;
      break ;

    case DOF_FIXED :
    case DOF_FIXEDWITHASSOCIATE :
      if(Vec){
	if(NbrHar==1){
	  if(Val[0]){
	    LinAlg_ProdScalarDouble
	      (&Dof_P->Val, 
	       CurrentDofData->CurrentSolution->
	       TimeFunctionValues[Dof_P->Case.FixedAssociate.TimeFunctionIndex],
	       &tmp);
	    LinAlg_ProdScalarDouble(&tmp, -Val[0], &tmp) ;
	    LinAlg_AddScalarInVector(&tmp, Vec, Equ_P->Case.Unknown.NumDof-1) ;
	  }
	}
	else{
	  LinAlg_ProdScalarDouble
	    (&Dof_P->Val, 
	     CurrentDofData->CurrentSolution->
	     TimeFunctionValues[Dof_P->Case.FixedAssociate.TimeFunctionIndex],
	     &tmp);
	  if(gSCALAR_SIZE == 2){
	    LinAlg_ProdScalarComplex(&tmp, -Val[0], -Val[1], &valtmp[0], &valtmp[1]) ;
	  }
	  else{
	    LinAlg_GetDoubleInScalar(&d1, &tmp);
	    LinAlg_ProdScalarDouble
	      (&(Dof_P+1)->Val, 
	       CurrentDofData->CurrentSolution->
	       TimeFunctionValues[Dof_P->Case.FixedAssociate.TimeFunctionIndex],
	       &tmp2);
	    LinAlg_GetDoubleInScalar(&d2, &tmp2);
	    valtmp[0] = -d1*Val[0] + d2*Val[1] ;
	    valtmp[1] = -d1*Val[1] - d2*Val[0] ;
	  }
	  LinAlg_AddComplexInVector
	    (valtmp[0], valtmp[1], Vec,
	     Equ_P->Case.Unknown.NumDof-1,
	     (gSCALAR_SIZE==1)?((Equ_P+1)->Case.Unknown.NumDof-1):-1) ;
	}
      }
      break ;

    case DOF_LINK :
      if(NbrHar==1)
	valtmp[0] = Val[0] * Dof_P->Case.Link.Coef ;
      else{
	valtmp[0] = Val[0] * Dof_P->Case.Link.Coef ;
	valtmp[1] = Val[1] * Dof_P->Case.Link.Coef ;
      }
      Dof_AssembleInMat(Equ_P, Dof_P->Case.Link.Dof, NbrHar, valtmp, Mat, Vec) ;
      break ;

    case DOF_LINKCPLX :
      if(NbrHar==1)
	Msg::Error("LinkCplx only valid for Complex systems") ;
      else{
	valtmp[0] = Val[0] * Dof_P->Case.Link.Coef - Val[1] * Dof_P->Case.Link.Coef2 ;
	valtmp[1] = Val[1] * Dof_P->Case.Link.Coef + Val[0] * Dof_P->Case.Link.Coef2 ;
      }
      Dof_AssembleInMat(Equ_P, Dof_P->Case.Link.Dof, NbrHar, valtmp, Mat, Vec) ;
      break ;

    case DOF_FIXED_SOLVE :  case DOF_FIXEDWITHASSOCIATE_SOLVE :
      Msg::Error("Wrong Constraints: "
		 "remaining Dof(s) waiting to be fixed by a Resolution");
      break;

    case DOF_UNKNOWN_INIT :
      Msg::Error("Wrong Initial Constraints: "
		 "remaining Dof(s) with non-fixed initial conditions");
      break;
    }

    break ;

  case DOF_LINK :
    if(NbrHar==1)
      valtmp[0] = Val[0] * Equ_P->Case.Link.Coef ;
    else{
      valtmp[0] = Val[0] * Equ_P->Case.Link.Coef ;
      valtmp[1] = Val[1] * Equ_P->Case.Link.Coef ;
    }
    Dof_AssembleInMat(Equ_P->Case.Link.Dof, Dof_P, NbrHar, valtmp, Mat, Vec) ;
    break ;

  case DOF_LINKCPLX :
    if(NbrHar==1)
      Msg::Error("LinkCplx only valid for Complex systems") ;
    else{ /* Warning: conjugate! */
      valtmp[0] = Val[0] * Equ_P->Case.Link.Coef + Val[1] * Equ_P->Case.Link.Coef2 ;
      valtmp[1] = Val[1] * Equ_P->Case.Link.Coef - Val[0] * Equ_P->Case.Link.Coef2 ;
    }
    Dof_AssembleInMat(Equ_P->Case.Link.Dof, Dof_P, NbrHar, valtmp, Mat, Vec) ;
    break ;

  }
}

/* ------------------------------------------------------------------------ */
/*  D o f _ A s s e m b l e I n V e c                                       */
/* ------------------------------------------------------------------------ */

void Dof_AssembleInVec(struct Dof * Equ_P, struct Dof * Dof_P, int NbrHar, 
		       double * Val, struct Solution * OtherSolution,
		       gVector * Vec0, gVector * Vec)
{
  gScalar tmp ;
  double valtmp[2] ;
  double a, b, c, d ;

  switch (Equ_P->Type) {
  case DOF_UNKNOWN :
  case DOF_FIXEDWITHASSOCIATE :

    switch (Dof_P->Type) {

    case DOF_UNKNOWN :
      if(NbrHar==1){	
	if(Val[0]){
	  LinAlg_GetDoubleInVector(&a, Vec0, Dof_P->Case.Unknown.NumDof-1) ;
	  a *= Val[0] ;
	  LinAlg_AddDoubleInVector(a, Vec, Equ_P->Case.Unknown.NumDof-1) ;
	}
      }
      else{
	LinAlg_GetComplexInVector(&a, &b, Vec0,
				  Dof_P->Case.Unknown.NumDof-1, 
				  (gSCALAR_SIZE==1)?((Dof_P+1)->Case.Unknown.NumDof-1):-1) ;
	c = a * Val[0] - b * Val[1] ; 
	d = a * Val[1] + b * Val[0] ;
	LinAlg_AddComplexInVector(c, d, Vec,
				  Equ_P->Case.Unknown.NumDof-1, 
				  (gSCALAR_SIZE==1)?((Equ_P+1)->Case.Unknown.NumDof-1):-1) ;
      }
      break ;

    case DOF_FIXED :
    case DOF_FIXEDWITHASSOCIATE :
      if(NbrHar==1){	
	if(Val[0]){
	  LinAlg_ProdScalarDouble
	    (&Dof_P->Val, 
	     Val[0] * OtherSolution->
	     TimeFunctionValues[Dof_P->Case.FixedAssociate.TimeFunctionIndex], 
	     &tmp) ;
	  LinAlg_AddScalarInVector(&tmp, Vec, Equ_P->Case.Unknown.NumDof-1) ;
	}
      }
      else{
	if(gSCALAR_SIZE == 2){
	  LinAlg_ProdScalarComplex
	    (&Dof_P->Val, 
	     Val[0] * OtherSolution->
	     TimeFunctionValues[Dof_P->Case.FixedAssociate.TimeFunctionIndex], 
	     Val[1] * OtherSolution->
	     TimeFunctionValues[Dof_P->Case.FixedAssociate.TimeFunctionIndex], 
	     &a, &b) ;
	  LinAlg_AddComplexInVector(a, b, Vec,
				    Equ_P->Case.Unknown.NumDof-1, 
				    (gSCALAR_SIZE==1)?((Equ_P+1)->Case.Unknown.NumDof-1):-1) ;
	}
	else{
	  Msg::Error("Assemby in vectors with more than one harmonic not yet implemented") ;
	}
      }
      break ;

    case DOF_LINK :
      if(NbrHar==1)
	valtmp[0] = Val[0] * Dof_P->Case.Link.Coef ;
      else{
	valtmp[0] = Val[0] * Dof_P->Case.Link.Coef ;
	valtmp[1] = Val[1] * Dof_P->Case.Link.Coef ;
      }
      Dof_AssembleInVec(Equ_P, Dof_P->Case.Link.Dof, NbrHar, 
			valtmp, OtherSolution, Vec0, Vec) ;
      break ;

    case DOF_LINKCPLX :
      if(NbrHar==1)
	Msg::Error("LinkCplx only valid for Complex systems") ;
      else{
	valtmp[0] = Val[0] * Dof_P->Case.Link.Coef - Val[1] * Dof_P->Case.Link.Coef2 ;
	valtmp[1] = Val[1] * Dof_P->Case.Link.Coef + Val[0] * Dof_P->Case.Link.Coef2 ;
      }
      Dof_AssembleInVec(Equ_P, Dof_P->Case.Link.Dof, NbrHar, 
			valtmp, OtherSolution, Vec0, Vec) ;
      break ;

    case DOF_FIXED_SOLVE :  case DOF_FIXEDWITHASSOCIATE_SOLVE :
      Msg::Error("Wrong Constraints: "
		 "remaining Dof(s) waiting to be fixed by a Resolution");
      break;

    case DOF_UNKNOWN_INIT :
      Msg::Error("Wrong Initial Constraints: "
		 "remaining Dof(s) with non-fixed initial conditions");
      break;
    }
    break ;

  case DOF_LINK :
    if(NbrHar==1)
      valtmp[0] = Val[0] * Equ_P->Case.Link.Coef ;
    else{
      valtmp[0] = Val[0] * Equ_P->Case.Link.Coef ;
      valtmp[1] = Val[1] * Equ_P->Case.Link.Coef ;
    }
    Dof_AssembleInVec(Equ_P->Case.Link.Dof, Dof_P, NbrHar, 
		      valtmp, OtherSolution, Vec0, Vec) ;
    break ;

  case DOF_LINKCPLX :
    if(NbrHar==1)
      Msg::Error("LinkCplx only valid for Complex systems") ;
    else{ /* Warning: conjugate! */
      valtmp[0] = Val[0] * Equ_P->Case.Link.Coef + Val[1] * Equ_P->Case.Link.Coef2 ;
      valtmp[1] = Val[1] * Equ_P->Case.Link.Coef - Val[0] * Equ_P->Case.Link.Coef2 ;
    }
    Dof_AssembleInVec(Equ_P->Case.Link.Dof, Dof_P, NbrHar, 
		      valtmp, OtherSolution, Vec0, Vec) ;
    break ;

  }
}

/* ------------------------------------------------------------------------ */
/*  D o f _ T r a n s f e r S o l u t i o n T o C o n s t r a i n t         */
/* ------------------------------------------------------------------------ */

void Dof_TransferSolutionToConstraint(struct DofData * DofData_P)
{
  struct Dof * Dof_P, * Dof_P0 ;
  int  i ;

  if (!DofData_P->NbrAnyDof){
    return;
  }

  Dof_P0 = (struct Dof *)List_Pointer(DofData_P->DofList, 0) ;


  for (i = 0 ; i < DofData_P->NbrAnyDof ; i++) {
    Dof_P = Dof_P0 + i ;

    switch (Dof_P->Type) {

    case DOF_UNKNOWN :
      Dof_P->Type = DOF_FIXED ;
      LinAlg_GetScalarInVector(&Dof_P->Val, 
			       &DofData_P->CurrentSolution->x, 
			       Dof_P->Case.Unknown.NumDof-1) ;
      Dof_P->Case.FixedAssociate.TimeFunctionIndex = 0 ;
      break ;

    case DOF_FIXED :
    case DOF_FIXEDWITHASSOCIATE :
    case DOF_LINK :
    case DOF_LINKCPLX :
      break ;

    default :  break ;
    }
  }

  DofData_P->NbrDof = 0 ;
}

/* ------------------------------------------------------------------------ */
/*  D o f _ G e t D o f V a l u e                                           */
/* ------------------------------------------------------------------------ */

gScalar Dof_GetDofValue(struct DofData * DofData_P, struct Dof * Dof_P)
{
  gScalar tmp ;

  switch (Dof_P->Type) {

  case DOF_UNKNOWN :
    if(!DofData_P->CurrentSolution->SolutionExist){
      Msg::Error("Empty solution in DofData %d", DofData_P->Num);
    }
    LinAlg_GetScalarInVector(&tmp, &DofData_P->CurrentSolution->x, 
			     Dof_P->Case.Unknown.NumDof-1) ;
    break ;

  case DOF_FIXED :
  case DOF_FIXEDWITHASSOCIATE :
    LinAlg_ProdScalarDouble(&Dof_P->Val, 
			    ((Dof_P->Case.FixedAssociate.TimeFunctionIndex)?
			     DofData_P->CurrentSolution->TimeFunctionValues
			     [Dof_P->Case.FixedAssociate.TimeFunctionIndex] :
			     1.),
			    &tmp);
    break ;

  case DOF_LINK :
    tmp = Dof_GetDofValue(DofData_P, Dof_P->Case.Link.Dof) ;
    LinAlg_ProdScalarDouble(&tmp, Dof_P->Case.Link.Coef, &tmp) ;
    break ;

  case DOF_LINKCPLX :
    /* Too soon to treat LinkCplx: we need the real and imaginary parts */
    Msg::Error("Cannot call Dof_GetDofValue for LinkCplx");
    break ;

  default :
    LinAlg_ZeroScalar(&tmp) ;
    break ;
  }

  return tmp ;
}

void Dof_GetRealDofValue(struct DofData * DofData_P, struct Dof * Dof_P, double *d)
{
  gScalar tmp ;

  if (Dof_P->Type == DOF_LINKCPLX) {
    Msg::Error("Cannot call Dof_GetRealDofValue for LinkCplx");
  }

  tmp = Dof_GetDofValue(DofData_P, Dof_P) ;
  LinAlg_GetDoubleInScalar(d, &tmp) ;
}

void Dof_GetComplexDofValue(struct DofData * DofData_P, struct Dof * Dof_P, 
			    double *d1, double *d2)
{
  gScalar tmp1, tmp2 ;
  double valtmp[2] ;

  if(gSCALAR_SIZE == 1){
    if(Dof_P->Type == DOF_LINKCPLX) { /* Can only be done here */
      if (Dof_P->Case.Link.Dof->Type == DOF_LINKCPLX) { /* recurse */
	Dof_GetComplexDofValue(DofData_P, Dof_P->Case.Link.Dof, d1, d2);
      }
      else{
	tmp1 = Dof_GetDofValue(DofData_P, Dof_P->Case.Link.Dof) ;
	tmp2 = Dof_GetDofValue(DofData_P, (Dof_P+1)->Case.Link.Dof) ;
	LinAlg_GetDoubleInScalar(d1, &tmp1) ;
	LinAlg_GetDoubleInScalar(d2, &tmp2) ;
      }
    }
    else{
      tmp1 = Dof_GetDofValue(DofData_P, Dof_P) ;
      tmp2 = Dof_GetDofValue(DofData_P, Dof_P+1) ;
      LinAlg_GetDoubleInScalar(d1, &tmp1) ;
      LinAlg_GetDoubleInScalar(d2, &tmp2) ;
    }
  }
  else{
    if (Dof_P->Type == DOF_LINKCPLX) { /* Can only be done here */
      if (Dof_P->Case.Link.Dof->Type == DOF_LINKCPLX) { /* recurse */
	Dof_GetComplexDofValue(DofData_P, Dof_P->Case.Link.Dof, d1, d2);
      }
      else{
	tmp1 = Dof_GetDofValue(DofData_P, Dof_P->Case.Link.Dof) ; 
	LinAlg_GetComplexInScalar(d1, d2, &tmp1) ;
      }
    }
    else{
      tmp1 = Dof_GetDofValue(DofData_P, Dof_P) ; 
      LinAlg_GetComplexInScalar(d1, d2, &tmp1) ;
    }
  }

  if(Dof_P->Type == DOF_LINKCPLX){
    valtmp[0] = Dof_P->Case.Link.Coef*(*d1) - Dof_P->Case.Link.Coef2*(*d2) ;
    valtmp[1] = Dof_P->Case.Link.Coef*(*d2) + Dof_P->Case.Link.Coef2*(*d1) ;
    *d1 = valtmp[0] ;
    *d2 = valtmp[1] ; 
  }
}

/* ------------------------------------------------------------------------- */
/*  D o f _ D e f i n e Unknown D o f F r o m Solve o r Init D o f           */
/* ------------------------------------------------------------------------- */

void Dof_DefineUnknownDofFromSolveOrInitDof(struct DofData ** DofData_P)
{
  int         i, Nbr_AnyDof ;
  struct Dof  * Dof_P ;

  Nbr_AnyDof = List_Nbr((*DofData_P)->DofList) ;

  for(i = 0 ; i < Nbr_AnyDof ; i++) {

    Dof_P = (struct Dof*)List_Pointer((*DofData_P)->DofList, i) ;

    switch (Dof_P->Type) {
    case DOF_FIXED_SOLVE :
    case DOF_FIXEDWITHASSOCIATE_SOLVE :
      Dof_P->Type = DOF_UNKNOWN ;
      Dof_P->Case.Unknown.NumDof = ++((*DofData_P)->NbrDof) ;
      break ;
    case DOF_UNKNOWN_INIT :
      Dof_P->Type = DOF_UNKNOWN ;
      break ;
    }
  }
}

/* ------------------------------------------------------------------------ */
/*  D o f _ T r a n s f e r D o f                                           */
/* ------------------------------------------------------------------------ */

void Dof_TransferDof(struct DofData  * DofData_P1, 
		     struct DofData ** DofData_P2)
{
  int              i, Nbr_AnyDof ;
  struct Dof       Dof, * Dof_P ;
  struct Solution  * Solutions_P0 ;

  Nbr_AnyDof   = List_Nbr(DofData_P1->DofList) ;
  Solutions_P0 = (struct Solution*)List_Pointer(DofData_P1->Solutions, 0) ;
  DofData_P1->CurrentSolution = Solutions_P0 ;

  for(i = 0; i < Nbr_AnyDof; i++) {
    Dof = *(struct Dof *)List_Pointer(DofData_P1->DofList, i) ;

    if((Dof_P = (struct Dof*)Tree_PQuery((*DofData_P2)->DofTree, &Dof))){

      switch (Dof_P->Type) {
      case DOF_FIXED_SOLVE :
	Dof_P->Type = DOF_FIXED ;
	Dof_P->Val = Dof_GetDofValue(DofData_P1, &Dof) ;
	break ;
      case DOF_FIXEDWITHASSOCIATE_SOLVE :
	Dof_P->Type = DOF_FIXEDWITHASSOCIATE ;
	Dof_P->Val = Dof_GetDofValue(DofData_P1, &Dof) ;
	break ;
      case DOF_UNKNOWN_INIT :
	Dof_P->Val = Dof_GetDofValue(DofData_P1, &Dof) ;
	break ;

	/* Un DOF_UNKNOWN_INIT prendra toujours une valeur obtenue par
	   pre-resolution, meme si on ne demande qu'une simple initialisation ...
	   Pourquoi pas definir un type DOF_UNKNOWN_INIT_SOLVE,
	   propre a Dof_DefineInitSolveDof() ? ... */

      }
    }
  }
}

/* ------------------------------------------------------------------------ */
/*  D o f _ I n i t D o f F o r N o D o f                                   */
/* ------------------------------------------------------------------------ */

void Dof_InitDofForNoDof(struct Dof * DofForNoDof, int NbrHar)
{
  int k ;
  double Val[2] = {1.,0.} ;

  for (k=0 ; k<NbrHar ; k+=gSCALAR_SIZE) {
    DofForNoDof[k].Type = DOF_FIXED ;
    LinAlg_SetScalar(&DofForNoDof[k].Val, &Val[k%2]) ;
    DofForNoDof[k].Case.FixedAssociate.TimeFunctionIndex = 0 ;
  }
}

/* ------------------------------------------------------- */
/*  P r i n t _  D o f N u m b e r                         */
/* ------------------------------------------------------- */

void Print_DofNumber(struct Dof *Dof_P)
{
  switch(Dof_P->Type){
  case DOF_UNKNOWN : 
    printf("%d(%d) ", Dof_P->Case.Unknown.NumDof, Dof_P->Entity) ; 
    break ;
  case DOF_FIXED   : 
    printf("Fixed(%d) ", Dof_P->Entity) ; 
    break ;
  case DOF_FIXEDWITHASSOCIATE : 
    printf("Assoc-%d ", Dof_P->Case.FixedAssociate.NumDof) ; 
    break ;
  case DOF_LINK : 
    printf("Link-");
    Print_DofNumber(Dof_P->Case.Link.Dof);
    break ;
  case DOF_LINKCPLX : 
    printf("LinkCplx-");
    Print_DofNumber(Dof_P->Case.Link.Dof);
    break ;
  default : 
    printf(" ? ") ; 
    break ;
  }
}

/* ------------------------------------------------------- */
/*  D u m m y  D o f s                                     */
/* ------------------------------------------------------- */

void Dof_GetDummies(struct DefineSystem * DefineSystem_P, struct DofData * DofData_P)
{
  struct Formulation      * Formulation_P ; 
  struct DefineQuantity   * DefineQuantity_P ;
  struct FunctionSpace    * FunctionSpace_P ;
  struct BasisFunction    * BasisFunction_P ;
  struct GlobalQuantity   * GlobalQuantity_P ;
  struct Dof              * Dof_P ;

  int i, j, k, l, iDof, ii,iit, iNum, iHar;
  int Nbr_Formulation, Index_Formulation ;
  int * DummyDof;
  double DummyFrequency, * Val_Pulsation;

  if (!(Val_Pulsation = Current.DofData->Val_Pulsation))
    Msg::Error("Dof_GetDummies can only be used for harmonic problems");

  DummyDof = DofData_P->DummyDof = (int *)Malloc(DofData_P->NbrDof*sizeof(int)); 
  for (iDof = 0 ; iDof < DofData_P->NbrDof ; iDof++) DummyDof[iDof]=0;

  Nbr_Formulation = List_Nbr(DefineSystem_P->FormulationIndex) ;

  for (i = 0 ; i < Nbr_Formulation ; i++) {
    List_Read(DefineSystem_P->FormulationIndex, i, &Index_Formulation) ;
    Formulation_P = (struct Formulation*)
      List_Pointer(Problem_S.Formulation, Index_Formulation) ;
    for (j = 0 ; j < List_Nbr(Formulation_P->DefineQuantity) ; j++) {
      DefineQuantity_P = (struct DefineQuantity*)
	List_Pointer(Formulation_P->DefineQuantity, j) ;
      for (l = 0 ; l < List_Nbr(DefineQuantity_P->DummyFrequency) ; l++) {
	DummyFrequency = *(double *)List_Pointer(DefineQuantity_P->DummyFrequency, l) ;

	iHar=-1;
	for (k = 0 ; k < Current.NbrHar/2 ; k++)      
	  if (fabs (Val_Pulsation[k]-TWO_PI*DummyFrequency) <= 1e-10 * Val_Pulsation[k]) {
	    iHar = 2*k; break;
	  }
	if(iHar>=0) {
	  FunctionSpace_P = (struct FunctionSpace*)
	    List_Pointer(Problem_S.FunctionSpace, DefineQuantity_P->FunctionSpaceIndex) ;
	  
	  for (k = 0 ; k < List_Nbr(FunctionSpace_P->BasisFunction) ; k++) {
	    BasisFunction_P = (struct BasisFunction *)
	      List_Pointer(FunctionSpace_P->BasisFunction, k) ;
	    iNum = ((struct BasisFunction *)BasisFunction_P)->Num;
	    ii=iit=0;
	    for (iDof = 0 ; iDof < List_Nbr(DofData_P->DofList) ; iDof++) { 
	      Dof_P = (struct Dof *)List_Pointer(DofData_P->DofList, iDof) ;
	      if (Dof_P->Type == DOF_UNKNOWN && Dof_P->NumType == iNum) {
		iit++;
		if (Dof_P->Harmonic == iHar || Dof_P->Harmonic == iHar+1) { 
		  DummyDof[Dof_P->Case.Unknown.NumDof-1]=1; ii++;
		}
	      }
	    }
	    Msg::Info("Freq %e (%d/%d) Form %d  Quant %d  Basis %d  #dummies %d/%d", 
		Val_Pulsation[iHar/2]/TWO_PI, iHar/2, Current.NbrHar/2, 
		i, j, ((struct BasisFunction *)BasisFunction_P)->Num, ii, iit) ;
	  }
	  
	  for (k = 0 ; k < List_Nbr(FunctionSpace_P->GlobalQuantity) ; k++) {
	    GlobalQuantity_P = (struct GlobalQuantity *)
	      List_Pointer(FunctionSpace_P->GlobalQuantity, k) ;
	    iNum = ((struct GlobalQuantity *)GlobalQuantity_P)->Num;
	    ii=iit=0;    
	    for (iDof = 0 ; iDof < List_Nbr(DofData_P->DofList) ; iDof++) { 
	      Dof_P = (struct Dof *)List_Pointer(DofData_P->DofList, iDof) ;
	      if (Dof_P->Type == DOF_UNKNOWN && Dof_P->NumType == iNum) {
		iit++;
		if (Dof_P->Harmonic == iHar || Dof_P->Harmonic == iHar+1) {  
		  DummyDof[Dof_P->Case.Unknown.NumDof-1]=1; ii++;
		}
	      }
	    }
	    Msg::Info("Freq %e (%d/%d) Form %d  Quant %d  Global %d  #dummies %d/%d", 
		Val_Pulsation[iHar/2]/TWO_PI, iHar/2, Current.NbrHar/2, 
		i, j, ((struct GlobalQuantity *)GlobalQuantity_P)->Num, ii, iit) ;
	  }
	    
	}   /*  end DummyFrequency in DofData */ 
      }   /* end DummyFrequency in Quantity */
    }   /* end Quantity */
  }   /* end Formulation */

  i=0;
  for (iDof = 0 ; iDof < DofData_P->NbrDof ; iDof++) {
    if(DummyDof[iDof]) i++;
    /*    
    Dof_P = (struct Dof *)List_Pointer(DofData_P->DofList, iDof) ;
    Msg::Info("Dof Num iHar, Entity %d %d %d",
	iDof, Dof_P->NumType, Dof_P->Harmonic, Dof_P->Entity);
    */
  }

  Msg::Info("Total %d Dummies %d", DofData_P->NbrDof,i) ;
}
