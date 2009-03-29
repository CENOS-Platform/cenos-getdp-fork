// Gmsh - Copyright (C) 1997-2009 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.

#ifndef _GMSH_LINEAR_SYSTEM_FULL_H_
#define _GMSH_LINEAR_SYSTEM_FULL_H_

// Interface to a linear system with a FULL matrix

#include "GmshMessage.h"
#include "gmshLinearSystem.h"
#include "GmshMatrix.h"

template <class scalar>
class gmshLinearSystemFull : public gmshLinearSystem<scalar> {
 private:
  gmshMatrix<scalar> *_a;
  gmshVector<scalar> *_b, *_x;
 public :
  gmshLinearSystemFull() : _a(0), _b(0), _x(0){}
  virtual bool isAllocated() const { return _a != 0; }
  virtual void allocate(int _nbRows)
  {
    if(_a) delete _a;
    if(_x) delete _x;
    if(_b) delete _b;
    _a = new gmshMatrix<scalar>(_nbRows, _nbRows);
    _b = new gmshVector<scalar>(_nbRows);
    _x = new gmshVector<scalar>(_nbRows);
  }
  virtual ~gmshLinearSystemFull()
  {
    delete _a;
    delete _b;
    delete _x;
  }
  virtual void addToMatrix(int _row, int _col, scalar _val)
  {
    if(_val != 0.0) (*_a)(_row, _col) += _val;
  }
  virtual scalar getFromMatrix(int _row, int _col) const
  {
    return (*_a)(_row, _col);
  }
  virtual void addToRightHandSide(int _row, scalar _val)
  {
    if(_val != 0.0) (*_b)(_row) += _val;
  }
  virtual scalar getFromRightHandSide(int _row) const 
  {
    return (*_b)(_row);
  }
  virtual scalar getFromSolution(int _row) const 
  {
    return (*_x)(_row);
  }
  virtual void zeroMatrix() 
  {
    _a->set_all(0.);
  }
  virtual void zeroRightHandSide()
  {
    for(int i = 0; i < _b->size(); i++) (*_b)(i) = 0.;
  }
  virtual int systemSolve() 
  {
    _a->lu_solve(*_b, *_x);
    return 1;
  }
};

#endif
