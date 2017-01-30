/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Asset Request
// 25-MAR-1999
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_asset_request.h"
#include "strategic_object.h"
#include "unitobjctrl.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Asset::Request::ForceMap
  //

  //
  // Constructor
  //
  Asset::Request::ForceMap::ForceMap(Script *script, U32 handle, const Config::RecruitForce *config, const Point<F32> &location, Bool acceptInsufficient, Object *object)
  : Asset::Request(script, handle),
    config(config),
    location(location),
    acceptInsufficient(acceptInsufficient)
  {
    // Get the strategic ai's team
    Team *team = object->GetTeam();

    // Setup the allocation threats and defenses
    allocThreat = new F32[ArmourClass::NumClasses()];
    allocDefense = new F32[ArmourClass::NumClasses()];
    Reset();

    // Clear the threats and defenses
    threat = new F32[ArmourClass::NumClasses()];
    defense = new F32[ArmourClass::NumClasses()];
    Utils::Memset(threat, 0x00, sizeof (F32) * ArmourClass::NumClasses());
    Utils::Memset(defense, 0x00, sizeof (F32) * ArmourClass::NumClasses());
    threatTotal = 0.0f;
    defenseTotal = 0.0f;

    // Iterate the armour classes
    for (U32 ac = 0; ac < ArmourClass::NumClasses(); ac++)
    {
      // Iterate all of the enemy teams
      for (List<Team>::Iterator t(&team->RelatedTeams(Relation::ENEMY)); *t; t++)
      {
        // Add the threats and defenses of our enemy
        threat[ac] += (F32) AI::Map::GetThreat((*t)->GetId(), ac);
        threatTotal += (F32) AI::Map::GetThreat((*t)->GetId(), ac);
        defense[ac] += (F32) AI::Map::GetDefense((*t)->GetId(), ac);
        defenseTotal += (F32) AI::Map::GetDefense((*t)->GetId(), ac);
      }
    }

    // Modify threats and defenses
    for (ac = 0; ac < ArmourClass::NumClasses(); ac++)
    {
      threat[ac] *= config->GetAllocationDefense();
      defense[ac] *= config->GetAllocationThreat();
    }
    threatTotal *= config->GetAllocationDefense();
    defenseTotal *= config->GetAllocationThreat();

//    LOG_AI(("RequestForceMap: %08Xh Threat %f Defense %f", this, threatTotal, defenseTotal))

    // Invert totals
    threatTotalInv = threatTotal ? 1 / threatTotal : 0.0f;
    defenseTotalInv = defenseTotal ? 1 / defenseTotal : 0.0f;

    ERR_FATAL(("DO SAVE GAME MATT!"));
  }


  //
  // Constructor
  //
  Asset::Request::ForceMap::ForceMap()
  {
  }


  //
  // Destructor
  //
  Asset::Request::ForceMap::~ForceMap()
  {
    delete allocThreat;
    delete allocDefense;

    delete threat;
    delete defense;
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Asset::Request::ForceMap::SaveState(FScope *scope)
  {
    // Add the name of this type as an argument
    scope->AddArgString(GetName());

    // Save parent data
    Request::SaveState(scope->AddFunction("Parent"));
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Asset::Request::ForceMap::LoadState(FScope *scope, void *context)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x411AC76D: // "Parent"
          Request::LoadState(sScope, context);
          break;
      }
    }
  }


  //
  // Reset
  //
  // Reset the request
  //
  void Asset::Request::ForceMap::Reset()
  {
    Request::Reset();

    Utils::Memset(allocThreat, 0x00, sizeof (F32) * ArmourClass::NumClasses());
    Utils::Memset(allocDefense, 0x00, sizeof (F32) * ArmourClass::NumClasses());
  }


  //
  // Evaluate
  //
  // Evaluate the given asset
  //
  F32 Asset::Request::ForceMap::Evaluate(Asset &asset)
  {
    // Grab the unit from the asset
    UnitObj *unit = asset.GetUnit();
    ASSERT(unit)


    //
    // Acid Tests
    //

    // Can the unit move ?
    if (!unit->CanEverMove())
    {
      return (0.0f);
    }


    //
    // Constitution
    //
    // How good is the armour agains the weapons
    //
    F32 constitution;

    // Is there any threat ?
    if (threatTotalInv)
    {
      // Does this object have any hitpoints ?
      if (unit->UnitType()->GetHitPoints())
      {
        // Develop the constitution of this object on that basis
        constitution = 
          (1.0f - (threat[unit->UnitType()->GetArmourClass()] * threatTotalInv)) * 
          unit->GetHitPoints() * unit->UnitType()->GetHitPointsInv();
      }
      else
      {
        constitution = 0.0f;
      }
      constitution = Max<F32>(0.0f, constitution);
    }
    else
    {
      constitution = 1.0f;
    }


    //
    // Effectiveness
    //
    // How good are the weapons against the defenses
    //
    F32 effectiveness;

    // Are there any defenses ?
    if (defenseTotalInv)
    {
      effectiveness = 0;
      // How effective are this units weapons against the defense ?
      for (U32 ac = 0; ac < ArmourClass::NumClasses(); ac++)
      {
        // The effectiveness against an armour class is based on how much of
        // the defense has the given armour class and what percentage of the
        // whole teams threat does this unit impose against that kind of defense
        effectiveness = Max(
          effectiveness, 
          defense[ac] * defenseTotalInv * unit->UnitType()->GetThreat(ac) * 
          ArmourClass::GetMaximumThreatInv(ac));
      }

      // If the unit is non-effective, then we don't want it
      if (effectiveness == 0.0f)
      {
        return (0.0f);
      }

    }
    else
    {
      effectiveness = 1.0f;
    }


    //
    // Time
    //
    // The less time to the point the better
    //
    F32 time;

    // How far is this unit from the location given ?
    // For the time being do straight line calculations 
    // (even though this may be completely incorrect)
    // The cost for distance goes up in a squared fashion 
    // and is made into a fraction of the map
    time = 
      (unit->WorldMatrix().posit.x - location.x) * (unit->WorldMatrix().posit.x - location.x) +
      (unit->WorldMatrix().posit.z - location.y) * (unit->WorldMatrix().posit.z - location.y);
    time *= WorldCtrl::MetreMapDiag2Inv();
    time = 1.0f - time;


    //
    // Cost
    //
    // The less expensive, the better
    //
    F32 cost = 1.0f - F32(unit->UnitType()->GetResourceCost()) * UnitObjCtrl::GetMaximumCostInv();


    //
    // Tweak
    //
    // Configuration based tweaks
    //
    F32 tweak = config->GetEvaluationTweak(unit->UnitType());


    //
    // Combine all values
    //
    F32 val = 
      constitution * config->GetEvaluationConstitution() +
      effectiveness * config->GetEvaluationEffectiveness() +
      time * config->GetEvaluationTime() +
      cost * config->GetEvaluationCost() +
      tweak * config->GetEvaluationTweak();

    return (val);
  }


  //
  // Accept
  //
  // The given asset has been accepted
  //
  void Asset::Request::ForceMap::Accept(Asset &asset, Bool primary)
  {
    if (primary)
    {
      // Add the object to the list which are assigned to us
      assigned.Append(&asset);
    }

    // Get the unit out of the asset
    UnitObj *unit = asset.GetUnit();
    ASSERT(unit)
    
    // Add its threats and defenses to the totals
    allocDefense[unit->UnitType()->GetArmourClass()] += unit->GetHitPoints();
    for (U32 ac = 0; ac < ArmourClass::NumClasses(); ac++)
    {
      allocThreat[ac] += unit->UnitType()->GetThreat(ac);
    }
  }


  //
  // Offer
  //
  // The given asset is being offered, do we want it ?
  //
  Bool Asset::Request::ForceMap::Offer(Asset &asset)
  {
    // Get the unit out of the asset
    UnitObj *unit = asset.GetUnit();
    ASSERT(unit)

    U32 ac = unit->UnitType()->GetArmourClass();

    // Do we have enough of this kind of defense ?
    if (allocDefense[ac] && allocDefense[ac] < threat[ac])
    {
//      LOG_AI(("Need %d for Defense AC '%s' [%f:%f]", unit->Id(), ArmourClass::Id2ArmourClassName(ac), allocDefense[ac], threat[ac]))
      return (TRUE);
    }
    else
    {
      // Do we have enough of this kind of threat ?
      for (ac = 0; ac < ArmourClass::NumClasses(); ac++)
      {
        if (unit->UnitType()->GetThreat(ac) && allocThreat[ac] < defense[ac])
        {
//          LOG_AI(("Need %d for Threat AC '%s' [%f:%f]", unit->Id(), ArmourClass::Id2ArmourClassName(ac), allocThreat[ac], defense[ac]))
          return (TRUE);
          break;
        }
      }
    }

    return (FALSE);
  }


  //
  // OutOfAssets
  //
  // The asset manager is informing us there's no more assets
  //
  Bool Asset::Request::ForceMap::OutOfAssets(Manager &manager)
  {
    // Check to see if we have enough assets to continue
    U32 ac;
    Bool enough = TRUE;

    for (ac = 0; ac < ArmourClass::NumClasses(); ac++)
    {
      if (allocDefense[ac] && allocDefense[ac] < threat[ac])
      {
//        LOG_AI(("Insufficient defense for AC '%s'", ArmourClass::Id2ArmourClassName(ac)))
        enough = FALSE;
        break;
      }

      if (allocThreat[ac] < defense[ac])
      {
//        LOG_AI(("Insufficient threat to AC '%s'", ArmourClass::Id2ArmourClassName(ac)))
        enough = FALSE;
        break;
      }
    }

    if (enough || acceptInsufficient)
    {
      Completed(manager);
      return (TRUE);
    }
    else
    {
      Reset();
      Abandoned(manager);
      return (FALSE);
    }
    
  }
}
