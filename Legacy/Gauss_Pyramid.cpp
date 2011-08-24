// GetDP - Copyright (C) 1997-2011 P. Dular, C. Geuzaine
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <getdp@geuz.org>.

#include "Gauss_Pyramid.h"
#include "Message.h"

/* Gauss integration over a pyramid */

void Gauss_Pyramid(int Nbr_Points, int Num,
		   double *u, double *v, double *w, double *wght)
{
  switch (Nbr_Points) {

  case  8 : 
    *u = upyr8[Num]; *v = vpyr8[Num]; *w = wpyr8[Num]; *wght = ppyr8[Num]; 
    break ;

  default : 
    Message::Error("Wrong number of Gauss points for Pyramid: "
                   "valid choice: 8");
    break;
  }
}
