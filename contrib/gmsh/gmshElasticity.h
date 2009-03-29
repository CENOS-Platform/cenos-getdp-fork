// Gmsh - Copyright (C) 1997-2009 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.

#ifndef _GMSH_ELASTICITY_H_
#define _GMSH_ELASTICITY_H_

#include "gmshTermOfFormulation.h"
#include "Gmsh.h"
#include "GModel.h"
#include "MElement.h"
#include "GmshMatrix.h"

class gmshElasticityTerm : public gmshNodalFemTerm<double> {
  double _E, _nu;
  int _iField;
  SVector3 _f;
 protected:
  virtual int sizeOfR(MElement *e) const { return 3 * e->getNumVertices(); }
  virtual int sizeOfC(MElement *e) const { return 3 * e->getNumVertices(); }
  void getLocalDofR(MElement *e, int iRow, MVertex **vR, int *iCompR, int *iFieldR) const
  {
    *iCompR = iRow / e->getNumVertices();
    int ithLocalVertex = iRow % e->getNumVertices();
    *vR = e->getVertex(ithLocalVertex);
    *iFieldR = _iField;
  }
 public:
 gmshElasticityTerm(GModel *gm, double E, double nu, int iField = 1) : 
  gmshNodalFemTerm<double>(gm), _E(E), _nu(nu), _iField(iField){}
  void setVector(const SVector3 &f) {_f = f;}
  void elementMatrix(MElement *e, gmshMatrix<double> &m) const;
  void elementVector(MElement *e, gmshVector<double> &m) const;
};

#endif
