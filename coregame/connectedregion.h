///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Connected Region
//

#ifndef __CONNECTEDREGION_H
#define __CONNECTEDREGION_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "blobs.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace ConnectedRegion
//
namespace ConnectedRegion
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Defenitions
  //
  typedef Blobs::Pixel Pixel;
  typedef Blobs::Region Region;
  
  // Initialization and Shutdown
  void Init();
  void Done();

  // Get the region number of a pixel in the map
  Pixel GetValue(U8 traction, U32 x, U32 z);

  // For the given pixel get the statistical information for that region
  const Region & GetRegion(U8 traction, Pixel pixel);

  // Get the number of regions available for the given traction type
  U32 GetNumRegions(U8 traction);

  // Recalculate cre data
  void Recalc();

}

#endif
