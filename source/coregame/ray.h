///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Ray Tests
//
// 30-SEP-1998
//


#ifndef __RAY_H
#define __RAY_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mapobjdec.h"
#include "mathtypes.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Ray
//
namespace Ray
{
  // Types of tests
  enum
  {
    SPHERE = 0x0001,
    BOX    = 0x0002,
    POLY   = 0x0004
  };

  // Test to see if the given map object
  Bool Test(MapObj *mapObj, const Vector &src, const Vector &dst, F32 &distance, U32 tests = SPHERE);

  // Test for a collision with ground along a given ray
  Bool TerrainTest(const Vector &start, const Vector &end, F32 tolerance, F32 margin, Bool testWater, F32 *result);

  // Test for collision with terrain
  Bool TerrainTest(Vector pos, Vector end, Vector dir, F32 &dist, Vector &hitPos, F32 margin, Bool testWater);
}

#endif