///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 25-NOV-1998
//

#ifndef __FOOTPRINT_PRIVATE_H
#define __FOOTPRINT_PRIVATE_H


#include "footprint.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace FootPrint - Object footprinting system
//
namespace FootPrint
{
  // Setup axis maximums using the given direction
  void SetupMaximums(S32 &xMax, S32 &zMax, S32 xSize, S32 zSize, WorldCtrl::CompassDir dir);

  // Used to calculate zipping and footprint offsets
  void CalculateOffsets
  (
    U32 dir, S32 xArray, S32 zArray, S32 x, S32 z, 
    S32 &xZip, S32 &zZip, S32 &xFoot, S32 &zFoot
  );

  // Create a new footprint instance
  Instance & CreateInstance(MapObj *obj, Placement &place);

  // Delete an existing footprint instance
  void DeleteInstance(MapObj *obj);
}

#endif