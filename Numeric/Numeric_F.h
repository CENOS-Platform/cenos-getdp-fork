/*
 * Copyright (C) 1997-2003 P. Dular, C. Geuzaine
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 * Please report all bugs and problems to "getdp@geuz.org".
 */

#ifndef _NUMERIC_F_H_
#define _NUMERIC_F_H_

#include "Compat.h"

#ifdef _UNDERSCORE
#define solcyl_     solcyl
#define solsph_     solsph
#define cglt2d_     cglt2d
#define clt2d_      clt2d
#define clt2dl_     clt2dl
#endif

EXTERN_C_BEGIN

extern void solsph_(double*,double*,double*,double*,double*,double*,double*,double*,
		    double*,double*,double*,double*,double*);  
extern void solcyl_(double*,double*,double*,double*,double*,double*,double*,double*,
		    double*,double*,double*);
extern void cglt2d_(double*,double*,double*,double*,double*,double*,
		    double*,double*,double*,double*);
extern void clt2d_ (double*,double*,double*,double*,double*,double*,
		    double*,double*,double*,double*);
extern void clt2dl_(double*,double*,double*,double*,double*,double*,
		    double*,double*,double*,double*);

EXTERN_C_END

#endif
