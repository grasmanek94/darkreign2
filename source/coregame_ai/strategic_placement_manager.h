/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Placement
//


#ifndef __STRATEGIC_PLACEMENT_MANAGER_H
#define __STRATEGIC_PLACEMENT_MANAGER_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_placement.h"
#include "connectedregion.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Placement::Manager
  //
  class Placement::Manager
  {
  public:

    class ClusterSet;
    class ClusterInfo;

  private:

    // Strategic Object the manager belongs to
    Object *strategic;

    // Cluster sets by traction type
    NBinTree<ClusterSet, U8> clusterSets;

    // Traction type of constructors
    U8 constructorTraction;

    // Connected region of constructors
    ConnectedRegion::Pixel constructorRegion;

    // Placements under the care of this manager indexed by name
    NBinTree<Placement> placements;

  public:

    // Constructor and destructor
    Manager();
    ~Manager();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope);

    // Process placement
    void Process();

    // Tell the placement manager what the traction type of 
    // constructors are and what connected region they are one
    void SetupConstructor(UnitObj &unit);

    // Get the cluster set for the given traction type
    ClusterSet & GetClusterSet(U8 traction);

    // Get a placement by name
    Placement & GetPlacement(const GameIdent &placementName);

  public:

    // Setup
    void Setup(Object *object)
    {
      strategic = object;
    }

    // Get the strategic object from the manager
    Object & GetObject()
    {
      ASSERT(strategic)
      return (*strategic);
    }

  };

}

#endif
