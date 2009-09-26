// GetDP - Copyright (C) 1997-2009 P. Dular, C. Geuzaine
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <getdp@geuz.org>.

#ifndef _GET_ELEMENT_SOURCE_H_
#define _GET_ELEMENT_SOURCE_H_

#include "ProData.h"

void Get_InitElementSource(struct Element *Element, int InIndex);
int Get_NextElementSource(struct Element *ElementSource);
void Get_ElementTrace(struct Element *Element, int InIndex);

#endif
