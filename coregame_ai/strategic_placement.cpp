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
#include "strategic_placement.h"
#include "strategic_placement_manager.h"
#include "strategic_placement_cluster.h"
#include "strategic_placement_clusterset.h"
#include "strategic_placement_locator.h"
#include "strategic_placement_ruleset.h"
#include "strategic_object.h"

#include "unitobjctrl.h"
#include "spyobj.h"
#include "orders_game.h"
#include "sync.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Placement
  //


  //
  // Constructor
  //
  Placement::Placement(Manager &manager, const GameIdent &name, FScope *fScope)
  : manager(manager),
    name(name),
    locatorsIdle(&Locator::nodePlacementTree),
    locatorsActive(&Locator::nodePlacementTree),
    locatorsActiveList(&Locator::nodePlacementList),
    locatorsActiveIterator(&locatorsActiveList),
    ruleSet(*new RuleSet(fScope))
  {
    fallback = StdLoad::TypeString(fScope, "Fallback", "");
  }


  //
  // Destructor
  //
  Placement::~Placement()
  {
    // Cleanup
    locatorsIdle.DisposeAll();
    locatorsActiveList.UnlinkAll();
    locatorsActive.DisposeAll();

    delete &ruleSet;
  }


  //
  // Find a location for a token
  //
  void Placement::FindLocation(Base::Token &token)
  {
    if (locatorsActive.Exists(U32(&token)))
    {
      // There's already an active locator for this token!
      ERR_FATAL(("Token [%08X] already has an Active locator", U32(&token)))
    }

    if (locatorsIdle.Exists(U32(&token)))
    {
      // There's already a locator for this token ?
      LOG_AI(("There's already an idle locator for token [%08X]", U32(&token)))
      FindNextLocation(token);
      return;
    }

    // Does this token have an associated resource ?
    Resource *resource = token.GetResource();

    // Does this token have an associated water ?
    Water *water = token.GetWater();

    // Ask the ruleset if there's a pre-existing cluster set for this type
    ClusterSet *clusterSet = NULL;

    if (resource)
    {
      TypeClusterSet *typeClusterSet = ruleSet.clusterSetsResource.Find(resource->GetId());
      if (!typeClusterSet)
      {
        ruleSet.clusterSetsResource.Add(resource->GetId(), typeClusterSet = new TypeClusterSet);
      }
      clusterSet = typeClusterSet->clusterSets.Find(token.GetType().GetNameCrc());

      if (!clusterSet)
      {
        Point<F32> offset(WorldCtrl::MetreMapX() * 0.5f, WorldCtrl::MetreMapZ() * 0.5f);
        offset -= resource->GetCentre();
        offset.Normalize();

        clusterSet = ClusterSet::Create(
          resource->GetCentre(), 
          ruleSet, 
          token.GetType(), 
          F32(atan2(offset.z, offset.x)), 
          *this);

        typeClusterSet->clusterSets.Add(token.GetType().GetNameCrc(), clusterSet);
      }
    }
    else if (water)
    {
      TypeClusterSet *typeClusterSet = ruleSet.clusterSetsWater.Find(water->GetId());
      if (!typeClusterSet)
      {
        ruleSet.clusterSetsWater.Add(water->GetId(), typeClusterSet = new TypeClusterSet);
      }
      clusterSet = typeClusterSet->clusterSets.Find(token.GetType().GetNameCrc());

      if (!clusterSet)
      {
        Point<F32> offset(WorldCtrl::MetreMapX() * 0.5f, WorldCtrl::MetreMapZ() * 0.5f);
        offset -= water->GetClosest();
        offset.Normalize();

        clusterSet = ClusterSet::Create(
          water->GetClosest(), 
          ruleSet, 
          token.GetType(), 
          F32(atan2(offset.z, offset.x)), 
          *this);

        typeClusterSet->clusterSets.Add(token.GetType().GetNameCrc(), clusterSet);
      }
    }
    else
    {
      // Find/Create the type clusters for this type
      clusterSet = ruleSet.clusterSetsBase.Find(token.GetType().GetNameCrc());

      if (!clusterSet)
      {
        clusterSet = ClusterSet::Create(
          token.GetBase().GetLocation(), 
          ruleSet, 
          token.GetType(), 
          token.GetBase().GetOrientation(), 
          *this);

        ruleSet.clusterSetsBase.Add(token.GetType().GetNameCrc(), clusterSet);
      }
    }

    ASSERT(clusterSet)

    // We'll need to create a new locator
    Locator *locator = new Locator(token, *clusterSet);

    // Make it active
    locatorsActive.Add(U32(&token), locator);
    locatorsActiveList.Append(locator);
  }


  //
  // Find the next location for a token
  //
  void Placement::FindNextLocation(Base::Token &token)
  {
    Locator *locator = locatorsIdle.Find(U32(&token));
    
    if (!locator)
    {
      LOG_AI(("No idle locator could be found for the given token [%08X]", U32(&token)))
      FindLocation(token);
      return;
    }

    LOG_AI(("Finding next location for token [%08X]", U32(&token)))

    // Remove the locator from the idle tree and add it to the active tree
    locatorsIdle.Unlink(locator);
    locatorsActive.Add(U32(&token), locator);
    locatorsActiveList.Append(locator);
  }


  //
  // Abort finding a location for a token
  //
  void Placement::AbortFind(Base::Token &token)
  {
    LOG_AI(("Placement of Token [%08X] is being aborted", U32(&token)))

    Locator *locator = locatorsActive.Find(U32(&token));

    if (locator)
    {
      locatorsActive.Unlink(locator);
      locatorsActiveList.Unlink(locator);
      locatorsIdle.Add(U32(&token), locator);
    }
    else
    {
      if (!locatorsIdle.Exists(U32(&token)))
      {
        ERR_FATAL(("Token [%08X] is not idle or active", U32(&token)))
      }
    }
  }


  //
  // Remove the token from the system
  //
  void Placement::RemoveToken(Base::Token &token)
  {
    Locator *locator = locatorsIdle.Find(U32(&token));

    if (locator)
    {
      locatorsIdle.Dispose(locator);
    }
    else
    {
      locator = locatorsActive.Find(U32(&token));

      if (locator)
      {
        // If the iterator is pointing at this locator, advance the iterator
        if (locator == *locatorsActiveIterator)
        {
          ++locatorsActiveIterator;
        }

        locatorsActiveList.Unlink(locator);
        locatorsActive.Dispose(locator);
      }
      else
      {
        LOG_AI(("No Idle or Active locator for token [%08X]", U32(&token)))
      }
    }
  }


  //
  // Process this placement
  //
  void Placement::Process()
  {
    // Check to see if the iterator requires reset
    if (!*locatorsActiveIterator)
    {
      !locatorsActiveIterator;
    }

    if (Locator *locator = locatorsActiveIterator++)
    {
      Point<U32> cell;
      WorldCtrl::CompassDir dir = WorldCtrl::GetCompassDirection(locator->token.GetBase().GetOrientation());

      // Ask the type locator to check the current cluster for buildability
      switch (locator->clusterSet.FindCell(cell, dir))
      {
        case ClusterSet::FOUND:
        {
          // Tell the token that there's somewhere to build
          locator->token.SetLocation(
            Vector(
              WorldCtrl::CellToMetresX(cell.x), 
              0, 
              WorldCtrl::CellToMetresZ(cell.z)),
            dir);

          // This locator is now idle
          U32 key = locator->nodePlacementTree.GetKey();

          locatorsActive.Unlink(locator);
          locatorsActiveList.Unlink(locator);
          locatorsIdle.Add(key, locator);
          break;
        }

        case ClusterSet::NOTFOUND:
        {
          // This locator is now idle
          U32 key = locator->nodePlacementTree.GetKey();

          locatorsActive.Unlink(locator);
          locatorsActiveList.Unlink(locator);
          locatorsIdle.Add(key, locator);
          locator->token.NoLocation();
          break;
        }

        case ClusterSet::PENDING:
          break;
      }

    }
  }


  //
  // Get the fallback placement
  //
  Placement * Placement::GetFallback()
  {
    if (fallback.Null())
    {
      return (NULL);
    }
    else
    {
      LOG_AI(("Getting fallback '%s'", fallback.str))
      return (&GetManager().GetPlacement(fallback));
    }
  }

}

