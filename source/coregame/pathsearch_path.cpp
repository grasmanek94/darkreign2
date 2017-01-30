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


namespace PathSearch 
{ 
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Path
  //

  //
  // Constructor
  //
  Path::Path(U32 sx, U32 sz, U32 dx, U32 dz, U8 tractionType, UnitObj *unit, SearchType type, U32 flags, PointList *blockList) :
    refCount(1),
    blockList(blockList),
    state(FS_QUEUED)
  {
    // Fill in request data
    request.sx = sx;
    request.sz = sz;
    request.dx = dx;
    request.dz = dz;
    request.tractionType = tractionType;
    request.unit = unit;
    request.type = type;
    request.flags = flags;
  }


  //
  // Destructor
  //
  Path::~Path()
  {
    ASSERT(!refCount);

    // Our responsibility to delete the blocklist
    if (blockList)
    {
      blockList->DisposeAll();
      delete blockList;
    }

    // Delete all waypoints
    points.DisposeAll();
  }


  //
  // AddReference
  //
  // Add a reference to this path
  //
  void Path::AddReference()
  {
    ASSERT(refCount != U32_MAX);

    refCount++;
  }


  //
  // RemoveReference
  //
  // Remove a reference to this path
  //
  void Path::RemoveReference()
  {
    ASSERT(refCount);

    refCount--;

    // Are there any more references
    if (!refCount)
    {
      // Go into idle state
      state = FS_IDLE;
    }
  }
}
