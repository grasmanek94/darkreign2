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
  // Class Asset::Request::Force
  //

  //
  // Constructor
  //
  Asset::Request::Force::Force(Script *script, U32 handle, const Config::RecruitForce *config, const Point<F32> &location, F32 range, Bool acceptInsufficient, Object *object)
  : Asset::Request(script, handle),
    config(config),
    location(location),
    range(range),
    acceptInsufficient(acceptInsufficient)
  {
    // Get the strategic ai's team
    Team *team = object->GetTeam();

    // Allocate and reset data buffers
    AllocateData();

    // Clear the threats and defenses
    threatTotal = 0.0f;
    defenseTotal = 0.0f;

    // Top Left & Bottom Right (metres)
    Point<F32> tl(location.x - range, location.y - range);
    Point<F32> br(location.x + range, location.y + range);

    WorldCtrl::ClampMetreMapPoint(tl);
    WorldCtrl::ClampMetreMapPoint(br);

    // Top Left & Bottom Right (clusters)
    Point<U32> ctl;
    WorldCtrl::MetresToClusterPoint(tl, ctl);
    Point<U32> cbr;
    WorldCtrl::MetresToClusterPoint(br, cbr);

    F32 range2 = (range + WorldCtrl::ClusterRadius()) * (range + WorldCtrl::ClusterRadius());

    for (U32 cy = ctl.y; cy <= cbr.y; cy++)
    {
      for (U32 cx = ctl.x; cx <= cbr.x; cx++)
      {
        F32 centrex = WorldCtrl::ClusterSize() * cx + WorldCtrl::ClusterSizeHalf();
        F32 centrey = WorldCtrl::ClusterSize() * cy + WorldCtrl::ClusterSizeHalf();

        // If the range from the centre of the cluster is less 
        // than the radius of the cluster + radius of the circle
        // then the cluster should be considered
        if ((((location.x - centrex) * (location.x - centrex)) +
             ((location.y - centrey) * (location.y - centrey))) < 
            range2)
        {
          MapCluster *cluster = WorldCtrl::GetCluster(cx, cy);
          AI::Map::Cluster *ai = &cluster->ai;

          // Iterate the armour classes
          for (U32 ac = 0; ac < ArmourClass::NumClasses(); ac++)
          {
            // Iterate all of the enemy teams
            for (List<Team>::Iterator t(&team->RelatedTeams(Relation::ENEMY)); *t; t++)
            {
              // Add the threats and defenses of our enemy
              threat[ac] += (F32) ai->GetThreat((*t)->GetId(), ac);
              threatTotal += (F32) ai->GetThreat((*t)->GetId(), ac);
              defense[ac] += (F32) ai->GetDefense((*t)->GetId(), ac);
              defenseTotal += (F32) ai->GetDefense((*t)->GetId(), ac);
            }
          }
        }
      }
    }

    // Calculate area of interest (in clusters)
    F32 area = range2 * PI * WorldCtrl::ClusterAreaInv();
    F32 areaInv = 1 / area;

    // Convert threats into threat densities
    for (U32 ac = 0; ac < ArmourClass::NumClasses(); ac++)
    {
      threat[ac] *= areaInv;
    }
    threatTotal *= areaInv;

    // Modify threats and defenses
    for (ac = 0; ac < ArmourClass::NumClasses(); ac++)
    {
      threat[ac] *= config->GetAllocationDefense();
      defense[ac] *= config->GetAllocationThreat();
    }
    threatTotal *= config->GetAllocationDefense();
    defenseTotal *= config->GetAllocationThreat();

//    LOG_AI(("Request::Force: %08Xh Threat %f Defense %f", this, threatTotal, defenseTotal))

    // Invert totals
    threatTotalInv = threatTotal ? 1 / threatTotal : 0.0f;
    defenseTotalInv = defenseTotal ? 1 / defenseTotal : 0.0f;

    ERR_FATAL(("TEST SAVE GAME MATT!"));
  }


  //
  // Constructor
  //
  Asset::Request::Force::Force()
  {
    // Allocate and reset data buffers
    AllocateData();
  }


  //
  // Destructor
  //
  Asset::Request::Force::~Force()
  {
    delete allocThreat;
    delete allocDefense;

    delete threat;
    delete defense;
  }


  //
  // AllocateData
  //
  // Allocate and reset data buffers
  //
  void Asset::Request::Force::AllocateData()
  {
    // Setup the allocation threats and defenses
    allocThreat = new F32[ArmourClass::NumClasses()];
    allocDefense = new F32[ArmourClass::NumClasses()];
    Reset();

    // Clear the threats and defenses
    threat = new F32[ArmourClass::NumClasses()];
    defense = new F32[ArmourClass::NumClasses()];
    Utils::Memset(threat, 0x00, sizeof (F32) * ArmourClass::NumClasses());
    Utils::Memset(defense, 0x00, sizeof (F32) * ArmourClass::NumClasses());
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Asset::Request::Force::SaveState(FScope *scope)
  {
    // Add the name of this type as an argument
    scope->AddArgString(GetName());

    StdSave::TypeString(scope, "Config", config->GetName().str);
    StdSave::TypePoint(scope, "Location", location);
    StdSave::TypeF32(scope, "Range", range);
    StdSave::TypeArray(scope, "Threat", threat, ArmourClass::NumClasses());
    StdSave::TypeF32(scope, "ThreatTotal", threatTotal);
    StdSave::TypeArray(scope, "Defense", defense, ArmourClass::NumClasses());
    StdSave::TypeF32(scope, "DefenseTotal", defenseTotal);
    StdSave::TypeArray(scope, "AllocThreat", allocThreat, ArmourClass::NumClasses());
    StdSave::TypeArray(scope, "AllocDefense", allocDefense, ArmourClass::NumClasses());
    StdSave::TypeU32(scope, "AcceptInsufficient", acceptInsufficient);

    // Save parent data
    Request::SaveState(scope->AddFunction("Parent"));
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Asset::Request::Force::LoadState(FScope *scope, void *context)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x2FDBDEBB: // "Config"
        {
          // Load the name
          GameIdent name = StdLoad::TypeString(sScope);

          // Find the config
          if ((config = Config::FindRecruitForce(name.crc)) == NULL)
          {
            ERR_FATAL(("Unable to find RecruitForce config [%s]", name.str));
          }

          break;
        }

        case 0x693D5359: // "Location"
          StdLoad::TypePoint(sScope, location);
          break;

        case 0xE17D7C71: // "Range"
          range = StdLoad::TypeF32(sScope);
          break;

        case 0x98D9AF2E: // "Threat"
          StdLoad::TypeArray<F32>(sScope, threat, ArmourClass::NumClasses()); 
          break;

        case 0x440EC12E: // "ThreatTotal"
          threatTotal = StdLoad::TypeF32(sScope);
          threatTotalInv = threatTotal ? 1 / threatTotal : 0.0f;
          break;

        case 0x07B0615D: // "Defense"
          StdLoad::TypeArray<F32>(sScope, defense, ArmourClass::NumClasses()); 
          break;

        case 0xFD0B4255: // "DefenseTotal"
          defenseTotal = StdLoad::TypeF32(sScope);
          defenseTotalInv = defenseTotal ? 1 / defenseTotal : 0.0f;
          break;

        case 0x8B11D7F9: // "AllocThreat"
          StdLoad::TypeArray<F32>(sScope, allocThreat, ArmourClass::NumClasses()); 
          break;

        case 0x8E9A4BF4: // "AllocDefense"
          StdLoad::TypeArray<F32>(sScope, allocDefense, ArmourClass::NumClasses()); 
          break;

        case 0x6A5ADB35: // "AcceptInsufficient"
          acceptInsufficient = StdLoad::TypeU32(sScope);
          break;

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
  void Asset::Request::Force::Reset()
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
  F32 Asset::Request::Force::Evaluate(Asset &asset)
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
      effectiveness = 0.0f;
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
  void Asset::Request::Force::Accept(Asset &asset, Bool primary)
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
  Bool Asset::Request::Force::Offer(Asset &asset)
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
  Bool Asset::Request::Force::OutOfAssets(Manager &manager)
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
