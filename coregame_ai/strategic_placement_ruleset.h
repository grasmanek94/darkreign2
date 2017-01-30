/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Placement
//


#ifndef __STRATEGIC_PLACEMENT_RULESET_H
#define __STRATEGIC_PLACEMENT_RULESET_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_placement.h"
#include "strategic_placement_clusterset.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Struct Placement::TypeClusterSet
  //
  struct Placement::TypeClusterSet
  {
    NBinTree<ClusterSet> clusterSets;
    NBinTree<TypeClusterSet>::Node nodeRuleSet;

    TypeClusterSet()
    : clusterSets(&ClusterSet::nodeRuleSet)
    {
    }

    ~TypeClusterSet()
    {
      clusterSets.DisposeAll();
    }
  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Placement::RuleSet
  //
  class Placement::RuleSet
  {
  public:

    class Rule;


    /////////////////////////////////////////////////////////////////////////////
    //
    // Struct Info
    //
    struct Info
    {
      // The strategic object
      Object &object;

      // The origin
      const Point<F32> origin;

      // The world ctrl cluster
      MapCluster &mapCluster;

      // The cluster
      const Cluster &cluster;

      // The orienatation
      F32 orientation;

      // Constructor
      Info(Object &object, const Point<F32> &origin, MapCluster &mapCluster, const Cluster &cluster, F32 orientation)
      : object(object),
        origin(origin),
        mapCluster(mapCluster),
        cluster(cluster),
        orientation(orientation)
      {
      }

    };

  private:

    // Rules in this ruleset
    NList<Rule> rules;

    // Cluster sets by type for resources (by resource id)
    NBinTree<TypeClusterSet> clusterSetsResource;

    // Cluster sets by type for water (by water id)
    NBinTree<TypeClusterSet> clusterSetsWater;

    // Cluster sets by type name
    NBinTree<ClusterSet> clusterSetsBase;

  public:

    // Constructor
    RuleSet(FScope *fScope);

    // Destructor
    ~RuleSet();

    // Adjust initial position
    void AdjustOrigin(Point<F32> &origin, F32 orientation);

    // Evaluate a cluster
    Bool Evaluate(const Info &info, F32 &score);

    // Create a rule using the given fScope
    Rule * CreateRule(FScope *fScope);

  public:

    friend Placement;

  };

}

#endif
