/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Resource
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_resource_decomposition.h"
#include "strategic_object.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Resource::ClusterGroup
  //


  //
  // Merge the cluster group into ourselves
  //
  void Resource::ClusterGroup::Merge(const ClusterGroup &clusterGroup)
  {
    // Add all of the clusters in the given cluster group
    for (List<MapCluster>::Iterator c(&clusterGroup.clusters); *c; ++c)
    {
      AddCluster(**c);
    }
  }


  //
  // Add a cluster to this cluster group
  //
  void Resource::ClusterGroup::AddCluster(MapCluster &cluster)
  {
    // Add to the list of clusters we cover
    clusters.Append(&cluster);

    // Add all of the resources in this cluster to our own stockpile
    for (NList<ResourceObj>::Iterator r(&cluster.resourceList); *r; ++r)
    {
      // Add the resource
      resources.Append(*r);

      // Update the total resource and regeneration
      resource += (*r)->GetResource();
      regen += (*r)->ResourceType()->GetResourceRate();
    }
 
    // Update the sum and average mid points
    midSum.x += (cluster.x0 + cluster.x1) * 0.5f;
    midSum.z += (cluster.z0 + cluster.z1) * 0.5f;
    midAvg = midSum / F32(clusters.GetCount());
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Resource::Decomposition
  //


  // Initialized flag
  U32 Resource::Decomposition::initialized = FALSE;

  // The resource id counter
  U32 Resource::Decomposition::resourceId;

  // Resource cluster groups
  NList<Resource::ClusterGroup> Resource::Decomposition::clusterGroups(&ClusterGroup::node);


  //
  // Find a resource given a cluster
  //
  Resource::ClusterGroup * Resource::Decomposition::FindClusterGroup(MapCluster *cluster)
  {
    for (NList<ClusterGroup>::Iterator g(&clusterGroups); *g; ++g)
    {
      for (List<MapCluster>::Iterator c(&(*g)->clusters); *c; ++c)
      {
        if (*c == cluster)
        {
          return (*g);
        }
      }
    }
    return (NULL);
  }


  //
  // Initialization
  //
  void Resource::Decomposition::Init()
  {
    ASSERT(!initialized)
    initialized = TRUE;
  }


  //
  // Shutdown
  //
  void Resource::Decomposition::Done()
  {
    ASSERT(initialized)
    clusterGroups.DisposeAll();
    initialized = FALSE;
  }


  //
  // Reset
  //
  void Resource::Decomposition::Reset()
  {
    ASSERT(initialized)

    LOG_AI(("Starting Resource Decomposition"))

    for (U32 z = 0; z < WorldCtrl::ClusterMapZ(); ++z)
    {
      for (U32 x = 0; x < WorldCtrl::ClusterMapX(); ++x)
      {
        MapCluster *currentCluster = WorldCtrl::GetCluster(x, z);
        MapCluster *cluster[4] = { NULL, NULL, NULL, NULL };

        // Is there any resource in this cluster ?
        if (currentCluster->ai.GetResource())
        {
          //
          // Is there resource in the adjacent clusters which we've already evaluated ?
          //
          //  0 | 1 | 2
          // ---+---+---
          //  3 | o |
          // ---+---+---
          //    |   |
          //

          if (z > 0) 
          {
            // Check the cluster 0
            if (x > 0)
            {
              cluster[0] = WorldCtrl::GetCluster(x - 1, z - 1);
              if (!cluster[0]->ai.GetResource())
              {
                cluster[0] = NULL;
              }
            }

            // Check the cluster 1
            cluster[1] = WorldCtrl::GetCluster(x, z - 1);
            if (!cluster[1]->ai.GetResource())
            {
              cluster[1] = NULL;
            }

            // Check the cluster 2
            if (z < WorldCtrl::ClusterMapX())
            {
              cluster[2] = WorldCtrl::GetCluster(x, z - 1);
              if (!cluster[2]->ai.GetResource())
              {
                cluster[2] = NULL;
              }
            }
          }

          // Check the cluster 3
          if (x > 0)
          {
            cluster[3]= WorldCtrl::GetCluster(x - 1, z);
            if (!cluster[3]->ai.GetResource())
            {
              cluster[3] = NULL;
            }
          }

          // Find the resource in any clusters which matched
          ClusterGroup *g[4];
          g[0] = cluster[0] ? FindClusterGroup(cluster[0]) : NULL;
          g[1] = cluster[1] ? FindClusterGroup(cluster[1]) : NULL;
          g[2] = cluster[2] ? FindClusterGroup(cluster[2]) : NULL;
          g[3] = cluster[3] ? FindClusterGroup(cluster[3]) : NULL;

          // If there's a resource at 3 and there's a resource at 1 or 2, make sure they're the same
          if (g[1] && g[3] && g[1] != g[3])
          {
            // Merge 1 and 3
            g[1]->Merge(*g[3]);
            clusterGroups.Dispose(g[3]);
            g[3] = g[1];
          }
          else if (g[2] && g[3] && g[2] != g[3])
          {
            // Merge 2 and 3
            g[2]->Merge(*g[3]);
            clusterGroups.Dispose(g[3]);
            g[3] = g[2];
          }

          // If a resource was found
          ClusterGroup *clusterGroup = NULL;
          if (g[0])
          {
            clusterGroup = g[0];
          }
          if (g[1])
          {
            ASSERT(!clusterGroup || clusterGroup == g[1])
            clusterGroup = g[1];
          }
          if (g[2])
          {
            ASSERT(!clusterGroup || clusterGroup == g[2])
            clusterGroup = g[2];
          }
          if (g[3])
          {
            ASSERT(!clusterGroup || clusterGroup == g[3])
            clusterGroup = g[3];
          }

          // Was there an existing clusterGroup ?
          if (!clusterGroup)
          {
            // Time to create a new cluster group
            clusterGroup = new ClusterGroup(resourceId++);

            // Add to the list of cluster groups
            clusterGroups.Append(clusterGroup);
          }

          // Add the current cluster 
          clusterGroup->AddCluster(*currentCluster);
        }
      }
    }

    LOG_AI(("Finished Resource Decomposition"))
  }

}
