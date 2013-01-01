// GetDP - Copyright (C) 1997-2013 P. Dular, C. Geuzaine
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to the public mailing list <getdp@geuz.org>.

/* 1 integration point */

static double xt1[1] = {0.333333333333333};
static double yt1[1] = {0.333333333333333};
static double pt1[1] = {0.5};

/* 3 integration points */

static double xt3[3] = {0.16666666666666,0.66666666666666,0.16666666666666};
static double yt3[3] = {0.16666666666666,0.16666666666666,0.66666666666666};
static double pt3[3] = {0.16666666666666,0.16666666666666,0.16666666666666};

/* 4 integration points */

static double xt4[4] = {0.333333333333333,0.6,0.2,0.2};
static double yt4[4] = {0.3333333333333333,0.2,0.6,0.2};
static double pt4[4] = {-0.28125,.260416666666,.260416666666,.260416666666};

/* 6 integration points */

static double xt6[6] = {0.816847572980459,0.091576213509771,0.091576213509771,
			0.108103018168070,0.445948490915965,0.445948490915965};
static double yt6[6] = {0.091576213509771,0.816847572980459,0.091576213509771,
			0.445948490915965,0.108103018168070,0.445948490915965};
static double pt6[6] = {0.054975871827661,0.054975871827661,0.054975871827661,
			0.111690794839,0.111690794839,0.111690794839};

/* 7 integration points */

static double xt7[7] = {0.333333333333333,0.797426985353087,0.101286507323456,
                        0.101286507323456,0.470142064105115,0.059715871789770,
                        0.470142064105115};
static double yt7[7] = {0.333333333333333,0.101286507323456,0.797426985353087,
                        0.101286507323456,0.059715871789770,0.470142064105115,
                        0.470142064105115};
static double pt7[7] = {0.112500000000000,0.062969590272414,0.062969590272414,
                        0.062969590272414,0.066197076394253,0.066197076394253,
                        0.066197076394253};
/* 12 integration points */

static double xt12[12] = {0.873821971016996,0.063089014491502,0.063089014491502,
                          0.501426509658179,0.249286745170910,0.249286745170910,
                          0.636502499121399,0.310352451033785,0.636502499121399,
                          0.310352451033785,0.053145049844816,0.053145049844816};
static double yt12[12] = {0.063089014491502,0.873821971016996,0.063089014491502,
                          0.249286745170910,0.501426509658179,0.249286745170910,
                          0.310352451033785,0.636502499121399,0.053145049844816,
                          0.053145049844816,0.310352451033785,0.636502499121399};
static double pt12[12] = {0.025422453185104,0.025422453185104,0.025422453185104,
                          0.058393137863189,0.058393137863189,0.058393137863189,
                          0.041425537809187,0.041425537809187,0.041425537809187,
                          0.041425537809187,0.041425537809187,0.041425537809187};

/* 13 integration points */

static double xt13[13] = {0.333333333333333,0.479308067841920,0.260345966079040,
                          0.260345966079040,0.869739794195568,0.065130102902216,
                          0.065130102902216,0.048690315425316,0.312865496004874,
                          0.638444188569810,0.048690315425316,0.312865496004874,
                          0.638444188569810};
static double yt13[13] = {0.333333333333333,0.260345966079040,0.479308067841920,
                          0.260345966079040,0.065130102902216,0.869739794195568,
                          0.065130102902216,0.312865496004874,0.048690315425316,
                          0.048690315425316,0.638444188569810,0.638444188569810,
                          0.312865496004874};
static double pt13[13] = {-0.074785022233841,0.087807628716604,0.087807628716604,
                          0.087807628716604,0.026673617804419,0.026673617804419,
                          0.026673617804419,0.038556880445128,0.038556880445128,
                          0.038556880445128,0.038556880445128,0.038556880445128,
                          0.038556880445128};

/* 16 integration points */

static double xt16[16] = {0.333333333333333,0.081414823414554,0.459292588292723,
                          0.459292588292723,0.658861384496480,0.170569307751760,
                          0.170569307751760,0.898905543365938,0.050547228317031,
                          0.050547228317031,0.008394777409958,0.728492392955404,
                          0.263112829634638,0.008394777409958,0.263112829634638,
                          0.728492392955404};
static double yt16[16] = {0.333333333333333,0.459292588292723,0.081414823414554,
                          0.459292588292723,0.170569307751760,0.658861384496480,
                          0.170569307751760,0.050547228317031,0.898905543365938,
                          0.050547228317031,0.728492392955404,0.008394777409958,
                          0.008394777409958,0.263112829634638,0.728492392955404,
                          0.263112829634638};
static double pt16[16] = {0.072157803838894,0.047545817133643,0.047545817133643,
                          0.047545817133643,0.051608685267359,0.051608685267359,
                          0.051608685267359,0.016229248811599,0.016229248811599,
                          0.016229248811599,0.013615157087217,0.013615157087217,
                          0.013615157087217,0.013615157087217,0.013615157087217,
                          0.013615157087217};

/*
   GAUSS TRIANGLE WITH 1/R SINGULARITY OVER NODE (0,0,0)

   ref.: H. L. G. Pina, J. L. M. Fernandes, C. A. Brebbia,
         Some numerical integration formulae over triangles and
	 squares with a 1/R singularity,
	 Appl. Math. Modelling, Vol 5, June 1981, pp 209--211
*/

/* 1 integration point */

static double xts1[1] = {0.25};
static double yts1[1] = {0.25};
static double pts1[1] = {1.24645048};

/* 3 integration points */

static double xts3[3] = {0.16666667,0.81742619,0.18257381};
static double yts3[3] = {0.16666667,0.18257381,0.81742619};
static double pts3[3] = {0.93483790,0.15580629,0.15580629};

/* 4 integration points */

static double xts4[4] = {0.16385495,0.61114353,0.04756957,0.17753138};
static double yts4[4] = {0.04756957,0.17753138,0.16385495,0.61114353};
static double pts4[4] = {0.31161231,0.31161293,0.31161231,0.31161293};
