Include "beam_data.geo";

// square
p1 = newp; Point(p1) = {-Lx/2,-Ly/2,-Lz/2, lc};
p2 = newp; Point(p2) = { Lx/2,-Ly/2,-Lz/2, lc};
p3 = newp; Point(p3) = { Lx/2, Ly/2,-Lz/2, lc};
p4 = newp; Point(p4) = {-Lx/2, Ly/2,-Lz/2, lc};
p10 = newp; Point(p10) = {Lx/2, 0,-Lz/2, lc};
l1 = newl; Line(l1) = {p1,p2}; 
l2 = newl; Line(l2) = {p2,p10,p3};
l3 = newl; Line(l3) = {p3,p4}; 
l4 = newl; Line(l4) = {p4,p1};
ll1 = newll; Line Loop(ll1) = {l1,l2,l3,l4};
ll_[] = {ll1};

// hole
If(Flag_hole==1) //ellipse
  p_ec = newp;Point(p_ec) = {0, 0, -Lz/2, lc}; //center
  p_e1 = newp;Point(p_e1) = {hole_length/2, 0, -Lz/2, lc};  //right
  p_e2 = newp;Point(p_e2) = {0, hole_width/2, -Lz/2, lc};   //up
  p_e3 = newp;Point(p_e3) = {-hole_length/2, 0, -Lz/2, lc};  //left
  p_e4 = newp;Point(p_e4) = {0, -hole_width/2, -Lz/2, lc};  //down
  l_e1 = newll;Ellipse(l_e1) = {p_e1, p_ec, p_e1, p_e2};
  l_e2 = newll;Ellipse(l_e2) = {p_e2, p_ec, p_e2, p_e3};
  l_e3 = newll;Ellipse(l_e3) = {p_e3, p_ec, p_e3, p_e4};
  l_e4 = newll;Ellipse(l_e4) = {p_e4, p_ec, p_e4, p_e1}; 
  ll_e = newll;Line Loop(ll_e) = {l_e1,l_e2,l_e3,l_e4};
  ll_[] += -ll_e;
EndIf
If(Flag_hole==2)//spline
  aa[] = {};
  For i In {0:(NSpline-1)}
    theta = i*2*Pi/NSpline;
    aa +=newp;
    Point(aa[i]) = {RSpline~{i}*Cos[theta], RSpline~{i}*Sin[theta], -Lz/2, lc}; 
  EndFor
  l_s = newl;Spline(l_s) = { aa[],aa[0] };
  ll_s = newll; Line Loop(ll_s) = {l_s};
  ll_[] += -ll_s;
EndIf
s1 = news; Plane Surface(s1) = ll_[];

If(transfinite)
  Transfinite Line{l1} = nbE_X;//Using Progression 1.3;
  Transfinite Line{l3} = nbE_X; //Using Progression 1.2;
  Transfinite Line{l2} = nbE_Y;
  Transfinite Line{l4} = nbE_Y;
  Transfinite Surface{s1} = {l1,l2,l3,l4};
  Recombine Surface "*";
EndIf

// Extrude: 3D
If(Flag_extrude) 
  If(!transfinite)
    e1[] = Extrude {0, 0, Lz} { Surface{s1}; };
  EndIf
  If(transfinite)
    Transfinite Surface {s1} = {l1, l2, l3, l4};
    Recombine Surface "*";
    e1[] = Extrude {0, 0, Lz} { Surface{s1}; Layers {nbE_Z}; Recombine;};
  EndIf
  pl[] = Line "*";
  vol[] = {e1[1]};
EndIf

If(!Flag_extrude) //2D
  Physical Surface(BLOC) = {s1}; 
  Physical Line(SURF_GAUCHE) = {l4};
  Physical Line(SURF_HAUT) = {l3}; 
  Physical Line(SURF_DROITE) = {l2};//{l2_1,l2_2}; 
  Physical Line(SURF_BAS) = {l1};
  Physical Point(POINT_1) = {p1};
  Physical Point(POINT_2) = {p2};
  Physical Point(POINT_3) = {p3};
  Physical Point(POINT_4) = {p4};
  Physical Point(POINT_5) = {p10};
  If(Flag_hole)
    Physical Line(HOLE) = {-ll_[1]}; 
  EndIf
EndIf
If(Flag_extrude) //3D
  Printf("pl:",pl[]);
  Physical Volume(BLOC) = vol[]; 
  Physical Surface(SURF_GAUCHE) = e1[5];
  Physical Surface(SURF_HAUT) = e1[4]; 
  Physical Surface(SURF_DROITE) = e1[3]; 
  Physical Surface(SURF_BAS) = e1[2];
  If(!Flag_hole)
    Physical Line(LINE_BAS) = {pl[9]};
  EndIf
  If(Flag_hole==1)//ellipse
    Physical Line(LINE_BAS) = {pl[17]};
  EndIf
  If(Flag_hole==2)//spline
    Physical Line(LINE_BAS) = {pl[11]};
  EndIf
  Physical Point(POINT_1) = p1;
  Physical Point(POINT_2) = p2;
  Physical Point(POINT_3) = p3;
  Physical Point(POINT_4) = p4;
EndIf
