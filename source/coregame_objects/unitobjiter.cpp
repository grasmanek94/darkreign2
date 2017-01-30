///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-APR-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "unitobjiter.h"
#include "unitobj.h"
#include "mapobjiter.h"
#include "worldctrl.h"
#include "promote.h"
#include "sight.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace UnitObjIter
//
namespace UnitObjIter
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Pre Defined Filters
  //


  //
  // CanBeSeenBy
  //
  // Can be seen by the unit (requires FilterDataUnit!)
  //
  Bool CanBeSeenBy(UnitObj *subject, const FilterData &filterData)
  {
    // Upgrade filter data to filter data unit
    const FilterDataUnit &filterDataUnit = static_cast<const FilterDataUnit &>(filterData);
    return (filterDataUnit.source->GetCanSee(subject));
  }


  //
  // CanBeSeenByTeam
  //
  // Can be seen by this team
  //
  Bool CanBeSeenByTeam(UnitObj *subject, const FilterData &filterData)
  {
    ASSERT(filterData.team)
    return (subject->TestCanSee(filterData.team->GetId()));
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Tactical
  //

  //
  // Constructor
  //
  Tactical::Tactical(Filter filter, const FilterData &filterData) :
    filter(filter), filterData(filterData) 
  {
    // Using proximity and location figure out which clusters to look at
    startX = Clamp<S32>(0, (S32) ((filterData.location.x - filterData.proximity) * WorldCtrl::ClusterSizeInv()), WorldCtrl::ClusterMapX() - 1);
    endX = Clamp<S32>(0, (S32) ((filterData.location.x + filterData.proximity) * WorldCtrl::ClusterSizeInv()), WorldCtrl::ClusterMapX() - 1);

    startZ = Clamp<S32>(0, (S32) ((filterData.location.z - filterData.proximity) * WorldCtrl::ClusterSizeInv()), WorldCtrl::ClusterMapZ() - 1);
    endZ = Clamp<S32>(0, (S32) ((filterData.location.z + filterData.proximity) * WorldCtrl::ClusterSizeInv()), WorldCtrl::ClusterMapZ() - 1);

    currentX = startX;
    currentZ = startZ;

    // Initialize the iteratator to use the first cluster's list
    cluster.SetList(&WorldCtrl::GetCluster(currentX, currentZ)->unitList);
  }


  //
  // Next
  //
  // Iterate to the next map object
  // Returns NULL if the iterator is finished
  //
  UnitObj * Tactical::Next()
  {
    for (;;)
    {
      UnitObj *obj;

      // Proceed through the list until one of them succeeds the filter or we run out of list
      while ((obj = cluster++) != NULL)
      {
        // Filter out null objects
        if (obj->MapType()->IsNullObj())
        {
          continue;
        }

        // Ignore if unmatched relationship
        if (!Team::TestUnitRelation(obj, filterData.team, filterData.relation))
        {
          continue;
        }

        // Is this object within the given range
        proximity2 = (obj->WorldMatrix().posit - filterData.location).Magnitude2();
        if (proximity2 > filterData.proximity2)
        {
          continue;
        }

        // Now make sure it passes the filter
        if (!filter || filter(obj, filterData))
        {
          // Found something which passed the filter, return it
          return (obj);
        }
      }

      // Ran out of items in the list, proceed to the next cluster
  
      // Are we in the final cluster ?
      if (currentX == endX && currentZ == endZ)
      {
        return (NULL);
      }

      // Move horizontally first and then vertically
      currentX++;
      if (currentX > endX)
      {
        currentX = startX;
        currentZ++;
      }

      // Set the list the iterator is using
      cluster.SetList(&WorldCtrl::GetCluster(currentX, currentZ)->unitList);
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class ClaimRect
  //

  //
  // Constructor
  //
  ClaimRect::ClaimRect(const Point<S32> &p0, const Point<S32> &p1) 
  {
    ASSERT(p0.x <= p1.x)
    ASSERT(p0.z <= p1.z)
    ASSERT(WorldCtrl::CellOnMap(p0.x, p0.z))
    ASSERT(WorldCtrl::CellOnMap(p1.x, p1.z))

    // Convert cells to grains
    WorldCtrl::CellToFirstGrain(p0.x, p0.z, g0.x, g0.z);
    WorldCtrl::CellToLastGrain(p1.x, p1.z, g1.x, g1.z);

    // Initialise iterator
    x = g0.x - 1;
    z = g0.z;

    // Increment the iterator ticker by one
    MapObjIter::IncIterTicker();
  }


  //
  // Iterate to the next object
  //
  UnitObj *ClaimRect::Next()
  {
    for (;;)
    {
      // Advance iterators
      if (++x > g1.x)
      {
        x = g0.x;

        if (++z > g1.z)
        {
          // Ran out of grains
          break;
        }
      }

      // Is there a claim at this grain?
      UnitObj *unit;

      if ((unit = Claim::GetOwner(x, z, Claim::LAYER_LOWER)) != NULL)
      {
        // Prune of duplicates
        if (unit->iterTicker != MapObjIter::GetIterTicker())
        {
          unit->iterTicker = MapObjIter::GetIterTicker();
          return (unit);
        }
      }
    }

    // Didnt find any units
    return (NULL);
  }
}
