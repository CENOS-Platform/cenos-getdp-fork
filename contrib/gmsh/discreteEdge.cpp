// Gmsh - Copyright (C) 1997-2009 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.

#include "GmshConfig.h"
#include "GmshMessage.h"
#include "discreteEdge.h"

#if !defined(HAVE_GMSH_EMBEDDED)
#include "Geo.h"
#endif

discreteEdge::discreteEdge(GModel *model, int num) : GEdge(model, num, 0, 0) 
{
#if !defined(HAVE_GMSH_EMBEDDED)
  Curve *c = Create_Curve(num, MSH_SEGM_DISCRETE, 0, 0, 0, -1, -1, 0., 1.);
  Tree_Add(model->getGEOInternals()->Curves, &c);
  CreateReversedCurve(c);
#endif
}

GPoint discreteEdge::point(double p) const 
{
  Msg::Error("Cannot evaluate point on discrete edge");
  return GPoint();
}

SVector3 discreteEdge::firstDer(double par) const 
{
  Msg::Error("Cannot evaluate derivative on discrete edge");
  return SVector3();
}

Range<double> discreteEdge::parBounds(int i) const {
  Msg::Error("Cannot specify bounds for parametric coordinate on discrete edge");
  return Range<double>(0,0);
}
