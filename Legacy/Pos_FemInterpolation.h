// GetDP - Copyright (C) 1997-2013 P. Dular, C. Geuzaine
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to the public mailing list <getdp@geuz.org>.

#ifndef _POS_FEM_INTERPOLATION_H_
#define _POS_FEM_INTERPOLATION_H_

#include "ProData.h"

void Pos_FemInterpolation(struct Element * Element, 
			  struct QuantityStorage * QuantityStorage_P0,
			  struct QuantityStorage * QuantityStorage_P,
			  int Type_Quantity, int Type_Operator, 
			  int Type_Dimension, int UseXYZ, 
			  double u, double v, double w, 
			  double x, double y, double z, 
			  double Val[], int * Type_Value,
			  int Flag_ChangeOfCoordinates);

#endif
