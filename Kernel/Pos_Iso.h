// GetDP - Copyright (C) 1997-2019 P. Dular and C. Geuzaine, University of Liege
//
// See the LICENSE.txt file for license information. Please report all
// issues on https://gitlab.onelab.info/getdp/getdp/issues.

#ifndef _POS_ISO_H_
#define _POS_ISO_H_

#include "Pos_Element.h"
#include "ListUtils.h"

void Cal_Iso(struct PostElement *PE, List_T *list, 
	     double val, double vmin, double vmax, 
	     int DecomposeInSimplex) ;

#endif
