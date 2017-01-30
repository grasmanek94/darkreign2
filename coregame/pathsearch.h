///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Path Searching
//
// 8-SEP-1998
//

#ifndef __PATHSEARCH_H
#define __PATHSEARCH_H


#include "terraindata.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace PathSearch - Point-based path generation
//
namespace PathSearch
{

  // Enumeration of all search types
  enum SearchType
  {
    // A-star
    ST_ASTAR,

    // Trace search
    ST_TRACE,

    // As the crow flies
    ST_CROW,
  };

  // 
  // A single point on a path
  //
  struct Point
  {
    // Position in game cells
    U32 x, z;

    // NList node
    NList<Point>::Node node;

    // Set the point
    void Set(U32 xIn, U32 zIn)
    {
      x = xIn;
      z = zIn;
    }

    // Constructor
    Point()
    {
    }

    // Constructor
    Point(U32 xIn, U32 zIn)
    {
      Set(xIn, zIn);
    }
  };

  
  //
  // A list of path points
  //
  class PointList : public NList<Point>
  {
  public:

    // Constructor sets the node member
    PointList() : NList<Point>()
    {
      SetNodeMember(&Point::node);
    }

    // Append a new point
    void AppendPoint(U32 x, U32 z)
    {
      Append(new Point(x, z));
    }

    // Prepend a new point
    void PrependPoint(U32 x, U32 z)
    {
      Prepend(new Point(x, z));
    }
  };


  //
  // Possible states for path finding
  //
  enum FindState
  {   
    // No current search request
    FS_IDLE,

    // Request is queued for processing
    FS_QUEUED,

    // Request is currently being processed
    FS_ACTIVE,

    // A valid path has been found (may be partial path)
    FS_FOUND,

    // A valid unobstructed path has been found
    FS_DIRECT,

    // Unable to get to destination, but path to closest point
    FS_CLOSEST,

    // No path could be found
    FS_NOPATH
  };

  
  //
  // The path finder (interface to path searching)
  //
  class Finder
  {
  public:

    enum RequestResult
    {   
      // Source or destination was off the map
      RR_OFFMAP,

      // Source is the same cell as the destination
      RR_SAMECELL,

      // The path request was submitted
      RR_SUBMITTED
    };

    enum RequestFlags
    {
      // Remove redundant points
      RF_OPTIMIZE = 0x0001
    };

  private:

    // Points to the assigned path object
    class Path *path;

  public:

    // Constructor and destructor
    Finder();
    ~Finder();

    // Request a new path be found
    RequestResult RequestPath
    (
      U32 sx, U32 sz, U32 dx, U32 dz, U8 traction, UnitObj *unit = NULL,
      SearchType type = ST_ASTAR, 
      U32 flags = RF_OPTIMIZE, 
      PointList *blockList = NULL
    );

    // Get the requested destination
    void GetDestination(U32 &dx, U32 &dz);

    // Forget current path, if any
    void ForgetPath();

    // Returns the current state
    FindState State();

    // Returns the list of points
    const PointList & GetPointList();
  };


  // Intialize and shutdown system
  void Init();
  void Done();

  // To be called after the mission is loaded
  void NotifyPostMissionLoad();

  // Do a single processing slice
  void ProcessRequests();

  // Can the given balance data be used to move to the given cell
  Bool CanMoveToCell(MoveTable::BalanceData &data, TerrainData::Cell &cell);

  // Can the specified traction type move onto the specified cell
  Bool CanMoveToCell(U8 tractionType, TerrainData::Cell &cell);

  // Same as above method, but returns FALSE if position is off the map
  Bool CanMoveToCell(U8 tractionType, U32 x, U32 z);

  // Can travel directly a->b (FALSE for non-neighbours or diagonal neighbours)
  Bool CanTravel(U8 traction, U32 ax, U32 az, U32 bx, U32 bz);

  // Returns the first claimable grain in the surrounding area
  Bool FindGrain(U32 x, U32 z, S32 &gx, S32 &gz, U8 traction, U32 grains, S32 area = 8);

  // Using the heuristic as a maximizer, return the best cell
  Bool FindConnectedCell(U32 xStart, U32 zStart, U32 &xPos, U32 &zPos, UnitObj &unit, void *context, Bool (UnitObj::*heuristic)(U32 &val, U32 x, U32 z, void *context));

  // Finds the closest movable cell within the given range
  Bool FindClosestCell(U8 tractionType, U32 xStart, U32 zStart, U32 &xPos, U32 &zPos, U32 range);
}

#endif