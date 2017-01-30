/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Placement
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_placement_clusterset.h"
#include "strategic_placement_cluster.h"
#include "strategic_placement_ruleset.h"
#include "sync.h"

//#include "common.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Placement::ClusterSet
  //


  const U32 MAXTESTS = 100;


  //
  // Constructor
  //
  Placement::ClusterSet::ClusterSet(const Point<F32> &origin, RuleSet &ruleSet, UnitObjType &type, F32 orientation, Placement &placement)
  : current(NULL),
    currentIndex(0),
    currentCell(0),
    nextByPosition(&Cluster::nodePosition),
    nextByScore(&Cluster::nodeScore),
    badByPosition(&Cluster::nodePosition),
    origin(origin),
    ruleSet(ruleSet),
    type(type),
    orientation(orientation),
    managerClusterSet
    (
      placement.GetManager().GetClusterSet
      (
        type.GetTractionIndex(type.GetDefaultLayer())
      )
    ),
    object(placement.GetManager().GetObject())
  {
    // Allow the rules to move the origin around
    ruleSet.AdjustOrigin(this->origin, orientation);

    // Given the origin get the initial cluster
    U32 index = WorldCtrl::MetresToClusterIndex(this->origin.x, this->origin.z);

    // We have the current cluster
    current = new Cluster(managerClusterSet.GetInfo(index));
    currentIndex = index;
    currentCell = 0;

    // Add all of the adjacent clusters
    AddAdjacent(index);
  }


  //
  // Destructor
  //
  Placement::ClusterSet::~ClusterSet()
  {
    // Cleanup
    nextByPosition.UnlinkAll();
    nextByScore.DisposeAll();
    badByPosition.DisposeAll();
    delete current;
  }


  //
  // Find a cell for this type
  //
  Placement::ClusterSet::FindResult Placement::ClusterSet::FindCell(Point<U32> &cell, WorldCtrl::CompassDir &dir)
  {
    if (current)
    {
      for (U32 i = 0; i < MAXTESTS; i++)
      {
        // Lets look at the current cluster
        MapCluster *mapCluster = WorldCtrl::GetCluster(currentIndex);

        U32 cellX = WorldCtrl::ClusterToLeftCell(mapCluster->xIndex);
        U32 cellZ = WorldCtrl::ClusterToTopCell(mapCluster->zIndex);

        // Is the current cluster still viable ?
        if 
        (
          current->info.IsViable() && 
          currentCell < (WorldCtrl::ClusterSizeInCells() * WorldCtrl::ClusterSizeInCells())
        )
        {
          LOG_AI(("Checking cluster %d, %d cell %d for '%s'", mapCluster->xIndex, mapCluster->zIndex, currentCell, type.GetName()))

          if (current->info.TestCell(currentCell))
          {
            LOG_AI(("Cell isn't claimed, proceeding"))

            ++currentCell;

            dir = WorldCtrl::GetCompassDirection(orientation);

            // We have found a cell worthy of testing, lets 
            // test the placement of the type at this cell 
            // using the 4 rotations
            cell.x = cellX + (currentCell & WC_CLUSTERCELLMASK);
            cell.z = cellZ + (currentCell >> WC_CLUSTERCELLSHIFT);

            // Make sure that this cell is on the playing field
            if (WorldCtrl::CellOnPlayFieldPoint(cell))
            {
              LOG_AI(("Passed test placing at cell %d, %d", cell.x, cell.z))

              if (PlaceAtCell(cell, dir))
              {
                LOG_AI(("Placed at cell!"))
                //Common::Display::AddMarker(cell.x, cell.z, 1.0f);
                return (FOUND);
              }
              else
              {
                //Common::Display::AddMarker(cell.x, cell.z, 0.3f);
              }
            }
          }
          else
          {
            //cell.x = cellX + (currentCell & WC_CLUSTERCELLMASK);
            //cell.z = cellZ + (currentCell >> WC_CLUSTERCELLSHIFT);
            //Common::Display::AddMarker(cell.x, cell.z, 0.2f);

            ++currentCell;
          }
        }
        else
        {
          LOG_AI(("Cluster %d marked used for '%s'", currentIndex, type.GetName()))

          // Mark this cluster as bad
          badByPosition.Add(currentIndex, current);

          // Get the next cluster with the highest score
          current = nextByScore.GetLast();

          if (current)
          {
            LOG_AI(("Cluster %d next [%f] to be evaluated for '%s'", current->nodePosition.GetKey(), current->nodeScore.GetKey(), type.GetName()))

            currentIndex = current->nodePosition.GetKey();
            currentCell = 0;
            nextByScore.Unlink(current);
            nextByPosition.Unlink(current);

            // Add the adjacent cells to the one we just removed
            AddAdjacent(currentIndex);
          }
          else
          {
            // There are no more clusters to build on!
            return (NOTFOUND);
          }
        }
      }
      return (PENDING);
    }
    else
    {
      return (NOTFOUND);
    }
  }


  //
  // Attempt to place at the given cell
  //
  Bool Placement::ClusterSet::PlaceAtCell(const Point<U32> &cell, WorldCtrl::CompassDir &dir)
  {
    FootPrint::Type *footprint = type.GetFootPrintType();

    // Are we placing a footprinted object ?
    if (footprint)
    {
      FootPrint::Placement placement(footprint);

      Vector v(
        WorldCtrl::CellToMetresX(cell.x), 
        0, 
        WorldCtrl::CellToMetresZ(cell.z)); 

      // Try to place the footprint the four ways
      for (int i = 0; i < 4; i++)
      {
        if
        (
          placement.Check
          (
            v,
            dir,
            FootPrint::Placement::CHECK_IGNOREMOBILE |
            FootPrint::Placement::CHECK_IGNORESHROUD
          )
          == FootPrint::Placement::PR_OK
        )
        {
          //LOG_AI(("Passed placement check"))

          Bool failed = FALSE;

          // Test all of the cells this footprint occupies
          for (S32 z = 0; z < placement.Size().z && !failed; z++)
          {
            for (S32 x = 0; x < placement.Size().x && !failed; x++)
            {
              // Get the cell at this position
              FootPrint::Placement::Cell &cell = placement.GetCell(x, z);

              // Get the actual terrain position
              ASSERT(WorldCtrl::CellOnMap(cell.map.x, cell.map.z))

              if (cell.onFoot && !managerClusterSet.TestCell(cell.map.x, cell.map.z))
              {
                failed = TRUE;
                LOG_AI(("Failed claim test"))
                //Common::Display::AddMarker(cell.map.x, cell.map.z, 2.0f);
                break;
              }
            }
          }

          if (!failed)
          {
            // Remove all of the cells this footprint occupies
            for (S32 z = 0; z < placement.Size().z; z++)
            {
              for (S32 x = 0; x < placement.Size().x; x++)
              {
                // Get the cell at this position
                FootPrint::Placement::Cell &cell = placement.GetCell(x, z);

                // Get the actual terrain position
                ASSERT(WorldCtrl::CellOnMap(cell.map.x, cell.map.z))

                // Tell the manager cluster set to claim this cell
                managerClusterSet.ClaimCell(cell.map.x, cell.map.z);
                //Common::Display::AddMarker(cell.map.x, cell.map.z, 0.5f);
              }
            }

            LOG_AI(("Constructing '%s' at %f,%f [%d,%d] dir %d", type.GetName(), v.x, v.z, cell.x, cell.z, dir))

            return (TRUE);
          }
        }

        switch (dir)
        {
          case WorldCtrl::NORTH: 
            dir = WorldCtrl::SOUTH; 
            break;

          case WorldCtrl::SOUTH: 
            dir = WorldCtrl::EAST; 
            break;

          case WorldCtrl::EAST: 
            dir = WorldCtrl::WEST; 
            break;

          case WorldCtrl::WEST: 
            dir = WorldCtrl::NORTH; 
            break;
        }
      }
      return (FALSE);
    }
    else
    {
      // Check to see if there's already units in that cell
      //return (UnitObjCtrl::GetUnitCount(cell.x, cell.z) ? FALSE : TRUE);
      return (FALSE);
    }
  }


  //
  // Add all of the adjacent clusters to the given cluster
  //
  void Placement::ClusterSet::AddAdjacent(U32 index)
  {
    MapCluster *mapCluster = WorldCtrl::GetCluster(index);

    //
    // There are 8 adjacent clusters to this index
    //
    //  2 | 1 | 3
    // ---+---+---
    //  4 | X | 5
    // ---+---+--
    //  7 | 6 | 8
    //
    // Check to see that each one is 
    // a) on the map
    // b) not already in the next set
    // c) not in the exipred set
    //
    // If it passes all of these tests, evaluate it and add it
    //

    MapCluster *c;

    c = mapCluster->GetPreviousZ();
    if (c)
    {
      AddCluster(*c);

      MapCluster *n = c->GetPreviousX();
      if (n)
      {
        AddCluster(*n);
      }

      n = c->GetNextX();
      if (n)
      {
        AddCluster(*n);
      }
    }

    c = mapCluster->GetPreviousX();
    if (c)
    {
      AddCluster(*c);
    }

    c = mapCluster->GetNextX();
    if (c)
    {
      AddCluster(*c);
    }

    c = mapCluster->GetNextZ();
    if (c)
    {
      AddCluster(*c);

      MapCluster *n = c->GetPreviousX();
      if (n)
      {
        AddCluster(*n);
      }

      n = c->GetNextX();
      if (n)
      {
        AddCluster(*n);
      }
    }
  }


  //
  // Add a cluster
  //
  void Placement::ClusterSet::AddCluster(MapCluster &mapCluster)
  {
    U32 index = mapCluster.GetIndex();

    // Is this cluster already in the next set ?
    if (nextByPosition.Exists(index))
    {
      return;
    }

    // Is this cluster already in the bad set ?
    if (badByPosition.Exists(index))
    {
      return;
    }

    // Evaluate this cluster and add it to the set of next clusters
    Cluster *cluster = new Cluster(managerClusterSet.GetInfo(index));

    F32 score;
    if (cluster->info.IsViable())
    {
      if (ruleSet.Evaluate(RuleSet::Info(object, origin, mapCluster, *cluster, orientation), score))
      {
        LOG_AI(("Cluster %d (%d, %d) added at score %f for '%s'", index, mapCluster.xIndex, mapCluster.zIndex, score, type.GetName()))

        nextByPosition.Add(index, cluster);
        nextByScore.Add(score, cluster);
        return;
      }
      else
      {
        LOG_AI(("Cluster %d failed evaluation '%s'", index, type.GetName()))
      }
    }
    else
    {
      LOG_AI(("Cluster %d is not viable for '%s'", index, type.GetName()))
    }

    badByPosition.Add(index, cluster);
  }


  //
  // Why not just use the constructor with new, cause the compiler is a broken piece of turd!
  //
  Placement::ClusterSet * Placement::ClusterSet::Create(const Point<F32> &origin, RuleSet &ruleSet, UnitObjType &type, F32 orientation, Placement &placement)
  {
    return (new ClusterSet(origin, ruleSet, type, orientation, placement));
  }

}

