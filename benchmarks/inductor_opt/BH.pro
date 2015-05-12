Function{
  // nu = 100. + 10. * exp ( 1.8 * b * b )
  // analytical
  nu_1a[] = 100. + 10. * Exp[1.8*SquNorm[$1]] ;
  dnudb2_1a[] = 18. * Exp[1.8*SquNorm[$1]] ;
  h_1a[] = nu_1a[$1]*$1 ;
  dhdb_1a[] = TensorDiag[1,1,1] * nu_1a[$1#1] + 2*dnudb2_1a[#1] * SquDyadicProduct[#1]  ;
  dhdb_1a_NL[] = 2*dnudb2_1a[$1#1] * SquDyadicProduct[#1]  ;

  // interpolated
  Mat1_h = {
    0.0000e+00, 5.5023e+00, 1.1018e+01, 1.6562e+01, 2.2149e+01, 2.7798e+01, 3.3528e+01,
    3.9363e+01, 4.5335e+01, 5.1479e+01, 5.7842e+01, 6.4481e+01, 7.1470e+01, 7.8906e+01,
    8.6910e+01, 9.5644e+01, 1.0532e+02, 1.1620e+02, 1.2868e+02, 1.4322e+02, 1.6050e+02,
    1.8139e+02, 2.0711e+02, 2.3932e+02, 2.8028e+02, 3.3314e+02, 4.0231e+02, 4.9395e+02,
    6.1678e+02, 7.8320e+02, 1.0110e+03, 1.3257e+03, 1.7645e+03, 2.3819e+03, 3.2578e+03,
    4.5110e+03, 6.3187e+03, 8.9478e+03, 1.2802e+04, 1.8500e+04, 2.6989e+04, 3.9739e+04,
    5.9047e+04, 8.8520e+04, 1.3388e+05, 2.0425e+05, 3.1434e+05, 4.8796e+05, 7.6403e+05
  } ;
  Mat1_b = {
    0.0000e+00, 5.0000e-02, 1.0000e-01, 1.5000e-01, 2.0000e-01, 2.5000e-01, 3.0000e-01,
    3.5000e-01, 4.0000e-01, 4.5000e-01, 5.0000e-01, 5.5000e-01, 6.0000e-01, 6.5000e-01,
    7.0000e-01, 7.5000e-01, 8.0000e-01, 8.5000e-01, 9.0000e-01, 9.5000e-01, 1.0000e+00,
    1.0500e+00, 1.1000e+00, 1.1500e+00, 1.2000e+00, 1.2500e+00, 1.3000e+00, 1.3500e+00,
    1.4000e+00, 1.4500e+00, 1.5000e+00, 1.5500e+00, 1.6000e+00, 1.6500e+00, 1.7000e+00,
    1.7500e+00, 1.8000e+00, 1.8500e+00, 1.9000e+00, 1.9500e+00, 2.0000e+00, 2.0500e+00,
    2.1000e+00, 2.1500e+00, 2.2000e+00, 2.2500e+00, 2.3000e+00, 2.3500e+00, 2.4000e+00
  } ;

  Mat1_b2 = List[Mat1_b]^2;
  Mat1_nu = List[Mat1_h]/List[Mat1_b];
  Mat1_nu(0) = Mat1_nu(1);

  Mat1_nu_b2  = ListAlt[Mat1_b2, Mat1_nu] ;
  nu_1[] = InterpolationLinear[ SquNorm[$1] ]{List[Mat1_nu_b2]} ;
  dnudb2_1[] = dInterpolationLinear[SquNorm[$1]]{List[Mat1_nu_b2]} ;
  dnudb_1[] = dInterpolationLinear[SquNorm[$1]]{List[Mat1_nu_b2]}*2.0*$1; //new
  h_1[] = nu_1[$1] * $1 ;
  dhdb_1[] = TensorDiag[1,1,1] * nu_1[$1#1] + 2*dnudb2_1[#1] * SquDyadicProduct[#1]  ;
  dhdb_1_NL[] = 2*dnudb2_1[$1#1] * SquDyadicProduct[#1] ;


  // nu = 123. + 0.0596 * exp ( 3.504 * b * b )
  // analytical 3kW machine
  nu_3kWa[] = 123. + 0.0596 * Exp[3.504*SquNorm[$1]] ;
  dnudb2_3kWa[] = 0.0596*3.504 * Exp[3.504*SquNorm[$1]] ;
  h_3kWa[] = nu_3kWa[$1]*$1 ;
  dhdb_3kWa[] = TensorDiag[1,1,1] * nu_3kWa[$1#1] + 2*dnudb2_3kWa[#1] * SquDyadicProduct[$1]  ;
  dhdb_3kWa_NL[] = 2*dnudb2_3kWa[$1#1] * SquDyadicProduct[#1]  ;

  // interpolated
  Mat3kW_h = {
    0.0000e+00, 6.1465e+00, 1.2293e+01, 1.8440e+01, 2.4588e+01, 3.0736e+01, 3.6886e+01,
    4.3037e+01, 4.9190e+01, 5.5346e+01, 6.1507e+01, 6.7673e+01, 7.3848e+01, 8.0036e+01,
    8.6241e+01, 9.2473e+01, 9.8745e+01, 1.0508e+02, 1.1150e+02, 1.1806e+02, 1.2485e+02,
    1.3199e+02, 1.3971e+02, 1.4836e+02, 1.5856e+02, 1.7137e+02, 1.8864e+02, 2.1363e+02,
    2.5219e+02, 3.1498e+02, 4.2161e+02, 6.0888e+02, 9.4665e+02, 1.5697e+03, 2.7417e+03,
    4.9870e+03, 9.3633e+03, 1.8037e+04, 3.5518e+04, 7.1329e+04, 1.4591e+05, 3.0380e+05,
    6.4363e+05, 1.3872e+06, 3.0413e+06, 6.7826e+06, 1.5386e+07, 3.5504e+07, 8.3338e+07
  } ;
  Mat3kW_b = {
    0.0000e+00, 5.0000e-02, 1.0000e-01, 1.5000e-01, 2.0000e-01, 2.5000e-01, 3.0000e-01,
    3.5000e-01, 4.0000e-01, 4.5000e-01, 5.0000e-01, 5.5000e-01, 6.0000e-01, 6.5000e-01,
    7.0000e-01, 7.5000e-01, 8.0000e-01, 8.5000e-01, 9.0000e-01, 9.5000e-01, 1.0000e+00,
    1.0500e+00, 1.1000e+00, 1.1500e+00, 1.2000e+00, 1.2500e+00, 1.3000e+00, 1.3500e+00,
    1.4000e+00, 1.4500e+00, 1.5000e+00, 1.5500e+00, 1.6000e+00, 1.6500e+00, 1.7000e+00,
    1.7500e+00, 1.8000e+00, 1.8500e+00, 1.9000e+00, 1.9500e+00, 2.0000e+00, 2.0500e+00,
    2.1000e+00, 2.1500e+00, 2.2000e+00, 2.2500e+00, 2.3000e+00, 2.3500e+00, 2.4000e+00
  } ;

  Mat3kW_b2 = List[Mat3kW_b]^2;
  Mat3kW_nu = List[Mat3kW_h]/List[Mat3kW_b];
  Mat3kW_nu(0) = Mat3kW_nu(1);

  Mat3kW_nu_b2  = ListAlt[Mat3kW_b2, Mat3kW_nu] ;
  nu_3kW[] = InterpolationLinear[SquNorm[$1]]{List[Mat3kW_nu_b2]} ;
  dnudb2_3kW[] = dInterpolationLinear[SquNorm[$1]]{List[Mat3kW_nu_b2]} ;
  h_3kW[] = nu_3kW[$1] * $1 ;
  dhdb_3kW[] = TensorDiag[1,1,1]*nu_3kW[$1#1] + 2*dnudb2_3kW[#1] * SquDyadicProduct[#1] ;
  dhdb_3kW_NL[] = 2*dnudb2_3kW[$1] * SquDyadicProduct[$1] ;

  // BH-curve of team 20 problem

  MatT20_h = {
    0.0000e+00,1.3341e+01,2.7000e+01,4.1681e+01,5.8000e+01,7.6769e+01,1.0000e+02,1.2720e+02,
    1.5300e+02,1.7218e+02,1.8500e+02,1.9445e+02,2.0500e+02,2.1877e+02,2.3300e+02,2.4418e+02,
    2.5500e+02,2.6852e+02,2.8500e+02,3.0253e+02,3.2000e+02,3.3647e+02,3.5500e+02,3.7763e+02,
    4.0500e+02,4.3537e+02,4.7000e+02,5.0887e+02,5.5500e+02,6.1009e+02,6.7300e+02,7.4274e+02,
    8.3600e+02,9.5538e+02,1.0700e+03,1.1509e+03,1.2200e+03,1.3063e+03,1.4200e+03,1.5553e+03,
    1.7200e+03,1.9121e+03,2.1300e+03,2.3704e+03,2.6700e+03,3.0458e+03,3.4800e+03,3.9426e+03,
    4.5000e+03,5.1863e+03,5.9500e+03,6.7354e+03,7.6500e+03,8.7596e+03,1.0100e+04,1.1586e+04,
    1.3000e+04,1.4250e+04,1.5900e+04,1.8313e+04,2.1100e+04,2.3656e+04,2.6300e+04,2.9403e+04,
    3.2900e+04,3.6855e+04,4.2700e+04,5.1268e+04,6.1700e+04,7.2615e+04,8.4300e+04,9.6947e+04,
    1.1000e+05,1.2262e+05,1.3500e+05,1.4730e+05} ;

  MatT20_b = {
    0.0000e+00,4.6061e-03,1.0000e-02,1.6915e-02,2.5000e-02,3.4676e-02,5.0000e-02,7.2763e-02,
    1.0000e-01,1.2684e-01,1.5000e-01,1.7058e-01,2.0000e-01,2.4573e-01,3.0000e-01,3.5120e-01,
    4.0000e-01,4.4953e-01,5.0000e-01,5.4984e-01,6.0000e-01,6.4982e-01,7.0000e-01,7.4979e-01,
    8.0000e-01,8.4976e-01,9.0000e-01,9.4973e-01,1.0000e+00,1.0500e+00,1.1000e+00,1.1484e+00,
    1.2000e+00,1.2539e+00,1.3000e+00,1.3288e+00,1.3500e+00,1.3733e+00,1.4000e+00,1.4249e+00,
    1.4500e+00,1.4746e+00,1.5000e+00,1.5246e+00,1.5500e+00,1.5746e+00,1.6000e+00,1.6245e+00,
    1.6500e+00,1.6745e+00,1.7000e+00,1.7245e+00,1.7500e+00,1.7745e+00,1.8000e+00,1.8245e+00,
    1.8500e+00,1.8745e+00,1.9000e+00,1.9245e+00,1.9500e+00,1.9744e+00,2.0000e+00,2.0244e+00,
    2.0500e+00,2.0744e+00,2.1000e+00,2.1244e+00,2.1500e+00,2.1744e+00,2.2000e+00,2.2244e+00,
    2.2500e+00,2.2744e+00,2.3000e+00,2.3243e+00} ;

  MatT20_b2 = List[MatT20_b]^2;
  MatT20_nu = List[MatT20_h]/List[MatT20_b];
  MatT20_nu(0) = MatT20_nu(1);

  MatT20_nu_b2 = ListAlt[MatT20_b2, MatT20_nu] ;
  nu_T20[] = InterpolationLinear[SquNorm[$1]]{List[MatT20_nu_b2]} ;
  dnudb2_T20[] = dInterpolationLinear[SquNorm[$1]]{List[MatT20_nu_b2]} ;
  h_T20[] = nu_T20[$1] * $1 ;
  dhdb_T20[]    = TensorDiag[1,1,1] * nu_T20[$1#1] + 2*dnudb2_T20[#1] * SquDyadicProduct[$1] ;
  dhdb_NL_T20[] = 2*dnudb2_T20[$1] * SquDyadicProduct[$1] ;


  // BH data EIcore

  MatEIcore_h = {0.,
    2.1827e+02,3.8272e+02,5.2036e+02,7.1167e+02,
    8.4921e+02,1.0405e+03,1.4741e+03,2.0409e+03,
    3.0109e+03,5.0572e+03,7.5335e+03,1.0037e+04,
    1.2486e+04,1.5015e+04,1.7464e+04,2.0021e+04,
    2.2040e+04,2.5000e+04} ;

  MatEIcore_b = {0.,
    2.0329e-01,4.0287e-01,6.0986e-01,8.0575e-01,
    1.0053e+00,1.1975e+00,1.4008e+00,1.5154e+00,
    1.5967e+00,1.6706e+00,1.7076e+00,1.7335e+00,
    1.7446e+00,1.7593e+00,1.7667e+00,1.7815e+00,
    1.7889e+00,1.7963e+00} ;

  MatEIcore_b2 = List[MatEIcore_b]^2 ;
  MatEIcore_nu = List[MatEIcore_h]/List[MatEIcore_b] ;
  MatEIcore_nu(0) = MatEIcore_nu(1);

  MatEIcore_nu_b2  = ListAlt[MatEIcore_b2, MatEIcore_nu] ;
  nu_EIcore[] = InterpolationLinear[SquNorm[$1]]{List[MatEIcore_nu_b2]} ;
  dnudb2_EIcore[] = dInterpolationLinear[SquNorm[$1]]{List[MatEIcore_nu_b2]} ;
  dnudb_EIcore[] = dInterpolationLinear[SquNorm[$1]]{List[MatEIcore_nu_b2]}*2.0*$1;//new
  h_EIcore[] = nu_EIcore[$1] * $1 ;
  dhdb_EIcore[] = TensorDiag[1,1,1]*nu_EIcore[$1#1] + 2*dnudb2_EIcore[#1] * SquDyadicProduct[#1] ;
  dhdb_EIcore_NL[] = 2*dnudb2_EIcore[$1] * SquDyadicProduct[$1] ;
  //----------------------
  // nu = 520. + 49.4 * exp ( 1.46 * b * b )
  // analytical
  nu_Park[] = 520.6 + 49.4 * Exp[1.46*SquNorm[$1]] ;
  dnudb2_Park[] = 49.4*1.46*Exp[1.46*SquNorm[$1]] ;// d{nu}/d{b2}
  h_Park[] = nu_Park[$1]*$1 ;
  dhdb_Park[] = TensorDiag[1,1,1]*nu_Park[$1#1]+2*dnudb2_Park[#1]*SquDyadicProduct[#1];
  dhdb_Park_NL[] = 2*dnudb2_Park[$1#1]*SquDyadicProduct[#1];
}






