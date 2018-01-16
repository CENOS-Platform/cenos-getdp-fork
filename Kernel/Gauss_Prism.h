// GetDP - Copyright (C) 1997-2018 P. Dular and C. Geuzaine, University of Liege
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to the public mailing list <getdp@onelab.info>.

/* 6 integration points (DHATT ET TOUZOT) */

static double upri6[6] = {0.166666666666666, 0.666666666666666, 0.166666666666666,
			  0.166666666666666, 0.666666666666666, 0.166666666666666};
static double vpri6[6] = {0.166666666666666, 0.166666666666666, 0.666666666666666,
			  0.166666666666666, 0.166666666666666, 0.666666666666666};
static double wpri6[6] = {-0.577350269189, -0.577350269189, -0.577350269189,
			   0.577350269189,  0.577350269189,  0.577350269189};
static double ppri6[6] = {0.166666666666666,0.166666666666666,0.166666666666666,
			  0.166666666666666,0.166666666666666,0.166666666666666};

/* 9 integration points (DHATT ET TOUZOT) */

static double upri9[9] = {0.166666666666666, 0.666666666666666, 0.166666666666666,
			  0.166666666666666, 0.666666666666666, 0.166666666666666,
			  0.166666666666666, 0.666666666666666, 0.166666666666666};
static double vpri9[9] = {0.166666666666666, 0.166666666666666, 0.666666666666666,
			  0.166666666666666, 0.166666666666666, 0.666666666666666,
			  0.166666666666666, 0.166666666666666, 0.666666666666666};
static double wpri9[9] = {-0.774596669241483, -0.774596669241483, -0.774596669241483,
			  0.               ,  0.               ,  0.               ,
			  0.774596669241483,  0.774596669241483,  0.774596669241483};
static double ppri9[9] = {0.0925925925929,0.0925925925929,0.0925925925929,
			  0.1481481481481,0.1481481481481,0.1481481481481,
			  0.0925925925929,0.0925925925929,0.0925925925929};

/* 21 integration points (DHATT ET TOUZOT) */

static double upri21[21] = {0.333333333333333, 0.470142064105115,
			    0.05971587179    , 0.470142064105115,
			    0.101286507323456, 0.79742698535    ,
			    0.101286507323456,
			    0.333333333333333, 0.470142064105115,
			    0.05971587179    , 0.470142064105115,
			    0.101286507323456, 0.79742698535    ,
			    0.101286507323456,
			    0.333333333333333, 0.470142064105115,
			    0.05971587179    , 0.470142064105115,
			    0.101286507323456, 0.79742698535    ,
			    0.101286507323456};
static double vpri21[21] = {0.333333333333333, 0.470142064105115,
			    0.470142064105115, 0.05971587179    ,
			    0.101286507323456, 0.101286507323456,
			    0.79742698535    ,
			    0.333333333333333, 0.470142064105115,
			    0.470142064105115, 0.05971587179    ,
			    0.101286507323456, 0.101286507323456,
			    0.79742698535    ,
			    0.333333333333333, 0.470142064105115,
			    0.470142064105115, 0.05971587179    ,
			    0.101286507323456, 0.101286507323456,
			    0.79742698535};
static double wpri21[21] = {-0.774596669241483,-0.774596669241483,
			    -0.774596669241483,-0.774596669241483,
			    -0.774596669241483,-0.774596669241483,
			    -0.774596669241483,
			    0.               , 0.               ,
			    0.               , 0.               ,
			    0.               , 0.               ,
			    0.               ,
			    0.774596669241483, 0.774596669241483,
			    0.774596669241483, 0.774596669241483,
			    0.774596669241483, 0.774596669241483,
			    0.774596669241483};
static double ppri21[21] = {0.625000000000000E-01, 0.367761535523628E-01,
			    0.367761535523628E-01, 0.367761535523628E-01,
			    0.349831057068964E-01, 0.349831057068964E-01,
			    0.349831057068964E-01,
			    0.100000000000000E+00, 0.588418456837804E-01,
			    0.588418456837804E-01, 0.588418456837804E-01,
			    0.559729691310342E-01, 0.559729691310342E-01,
			    0.559729691310342E-01,
			    0.625000000000000E-01, 0.367761535523628E-01,
			    0.367761535523628E-01, 0.367761535523628E-01,
			    0.349831057068964E-01, 0.349831057068964E-01,
			    0.349831057068964E-01};

/* 42 integration points (DHATT ET TOUZOT) */

static double upri42[42] = {0.333333333333333, 0.470142064105115,
			    0.05971587179    , 0.470142064105115,
			    0.101286507323456, 0.79742698535    ,
			    0.101286507323456,
			    0.333333333333333, 0.470142064105115,
			    0.05971587179    , 0.470142064105115,
			    0.101286507323456, 0.79742698535    ,
			    0.101286507323456,
			    0.333333333333333, 0.470142064105115,
			    0.05971587179    , 0.470142064105115,
			    0.101286507323456, 0.79742698535    ,
			    0.101286507323456,
			    0.333333333333333, 0.470142064105115,
			    0.05971587179    , 0.470142064105115,
			    0.101286507323456, 0.79742698535    ,
			    0.101286507323456,
			    0.333333333333333, 0.470142064105115,
			    0.05971587179    , 0.470142064105115,
			    0.101286507323456, 0.79742698535    ,
			    0.101286507323456,
			    0.333333333333333, 0.470142064105115,
			    0.05971587179    , 0.470142064105115,
			    0.101286507323456, 0.79742698535    ,
			    0.101286507323456};
static double vpri42[42] = {0.333333333333333, 0.470142064105115,
			    0.470142064105115, 0.05971587179    ,
			    0.101286507323456, 0.101286507323456,
			    0.79742698535    ,
			    0.333333333333333, 0.470142064105115,
			    0.470142064105115, 0.05971587179    ,
			    0.101286507323456, 0.101286507323456,
			    0.79742698535    ,
			    0.333333333333333, 0.470142064105115,
			    0.470142064105115, 0.05971587179    ,
			    0.101286507323456, 0.101286507323456,
			    0.79742698535    ,
			    0.333333333333333, 0.470142064105115,
			    0.470142064105115, 0.05971587179    ,
			    0.101286507323456, 0.101286507323456,
			    0.79742698535    ,
			    0.333333333333333, 0.470142064105115,
			    0.470142064105115, 0.05971587179    ,
			    0.101286507323456, 0.101286507323456,
			    0.79742698535    ,
			    0.333333333333333, 0.470142064105115,
			    0.470142064105115, 0.05971587179    ,
			    0.101286507323456, 0.101286507323456,
			    0.79742698535};
static double wpri42[42] = {-0.238619186083197,
			    -0.238619186083197,-0.238619186083197,
			    -0.238619186083197,-0.238619186083197,
			    -0.238619186083197,-0.238619186083197,
			    0.238619186083197,
			    0.238619186083197, 0.238619186083197,
			    0.238619186083197, 0.238619186083197,
			    0.238619186083197, 0.238619186083197,
			    -0.661209386466265,
			    -0.661209386466265,-0.661209386466265,
			    -0.661209386466265,-0.661209386466265,
			    -0.661209386466265,-0.661209386466265,
			    0.661209386466265,
			    0.661209386466265, 0.661209386466265,
			    0.661209386466265, 0.661209386466265,
			    0.661209386466265, 0.661209386466265,
			    -0.932469514203152,
			    -0.932469514203152,-0.932469514203152,
			    -0.932469514203152,-0.932469514203152,
			    -0.932469514203152,-0.932469514203152,
			    0.932469514203152,
			    0.932469514203152, 0.932469514203152,
			    0.932469514203152, 0.932469514203152,
			    0.932469514203152, 0.932469514203152};
static double ppri42[42] = {0.5264031763942775E-01, 0.3097453447284393E-01,
			    0.3097453447284393E-01, 0.3097453447284393E-01,
			    0.2946434874279526E-01, 0.2946434874279526E-01,
			    0.2946434874279526E-01,
			    0.5264031763942775E-01, 0.3097453447284393E-01,
			    0.3097453447284393E-01, 0.3097453447284393E-01,
			    0.2946434874279526E-01, 0.2946434874279526E-01,
			    0.2946434874279526E-01,
			    0.4058567696791564E-01, 0.2388136141117854E-01,
			    0.2388136141117854E-01, 0.2388136141117854E-01,
			    0.2271700844087269E-01, 0.2271700844087269E-01,
			    0.2271700844087269E-01,
			    0.4058567696791564E-01, 0.2388136141117854E-01,
			    0.2388136141117854E-01, 0.2388136141117854E-01,
			    0.2271700844087269E-01, 0.2271700844087269E-01,
			    0.2271700844087269E-01,
			    0.1927400539265662E-01, 0.1134118051023053E-01,
			    0.1134118051023053E-01, 0.1134118051023053E-01,
			    0.1078823308874557E-01, 0.1078823308874557E-01,
			    0.1078823308874557E-01,
			    0.1927400539265662E-01, 0.1134118051023053E-01,
			    0.1134118051023053E-01, 0.1134118051023053E-01,
			    0.1078823308874557E-01, 0.1078823308874557E-01,
			    0.1078823308874557E-01};

