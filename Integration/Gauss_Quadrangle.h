/* $Id: Gauss_Quadrangle.h,v 1.4 2000-09-07 18:47:25 geuzaine Exp $ */
/* 
   GAUSS QUADRANGLE
*/

/* 3 integration points */

double xq3[3] = {0.816496580928,-0.408248290464,-0.408248290464};
double yq3[3] = {0.0,0.840896415255,-0.840896415255};
double pq3[3] = {1.33333333333,1.33333333333,1.33333333333,};

/* 4 integration points */

double xq4[4] = {0.577350269189,-0.577350269189,0.577350269189,-0.577350269189};
double yq4[4] = {0.577350269189,0.577350269189,-0.577350269189,-0.577350269189};
double pq4[4] = {1.,1.,1.,1.};

/* 7 integration points */

double xq7[7] = {0.0,-0.683130051064,0.683130051064,0.890654421782,
		 -0.890654421782,0.374256642286,-0.374256642286};
double yq7[7] = {0.0,-0.683130051064,0.683130051064,-0.374256642286,0.374256642286,
		   -0.890654421782,0.890654421782,};
double pq7[7] = {1.142857142857,0.595238095238,0.595238095238,
		 0.416666666666,0.416666666666,0.416666666666,0.416666666666};

/* 
   GAUSS QUADRANGLE WITH 1/R SINGULARITY OVER NODE (-1,-1,0)
   
   ref.: H. L. G. Pina, J. L. M. Fernandes, C. A. Brebbia, 
         Some numerical integration formulae over triangles and 
	 squares with a 1/R singularity, 
	 Appl. Math. Modelling, Vol 5, June 1981, pp 209--211 
*/ 

/* 1 integration point */

double xqs1[1] = {-0.26501817};
double yqs1[1] = {-0.26501817};
double pqs1[1] = {3.52549435};

/* 3 integration points */

double xqs3[3] = {-0.58105530,1.0,-0.21877566};
double yqs3[3] = {-0.58105530,-0.21877566,1.0};
double pqs3[3] = {2.37881900,0.57333767,0.57333767};

/* 4 integration points */

double xqs4[4] = {-0.37512304,0.69629093,-0.92928746,-0.15602536};
double yqs4[4] = {-0.92928746,-0.15602536,-0.37512304,0.69629093};
double pqs4[4] = {1.02276580,0.73998134,1.02276580,0.73998134};
