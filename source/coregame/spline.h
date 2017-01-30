///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Low pass filter
//
// 27-JUL-1999
//


#ifndef __SPLINE_H
#define __SPLINE_H


///////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mathtypes.h"


///////////////////////////////////////////////////////////////////////////
//
// Class Spline
//
class CubicSpline
{
protected:

  // Coefficients
  F32 cx[4], cy[4], cz[4];

  // 1/Length in seconds
  F32 lenInv;

public:

  // Length in seconds
  F32 length;

  // Start position
  Vector e0;

public:

  // Setup coefficients from hermite parameters
  void SetupHermite(const Vector &e0, const Vector &t0, const Vector &e1, const Vector &t1, F32 length = 1.0F);

  // Setup coefficients from bezier parameters
  void SetupBezier(const Vector &p1, const Vector &p2, const Vector &p3, const Vector &p4, F32 length = 1.0F);

  // Step to desired position (0..1)
  Vector Step(F32 t, Vector *tangent = NULL);
};

#endif
