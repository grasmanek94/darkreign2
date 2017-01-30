/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Resource
// 25-MAR-1999
//


#ifndef __STRATEGIC_RESOURCE_DECOMPOSITION_H
#define __STRATEGIC_RESOURCE_DECOMPOSITION_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_resource.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Struct Resource::ClusterGroup
  //
  struct Resource::ClusterGroup
  {
    // List node
    NList<ClusterGroup>::Node node;

    // Map Clusters which this cluster group covers
    List<MapCluster> clusters;

    // The resource objects which make up this cluster group
    ResourceObjList resources;

    // The id of this cluster group
    U32 id;

    // The total amount of resource 
    U32 resource;

    // The total regeneration rate
    U32 regen;

    // Sum of the metre positions of the centres of the clusters
    Point<F32> midSum;

    // Average metre position of the centres of the clusters
    Point<F32> midAvg;

    // Constructor
    ClusterGroup(U32 id)
    : id(id),
      resource(0),
      regen(0),
      midSum(0.0f, 0.0f)
    {
    }

    // Destructor
    ~ClusterGroup()
    {
      // Unlink all clusters
      clusters.UnlinkAll();

      // Clear resources
      resources.Clear();
    }

    // Merge the cluster group into ourselves
    void Merge(const ClusterGroup &clusterGroup);

    // Add a cluster to this resource
    void AddCluster(MapCluster &cluster);

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Resource::Decomposition
  //
  class Resource::Decomposition
  {
  private:

    // Initialized flag
    static U32 initialized;

    // The resource id counter
    static U32 resourceId;

    // Resource cluster groups
    static NList<ClusterGroup> clusterGroups;

  private:

    // Find a resource given a cluster
    static ClusterGroup * FindClusterGroup(MapCluster *cluster);

  public:

    // Initialization and Shutdown
    static void Init();
    static void Done();

    // Reset
    static void Reset();

  public:

    // Get cluster groups
    static const NList<ClusterGroup> & GetClusterGroups()
    {
      return (clusterGroups);
    }

  };

}

#endif
