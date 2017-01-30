///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Path Searching
//
// 8-SEP-1998
//

#ifndef __PATHSEARCH_PRIV_H
#define __PATHSEARCH_PRIV_H


//
// Includes
//
#include "pathsearch.h"
#include "gameobj.h"
#include "worldctrl.h"
#include "bitarray.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace PathSearch - Waypoint-based path generation
//

namespace PathSearch
{
  // Forward declaration
  class PQueue;

  // Data maintained per cell
  struct Cell
  {
    union
    {
      // AStar searching
      struct
      {
        U16 g, f;
        U8 parent : 2, closed : 1, onpath : 1;
      };

      // Trace searching
      struct
      {
        struct
        {
          U8 parent : 2, onpath : 1, visited : 1;
        } sense[2];
      };
    };

    // Z Marker
    U8 zMark;
  };


  //
  // Request data for a single path request
  //
  struct RequestData
  {
    U32 sx, sz, dx, dz;   // Source and destination points
    U8 tractionType;      // Traction type to use for searching
    UnitObjPtr unit;      // Unit which is requesting the search
    SearchType type;      // Requested search type
    U32 flags;            // Request flags
  };

  // Information for trace search
  struct TraceData
  {
    struct SenseData
    {     
      S32 lastDir;          // Last movement direction 
      Point curPos;         // Current position
      Bool aborted;         // Has this sense been aborted
    } sense[2];             // For "left hand to wall" and "right hand to wall" senses
    
    Point oPos;             // Point at which an obstacle was hit in the direct search
    Point minBound;         // Bounding box from oPos to destination
    Point maxBound;         
    S32 normX;              // Line normal for destination to oPos
    S32 normZ;
    S32 closestSense;       // The sense that owns the closest point
  };


  //
  // Multi-search persistant data
  //
  struct SystemData
  {
    // The pathsearch cell map
    Cell *cellMap;

    // Blocking list bit array
    BitArray2d *blockArray;

    // Bit array for immediate use functions
    BitArray2d *immediateArray;

    // How many cells to process per cycle
    U32 cellsPerCycle;

    // Current number of cells processed this cycle
    U32 cycleCellCount;

    // Current number of cells processed this search
    U32 searchCellCount;

    // Current Z search id's
    U8 *zMarks;

    // Open queue
    PQueue *open;

    // Current search type
    SearchType searchType;

    // Path currently being processed (or NULL if none)
    Path *path;

    // Point closest to destination
    Point closestPoint;

    // Distance to closest point
    S32 closestDistance;

    // Last successfully found path (cleared on each search)
    Path *lastPath;

    // Request data from current path
    RequestData request;

    // Information for a trace search
    TraceData trace;
  };


  //
  // Class Path - Instance class for above type
  //
  class Path
  {
  private:

    // Reference count
    U32 refCount;

  public:

    // List node
    NList<Path>::Node node;

    // Current processing state
    FindState state;

    // Current request data
    RequestData request;

    // List of points to block
    PointList *blockList;  

    // Points that make up the path
    PointList points;

    // Constructor and destructor
    Path(U32 sx, U32 sz, U32 dx, U32 dz, U8 tractionType, UnitObj *unit, SearchType type, U32 flags, PointList *blockList);
    ~Path();

    // Add and remove references to this path
    void AddReference();
    void RemoveReference();

    // Get the current reference count
    U32 ReferenceCount()
    {
      return (refCount);
    }
  };


  // Current system-wide data 
  extern SystemData data;

  // Command handling system
  void InitCmd();
  void DoneCmd();

  // Add a new path to the system
  void AddPath(Path *path);

  // Remove a cell and all its children
  void ConsistentRemove(U32 ax, U32 az, Cell *aCell);

  // Returns true if you can travel from a->b (both cells MUST be on the map)
  // (s is the successor value for a->b)
  Bool CanTravel(U8 traction, U32 ax, U32 az, U32 bx, U32 bz, S32 s, Bool system = TRUE);

  // Get a pathsearch cell
  inline Cell * GetCell(U32 x, U32 z)
  {
    #ifdef DEVELOPMENT

    if (x >= WorldCtrl::CellMapX() || z >= WorldCtrl::CellMapZ())
    {
      ERR_FATAL
      ((
        "Invalid cell access (%u, %u)(%u, %u)", x, z, WorldCtrl::CellMapX(), WorldCtrl::CellMapZ()
      ));
    }

    #endif

    return (&data.cellMap[z * WorldCtrl::CellMapX() + x]);
  }
}

#endif