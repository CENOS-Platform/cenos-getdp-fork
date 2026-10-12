// GetDP - Copyright (C) 1997-2026 P. Dular and C. Geuzaine, University of Liege
//
// See the LICENSE.txt file for license information. Please report all
// issues on https://gitlab.onelab.info/getdp/getdp/issues.

#include <stdlib.h>
#include "GetDPConfig.h"
#include "ProData.h"
#include "Message.h"

#if defined(HAVE_KERNEL)
#include "GeoData.h"
#endif

extern struct CurrentData Current;

/* The non-symmetric facet functions are selected according to the NumIndex^th
   smallest global node number */

int fcmp_Int2(const void *a, const void *b)
{ return ((struct TwoInt *)a)->Int2 - ((struct TwoInt *)b)->Int2; }

int Get_FacetFunctionIndex(struct Element *Element, int NumEntity, int NumIndex)
{
#if !defined(HAVE_KERNEL)
  Message::Error("Get_FacetFunctionIndex requires Kernel");
  return 0;
#else

  int i, j, *NumNodes;

  if(Element->NumLastElementForSortedNodesByFacet != Element->Num) {
    for(i = 0; i < Element->GeoElement->NbrFacets; i++) {
      NumNodes = Geo_GetNodesOfFacetInElement(Element->GeoElement, i);
      j = 0;
      while(NumNodes[j]) {
        Element->SortedNodesByFacet[i][j].Int1 = NumNodes[j];
        int t = Element->GeoElement->NumNodes[NumNodes[j] - 1];
        // check if the node is a periodic copy of a master (reference) node; if
        // it is, use the global tag of the master node, so that the basis
        // function will be the same in the master facet and its periodic
        // counterpart
        if(Current.GeoData->PeriodicNodes) {
          auto it = Current.GeoData->PeriodicNodes->find(t);
          if(it != Current.GeoData->PeriodicNodes->end()) {
            t = it->second;
            Message::Debug("Found master node %d in BF_Edge_3F", t);
          }
        }
        Element->SortedNodesByFacet[i][j].Int2 = t;
        j++;
      }
      qsort(Element->SortedNodesByFacet[i], j, sizeof(struct TwoInt),
            fcmp_Int2);
    }

    Element->NumLastElementForSortedNodesByFacet = Element->Num;
  }

  return Element->SortedNodesByFacet[NumEntity - 1][NumIndex - 1].Int1;
#endif
}

/* Prisms: lambda_1..3 are the barycentric coordinates of the triangle (u,v) and
   phi_0 = (1-w)/2, phi_1 = (1+w)/2 the linear functions along w. Local nodes
   1-3 lie at w = -1 and nodes 4-6 above them at w = 1. */

static void Prism_Lambda(double u, double v, double l[3], double dl[3][3])
{
  l[0] = 1. - u - v;
  l[1] = u;
  l[2] = v;
  dl[0][0] = -1.;
  dl[0][1] = -1.;
  dl[0][2] = 0.;
  dl[1][0] = 1.;
  dl[1][1] = 0.;
  dl[1][2] = 0.;
  dl[2][0] = 0.;
  dl[2][1] = 1.;
  dl[2][2] = 0.;
}

static void Cross(const double a[3], const double b[3], double c[3])
{
  c[0] = a[1] * b[2] - a[2] * b[1];
  c[1] = a[2] * b[0] - a[0] * b[2];
  c[2] = a[0] * b[1] - a[1] * b[0];
}

/* Facet functions are written f grad(g), so that their curl is
   grad(f) x grad(g). */

/* Triangular facets of a prism carry phi * lambda_p lambda_q grad(lambda_k),
   whose trace is the tetrahedron facet function selected by the same global
   node ranking */

static void Prism_TriangleFacet(struct Element *Element, int NumEntity,
                                int Index, double u, double v, double w,
                                double *f, double df[3], double dg[3])
{
  *f = 0.;
  for(int i = 0; i < 3; i++) df[i] = dg[i] = 0.;
  if(NumEntity != 2 && NumEntity != 5) {
    Message::Error("Facet function on non-triangular facet %d of a PRISM",
                   NumEntity);
    return;
  }
  double l[3], dl[3][3];
  Prism_Lambda(u, v, l, dl);
  int k = (Get_FacetFunctionIndex(Element, NumEntity, Index) - 1) % 3;
  int p = (k + 1) % 3, q = (k + 2) % 3;
  double phi = (NumEntity == 2) ? 0.5 * (1. - w) : 0.5 * (1. + w);
  double dphi = (NumEntity == 2) ? -0.5 : 0.5;
  *f = phi * l[p] * l[q];
  for(int i = 0; i < 3; i++) {
    df[i] = phi * (l[q] * dl[p][i] + l[p] * dl[q][i]);
    dg[i] = dl[k][i];
  }
  df[2] += l[p] * l[q] * dphi;
}

static bool Is_Quadrangle(int Type)
{
  return Type & (QUADRANGLE | QUADRANGLE_2 | QUADRANGLE_2_8N | QUADRANGLE_3 |
                 QUADRANGLE_4);
}

static bool Is_Prism(int Type)
{ return Type & (PRISM | PRISM_2 | PRISM_2_15N | PRISM_3 | PRISM_4); }

// Facet nodes as in Dfn_Quadrangle and Dfn_Prism (GeoEntity.h)
static const int *Get_QuadFacetNodes(int Type, int NumEntity)
{
  static const int quadrangle[4] = {1, 2, 3, 4};
  static const int prism[5][4] = {
    {1, 2, 5, 4}, {0, 0, 0, 0}, {1, 4, 6, 3}, {2, 3, 6, 5}, {0, 0, 0, 0}};
  return Is_Quadrangle(Type) ? quadrangle : prism[NumEntity - 1];
}

/* Quadrangular facets: X[i] vanishes on the facet edge from facet node i to
   node i+1 and equals 1 on the opposite edge, so that its restriction to the
   facet is the same bilinear function seen from every element sharing it. */

static bool Get_QuadFacetFactors(struct Element *Element, int NumEntity,
                                 double u, double v, double w, double X[4],
                                 double dX[4][3])
{
  for(int i = 0; i < 4; i++) X[i] = dX[i][0] = dX[i][1] = dX[i][2] = 0.;

  if(Is_Quadrangle(Element->Type)) {
    X[0] = 0.5 * (1. + v);
    dX[0][1] = 0.5;
    X[1] = 0.5 * (1. - u);
    dX[1][0] = -0.5;
    X[2] = 0.5 * (1. - v);
    dX[2][1] = -0.5;
    X[3] = 0.5 * (1. + u);
    dX[3][0] = 0.5;
    return true;
  }

  if(!Is_Prism(Element->Type)) {
    Message::Error("Quadrangular facet functions are not defined for %s",
                   Element->Type &
                       (PYRAMID | PYRAMID_2 | PYRAMID_2_13N | PYRAMID_3) ?
                     "pyramids" :
                     "this element type");
    return false;
  }
  if(NumEntity == 2 || NumEntity == 5) {
    Message::Error("Quadrangular facet function on triangular facet %d of a "
                   "PRISM",
                   NumEntity);
    return false;
  }

  double l[3], dl[3][3];
  Prism_Lambda(u, v, l, dl);
  const int *n = Get_QuadFacetNodes(Element->Type, NumEntity);
  int triangleVertices = 0;
  for(int i = 0; i < 4; i++) triangleVertices |= 1 << ((n[i] - 1) % 3);
  for(int i = 0; i < 4; i++) {
    int a = n[i] - 1, b = n[(i + 1) % 4] - 1;
    if(a / 3 == b / 3) { // horizontal edge, at the bottom if a / 3 == 0
      X[i] = (a / 3 == 0) ? 0.5 * (1. + w) : 0.5 * (1. - w);
      dX[i][2] = (a / 3 == 0) ? 0.5 : -0.5;
    }
    else { // vertical edge through triangle vertex a % 3
      int other = 0;
      while(!(triangleVertices & (1 << other)) || other == a % 3) other++;
      X[i] = l[other];
      for(int j = 0; j < 3; j++) dX[i][j] = dl[other][j];
    }
  }
  return true;
}

/* Of the four functions (product of the other factors) * grad(X[i]), whose sum
   is the gradient of the facet bubble, the one on the edge between the node
   with the lowest global number and its lower-ranked neighbour is dropped.
   The ranking makes the choice identical in every element sharing the
   facet. */

static int Get_QuadFacetFunctionEdge(struct Element *Element, int NumEntity,
                                     int Index)
{
  const int *n = Get_QuadFacetNodes(Element->Type, NumEntity);
  int rank[4] = {0, 0, 0, 0};
  for(int r = 1; r <= 4; r++) {
    int node = Get_FacetFunctionIndex(Element, NumEntity, r);
    for(int i = 0; i < 4; i++)
      if(n[i] == node) rank[i] = r;
  }
  int o = 0;
  while(o < 3 && rank[o] != 1) o++;
  int e1 = (o + 1) % 4, e2 = (o + 3) % 4, d = (o + 2) % 4;
  if(rank[e2] < rank[e1]) {
    int t = e1;
    e1 = e2;
    e2 = t;
  }
  int i = e2, j = d;
  if(Index == 1) {
    i = o;
    j = e2;
  }
  else if(Index == 2) {
    i = e1;
    j = d;
  }
  return (j == (i + 1) % 4) ? i : j;
}

static void Quad_FacetFunction(struct Element *Element, int NumEntity,
                               int Index, double u, double v, double w,
                               double *f, double df[3], double dg[3])
{
  double X[4], dX[4][3];
  *f = 0.;
  for(int c = 0; c < 3; c++) df[c] = dg[c] = 0.;
  if(!Get_QuadFacetFactors(Element, NumEntity, u, v, w, X, dX)) return;
  int e = Get_QuadFacetFunctionEdge(Element, NumEntity, Index);
  *f = 1.;
  for(int j = 0; j < 4; j++) {
    if(j == e) continue;
    double others = 1.;
    for(int k = 0; k < 4; k++)
      if(k != e && k != j) others *= X[k];
    *f *= X[j];
    for(int c = 0; c < 3; c++) df[c] += others * dX[j][c];
  }
  for(int c = 0; c < 3; c++) dg[c] = dX[e][c];
}

/* ------------------------------------------------------------------------ */
/*  B F _ E d g e _ 3                                                       */
/* ------------------------------------------------------------------------ */

/* ------- */
/*  Edges  */
/* ------- */

#define WrongNumEntity Message::Error("Wrong Edge number in 'BF_Edge_3E'")

void BF_Edge_3E(struct Element *Element, int NumEntity, double u, double v,
                double w, double s[])
{ Message::Error("You should never end up here!"); }

#undef WrongNumEntity

/* -------- */
/*  Facets  */
/* -------- */

#define WrongNumEntity Message::Error("Wrong Face number in 'BF_Edge_3F'")

void BF_Edge_3F(struct Element *Element, int NumEntity, int Index, double u,
                double v, double w, double s[])
{
  switch(Element->Type) {
  case LINE:
  case LINE_2:
  case LINE_3:
  case LINE_4: Message::Error("You should never end up here!"); break;

  case TRIANGLE:
  case TRIANGLE_2:
  case TRIANGLE_3:
  case TRIANGLE_4:
    switch(NumEntity) {
    case 1:
      switch(Get_FacetFunctionIndex(Element, NumEntity, Index)) {
      case 3:
        s[0] = 0.;
        s[1] = (1 - u - v) * u;
        s[2] = 0.;
        break;
      case 1:
        s[0] = -u * v;
        s[1] = -u * v;
        s[2] = 0.;
        break;
      case 2:
        s[0] = (1 - u - v) * v;
        s[1] = 0.;
        s[2] = 0.;
        break;
      }
      break;
    default: WrongNumEntity;
    }
    break;

  case QUADRANGLE:
  case QUADRANGLE_2:
  case QUADRANGLE_2_8N:
  case QUADRANGLE_3:
  case QUADRANGLE_4:
    switch(NumEntity) {
    case 1:
      // WARNING: this should be generalized for 3D elements (hexahedrons), by
      // using the Get_FacetFunctionIndex function the 2D basis functions should
      // be the restriction of the 3D basis functions on each facet for now, the
      // BF_Edge_3F_c function is not implemented
      // switch(Get_FacetFunctionIndex(Element, NumEntity, Index)) {
      switch(Index) {
      case 1: // BF_Edge_3F_a
        s[0] = 45. / 16. * (1 - u) * (1 - v * v);
        s[1] = 45. / 16. * (1 - v) * (1 - u * u);
        s[2] = 0.;
        break;
      case 2: // BF_Edge_3F_b
        s[0] = 45. / 16. * (1 + u) * (v * v - 1);
        s[1] = 45. / 16. * (1 - v) * (1 - u * u);
        s[2] = 0.;
        break;
      case 3: // BF_Edge_3F_c
        Message::Error("You should never end up here!");
        break;
      }
      break;
    default: WrongNumEntity;
    }
    break;

  case TETRAHEDRON:
  case TETRAHEDRON_2:
  case TETRAHEDRON_3:
  case TETRAHEDRON_4:
    switch(NumEntity) {
    case 1:
      switch(Get_FacetFunctionIndex(Element, NumEntity, Index)) {
      case 4:
        s[0] = 0.;
        s[1] = 0.;
        s[2] = u * (1. - u - v - w);
        break;
      case 1:
        s[0] = -u * w;
        s[1] = -u * w;
        s[2] = -u * w;
        break;
      case 2:
        s[0] = (1. - u - v - w) * w;
        s[1] = 0.;
        s[2] = 0.;
        break;
      }
      break;
    case 2:
      switch(Get_FacetFunctionIndex(Element, NumEntity, Index)) {
      case 2:
        s[0] = v * (1 - u - v - w);
        s[1] = 0.;
        s[2] = 0.;
        break;
      case 1:
        s[0] = -u * v;
        s[1] = -u * v;
        s[2] = -u * v;
        break;
      case 3:
        s[0] = 0.;
        s[1] = u * (1. - u - v - w);
        s[2] = 0.;
        break;
      }
      break;
    case 3:
      switch(Get_FacetFunctionIndex(Element, NumEntity, Index)) {
      case 3:
        s[0] = 0.;
        s[1] = (1. - u - v - w) * w;
        s[2] = 0.;
        break;
      case 1:
        s[0] = -v * w;
        s[1] = -v * w;
        s[2] = -v * w;
        break;
      case 4:
        s[0] = 0.;
        s[1] = 0.;
        s[2] = v * (1. - u - v - w);
        break;
      }
      break;
    case 4:
      switch(Get_FacetFunctionIndex(Element, NumEntity, Index)) {
      case 4:
        s[0] = 0.;
        s[1] = 0.;
        s[2] = u * v;
        break;
      case 2:
        s[0] = v * w;
        s[1] = 0.;
        s[2] = 0.;
        break;
      case 3:
        s[0] = 0.;
        s[1] = u * w;
        s[2] = 0.;
        break;
      }
      break;
    default: WrongNumEntity;
    }
    break;

  case HEXAHEDRON:
  case HEXAHEDRON_2:
  case HEXAHEDRON_2_20N:
  case HEXAHEDRON_3:
  case HEXAHEDRON_4:
    switch(NumEntity) {
    default: Message::Error("BF_Edge_3F not ready for HEXAHEDRON");
    }
    break;

  case PRISM:
  case PRISM_2:
  case PRISM_2_15N:
  case PRISM_3:
  case PRISM_4: {
    double f, df[3], dg[3];
    Prism_TriangleFacet(Element, NumEntity, Index, u, v, w, &f, df, dg);
    for(int i = 0; i < 3; i++) s[i] = f * dg[i];
  } break;

  case PYRAMID:
  case PYRAMID_2:
  case PYRAMID_2_13N:
  case PYRAMID_3: // case PYRAMID_4
    switch(NumEntity) {
    default: Message::Error("BF_Edge_3F not ready for PYRAMID");
    }
    break;

  default: Message::Error("Unknown type of Element in BF_Edge_3F"); break;
  }
}

#undef WrongNumEntity

void BF_Edge_3F_a(struct Element *Element, int NumEntity, double u, double v,
                  double w, double s[])
{ BF_Edge_3F(Element, NumEntity, 1, u, v, w, s); }

void BF_Edge_3F_b(struct Element *Element, int NumEntity, double u, double v,
                  double w, double s[])
{ BF_Edge_3F(Element, NumEntity, 2, u, v, w, s); }

void BF_Edge_3F_c(struct Element *Element, int NumEntity, double u, double v,
                  double w, double s[])
{ BF_Edge_3F(Element, NumEntity, 3, u, v, w, s); }

/* -------- */
/*  Volume  */
/* -------- */

void BF_Edge_3V(struct Element *Element, int NumEntity, double u, double v,
                double w, double s[])
{ Message::Error("You should never end up here!"); }

/* ------------------------------------------------------------------------ */
/*  B F _ C u r l E d g e _ 3                                               */
/* ------------------------------------------------------------------------ */

/* ------- */
/*  Edges  */
/* ------- */

#define WrongNumEntity Message::Error("Wrong Edge number in 'BF_CurlEdge_3E'")

void BF_CurlEdge_3E(struct Element *Element, int NumEntity, double u, double v,
                    double w, double s[])
{ Message::Error("You should never end up here!"); }

#undef WrongNumEntity

/* -------- */
/*  Facets  */
/* -------- */

#define WrongNumEntity Message::Error("Wrong Face number in 'BF_CurlEdge_3F'")

void BF_CurlEdge_3F(struct Element *Element, int NumEntity, int Index, double u,
                    double v, double w, double s[])
{
  switch(Element->Type) {
  case LINE:
  case LINE_2:
  case LINE_3:
  case LINE_4: Message::Error("You should never end up here!"); break;

  case TRIANGLE:
  case TRIANGLE_2:
  case TRIANGLE_3:
  case TRIANGLE_4:
    switch(NumEntity) {
    case 1:
      switch(Get_FacetFunctionIndex(Element, NumEntity, Index)) {
      case 3:
        s[0] = 0.;
        s[1] = 0.;
        s[2] = -2.0 * u + 1.0 - v;
        break;
      case 1:
        s[0] = 0.;
        s[1] = 0.;
        s[2] = -v + u;
        break;
      case 2:
        s[0] = 0.;
        s[1] = 0.;
        s[2] = 2.0 * v - 1.0 + u;
        break;
      }
      break;
    default: WrongNumEntity;
    }
    break;

  case QUADRANGLE:
  case QUADRANGLE_2:
  case QUADRANGLE_2_8N:
  case QUADRANGLE_3:
  case QUADRANGLE_4:
    switch(NumEntity) {
    case 1:
      // WARNING: this should be generalized for 3D elements (hexahedrons), by
      // using the Get_FacetFunctionIndex function the 2D basis functions should
      // be the restriction of the 3D basis functions on each facet for now, the
      // BF_CurlEdge_3F_c function is not implemented
      // switch(Get_FacetFunctionIndex(Element, NumEntity, Index)) {
      switch(Index) {
      case 1: // BF_CurlEdge_3F_a
        s[0] = 0.;
        s[1] = 0.;
        s[2] = 45. / 8. * (v - u);
        break;
      case 2: // BF_CurlEdge_3F_b
        s[0] = 0.;
        s[1] = 0.;
        s[2] = -45. / 8. * (v + u);
        break;
      case 3: // BF_CurlEdge_3F_c
        Message::Error("You should never end up here!");
        break;
      }
      break;
    default: WrongNumEntity;
    }
    break;

  case TETRAHEDRON:
  case TETRAHEDRON_2:
  case TETRAHEDRON_3:
  case TETRAHEDRON_4:
    switch(NumEntity) {
    case 1:
      switch(Get_FacetFunctionIndex(Element, NumEntity, Index)) {
      case 4:
        s[0] = -u;
        s[1] = -1. + 2. * u + v + w;
        s[2] = 0.;
        break;
      case 1:
        s[0] = u;
        s[1] = -u + w;
        s[2] = -w;
        break;
      case 2:
        s[0] = 0.;
        s[1] = 1. - u - v - 2. * w;
        s[2] = w;
        break;
      }
      break;
    case 2:
      switch(Get_FacetFunctionIndex(Element, NumEntity, Index)) {
      case 2:
        s[0] = 0.;
        s[1] = -v;
        s[2] = -1. + u + 2. * v + w;
        break;
      case 1:
        s[0] = -u;
        s[1] = v;
        s[2] = u - v;
        break;
      case 3:
        s[0] = u;
        s[1] = 0.;
        s[2] = 1. - 2. * u - v - w;
        break;
      }
      break;
    case 3:
      switch(Get_FacetFunctionIndex(Element, NumEntity, Index)) {
      case 3:
        s[0] = -1. + u + v + 2. * w;
        s[1] = 0.;
        s[2] = -w;
        break;
      case 1:
        s[0] = v - w;
        s[1] = -v;
        s[2] = w;
        break;
      case 4:
        s[0] = 1. - u - 2. * v - w;
        s[1] = v;
        s[2] = 0.;
        break;
      }
      break;
    case 4:
      switch(Get_FacetFunctionIndex(Element, NumEntity, Index)) {
      case 4:
        s[0] = u;
        s[1] = -v;
        s[2] = 0.;
        break;
      case 2:
        s[0] = 0.;
        s[1] = v;
        s[2] = -w;
        break;
      case 3:
        s[0] = -u;
        s[1] = 0.;
        s[2] = w;
        break;
      }
      break;
    default: WrongNumEntity;
    }
    break;

  case HEXAHEDRON:
  case HEXAHEDRON_2:
  case HEXAHEDRON_2_20N:
  case HEXAHEDRON_3:
  case HEXAHEDRON_4:
    switch(NumEntity) {
    default: Message::Error("BF_CurlEdge_3F not ready for HEXAHEDRON");
    }
    break;

  case PRISM:
  case PRISM_2:
  case PRISM_2_15N:
  case PRISM_3:
  case PRISM_4: {
    double f, df[3], dg[3];
    Prism_TriangleFacet(Element, NumEntity, Index, u, v, w, &f, df, dg);
    Cross(df, dg, s);
  } break;

  case PYRAMID:
  case PYRAMID_2:
  case PYRAMID_2_13N:
  case PYRAMID_3: // case PYRAMID_4
    switch(NumEntity) {
    default: Message::Error("BF_CurlEdge_3F not ready for PYRAMID");
    }
    break;

  default: Message::Error("Unknown type of Element in BF_CurlEdge_3F"); break;
  }
}

#undef WrongNumEntity

void BF_CurlEdge_3F_a(struct Element *Element, int NumEntity, double u,
                      double v, double w, double s[])
{ BF_CurlEdge_3F(Element, NumEntity, 1, u, v, w, s); }

void BF_CurlEdge_3F_b(struct Element *Element, int NumEntity, double u,
                      double v, double w, double s[])
{ BF_CurlEdge_3F(Element, NumEntity, 2, u, v, w, s); }

void BF_CurlEdge_3F_c(struct Element *Element, int NumEntity, double u,
                      double v, double w, double s[])
{ BF_CurlEdge_3F(Element, NumEntity, 3, u, v, w, s); }

/* -------- */
/*  Volume  */
/* -------- */

void BF_CurlEdge_3V(struct Element *Element, int NumEntity, double u, double v,
                    double w, double s[])
{ Message::Error("You should never end up here!"); }

/* ------------------------------------------------------------------------ */
/*  B F _ E d g e _ 3 F Q   a n d   B F _ E d g e _ 3 V Z                   */
/* ------------------------------------------------------------------------ */

/* Second-order facet functions of quadrangular facets (prism facets 1, 3, 4,
   and boundary quadrangles, which carry their exact traces) */

static void BF_Edge_3FQ(struct Element *Element, int NumEntity, int Index,
                        double u, double v, double w, double s[])
{
  double f, df[3], dg[3];
  Quad_FacetFunction(Element, NumEntity, Index, u, v, w, &f, df, dg);
  for(int i = 0; i < 3; i++) s[i] = f * dg[i];
}

static void BF_CurlEdge_3FQ(struct Element *Element, int NumEntity, int Index,
                            double u, double v, double w, double s[])
{
  double f, df[3], dg[3];
  Quad_FacetFunction(Element, NumEntity, Index, u, v, w, &f, df, dg);
  Cross(df, dg, s);
}

void BF_Edge_3FQ_a(struct Element *Element, int NumEntity, double u, double v,
                   double w, double s[])
{ BF_Edge_3FQ(Element, NumEntity, 1, u, v, w, s); }

void BF_Edge_3FQ_b(struct Element *Element, int NumEntity, double u, double v,
                   double w, double s[])
{ BF_Edge_3FQ(Element, NumEntity, 2, u, v, w, s); }

void BF_Edge_3FQ_c(struct Element *Element, int NumEntity, double u, double v,
                   double w, double s[])
{ BF_Edge_3FQ(Element, NumEntity, 3, u, v, w, s); }

void BF_CurlEdge_3FQ_a(struct Element *Element, int NumEntity, double u,
                       double v, double w, double s[])
{ BF_CurlEdge_3FQ(Element, NumEntity, 1, u, v, w, s); }

void BF_CurlEdge_3FQ_b(struct Element *Element, int NumEntity, double u,
                       double v, double w, double s[])
{ BF_CurlEdge_3FQ(Element, NumEntity, 2, u, v, w, s); }

void BF_CurlEdge_3FQ_c(struct Element *Element, int NumEntity, double u,
                       double v, double w, double s[])
{ BF_CurlEdge_3FQ(Element, NumEntity, 3, u, v, w, s); }

/* Prism bubble lambda_1 lambda_2 lambda_3 grad(w): without it the curl of the
   space misses linear fields along w whenever the bottom and top triangular
   facets select different facet functions */

void BF_Edge_3VZ(struct Element *Element, int NumEntity, double u, double v,
                 double w, double s[])
{
  s[0] = s[1] = s[2] = 0.;
  if(!Is_Prism(Element->Type)) {
    Message::Error("BF_Edge_3VZ is only defined for prisms");
    return;
  }
  s[2] = (1. - u - v) * u * v;
}

void BF_CurlEdge_3VZ(struct Element *Element, int NumEntity, double u, double v,
                     double w, double s[])
{
  s[0] = s[1] = s[2] = 0.;
  if(!Is_Prism(Element->Type)) {
    Message::Error("BF_CurlEdge_3VZ is only defined for prisms");
    return;
  }
  s[0] = u * (1. - u - 2. * v);
  s[1] = -v * (1. - 2. * u - v);
}
