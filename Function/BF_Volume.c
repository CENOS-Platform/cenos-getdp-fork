#define RCSID "$Id: BF_Volume.c,v 1.6 2002-03-06 09:24:48 trophime Exp $"
#include <stdio.h>

#include "GetDP.h"
#include "BF_Function.h"


/* ------------------------------------------------------------------------ */
/*  B F _ V o l u m e                                                       */
/* ------------------------------------------------------------------------ */

#define WrongNumVolume   Msg(ERROR, "Wrong Volume number in 'BF_Volume'")

void  BF_Volume  (struct Element * Element, int NumVolume,
		  double u, double v, double w,  double *s ) {

  GetDP_Begin("BF_Volume");

  switch (Element->Type) {
  case POINT :
    switch(NumVolume) {
    case 1  : *s = 1. ; break ;
    default : WrongNumVolume ;
    }
    break ;

  case LINE :
    switch(NumVolume) {
    case 1  : *s = 0.5 ; break ;
    default : WrongNumVolume ;
    }
    break ;

  case TRIANGLE :
    switch(NumVolume) {
    case 1  : *s = 2. ; break ;
    default : WrongNumVolume ;
    }
    break ;

  case QUADRANGLE :
    switch(NumVolume) {
    case 1  : *s = 0.25 ; break ;
    default : WrongNumVolume ;
    }
    break ;

  case TETRAHEDRON :
    switch(NumVolume) {
    case 1  : *s = 6. ; break ;
    default : WrongNumVolume ;
    }
    break ;

  case HEXAHEDRON :
    switch(NumVolume) {
    case 1  : *s = 0.125 ; break ;
    default : WrongNumVolume ;
    }
    break ;

  case PRISM :
    switch(NumVolume) {
    case 1  : *s = 1. ; break ;
    default : WrongNumVolume ;
    }
    break ;

  case PYRAMID :
    switch(NumVolume) {
    case 1  : *s = 3. ; break ;
    default : WrongNumVolume ;
    }
    break ;

  default :
    Msg(ERROR, "Unkown type of Element in BF_Volume");
    break ;
  }

  GetDP_End ;
}

#undef WrongNumVolume


