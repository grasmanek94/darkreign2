///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Path Searching
//
// 8-SEP-1998
//

//
// Includes
//
#include "pathsearch_priv.h"
#include "terraindata.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class Finder - Public interface to the path searching/following system
//

namespace PathSearch
{
  //
  // Constructor
  //
  Finder::Finder()
  {
    path = NULL;
  }


  //
  // Destructor
  //
  Finder::~Finder()
  {
    // Forget any current path
    ForgetPath();    
  }


  //
  // RequestPath
  //
  // Request a new path be found.  Returns FALSE if request is invalid.
  //
  Finder::RequestResult Finder::RequestPath
  (
    U32 sx, U32 sz, U32 dx, U32 dz, U8 traction, UnitObj *unit,
    SearchType type, U32 flags, PointList *blockList
  )
  {
    // Forget any current path
    ForgetPath();

    // Is destination on the map
    if (!WorldCtrl::CellOnMap(sx, sz) || !WorldCtrl::CellOnMap(dx, dz))
    {
      LOG_DIAG(("Request position is not on the map (%u, %u)->(%u,%u)", sx, sz, dx, dz));
      return (RR_OFFMAP);
    }

    // Filter out requests to move to the same cell
    if (sx == dx && sz == dz)
    {
      return (RR_SAMECELL);
    }

    // Can this traction type move to this cell
    if (!CanMoveToCell(traction, dx, dz))
    {
      U32 xNew, zNew;

      // Find the closest location we can move to
      if (FindClosestCell(traction, dx, dz, xNew, zNew, 15))
      {
        // Use the new location
        dx = xNew;
        dz = zNew;
      }
      else

      // AStar will fail, so jump straight to trace
      if (type == ST_ASTAR)
      {
        type = ST_TRACE;
      }
    }

    // Create a new path
    path = new Path(sx, sz, dx, dz, traction, unit, type, flags, blockList);

    // Add to the system
    AddPath(path);

    // Success
    return (RR_SUBMITTED);
  }


  //
  // GetDestination
  //
  // Get the requested destination
  //
  void Finder::GetDestination(U32 &dx, U32 &dz)
  {
    ASSERT(path);
    dx = path->request.dx;
    dz = path->request.dz;
  }


  //
  // ForgetPath
  //
  // Forget current path, if any
  //
  void Finder::ForgetPath()
  {
    // Do we have a current path
    if (path)
    {
      // Remove our reference
      path->RemoveReference();

      // We no longer have a path
      path = NULL;
    }
  }


  //
  // State
  //
  // Returns the current path state
  //
  FindState Finder::State()
  {
    return (path ? path->state : FS_IDLE);
  }


  //
  // PointList
  //
  // Returns the list of points
  //
  const PointList & Finder::GetPointList()
  {
    ASSERT(path);
    return (path->points);
  }
}

