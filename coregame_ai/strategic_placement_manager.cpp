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
#include "strategic_placement_manager.h"
#include "strategic_placement_manager_clusterinfo.h"
#include "strategic_config.h"
#include "resolver.h"


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


  //
  // Constructor
  //
  Placement::Manager::Manager()
  : clusterSets(&ClusterSet::nodeManager),
    placements(&Placement::nodeManager)
  {
  }


  //
  // Destructor
  //
  Placement::Manager::~Manager()
  {
    // Cleanup
    placements.DisposeAll();
    clusterSets.DisposeAll();
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Placement::Manager::SaveState(FScope *scope)
  {
    scope;
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Placement::Manager::LoadState(FScope *scope)
  {
    scope;
  }


  //
  // Process Placement
  //
  void Placement::Manager::Process()
  {
    // Process all of the placements
    for (NBinTree<Placement>::Iterator p(&placements); *p; ++p)
    {
      (*p)->Process();
    }
  }


  //
  // Tell the placement manager what the traction type of 
  // constructors are and what connected region they are one
  //
  void Placement::Manager::SetupConstructor(UnitObj &unitObj)
  {
    constructorTraction = unitObj.UnitType()->GetTractionIndex(unitObj.UnitType()->GetDefaultLayer());
    constructorRegion = ConnectedRegion::GetValue(constructorTraction, unitObj.cellX, unitObj.cellZ);
  }


  //
  // Get the cluster set for the given traction type
  //
  Placement::Manager::ClusterSet & Placement::Manager::GetClusterSet(U8 traction)
  {
    ClusterSet *clusterSet = clusterSets.Find(traction);
    if (!clusterSet)
    {
      clusterSet = new ClusterSet(traction, constructorTraction, constructorRegion);
      clusterSets.Add(traction, clusterSet);
    }
    return (*clusterSet);
  }


  //
  // Get a placement by name
  //
  Placement & Placement::Manager::GetPlacement(const GameIdent &placementName)
  {
    Placement *placement = placements.Find(placementName.crc);

    if (!placement)
    {
      // Ask config if there's a placement
      Config::Generic *config = Config::FindConfig(0x5D7C647F, placementName); // "Placement"

      if (config)
      {
        placement = new Placement(*this, placementName, config->GetFScope());
      }
      else
      {
        ERR_FATAL(("Could not find Placement config '%s'", placementName.str))
      }

      placements.Add(placementName.crc, placement);
    }

    return (*placement);
  }

}
