#define RCSID "$Id: Pos_Format.c,v 1.12 2000-11-25 23:09:38 geuzaine Exp $"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "GetDP.h"
#include "GeoData.h"
#include "Data_Passive.h"
#include "Data_DefineE.h"
#include "Data_Numeric.h"
#include "Version.h"
#include "CurrentData.h"
#include "Magic.h"
#include "Pos_Iso.h"
#include "Pos_Format.h"
#include "Pos_Element.h"
#include "F_Function.h"
#include "Cal_Value.h"

#define NBR_MAX_ISO  200

List_T *PostElement_L = NULL ;
List_T *TimeValue_L = NULL ;

int     Gmsh_StartNewView = 0 ;
int     NbSP, NbVP, NbTP, NbSL, NbVL, NbTL, NbST, NbVT, NbTT, NbSS, NbVS, NbTS ;
List_T *SP = NULL, *VP = NULL, *TP = NULL;
List_T *SL = NULL, *VL = NULL, *TL = NULL;
List_T *ST = NULL, *VT = NULL, *TT = NULL;
List_T *SS = NULL, *VS = NULL, *TS = NULL;

/* ------------------------------------------------------------------------ */
/*  F o r m a t _ P o s t F o r m a t / H e a d e r / F o o t e r           */
/* ------------------------------------------------------------------------ */

void  Format_PostFormat(int Format){
  
  GetDP_Begin("Format_PostFormat");

  switch(Format){
  case FORMAT_GMSH :
    fprintf(PostStream, "$PostFormat /* Gmsh 0.995, %s */\n",
	    Flag_BIN ? "binary" : "ascii") ;
    fprintf(PostStream, "0.995 %d\n", Flag_BIN) ;
    fprintf(PostStream, "$EndPostFormat\n") ;
    break ;
  case FORMAT_GNUPLOT :
    fprintf(PostStream, "# GetDP v%g, %s\n",
	    GETDP_VERSION, Flag_BIN ? "binary" : "ascii") ;
    break ;
  }
  
  GetDP_End ;
}

void  Format_PostHeader(int Format, int Contour, 
			int NbTimeStep, int HarmonicToTime,
			int Type, int Order,
			struct PostQuantity  *NCPQ_P,
			struct PostQuantity  *CPQ_P){

  char name[MAX_STRING_LENGTH] ;

  GetDP_Begin("Format_PostHeader");

  if(Contour){
    if(!PostElement_L) 
      PostElement_L = List_Create(20, 20, sizeof(struct PostElement*));
    else
      List_Reset(PostElement_L);
  }

  if(NCPQ_P && CPQ_P) {
    strcpy(name, Order ? NCPQ_P->Name : CPQ_P->Name) ;
    strcat(name, Get_StringForDefine(PostSubOperation_CombinationType, Type)) ;
    strcat(name, Order ? CPQ_P->Name : NCPQ_P->Name) ;
  }
  else if(NCPQ_P)
    strcpy(name, NCPQ_P->Name) ;
  else
    strcpy(name, CPQ_P->Name) ;

  switch(Format){
  case FORMAT_GMSH_PARSED :
    fprintf(PostStream, "View \"%s\" {\n", name) ;
    break ;
  case FORMAT_GMSH :
    fprintf(PostStream, "$View /* %s */\n", name);
    fprintf(PostStream, "%s ", name);
    Gmsh_StartNewView = 1 ;
    break ;
  case FORMAT_GNUPLOT :
    fprintf(PostStream, "# PostData '%s'\n", name);
    fprintf(PostStream, "# Type Num  X Y Z  N1 N2 N3  Values  <Values>...\n");
    break ;
  case FORMAT_ADAPT :
    fprintf(PostStream, "$Adapt /* %s */\n", name) ;
    break ;
  }

  GetDP_End ;
}

void  Format_PostFooter(struct PostSubOperation *PSO_P, int Store){
  List_T  * Iso_L[NBR_MAX_ISO] ;
  double    IsoMin = 1.e200, IsoMax = -1.e200, IsoVal = 0.0 ;
  int       NbrIso = 0, DecomposeInSimplex = 0 ; 
  int       iPost, iNode, iIso, i, f ;
  struct PostElement *PE ;

  GetDP_Begin("Format_PostFooter");

  if(PSO_P->Iso){

    if(PSO_P->Format == FORMAT_GMSH || PSO_P->Format == FORMAT_GMSH_PARSED)
      DecomposeInSimplex = 1 ;

    for(iPost = 0 ; iPost < List_Nbr(PostElement_L) ; iPost++){ 
      PE = *(struct PostElement**)List_Pointer(PostElement_L, iPost);
      for (iNode = 0 ; iNode < PE->NbrNodes ; iNode++ ){
	IsoMin = MIN(IsoMin, PE->Value[iNode].Val[0]) ;
	IsoMax = MAX(IsoMax, PE->Value[iNode].Val[0]) ;
      }
    }
    if((NbrIso = PSO_P->Iso) < 0)
      NbrIso = List_Nbr(PSO_P->Iso_L) ;
    if(NbrIso > NBR_MAX_ISO) 
      Msg(ERROR, "Too Many Iso Values");
    for(iIso = 0 ; iIso < NbrIso ; iIso++)
      Iso_L[iIso] = List_Create(10, 10, sizeof(struct PostElement*)) ;

    for(iPost = 0 ; iPost < List_Nbr(PostElement_L) ; iPost++){ 
      PE = *(struct PostElement**)List_Pointer(PostElement_L, iPost);
      for(iIso = 0 ; iIso < NbrIso ; iIso++){
	if(PSO_P->Iso > 0){
	  Cal_Iso(PE, Iso_L[iIso], IsoMin+iIso*(IsoMax-IsoMin)/(double)(NbrIso-1), 
		  IsoMin, IsoMax, DecomposeInSimplex) ;
	}
	else{
	  List_Read(PSO_P->Iso_L, iIso, &IsoVal) ;
	  Cal_Iso(PE, Iso_L[iIso], IsoVal, IsoMin, IsoMax, DecomposeInSimplex) ;
	}
      }
      if(!Store) Destroy_PostElement(PE);
    }

    for(iIso = 0 ; iIso < NbrIso ; iIso++){
      for(iPost = 0 ; iPost < List_Nbr(Iso_L[iIso]) ; iPost++){
	PE = *(struct PostElement**)List_Pointer(Iso_L[iIso], iPost) ;
	Format_PostElement(PSO_P->Format, 0, 0, 
			   Current.Time, 0, 1, 
			   Current.NbrHar, PSO_P->HarmonicToTime, 
			   NULL, PE);
	Destroy_PostElement(PE) ;
      }
      List_Delete(Iso_L[iIso]) ;
      if(PSO_P->Format == FORMAT_GNUPLOT) fprintf(PostStream, "\n") ;
    }
  }

  switch(PSO_P->Format){
  case FORMAT_GMSH_PARSED :
    fprintf(PostStream, "};\n") ;
    break ;
  case FORMAT_GMSH :
    fprintf(PostStream, "%d %d %d %d %d %d %d %d %d %d %d %d %d\n", 
	    List_Nbr(TimeValue_L),
	    NbSP, NbVP, NbTP, NbSL, NbVL, NbTL, 
	    NbST, NbVT, NbTT, NbSS, NbVS, NbTS);
    f = Flag_BIN ? LIST_FORMAT_BINARY : LIST_FORMAT_ASCII ;
    List_WriteToFile(TimeValue_L, PostStream, f); 
    List_WriteToFile(SP, PostStream, f); List_WriteToFile(VP, PostStream, f);
    List_WriteToFile(TP, PostStream, f); List_WriteToFile(SL, PostStream, f);
    List_WriteToFile(VL, PostStream, f); List_WriteToFile(TL, PostStream, f);
    List_WriteToFile(ST, PostStream, f); List_WriteToFile(VT, PostStream, f);
    List_WriteToFile(TT, PostStream, f); List_WriteToFile(SS, PostStream, f);
    List_WriteToFile(VS, PostStream, f); List_WriteToFile(TS, PostStream, f);
    if(Flag_BIN) fprintf(PostStream, "\n");
    fprintf(PostStream, "$EndView\n");
    break ;
  case FORMAT_ADAPT :
    fprintf(PostStream, "$EndAdapt\n");
    break ;
  }

  GetDP_End ;
}

/* ------------------------------------------------------------------------ */
/*  F o r m a t _ G m s h P a r s e d                                       */
/* ------------------------------------------------------------------------ */

void  Format_GmshParsed(int TimeStep, int NbTimeStep, int NbHarmonic, 
			int HarmonicToTime, int Type, int NbrNodes, 
			double *x, double *y, double *z, struct Value *Value){
  int     i,j,k;
  double      TimeMH ;
  struct Value  TmpValue ;

  GetDP_Begin("Format_GmshParsed");

  switch (Value[0].Type) {

  case SCALAR :

    if(TimeStep == 0){
      switch(Type){
      case POINT       : fprintf(PostStream, "SP("); break;
      case LINE        : fprintf(PostStream, "SL("); break;
      case TRIANGLE    : fprintf(PostStream, "ST("); break;
      case TETRAHEDRON : fprintf(PostStream, "SS("); break;
      }    
      for(i = 0 ; i < NbrNodes ; i++){
	if(i) fprintf(PostStream, ",");
	fprintf(PostStream, "%.8g,%.8g,%.8g", x[i], y[i], z[i]);
      }
      fprintf(PostStream, "){");
    }

    if (HarmonicToTime == 1) {
      for(k = 0 ; k < NbHarmonic ; k++) {
	if(k || TimeStep) fprintf(PostStream, ",");
	for(i = 0 ; i < NbrNodes ; i++){
	  if(i) fprintf(PostStream, ",");
	  fprintf(PostStream, "%.8g", Value[i].Val[MAX_DIM*k]);
	}
      }
    }
    else {
      for(k = 0 ; k < HarmonicToTime ; k++){
	if(k || TimeStep) fprintf(PostStream, ",");
	for(i = 0 ; i < NbrNodes ; i++){
	  if(i) fprintf(PostStream, ",");
	  F_MHToTime0(NULL, &Value[i], &TmpValue, k, HarmonicToTime, &TimeMH) ;
	  fprintf(PostStream, "%.8g", TmpValue.Val[0]);
	}
      }
    }

    if(TimeStep == NbTimeStep-1){
      fprintf(PostStream, "};\n") ;
    }
    break ;

  case VECTOR :

    if(TimeStep == 0){
      switch(Type){
      case POINT       : fprintf(PostStream, "VP("); break;
      case LINE        : fprintf(PostStream, "VL("); break;
      case TRIANGLE    : fprintf(PostStream, "VT("); break;
      case TETRAHEDRON : fprintf(PostStream, "VS("); break;
      }
      for(i = 0 ; i < NbrNodes ; i++){
	if(i) fprintf(PostStream, ",");
	fprintf(PostStream, "%.8g,%.8g,%.8g", x[i], y[i], z[i]);
      }
      fprintf(PostStream, "){");
    }
    
    if (HarmonicToTime == 1) {
      for(k = 0 ; k < NbHarmonic ; k++) {
	if(k || TimeStep) fprintf(PostStream, ",");
	for(i = 0 ; i < NbrNodes ; i++){
	  if(i) fprintf(PostStream, ",");
	  for(j = 0 ; j < 3 ; j++){
	    if(j) fprintf(PostStream, ",");
	    fprintf(PostStream, "%.8g", Value[i].Val[MAX_DIM*k+j]);
	  }
	}
      }
    }
    else {
      for(k = 0 ; k < HarmonicToTime ; k++){
	if(k || TimeStep) fprintf(PostStream, ",");
	for(i = 0 ; i < NbrNodes ; i++){
	  if(i) fprintf(PostStream, ",");
	  F_MHToTime0(NULL, &Value[i], &TmpValue, k, HarmonicToTime, &TimeMH) ;
	  for(j = 0 ; j < 3 ; j++){
	    if(j) fprintf(PostStream, ",");
	    fprintf(PostStream, "%.8g", TmpValue.Val[j]);
	  }
	}
      }
    }

    if(TimeStep == NbTimeStep-1){
      fprintf(PostStream, "};\n") ;
    }
    break ;

  case TENSOR_DIAG :
  case TENSOR_SYM :
  case TENSOR :
    Msg(ERROR, "Format_PostElement not done for Tensors");
    break;
    
  }

  GetDP_End ;
}


void  Format_Gmsh(double Time, int TimeStep, int NbTimeStep, int NbHarmonic, 
		  int HarmonicToTime, int Type, int NbrNodes, 
		  double *x, double *y, double *z, struct Value *Value){
  int           i,j,k;
  double        TimeMH ;
  struct Value  TmpValue ;
  List_T       *Current_L ;

  GetDP_Begin("Format_Gmsh");

  if(Gmsh_StartNewView){
    Gmsh_StartNewView = 0 ;
    NbSP = NbVP = NbTP = NbSL = NbVL = NbTL = 0;
    NbST = NbVT = NbTT = NbSS = NbVS = NbTS = 0;
    if(!SP) SP = List_Create(100,1000,sizeof(double)); else List_Reset(SP);
    if(!VP) VP = List_Create(100,1000,sizeof(double)); else List_Reset(VP);
    if(!TP) TP = List_Create(100,1000,sizeof(double)); else List_Reset(TP);
    if(!SL) SL = List_Create(100,1000,sizeof(double)); else List_Reset(SL);
    if(!VL) VL = List_Create(100,1000,sizeof(double)); else List_Reset(VL);
    if(!TL) TL = List_Create(100,1000,sizeof(double)); else List_Reset(TL);
    if(!ST) ST = List_Create(100,1000,sizeof(double)); else List_Reset(ST);
    if(!VT) VT = List_Create(100,1000,sizeof(double)); else List_Reset(VT);
    if(!TT) TT = List_Create(100,1000,sizeof(double)); else List_Reset(TT);
    if(!SS) SS = List_Create(100,1000,sizeof(double)); else List_Reset(SS);
    if(!VS) VS = List_Create(100,1000,sizeof(double)); else List_Reset(VS);
    if(!TS) TS = List_Create(100,1000,sizeof(double)); else List_Reset(TS);
    if(!TimeValue_L) TimeValue_L = List_Create(100,1000,sizeof(double));
    else List_Reset(TimeValue_L);
  }

  switch (Value[0].Type) {

  case SCALAR :

    if(TimeStep == 0){
      switch(Type){
      case POINT       : Current_L = SP ; NbSP++ ; break ;
      case LINE        : Current_L = SL ; NbSL++ ; break ;
      case TRIANGLE    : Current_L = ST ; NbST++ ; break ;
      case TETRAHEDRON : Current_L = SS ; NbSS++ ; break ;
      }    
      for(i = 0 ; i < NbrNodes ; i++) List_Add(Current_L, &x[i]);
      for(i = 0 ; i < NbrNodes ; i++) List_Add(Current_L, &y[i]);
      for(i = 0 ; i < NbrNodes ; i++) List_Add(Current_L, &z[i]);
    }
    List_Put(TimeValue_L, TimeStep, &Time);
    if (HarmonicToTime == 1)
      for(k = 0 ; k < NbHarmonic ; k++)
	for(i = 0 ; i < NbrNodes ; i++)
	  List_Add(Current_L, &Value[i].Val[MAX_DIM*k]);
    else
      for(k = 0 ; k < HarmonicToTime ; k++)
	for(i = 0 ; i < NbrNodes ; i++){
	  F_MHToTime0(NULL, &Value[i], &TmpValue, k, HarmonicToTime, &TimeMH) ;
	  List_Add(Current_L, &TmpValue.Val[0]);
	}

    break ;

  case VECTOR :

    if(TimeStep == 0){
      switch(Type){
      case POINT       : Current_L = VP ; NbVP++ ; break ;
      case LINE        : Current_L = VL ; NbVL++ ; break ;
      case TRIANGLE    : Current_L = VT ; NbVT++ ; break ;
      case TETRAHEDRON : Current_L = VS ; NbVS++ ; break ;
      }    
      for(i = 0 ; i < NbrNodes ; i++) List_Add(Current_L, &x[i]);
      for(i = 0 ; i < NbrNodes ; i++) List_Add(Current_L, &y[i]);
      for(i = 0 ; i < NbrNodes ; i++) List_Add(Current_L, &z[i]);
    }
    if (HarmonicToTime == 1)
      for(k = 0 ; k < NbHarmonic ; k++)
	for(i = 0 ; i < NbrNodes ; i++)
	  for(j = 0 ; j < 3 ; j++)
	    List_Add(Current_L, &Value[i].Val[MAX_DIM*k+j]);
    else
      for(k = 0 ; k < HarmonicToTime ; k++)
	for(i = 0 ; i < NbrNodes ; i++){
	  F_MHToTime0(NULL, &Value[i], &TmpValue, k, HarmonicToTime, &TimeMH) ;
	  for(j = 0 ; j < 3 ; j++)
	    List_Add(Current_L, &TmpValue.Val[j]);
	}
    break ;

  case TENSOR_DIAG :
  case TENSOR_SYM :
  case TENSOR :
    Msg(ERROR, "Gmsh Format not done for Tensors");
    break;
    
  }

  GetDP_End ;
}

/* ------------------------------------------------------------------------ */
/*  F o r m a t _ G n u p l o t                                             */
/* ------------------------------------------------------------------------ */

int Get_GmshElementType(int Type){

  GetDP_Begin("Get_GmshElementType");

  switch(Type){
  case POINT :         GetDP_Return(15) ; 
  case LINE :	       GetDP_Return(1)  ; 
  case TRIANGLE :      GetDP_Return(2)  ; 
  case QUADRANGLE :    GetDP_Return(3)  ; 
  case TETRAHEDRON :   GetDP_Return(4)  ;    
  case HEXAHEDRON :    GetDP_Return(5)  ; 
  case PRISM :	       GetDP_Return(6)  ; 
  case PYRAMID :       GetDP_Return(7)  ; 
  case LINE_2 :	       GetDP_Return(8)  ; 
  case TRIANGLE_2 :    GetDP_Return(9)  ; 
  case QUADRANGLE_2 :  GetDP_Return(10) ; 
  case TETRAHEDRON_2 : GetDP_Return(11) ;      
  case HEXAHEDRON_2 :  GetDP_Return(12) ; 
  case PRISM_2 :       GetDP_Return(13) ; 
  case PYRAMID_2 :     GetDP_Return(14) ; 
  default : 
    Msg(ERROR, "Unknown Type in Get_GmshElementType") ;
    GetDP_Return(-1) ;
  }
}

void  Format_Gnuplot(int Format, double Time, int TimeStep, int NbrTimeSteps,
		     int NbrHarmonics, int HarmonicToTime, 
		     int ElementType, int NumElement, int NbrNodes,
		     double *x, double *y, double *z, double *Dummy,
		     struct Value *Value){
  static int  Size, TmpIndex ;
  static double  * TmpValues ;
  int         i, j, k, t, i2, k2 ;
  double      TimeMH ;
  struct Value  TmpValue ;

  GetDP_Begin("Format_Gnuplot");

  if(TimeStep == 0){
    switch(Value->Type){
    case SCALAR      : Size = 1 ; break ;
    case VECTOR      : Size = 3 ; break ;
    case TENSOR_DIAG : Size = 3 ; break ;
    case TENSOR_SYM  : Size = 6 ; break ;
    case TENSOR      : Size = 9 ; break ;
    }
    TmpValues = (double*) Malloc(NbrTimeSteps*NbrNodes*NbrHarmonics*Size*sizeof(double));
    TmpIndex = 0;
  }

  for(i = 0 ; i < NbrNodes ; i++)
    for(k = 0 ; k < NbrHarmonics ; k++)
      for(j = 0 ; j < Size ; j++)
	TmpValues[TmpIndex++] = Value[i].Val[MAX_DIM*k+j];

  if(TimeStep == NbrTimeSteps-1){

    for(i = 0 ; i <= NbrNodes ; i++){ /* New line for each node, closed loop for tri/qua */
      
      if(i != NbrNodes)
	i2 = i ;
      else{
	if(NbrNodes < 3) break ;
	else i2 = 0 ;
      }

      fprintf(PostStream, "%d %d ", Get_GmshElementType(ElementType), NumElement);
      fprintf(PostStream, " %.8g %.8g %.8g ", x[i2], y[i2], z[i2]);
      if(Dummy){
	if(Dummy[3]<0){
	  if(!i)
	    fprintf(PostStream, " %.8g %.8g 0 ", Dummy[0], Dummy[2]);
	  else
	    fprintf(PostStream, " %.8g %.8g 0 ", Dummy[1], Dummy[2]);
	}
	else
	  fprintf(PostStream, " %.8g %.8g %.8g ", Dummy[0], Dummy[1], Dummy[2]);
      }
      else
	fprintf(PostStream, " 0 0 0 ");

      for(t = 0 ; t < NbrTimeSteps ; t++){

	if (HarmonicToTime == 1) {
	  for(k = 0 ; k < NbrHarmonics ; k++) {
	    for(j = 0 ; j < Size ; j++){
	      fprintf(PostStream, " %.8g", 
		      TmpValues[ t*NbrNodes*NbrHarmonics*Size
			       + i2*NbrHarmonics*Size
			       + k*Size
			       + j ]);
	    }
	    fprintf(PostStream, " ");
	  }
	}
	else {
	  TmpValue.Type = Value->Type ;
	  for(k = 0 ; k < HarmonicToTime ; k++){

	    for(k2 = 0 ; k2 < NbrHarmonics ; k2++)
	      for(j = 0 ; j < Size ; j++)
		TmpValue.Val[MAX_DIM*k2+j] =
		  TmpValues[ t*NbrNodes*NbrHarmonics*Size
			   + i2*NbrHarmonics*Size
			   + k2*Size
			   + j ] ;

	    F_MHToTime0(NULL, &TmpValue, &TmpValue, k, HarmonicToTime, &TimeMH) ;
	    for(j = 0 ; j < Size ; j++)
	      fprintf(PostStream, "%.8g", TmpValue.Val[0]);
	    fprintf(PostStream, " ");
	  }
	}
	fprintf(PostStream, " ");

      } /* for t */
      fprintf(PostStream, "\n");      

    } /* for i */
    if(NbrNodes > 1) fprintf(PostStream, "\n");

    Free(TmpValues);
  }

  GetDP_End ;
}

/* ------------------------------------------------------------------------ */
/*  F o r m a t _ T a b u l a r                                             */
/* ------------------------------------------------------------------------ */

void  Format_Tabular(int Format, double Time, int TimeStep, int NbrTimeSteps,
		     int NbrHarmonics, int HarmonicToTime, 
		     int ElementType, int NumElement, int NbrNodes,
		     double *x, double *y, double *z, double *Dummy,
		     struct Value *Value){
  static int  Size ;
  int         i,j,k ;
  double      TimeMH ;
  struct Value  TmpValue ;

  GetDP_Begin("Format_Tabular");

  if(TimeStep == 0){
    switch(Value->Type){
    case SCALAR      : Size = 1 ; break ;
    case VECTOR      : Size = 3 ; break ;
    case TENSOR_DIAG : Size = 3 ; break ;
    case TENSOR_SYM  : Size = 6 ; break ;
    case TENSOR      : Size = 9 ; break ;
    }
  }

  switch(Format){
  case FORMAT_SPACE_TABLE :
    if(TimeStep == 0){
      fprintf(PostStream, "%d %d ", Get_GmshElementType(ElementType), NumElement);
      for(i=0 ; i<NbrNodes ; i++)
	fprintf(PostStream, " %.8g %.8g %.8g ", x[i], y[i], z[i]);
      if(Dummy) 
	fprintf(PostStream, " %.8g %.8g %.8g ", Dummy[0], Dummy[1],  Dummy[2]);
      else
	fprintf(PostStream, " 0 0 0 ");
    }
    break ;
  case FORMAT_TIME_TABLE :
    fprintf(PostStream, "%d %.8g ", TimeStep, Time);
    for(i=0 ; i<NbrNodes ; i++)
      fprintf(PostStream, " %.8g %.8g %.8g ", x[i], y[i], z[i]);
    break ;
  case FORMAT_HARMONICTOTIME_TABLE : /* Time not known jet */
    break ;
  }
  
  if (HarmonicToTime == 1) {
    for(k = 0 ; k < NbrHarmonics ; k++) {
      for(i = 0 ; i < NbrNodes ; i++){
	for(j = 0 ; j < Size ; j++){
	  fprintf(PostStream, " %.8g", Value[i].Val[MAX_DIM*k+j]);
	}
	fprintf(PostStream, " ");
      }
      fprintf(PostStream, " ");
    }
  }
  else {
    for(k = 0 ; k < HarmonicToTime ; k++){
      for(i = 0 ; i < NbrNodes ; i++){
	F_MHToTime0(NULL, &Value[i], &TmpValue, k, HarmonicToTime, &TimeMH) ;
	if (!i && Format == FORMAT_HARMONICTOTIME_TABLE) {
	  fprintf(PostStream, "%d %.8g ", TimeStep, TimeMH);
	  for(i=0 ; i<NbrNodes ; i++)
	    fprintf(PostStream, " %.8g %.8g %.8g ", x[i], y[i], z[i]);
	}
	for(j = 0 ; j < Size ; j++)
	  fprintf(PostStream, " %.8g", TmpValue.Val[j]) ;
	fprintf(PostStream, " ");
      }
      if(Format == FORMAT_HARMONICTOTIME_TABLE)
	fprintf(PostStream, "\n");
    }
  }

  if(TimeStep == NbrTimeSteps-1 || Format == FORMAT_TIME_TABLE)
    fprintf(PostStream, "\n");

  GetDP_End ;
}

void  Format_Adapt(double * Dummy){

  GetDP_Begin("Format_Adapt");

  if(Dummy[4]) fprintf(PostStream, "%d\n", (int)Dummy[4]) ;
  fprintf(PostStream, "%d %g %g %g\n", 
	  (int)Dummy[0], Dummy[1], Dummy[2], Dummy[3]);

  GetDP_End ;
}

/* ------------------------------------------------------------------------ */
/*  F o r m a t _ P o s t E l e m e n t                                     */
/* ------------------------------------------------------------------------ */

void  Format_PostElement(int Format, int Contour, int Store, 
			 double Time, int TimeStep, int NbTimeStep, 
			 int NbrHarmonics, int HarmonicToTime, double *Dummy,
			 struct PostElement * PE){

  int Num_Element ;
  static int Warning_FirstHarmonic = 0 ;
  struct PostElement *PE2 ;

  GetDP_Begin("Format_PostElement");

  Num_Element = Geo_GetGeoElement(PE->Index)->Num ;

  if(Contour){
    if(PE->Value[0].Type != SCALAR)
      Msg(ERROR, "Non Scalar Element %d in Contour Creation", Num_Element);
    if(NbTimeStep != 1)
      Msg(ERROR, "Contour Creation not Allowed for Multiple Time Steps");
    if(Current.NbrHar != 1 && !Warning_FirstHarmonic){
      Msg(WARNING, "Contour Creation done only for First Harmonic: Please use Re[] or Im[]");
      Warning_FirstHarmonic = 1 ;
    }
    if(Store)
      List_Add(PostElement_L, &PE) ;
    else{
      PE2 = PartialCopy_PostElement(PE) ;
      List_Add(PostElement_L, &PE2) ;
    }
    GetDP_End ;
  }

  switch(Format){
  case FORMAT_GMSH_PARSED :
    Format_GmshParsed(TimeStep, NbTimeStep, NbrHarmonics, HarmonicToTime,
		      PE->Type, PE->NbrNodes, PE->x, PE->y, PE->z, 
		      PE->Value) ;
    break ;
  case FORMAT_GMSH :
    Format_Gmsh(Time, TimeStep, NbTimeStep, NbrHarmonics, HarmonicToTime,
		PE->Type, PE->NbrNodes, PE->x, PE->y, PE->z, 
		PE->Value) ;
    break ;
  case FORMAT_GNUPLOT :
    Format_Gnuplot(Format, Time, TimeStep, NbTimeStep, NbrHarmonics, HarmonicToTime,
		   PE->Type, Num_Element, PE->NbrNodes, PE->x, PE->y, PE->z, Dummy, 
		   PE->Value) ;
    break ;
  case FORMAT_SPACE_TABLE :
  case FORMAT_TIME_TABLE :
  case FORMAT_HARMONICTOTIME_TABLE :
    Format_Tabular(Format, Time, TimeStep, NbTimeStep, NbrHarmonics, HarmonicToTime,
		   PE->Type, Num_Element, PE->NbrNodes, PE->x, PE->y, PE->z, Dummy, 
		   PE->Value) ;
    break ;
  case FORMAT_ADAPT:
    Format_Adapt(Dummy) ;
    break ;
  default :
    Msg(ERROR, "Unknown Format in Format_PostElement");
  }

  GetDP_End ;
}


/* ------------------------------------------------------------------------ */
/*  F o r m a t _ P o s t V a l u e                                         */
/* ------------------------------------------------------------------------ */

void  Format_PostValue(int Format,
		       double Time, int iRegion, int NbrRegion,
		       int NbrHarmonics, int HarmonicToTime, int Flag_NoNewLine,
		       struct Value * Value) {

  static int  Size ;
  int  j, k ;
  double TimeMH, Freq ;
  static struct Value  TmpValue, *TmpValues ;

  GetDP_Begin("Format_PostValue");

  if(iRegion == 0){
    switch(Value->Type){
    case SCALAR      : Size = 1 ; break ;
    case VECTOR      : Size = 3 ; break ;
    case TENSOR_DIAG : Size = 3 ; break ;
    case TENSOR_SYM  : Size = 6 ; break ;
    case TENSOR      : Size = 9 ; break ;
    }
    TmpValues = (struct Value*) Malloc(NbrRegion*sizeof(struct Value)) ;
   }

  Cal_CopyValue(Value, &TmpValues[iRegion]) ;

  if (iRegion == NbrRegion-1) {

    if (HarmonicToTime == 1) {
      switch (Format) {
      case FORMAT_FREQUENCY_TABLE :
	if (NbrHarmonics == 1)
	  Msg(ERROR, "FrequencyTable Format not allowed") ;
	break ;
      default :
	fprintf(PostStream, "%.8g ", Time) ;
	break ;
      }
      for (iRegion = 0 ; iRegion < NbrRegion ; iRegion++)
	for (k = 0 ; k < NbrHarmonics ; k++) {
	  if (Format == FORMAT_FREQUENCY_TABLE && !(k%2)) {
	    Freq = Current.DofData->Val_Pulsation[0] / TWO_PI ;
	    fprintf(PostStream, "%.8g ", Freq) ;
	  }
	  for(j = 0 ; j < Size ; j++)
	    fprintf(PostStream, " %.8g", TmpValues[iRegion].Val[MAX_DIM*k+j]) ;
	}
      if (Flag_NoNewLine)
	fprintf(PostStream, "  ") ;
      else
	fprintf(PostStream, "\n") ;
    }

    else { /* FORMAT_HARMONICTOTIME_TABLE */
      for(k = 0 ; k < HarmonicToTime ; k++) {
	for (iRegion = 0 ; iRegion < NbrRegion ; iRegion++) {
	  F_MHToTime0(NULL, &TmpValues[iRegion], &TmpValue, 
		      k, HarmonicToTime, &TimeMH) ;
	  if (iRegion == 0)  fprintf(PostStream, "%.8g ", TimeMH) ;
	  for(j = 0 ; j < Size ; j++)
	    fprintf(PostStream, " %.8g", TmpValue.Val[j]) ;
	}
	fprintf(PostStream, "\n") ;
      }
    }

    Free(TmpValues) ;
  }

  GetDP_End ;
}
