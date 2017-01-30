///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Path Searching
//
// 8-SEP-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "pathsearch_pqueue.h"
#include "gameobjctrl.h"
#include "movetable.h"
#include "terraindata.h"
#include "team.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
 
// Size of the open queue
#define PS_QUEUESIZE 500

// Give up if considered this many cells
#define PS_ASTAR_SEARCH 1500
#define PS_TRACE_SEARCH 15000

// Number of cells to consider per processing cycle
#define PS_PERCYCLE 1000

// For path diagnostic logging
//#define LOG_PATH LOG_DIAG
#define LOG_PATH(x)



///////////////////////////////////////////////////////////////////////////////
//
// Namespace PathSearch - Point-based path generation
//

namespace PathSearch
{
  // Has the system been initialized
  static Bool initialized = FALSE;

  // Ensure we've been notified of essential events
  static Bool notifiedMission;

  // List of all current paths
  static NList<Path> pathList(&Path::node);

  // Current system-wide data 
  SystemData data;

  // Number of successors that lead from a single cell
  enum { NUM_SUCCESSORS = 4 };

  // Delta values for each possible successor
  static struct { S32 x, z; } successorToDelta[NUM_SUCCESSORS] =
  {
    {0, 1}, {1, 0}, {0, -1}, {-1, 0}
  };

  // Successor value for each possible [dx+1][dz+1] (-1 is invalid)
  static S32 deltaToSuccessor[3][3] =
  {
    -1,  3, -1,
     2, -1,  0,
    -1,  1, -1
  };

  // Returns a rotated successor using a direction/successor pair
  static S32 rotatedSuccessor[4][NUM_SUCCESSORS] =
  {
    0, 1, 2, 3,
    3, 0, 1, 2,
    2, 3, 0, 1,
    1, 2, 3, 0
  };

  // Returns successor index that is opposite to 's'
  #define SuccessorOpposite(s) ((s + NUM_SUCCESSORS / 2) & (NUM_SUCCESSORS - 1))

  // Rotates the successor 'suc' by 'v' for the sense 's'
  #define SuccessorAdvance(s, suc, v) \
    ((s) ? ((suc) + (v)) & (NUM_SUCCESSORS - 1) : ((suc) - (v)) & (NUM_SUCCESSORS - 1))

  // Forward declaration for use in ContinueTrace
  static Bool StartTrace(Path *path, Bool firstTime);


  //
  // UpdateZMarks
  //
  // Increments each Z-mark and clears X-marks if needed
  //
  static void UpdateZMarks()
  {
    // Step through each row
    for (U32 z = 0; z < WorldCtrl::CellMapZ(); z++)
    {
      // Check if mark loops around
      if ((++data.zMarks[z]) == 0)
      {
        // Zero indicates time to clear row, so skip to 1
        data.zMarks[z] = 1;

        // Get pointer to start of row
        Cell *cellRow = GetCell(0, z);

        // Set each mark to zero
        for (U32 x = 0; x < WorldCtrl::CellMapX(); x++)
        {
          cellRow[x].zMark = 0;
        }
      }
    }
  }


  //
  // ConsistentRemove
  //
  // Remove all open cells that are using cell ax,az as a parent.
  // Assumes cell ax,az has already been removed from the open set.
  //
  void ConsistentRemove(U32 ax, U32 az, Cell *aCell)
  {
    // Clear the zMark
    aCell->zMark = 0;

    // Now loop over each successor
    for (U32 s = 0; s < NUM_SUCCESSORS; s++)
    {
      // Get the map position of this cell
      U32 bx = ax + successorToDelta[s].x;
      U32 bz = az + successorToDelta[s].z;

      // Skip any cells that are not on the map
      if (!WorldCtrl::CellOnMap(bx, bz))
      {
        continue;
      }

      // Get a pointer to the cell
      Cell *bCell = GetCell(bx, bz);

      // If cell has been visited by the current search, and leads from aCell
      if (bCell->zMark == data.zMarks[bz] && bCell->parent == SuccessorOpposite(s))
      {
        // If cell is in open set, remove it
        if (!bCell->closed)
        {
          data.open->Remove(bx, bz);
        }

        // Recurse to the next cell
        ConsistentRemove(bx, bz, bCell);
      }
    }
  }


  //
  // CanMoveToCell
  //
  // Can we move onto 'tCell' using the given balance data
  //
  Bool CanMoveToCell(MoveTable::BalanceData &data, TerrainData::Cell &cell)
  {
    // Speed and slope are acceptable
    return ((data.speed > MoveTable::MIN_PASSABLE_SPEED) && (cell.slope < data.slope));
  }


  //
  // CanMoveToCell
  //
  // Calls above method, but first gets the movement balance data
  //
  Bool CanMoveToCell(U8 tractionType, TerrainData::Cell &cell)
  {
    // Get the balance info
    MoveTable::BalanceData &d = MoveTable::GetBalance(cell.surface, tractionType);

    // And call the above method
    return (CanMoveToCell(d, cell));
  }

  
  //
  // CanMoveToCell
  //
  // Calls above method, but first checks that the position is on the map
  // 
  Bool CanMoveToCell(U8 tractionType, U32 x, U32 z)
  {
    return 
    (
      WorldCtrl::CellOnMap(x, z) && 
      CanMoveToCell(tractionType, TerrainData::GetCell(x, z))
    );
  }


  //
  // GetClosestDistance
  //
  // The value used for closest distance calculations
  //
  static S32 GetClosestDistance(U32 x, U32 z)
  {
    return (abs(x - data.request.dx) + abs(z - data.request.dz));
  }


  //
  // RotateSuccessor
  //
  // Returns the rotated successor using the given direction
  //
  static U32 RotateSuccessor(WorldCtrl::CompassDir dir, S32 s)
  {
    ASSERT(dir >= 0 && dir <= 3);
    ASSERT(s >= 0 && s < NUM_SUCCESSORS);

    return (rotatedSuccessor[dir][s]);
  }

  
  //
  // EstimateHeuristic
  //
  // Estimates the total distance from x,z to the current
  // requested destination.
  //
  static S32 EstimateHeuristic(U32 x, U32 z)
  {
    S32 xDelta = abs(x - data.request.dx);
    S32 zDelta = abs(z - data.request.dz);
    
    if (xDelta < zDelta)
    {
      return ((xDelta * 21 + (zDelta - xDelta) * 15));
    }
    else
    {
      return ((zDelta * 21 + (xDelta - zDelta) * 15));
    }
  }


  //
  // CellCostHeuristic
  //
  // Calculates the actual cost of moving from cell A to cell B.  Returns FALSE
  // if transition is impassable, otherwise returns TRUE and cost value.
  // Should be consistent with the estimation heuristic above.
  //
  static Bool CellCostHeuristic
  (
    S32 s, U32 ax, U32 az, U32 bx, U32 bz, 
    FootPrint::Instance *footInstance, FootPrint::Type::Cell *footCell, U16 &cost
  )
  { 
    // Is the destination blocked
    if (data.blockArray->Get2(bx, bz))
    {
      return (FALSE);
    }

    // Is source a second layer cell
    if (footCell)
    {
      ASSERT(footInstance);

      // Can we travel from a->b in this direction
      if (!footCell->CheckDir(RotateSuccessor(footInstance->GetDir(), s)))
      {
        return (FALSE);
      }
    }

    // Now check the destination cell
    if (TerrainData::UseSecondLayer(bx, bz))
    {
      footInstance = &TerrainData::GetFootPrintInstance(bx, bz);
      footCell = &footInstance->GetTypeCell(bx, bz);
      
      // Can we travel from b->a in this direction
      if (!footCell->CheckDir(RotateSuccessor(footInstance->GetDir(), SuccessorOpposite(s))))
      {
        return (FALSE);
      }
    }

    // Get the destination data cell
    TerrainData::Cell &bDataCell = TerrainData::GetCell(bx, bz);

    // Get the balance info
    MoveTable::BalanceData &d = MoveTable::GetBalance
    (
      bDataCell.surface, data.request.tractionType
    );

    // Call above method
    if (CanMoveToCell(d, bDataCell))
    {
      // Get base cost value for this cell
      U16 base = (U16)((ax == bx || az == bz) ? 10 : 14);

      // Set the cost based on the speed of this traction over this cell
      cost = U16(base + (2 * U16(base * (1.0F - d.speed))));

      // If we have a unit, check to see how much enemy 
      // threat there is to this unit in the current cluster 
      // and factor that into the cost
      if (data.request.unit.Alive())
      {
        // Get the cluster from the cell
        MapCluster *cluster = WorldCtrl::CellsToCluster(bx, bz);

        Team *team = data.request.unit->GetTeam();
        U32 ac = data.request.unit->UnitType()->GetArmourClass();

        // How much threat is there to this unit ?
        U32 threat = cluster->ai.EvaluateThreat(team, Relation::ENEMY, ac);
        cost = U16(Clamp<U32>(0, threat + cost, U16_MAX >> 2));

        // How much relative pain is there to this unit ?
        U32 pain = cluster->ai.GetPain(team->GetId(), ac);
        U32 totalPain = AI::Map::GetPain(team->GetId(), ac);

        if (totalPain)
        {
          if (totalPain == pain)
          {
            cost = U16(Clamp<U32>(0, cost + (U32_MAX >> 18), U16_MAX >> 2));
          }
          else
          {
            cost = U16(Clamp<U32>(0, cost + (Utils::Div(pain, 0, totalPain) >> 18), U16_MAX >> 2));
          }
        }
      }

      return (TRUE);
    }

    return (FALSE);
  }


  //
  // SetPathResult
  //
  // Used in each search to signal a result has been achieved, or
  // the path has been postponed until next cycle.  Returns TRUE
  // 
  //
  Bool SetPathResult(FindState state)
  {
    ASSERT(data.path);

    // Set the state of the current path
    data.path->state = state;

    // Now do actions based on the state
    switch (state)
    {
      // More processing required
      case FS_ACTIVE:
        return (FALSE);

      // Set the last successful path
      case FS_FOUND:
      case FS_CLOSEST:
      case FS_DIRECT:
        data.lastPath = data.path;
        data.path = NULL;
        break;

      // Clear the current path
      case FS_NOPATH:
        data.path = NULL;
        break;

      default :
        ERR_FATAL(("Unhandled path result %d", state));
    }

    return (TRUE);
  }


  //
  // SetupBlockArray
  //
  // Setup the cell blocking array with the given list (can be NULL)
  //
  static void SetupBlockArray(PointList *blockList)
  {
    // Clear the array
    data.blockArray->Reset(0);

    // Do we have a point list
    if (blockList)
    {
      for (PointList::Iterator i(blockList); *i; i++)
      {
        data.blockArray->Set2((*i)->x, (*i)->z);
      }
    }
  }

  
  //
  // CheckLayer
  //
  // True if the given cell has the same 'second level' flag
  //
  static Bool CheckLayer(Bool flag, U32 x, U32 z)
  {
    return (TerrainData::UseSecondLayer(x, z) == flag);
  }


  //
  // OptimizePath
  //
  // 1. Remove repeated points that may have been created by a trace search
  // 2. Remove dog-legs by adding diagonal cell transitions where possible
  // 3. Remove the first point on the path (after any possible diagonal)
  // 4. Remove obsolete points where path has a continuous slope
  //
  static void OptimizePath()
  {
    ASSERT(data.path);

    // Is optimization requested
    if (!(data.path->request.flags & Finder::RF_OPTIMIZE))
    {
      return;
    }

    S32 slopeX = 0, slopeZ = 0;
    Bool first = TRUE;
    
    // Get the first node in the path
    NList<Point>::Node *aNode = data.path->points.GetHeadNode();
  
    while (aNode)
    {
      Point *a = aNode->GetData();
      NList<Point>::Node *bNode = aNode->GetNext();
      NList<Point>::Node *aPrev = aNode->GetPrev();

      // Is the previous node obsolete because it's in a line
      if (aPrev)
      {
        Point *p = aPrev->GetData();
        S32 newSlopeX = a->x - p->x;
        S32 newSlopeZ = a->z - p->z;
      
        // Is this the first point OR same position OR redundant point
        if (first || (!newSlopeX && !newSlopeZ) || (slopeX == newSlopeX && slopeZ == newSlopeZ))
        {
          data.path->points.Dispose(p);
          first = FALSE;
        }
        else
        {
          slopeX = newSlopeX;
          slopeZ = newSlopeZ;
        }
      }

      if (bNode)
      { 
        Point *b = bNode->GetData();
        NList<Point>::Node *cNode = bNode->GetNext();

        if (cNode)
        {
          // Get the third point
          Point *c = cNode->GetData();

          // Get delta from first to third point
          S32 cdx = a->x - c->x;
          S32 cdz = a->z - c->z;

          // Is this a dog-leg
          if ((cdx == 1 || cdx == -1) && (cdz == 1 || cdz == -1))
          {
            // Get delta from first to second point
            S32 bdx = a->x - b->x;
            S32 bdz = a->z - b->z;
          
            // Get delta to opposite of second point
            S32 ddx = bdx ? 0 : -cdx;
            S32 ddz = bdz ? 0 : -cdz;

            // Get position of opposite point
            Point d(a->x + ddx, a->z + ddz);

            // Can we move to the fourth cell (we know it's on the map)
            if (CanMoveToCell(data.path->request.tractionType, TerrainData::GetCell(d.x, d.z)))
            {
              // Get the layer flag for the first cell
              Bool flag = TerrainData::UseSecondLayer(a->x, a->z);

              // All four cells must be on the same layer
              if 
              (
                CheckLayer(flag, b->x, b->z) && 
                CheckLayer(flag, c->x, c->z) && 
                CheckLayer(flag, d.x, d.z)
              )
              {
                // Remove the second point and skip to the third
                data.path->points.Dispose(b);
                aNode = cNode;
                continue;
              }
            }
          }
        }
      }

      // Just skip to the next node
      aNode = bNode;
    }
  }


  //
  // TransferPoints
  //
  // Transfers all points in 'src' to the front of 'dst', 
  // maintaining their original order.
  //
  static void TransferPoints(PointList &src, PointList &dst, Bool prepend)
  {
    // Now add the direct path
    PointList::Iterator i(&src);
    Point *p;

    if (prepend)
    {
      // Start from the end
      i.GoToTail();

      // Add each point to the start of our path
      while ((p = i--) != NULL)
      {
        // Remove from src and add to dst
        src.Unlink(p);       
        dst.Prepend(p);
      }
    }
    else
    {
      // Start from the beginning
      i.GoToHead();

      // Add each point to the end of our path
      while ((p = i++) != NULL)
      {
        // Remove from src and add to dst
        src.Unlink(p);       
        dst.Append(p);
      }
    }
  }


  //
  // ConstructTracePath
  //
  // Builds a path using the sense 's'.  Returns FALSE on failure.
  //  
  static void ConstructTracePath(U32 x, U32 z, U32 s)
  {
    // Make sure we're valid
    ASSERT(WorldCtrl::CellOnMap(x, z));
    ASSERT(data.path);

    // Get the last node in the current path
    PointList::Node *node = data.path->points.GetTailNode();

    // Add all points before the obstacle position
    while (x != data.trace.oPos.x || z != data.trace.oPos.z)
    {
      // Create a new point
      Point *p = new Point(x, z);

      // And insert it after the previously existing path
      data.path->points.InsertAfter(node, p);

      // Get the cell at this location
      Cell *cell = GetCell(x, z);

      // Check for path corruption (debugging)
      if (cell->zMark != data.zMarks[z])
      {
        ERR_FATAL(("Hit a cell that wasn't in the last search!"));
      }

      // Is this cell already on our path (debugging)
      if (cell->sense[s].onpath)
      {
        ERR_FATAL(("Tried to add cell that was already on path! (%u, %u)", x, z));
      }

      // Flag that this cell has been added
      cell->onpath = TRUE;

      // Get the parent position
      x = x + successorToDelta[cell->sense[s].parent].x;
      z = z + successorToDelta[cell->sense[s].parent].z;
    }
  }


  //
  // SideOfTraceLine
  //
  // Returns which side of the trace normal 'pos' is on (-1, 0, or 1)
  //
  S32 SideOfTraceLine(const Point &pos)
  {
    S32 dx = (S32)pos.x - (S32)data.trace.oPos.x;
    S32 dz = (S32)pos.z - (S32)data.trace.oPos.z;
    
    S32 dot = dx * data.trace.normX + dz * data.trace.normZ;

    return ((dot == 0) ? 0 : (dot < 0) ? -1 : 1);
  }


  //
  // ReachedLine
  //
  // Returns TRUE if search has reached the obstacle->destination line
  //
  Bool ReachedLine(Point &curPos, Point &pos)
  {
    // Get delta from start
    U32 dx = abs(pos.x - data.trace.oPos.x);
    U32 dz = abs(pos.z - data.trace.oPos.z);

    return
    (
      // Inside the obstacle->destination bounding box
      pos.x >= data.trace.minBound.x && pos.x <= data.trace.maxBound.x  &&
      pos.z >= data.trace.minBound.z && pos.z <= data.trace.maxBound.z  &&

      // Not on first step (stop false trigger, but avoid the diagonal situation)
      ((dx > 1 || dz > 1) || (dx == 1 && dz == 1)) &&

      // Just crossed the obstacle->destination line
      SideOfTraceLine(curPos) != SideOfTraceLine(pos)
    );
  }


  //
  // CanTravel
  //
  // Returns true if you can travel from a->b (both cells MUST be on the map)
  // (s is the successor value for a->b)
  //
  Bool CanTravel(U8 traction, U32 ax, U32 az, U32 bx, U32 bz, S32 s, Bool system)
  {
    ASSERT(WorldCtrl::CellOnMap(ax, az));
    ASSERT(WorldCtrl::CellOnMap(bx, bz));
    ASSERT(s >= 0 && s < NUM_SUCCESSORS);

    // Is the destination blocked
    if (system && data.blockArray->Get2(bx, bz))
    {
      return (FALSE);
    }
    
    // Check the source cell
    if (TerrainData::UseSecondLayer(ax, az))
    {
      FootPrint::Instance &i = TerrainData::GetFootPrintInstance(ax, az);
      FootPrint::Type::Cell &c = i.GetTypeCell(ax, az);

      // Can we travel to successor
      if (!c.CheckDir(RotateSuccessor(i.GetDir(), s)))
      {
        return (FALSE);
      }
    }

    // Get the destination cell
    TerrainData::Cell &bCell = TerrainData::GetCell(bx, bz);

    // Can we move to this cell
    if (CanMoveToCell(traction, bCell))
    {
      // Does it use the second layer
      if (TerrainData::UseSecondLayer(bx, bz))
      {       
        FootPrint::Instance &i = FootPrint::GetInstance(bCell.footIndex);
        FootPrint::Type::Cell &c = i.GetTypeCell(bx, bz);
     
        // Can we travel from b->a in this direction
        if (!c.CheckDir(RotateSuccessor(i.GetDir(), SuccessorOpposite(s))))
        {
          return (FALSE);
        }
      }
    }
    else
    {
      // Unable to move onto this cell
      return (FALSE);
    }

    // Success
    return (TRUE);
  }


  //
  // CanTravel
  //
  // Can the given traction type travel directly from one cell to another.
  // Will return false for non-neighbours or diagonal neighbours.
  //
  Bool CanTravel(U8 traction, U32 ax, U32 az, U32 bx, U32 bz)
  {
    S32 dx = bx - ax + 1;
    S32 dz = bz - az + 1;

    // Must be a neighbour
    if (dx >= 0 && dx <= 2 && dz >= 0 && dz <= 2)
    {
      // Get the successor value
      S32 s = deltaToSuccessor[dx][dz];

      // Must not be a diagonal
      if (s != -1)
      {
        // Can we move to this position
        return (CanTravel(traction, ax, az, bx, bz, s, FALSE));
      }
    }

    return (FALSE);
  }


  //
  // DirectPath
  //
  // Attempts to create a direct path between the source and dest in 'r', which
  // may be the same point.  Returns TRUE if destination was reached.
  //
  static Bool DirectPath(const RequestData &r, PointList &path, Point &end, S32 &endDir)
  {
    // Avoid assumptions made below (needed for repeated trace searches)
    if (r.sx == r.dx && r.sz == r.dz)
    {
      return (TRUE);
    }

    // Setup current position
    U32 x = r.sx;
    U32 z = r.sz;
    U32 px, pz;

    // Save first position
    path.AppendPoint(x, z);
    end.x = x;
    end.z = z;

    // Calculate deltas to destination
    S32 dx = r.dx - x;
    S32 dz = r.dz - z;

    ASSERT(dx || dz);

    // Must move in increments of one
    S32 xInc = 1;
    S32 zInc = 1;

    // Maintain positive deltas and flip the increment values
    if (dx < 0) { xInc = -xInc; dx = -dx; }
    if (dz < 0) { zInc = -zInc; dz = -dz; }
    
    do
    {
      // Save previous position
      px = x;
      pz = z;

      // Move to next position
      if (dx >= dz)
      {
        x += xInc; 
        dx--;
      }     
      else
      {
        z += zInc; 
        dz--;
      }

      // Get deltas plus one for use in the successor lookup table
      S32 cdx = x - px + 1;
      S32 cdz = z - pz + 1;

      ASSERT(cdx >= 0 && cdx <= 2);
      ASSERT(cdz >= 0 && cdz <= 2);

      // Get the direction we're heading in
      endDir = deltaToSuccessor[cdx][cdz];

      // Can we move to this position
      if (CanTravel(r.tractionType, px, pz, x, z, endDir))
      {
        // Add to the end of the path
        path.AppendPoint(x, z);
        end.x = x;
        end.z = z;
      }
      else
      {
        break;
      }
    }
    while (dx || dz);

    ASSERT(endDir >= 0 && endDir < NUM_SUCCESSORS);

    // Did we reach the destination
    return (end.x == r.dx && end.z == r.dz);
  }


  //
  // ConstructTraceToClosest
  //
  // Construct a path to the closest trace location
  //
  static Bool ConstructTraceToClosest()
  {
    // Build the path
    ConstructTracePath(data.closestPoint.x, data.closestPoint.z, data.trace.closestSense); 

    // Do some funky optimization
    OptimizePath();

    // Pass through result
    return (SetPathResult(FS_CLOSEST));
  }


  //
  // ContinueTrace
  //
  // Continues processing of the currently active path.  Returns TRUE
  // if the path was finished and requires no more processing.
  //
  static Bool ContinueTrace()
  {
    ASSERT(data.path);

    // Loop until we get a result
    for(;;)
    {
      // For each sense
      for (U32 s = 0; s < 2; s++)
      {
        // Has this sense been aborted
        if (data.trace.sense[s].aborted) 
        { 
          continue; 
        }

        Point curPos = data.trace.sense[s].curPos;
        U32 otherSense = 1 - s;
        Point pos;

        // Keep our hand to the wall
        S32 dir = SuccessorAdvance(s, data.trace.sense[s].lastDir, 1);
        Cell *cell = NULL;
       
        // Check each successor for an opening
        for (U32 c = 0; c < NUM_SUCCESSORS; c++)
        {
          // Get the position of this successor
          pos.x = curPos.x + successorToDelta[dir].x;
          pos.z = curPos.z + successorToDelta[dir].z;

          // Is it on the map
          if (WorldCtrl::CellOnMap(pos.x, pos.z))
          {
            // Can we move to this cell
            if (CanTravel(data.request.tractionType, curPos.x, curPos.z, pos.x, pos.z, dir))
            {
              // Get pathsearch cell
              cell = GetCell(pos.x, pos.z);
              break;
            }
          }
          else
          {
            // Hit the edge of the map.  Abort and let the other sense do its thang
            data.trace.sense[s].aborted = TRUE;
            break;
          }
          
          // Move direction to next successor
          dir = SuccessorAdvance(s, dir, -1);
        }

        // Unable to move off cell, so start position MUST be stuck in a box
        if (c == NUM_SUCCESSORS)
        {
          return (SetPathResult(FS_NOPATH));
        }

        // Sense may have been aborted
        if (!data.trace.sense[s].aborted)
        {
          // Move to this new position
          data.trace.sense[s].curPos = pos;

          // If neither sense has been to this cell before
          if (cell->zMark != data.zMarks[pos.z])
          {
            // Clear this sense
            cell->sense[s].visited = FALSE;
            cell->sense[s].onpath = FALSE;

            // And the other one
            cell->sense[otherSense].visited = FALSE;
            cell->sense[otherSense].onpath = FALSE;

            // Set the zMark
            cell->zMark = data.zMarks[pos.z];
          }

          // If we've never been here before
          if (!cell->sense[s].visited)
          {
            // Has the other sense been here
            if (cell->sense[otherSense].visited)
            {
              // Check to see if we've collided
              Cell *pCell = GetCell(curPos.x, curPos.z);

              // Did we collide head on
              if (pCell->sense[otherSense].visited && (pCell->sense[otherSense].parent == dir))
              {
                LOG_PATH(("ToClosest: Collision"));
                return (ConstructTraceToClosest());
              }
            }

            // Set the parent
            cell->sense[s].parent = SuccessorOpposite(dir);
            cell->sense[s].visited = TRUE;
          }

          // Get new closest distance
          S32 newDistance = GetClosestDistance(pos.x, pos.z);

          // Update closest point
          if (newDistance < data.closestDistance)
          {
            data.closestPoint.Set(pos.x, pos.z);
            data.closestDistance = newDistance;
            data.trace.closestSense = s;
          }

          // Are we done
          if (ReachedLine(curPos, pos))
          {
            // Build the path
            ConstructTracePath(data.trace.sense[s].curPos.x, data.trace.sense[s].curPos.z, s);

            LOG_PATH(("Trace successful (%u cells) Repeating...", data.searchCellCount));

            // Setup the new source location
            data.path->request.sx = pos.x;
            data.path->request.sz = pos.z;

            // And start another trace search
            return (StartTrace(data.path, FALSE));
          }
          else
          {
            // Continue tracing
            data.trace.sense[s].lastDir = dir;
          }
        }
      }

      // Increment cell processing counters
      data.cycleCellCount++;
      data.searchCellCount++;

      // Have both senses been aborted
      if (data.trace.sense[0].aborted && data.trace.sense[1].aborted)
      {
        LOG_PATH(("ToClosest: Aborted"));
        return (ConstructTraceToClosest());
      }
      else

      // Have we reached our search cell limit
      if (data.searchCellCount > PS_TRACE_SEARCH)
      {
        LOG_PATH(("ToClosest: CellCount"));
        return (ConstructTraceToClosest());
      }
      else

      // Have we reached our cycle cell limit
      if (data.cycleCellCount >= data.cellsPerCycle)
      {
        // More processing required
        return (SetPathResult(FS_ACTIVE));
      }
    }
  }


  //
  // StartTrace
  //
  // Starts a trace search, and returns the value from ContinueTrace
  //
  static Bool StartTrace(Path *path, Bool firstTime = TRUE)
  {
    // New path or repeating an 'allway' trace
    ASSERT(firstTime ? !data.path : data.path == path);

    Point end;
    S32 endDir;

    // Clear the last found path pointer
    data.lastPath = NULL;

    // Setup the block array
    if (firstTime)
    {
      SetupBlockArray(path->blockList);
    }

    // Set the z-marks for the new search
    UpdateZMarks();

    // Path is now active
    path->state = FS_ACTIVE;

    // Set the search type
    data.searchType = ST_TRACE;

    // Record that this path is our focus
    data.path = path;

    // Copy the request data
    data.request = path->request;

    // Ensure path is clear if this is our first time
    if (firstTime)
    {
      data.path->points.DisposeAll();
    }

    // Generate a direct path towards the destination, until we hit something
    if (DirectPath(data.request, data.path->points, data.trace.oPos, endDir))
    {
      // Do some funky optimization
      OptimizePath();

      // Success
      return (SetPathResult(FS_DIRECT));
    }

    // Setup bounding box 
    data.trace.minBound.x = Min(data.request.dx, data.trace.oPos.x);
    data.trace.minBound.z = Min(data.request.dz, data.trace.oPos.z);
    data.trace.maxBound.x = Max(data.request.dx, data.trace.oPos.x);
    data.trace.maxBound.z = Max(data.request.dz, data.trace.oPos.z);

    // Get the obstacle cell
    Cell *oCell = GetCell(data.trace.oPos.x, data.trace.oPos.z);

    // Need to do this for the back-looking sense collision detection
    oCell->sense[0].visited = FALSE;
    oCell->sense[1].visited = FALSE;
    
    // Setup data for each sense
    for (U32 s = 0; s < 2; s++)
    {
      data.trace.sense[s].lastDir = SuccessorAdvance(s, endDir, -1);
      data.trace.sense[s].curPos  = data.trace.oPos;
      data.trace.sense[s].aborted = FALSE;
    }

    // Find delta for destination to obstacle position
    S32 deltaX = (S32)data.request.dx - (S32)data.trace.oPos.x;
    S32 deltaZ = (S32)data.request.dz - (S32)data.trace.oPos.z;

    // Save the normal to this line
    data.trace.normX = -deltaZ;
    data.trace.normZ =  deltaX;

    // Setup closest point data
    data.closestPoint.Set(data.trace.oPos.x, data.trace.oPos.z);
    data.closestDistance = GetClosestDistance(data.closestPoint.x, data.closestPoint.z);
    data.trace.closestSense = 0;

    // Reset our cell count for this search
    data.searchCellCount = 0;

    // Now continue processing
    return (ContinueTrace());
  }

  
  //
  // ConstructAStarPath
  //
  // Builds the path from x,z back to the source location. 
  // Returns FALSE if failed building path.
  //  
  static Bool ConstructAStarPath(U32 x, U32 z)
  {
    ASSERT(WorldCtrl::CellOnMap(x, z));
    ASSERT(data.path);
    ASSERT(!data.path->points.GetCount());

    Cell *cell = GetCell(x, z);

    // Add the destination cell
    data.path->points.PrependPoint(x, z);

    // Walk back over path until we get to the source cell
    while (x != data.request.sx || z != data.request.sz)
    {
      // Get the parent position
      U32 px = x + successorToDelta[cell->parent].x;
      U32 pz = z + successorToDelta[cell->parent].z;

      ASSERT(WorldCtrl::CellOnMap(px, pz));

      // Get the parent cell
      Cell *pCell = GetCell(px, pz);

      // Check for path corruption (debugging)
      if (pCell->zMark != data.zMarks[pz])
      {
        LOG_DIAG(("Hit a cell that wasn't in the last search! (%u, %u)", px, pz));
        return (FALSE);
      }

      // Is this cell already on our path (debugging)
      if (pCell->onpath)
      {
        LOG_DIAG(("Tried to add cell that was already on path! (%u, %u)", px, pz));
        return (FALSE);
      }

      // May have gone infinite (debugging)
      if (data.path->points.GetCount() == 30000)
      {
        LOG_DIAG(("Path generation may be looping!"));
        return (FALSE);
      }

      // Create a new point at this location
      data.path->points.PrependPoint(px, pz);

      // Flag that this cell has been added
      pCell->onpath = TRUE;

      // Move to the parent cell
      x = px;
      z = pz;
      cell = pCell;
    }

    return (TRUE);
  }


  //
  // ContinueAStar
  //
  // Continues processing of the currently active path.  Returns TRUE
  // if the path was finished and requires no more processing.
  //
  static Bool ContinueAStar()
  {
    ASSERT(data.path);

    // Position of cell being processed
    U32 ax = 0, az = 0;

    // Footprint data for the 'a' cell
    FootPrint::Type::Cell *footCell;
    FootPrint::Instance *footInstance;

    // Loop until we reach the limit of the number of cells to process 
    while (data.cycleCellCount < data.cellsPerCycle)
    {
      // Have we considered more than we're allowed
      if (data.searchCellCount > PS_ASTAR_SEARCH)
      {       
        // Save the currently active path
        Path *path = data.path;

        // Terminate this search
        SetPathResult(FS_NOPATH);

        LOG_PATH(("Examined too many cells, switching to ST_TRACE"));

        // And start a trace search
        return (StartTrace(path));
      }

      // Remove next node from the open queue
      if (!data.open->RemoveHighest(ax, az))
      {
        // Construct path to closest point
        if (ConstructAStarPath(data.closestPoint.x, data.closestPoint.z))
        {    
          // Do some funky optimization
          OptimizePath();

          // Success
          return (SetPathResult(FS_CLOSEST));
        }
        else
        {
          // Failed
          return (SetPathResult(FS_NOPATH));
        }
      }

      // Increment cell processing counters
      data.cycleCellCount++;
      data.searchCellCount++;

      // Is this the destination
      if (ax == data.request.dx && az == data.request.dz)
      {
        // Now construct the path
        if (ConstructAStarPath(ax, az))
        {    
          // Do some funky optimization
          OptimizePath();

          // Success
          return (SetPathResult(FS_FOUND));
        }
        else
        {
          // Failed
          return (SetPathResult(FS_NOPATH));
        }
      }

      // Get new closest distance
      S32 newDistance = GetClosestDistance(ax, az);

      // Update closest point
      if (newDistance < data.closestDistance)
      {
        data.closestPoint.Set(ax, az);
        data.closestDistance = newDistance;
      }

      // Get the cell at this position
      Cell *aCell = GetCell(ax, az);

      // Flag aCell as closed NOW because it may be tested in ConsistentRemove
      aCell->closed = TRUE;

      // Setup the footprint cell for use over multiple successors
      if (TerrainData::UseSecondLayer(ax, az))
      {
        footInstance = &TerrainData::GetFootPrintInstance(ax, az);
        footCell = &footInstance->GetTypeCell(ax, az);
      }
      else
      {
        footInstance = NULL;
        footCell = NULL;
      }

      // Now loop over each successor
      for (S32 s = 0; s < NUM_SUCCESSORS; s++)
      {
        U16 cost;

        // Get the map position of this cell
        U32 bx = ax + successorToDelta[s].x;
        U32 bz = az + successorToDelta[s].z;

        // Skip any cells that are not on the map
        if (!WorldCtrl::CellOnMap(bx, bz))
        {
          continue;
        }

        // Calculate the total cost of getting to this cell
        if (!CellCostHeuristic(s, ax, az, bx, bz, footInstance, footCell, cost))
        {
          continue;
        }

        // Get pathsearch cell
        Cell *bCell = GetCell(bx, bz);

        // Calculate the new g-value
        U16 newG = (U16)(aCell->g + cost);

        // Current search has NOT visited bCell yet (not in open or closed)
        if (bCell->zMark != data.zMarks[bz])
        {
          // Setup cell data
          bCell->g = newG;
          bCell->f = (U16)(newG + EstimateHeuristic(bx, bz));
          bCell->parent = SuccessorOpposite(s);
          bCell->closed = FALSE;
          bCell->onpath = FALSE;

          // Insert this cell into the open list
          if (data.open->Insert(bx, bz, bCell->f))
          {
            // Cell was allowed into the queue, so include it in the search
            bCell->zMark = data.zMarks[bz];
          }
        }

        // Current search HAS visited bCell (either in open or closed)
        else
        {
          // Ignore paths that are very close to each other
          //U32 ratio = (bCell->g) * 210 >> 8;

          // Have we found a shorter path to this cell
          //if (newG < ratio)
          if (newG < bCell->g)
          {
            // Setup cell data
            bCell->g = newG;
            bCell->f = (U16)(newG + EstimateHeuristic(bx, bz));
            bCell->parent = SuccessorOpposite(s);

            // Is this cell in the closed set
            if (bCell->closed)
            {
              // Try and insert cell into the open set
              if (data.open->Insert(bx, bz, bCell->f))
              {
                // Success, so remove from the closed set
                bCell->closed = FALSE;
              }
              else
              {
                ConsistentRemove(bx, bz, bCell);
              }
            }
            else
            {
              // Change the position in the priority queue
              data.open->Modify(bx, bz, bCell->f);
            }
          }
        } 
      }
    }

    // Did not finish with this path, requires more processing
    return (SetPathResult(FS_ACTIVE));
  }


  //
  // StartAStar
  //
  // Starts the processing of the active search, then returns the
  // value from ContinueAStar.
  //
  static Bool StartAStar(Path *path)
  {
    ASSERT(!data.path);

    // Clear the last found path pointer
    data.lastPath = NULL;

    // Setup the block array
    SetupBlockArray(path->blockList);

    // Set the z-marks for the new search
    UpdateZMarks();

    // Clear the open set
    data.open->Clear();

    // Path is now active
    path->state = FS_ACTIVE;

    // Set the search type
    data.searchType = ST_ASTAR;

    // Record that this path is our focus
    data.path = path;

    // Copy the request data
    data.request = path->request;

    // If only a short distance, try a direct path
    if (Max<U32>(abs(data.request.sx - data.request.dx), abs(data.request.sz - data.request.dz)) < 20)
    {
      // Data only used in trace searches
      S32 endDir;
      Point oPos;

      // Generate a direct path towards the destination, until we hit something
      if (DirectPath(data.request, data.path->points, oPos, endDir))
      {
        // Do some funky optimization
        OptimizePath();

        // Success
        return (SetPathResult(FS_DIRECT));
      }
    }

    // Ensure path is clear
    data.path->points.DisposeAll();

    // Get start cell
    Cell *sCell = GetCell(data.request.sx, data.request.sz);

    // This is our source, so actual distance is zero
    sCell->g = 0;

    // Now get the estimate from here to our destination
    sCell->f = (U16)EstimateHeuristic(data.request.sx, data.request.sz);

    // Setup closest point data
    data.closestPoint.Set(data.request.sx, data.request.sz);
    data.closestDistance = GetClosestDistance(data.request.sx, data.request.sz);

    // Copy zMark
    sCell->zMark = data.zMarks[data.request.sz];

    // Clear flags
    sCell->closed = FALSE;
    sCell->onpath = FALSE;

    // And place on the open queue
    data.open->Insert(data.request.sx, data.request.sz, sCell->f);

    // Reset our cell count for this search
    data.searchCellCount = 0;

    // Now continue processing
    return (ContinueAStar());
  }


  //
  // StartCrow
  //
  // Begin a crow path search
  //
  static Bool StartCrow(Path *path)
  {
    ASSERT(!data.path);

    // Record that this path is our focus
    data.path = path;

    // Copy the request data
    data.request = path->request;

    // Insert an intermediate point if needed, to keep points at 45/90 degree angles
    ::Point<S32> delta(data.request.dx - data.request.sx, data.request.dz - data.request.sz);
    ::Point<S32> absd(abs(delta.x), abs(delta.z));

    if (absd.x != absd.z)
    {
      // Add an intermediate point
      S32 m = abs(absd.x - absd.z);

      if (absd.x < absd.z)
      {
        // Insert extra point along z axis
        path->points.AppendPoint(data.request.sx, data.request.sz + (delta.z > 0 ? m : -m));
      }
      else
      {
        // Insert extra point along x axis
        path->points.AppendPoint(data.request.sx + (delta.x > 0 ? m : -m), data.request.sz);
      }
    }

    // Append the destination point
    path->points.AppendPoint(data.request.dx, data.request.dz);

    // Done
    return (SetPathResult(FS_DIRECT));
  }


  //
  // AddPath
  //
  // Add a new path to the system
  //
  void AddPath(Path *path)
  {
    // Simply add to the path list
    pathList.Append(path);
  }


  //
  // Init
  //
  // Intialize system
  //
  void Init()
  {
    ASSERT(!initialized);

    // Clear the notificaiton flags
    notifiedMission = FALSE;

    // Initialize the command system
    InitCmd();

    // System now initialized
    initialized = TRUE;
  }


  //
  // NotifyPostMissionLoad
  //
  // To be called after the mission is loaded
  //
  void NotifyPostMissionLoad()
  {
    ASSERT(initialized);
    ASSERT(!notifiedMission);

    // Number of cells to consider per process loop
    data.cellsPerCycle = PS_PERCYCLE;

    // Allocate the cell map
    data.cellMap = new Cell[WorldCtrl::CellMapX() * WorldCtrl::CellMapZ()];

    // Set all members to zero
    Utils::Memset(data.cellMap, 0, WorldCtrl::CellMapX() * WorldCtrl::CellMapZ() * sizeof(Cell));

    // Allocate the blocking bit array
    data.blockArray = new BitArray2d(WorldCtrl::CellMapX(), WorldCtrl::CellMapZ());

    // Allocate the immediate use bit array
    data.immediateArray = new BitArray2d(WorldCtrl::CellMapX(), WorldCtrl::CellMapZ());

    // Allocate the Z marks
    data.zMarks = new U8[WorldCtrl::CellMapZ()];

    // Set initial mark value (zero is reserved)
    U8 m = 1;

    // Setup with incrementing values (minimum rows cleared per search)
    for (U32 z = 0; z < WorldCtrl::CellMapZ(); z++, m = (U8)(m == U8_MAX ? 1 : m + 1))
    {
      // Set the mark
      data.zMarks[z] = m;
    }
    
    // Create the open queue
    data.open = new PQueue(PS_QUEUESIZE);

    // Clear the active path pointer
    data.path = NULL;

    // Flag that we were notified
    notifiedMission = TRUE;
  }


  //
  // Done
  //
  // Shutdown system
  //
  void Done()
  {
    ASSERT(initialized);

    // Delete any remaining paths
    pathList.DisposeAll();

    // Did we get past mission load
    if (notifiedMission)
    {
      // Delete allocated data
      delete data.open;
      delete data.zMarks;
      delete data.cellMap;
      delete data.blockArray;
      delete data.immediateArray;
    }

    // Shutdown the command system
    DoneCmd();

    // System now shutdown
    initialized = FALSE;
  }

  
  //
  // ContinueSearch
  //
  // Continue the current search
  //
  Bool ContinueSearch()
  {
    if (data.searchType == ST_ASTAR)
    {
      return (ContinueAStar());
    }

    ASSERT(data.searchType == ST_TRACE);

    return (ContinueTrace());
  }


  //
  // ProcessRequests
  //
  // Do a single processing slice
  //
  void ProcessRequests()
  {
    ASSERT(initialized);
    ASSERT(notifiedMission);

    // Reset the cell counter
    data.cycleCellCount = 0;

    // Do we need to continue processing an active path
    if (data.path)
    {
      // Has this path become idle since the last cycle
      if (data.path->state == FS_IDLE)
      {
        // No need to continue processing
        data.path = NULL;
      }
      else
      {
        ASSERT(data.path->state == FS_ACTIVE);

        // Keep searching for this path
        if (!ContinueSearch())
        {
          // Still requires more processing
          return;
        }
      }
    }

    ASSERT(!data.path);

    // Create an iterator for all existing paths
    NList<Path>::Iterator i(&pathList);
    Path *path;

    // Step through them all, allowing for deletions along the way
    while (((path = i++) != NULL) && (data.cycleCellCount < data.cellsPerCycle))
    {
      ASSERT(path->state != FS_ACTIVE);

      // State dependent operations
      switch (path->state)
      {
        case FS_IDLE :
        {
          // Clear last path
          if (data.lastPath == path)
          {
            data.lastPath = NULL;
          }

          // Path is no longer being used
          pathList.Dispose(path);
          break;
        }

        case FS_QUEUED :
        {
          // Start a new search for a queued path
          switch (path->request.type)
          {
            case ST_ASTAR:
            {
              if (!StartAStar(path))
              {
                // Requires more processing
                return;
              }
              break;
            }

            case ST_TRACE:
            {
              if (!StartTrace(path))
              {
                // Requires more processing
                return;
              }
              break;
            }

            case ST_CROW:
            {
              if (!StartCrow(path))
              {
                // Requires more processing
                return;
              }
              break;
            }
          }
          break;
        }

        // Ignore, since finders are still pointing at these paths
        case FS_FOUND :
        case FS_NOPATH :
        case FS_DIRECT :
          break;
      }
    }
  }


  //
  // FindClosestCell
  //
  // Finds the closest movable cell within the given range
  //
  Bool FindClosestCell(U8 tractionType, U32 xStart, U32 zStart, U32 &xPos, U32 &zPos, U32 range)
  {
    for (S32 r = 0; r <= (S32)range; r++)
    {
      for (S32 z = 0; z <= r; z++)
      {
        S32 x = r - z;

        // Check ++ quadrant
        xPos = xStart + x;
        zPos = zStart + z;
        if (CanMoveToCell(tractionType, xPos, zPos))
        {
          return TRUE;
        }

        // Check +- quadrant
        xPos = xStart + x;
        zPos = zStart - z;
        if (CanMoveToCell(tractionType, xPos, zPos))
        {
          return TRUE;
        }

        // Check -+ quadrant
        xPos = xStart - x;
        zPos = zStart + z;
        if (CanMoveToCell(tractionType, xPos, zPos))
        {
          return TRUE;
        }

        // Check -- quadrant
        xPos = xStart - x;
        zPos = zStart - z;
        if (CanMoveToCell(tractionType, xPos, zPos))
        {
          return TRUE;
        }
      }
    }

    return FALSE;
  }


  //
  // Returns the first connected cell which satisfied the heuristic
  //
  Bool FindConnectedCell(U32 xStart, U32 zStart, U32 &xPos, U32 &zPos, UnitObj &unit, void *context, Bool (UnitObj::*heuristic)(U32 &val, U32 x, U32 z, void *context))
  {
    ASSERT(WorldCtrl::CellOnMap(xStart, zStart))
    ASSERT(initialized)
    ASSERT(notifiedMission)

    // Get the traction type from the unit
    U8 tractionType = unit.UnitType()->GetTractionIndex(unit.UnitType()->GetDefaultLayer());

    // Return immediately if can not move to this cell
    if (!CanMoveToCell(tractionType, xStart, zStart))
    {
      return (FALSE);
    }

    // List of all cells to be processed
    PointList list;

    // Reset the bit array
    data.immediateArray->Reset(0);

    // Add the first cell
    list.AppendPoint(xStart, zStart);

    // And update the bit array
    data.immediateArray->Set2(xStart, zStart);

    // Maximum score so far
    U32 max = U32_MIN;
    xPos = xStart;
    zPos = zStart;

    // Only process 100 cells
    U32 count = 100;

    // Process the cells
    for (PointList::Iterator i(&list); *i && count--; i++)
    {
      // Get the point
      Point &p = **i;

      U32 value;

      // Check this point to see if it satisfies the heuristic
      if (!(unit.*heuristic)(value, p.x, p.z, context))
      {
        list.DisposeAll();
        return (TRUE);
      }

      // Is this a new max ?
      if (value > max)
      {
        xPos = p.x;
        zPos = p.z;
        max = value;
      }

      // Add successor cells
      for (U32 s = 0; s < NUM_SUCCESSORS; s++)
      {
        // Get the map position of this cell
        U32 sx = p.x + successorToDelta[s].x;
        U32 sz = p.z + successorToDelta[s].z;

        // Is the cell on-map and not yet visited
        if (WorldCtrl::CellOnMap(sx, sz) && !data.immediateArray->Get2(sx, sz))
        {
          // Can we travel between these cells
          if (CanTravel(tractionType, p.x, p.z, sx, sz, s, FALSE))
          {
            // Add the cell
            list.AppendPoint(sx, sz);

            // And update the bit array
            data.immediateArray->Set2(sx, sz);
          }
        }
      }
    }

    list.DisposeAll();

    // Did we find anything ?
    return (max == U32_MIN ? FALSE : TRUE);
  }


  //
  // FindGrain
  //
  // Returns the first claimable grain in the surrounding area
  //
  Bool FindGrain(U32 x, U32 z, S32 &gx, S32 &gz, U8 traction, U32 grains, S32 area)
  {
    ASSERT(WorldCtrl::CellOnMap(x, z))
    ASSERT(initialized)
    ASSERT(notifiedMission)
    ASSERT(grains == 1 || grains == 2)

    // Return immediately if can not move to this cell
    if (!CanMoveToCell(traction, x, z))
    {
      return (FALSE);
    }

    // List of all cells to be processed
    PointList list;

    // Reset the bit array
    data.immediateArray->Reset(0);

    // Add the first cell
    list.AppendPoint(x, z);

    // And update the bit array
    data.immediateArray->Set2(x, z);

    // Process the cells
    for (PointList::Iterator i(&list); *i; i++)
    {
      // Get the point
      Point &p = **i;

      if (Claim::FindGrainInCell(p.x, p.z, gx, gz, grains))
      {
        list.DisposeAll();
        return (TRUE);
      }

      // Is this cell within the area
      if (abs(p.x - x) < area && abs(p.z - z) < area)
      {
        // Add successor cells
        for (U32 s = 0; s < NUM_SUCCESSORS; s++)
        {
          // Get the map position of this cell
          U32 sx = p.x + successorToDelta[s].x;
          U32 sz = p.z + successorToDelta[s].z;

          // Is the cell on-map and not yet visited
          if (WorldCtrl::CellOnMap(sx, sz) && !data.immediateArray->Get2(sx, sz))
          {
            // Can we travel between these cells
            if (CanTravel(traction, p.x, p.z, sx, sz, s, FALSE))
            {
              // Add the cell
              list.AppendPoint(sx, sz);

              // And update the bit array
              data.immediateArray->Set2(sx, sz);
            }
          }
        }
      }
    }

    list.DisposeAll();

    return (FALSE);
  }
}