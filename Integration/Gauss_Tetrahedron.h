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

/* 1 integration point */

double xtet1[1] = {0.25};
double ytet1[1] = {0.25};
double ztet1[1] = {0.25};
double ptet1[1] = {0.166666666667};

/* 4 integration points */

double xtet4[4] = {0.138196601125,0.138196601125,0.138196601125,0.585410196625};
double ytet4[4] = {0.138196601125,0.138196601125,0.585410196625,0.138196601125};
double ztet4[4] = {0.138196601125,0.585410196625,0.138196601125,0.138196601125};
double ptet4[4] = {0.0416666666667,0.0416666666667,0.0416666666667,0.0416666666667};

/* 5 integration points */

double xtet5[5] = {0.25,0.166666666667,0.166666666667,0.166666666667,0.5};
double ytet5[5] = {0.25,0.166666666667,0.166666666667,0.5,0.166666666667};
double ztet5[5] = {0.25,0.166666666667,0.5,0.166666666667,0.166666666667};
double ptet5[5] = {-0.133333333333,0.075,0.075,0.075,0.075};

/* 15 integration points */

double xtet15[15] = {0.25,
		     0.0919710780526,0.0919710780526,0.0919710780526,0.724086765841,
		     0.319793627829,0.319793627829,0.319793627829,0.0406191165118,
		     0.0563508326895,0.0563508326895,0.44364916731,0.0563508326895,
		     0.44364916731,0.44364916731};
double ytet15[15] = {0.25,
		     0.0919710780526,0.0919710780526,0.724086765841,0.0919710780526,
		     0.319793627829,0.319793627829,0.0406191165118,0.319793627829,
		     0.0563508326895,0.44364916731,0.0563508326895,0.44364916731,
		     0.0563508326895,0.44364916731};
double ztet15[15] = {0.25,
		     0.0919710780526,0.724086765841,0.0919710780526,0.0919710780526,
		     0.319793627829,0.0406191165118,0.319793627829,0.319793627829,
		     0.44364916731,0.0563508326895,0.0563508326895,0.44364916731,
		     0.44364916731,0.0563508326895};
double ptet15[15] = {0.0197530864198,
		     0.0119895139632,0.0119895139632,0.0119895139632,0.0119895139632,  
		     0.011511367871,0.011511367871,0.011511367871,0.011511367871,
		     0.00881834215168,0.00881834215168,0.00881834215168,0.00881834215168,
		     0.00881834215168,0.00881834215168};

/* 16 integration points */

#define a16  0.0503737941001228 / 6.0  
#define b16  0.0665420686332923 / 6.0
#define c16  0.7716429020672371  
#define d16  0.0761190326442543  
#define e16  0.1197005277978019  
#define f16  0.0718316452676693  
#define g16  0.4042339134672644  

double xtet16[16] = {c16,d16,d16,d16,e16,f16,e16,f16,
		     g16,g16,g16,g16,e16,f16,g16,g16};
double ytet16[16] = {d16,c16,d16,d16,f16,e16,g16,g16,
		     g16,g16,e16,f16,g16,g16,e16,f16};
double ztet16[16] = {d16,d16,c16,d16,g16,g16,g16,g16,
		     e16,f16,f16,e16,f16,e16,g16,g16};
double ptet16[16] = {a16,a16,a16,a16,b16,b16,b16,b16,
		     b16,b16,b16,b16,b16,b16,b16,b16};
#undef a16  
#undef b16  
#undef c16  
#undef d16  
#undef e16  
#undef f16  
#undef g16  

/* 17 integration points */

#define a17  0.1884185567365411 / 6.0
#define b17  0.0670385837260428 / 6.0
#define c17  0.0452855923632739 / 6.0
#define p17  0.7316369079576180  
#define q17  0.0894543640141273  
#define e17  0.1325810999384657  
#define f17  0.0245400397290300  
#define g17  0.4214394310662522  

double xtet17[17] = {0.25,p17,q17,q17,q17,e17,f17,e17,f17,g17,
		     g17,g17,g17,e17,f17,g17,g17};
double ytet17[17] = {0.25,q17,p17,q17,q17,f17,e17,g17,g17,g17,
		     g17,e17,f17,g17,g17,e17,f17};
double ztet17[17] = {0.25,q17,q17,p17,q17,g17,g17,g17,g17,e17,
		     f17,f17,e17,f17,e17,g17,g17};
double ptet17[17] = {a17,b17,b17,b17,b17,c17,c17,c17,c17,c17,
		     c17,c17,c17,c17,c17,c17,c17};
#undef a17
#undef b17
#undef c17
#undef p17
#undef q17
#undef e17
#undef f17
#undef g17

/* 29 integration points */

#define a29  0.0904012904601475 / 6.0  
#define b29  0.0191198342789912 / 6.0  
#define c29  0.0436149384066657 / 6.0  
#define d29  0.0258116759619916 / 6.0  
#define p29  0.8277192480479295  
#define q29  0.0574269173173568  
#define e29  0.0513518841255634  
#define f29  0.4860510285706072  
#define g29  0.2312985436519147  
#define h29  0.2967538129690260  
#define i29  0.6081079894015281  
#define j29  0.0475690988147229  

double xtet29[29] = {0.25,p29,q29,q29,q29,e29,f29,e29,f29,g29,
		     g29,g29,g29,e29,f29,g29,g29,h29,i29,h29,
		     i29,j29,j29,j29,j29,h29,i29,j29,j29};
double ytet29[29] = {0.25,q29,p29,q29,q29,f29,e29,g29,g29,g29,
		     g29,e29,f29,g29,g29,e29,f29,i29,h29,j29,
		     j29,j29,j29,h29,i29,j29,j29,h29,i29};
double ztet29[29] = {0.25,q29,q29,p29,q29,g29,g29,g29,g29,e29,
		     f29,f29,e29,f29,e29,g29,g29,j29,j29,j29,
		     j29,h29,i29,i29,h29,i29,h29,j29,j29};
double ptet29[29] = {a29,b29,b29,b29,b29,c29,c29,c29,c29,c29,
		     c29,c29,c29,c29,c29,c29,c29,d29,d29,d29,
		     d29,d29,d29,d29,d29,d29,d29,d29,d29};
#undef a29
#undef b29
#undef c29
#undef d29
#undef p29
#undef q29
#undef e29
#undef f29
#undef g29
#undef h29
#undef i29
#undef j29
