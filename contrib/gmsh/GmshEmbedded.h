#ifndef _GMSH_EMBEDDED_H_
#define _GMSH_EMBEDDED_H_

#include "NumericEmbedded.h"
#include "Message.h"

class Context_T{
public:
  Context_T()
  {
    lc = 1.;
    hide_unselected = 0;
    geom.tolerance = 1.e-6;
    mesh.reverse_all_normals = 1;
    pick_elements = 0;
  }
  double lc;
  int hide_unselected;
  int pick_elements;
  struct{
    double tolerance;
  } geom;
  struct{
    int reverse_all_normals;
  } mesh;
  unsigned int PACK_COLOR(int,int,int,int){ return 0; }
  int UNPACK_RED(unsigned int){ return 0; }
  int UNPACK_GREEN(unsigned int){ return 0; }
  int UNPACK_BLUE(unsigned int){ return 0; }
  int UNPACK_ALPHA(unsigned int){ return 0; }
};

class smooth_normals{
 public:
  int dummy;
};

class VertexArray{
 public:
  int dummy;
};

#endif
