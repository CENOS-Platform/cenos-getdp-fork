// GetDP - Copyright (C) 1997-2010 P. Dular, C. Geuzaine
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <getdp@geuz.org>.

#ifndef _CAL_GLOBAL_TERM_OF_FEM_EQUATION_H_
#define _CAL_GLOBAL_TERM_OF_FEM_EQUATION_H_

#include "ProData.h"

void  Cal_GlobalTermOfFemEquation(int  Num_Region,
				  struct EquationTerm     * EquationTerm_P,
				  struct QuantityStorage  * QuantityStorage_P0,
				  struct QuantityStorage  * QuantityStorageNoDof,
				  struct Dof              * DofForNoDof_P);
#endif
