/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Placement
//


#ifndef __STRATEGIC_PLACEMENT_CLUSTERSET_H
#define __STRATEGIC_PLACEMENT_CLUSTERSET_H


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
  // Class Placement::ClusterSet
  //
  class Placement::ClusterSet
  {
  public:

    enum FindResult
    {
      FOUND,
      PENDING,
      NOTFOUND
    };

  private:

    // Placement Node
    NBinTree<ClusterSet>::Node nodeRuleSet;

    // Strategic object in charge of the operation
    Object &object;

    // The managerial clusterset
    Manager::ClusterSet &managerClusterSet;

    // Origin
    Point<F32> origin;

    // Orientation
    F32 orientation;

    // Ruleset
    RuleSet &ruleSet;

    // Unit type
    UnitObjType &type;

    // The current cluster
    Cluster *current;

    // Index of the current cluster
    U32 currentIndex;

    // Index of the current cell within the current cluster
    U32 currentCell;

    // The set of next clusters
    NBinTree<Cluster, U32> nextByPosition;
    NBinTree<Cluster, F32> nextByScore;

    // The set of bad clusters
    NBinTree<Cluster, U32> badByPosition;

  private:

    // Constructor
    ClusterSet(const Point<F32> &origin, RuleSet &ruleSet, UnitObjType &type, F32 orientation, Placement &placement);

  public:

    // Destructor
    ~ClusterSet();

    // Find a cell for this type
    FindResult FindCell(Point<U32> &cell, WorldCtrl::CompassDir &dir);

    // Attempt to place at the given cell
    Bool PlaceAtCell(const Point<U32> &cell, WorldCtrl::CompassDir &dir);

    // Add all of the adjacent clusters to the given cluster
    void AddAdjacent(U32 index);

    // Add a cluster
    void AddCluster(MapCluster &mapCluster);

  public:

    // Why not just use the constructor with new, cause the compiler is a broken piece of turd!
    // The constructor is private to prevent its usage since the compiler cannot successfully
    // calculate the size of the structure from some reason
    static ClusterSet * Create(const Point<F32> &origin, RuleSet &ruleSet, UnitObjType &type, F32 orientation, Placement &placement);

  public:

    friend Placement;
    friend RuleSet;
    friend TypeClusterSet;

  };

}

#endif
