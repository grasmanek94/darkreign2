/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Placement
//


#ifndef __STRATEGIC_PLACEMENT_CLUSTER_H
#define __STRATEGIC_PLACEMENT_CLUSTER_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_placement.h"
#include "strategic_placement_manager_clusterinfo.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Placement::Cluster
  //
  class Placement::Cluster
  {
  private:

    // Cluster information
    const Manager::ClusterInfo &info;

    // Position node
    NBinTree<Cluster, U32>::Node nodePosition;

    // Score node
    NBinTree<Cluster, F32>::Node nodeScore;

  public:

    // Constructor
    Cluster(const Manager::ClusterInfo &info);

    // Destructor
    ~Cluster();

  public:

    friend Placement::ClusterSet;

  };

}

#endif
