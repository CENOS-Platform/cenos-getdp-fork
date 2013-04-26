// GetDP - Copyright (C) 1997-2013 P. Dular, C. Geuzaine
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to the public mailing list <getdp@geuz.org>.

#ifndef _PRO_DATA_H_
#define _PRO_DATA_H_

#include <string>
#include <map>
#include "GetDPConfig.h"
#include "ListUtils.h"

#define _0D      0
#define _1D      1
#define _2D      2
#define _3D      3
#define _ALL     4
#define _AXI     5

#define NBR_MAX_RES           2500
#define NBR_MAX_POS           10

#define _PRE  1 // pre-processing
#define _CAL  2 // processing
#define _POS  3 // post-processing
#define _CST  5 // update constraint

#define TIME_STATIC   1
#define TIME_THETA    2
#define TIME_NEWMARK  3
#define TIME_GEAR     4

#define ASSEMBLY_AGGREGATE  1
#define ASSEMBLY_SEPARATE   2

/* ------------------------------------------------------------------------ */
/*  V a l u e                                                               */
/* ------------------------------------------------------------------------ */

#define MAX_DIM  9   /* second-rank tensor of order 3 : 3^2 = 9  */

/* Please set NBR_MAX_HARMONIC to the lowest possible value for common
   getdp versions, until we do dynamic allocation. Otherwise, some
   postprocessing operations become almost impossible to perform in
   3D.
*/

#if defined(HAVE_MULTIHARMONIC)
#define NBR_MAX_HARMONIC    40
#else
#define NBR_MAX_HARMONIC    2
#endif

#define MAX_STACK_SIZE      40

struct Value {
  int     Type;
  double  Val [NBR_MAX_HARMONIC * MAX_DIM];
};

struct TwoInt { int  Int1, Int2; };

/* ------------------------------------------------------------------------ */
/*  P r o b l e m                                                           */
/* ------------------------------------------------------------------------ */

struct Problem {
  List_T *Group         , *Expression;
  List_T *FunctionSpace , *Constraint        , *Formulation;
  List_T *JacobianMethod, *IntegrationMethod;
  List_T *Resolution    , *PostProcessing    , *PostOperation;
};

/* ------------------------------------------------------------------------ */
/*  G r o u p                                                               */
/* ------------------------------------------------------------------------ */

struct Group {
  char   *Name;
  int     Num,  Type, FunctionType, SuppListType;
  List_T *InitialList, *InitialSuppList;
  List_T *ExtendedList, *ExtendedSuppList;
  std::multimap<int, TwoInt> ExtendedListForSearch;
  struct  MovingBand2D *MovingBand2D;
};

struct MovingBand2D {
  List_T *InitialList1,  *InitialList2;
  List_T *ExtendedList1, *ExtendedList2;
  int NbrNodes1, *NumNodes1, NbrNodes2, *NumNodes2;
  double *x1, *y1, *z1, *x2, *y2, *z2, Area;
  int  Period2, ntr1, ntr2, Closed1, Closed2;
  int PhysNum, StartNumTr, StartIndexTr;
  int *b1_p1, *b1_p2, *b1_p3, *b2_p1, *b2_p2, *b2_p3;
};

/* Group.Type */
#define REGIONLIST   1
#define ELEMENTLIST  2
#define MOVINGBAND2D 3

/* Group.FunctionType */
#define REGION                     1

#define NODESOF                    2
#define EDGESOF                    3
#define FACETSOF                   4
#define VOLUMESOF                  5
#define ELEMENTSOF                 6

#define GLOBAL                     7

#define GROUPSOFNODESOF           11
#define GROUPSOFEDGESOF           12
#define GROUPSOFFACETSOF          13
#define GROUPSOFEDGESONNODESOF    14

#define EDGESOFTREEIN             21
#define FACETSOFTREEIN            22

#define DUALNODESOF               30
#define DUALEDGESOF               31
#define DUALFACETSOF              32
#define DUALVOLUMESOF             33

#define BOUNDARYOFDUALNODESOF     40
#define BOUNDARYOFDUALEDGESOF     41
#define BOUNDARYOFDUALFACETSOF    42



/* Group.SuppListType */
#define SUPPLIST_NONE              0
#define SUPPLIST_NOT               1
#define SUPPLIST_STARTINGON        2
#define SUPPLIST_ONONESIDEOF       3
#define SUPPLIST_INSUPPORT         4
#define SUPPLIST_CONNECTEDTO       5


/* ------------------------------------------------------------------------ */
/*  E x p r e s s i o n                                                     */
/* ------------------------------------------------------------------------ */

struct Expression {
  char *Name;
  int Type;
  union {
    double  Constant;
    List_T *WholeQuantity;
    struct {
      List_T *ExpressionPerRegion;
      int  NumLastRegion;  struct Expression *ExpressionForLastRegion;
    }  PieceWiseFunction;
  }  Case;
};

struct ExpressionPerRegion {
  int  RegionIndex, ExpressionIndex;
};

/* Expression.Type */
#define UNDEFINED_EXP         0
#define CONSTANT              1
#define WHOLEQUANTITY         2
#define PIECEWISEFUNCTION     3


/* ------------------------------------------------------------------------ */
/*  C o n s t r a i n t                                                     */
/* ------------------------------------------------------------------------ */

struct Constraint {
  char   *Name;
  int     Type;
  List_T *ConstraintPerRegion;
  List_T *MultiConstraintPerRegion;
};

struct ConstraintPerRegion {
  int  Type, RegionIndex, SubRegionIndex, TimeFunctionIndex;
  union {
    struct { int  ExpressionIndex; } Fixed;
    struct { char *ResolutionName; } Solve;
    struct { int  Node1, Node2;    } Network;
    struct {
      int  RegionRefIndex, SubRegionRefIndex;
      int  FilterIndex, CoefIndex, FunctionIndex;
      int  FilterIndex2, CoefIndex2, FunctionIndex2;
    } Link;
  } Case;
};


struct MultiConstraintPerRegion {
  char   *Name;
  List_T *ConstraintPerRegion;
  struct ConstraintActive *Active;
};

struct ConstraintActive {
  int TimeStep, SubTimeStep;
  union {
    struct {
      int NbrNode, NbrBranch, NbrLoop;
      int **MatNode, **MatLoop;
    } Network;
    struct {
      List_T *Couples;
    } Link;
  } Case;
};

/* Constraint.Type & ConstraintPerRegion.Type */
#define NONE                  0
#define ASSIGN                1
#define INIT                  2
#define ASSIGNFROMRESOLUTION  3
#define INITFROMRESOLUTION    4
#define NETWORK               5
#define CST_LINK              6
#define CST_LINKCPLX          7


/* ------------------------------------------------------------------------ */
/*  J a c o b i a n M e t h o d                                             */
/* ------------------------------------------------------------------------ */

struct JacobianMethod {
  char   *Name;
  List_T *JacobianCase;
};

struct JacobianCase {
  int  RegionIndex, TypeJacobian;
  int  NbrParameters;  double *Para;
};

/* JacobianCase.TypeJacobian */
/* WARNING! The numbering is important (boundary operator -> -1) */

#define JACOBIAN_PNT                     0
#define JACOBIAN_LIN                     1
#define JACOBIAN_SUR                     2
#define JACOBIAN_VOL                     3

#define JACOBIAN_SUR_AXI                 10
#define JACOBIAN_VOL_AXI                 11

#define JACOBIAN_SUR_AXI_SQU             20
#define JACOBIAN_VOL_AXI_SQU             21

#define JACOBIAN_SUR_SPH_SHELL           30
#define JACOBIAN_VOL_SPH_SHELL           31

#define JACOBIAN_SUR_AXI_SPH_SHELL       40
#define JACOBIAN_VOL_AXI_SPH_SHELL       41

#define JACOBIAN_SUR_AXI_SQU_SPH_SHELL   50
#define JACOBIAN_VOL_AXI_SQU_SPH_SHELL   51

#define JACOBIAN_SUR_RECT_SHELL          60
#define JACOBIAN_VOL_RECT_SHELL          61

#define JACOBIAN_SUR_AXI_RECT_SHELL      70
#define JACOBIAN_VOL_AXI_RECT_SHELL      71

#define JACOBIAN_SUR_AXI_SQU_RECT_SHELL  80
#define JACOBIAN_VOL_AXI_SQU_RECT_SHELL  81

#define JACOBIAN_VOL_PLPD_X              90

#define JACOBIAN_VOL_AXI_PLPD_X          100


/* type of transformation */
#define JACOBIAN_SPH                     0
#define JACOBIAN_RECT                    1

/* ------------------------------------------------------------------------ */
/*  I n t e g r a t i o n   M e t h o d                                     */
/* ------------------------------------------------------------------------ */

struct IntegrationMethod {
  char   *Name;
  List_T *IntegrationCase;
  int     CriterionIndex;
};

struct IntegrationCase {
  int     Type, SubType;
  List_T *Case;
};

/* IntegrationCase.Type */
#define ANALYTIC               1
#define GAUSS                  2
#define GAUSSLEGENDRE          3

/* IntegrationCase.SubType */
#define STANDARD               1
#define SINGULAR               2
#define ADAPTATIVE             3

struct Quadrature {
  int    ElementType;
  int    NumberOfPoints, MaxNumberOfPoints;
  int    NumberOfDivisions, MaxNumberOfDivisions;
  double StoppingCriterion;
  void   (*Function)();
};


/* ------------------------------------------------------------------------ */
/*  F u n c t i o n   S p a c e                                             */
/* ------------------------------------------------------------------------ */

struct FunctionSpace {
  char    *Name;
  int     Type;
  List_T  *BasisFunction, *SubSpace, *GlobalQuantity, *Constraint;

  struct DofData  *DofData, *MainDofData;
};

struct BasisFunction {
  char    *Name, *NameOfCoef;
  List_T  *GlobalBasisFunction;
  int     Dimension, Num;
  void    (*Function)();
  void    (*dFunction)();
  void    (*dInvFunction)();
  List_T  *SubFunction, *SubdFunction;
  int     SupportIndex, EntityIndex;
  double  Order;
  int     ElementType;
  int     Orient;
};

struct GlobalBasisFunction {
  int   EntityIndex; /* Must be the first element of the structure */
  int   FormulationIndex, DefineQuantityIndex, ResolutionIndex;
  struct QuantityStorage  *QuantityStorage;
};

/* BasisFunction.Type */
/* WARNING! The numbering is important (exterior derivative -> +1) */
#define FORM0        0
#define FORM1        1
#define FORM2        2
#define FORM3        3

#define FORM0S       4
#define FORM1S       5
#define FORM2S       6
#define FORM3S       7

#define FORM0P      10
#define FORM1P      11
#define FORM2P      12
#define FORM3P      13

#define SCALAR      20
#define VECTOR      21
#define TENSOR      22  /* second-rank tensor of order 3 */
#define TENSOR_SYM  23
#define TENSOR_DIAG 24

#define TENSOR_MH   25

/*  VECTOR  TENSOR_DIAG  TENSOR_SYM  TENSOR
    |0|     |0    |      |0 1 2|     |0 1 2|
    |1|     |  1  |      |s 3 4|     |3 4 5|
    |2|     |    2|      |s s 5|     |6 7 8| */

#define VECTORP     31

struct SubSpace {
  char   *Name;
  List_T *BasisFunction;
};

struct GlobalQuantity {
  char *Name;
  int   Num,  Type, ReferenceIndex;
};

/* GlobalQuantity.Type */
#define ALIASOF              1
#define ASSOCIATEDWITH       2

struct ConstraintInFS {
  int  QuantityType, ReferenceIndex, EntityIndex;
  struct ConstraintPerRegion *ConstraintPerRegion;
  struct {
    int ResolutionIndex;
    struct ConstraintActive *Active;
  } Active;  /* a deplacer lorsque sera necessaire */
};

/* ConstraintInFS.QuantityType */
#define LOCALQUANTITY       1
#define GLOBALQUANTITY      2
#define INTEGRALQUANTITY    3
#define NODOF               4


/* ------------------------------------------------------------------------ */
/*  F u n c t i o n                                                         */
/* ------------------------------------------------------------------------ */

struct Function {
  void    (*Fct)(); /* ANSI C++ forbids data member `Function' with
			same name as enclosing class */
  int     TypeOfValue, NbrArguments, NbrParameters;
  double *Para;
  struct FunctionActive *Active;
};

struct FunctionActive {
  union {
    struct {
      int     NbrPoint;
      double *x, *y, *xc, *yc;
      double *mi, *bi, *ci, *di;  /* Akima */
    } Interpolation;
    struct {
      double  Value;
    } SurfaceArea;
    struct {
      double  Value;
    } GetVolume;
    struct {
      List_T *Table;
    } ValueFromIndex;
    struct {
      int     NbrLines, NbrColumns;
      double *x, *y ;
      double *data ;
    } ListMatrix;
  } Case;
};

/* ------------------------------------------------------------------------ */
/*  F o r m u l a t i o n                                                   */
/* ------------------------------------------------------------------------ */

struct Formulation {
  char   *Name;
  int     Type;
  List_T *DefineQuantity, *Equation;
};

/* Formulation.Type */
#define FEMEQUATION       1
#define BEMEQUATION       2
#define GLOBALEQUATION    3

struct IntegralQuantity {
  List_T  *WholeQuantity;
  int      DofIndexInWholeQuantity;

  int      TypeOperatorDof, DefineQuantityIndexDof;
  int      DefineQuantityIndexNoDof;

  int      NbrQuantityIndex, *QuantityIndexTable;
  int    *QuantityTraceGroupIndexTable ;

  int      InIndex;
  int      IntegrationMethodIndex, JacobianMethodIndex;
  int      Symmetry;

  int      CanonicalWholeQuantity, ExpressionIndexForCanonical;
  struct   Function FunctionForCanonical, AnyFunction;
};

struct IntegralQuantityActive {
  int     Type_FormDof, Type_ValueDof;

  List_T  *IntegrationCase_L;
  struct  IntegrationCase *IntegrationCase_P;
  int     CriterionIndex;
  void    (*Get_IntPoint)();
  int     Nbr_IntPoints;

  List_T  *JacobianCase_L;
  double  (*Get_Jacobian)();
  int     Type_Dimension;
  void    (*xChangeOfCoordinates)();
};


struct FirstElement {
  struct Dof *Equ;
  struct Dof *Dof;
  double Value;
};

struct DefineQuantity {
  char    *Name;
  int     Type;
  int     FunctionSpaceIndex;

  /* for subspaces */
  List_T  *IndexInFunctionSpace;

  /* for MH calculation */
  List_T  *DummyFrequency;

  /* for multiple DofData vs. one FunctionSpace */
  int      DofDataIndex;
  struct   DofData  *DofData;

  /* for integral quantities */
  struct   IntegralQuantity  IntegralQuantity;
};

// second order hex with 3 BFs per node for elasticity
#define NBR_MAX_BASISFUNCTIONS         81

struct QuantityStorage {

  struct DefineQuantity  *DefineQuantity;

  int    NumLastElementForFunctionSpace;
  int    NumLastElementForDofDefinition;
  int    NumLastElementForEquDefinition;

  struct FunctionSpace   *FunctionSpace;

  int       NbrElementaryBasisFunction;
  int       TypeQuantity;
  struct {
    struct Dof  *Dof;
    int    NumEntityInElement;
    int    CodeBasisFunction, CodeEntity;
    int    CodeAssociateBasisFunction;
    int    Constraint;
    double Value[NBR_MAX_HARMONIC];
    int    TimeFunctionIndex;
    int    CodeEntity_Link;
    struct BasisFunction  *BasisFunction;
  } BasisFunction [NBR_MAX_BASISFUNCTIONS];

};

/* DefineQuantity.Type */
/* LOCALQUANTITY
   GLOBALQUANTITY
   INTEGRALQUANTITY */

struct EquationTerm {
  int        Type;

  union {

    struct FemLocalTerm {
      struct {
	int     TypeTimeDerivative;

	List_T  *WholeQuantity;
	int     DofIndexInWholeQuantity;
	int     CanonicalWholeQuantity, ExpressionIndexForCanonical;
	struct  Function  FunctionForCanonical;
	int     CanonicalWholeQuantity_Equ, ExpressionIndexForCanonical_Equ,
	          OperatorTypeForCanonical_Equ;
	void    (*BuiltInFunction_Equ)();

	int     NbrQuantityIndex, *QuantityIndexTable, QuantityIndexPost;
	int     *QuantityTraceGroupIndexTable;

	int     TypeOperatorEqu, DefineQuantityIndexEqu;
	int     TypeOperatorDof, DefineQuantityIndexDof;
	int     DefineQuantityIndexNoDof, DofInTrace;
      } Term;

      int  InIndex;
      int  Full_Matrix;
      int  IntegrationMethodIndex, JacobianMethodIndex;

      int  ExpressionIndexForMetricTensor;

      int  MatrixIndex;
      struct FemLocalTermActive  *Active;
    } LocalTerm;


    struct FemGlobalTerm {
      int  TypeTimeDerivative;
      int  DefineQuantityIndex;

      struct {
	int  TypeTimeDerivative;

	List_T  *WholeQuantity;
	int     DofIndexInWholeQuantity;
	int     CanonicalWholeQuantity, ExpressionIndexForCanonical;

	int     NbrQuantityIndex, *QuantityIndexTable;
	int     *QuantityTraceGroupIndexTable ;

	int     TypeOperatorEqu, DefineQuantityIndexEqu;
	int     TypeOperatorDof, DefineQuantityIndexDof;
	int     DefineQuantityIndexNoDof;
      } Term;
      int  InIndex;
    } GlobalTerm;


    struct GlobalEquation {
      int  Type, ConstraintIndex;
      List_T  *GlobalEquationTerm;
    } GlobalEquation;

  } Case;
};

struct FemLocalTermActive {
  struct QuantityStorage  *QuantityStorageEqu_P;
  struct QuantityStorage  *QuantityStorageDof_P;
  struct Dof              *DofForNoDof_P;

  int  Type_FormEqu, Type_FormDof, Type_ValueDof;
  int  Type_DefineQuantityDof;
  int  SymmetricalMatrix;

  List_T *IntegrationCase_L, *JacobianCase_L;
  int     CriterionIndex;
  struct JacobianCase  *JacobianCase_P0;
  int    NbrJacobianCase, Flag_ChangeCoord, Flag_InvJac;

  void   (*xChangeOfCoordinatesEqu)();
  void   (*xChangeOfCoordinatesDof)();
  double (*Cal_Productx)();
  void   (*Function_AssembleTerm)();

  struct IntegralQuantityActive  IntegralQuantityActive;


  int MHJacNL, MHJacNL_Index, MHJacNL_NbrPointsX, MHJacNL_HarOffSet;
  double MHJacNL_Factor;
  double **MHJacNL_H, ***MHJacNL_HH, *MHJacNL_t, *MHJacNL_w;

  int Full_Matrix;
  int NbrEqu, NbrHar, *NumEqu, *NumDof;
  struct Dof *Equ, *Dof;
  List_T *FirstElements;
  double **Matrix;
};

struct GlobalQuantityStorage {
  int     NumEquation;
  int     NumDof;

  int     CodeGlobalQuantity, CodeAssociateBasisFunction;

  int     CodeEntity;
  int     Constraint;
  double  Value[NBR_MAX_HARMONIC];
  int     TimeFunctionIndex;
};

struct GlobalEquationTerm {
  int  DefineQuantityIndexNode, DefineQuantityIndexLoop;
  int  DefineQuantityIndexEqu;
  int  InIndex;
};


/* EquationTerm.Type */
#define GALERKIN        1
#define GLOBALTERM      2
#define GLOBALEQUATION  3
#define DERHAM          4

/* Term.TypeOfTimeDerivative */
#define NODT_        0
#define DT_          1
#define DTDOF_       2
#define DTDT_        3
#define DTDTDOF_     4
#define JACNL_      10
#define NEVERDT_    11
#define DTNL_       12
#define DTDOFJACNL_ 13

/* Term.TypeOperator */
#define NOOP       0

#define EXTDER     1
#define GRAD       2
#define CURL       3
#define DIV        4

#define EXTDERINV  5
#define GRADINV    6
#define CURLINV    7
#define DIVINV     8

/* Tous ces operateurs de trace ne servent a RIEN pour le moment
   De plus, les 'x' sont ambigus. Il faut penser a definir des
   operateurs de trace (T ou T*), qui doivent avoir, outre
   \Gamma=\partial\Omega, l'info concernant \Omega.
 */
#define NPx        9
#define NPxEXTDER 10
#define NPxGRAD   11
#define NPxCURL   12
#define NPxDIV    13

#define NSx       14
#define NSxEXTDER 15
#define NSxGRAD   16
#define NSxCURL   17
#define NSxDIV    18

#define _D1       21
#define _D2       22


/* CanonicalWholeQuantity */
#define CWQ_NONE           0
#define CWQ_DOF            1
#define CWQ_EXP_TIME_DOF   2
#define CWQ_FCT_TIME_DOF   3
#define CWQ_FCT_PVEC_DOF   4
#define CWQ_FCT_DOF       20

#define CWQ_GF             5
#define CWQ_GF_PSCA_DOF    6
#define CWQ_GF_PSCA_EXP    7
#define CWQ_GF_PVEC_DOF    8
#define CWQ_DOF_PVEC_GF    9
#define CWQ_GF_PVEC_EXP   10
#define CWQ_EXP_PVEC_GF   11

#define CWQ_EXP_TIME_GF_PSCA_DOF  12
#define CWQ_EXP_TIME_GF_PVEC_DOF  13
#define CWQ_EXP_PVEC_GF_PSCA_DOF  14
#define CWQ_EXP_PVEC_GF_PVEC_DOF  15
#define CWQ_FCT_TIME_GF_PSCA_DOF  16
#define CWQ_FCT_TIME_GF_PVEC_DOF  17
#define CWQ_FCT_PVEC_GF_PSCA_DOF  18
#define CWQ_FCT_PVEC_GF_PVEC_DOF  19

/* ------------------------------------------------------------------------ */
/*  W h o l e Q u a n t i t y                                               */
/* ------------------------------------------------------------------------ */

struct WholeQuantity {
  int  Type;

  union {
    double Constant;
    struct Function Function;
    struct { int  TypeOperator, Index, NbrArguments;
             int  TypeQuantity; }                                OperatorAndQuantity;
    struct { int  Index, NbrArguments; }                         Expression;
    struct { List_T *WholeQuantity; }                            TimeDerivative;
    struct { List_T *WholeQuantity; int TimeStep; }              AtAnteriorTimeStep;
    struct { double *Value; }                                    CurrentValue;
    struct { int  Index; }                                       Argument;
    struct { List_T *WholeQuantity_True, *WholeQuantity_False; } Test;
    struct { int  Index; }                                       SaveValue;
    struct { int  Index; }                                       ShowValue;
    struct { int  Index; }                                       ValueSaved;
    struct { int  TypeOperator; void  (*Function)(); }           Operator; /* binary or unary */
    struct { List_T *WholeQuantity;
             int FunctionSpaceIndexForType, NbrHar; }            Cast;
    struct { List_T *WholeQuantity ; }                           ChangeCurrentPosition ;
    struct { List_T *WholeQuantity ;
             int InIndex, DofIndexInWholeQuantity; }             Trace;
    struct { char *SystemName; int DefineSystemIndex;
             int DofNumber; }                                    DofValue;
    struct { List_T *WholeQuantity;
             int Index, NbrPoints; }                             MHTransform;
    struct { int Index, NbrPoints, FreqOffSet; }                 MHJacNL;
  } Case;

};

/* WholeQuantity.Type */
#define WQ_OPERATORANDQUANTITY     1
#define WQ_OPERATORANDQUANTITYEVAL 2
#define WQ_BINARYOPERATOR          3
#define WQ_UNARYOPERATOR           4
#define WQ_EXPRESSION              5
#define WQ_BUILTINFUNCTION         6
#define WQ_EXTERNBUILTINFUNCTION   7
#define WQ_CONSTANT                8
#define WQ_CURRENTVALUE            9
#define WQ_ARGUMENT                10
#define WQ_TIMEDERIVATIVE          11
#define WQ_CAST                    12
#define WQ_TEST                    13
#define WQ_SAVEVALUE               14
#define WQ_VALUESAVED              15
#define WQ_SOLIDANGLE              16
#define WQ_TRACE                   17
#define WQ_ORDER                   18
#define WQ_MHTIMEINTEGRATION       19
#define WQ_MHTRANSFORM             199
#define WQ_SHOWVALUE               20
#define WQ_MHTIMEEVAL              211
#define WQ_MHJACNL                 212
#define WQ_POSTSAVE                214
#define WQ_ATANTERIORTIMESTEP      22
#define WQ_CHANGECURRENTPOSITION   30

/* TypeOperator */
#define OP_PLUS           1
#define OP_MINUS          2
#define OP_TIME           3
#define OP_DIVIDE         4
#define OP_MODULO         5
#define OP_POWER          6
#define OP_CROSSPRODUCT   7
#define OP_LESS           8
#define OP_GREATER        9
#define OP_LESSOREQUAL    10
#define OP_GREATEROREQUAL 11
#define OP_EQUAL          12
#define OP_NOTEQUAL       13
#define OP_APPROXEQUAL    14
#define OP_AND            15
#define OP_OR             16
#define OP_NEG            17
#define OP_NOT            18

/* OperatorAndQuantity.TypeQuantity */

#define QUANTITY_SIMPLE  1
#define QUANTITY_DOF     2
#define QUANTITY_NODOF   3
#define QUANTITY_BF      4


/* ------------------------------------------------------------------------ */
/*  R e s o l u t i o n                                                     */
/* ------------------------------------------------------------------------ */

struct Resolution {
  char    *Name;
  List_T  *DefineSystem, *Operation;
};

struct DefineSystem {
  char    *Name;
  int     Type;
  List_T  *FormulationIndex, *FrequencyValue;
  char    *SolverDataFileName;

  char    *MeshName, *AdaptName;
  List_T  *OriginSystemIndex;
  char    *DestinationSystemName;
  int     DestinationSystemIndex;
};

/* DefineSystem.Type */
#define VAL_REAL     1
#define VAL_COMPLEX  2

struct Operation {
  int  Type, DefineSystemIndex, Rank;

  union {
    struct {
      List_T *MatrixIndex_L;
    } GenerateOnly;
    struct {
      char *String;
    } SystemCommand;
    struct {
      char *FileName;
      int ViewTag;
    } GmshRead;
    struct {
      char *FileName;
    } DeleteFile;
    struct {
      char *DirName;
    } CreateDir;
    struct {
      int     ExpressionIndex;
    } SetTime;
    struct {
      int     ExpressionIndex;
    } Update;
    struct {
      int     GroupIndex, Type;
    } UpdateConstraint;
    struct {
      int     ExpressionIndex;
    } SetFrequency;
    struct {
      List_T  *Frequency;
      int     DefineSystemIndex[2];
    } FourierTransform;
    struct {
      int     DefineSystemIndex[2];
      double Period, Period_sofar;
      double *Scales;
    } FourierTransform2;
    struct {
      int     Size;
      List_T  *Save;
      double  Shift;
    } Lanczos;
    struct {
      int     NumEigenvalues;
      double  Shift_r, Shift_i;
    } EigenSolve;
    struct {
      int     ExpressionIndex;
    } Evaluate;
    struct {
      int     Iteration ;
    } SelectCorrection ;
    struct {
      double  Alpha ;
    } AddCorrection ;
    struct {
      double  Alpha ;
    } MultiplySolution ;
    struct {
      int     Size;
      List_T  *Save;
      double  Shift;
      int     PertFreq;
      int  DefineSystemIndex2, DefineSystemIndex3;
    } Perturbation;
    struct {
      double  Time0, TimeMax;
      int     DTimeIndex, ThetaIndex;
      List_T  *Operation;
    } TimeLoopTheta;
    struct {
      double  Time0, TimeMax, Beta, Gamma;
      int     DTimeIndex;
      List_T  *Operation;
    } TimeLoopNewmark;
    struct {
      double  Time0, TimeMax;
      List_T *ButcherA, *ButcherB, *ButcherC;
      int     DTimeIndex;
    } TimeLoopRungeKutta;
    struct {
      double  Time0, TimeMax, DTimeInit, DTimeMin, DTimeMax;
      double  LTEtarget, DTimeMaxScal, DTimeScal_NotConverged;
      char    *Scheme;
      List_T  *Breakpoints_L;
      List_T  *TimeLoopAdaptiveSystems_L;
      List_T  *TimeLoopAdaptivePOs_L;
      List_T  *Operation, *OperationEnd;
    } TimeLoopAdaptive;
    struct {
      double  Criterion;
      int     NbrMaxIteration, RelaxationFactorIndex, Flag;
      List_T  *IterativeLoopSystems_L;
      List_T  *IterativeLoopPOs_L;
      List_T  *Operation;
    } IterativeLoop;
    struct {
      double  Criterion, DivisionCoefficient;
      int     NbrMaxIteration, Flag;
      List_T  *ChangeOfState;
      List_T  *Operation, *OperationEnd;
    } IterativeTimeReduction;
    struct {
      char   *OpMatMult;
      char   *Type;
      double Tolerance;
      int    MaxIter;
      int    Restart;
      List_T *MyFieldTag;
      List_T *NeighborFieldTag;
      List_T *DeflationIndices;
      List_T *Operations_Ax, *Operations_Mx;
    } IterativeLinearSolver;
    struct {
      int     ExpressionIndex;
      List_T  *Operation_True, *Operation_False;
    } Test;
    struct {
      List_T  *DofNumber, *TimeStep, *Expression;
      char    *FileOut;
    } Print;
    struct {
      int     GroupIndex, ExpressionIndex;
      int     NumNode, ExpressionIndex2;
    } ChangeOfCoordinates;
    struct {
      int    CheckAll;
      List_T *Factor_L;
    } SolveJac_AdaptRelax;
    struct{
      int    GroupIndex;
      bool   SaveFixed;
    } SaveSolutionWithEntityNum;
    struct {
      int NbrFreq;
      char    *ResFile;
    } SaveSolutionExtendedMH;
    struct {
      List_T  *Time;
      char    *ResFile;
    } SaveSolutionMHtoTime;
    struct {
      List_T *PostOperations;
    } PostOperation;
    struct {
      int     GroupIndex;
    } Init_MovingBand2D;
    struct {
      int     GroupIndex;
    } Mesh_MovingBand2D;
    struct {
      int     GroupIndex;
      double  Period;
      int     NbrStep;
      List_T  *Operation;
    } Generate_MH_Moving;
    struct {
      int     GroupIndex;
      double  Period;
      int     NbrStep;
      List_T  *Operation;
    } Generate_MH_Moving_S;
    struct {
      double  dummy;
    } Add_MH_Moving;
    struct {
      int     GroupIndex;
    } Generate;
    struct {
      int     GroupIndex;
      char    *FileName;
      int     ExprIndex;
    } SaveMesh;
    struct {
      char    *Quantity;
      char    *Name_MshFile;
      int     GeoDataIndex;
      double  Factor;
    } DeformeMesh;
    // FIXME: Roman
    struct {
      List_T *SystemIndex, *ExpectationIndex;
      List_T *LocalMatrixIndex;
      List_T *ExpansionCoef;
    } TensorProductSolve;

  } Case;

};

struct ChangeOfState {
  int     Type;
  int     QuantityIndex, InIndex, FormulationIndex;
  double  Criterion;
  double  *ActiveList[2];
  int     ExpressionIndex, ExpressionIndex2, FlagIndex;
};

struct TimeLoopAdaptiveSystem {
  int     SystemIndex;
  double  SystemLTEreltol;
  double  SystemLTEabstol;
  int     NormType;
  char    *NormTypeString;
};

struct LoopErrorPostOperation {
  char    *PostOperationName;
  int     PostOperationIndex;
  double  PostOperationReltol;
  double  PostOperationAbstol;
  int     NormType;
  char    *NormTypeString;
  List_T  *Save_Format_L, *Save_LastTimeStepOnly_L;
  List_T  *Save_FileOut_L;
};

struct IterativeLoopSystem {
  int     SystemIndex;
  double  SystemILreltol;
  double  SystemILabstol;
  int     NormType;
  char    *NormTypeString;
  int     NormOf;
  char    *NormOfString;
};

/* Operation.Type */
#define OPERATION_NONE                      0

#define OPERATION_GENERATE                  1
#define OPERATION_SOLVE                     2
#define OPERATION_SOLVEAGAIN              223
#define OPERATION_SOLVENL                 233
#define OPERATION_GENERATEJAC               3
#define OPERATION_GENERATERHS             100
#define OPERATION_GENERATEONLY            101
#define OPERATION_GENERATEONLYJAC         103
#define OPERATION_SOLVEJAC                  4
#define OPERATION_SOLVEJACAGAIN           224
#define OPERATION_SOLVEJACADAPTRELAX      888
#define OPERATION_GENERATESEPARATE          5
#define OPERATION_UPDATE                    6
#define OPERATION_UPDATECONSTRAINT          7
#define OPERATION_LANCZOS                   8
#define OPERATION_PERTURBATION              9
#define OPERATION_EIGENSOLVE               16
#define OPERATION_EIGENSOLVEJAC            17
#define OPERATION_EVALUATE                 18
#define OPERATION_SELECTCORRECTION         80
#define OPERATION_ADDCORRECTION            81
#define OPERATION_INITCORRECTION           82
#define OPERATION_MULTIPLYSOLUTION         83
#define OPERATION_ADDOPPOSITEFULLSOLUTION  84
#define OPERATION_TENSORPRODUCTSOLVE       85

#define OPERATION_SAVESOLUTION             10
#define OPERATION_SAVESOLUTIONS            11
#define OPERATION_SAVESOLUTION_WITH_ENTITY_NUM 121
#define OPERATION_SAVESOLUTIONEXTENDEDMH  111
#define OPERATION_SAVESOLUTIONMHTOTIME    131
#define OPERATION_INIT_MOVINGBAND2D       444
#define OPERATION_MESH_MOVINGBAND2D       222
#define OPERATION_GENERATE_MH_MOVING      999
#define OPERATION_GENERATE_MH_MOVING_S   9991
#define OPERATION_ADD_MH_MOVING          9992
#define OPERATION_DUMMYDOFS              9993

#define OPERATION_SAVEMESH                333
#define OPERATION_DEFORMEMESH             334
#define OPERATION_READSOLUTION             12
#define OPERATION_TRANSFERSOLUTION         13
#define OPERATION_INITSOLUTION             15
#define OPERATION_SETCURRENTSYSTEM         70

#define OPERATION_SETTIME                  20
#define OPERATION_SETFREQUENCY             21
#define OPERATION_TEST                     22
#define OPERATION_FOURIERTRANSFORM         23
#define OPERATION_FOURIERTRANSFORM2       777
#define OPERATION_BREAK                    24
#define OPERATION_PRINT                    25
#define OPERATION_WRITE                    26
#define OPERATION_SCAN                     27
#define OPERATION_READ                     28

#define OPERATION_TIMELOOPTHETA            30
#define OPERATION_TIMELOOPNEWMARK          31
#define OPERATION_ITERATIVELOOP            32
#define OPERATION_ITERATIVETIMEREDUCTION   33
#define OPERATION_TIMELOOPRUNGEKUTTA       34
#define OPERATION_ITERATIVELINEARSOLVER    35
#define OPERATION_TIMELOOPADAPTIVE         36
#define OPERATION_ITERATIVELOOPN           37

#define OPERATION_CHANGEOFCOORDINATES      40
#define OPERATION_CHANGEOFCOORDINATES2    400

#define OPERATION_SYSTEMCOMMAND            50

#define OPERATION_POSTOPERATION            60
#define OPERATION_GMSHREAD                 61
#define OPERATION_GMSHCLEARALL             62
#define OPERATION_DELETEFILE               71
#define OPERATION_CREATEDIR                72

#define OPERATION_SETCOMMSELF              63
#define OPERATION_SETCOMMWORLD             64
#define OPERATION_BARRIER                  65

/* ChangeOfState.Type */
#define CHANGEOFSTATE_NOCHANGE              0
#define CHANGEOFSTATE_CHANGESIGN            1
#define CHANGEOFSTATE_CHANGELEVEL           2
#define CHANGEOFSTATE_CHANGEREFERENCE       3
#define CHANGEOFSTATE_CHANGEREFERENCE2      4

/* TimeLoopAdaptiveSystem.NormType */
#define LINFNORM              1
#define L1NORM                2
#define MEANL1NORM            3
#define L2NORM                4
#define MEANL2NORM            5

/* IterativeLoopSystem.NormOf */
#define SOLUTION              1
#define RESIDUAL              2
#define RECALCRESIDUAL        3


/* ------------------------------------------------------------------------ */
/*  P r e R e s o l u t i o n I n f o                                       */
/* ------------------------------------------------------------------------ */

struct PreResolutionInfo {
  int  Index, Type ;
} ;

/* Type PreResolution */

#define PR_CONSTRAINT           1
#define PR_GLOBALBASISFUNCTION  2


/* ------------------------------------------------------------------------ */
/*  P o s t P r o c e s s i n g                                             */
/* ------------------------------------------------------------------------ */

struct PostProcessing {
  char    *Name;
  int      FormulationIndex;
  List_T  *OriginSystemIndex;
  char    *NameOfSystem;
  List_T  *PostQuantity;
  int	   Rank;
};

struct PostQuantity {
  char    *Name;
  List_T  *PostQuantityTerm;
};

struct PostQuantityTerm {
  int     Type, EvaluationType;

  int     TypeTimeDerivative;
  List_T  *WholeQuantity;
  int     NbrQuantityIndex, *QuantityIndexTable;
  int     *QuantityTraceGroupIndexTable;
  int     InIndex, JacobianMethodIndex, IntegrationMethodIndex;
};

/* PostQuantityTerm.Type */
/* LOCALQUANTITY
   GLOBALQUANTITY
   INTEGRALQUANTITY      */

/* PostQuantityTerm.EvaluationType */
#define LOCAL           1
#define INTEGRAL        2

/* ------------------------------------------------------------------------ */
/*  P o s t O p e r a t i o n                                               */
/* ------------------------------------------------------------------------ */

struct PostOperation {
  char    *Name, *AppendString;
  int      PostProcessingIndex, Format;
  List_T  *PostSubOperation;
  int      Rank;
  double   ResampleTimeStart, ResampleTimeStop, ResampleTimeStep;
  bool     ResampleTime;
};

struct PostSubOperation {
  int    PostQuantityIndex[2], PostQuantitySupport[2];
  int    Type, SubType, CombinationType;
  int    Depth, Skin, Smoothing, Dimension, Comma, HarmonicToTime, CatFile;
  int    Format, Adapt, Sort, Iso, NoNewLine, NoTitle, DecomposeInSimplex;
  int    NewCoordinates;
  char  *NewCoordinatesFile;
  int    ValueIndex;
  int    ChangeOfCoordinates[3], LastTimeStepOnly, AppendTimeStepToFileName;
  int    OverrideTimeStepValue, NoMesh;
  int    StoreInRegister;
  char  *SendToServer, *Color;
  int    StoreInField;
  int    Legend, FrozenTimeStepList;
  double LegendPosition[3];
  double Target;
  char   *ValueName, *Label;
  char   *FileOut;
  List_T *TimeStep_L, *Value_L, *Iso_L, *Frequency_L;
  List_T *ChangeOfValues;
  List_T *EvaluationPoints;
  union {
    struct { int RegionIndex; } OnRegion;
    struct { double x[4], y[4], z[4]; int n[3]; } OnGrid;
    struct { int ExpressionIndex[3]; List_T *ParameterValue[3]; } OnParamGrid;
    struct { double x[3], y[3], z[3]; } OnSection;
    struct { int RegionIndex, ArgumentIndex; double x[2]; int n; } WithArgument;
    struct { int ExtendedGroupIndex, GroupIndex; } Group;
    struct { char *String; char *String2; int ExpressionIndex; } Expression;
  } Case;
};

struct PostOpSolutions {
  PostOperation *PostOperation_P;
  List_T        *Solutions_L;
};

/* PostOperation.Type */
#define POP_NONE          0
#define POP_PRINT         1
#define POP_GROUP         2
#define POP_EXPRESSION    4

/* PostOperation.SubType */
#define PRINT_ONREGION        1
#define PRINT_ONELEMENTSOF    2
#define PRINT_ONSECTION_1D    3
#define PRINT_ONSECTION_2D    4
#define PRINT_ONGRID          5
#define PRINT_ONGRID_0D       6
#define PRINT_ONGRID_1D       7
#define PRINT_ONGRID_2D       8
#define PRINT_ONGRID_3D       9
#define PRINT_ONGRID_PARAM    10
#define PRINT_WITHARGUMENT    11

/* PostOperation.CombinationType */
#define ADDITION        1
#define SOUSTRACTION    2
#define MULTIPLICATION  3
#define DIVISION        4

/* Unsuccessful search results */
#define NO_BRICK      -999
#define NO_ELEMENT    -999
#define NO_REGION     -999

/* PostSubOperation Tags */
#define TAG_TIME        1
#define TAG_TIMESTEP    2
#define TAG_VALUE       3
#define TAG_X           4
#define TAG_Y           5
#define TAG_Z           6
#define TAG_NODES       7
#define TAG_TYPE        8
#define TAG_VERSION     9
#define TAG_DATE        10
#define TAG_HOST        11
#define TAG_FILENAME    12
#define TAG_USER        13
#define TAG_ABSCISSA    14
#define TAG_NORMAL      15
#define TAG_COMMAND     16

/* PostSubOperation.Format */
#define FORMAT_SPACE_TABLE            1
#define FORMAT_TIME_TABLE             2
#define FORMAT_SIMPLE_SPACE_TABLE     3
#define FORMAT_FREQUENCY_TABLE        4
#define FORMAT_VALUE_ONLY             5
#define FORMAT_ADAPT                  9
#define FORMAT_GMSH                  10
#define FORMAT_GMSH_PARSED           11
#define FORMAT_MATLAB                14
#define FORMAT_GNUPLOT               15
#define FORMAT_REGION_TABLE          16
#define FORMAT_REGION_VALUE          17
#define FORMAT_UNV                   18
#define FORMAT_NODE_TABLE            19
#define FORMAT_LOOP_ERROR            20

/* PostSubOperation.Sort */
#define SORT_BY_POSITION      1
#define SORT_BY_CONNECTIVITY  2

/* PostSubOperation.Legend */
#define LEGEND_NONE        0
#define LEGEND_TIME        1
#define LEGEND_FREQUENCY   2
#define LEGEND_EIGENVALUES 3

/* ------------------------------------------------------------------------ */
/*  C u r r e n t D a t a                                                   */
/* ------------------------------------------------------------------------ */

struct CurrentData {
  char   *Name;

  int     NbrSystem, NbrCpu, RankCpu;
  struct DefineSystem   *DefineSystem_P ;
  struct DofData  *DofData_P0;

  struct DofData  *DofData;
  struct GeoData  *GeoData;

  //PostOperation based solutions for TimeLoopAdaptive, IterativeLoopN
  List_T  *PostOpData_L;
  int     PostOpDataIndex;

  int     NbrHar;
  int     Region, SubRegion;
  int     NumEntity, NumEntityInElement;
  int     NumEntities[NBR_MAX_BASISFUNCTIONS];

  struct  Element  *Element;
  int     IntegrationSupportIndex;

  struct  Element  *ElementSource;

  int     SourceIntegrationSupportIndex;

  int     TypeTime, TypeAssembly;
  int     SubTimeStep;

  int     flagAssDiag;

  // All values below must be double

  double  x, y, z;
  double  u, v, w;

  double  xs, ys, zs;
  double  us, vs, ws;

  double  a, b, c;
  double  xp, yp, zp;
  double  ut, vt, wt;

  double  Val[NBR_MAX_HARMONIC * MAX_DIM];

  // For TimeLoopTheta and TimeLoopNewmark
  double  Time, TimeImag, TimeStep, DTime;
  double  Theta, Beta, Gamma;

  // For TimeLoopAdaptive
  double  PredOrder, CorrOrder;
  double  aPredCoeff[7], aCorrCoeff[6], bCorrCoeff, PredErrorConst, CorrErrorConst;
  double  Breakpoint;

  // For IterativeLoop
  double  Iteration, RelativeDifference, RelativeDifferenceOld;
  double  RelaxationFactor;
};

/* ------------------------------------------------------------------------ */
/*  E l e m e n t                                                           */
/* ------------------------------------------------------------------------ */

#define NBR_MAX_NODES_IN_ELEMENT       60
#define NBR_MAX_ENTITIES_IN_ELEMENT    60
#define NBR_MAX_GROUPS_IN_ELEMENT      60
#define NBR_MAX_SUBENTITIES_IN_ELEMENT  5

struct IntxList { int Int ; List_T * List ; } ;

struct Matrix3x3 {
  double  c11, c12, c13 ;
  double  c21, c22, c23 ;
  double  c31, c32, c33 ;
} ;
typedef struct Matrix3x3  MATRIX3x3 ;

struct Element {
  struct Geo_Element  * GeoElement ;

  int       Num, Type, Region ;

  struct Element  * ElementSource, * ElementTrace ;

  int       NumLastElementForNodesCoordinates ;
  double    x [NBR_MAX_NODES_IN_ELEMENT] ;
  double    y [NBR_MAX_NODES_IN_ELEMENT] ;
  double    z [NBR_MAX_NODES_IN_ELEMENT] ;

  int       NumLastElementForSolidAngle ;
  double    angle [NBR_MAX_NODES_IN_ELEMENT] ;

  int       NumLastElementForSortedNodesByFacet ;
  struct TwoInt  SortedNodesByFacet [6][NBR_MAX_SUBENTITIES_IN_ELEMENT] ;

  double    n    [NBR_MAX_NODES_IN_ELEMENT] ;
  double    dndu [NBR_MAX_NODES_IN_ELEMENT] [3] ;

  struct JacobianCase  * JacobianCase ;
  MATRIX3x3            Jac ;
  double               DetJac ;
  MATRIX3x3            InvJac ;

  int       NumLastElementForGroupsOfEntities ;
  int       NbrGroupsOfEntities ;
  int       NumGroupsOfEntities  [NBR_MAX_GROUPS_IN_ELEMENT] ;
  int       NbrEntitiesInGroups  [NBR_MAX_GROUPS_IN_ELEMENT] ;
  int       NumEntitiesInGroups  [NBR_MAX_GROUPS_IN_ELEMENT]
                                 [NBR_MAX_ENTITIES_IN_ELEMENT] ;
  int       NumSubFunction       [3][NBR_MAX_GROUPS_IN_ELEMENT] ;

  struct GlobalBasisFunction  * GlobalBasisFunction [NBR_MAX_GROUPS_IN_ELEMENT] ;
} ;

/* Element.Type */
#define POINT            (1<<0)
#define LINE             (1<<1)
#define TRIANGLE         (1<<2)
#define QUADRANGLE       (1<<3)
#define TETRAHEDRON      (1<<4)
#define HEXAHEDRON       (1<<5)
#define PRISM            (1<<6)
#define PYRAMID          (1<<7)
#define LINE_2           (1<<8)
#define TRIANGLE_2       (1<<9)
#define QUADRANGLE_2     (1<<10)
#define TETRAHEDRON_2    (1<<11)
#define HEXAHEDRON_2     (1<<12)
#define PRISM_2          (1<<13)
#define PYRAMID_2        (1<<14)
#define QUADRANGLE_2_8N  (1<<16)

/* Adapt.Type */
#define P1 1
#define P2 2
#define H1 3
#define H2 4

struct Geo_Node {
  int     Num ;
  double  x, y, z ;
} ;

struct Geo_Element {
  int  Num ;
  int  Type, Region, ElementaryRegion ;
  int  NbrNodes , * NumNodes ;
  int  NbrEdges , * NumEdges ;
  int  NbrFacets, * NumFacets ;
} ;

struct Entity2XEntity1 {
  int  Num ;
  int  NbrEntities, * NumEntities ;
} ;

struct EntityXVector {
  int     Num;
  double  Vector[3];
} ;

struct EntityInTree {
  int  Num, Index ;
} ;

/* ------------------------------------------------------------------------ */
/*  I n t e r f a c e   F u n c t i o n s                                   */
/* ------------------------------------------------------------------------ */

int fcmp_Integer                  (const void *a, const void *b);
int fcmp_Constant                 (const void *a, const void *b);
int fcmp_Expression_Name          (const void *a, const void *b);
int fcmp_Group_Name               (const void *a, const void *b);
int fcmp_Constraint_Name          (const void *a, const void *b);
int fcmp_JacobianMethod_Name      (const void *a, const void *b);
int fcmp_IntegrationMethod_Name   (const void *a, const void *b);
int fcmp_BasisFunction_Name       (const void *a, const void *b);
int fcmp_FunctionSpace_Name       (const void *a, const void *b);
int fcmp_BasisFunction_NameOfCoef (const void *a, const void *b);
int fcmp_SubSpace_Name            (const void *a, const void *b);
int fcmp_GlobalQuantity_Name      (const void *a, const void *b);
int fcmp_Formulation_Name         (const void *a, const void *b);
int fcmp_DefineQuantity_Name      (const void *a, const void *b);
int fcmp_DefineSystem_Name        (const void *a, const void *b);
int fcmp_Resolution_Name          (const void *a, const void *b);
int fcmp_PostProcessing_Name      (const void *a, const void *b);
int fcmp_PostQuantity_Name        (const void *a, const void *b);
int fcmp_PostOperation_Name       (const void *a, const void *b);

void Init_ProblemStructure();
void Read_ProblemStructure(const char *fileName);
void Finalize_ProblemStructure();
void Print_ProblemStructure();
void Free_ProblemStructure();

std::string Get_AbsolutePath(const char *fileName);

void Print_ListResolution(int choice, int flag_lres, char **name);
void Print_ListPostOperation(int choice, int flag_lpos, char *name[NBR_MAX_POS]);

#endif
