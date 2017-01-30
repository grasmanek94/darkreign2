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
#include "resourceobjiter.h"
#include "resourceobj.h"
#include "unitobj.h"
#include "worldctrl.h"
#include "sight.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace ResourceObjIter
//
namespace ResourceObjIter
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
  Bool CanBeSeenBy(ResourceObj *resource, const FilterData &filterData)
  {
    // Upgrade filter data to filter data unit
    const FilterDataUnit &filterDataUnit = static_cast<const FilterDataUnit &>(filterData);

    return (filterDataUnit.source->GetCanSee(resource));
  }

  
  //
  // HasBeenSeenByTeam
  //
  // Has it ever been seen by this units team (requires FilterDataUnit!)
  //
  Bool HasBeenSeenByTeam(ResourceObj *resource, const FilterData &filterData)
  {
    // Upgrade filter data to filter data unit
    const FilterDataUnit &filterDataUnit = static_cast<const FilterDataUnit &>(filterData);

    return 
    (
      filterDataUnit.source->GetTeam() && 
      resource->GetSeen(filterDataUnit.source->GetTeam())
    );
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Resource
  //

  //
  // Constructor
  //
  Resource::Resource(Filter filter, const FilterData &filterData) :
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
    cluster.SetList(&WorldCtrl::GetCluster(currentX, currentZ)->resourceList);
  }


  //
  // Next
  //
  // Iterate to the next map object
  // Returns NULL if the iterator is finished
  //
  ResourceObj * Resource::Next()
  {
    for (;;)
    {
      ResourceObj *obj;

      // Proceed through the list until one of them succeeds the filter or we run out of list
      while ((obj = cluster++) != NULL)
      {
        // Filter out null objects
        if (obj->MapType()->IsNullObj())
        {
          continue;
        }

        // Is this object within the given range
        Vector v = obj->WorldMatrix().posit - filterData.location;
        proximity2 = v.Magnitude2();
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
      cluster.SetList(&WorldCtrl::GetCluster(currentX, currentZ)->resourceList);
    }
  }
}
