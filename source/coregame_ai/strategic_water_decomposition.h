/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Water
// 25-MAR-1999
//


#ifndef __STRATEGIC_WATER_DECOMPOSITION_H
#define __STRATEGIC_WATER_DECOMPOSITION_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_water.h"
#include "connectedregion.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Struct Water::Body
  //
  struct Water::Body
  {
    // List node
    NList<Body>::Node node;

    // The connect region this water body belongs to
    Blobs::Pixel pixel;

    // How much of the map can be reached by this body of water
    F32 mapExpanse;

    // How much volume does this map have compared to 
    F32 mapCoverage;

    // Constructor
    Body(ConnectedRegion::Pixel pixel, const ConnectedRegion::Region &region);

    // Destructor
    ~Body();

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Water::Decomposition
  //
  class Water::Decomposition
  {
  private:

    // Initialized flag
    static U32 initialized;

    // Traction type
    static U8 traction;

    // Water bodys groups
    static NList<Body> bodies;

  public:

    // Initialization and Shutdown
    static void Init();
    static void Done();

    // Reset
    static void Reset();

  public:

    // Get bodies groups
    static const NList<Body> & GetBodies()
    {
      return (bodies);
    }

    // Get the traction
    static U8 GetTraction()
    {
      return (traction);
    }

  };

}

#endif
