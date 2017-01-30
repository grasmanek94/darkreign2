///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Low pass filter
//
// 27-JUL-1999
//


///////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "spline.h"


///////////////////////////////////////////////////////////////////////////
//
// Class CubicSpline
//

//
// CubicSpline::SetupHermite
//
// Setup coefficients from hermite parameters
//
void CubicSpline::SetupHermite(const Vector &p1, const Vector &r1, const Vector &p4in, const Vector &r4, F32 lengthIn)
{
  e0 = p1;
  length = lengthIn;
  lenInv = 1.0F / length;

  // Build the coefficient arrays
  // NOTE: p1 is at the origin and so is cancelled out of calculations
  Vector p4 = p4in - p1;

  cx[0] = -2.0F * p4.x + r1.x + r4.x;
  cx[1] =  3.0F * p4.x - 2.0F * r1.x - r4.x;
  cx[2] =  r1.x;
  cx[3] =  0.0F;

  cy[0] = -2.0F * p4.y + r1.y + r4.y;
  cy[1] =  3.0F * p4.y - 2.0F * r1.y - r4.y;
  cy[2] =  r1.y;
  cy[3] =  0.0F;

  cz[0] = -2.0F * p4.z + r1.z + r4.z;
  cz[1] =  3.0F * p4.z - 2.0F * r1.z - r4.z;
  cz[2] =  r1.z;
  cz[3] =  0.0F;
}


//
// CubicSpline::SetupBezier
//
// Setup coefficients from bezier parameters
//
void CubicSpline::SetupBezier(const Vector &p1, const Vector &p2, const Vector &p3, const Vector &p4in, F32 lengthIn)
{
  e0 = p1;
  length = lengthIn;
  lenInv = 1.0F / length;

  Vector p4 = p4in - p1;
  Vector r1 = (p2 - p1) * 3.0F;
  Vector r4 = (p4 - p3) * 3.0F;

  // Build the coefficient arrays
  // NOTE: p1 is at the origin and so is cancelled out of calculations
  cx[0] =  3.0F * p4.x - 3.0F * r1.x + r4.x;
  cx[1] = -6.0F * p4.x + 3.0F * r1.x;
  cx[2] =  3.0F * p4.x;
  cx[3] =  0.0F;

  cy[0] =  3.0F * p4.y - 3.0F * r1.y + r4.y;
  cy[1] = -6.0F * p4.y + 3.0F * r1.y;
  cy[2] =  3.0F * p4.y;
  cy[3] =  0.0F;

  cz[0] =  3.0F * p4.z - 3.0F * r1.z + r4.z;
  cz[1] = -6.0F * p4.z + 3.0F * r1.z;
  cz[2] =  3.0F * p4.z;
  cz[3] =  0.0F;
}


//
// CubicSpline::Step
//
// Step to desired position
//
Vector CubicSpline::Step(F32 t, Vector *tangent)
{
  F32 t2, t3;
  Vector v;

  t *= lenInv;
  t2 = t * t;
  t3 = t * t2;

  v.x = e0.x + cx[0] * t3 + cx[1] * t2 + cx[2] * t + cx[3];
  v.y = e0.y + cy[0] * t3 + cy[1] * t2 + cy[2] * t + cy[3];
  v.z = e0.z + cz[0] * t3 + cz[1] * t2 + cz[2] * t + cx[3];

  if (tangent)
  {
    tangent->x = (3.0F * cx[0] * t2) + (2.0F * cx[1] * t) + cx[2];
    tangent->y = (3.0F * cy[0] * t2) + (2.0F * cy[1] * t) + cy[2];
    tangent->z = (3.0F * cz[0] * t2) + (2.0F * cz[1] * t) + cz[2];
  }

  return (v);
}
