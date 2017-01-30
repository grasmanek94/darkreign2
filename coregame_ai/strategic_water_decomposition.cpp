/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Resource
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_water_decomposition.h"
#include "strategic_object.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Water::Body
  //


  //
  // Constructor
  //
  Water::Body::Body(ConnectedRegion::Pixel pixel, const ConnectedRegion::Region &region)
  : pixel(pixel)
  {
    F32 sizeX = F32(region.maxRow - region.minRow) / WorldCtrl::CellMapX();
    F32 sizeY = F32(region.maxColumn - region.minColumn) / WorldCtrl::CellMapZ();

    mapExpanse = F32(sqrt((sizeX * sizeX + sizeY * sizeY) * 0.5f));
    mapExpanse = Clamp(0.0f, mapExpanse, 1.0f);

    mapCoverage = F32(region.area) / F32(WorldCtrl::CellMapX() * WorldCtrl::CellMapZ());
    mapCoverage = Clamp(0.0f, mapCoverage, 1.0f);

    //LOG_DIAG(("Water Body: %d Exp %.2f Cov %.2f", pixel, mapExpanse, mapCoverage))
  }


  //
  // Destructor
  //
  Water::Body::~Body()
  {
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Water::Decomposition
  //


  // Initialized flag
  U32 Water::Decomposition::initialized = FALSE;

  // Traction type
  U8 Water::Decomposition::traction;

  // Water cluster groups
  NList<Water::Body> Water::Decomposition::bodies(&Body::node);


  //
  // Initialization
  //
  void Water::Decomposition::Init()
  {
    ASSERT(!initialized)
    initialized = TRUE;
    traction = MoveTable::TractionIndex("traction_aquatic_building");
  }


  //
  // Shutdown
  //
  void Water::Decomposition::Done()
  {
    ASSERT(initialized)
    initialized = FALSE;

    bodies.DisposeAll();
  }


  //
  // Reset
  //
  void Water::Decomposition::Reset()
  {
    ASSERT(initialized)

    LOG_AI(("Starting Water Decomposition"))

    U32 numRegions = ConnectedRegion::GetNumRegions(traction);

    for (ConnectedRegion::Pixel pixel = 1; pixel <= numRegions; pixel++)
    {
      const ConnectedRegion::Region &region = ConnectedRegion::GetRegion(traction, pixel);

      // Only consider water regions with more than 200 cells of water
      if (region.area > 200)
      {
        bodies.Append(new Body(pixel, region));
      }
    }

    // Given this region, calculate the map expanse and the map
    LOG_AI(("Finished Water Decomposition"))
  }

}
