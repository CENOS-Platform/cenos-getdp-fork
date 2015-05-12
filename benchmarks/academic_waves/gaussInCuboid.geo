//========================================================
// Benchmark "spherical scalar waves generated by an initial pulse"
// File: GMSH geometry
//========================================================

Include "gaussInCuboid.dat";

Mesh.CharacteristicLengthMax = lc;

// Points

ptX = {-ldom/2, ldom/2};
ptY = {-ldom/2, ldom/2};
ptZ = {-ldom/2, ldom/2};

pIter=0;
For i In {0:1}
  For j In {0:1}
    For k In {0:1}
      p[pIter]=newp; Point(p[pIter]) = {ptX[i], ptY[j], ptZ[k]};
      pIter++;
    EndFor
  EndFor
EndFor

// Lines

lxIter=0;
lyIter=0;
lzIter=0;
For j In {0:1}
  For k In {0:1}
    pNum1 = k + 2*j;
    pNum2 = pNum1+2*2;
    lx[lxIter]=newl; Line(lx[lxIter]) = {p[pNum1], p[pNum2]};
    lxIter++;
  EndFor
EndFor
For i In {0:1}
  For k In {0:1}
    pNum1 = k + 2*2*i;
    pNum2 = pNum1+2;
    ly[lyIter]=newl; Line(ly[lyIter]) = {p[pNum1], p[pNum2]};
    lyIter++;
  EndFor
EndFor
For i In {0:1}
  For j In {0:1}
    pNum1 = 2*j + 2*2*i;
    pNum2 = pNum1+1;
    lz[lzIter]=newl; Line(lz[lzIter]) = {p[pNum1], p[pNum2]};
    lzIter++;
  EndFor
EndFor

// Surfaces

sxIter=0;
syIter=0;
szIter=0;
For i In {0:1}
  lyNum1 = 2*i;
  lzNum1 = 2*i;
  lyNum2 = lyNum1 + 1;
  lzNum2 = lzNum1 + 1;
  sxLL=newll; Line Loop(sxLL) = {-lz[lzNum1], ly[lyNum1], lz[lzNum2], -ly[lyNum2]};
  sx[sxIter] = news; Plane Surface(sx[sxIter]) = {sxLL};
  sxIter++;
EndFor
For j In {0:1}
  lxNum1 = 2*j;
  lzNum1 = j;
  lxNum2 = lxNum1 + 1;
  lzNum2 = lzNum1 + 2;
  syLL=newll; Line Loop(syLL) = {-lx[lxNum1], lz[lzNum1], lx[lxNum2], -lz[lzNum2]};
  sy[syIter] = news; Plane Surface(sy[syIter]) = {syLL};
  syIter++;
EndFor
For k In {0:1}
  lxNum1 = k;
  lyNum1 = k;
  lxNum2 = lxNum1 + 2;
  lyNum2 = lyNum1 + 2;
  szLL=newll; Line Loop(szLL) = {-ly[lyNum1], lx[lxNum1], ly[lyNum2], -lx[lxNum2]};
  sz[szIter] = news; Plane Surface(sz[szIter]) = {szLL};
  szIter++;
EndFor

// Volumes

vIter = 0;
vSL=newsl; Surface Loop(vSL) = {-sx[0],-sy[0],-sz[0],sx[1],sy[1],sz[1]};
v[vIter] = news; Volume(v[vIter]) = {vSL};

// Physical elements

Physical Surface(BOUNDARY) = {sx[], sy[], sz[]};
Physical Volume(DOMAIN) = {v[]};
