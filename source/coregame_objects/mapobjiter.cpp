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
#include "mapobjiter.h"
#include "worldctrl.h"
#include "unitobj.h"
#include "promote.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MapObjIter
//
namespace MapObjIter
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //
  U32 iterTicker;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Pre Defined Filters
  //


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class All
  //


  //
  // Constructor
  //
  All::All(Filter filter, const FilterData &filterData) :
    filter(filter), filterData(filterData) 
  {
    // Using proximity and location figure out which clusters to look at
    startX = Clamp((S32) 0, (S32) ((filterData.location.x - filterData.proximity) * WorldCtrl::ClusterSizeInv()), (S32) (WorldCtrl::ClusterMapX() - 1));
    endX = Clamp((S32) 0, (S32) ((filterData.location.x + filterData.proximity) * WorldCtrl::ClusterSizeInv()), (S32) (WorldCtrl::ClusterMapX() - 1));

    startZ = Clamp((S32) 0, (S32) ((filterData.location.z - filterData.proximity) * WorldCtrl::ClusterSizeInv()), (S32) (WorldCtrl::ClusterMapZ() - 1));
    endZ = Clamp((S32) 0, (S32) ((filterData.location.z + filterData.proximity) * WorldCtrl::ClusterSizeInv()), (S32) (WorldCtrl::ClusterMapZ() - 1));

    currentX = startX;
    currentZ = startZ;

    // Initialize the iteratator to use the first cluster's list
    cluster.SetList(&WorldCtrl::GetCluster(currentX, currentZ)->listObjs);

    // Increment the iterator ticker by one
    IncIterTicker();
  }


  //
  // Next
  //
  // Iterate to the next map object
  // Returns NULL if the iterator is finished
  //
  MapObj * All::Next()
  {
    for (;;)
    {
      MapObj *obj;

      // Proceed through the list until one of them succeeds the filter or we run out of list
      while ((obj = cluster++) != NULL)
      {
        // Prune of duplicates
        if (obj->iterTicker != GetIterTicker())
        {
          obj->iterTicker = GetIterTicker();
          
          // Is this object on a team, and there is a specified team
          UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(obj);

          if (!Team::TestUnitRelation(unit, filterData.team, filterData.relation))
          {
            continue;
          }

          // Is any part of this object's bounding bounds within the given range
          const Bounds &bounds = obj->ObjectBounds();

          Vector v = obj->Origin() - filterData.location;

          // FIXME : IS THERE ANY WAY TO DO THIS WITHOUT THE SQUARE ROOT
          F32 proximity = Max<F32>(0.0F, v.Magnitude() - bounds.Radius());
          proximity2 = proximity * proximity;

          if (proximity2 > filterData.proximity2)
          {
            continue;
          }

          // Now make sure it passes the filter
          if (!filter || filter(*cluster, filterData))
          {
            // Found something which passed the filter, return it
            return (obj);
          }
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
      cluster.SetList(&WorldCtrl::GetCluster(currentX, currentZ)->listObjs);
    }

  }


  //
  // Initalise
  //
  void Init()
  {
    iterTicker = 0;
  }
}
