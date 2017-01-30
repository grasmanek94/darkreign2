///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-APR-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "meshent.h"
#include "unitobj.h"
#include "worldctrl.h"
#include "mapobjctrl.h"
#include "unitobjctrl.h"
#include "gameobjctrl.h"
#include "stdload.h"
#include "physicsctrl.h"
#include "promote.h"
#include "sight.h"
#include "perfstats.h"
#include "team.h"
#include "resolver.h"
#include "terrain.h"
#include "common.h"
#include "environment_light.h"
#include "squadobj.h"
#include "taskctrl.h"
#include "random.h"
#include "tasks_unitmove.h"
#include "fx.h"
#include "movement_pathfollow.h"
#include "render.h"
#include "tasks_unitattack.h"
#include "tasks_unitguard.h"
#include "tasks_unitconstruct.h"
#include "tasks_unitupgrade.h"
#include "tasks_unitrecycle.h"
#include "tasks_unitconstructor.h"
#include "tasks_restorestatic.h"
#include "tasks_unitgorestore.h"
#include "trailobj.h"
#include "unitobjiter.h"
#include "weapon.h"
#include "restoreobj.h"
#include "transportobj.h"
#include "savegame.h"
#include "sync.h"
#include "explosionobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Config scope name for type and world files
#define SCOPE_CONFIG   "UnitObj"

// How many seconds should elapse before a unit will seek shelter
static const F32 TIME_RESPONDTODANGER = 5.0;


///////////////////////////////////////////////////////////////////////////////
//
// Class UnitObjType
//
// Contains a certain level of the gameplay feature set, including weapons
// and path searching.
//


//
// UnitObjType::UnitObjType
//
// Constructor
//
UnitObjType::UnitObjType(const char *name, FScope *fScope) 
: MapObjType(name, fScope),
  seeingRange(InstanceModifierType::INTEGER),
  maxSpeed(InstanceModifierType::FPOINT),
  canEverMove(FALSE),
  canEverFire(FALSE),
  needsBoardManager(FALSE),
  antiClandestine(FALSE),
  weapon(NULL),
  weaponBleedMap(NULL),
  selfDestructBleedMap(NULL),
  disruptor(FALSE),
  disruptionBleedMap(NULL),
  fireNode(NULL),
  barrel(0.0f, 0.0f, 0.0f)
{
  ASSERT(fScope);

  FScope *sScope, *ssScope;

  // If detachable flag was not configured externally
  if (!detachableConfig)
  {
    // Set all units to be detachable
    isDetachable = TRUE;
  }

  // If the shadow flag was not configured externally
  if (!hasShadowConfig)
  {
    // Set all units to have a shadow
    hasShadow = TRUE;
  }

  // Get specific config scope
  fScope = fScope->GetFunction(SCOPE_CONFIG);

  // Get prereqs
  prereqs.Configure(fScope);

  // Get Seeing range, day time and night time modifiers
  Range<S32> SightRange(0, Sight::MaxRangeCells() - 1);
  seeingRange.LoadIntegerScaled(fScope->GetFunction("SeeingRange", FALSE), 100.0F, SightRange, WC_CELLSIZEF32INV, 0.0F, 1.1F);

  // Load day and night modifiers
  seeingRangeDay   = Utils::FtoLNearest(F32(seeingRange.GetInteger()) * StdLoad::TypeF32(fScope, "DayModifier", 1.0f));
  seeingRangeNight = Utils::FtoLNearest(F32(seeingRange.GetInteger()) * StdLoad::TypeF32(fScope, "NightModifier", 1.0f));

  if (!SightRange.Inc(seeingRangeDay) || !SightRange.Inc(seeingRangeNight))
  {
    fScope->ScopeError("Seeing range modifier out of range for [%s]", typeId.str);
  }

  // Update the seeing range in the IM type
  UpdateSeeingRange(1.0F);

  // If the seeing range differs from day to night then add this type to the types which require updating when the time changes
  if (seeingRangeDay != seeingRangeNight)
  {
    UnitObjCtrl::RegisterTimeSensitive(this);
  }

  // Seeing height
  if ((sScope = fScope->GetFunction("SeeingHeight", FALSE)) != NULL)
  {
    seeingHeight = StdLoad::TypeF32(sScope, Range<F32>::positive);
  }
  else
  {
    // Auto calculate in LoadResource
    seeingHeight = -1.0F;
  }

  // Load the disruption range
  disruptionRange = StdLoad::TypeF32(fScope, "DisruptionRange", seeingRange.GetInteger() * WC_CELLSIZEF32);
  disruptionRange2 = disruptionRange * disruptionRange;

  // Get the top speed, convert from km/h to m/s
  maxSpeed.LoadFPoint(fScope->GetFunction("TopSpeed", FALSE), 10.0F, Range<F32>::positive, PhysicsConst::KMH2MPS, 0.0F, 1.5F);
  maxSpeedInv = (fabs(maxSpeed.GetFPoint()) > 1e-4F) ? 1.0F / maxSpeed.GetFPoint() : 0.0F;

  // Compute linear acceleration so top speed is reached in 1/2 game cell
  // a = v^2 / (2 * s)
  linearAccel = (maxSpeed.GetFPoint() * maxSpeed.GetFPoint()) / (2.0F * WC_CELLSIZEF32 * 0.5F);
  linearAccel2inv = (fabs(linearAccel) > 1e-4F) ? 1.0F / (2.0F * linearAccel) : 0.0F;

  // Turning speed, convert from degrees/s rad/s
  turnSpeed = StdLoad::TypeF32(fScope, "TurnSpeed", 270.0F, Range<F32>(1.0F, F32_MAX)) * DEG2RAD;

  // Computer angular acceleration so that top speed is reached within 1 second
  // a = v / 1.0F
  turnAccel = turnSpeed / 0.5f;
  turnAccel2inv = (fabs(turnAccel) > 1e-4F) ? 1.0F / (2.0F * turnAccel) : 0.0F;

  // Altitude to at lower and upper levels
  if ((sScope = fScope->GetFunction("Altitude", FALSE)) != NULL)
  {
    altitude[Claim::LAYER_LOWER] = StdLoad::TypeF32(sScope, 0.0F);
    altitude[Claim::LAYER_UPPER] = StdLoad::TypeF32(sScope, altitude[Claim::LAYER_LOWER]);
  }
  else
  {
    altitude[Claim::LAYER_LOWER] = 0.0F;
    altitude[Claim::LAYER_UPPER] = 0.0F;
  }


  // Power
  power.Setup(fScope->GetFunction("Power", FALSE));

  // Efficiency
  efficiency.Setup(fScope->GetFunction("Efficiency", FALSE));

  // Get the weapon
  weaponName = StdLoad::TypeString(fScope, "Weapon", "");

  // Selectable flag
  isSelectable = StdLoad::TypeU32(fScope, "Selectable", TRUE, Range<U32>::flag);

  // IndirectFire flag
  canFireIndirect = StdLoad::TypeU32(fScope, "FireIndirect", FALSE, Range<U32>::flag);

  // Can this unit automatically find targets
  canAutoTarget = StdLoad::TypeU32(fScope, "CanAutoTarget", TRUE, Range<U32>::flag);

  // Load the constructor of this type
  StdLoad::TypeReaperObjType(fScope, "Constructor", constructorType);

  // Does the constructor get swallowed and returned
  constructorConsume = StdLoad::TypeU32(fScope, "ConstructorConsume", TRUE, Range<U32>::flag);

  // Construction time in seconds
  constructionTime = StdLoad::TypeF32(fScope, "ConstructionTime", 4.0f, Range<F32>::positive);

  // Initial hitpoints when being constructed
  constructionHitPoints = S32(GetHitPoints() * StdLoad::TypeF32(fScope, "ConstructionHitPoints", 0.1f, Range<F32>::percentage));

  // Load the companion of this type
  StdLoad::TypeReaperObjType(fScope, "Companion", companionType);

  // Default tactical behavior
  defaultBehavior = StdLoad::TypeString(fScope, "DefaultBehavior", "");

  // Load the upgrade information
  if ((sScope = fScope->GetFunction("Upgrades", FALSE)) != NULL)
  {
    while ((ssScope = sScope->NextFunction()) != NULL)
    {
      switch (ssScope->NameCrc())
      {
        case 0x9F1D54D0: // "Add"
        {
          UpgradeInfo *info = new UpgradeInfo;
          StdLoad::TypeReaperObjType(ssScope, info->type);
          info->point = StdLoad::TypeStringD(ssScope, "HP-UPGRADE");
          info->index = 0;
          upgrades.Append(info);
          break;
        }
      }
    }
  }

  // Resource this unit costs
  resourceCost = StdLoad::TypeU32(fScope, "ResourceCost", 0, Range<U32>(0, 1000000));
  UnitObjCtrl::RegisterCost(resourceCost);

  // Command points this unit uses
  commandCost = StdLoad::TypeU32(fScope, "CommandCost", 0, Range<U32>(0, 100));

  // Resources this unit can transport
  resourceTransport = StdLoad::TypeU32(fScope, "ResourceTransport", 0, Range<U32>(0, 1000000));
  resourceLoad = StdLoad::TypeU32(fScope, "ResourceLoad", 10, Range<U32>(0, 1000000));
  resourceUnload = StdLoad::TypeU32(fScope, "ResourceUnload", 10, Range<U32>(0, 1000000));

  // The number of seconds this unit takes to recycle
  recycleTime = StdLoad::TypeF32(fScope, "RecycleTime", GetConstructionTime() * 0.5f, Range<F32>::positive);

  // Percentage of resource cost refunded when recycled
  recyclePercentage = StdLoad::TypeF32(fScope, "RecyclePercentage", 0.5F, Range<F32>::percentage);

  // The number of seconds this unit takes to power down
  powerDownTime = StdLoad::TypeF32(fScope, "PowerDownTime", F32(constructionTime * 0.125), Range<F32>::positive);
  powerUpTime = StdLoad::TypeF32(fScope, "PowerUpTime", powerDownTime, Range<F32>::positive);

  // Is this a construction facility
  isFacility = (idleTask.crc == 0x4D283CF9); // "Tasks::UnitConstructor"

  // Can this object be infiltrated by a spy
  canBeInfiltrated = (
    HasProperty(0xAF90926C) ||  // "Provide::EnemyLOS"
    HasProperty(0x607C18C4) ||  // "Provide::EnemyResource"
    HasProperty(0xADD2D789))    // "Provide::EnemyPower"
    ? TRUE : FALSE;

  if (canBeInfiltrated)
  {
    UnitObjCtrl::RegisterInfiltratable(this);
  }

  isFiltered = FALSE;
}


//
// UnitObjType::~UnitObjType
//
// Destructor
//
UnitObjType::~UnitObjType()
{
  // Empty lists
  responseEvents.DisposeAll();
  upgrades.DisposeAll();

  constructionWares.Clear();

  if (weaponBleedMap)
  {
    delete weaponBleedMap;
  }
  if (selfDestructBleedMap)
  {
    delete selfDestructBleedMap;
  }
  if (disruptionBleedMap)
  {
    delete disruptionBleedMap;
  }
}


//
// UnitObjType::PostLoad
//
// Called after all types are loaded
//
void UnitObjType::PostLoad()
{
  // Call parent scope first
  MapObjType::PostLoad();

  // If a weapon name was specified resolve the weapon
  if (!weaponName.Null())
  {
    if ((weapon = Weapon::Manager::FindType(weaponName)) == NULL)
    {
      ERR_CONFIG(("Could not resolve weapon type '%s'", weaponName.str))
    }
    weaponBleedMap = AI::Map::CreateBleedMap(weapon->GetMaxRange(), movementModel->canEverMove ? TRUE : FALSE);
  }

  if (selfDestructExplosion.Alive())
  {
    selfDestructBleedMap = AI::Map::CreateBleedMap(selfDestructExplosion->GetAreaOuter(), movementModel->canEverMove ? TRUE : FALSE);
  }

  // Clean up the canEverFire flag
  canEverFire = weapon ? TRUE : FALSE;

  // Set up the canEverMove flag
  canEverMove = movementModel->canEverMove;

  // Register the threat this unit has to each armour class
  for (U32 ac = 0; ac < ArmourClass::NumClasses(); ac++)
  {
    ArmourClass::RegisterThreat(ac, GetThreat(ac));
  }

  // Resolve prereqs
  prereqs.PostLoad();

  // Check recursion
//  prereqs.CheckRecursion(*this);

  // Resolve reapers
  Resolver::Type(constructorType);
  Resolver::Type(companionType);

  if (constructorType.Alive())
  {
    // Add myself to my constructors wares list
    constructorType->constructionWares.AppendNoDup(this);
    
    // Add my constructor as a prereq
    prereqs.Add(constructorType);
  }

  // Resolve upgrades
  List<UpgradeInfo>::Iterator u(&upgrades);
  UpgradeInfo *info;
  U32 index = 0;
  
  while ((info = u++) != NULL)
  {
    Resolver::Type(info->type);

    if (info->type.Alive())
    {
      // Is my upgrade configured to upgrade something else
      if (info->type->upgradeFor.Alive())
      {
        ERR_CONFIG
        ((
          "Duplicate upgrade : [%s] is an upgrade for [%s] and [%s]",
          info->type->GetName(), info->type->upgradeFor->GetName(), GetName()
        ));
      }

      // Set the reverse pointer 'upgradeFor'
      info->type->upgradeFor = this;

      // Set the upgrade as non selectable
      info->type->isSelectable = FALSE;

      // Set the index
      info->index = index++;

      // Add this type as a prereq for the upgrade
      info->type->prereqs.Add(this);
    }
    else
    {
      // Dispose of this entry
      upgrades.Dispose(info);
    }
  }

  // Validate altitude change
  if (movementModel->canEverMove && (movementModel->layerChange != Movement::Model::LC_UNABLE))
  {
    if (altitude[Claim::LAYER_LOWER] > altitude[Claim::LAYER_UPPER])
    {
      ERR_FATAL
      ((
        "Lower Altitude (%f) > Upper Altitude (%f) for [%s]", 
        altitude[Claim::LAYER_LOWER], altitude[Claim::LAYER_UPPER], GetName()
      ))
    }
  }

  // Is this an anti-clandestine unit
  if (HasProperty(0x608E628D)) // "Ability::AntiClandestine"
  {
    // Set the flag
    antiClandestine = TRUE;

    // Register the seeing range
    UnitObjCtrl::RegisterAntiClandestineSeeingRange(WorldCtrl::CellSize() * GetMaxSeeingRange());
  }

  // Is this a disruptor unit
  if (HasProperty(0xAA8F3EA2)) // "Ability::Disruptor"
  {
    // Set the flag
    disruptor = TRUE;

    // Create a bleed map for it
    disruptionBleedMap = AI::Map::CreateBleedMap(disruptionRange);

    // Register the seeing range
    UnitObjCtrl::RegisterDisruptorSeeingRange(disruptionRange);
  }
}


//
// ResponseName
//
// Returns static buffer containing the file name of the given response event
//
const char * UnitObjType::ResponseName(const char *asset, const char *event, U32 variation)
{
  static char buf[512];

  // Generate name in accordance with the convention
  Utils::Sprintf(buf, 512, "%s_%s-%d.wav", asset, event, variation);

  return (buf);     
}


//
// LoadResponseEvent
//
// Load a single unit response event
//
Bool UnitObjType::LoadResponseEvent(const char *event)
{
  // Ignore duplicates
  if (!UnitObjType::GetResponse(Crc::CalcStr(event)))
  {
    // Find all variations
    for (U32 v = 0; FileSys::Exists(ResponseName(GetName(), event, v)); v++);

    if (v)
    {
      // Add to the event list
      responseEvents.Append(new ResponseEvent(event, v));
      return (TRUE);
    }
  }

  return (FALSE);
}


//
// UnitObjType::InitializeResources
//
Bool UnitObjType::InitializeResources()
{
  // Are we allowed to initialize resources ?
  if (MapObjType::InitializeResources())
  {
    // Initialize weapon resources
    if (weapon)
    {
      weapon->InitializeResources(*this);
    }

    // Initialize resources for anything we construct
    /*
    for (UnitObjTypeList::Iterator t(&GetConstructionWares()); *t; ++t)
    {
      if (UnitObjType *unitType = (*t)->GetPointer())
      {
        if (Team::AllowedTypeByAny(unitType))
        {
          unitType->InitializeResources();
        }
      }
    }
    */

    MeshRoot &root = *GetMeshRoot();

    // Check for any of the boarding points
    for (U32 op = Movement::OP_ENTER1; op < Movement::OP_EXIT2; op++)
    {
      Matrix tmp;

      if (root.FindOffsetLocal(Movement::offsetPointNames[op], tmp))
      {
        needsBoardManager = TRUE;
        break;
      }
    }

    // Validate driver/footprint combination
    if (movementModel->hasDriver && GetFootPrintType())
    {
      ERR_FATAL(("Unit Type [%s]: Can't have both driver and footprint", GetName()))
    }

    // Auto-calculate seeing height of object if not already specified
    if (seeingHeight == -1.0F)
    {
      seeingHeight = root.ObjectBounds().Height();
    }

    // Load response events
    LoadResponseEvent("Select");
    LoadResponseEvent("Move");
    LoadResponseEvent("Attack");
    LoadResponseEvent("Restore");
    LoadResponseEvent("Infiltrate");
    LoadResponseEvent("Attacked");
    LoadResponseEvent("Spotted");

    return (TRUE);
  }
  return (FALSE);
}



//
// UnitObjType::NewInstance
//
// Create a new map object instance using this type
//
GameObj* UnitObjType::NewInstance(U32 id)
{
  // Allocate new object instance
  return (new UnitObj(this, id));
}


//
// UnitObjType::GetThreat
//
// Get the treat this unit has to the given armour class
//
U32 UnitObjType::GetThreat(U32 ac)
{
  return (GetWeaponThreat(ac) + GetSelfDestructThreat(ac));
}


//
// UnitObjType::GetWeaponThreat
//
// Get the threat this unit has to the given armour class
//
U32 UnitObjType::GetWeaponThreat(U32 ac)
{
  return (weapon ? weapon->GetThreat(ac) : 0);
}


//
// UnitObjType::GetSelfDestructThreat
//
// Get the threat this unit has to the given armour class
//
U32 UnitObjType::GetSelfDestructThreat(U32 ac)
{
  return (selfDestructExplosion.Alive() ? selfDestructExplosion->GetThreat(ac) : 0);
}



//
// GetConstructorType
//
// Get the type that constructs this object, or NULL
//
UnitObjType * UnitObjType::GetConstructorType()
{
  return (constructorType.GetPointer());
}


//
// GetCompanionType
//
// Get the type that accompanies this object, or NULL
//
UnitObjType * UnitObjType::GetCompanionType()
{
  return (companionType.GetPointer());
}


//
// UpdateSeeingRange
//
// Update seing range
//
Bool UnitObjType::UpdateSeeingRange(F32 val)
{
  ASSERT(val >= 0.0f && val <= 1.0f)

  // Determine the new seeing range
  // trunc mode is the default set in Utils::FP::Reset
  //
  U32 valInt = Utils::FtoLNearest(val * 256.0f);
  U32 newSeeingRange = (seeingRangeDay * valInt + seeingRangeNight * (256 - valInt)) >> 8;

  // Did our seeing range change ?
  if (newSeeingRange != U32(seeingRange.GetInteger()))
  {
    // Modifier the IM type directly
    seeingRange.sint32.value = newSeeingRange;
    return (TRUE);
  }
  else
  {
    return (FALSE);
  }
}


//
// GetResponse
//
// Get the responses for this event, or NULL if none
//
UnitObjType::ResponseEvent * UnitObjType::GetResponse(U32 event)
{
  // Find the event
  for (List<ResponseEvent>::Iterator i(&responseEvents); *i; i++)
  {
    if ((*i)->name.crc == event)
    {
      return (*i);
    }
  }

  return (NULL);  
}


//
// RandomResponse
//
// Returns the name of a random response, or NULL if none available
//
const char * UnitObjType::RandomResponse(U32 event)
{
  ResponseEvent *r = GetResponse(event);

  return 
  (
    r ? ResponseName(GetName(), r->name.str, Random::nonSync.Integer(r->count)) : NULL
  );
}


//
// SpawnClosest
//
// Create a unit at the closest available location, or return NULL
//
UnitObj * UnitObjType::SpawnClosest(const Vector &pos, Team *team, Bool construct, F32 orientation)
{
  Vector closest;

  // Ensure resources are initialized
  InitializeResources();

  // Get the closest linked location
  if (FindClosestPos(pos, closest))
  {
    // Spawn the object
    return (SpawnLinked(closest, team, construct, orientation));
  }

  // Unable to find an available position
  return (NULL);
}


//
// SpawnLinked
//
// Create a unit at the closest available linked location, or return NULL
//
UnitObj * UnitObjType::SpawnLinked(const Vector &pos, Team *team, Bool construct, F32 orientation)
{
  Vector linked;

  // Ensure resources are initialized
  InitializeResources();

  // Get the linked location
  if (FindLinkedPos(pos, linked))
  {
    // Spawn the object
    return (&Spawn(linked, team, construct, orientation));
  }

  // Unable to find an available position
  return (NULL);
}


//
// Spawn
//
// Create a unit at the given location
//
UnitObj & UnitObjType::Spawn(const Vector &pos, Team *team, Bool construct, F32 orientation)
{
  // Generate a matrix
  Matrix m;
  m.ClearData();
  m.posit = pos;
  m.front.Set(orientation, PIBY2);
  m.SetFromFront(m.front);

  // Create the object
  return (Spawn(m, team, construct));
}


//
// Spawn
//
// Create a unit at the given location
//
UnitObj & UnitObjType::Spawn(const Matrix &mat, Team *team, Bool construct)
{
  // Create the object on the map
  UnitObj *obj = (UnitObj*)MapObjCtrl::ObjectNewOnMap(this, mat, 0, TRUE, !construct);

  ASSERT(obj)

  // Do we need to give it a construct task
  if (construct)
  {
    // Set the team but do not add to lists
    obj->SetTeam(team, FALSE);

    // Do the construction task setup
    Tasks::UnitConstruct::Setup(obj);

    // Assign the construct task
    obj->PrependTask(new Tasks::UnitConstruct(obj));
  }
  else
  {
    // Set the team and add to lists
    obj->SetTeam(team);
  }

  return (*obj);
}


//
// Spawn
//
// Create a unit inside the hold of a transport (team is automatically set by the transport)
//
UnitObj & UnitObjType::Spawn(TransportObj *transport)
{
  ASSERT(transport)

  // Create the unit off the map
  UnitObj *obj = (UnitObj *) MapObjCtrl::ObjectNew(this);

  // Set the team of the new object to be that of the transports team
  obj->SetTeam(transport->GetTeam());

  // Can this unit go into the supplied transport
  if (transport->CheckCargo(obj))
  {
    transport->AddCargo(obj);
  }
  else
  {
    LOG_WARN(("Whilst spawning '%s' the transport would not accept it!", GetName()))
  }

  return (*obj);
}


//
// Spawn
//
// Create a unit from the given facility
//
UnitObj & UnitObjType::Spawn(UnitObj *subject, Bool move)
{
  // Create the object
  UnitObj *u = (UnitObj*)MapObjCtrl::ObjectNew(this);

  ASSERT(u)

  // Set the team
  u->SetTeam(subject->GetTeam());

  // Stick it on the building
  if (u->CanEverMove())
  {
    u->GetDriver()->AddToMapBoard(subject);
  }
  else
  {
    ERR_CONFIG(("Attempt to build non-moveable unit type [%s] from [%s]", GetName(), subject->TypeName()))
  }

  // Move away
  if (move)
  {
    u->MoveAwayTask();
  }

  return (*u);
}


//
// FindClosestPos
//
// Get the closest position this type can move on
//
Bool UnitObjType::FindClosestPos(const Vector &src, Vector &dst, U32 range)
{
  // Get the cell location
  U32 x = WorldCtrl::MetresToCellX(src.x);
  U32 z = WorldCtrl::MetresToCellZ(src.z);

  // Try and find a closest cell
  if (PathSearch::FindClosestCell(GetTractionIndex(GetDefaultLayer()), x, z, x, z, range))
  { 
    // Convert to metre position
    F32 mx = WorldCtrl::CellToMetresX(x);
    F32 mz = WorldCtrl::CellToMetresZ(z);

    // Setup destination vector
    dst.Set(mx, TerrainData::FindFloorWithWater(mx, mz), mz);

    // Success
    return (TRUE);
  }

  // No position found within range
  return (FALSE);
}


//
// FindLinkedPos
//
// Get the closest linked location
//
Bool UnitObjType::FindLinkedPos(const Vector &src, Vector &dst, U32 range)
{
  ASSERT(WorldCtrl::MetreOnMap(src.x, src.z))

  // Get the starting cell
  U32 x = WorldCtrl::MetresToCellX(src.x);
  U32 z = WorldCtrl::MetresToCellZ(src.z);

  // The grain position
  S32 gx, gz;

  // Attempt to find an area we can claim
  if (PathSearch::FindGrain(x, z, gx, gz, GetTractionIndex(GetDefaultLayer()), GetGrainSize(), range))
  {
    // Get the metre position
    Movement::GrainToMetre(gx, gz, GetGrainSize(), dst.x, dst.z);

    // Set the height
    dst.y = TerrainData::FindFloorWithWater(dst.x, dst.z);

    // Success
    return (TRUE);
  }

  // No linked location
  return (FALSE);
}


//
// FindAntiClandestine
//
// Find an enemy anti-clandestine unit that can see the given location
//
UnitObj * UnitObjType::FindAntiClandestine(Team *team, const Vector &location)
{
  ASSERT(WorldCtrl::MetreOnMap(location.x, location.z))

  UnitObj *obj; 

  // Generate a unit iterator
  UnitObjIter::Tactical i
  (
    NULL, UnitObjIter::FilterData(team, Relation::ENEMY, location, UnitObjCtrl::GetAntiClandestineSeeingRange())
  );

  // Get the cell position
  U32 x = WorldCtrl::MetresToCellX(location.x);
  U32 z = WorldCtrl::MetresToCellZ(location.z);

  while ((obj = i.Next()) != NULL)
  {
    if (obj->UnitType()->GetAntiClandestine() && Sight::CanUnitSee(obj, x, z))
    {
      return (obj);
    }
  }   

  return (NULL);
}


//
// FindDisruptor
//
// Find an enemy disruptor unit that can see the given location
//
UnitObj * UnitObjType::FindDisruptor(Team *, const Vector &location)
{
  ASSERT(WorldCtrl::MetreOnMap(location.x, location.z))

  UnitObj *obj; 

  // Generate a unit iterator
  UnitObjIter::Tactical i
  (
    NULL, UnitObjIter::FilterData(location, UnitObjCtrl::GetDisruptorSeeingRange())
  );

  while ((obj = i.Next()) != NULL)
  {
    if (obj->UnitType()->GetDisruptor() && i.GetProximity2() < obj->UnitType()->GetDisruptionRange2())
    {
      return (obj);
    }
  }   

  return (NULL);
}



///////////////////////////////////////////////////////////////////////////////
//
// Class UnitObj - Instance class for above type
//

//
// UnitObj::UnitObj
//
// Constructor
//
UnitObj::UnitObj(UnitObjType *objType, U32 id) 
: MapObj(objType, id),
  flags(0),
  seeingRange(&objType->seeingRange),
  maxSpeed(&objType->maxSpeed),
  driver(NULL),
  boardManager(NULL),
  team(NULL),
  resourceTransport(0),
  teamsCanSee(0),
  teamsHaveSeen(0),
  weapon(NULL),
  spyingTeams(0),
  sightMap(NULL)
{
  ASSERT(objType->resourcesInitialized)

  // If the type defines a weapon the add a weapon of that type
  if (objType->weapon)
  {
    weapon = new Weapon::Object(*objType->weapon, *this);
  }

  // Create path follower object
  if (objType->movementModel->hasDriver)
  {
    driver = new Movement::Driver(this);
    altitude = objType->altitude[objType->movementModel->defaultLayer];
  }
  else
  {
    altitude = 0.0F;
  }

  // Create board manager
  if (objType->needsBoardManager)
  {
    boardManager = new Movement::BoardManager;
  }

  // Allocate upgrade reaper array
  upgrades = UnitType()->upgrades.GetCount() ? 
    new UnitObjPtr[UnitType()->upgrades.GetCount()] : NULL;

  // Setup default behavior
  if (objType->defaultBehavior.crc)
  {
    U8 mod, setting;

    if (Tactical::FindModifier(0xBC1CE62D, &mod)) // "Behavior"
    {
      if (!Tactical::FindSetting(mod, objType->defaultBehavior.crc, &setting))
      {
        ERR_FATAL(("Unknown Behavior setting [%s] for [%s]", objType->defaultBehavior.str, TypeName()));
      }

      SetTacticalModifierSetting(mod, setting);
    }
  }
}


//
// UnitObj::~UnitObj
//
// Destructor
//
UnitObj::~UnitObj()
{
  ASSERT(sightMap == NULL)

  if (weapon)
  {
    delete weapon;
  }

  if (driver)
  {
    delete driver;
  }

  if (boardManager)
  {
    delete boardManager;
  }

  if (upgrades)
  {
    delete [] upgrades;
  }
}


//
// UnitObj::PreDelete
//
// Pre deletion cleanup
//
void UnitObj::PreDelete()
{
  // Notify the player of our demise
  NotifyPlayer(0x11EAEF8E); // "Unit::Died"

  // Remove ourselves from our team
  SetTeam(NULL);

  // If we're in a squad, remove ourselves
  if (squad.Alive())
  {
    squad->RemoveUnitObj(this);
  }

  // Call parent scope last
  MapObj::PreDelete();
}


//
// UnitObj::SaveState
//
// Save a state configuration scope
//
void UnitObj::SaveState(FScope *fScope, MeshEnt * theMesh) // = NULL
{
  // Call parent scope first
  MapObj::SaveState(fScope, theMesh);

  // Create specific config scope
  fScope = fScope->AddFunction(SCOPE_CONFIG);

  // Save the team
  if (team)
  {
    StdSave::TypeString(fScope, "Team", team->GetName());
  }

  // Save the upgrade array
  if (upgrades)
  {
    FScope *sScope = fScope->AddFunction("Upgrades");

    for (U32 i = 0; i < UnitType()->upgrades.GetCount(); i++)
    {
      StdSave::TypeReaper(sScope, "ReaperId", upgrades[i]);
    }
  }

  // Save the linked unit
  StdSave::TypeReaper(fScope, "LinkedUnit", linkedUnit);

  // Save game data
  if (SaveGame::SaveActive())
  {
    // If not actively on team, save that fact
    if (!IsActivelyOnTeam())
    {
      fScope->AddFunction("NotActivelyOnTeam");
    }

    // Save state flags
    if (flags & ~FLAG_NONSAVING)
    {
      StdSave::TypeU32(fScope, "Flags", flags & ~FLAG_NONSAVING);
    }

    // Save the altitude
    if (CanEverMove())
    {
      StdSave::TypeF32(fScope, "Altitude", altitude);
    }

    // Save the weapon
    if (weapon)
    {
      weapon->SaveState(fScope->AddFunction("Weapon"));
    }

    // Save movement
    if (driver)
    {
      driver->SaveState(fScope->AddFunction("Driver"));
    }

    if (boardManager)
    {
      StdSave::TypeReaper(fScope, "BoardManager", *boardManager);
    }

    // Save Tactical Modifier Settings
    settings.SaveState(fScope->AddFunction("ModifierSettings"));

    // Save the resources
    if (resourceTransport)
    {
      StdSave::TypeU32(fScope, "ResourceTransport", resourceTransport);
    }

    // Save the squad
    StdSave::TypeReaper(fScope, "Squad", squad);

    // Save the primary restore facility
    StdSave::TypeReaper(fScope, "PrimaryRestore", primaryRestore);

    // Save the instance modifiers
    maxSpeed.SaveState(fScope->AddFunction("MaxSpeed"));
    seeingRange.SaveState(fScope->AddFunction("SeeingRange"));

    // Is blind targetting active
    if (!blindTarget.Test())
    {
      blindTarget.SaveState(fScope->AddFunction("BlindTarget"));
    }

    // Save team bitfields
    StdSave::TypeU32(fScope, "TeamsHaveSeen", teamsHaveSeen);

    // Spies inside this building
    if (UnitType()->CanBeInfiltrated())
    {
      StdSave::TypeU32(fScope, "SpyingTeams", spyingTeams);
    }
  }
}


//
// UnitObj::LoadState
//
// Load a state configuration scope
//
void UnitObj::LoadState(FScope *fScope)
{
  // Call parent scope first
  MapObj::LoadState(fScope);

  // Get specific config scope
  fScope = fScope->GetFunction(SCOPE_CONFIG);

  FScope *sScope;

  // Default to no team
  Team *newTeam = NULL;

  // Default to adding to the team list
  Bool onTeamList = TRUE;

  while ((sScope = fScope->NextFunction()) != NULL)
  {
    switch (sScope->NameCrc())
    {
      case 0xEDF0E1CF: // "Team"
        newTeam = Team::Name2Team(StdLoad::TypeString(sScope));
        break;

      case 0x7843B95E: // "NotActivelyOnTeam"
        onTeamList = FALSE;
        break;

      case 0x0A16EEBB: // "Upgrades"
      {
        for (U32 i = 0; i < UnitType()->upgrades.GetCount(); i++)
        {
          // Get an upgrade reaper
          FScope *uScope = sScope->NextFunction();

          if (uScope && uScope->NameCrc() == 0x2C77A1B7) // "ReaperId"
          {
            StdLoad::TypeReaper(uScope, upgrades[i]);
          }
        }
        break;
      }

      case 0xA0BEA058: // "LinkedUnit"
        StdLoad::TypeReaper(sScope, linkedUnit);
        break;

      case 0x46700A29: // "Flags"
        flags = U8(StdLoad::TypeU32(sScope));
        break;

      case 0x2C1AA2EC: // "Altitude"
        altitude = StdLoad::TypeF32(sScope);
        break;

      case 0x1DE7E3DA: // "Weapon"
        if (weapon)
        {
          weapon->LoadState(sScope);
        }
        break;

      case 0x5071F52C: // "Driver"
        if (driver)
        {
          driver->LoadState(sScope);
        }
        break;

      case 0xF698A86F: // "BoardManager"
        if (boardManager)
        {
          StdLoad::TypeReaper(sScope, *boardManager);
        }
        break;

      case 0x2F382D90: // "ModifierSettings"
        settings.LoadState(sScope);
        break;

      case 0x3A2ECFB8: // "ResourceTransport"
        resourceTransport = StdLoad::TypeU32(sScope);
        break;

      case 0xAF55CD8F: // "Squad"        
        StdLoad::TypeReaper(sScope, squad);
        break;

      case 0x243C96C1: // "PrimaryRestore"
        StdLoad::TypeReaper(sScope, primaryRestore);
        break;

      case 0xC5447F67: // "MaxSpeed"
        maxSpeed.LoadState(sScope);
        break;

      case 0x107B6FA2: // "SeeingRange"
        seeingRange.LoadState(sScope);
        break;

      case 0x4FD1A67D: // "BlindTarget"
        blindTarget.LoadState(sScope);
        break;

      case 0x7D1A92D4: // "TeamsHaveSeen"
        teamsHaveSeen = Game::TeamBitfield(StdLoad::TypeU32(sScope));
        break;

      case 0x5E9761F8: // "SpyingTeams"
        spyingTeams = Game::TeamBitfield(StdLoad::TypeU32(sScope));
        break;
    }
  }

  // Add to the team
  if (newTeam)
  {
    SetTeam(newTeam, onTeamList);
  }
}


//
// UnitObj::PostLoad
//
// Called after all objects are loaded
//
void UnitObj::PostLoad()
{
  // Call parent scope first
  MapObj::PostLoad();

  // Resolve squad
  Resolver::Object<SquadObj, SquadObjType>(squad);

  // Resolve the upgrades
  for (U32 i = 0; i < UnitType()->upgrades.GetCount(); i++)
  {
    Resolver::Object<UnitObj, UnitObjType>(upgrades[i]);
  }

  // Resolve the linked unit
  Resolver::Object<UnitObj, UnitObjType>(linkedUnit);

  // Resolve the primary restorer
  Resolver::Object<RestoreObj, RestoreObjType>(primaryRestore);

  // Post load the weapon
  if (weapon)
  {
    weapon->PostLoad();
  }

  // Post load the driver
  if (driver)
  {
    driver->PostLoad();
  }

  // Post load the board manager
  if (boardManager)
  {
    Resolver::Object<UnitObj, UnitObjType>(*boardManager);
  }
}


//
// UnitObj::SendEvent
//
// Send an Event
//
Bool UnitObj::SendEvent(const Task::Event &event, Bool idle)
{
  // If we have a weapon, offer the message to it
  if (weapon && weapon->ProcessEvent(event))
  {
    return (TRUE);
  }

  switch (event.message)
  {
    case 0x1B7D0A42: // "Unit::RestoreNow"
      RestoreNow();
      return (TRUE);
  }

  // If we don't want the event pass it down
  return (MapObj::SendEvent(event, idle));
}


//
// UnitObj::CaptureCellHooks
//
// Capture/Release cell hooks when part of the bounding sphere of an object moves cells
//
void UnitObj::CaptureCellHooks(Bool capture)
{
  MapObj::CaptureCellHooks(capture);

  // Flag LOS as needing updating
  SetFlag(FLAG_UPDATELOS, TRUE);

  if (capture)
  {
    // Find out which bits changed in this capture
    Game::TeamBitfield newBF = Sight::VisibleMask(cellX, cellZ);
    Game::TeamBitfield changedBF = (Game::TeamBitfield) (newBF ^ teamsCanSee);

    if (changedBF)
    {
      for (U32 t = 0; t < Game::MAX_TEAMS; t++)
      {
        if (Team::Id2Team(t))
        {
          if (Game::TeamTest(changedBF, t))
          {
            if (Game::TeamTest(newBF, t))
            {
              SetCanSee(t);
            }
            else
            {
              ClearCanSee(t);
            }
          }
        }
      }
    }
  }
}


//
// UnitObj::ProcessCycle
//
// Per-cycle processing
//
void UnitObj::ProcessCycle()
{
  MapObj::ProcessCycle();

  // Simulate instance modifiers
  maxSpeed.Simulate();

  // Process movement
  if (driver)
  {
    PERF_S(("PathFollowing"))
    driver->Process();
    PERF_E(("PathFollowing"))
  }

  // Update line of sight instance modifier and test if a rescan is required
  S32 prevRange = seeingRange.GetInteger();
  seeingRange.Simulate();

  if (prevRange != seeingRange.GetInteger())
  {
    SetFlag(FLAG_UPDATELOS, TRUE);
  }

  // Check for a sufficient change in the last LOS scan altitude
  ASSERT(OnMap())
  ASSERT(sightMap)

  if (fabs(Sight::EyePosition(this) - sightMap->lastAlt) > 4.0F)
  {
    SetFlag(FLAG_UPDATELOS, TRUE);
  }

  // If we have a weapon then process it 
  if (weapon)
  {
    PERF_S(("Weapon"))
    weapon->Process();
    PERF_E(("Weapon"))
  }

  if (GetFlag(FLAG_UPDATELOS) && OnMap())
  {
    PERF_S(("Line of sight"))
    Sight::UnSweep(this);
    Sight::Sweep(this);
    SetFlag(FLAG_UPDATELOS, FALSE);
    PERF_E(("Line of sight"))
  }

  if (GetFlag(FLAG_SELFDESTRUCT))
  {
    SelfDestruct(TRUE, GetTeam());
  }
}


//
// UnitObj::Move
//
// Tell unit to move to a location/direction
//
void UnitObj::Move(Movement::Handle &handle, const Vector *dst, const Vector *dir, Bool passUnit, Movement::RequestData *req)
{
  U32 notify = 0;

  if (driver)
  {
    if (!driver->Setup(handle, dst, dir, passUnit, req))
    {
      // Can't move for some reason
      notify = Movement::Notify::Incapable;
    }
  }
  else
  {
    // Doesn't have a path follower
    notify = Movement::Notify::Incapable;
  }

  // Notify the task if necessary
  if (notify)
  {
    PostEvent(Task::Event(notify, handle));
  }
}


//
// Move
//
// Tell unit to move onto the given trail
//
void UnitObj::Move(Movement::Handle &handle, TrailObj *trail, Bool, Movement::RequestData *)
{
  ASSERT(trail)

  // Reset handle before requesting
  handle.Invalidate();

  //LOG_DIAG(("Unit [%s/%d] moving to trail [%s/%d]", TypeName(), Id(), trail->GetName(), trail->Id()));
}


//
// UnitObj::MoveOnto
//
// Tell unit to move into an object with an ENTRY hardpoint
//
void UnitObj::MoveOnto(Movement::Handle &handle, UnitObj *obj, Bool passUnit, Movement::RequestData *req)
{
  U32 notify = 0;

  if (driver)
  {
    if (!driver->Setup(handle, obj, passUnit, req))
    {
      // Can't enter the building for some reason
      notify = Movement::Notify::Incapable;
    }
  }
  else
  {
    // Doesn't have a path follower
    notify = Movement::Notify::Incapable;
  }

  // Notify the task if necessary
  if (notify)
  {
    PostEvent(Task::Event(notify, handle));
  }
}


//
// GetMoveAwayPosition
//
// Get a random position that will cause a unit to move away from its current location
//
void UnitObj::GetMoveAwayPosition(Vector &dest, F32 distance)
{
  VectorDir dir;

  // Start with the direction the unit is facing
  WorldMatrix().front.Convert(dir);

  // Offset randomly
  dir.u += (Random::sync.Float() * PIBY2) - (PIBY2 / 2);
  dir.v = PIBY2;
  dir.FixU();

  // Generate raw destination
  dest = Vector(Position() + (Vector(dir) * distance)); 

  // Clamp to the play field
  WorldCtrl::ClampPlayField(dest.x, dest.z);
}


//
// MoveAwayDirect
//
// Make this unit move away
//
void UnitObj::MoveAwayDirect(Movement::Handle &handle, F32 distance)
{
  Vector dest;

  // Get a point to try and move to
  GetMoveAwayPosition(dest, distance);

  // Tell the unit to move
  Movement::RequestData rd;
  rd.giveUpGrains = Movement::MAX_GIVEUP_MOVEAWAY_DIST;

  Move(handle, &dest, NULL, FALSE, &rd);
}


//
// MoveAwayTask
//
// Make this unit move away, using the move task
//
Bool UnitObj::MoveAwayTask(F32 distance)
{
  // Can this unit ever move
  if (CanEverMove())
  {
    // Is the move task blocked
    if (!Blocked(Tasks::UnitMove::GetConfigBlockingPriority()))
    {
      Vector dest;

      // Get a point to try and move to
      GetMoveAwayPosition(dest, distance);

      // Setup the task
      Tasks::UnitMove *moveTask = new Tasks::UnitMove(this, dest);

      // Setup request data in the task
      Movement::RequestData rd;
      rd.giveUpGrains = Movement::MAX_GIVEUP_MOVEAWAY_DIST;

      moveTask->SetupRequest(rd);

      // Issue the task
      if (PrependTask(moveTask))
      {
        return (TRUE);
      }
    }
  }

  return (FALSE);
}


//
// UnitObj::Stop
//
// Order the unit to stop moving
//
Bool UnitObj::Stop()
{
  // Can we flush the current tasks
  if (FlushTasks())
  {
    // If we have a weapon, stop it firing
    if (weapon)
    {
      weapon->HaltFire();
    }

    // Stop the path follower
    if (driver)
    {
      driver->Stop();
    }

    // Success
    return (TRUE);
  }

  // Failed
  return (FALSE);
}


//
// UnitObj::AddToMapHook
//
// AddToMapHook
//
void UnitObj::AddToMapHook()
{
  // Clear the teams which can see us
  teamsCanSee = 0;

  // Call parent first
  MapObj::AddToMapHook();
  
  // EngineFX
  StartGenericFX(0xDAD834D2, EngineCallBack); // "UnitObj::Engine"

  // Inform the path follower
  if (driver)
  {
    driver->AddToMapHook();
  }

  // Create a sight map
  ASSERT(!sightMap)
  sightMap = new Sight::Map;
}


//
// UnitObj::RemoveFromMapHook
//
// RemoveFromMapHook
//
void UnitObj::RemoveFromMapHook()
{
  // Inform the path follower
  if (driver)
  {
    driver->RemoveFromMapHook();
  }

  // Unsweep line of sight
  Sight::Detach(&sightMap);

  MapObj::RemoveFromMapHook();
}


//
// UnitObj::CaptureMapHooks
//
// Capture map hooks when the object's centre point moves to a different cluster
//
void UnitObj::CaptureMapHooks(Bool capture)
{
  // Perform Map Object Processing
  MapObj::CaptureMapHooks(capture);

  if (capture)
  {
    ASSERT(currentCluster)

    // Hook the unit list
    currentCluster->unitList.Append(this);

    // Increment this teams occupation of this cluster
    if (team)
    {
      currentCluster->ai.IncOccupation(team->GetId());
    }

    // Only objects on teams perform ai cluster management
    if (team && GetHitPoints() > 0)
    {
      // The number of hitpoints this object has get added to the defense
      currentCluster->ai.AddDefense(team->GetId(), MapType()->GetArmourClass(), GetHitPoints() + GetArmour());
    }
  }
  else
  {
    // Unhook from lists of the previous cluster it was on
    if (currentCluster)
    {
      // Unhook it from the unit list
      currentCluster->unitList.Unlink(this);

      // Only objects on teams perform ai cluster management
      if (team && GetHitPoints() > 0)
      {
        // The number of hitpoints this object has get removed from the defense
        currentCluster->ai.RemoveDefense(team->GetId(), MapType()->GetArmourClass(), GetHitPoints() + GetArmour());
      }
    }
  }

  // Only objects on teams perform ai cluster management
  if (team)
  {
    // If cluster was NULL then the MapObj level should have calculated it !
    ASSERT(currentCluster)

    // For all of the weapons on the map, we need to either add or remove threat
    if (weapon)
    {
      // Add the damage this weapon can do to each armour class
      for (U32 ac = 0; ac < ArmourClass::NumClasses(); ac++)
      {
        U32 threat = UnitType()->GetWeaponThreat(ac);
        if (threat)
        {
          ASSERT(UnitType()->weaponBleedMap)

          UnitType()->weaponBleedMap->ApplyThreat(
            Point<S32>(currentCluster->xIndex, currentCluster->zIndex),
            team->GetId(), ac, threat, capture);
        }
      }
    }

    if (UnitType()->selfDestructExplosion.Alive())
    {
      // Add the damage this self destruct explosion can do to each armour class
      for (U32 ac = 0; ac < ArmourClass::NumClasses(); ac++)
      {
        U32 threat = UnitType()->GetSelfDestructThreat(ac);
        if (threat)
        {
          ASSERT(UnitType()->selfDestructBleedMap)

          UnitType()->selfDestructBleedMap->ApplyThreat(
            Point<S32>(currentCluster->xIndex, currentCluster->zIndex),
            team->GetId(), ac, threat, capture);
        }
      }
    }

    if (UnitType()->disruptor)
    {
      ASSERT(UnitType()->disruptionBleedMap)

      UnitType()->disruptionBleedMap->ApplyDisruption(
        Point<S32>(currentCluster->xIndex, currentCluster->zIndex),
        team->GetId(), capture);
    }
  }
}


//
// UnitObj::SetTeam
//
void UnitObj::SetTeam(Team *t, Bool modifyTeamList)
{
  // Remove from the map
  if (OnMap())
  {
    CaptureAllHooks(FALSE);
  }

  // Save the current team
  Team *previousTeam = team;

  // Are we currently on a team
  if (team)
  {
    // Always remove from the current team
    if (IsActivelyOnTeam())
    {
      team->RemoveFromTeam(this);
    }
    else
    {
      // Don't add objects that were on a team, but not on team list
      modifyTeamList = FALSE;
    }

    // Remove as a member
    team->RemoveMember(this);
  }

  // Set the team
  team = t;

  // If we have a team
  if (team)
  {
    // Add as a member of the team
    team->AddMember(this);

    // Set the color of the object
    Mesh().SetTeamColor(team->GetColor());
  }

  // Add to the map
  if (OnMap())
  {
    CaptureAllHooks(TRUE);
  }

  // Change team of all attached units
  for (NList<MapObj>::Iterator i(&attachments); *i; i++)
  {
    if (UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(*i))
    {
      // Only change team if on the same team (ignore parasites)
      if (unit->GetTeam() == previousTeam)
      {
        unit->SetTeam(t, FALSE);
      }
    }
  }

  // Add ourselves (and attachments) to the new team
  if (team && modifyTeamList)
  {
    team->AddToTeam(this);
  }

  // Notify client that team has changed
  NotifyPlayer(0x0DD0BE24); // "Unit::SetTeam"
}


//
// FindCover
//
// Find cover from enemies and the optional unit
//
Bool UnitObj::FindCover(Bool search, UnitObj *avoid)
{
  ASSERT(CanEverMove())

  // Can this unit move right now
  if (!Blocked(Tasks::UnitMove::GetConfigBlockingPriority()))
  {
    Point<U32> cell;

    // Find the nearest location to go to
    if (PathSearch::FindConnectedCell(cellX, cellZ, cell.x, cell.z, *this, avoid, &UnitObj::DefenseAndEnemyCantSee))
    {
      Point<F32> loc;
      WorldCtrl::CellToMetrePoint(cell, loc);
      U32 flags = Task::TF_FROM_ORDER;

      // Do we need to search for new targets as we are moving
      if (search)
      {
        flags |= Task::TF_FLAG1;
      }

      // Prepend the move task
      PrependTask(new Tasks::UnitMove(this, Vector(loc.x, 0.0f, loc.z)), flags);

      // The task was issued
      return (TRUE);
    }
  }

  // No task was issued
  return (FALSE);
}


//
// FindDanger
//
// Find the given source of danger
//
Bool UnitObj::FindDanger(UnitObj *sourceUnit)
{
  ASSERT(CanEverMove())
  ASSERT(sourceUnit)

  // Can we move to the source
  if (sourceUnit->OnMap() && !Blocked(Tasks::UnitMove::GetConfigBlockingPriority()))
  {
    // Prepend the move task
    PrependTask
    (
      new Tasks::UnitMove(this, sourceUnit->Position()), Task::TF_FROM_ORDER | Task::TF_FLAG1
    );
    
    // The task was issued
    return (TRUE);
  }

  // No task was issued
  return (FALSE);
}


//
// ReadyToRespondToDanger
//
// Is it time for this unit to respond to danger
//
Bool UnitObj::ReadyToRespondToDanger()
{
  // Get our current task
  if (Task *task = GetCurrentTask())
  {
    // Is this an idle unit
    if (task == GetIdleTask())
    {
      // Should we respond to danger in this state
      if (Tactical::QueryProperty(task->GetTaskTable(), this, Tactical::TP_RESPONDTODANGER))
      {
        // Have we been in this task long enough
        if (GameTime::TimeSinceCycle(task->GetInvoked()) > TIME_RESPONDTODANGER)
        {
          return (TRUE);
        }
      }
    }
  }

  return (FALSE);
}


//
// DangerResponse
//
// Give this unit the chance to respond to danger
//
Bool UnitObj::DangerResponse(UnitObj *sourceUnit, UnitObj *buddy)
{
  // Can this unit ever move
  if (CanEverMove())
  {
    // Is it time for this unit to respond to danger
    if (ReadyToRespondToDanger())
    {
      // Can this unit damage the source unit
      if (sourceUnit && CanDamageNow(sourceUnit))
      {
        return (FindDanger(sourceUnit));
      }
      else

      // Ignore notifications from non-mobile units
      if (!buddy || buddy->CanEverMove())
      {
        return (FindCover(TRUE, sourceUnit));
      }
    }
  }

  return (FALSE);
}


//
// DangerResponseAlert
//
// Give all friendly nearby units the chance to respond to danger
//
void UnitObj::DangerResponseAlert(UnitObj *sourceUnit)
{
  // Is it time for this unit to respond to danger
  if (ReadyToRespondToDanger())
  {
    UnitObj *unit;

    // Iterate the nearby area
    UnitObjIter::Tactical i
    (
      NULL, UnitObjIter::FilterData(GetTeam(), Relation::ALLY, Position(), 32.0F)
    );

    while ((unit = i.Next()) != NULL)
    {
      ASSERT(unit->OnMap())

      // Ignore this unit
      if (unit != this)
      {
        // Let the unit respond
        unit->DangerResponse(sourceUnit, this);
      }
    }
  }
}


//
// UnitObj::ModifyHitPoints
//
void UnitObj::ModifyHitPoints(S32 mod, UnitObj *sourceUnit, Team *sourceTeam, const Vector *veloc)
{
  // Apply difficulty modifier
  if (sourceTeam && mod < 0 && mod > S32_MIN)
  {
    mod = Utils::FtoLNearest(sourceTeam->GetDifficulty() * F32(mod));
  }

  S32 before = GetHitPoints() + GetArmour();

  // Let the map object level modify the hitpoints
  MapObj::ModifyHitPoints(mod, sourceUnit, sourceTeam, veloc);

  // Was the object alive before ?
  if (before >= 0)
  {
    // Is the object alive now ?
    if (GetHitPoints() >= 0)
    {
      // If this is a object is on a team, modify the ai clust map and generate a radio event
      if (team && mod)
      {
        // During Equip the team is NULL, so although off map, this is ignored
        ASSERT(OnMap())

        if (mod < 0)
        {
          // If hit points were removed then remove defense
          currentCluster->ai.RemoveDefense(team->GetId(), MapType()->GetArmourClass(), -mod);

          // Add pain to this armour class
          currentCluster->ai.AddPain(team->GetId(), MapType()->GetArmourClass(), -mod);

          // Signal team radio that we were damaged
          team->GetRadio().Trigger(0xB8C2654B, Radio::Event(this)); // "UnitDamaged"

          // Process movement actions
          if (CanEverMove())
          {
            // Get our current task
            Task *task = GetCurrentTask();

            // Are we now below our auto repair hitpoint 
            // threshhold and are we allowed to auto repair ?
            if 
            (
              before >= UnitType()->autoRepairHitpoints && 
              hitpoints < UnitType()->autoRepairHitpoints &&
              task && 
              Tactical::QueryProperty(task->GetTaskTable(), this, Tactical::TP_REPAIRSELF)
            )
            {
              PostRestoreNowEvent();
            }
            else 

            // Were we hit by another team (stop friendly fire scattering our units)
            if (sourceTeam && sourceTeam != team && task)
            {
              // Should we run away when hit (skirmishers etc)
              if (Tactical::QueryProperty(task->GetTaskTable(), this, Tactical::TP_RUNWHENHURT))
              {
                FindCover(FALSE, sourceUnit);
              }

              // Give AI units the chance to respond to the danger
              if (IsAI())
              {
                // Broadcast an alert BEFORE responding ourself
                DangerResponseAlert(sourceUnit);

                // Now respond to the danger ourself
                DangerResponse(sourceUnit);
              }
            }
          }

          // Did another team inflict this damage ?
          if (sourceTeam)
          {
            // Was this damage inflicted by an enemy ?
            if (team->TestRelation(sourceTeam->GetId(), Relation::ENEMY))
            {
              // Signal team radio that we're under attack from an enemy
              team->GetRadio().Trigger(0x95C3A90A, Radio::Event(this)); // "EnemyAttack"
            }

            // Was this damage inflicted bu an ally ?
            if (team->TestRelation(sourceTeam->GetId(), Relation::ALLY))
            {
              // Signal team radio that we were damaged
              team->GetRadio().Trigger(0xA2844D12, Radio::Event(this)); // "AllyAttack"
            }
          }
        }
        else
        {
          // If hit points were added then add defense
          currentCluster->ai.AddDefense(team->GetId(), MapType()->GetArmourClass(), (GetHitPoints() + GetArmour()) - before);
        }
      }
    }
    else
    {
      // Reveal clandestine and invisible units
      SetFlag(FLAG_CLANDESTINE, FALSE);
      SetFlag(FLAG_INVISIBLE, FALSE);

      // It died
      if (team)
      {
        // Report the loss
        team->ReportLoss(this);

        // Signal team radio that we were killed by someone
        if (sourceTeam)
        {
          team->GetRadio().Trigger(0x9C9C9917, Radio::Event(this)); // "UnitDestroyed"
        }

        // Kill any boarded units
        if (boardManager && boardManager->InUse())
        {
          boardManager->GetUnitObj()->SelfDestruct(FALSE, sourceTeam);
        }

        // Give AI units the chance to respond to the danger
        if (IsAI())
        {
          DangerResponseAlert(sourceUnit);
        }
      }

      if (before)
      {
        // Had hitpoints before, don't have them now ... 
        if (team)
        {
          currentCluster->ai.RemoveDefense(team->GetId(), MapType()->GetArmourClass(), before);
        }
      }
    }
  }
}


//
// UnitObj::ModifyArmour
//
void UnitObj::ModifyArmour(S32 mod)
{
  S32 before = GetArmour();

  // Let the map object level modify the armour
  MapObj::ModifyArmour(mod);

  if (mod >= 0)
  {
    S32 diff = GetArmour() - before;

    if (team && diff > 0)
    {
      // If hit points were added then add defense
      currentCluster->ai.AddDefense(team->GetId(), MapType()->GetArmourClass(), diff);
    }
  }
  else
  {
    ERR_FATAL(("ModifyArmour attempted to decrease armour"))
  }
}


//
// UnitObj::GetResourceTransport
//
// Get the amount of resource transported
//
U32 UnitObj::GetResourceTransport()
{
  return (resourceTransport);
}


//
// UnitObj::AddResouceTransport
//
// Add the amount of resource transported
//
U32 UnitObj::AddResourceTransport(U32 amount)
{
  // How much space is left
  ASSERT(resourceTransport <= UnitType()->GetResourceTransport())
  U32 left = UnitType()->GetResourceTransport() - resourceTransport;

  if (amount > left)
  {
    amount = left;
  }

  // Add to the resource
  resourceTransport += amount;

  // Return the amount actually added
  return (amount);
}


//
// UnitObj::RemoveResourceTransport
//
// Remove the amount of resource transported
//
U32 UnitObj::RemoveResourceTransport(U32 amount)
{
  if (resourceTransport < amount)
  {
    amount = resourceTransport;
  }

  // Remove the resource
  resourceTransport -= amount;

  // Return the amount actually removed
  return (amount);
}


//
// UnitObj::ClearResourceTransport
//
// Clear the amount of resource transported
//
void UnitObj::ClearResourceTransport()
{
  resourceTransport = 0;
}


//
// UnitObj::SetCanSee
//
// This unit can now be seen by the given team
//
void UnitObj::SetCanSee(U32 id)
{
  // Could this team see us before ?
  if (!TestCanSee(id))
  {
    // Convert the id into a team
    if (Team *t = Team::Id2Team(id))
    {
      // Can the team see that this unit is an enemy
      if (t->TestUnitRelation(this, Relation::ENEMY))
      {
        UnitObj *candidate, *ally = NULL, *winner = NULL;

        // Find the unit that spotted us
        UnitObjIter::Tactical i
        (
          NULL, UnitObjIter::FilterData(t, Relation::ALLY, Position(), 130.0F)
        );
  
        // Step through each possible target
        while ((candidate = i.Next()) != NULL)
        {
          // Is this unit on the required team
          if (candidate->GetTeam() == t)
          {
            // Does this unit have the spotted response
            if (candidate->UnitType()->GetResponse(0x0A4B7A25)) // "Spotted"
            {
              // Accept this as our new winner
              winner = candidate;

              // Can this unit see us
              if (winner->GetCanSee(this))
              {
                // No need to continue
                break;
              }
            }
          }
          else
          {
            ally = candidate;
          }
        }

        // Do we have a winner
        if (winner)
        {
          // Trigger event on the spotter
          t->GetRadio().Trigger(0xF50D7219, Radio::Event(winner)); // "EnemySighted::Spotter"

          // Trigger event on the unit that has been spotted
          t->GetRadio().Trigger(0xA39AE4DB, Radio::Event(this)); // "EnemySighted::Spotted"
        }
        else

        // Ignore enemies that have been spotted by an ally
        if (!ally)
        {
          // Trigger event on the unit that has been spotted
          t->GetRadio().Trigger(0x952FC343, Radio::Event(this)); // "EnemySighted"
        }
      }
    }
  }

  Game::TeamSet(teamsCanSee, id);
  Game::TeamSet(teamsHaveSeen, id);
}


//
// UnitObj::ClearCanSee
//
// This unit cannot be seen by the given team
//
void UnitObj::ClearCanSee(U32 id)
{
  Game::TeamClear(teamsCanSee, id);
}


//
// SeenByRelation
//
// Can this unit be seen by a team of the given relation
//
Bool UnitObj::SeenByRelation(Relation r)
{
  // Are we on a team
  if (team)
  {
    // Iterate each team id
    for (U32 i = 0; i < Game::MAX_TEAMS; i++)
    {
      // Valid team that can currently see this unit
      if (Team::Id2Team(i) && Game::TeamTest(teamsCanSee, i))
      {
        // Does it match the given relation
        if (team->TestRelation(i, r))
        {
          return (TRUE);
        }
      }
    }
  }
  
  return (FALSE);
}


//
// IsAI
//
// Is this an AI controlled unit
//
Bool UnitObj::IsAI()
{
  return (team ? team->IsAI() : FALSE);
}


//
// UnitObj::GetSquad
//
// Get the squad this unit belongs to
//
SquadObj * UnitObj::GetSquad()
{
  return (squad.GetPointer());
}


//
// UnitObj::ClearSquad
//
// Clear the squad this unit belongs to
//
void UnitObj::ClearSquad()
{
  // Make sure we're in a squad
  ASSERT(squad.Alive())

  // Remove ourselves from the squad
  squad->RemoveUnitObj(this);

  // Set the squad to NULL
  squad = NULL;
}


//
// UnitObj::SetSquad
//
// Set the squad this unit belongs to
//
void UnitObj::SetSquad(SquadObj *squadIn)
{
  // Make sure we're not already in a squad
  ASSERT(!squad.Alive())

  // Set the squad
  squad = squadIn;
}



//
// UnitObj::PostSquadEvent
//
// Send a notification to the squad if any
//
void UnitObj::PostSquadEvent(const Task::Event &event, Bool idle)
{
  if (squad.Alive())
  {
    Task::Event e(event.message, event.param1, Id());
    squad->PostEvent(e, idle);
  }
}


//
// CanEverDamage
//
// Can this unit ever damage the given target
//
Bool UnitObj::CanEverDamage(const Target &target)
{
  if (CanEverFire())
  {
    switch (target.GetType())
    {
      case Target::OBJECT:
        return (target.GetObj().Alive() ? CanEverDamage(target.GetObj()) : NULL);

      case Target::LOCATION:
        return (TRUE);

      case Target::INVALID:
        return (FALSE);
    }
  }
  
  return (FALSE);
}



//
// CanEverDamage
//
// Can this unit ever damage the given map object
//
Bool UnitObj::CanEverDamage(MapObj *target)
{
  ASSERT(target)

  return 
  (
    (target != this) && CanEverFire() && UnitType()->GetThreat(target->MapType()->GetArmourClass())
  );
}


//
// CanDamageNow
//
// Can this unit damage the target right now
//
Bool UnitObj::CanDamageNow(const Target &target)
{
  return (CanEverDamage(target) && GetWeapon()->HaveAmmunition());
}


//
// CanDamageNow
//
// Can this unit damage the given map object right now
//
Bool UnitObj::CanDamageNow(MapObj *target)
{
  return (CanEverDamage(target) && GetWeapon()->HaveAmmunition());
}


//
// ForceAttacking
//
// Is this unit force attacking the given object
//
Bool UnitObj::ForceAttacking(MapObj *target)
{
  ASSERT(target)

  // Do we have a weapon
  if (GetWeapon())
  {
    // Is there an attack task
    if (Tasks::UnitAttack *attackTask = TaskCtrl::Promote<Tasks::UnitAttack>(this))
    {
      // Did it come from an order
      if (attackTask->GetFlags() & Task::TF_FROM_ORDER)
      {
        // Get the current target
        const Target &t = GetWeapon()->GetTarget();

        // Are we attacking the target
        if ((t.GetType() == Target::OBJECT) && (t.GetObj().GetPointer() == target))
        {
          return (TRUE);
        }
      }
    }
  }

  return (FALSE);
}


//
// ForceAttacking
//
// Is this unit force attacking the given target
//
Bool UnitObj::ForceAttacking(const Target &target)
{
  // Is this an object
  if ((target.GetType() == Target::OBJECT) && target.GetObj().Alive())
  {
    return (ForceAttacking(target.GetObj()));
  }
  
  return (FALSE);
}


//
// Suggest a target
//
Bool UnitObj::SuggestTarget(const Target &target, Bool potShot, Bool notify)
{
  ASSERT(target.Alive())

  // Do we have a weapon ?
  if (weapon)
  {
    Task *task = GetCurrentTask();
    Tasks::UnitAttack *attackTask = TaskCtrl::Promote<Tasks::UnitAttack>(task);
    Tasks::UnitGuard *guardTask = TaskCtrl::Promote<Tasks::UnitGuard>(task);

    MapObj *guardObj = guardTask ? guardTask->GetGuardTarget().GetObject() : NULL;

    // If we can damage it, offer this target to the weapon 
    if (CanEverDamage(target) && GetWeapon()->OfferTarget(target, guardObj))
    {
      // Is this a pot shot ?
      if (potShot)
      {
        weapon->SetTarget(target);
      }
      else

      // Do we already have an attack task ?
      if (attackTask)
      {
        // Change the target of the current attack task
        attackTask->SetTarget(target);
      }
      else

      // Do we already have a guard task ?
      if (guardTask)
      {
        // Change the target of the current attack task
        guardTask->SetAttackTarget(target);
      }
      else
      {
        // Create a new task and issue it
        PrependTask(new Tasks::UnitAttack(this, target));
      }

      // If the target is an object notify the squad that we're engaging an enemy
      if (notify && target.GetType() == Target::OBJECT)
      {
        PostSquadEvent(Task::Event(0x2E537947, target.GetObj().Id())); // "Tactical::TargetFound"
      }
      return (TRUE);
    }
  }
  return (FALSE);
}


//
// UnitObj::GetMaximumAmmunition
//
// Get the maximum ammunition this unit has
//
U32 UnitObj::GetMaximumAmmunition()
{
  return (weapon ? weapon->GetType().GetAmmunition() : 0);
}


//
// UnitObj::GetAmmunition
//
// Get the current ammunition this unit has
//
U32 UnitObj::GetAmmunition()
{
  return (weapon ? weapon->GetAmmunition() : 0);
}


//
// UnitObj::ReloadAmmunition
//
void UnitObj::ReloadAmmunition(F32 pct)
{
  if (weapon)
  {
    weapon->ReloadAmmunition(U32(Utils::FtoLNearest(pct * weapon->GetType().GetAmmunition())));
  }
}


//
// GetPrimaryRestore
//
// Get the primary restore facility
//
RestoreObj * UnitObj::GetPrimaryRestore()
{
  return (primaryRestore.GetPointer());
}


//
// SetPrimaryRestore
//
// Set the primary restore facility
//
void UnitObj::SetPrimaryRestore(RestoreObj *target)
{
  primaryRestore = target;
}


//
// GetRestoreFacility
//
// If required, get an available restore facility
//
RestoreObj * UnitObj::GetRestoreFacility()
{
  // Is any restoration required
  if (RestoreObj::AnyRestoreRequired(this))
  {
    // Do we have a primary restorer set
    if (primaryRestore.Alive())
    {
      return (primaryRestore);
    }

    // Try and find an available restorer
    return (Tasks::RestoreStatic::Find(this));
  }

  return (NULL);
}


//
// RestoreNow
//
// Go to a restoration facility
//
void UnitObj::RestoreNow()
{
  // Are we currently on our way to restore
  if (Tasks::UnitGoRestore *task = TaskCtrl::Promote<Tasks::UnitGoRestore>(this))
  {
    // If a primary facility is set, offer it as a change to the task
    if (primaryRestore.Alive())
    {
      task->ChangeFacility(primaryRestore);
    }
  }
  else
  {
    // Find somewhere to restore
    if (RestoreObj *restorer = GetRestoreFacility())
    {
      // Try and flush the current task
      if (FlushTasks(Tasks::UnitGoRestore::GetConfigBlockingPriority()))
      {
        // Tell the object to go and restore
        PrependTask(new Tasks::UnitGoRestore(this));
      }
    }
    else
    {
      // No restore facility
      Stop();
    }
  }
}


//
// PostRestoreNowEvent
//
// Same as above, but posts an event
//
void UnitObj::PostRestoreNowEvent()
{
  PostEvent(Task::Event(0x1B7D0A42)); // "Unit::RestoreNow"
}


//
// UnitObj::NotifyPlayer
//
// Notify the player
//
Bool UnitObj::NotifyPlayer(U32 message, U32 param1, U32 param2)
{
  return (team ? team->NotifyPlayer(this, message, param1, param2) : FALSE);
}


//
// StartBlindTarget
//
// Start blind target timer
//
void UnitObj::StartBlindTarget(F32 time)
{
  blindTarget.Start(time);

  // Start associated FX
  // ("BlindCallBack" removed to make mesh effect work)
  StartGenericFX(0xF09C81AF, NULL, TRUE, NULL, NULL, time); // "UnitObj::Blind"
}


//
// GetCurrentDestination
//
// Get the current driver destination, if there is one
//
Bool UnitObj::GetCurrentDestination(Vector &v)
{
  if (driver)
  {
    return (driver->GetCurrentDestination(v));
  }
  return (FALSE);
}


//
// CanRecycleNow
//
// Can this unit currently recycle
//
Bool UnitObj::CanRecycleNow()
{
  return 
  (
    // Must be allowed to recycle  
    HasProperty(0xBFF53F36) // "Ability::Recycle"

    &&

    // Must not be recycling already
    !TaskCtrl::Promote<Tasks::UnitRecycle>(this)

    &&

    // Must not be blocked
    !Blocked(Tasks::UnitRecycle::GetConfigBlockingPriority())
  );
}


//
// RecycleNow
//
// Recycle this unit now
//
Bool UnitObj::RecycleNow()
{
  if (CanRecycleNow())
  {
    // Get information BEFORE flushing the tasks
    Bool cancel = UnderConstruction() ? FALSE : TRUE;
    F32 total = 0.0F, current = 0.0F;
    U32 refund = GetResourceValue();
    GetRecycleTimes(total, current);

    // Attempt to flush current tasks
    if (FlushTasks(Tasks::UnitRecycle::GetConfigBlockingPriority()))
    {
      // Create the recycle task
      PrependTask(new Tasks::UnitRecycle(this, refund, total, current, cancel));

      return (TRUE);
    }
  }

  return (FALSE);
}


//
// UpgradeNow
//
// Upgrade this unit (NULL if unable to upgrade)
//
UnitObj * UnitObj::UpgradeNow(Bool immediate)
{
  // Check each upgrade 
  for (List<UnitObjType::UpgradeInfo>::Iterator i(&UnitType()->upgrades); *i; i++)
  {
    // Get the info
    UnitObjType::UpgradeInfo *info = *i;

    // Is this an empty one
    if (!GetUpgrade(info->index).Alive())
    {
      // Attempt to flush current tasks
      if (immediate || FlushTasks(Tasks::UnitUpgrade::GetConfigBlockingPriority()))
      { 
        // Find the hardpoint
        MeshObj *hp = Mesh().FindIdent(info->point);

        if (hp)
        {
          UnitObj *obj = NULL;

          // Is this an immediate upgrade (in the studio)
          if (immediate)
          {         
            // Create the upgrade
            obj = &info->type->Spawn(hp->WorldMatrix(), GetTeam());
          }
          else
          { 
            // Create the upgrade
            obj = &info->type->Spawn(hp->WorldMatrix(), GetTeam(), TRUE);

            // Start the upgrade
            PrependTask(new Tasks::UnitUpgrade(this, obj));
          }

          // Attach to the hardpoint
          obj->Attach(this, *hp);

          // Fill the upgrade slot
          GetUpgrade(info->index) = obj;

          // Success
          return (obj);
        }
        else
        {
          LOG_WARN(("Upgrade hardpoint [%s] not found on [%s]", info->point.str, TypeName()));
        }
      }
    }
  }

  // Failed
  return (NULL);
}


//
// GetNextUpgrade
//
// Get the type of the next upgrade
//
UnitObjType * UnitObj::GetNextUpgrade()
{
  for (List<UnitObjType::UpgradeInfo>::Iterator i(&UnitType()->upgrades); *i; i++)
  {
    // Is this an empty one
    if (!GetUpgrade((*i)->index).Alive())
    {
      // Are we allowed to build it ?
      if (!GetTeam() || GetTeam()->AllowedType((*i)->type))
      {
        return ((*i)->type);
      }

      // An empty slot that we can not fill
      break;
    }
  }

  return (NULL);
}


//
// CanUpgradeNow
//
// Can this unit currently upgrade further
//
Bool UnitObj::CanUpgradeNow()
{
  // Upgrade task not blocked
  if (!Blocked(Tasks::UnitUpgrade::GetConfigBlockingPriority()))
  {  
    // Is there an upgrade type
    UnitObjType *upgrade = GetNextUpgrade();

    if (upgrade)
    {
      return (!GetTeam() || upgrade->GetPrereqs().Have(GetTeam()));
    }
  }

  return (FALSE);
}


//
// GetUpgrade
//
// Get the reaper for the given upgrade slot
//
UnitObjPtr & UnitObj::GetUpgrade(U32 i)
{
  ASSERT(upgrades)
  ASSERT(i < UnitType()->upgrades.GetCount())

  return (upgrades[i]);
}


//
// GetResourceValue
//
// Returns the total resource value of this unit (includes upgrades)
//
U32 UnitObj::GetResourceValue()
{
  // Is this unit under construction
  Tasks::UnitConstruct *task = TaskCtrl::Promote<Tasks::UnitConstruct>(this);

  // Set initial value
  U32 value = task ? task->GetResourceRemoved() : UnitType()->GetResourceCost();

  // Scale value by the current hitpoints
  //value = U32(GetHitPointPercentage() * F32(value));

  // Add value of all upgrades
  for (U32 i = 0; i < UnitType()->upgrades.GetCount(); i++)
  {
    if (GetUpgrade(i).Alive())
    {
      value += GetUpgrade(i)->GetResourceValue();
    }
  }

  return (value);
}


//
// GetConstructionPercentage
//
// Returns percentage construction is complete
//
F32 UnitObj::GetConstructionPercentage()
{
  // Is this unit under construction
  Tasks::UnitConstruct *task = TaskCtrl::Promote<Tasks::UnitConstruct>(this);

  // Ask the task, or unit is complete
  return (task ? task->GetProgress() : 1.0F);
}


//
// GetRecycleTimes
//
// Returns the recycle times, including upgrades & partial construction
//
void UnitObj::GetRecycleTimes(F32 &total, F32 &current)
{
  // Add on this units times
  total += UnitType()->GetRecycleTime();
  current += UnitType()->GetRecycleTime() * GetConstructionPercentage();

  // Add value of all upgrades
  for (U32 i = 0; i < UnitType()->upgrades.GetCount(); i++)
  {
    if (GetUpgrade(i).Alive())
    {
      GetUpgrade(i)->GetRecycleTimes(total, current);
    }
  }
}

  
//
// CanConstructEver
//
// Returns TRUE if this object can ever construct the given type
//
Bool UnitObj::CanConstructEver(UnitObjType *t)
{
  ASSERT(t)

  return 
  (
    // Constructed by this unit
    (t->GetConstructorType() == UnitType()) && 
    
    // Allowed by the team
    (!GetTeam() || GetTeam()->AllowedType(t))
  );
}


//
// CanConstructNow
//
// Returns TRUE if this object can currently construct the given type
//
Bool UnitObj::CanConstructNow(UnitObjType *t)
{
  ASSERT(t)
  ASSERT(CanConstructEver(t))

  // If we're incapacitated then bail
  if (!IsActivelyOnTeam())
  {
    return (FALSE);
  }

  // Have the prereqs
  if (GetTeam() && t->GetPrereqs().Have(GetTeam()))
  {  
    // For each upgrade
    for (List<UnitObjType::UpgradeInfo>::Iterator u(&UnitType()->upgrades); *u; u++)
    {
      // Get the info
      UnitObjType::UpgradeInfo *info = *u;

      // For each prereq
      for (UnitObjTypeList::Iterator p(&t->GetPrereqs().Get()); *p; p++)
      {
        // Check if it's a prereq for the target type
        if (
          (info->type == **p) && 
          (!GetUpgrade(info->index).Alive() || !GetUpgrade(info->index)->IsActivelyOnTeam())) 
        {
          // Missing a required upgrade
          return (FALSE);
        }
      }
    }

    // All required upgrades are present
    return (TRUE);
  }

  // Failed
  return (FALSE);
}


//
// Is it an Upgrade and I don't have it ?
//
Bool UnitObj::MissingUpgrade(UnitObjType *type)
{
  // If we're incapacitated then bail
  if (!IsActivelyOnTeam())
  {
    return (TRUE);
  }

  // For each upgrade
  for (List<UnitObjType::UpgradeInfo>::Iterator u(&UnitType()->upgrades); *u; u++)
  {
    // Get the info
    UnitObjType::UpgradeInfo *info = *u;

    // Check if it's a prereq for the target type
    if (
      (info->type == type) && 
      (!GetUpgrade(info->index).Alive() || !GetUpgrade(info->index)->IsActivelyOnTeam()))
    {
      // It is an upgrade and we don't have it !!!
      return (TRUE);
    }
  }
  return (FALSE);
}


//
// UnderConstruction
//
// Is this object under construction
//
Bool UnitObj::UnderConstruction()
{
  return 
  (
    TaskCtrl::Promote<Tasks::UnitConstruct>(this) || TaskCtrl::Promote<Tasks::UnitUpgrade>(this)
  );
}


//
// CanConstruct
//
// Returns TRUE if both the above methods succeed
//
Bool UnitObj::CanConstruct(UnitObjType *t)
{
  return (CanConstructEver(t) && CanConstructNow(t));
}


//
// GetEfficiency
//
// Get the current efficiency of this object
//
F32 UnitObj::GetEfficiency()
{
  return (GetTeam() ? UnitType()->GetEfficiency().Get(GetTeam()->GetPower()) : 1.0F);
}


//
// GetUpgradedUnit
//
// Get the highest upgrade, or this unit if none
//
UnitObj & UnitObj::GetUpgradedUnit()
{
  for (S32 i = UnitType()->upgrades.GetCount() - 1; i >= 0; i--)
  {
    if (GetUpgrade(i).Alive())
    {
      return (*(GetUpgrade(i).GetData()));
    }
  }

  return (*this);
}


//
// CanUseRallyPoint
//
// Can this unit use a rally point
//
Bool UnitObj::CanUseRallyPoint()
{
  return (SendEvent(Task::Event(UnitObjNotify::CanUseRallyPoint), TRUE));
}


//
// SetRallyPoint
//
// Set the current rally point
//
Bool UnitObj::SetRallyPoint(const Point<U32> &p)
{
  return (SendEvent(Task::Event(UnitObjNotify::SetRallyPoint, p.x, p.z), TRUE));
}


//
// GetRallyPoint
//
// Get the current rally point
//
Bool UnitObj::GetRallyPoint(Point<U32> &p)
{
  Task::RetrievedData taskInfo;

  if (Retrieve(UnitObjNotify::GetRallyPoint, taskInfo))
  {
    p.x = taskInfo.u1;
    p.z = taskInfo.u2;
    return (TRUE);
  }

  return (FALSE);
}


//
// SetAnimationUpgrades
//
// Set animation on this object and all upgrades
//
void UnitObj::SetAnimationUpgrades(U32 nameCrc, Bool blend)
{
  // Iterate the upgrades
  for (U32 i = 0; i < UnitType()->upgrades.GetCount(); i++)
  {
    // Get the upgrade reaper
    UnitObjPtr &u = GetUpgrade(i);

    // Is it active
    if (u.Alive())
    {
      // Is the animation on this object
      if (u->GetMeshRoot()->FindAnimCycle(nameCrc))
      {
        // Start it playing
        u->SetAnimation(nameCrc, blend);

        // Never start more than one animation
        return;
      }
    }
  }

  // If not found on an upgrade, always start on the base object
  SetAnimation(nameCrc, blend);
}


//
// Set tactical settings
//
void UnitObj::SetTacticalModifierSetting(U8 modifier, U8 setting)
{
  // Apply the modifer setting to the actual unit
  settings.Set(modifier, setting);

  // If this unit is in a squad, tell the squad to set its modifiers too
  if (squad.Alive())
  {
    squad->SetTacticalModifierSetting(modifier, setting);
  }

  Task *task = GetCurrentTask();
  if 
  (
    hitpoints < UnitType()->autoRepairHitpoints &&
    task && 
    Tactical::QueryProperty(task->GetTaskTable(), this, Tactical::TP_REPAIRSELF)
  )
  {
    PostRestoreNowEvent();
  }
}


//
// UnitObj::HasSpyInside
//
// Does this object have a spy inside it from this team?
//
Bool UnitObj::HasSpyInside(U32 teamId)
{
  ASSERT(teamId < Game::MAX_TEAMS)
  return (Game::TeamTest(spyingTeams, teamId));
}


//
// UnitObj::SetSpyInside
//
// Set spy inside this building state for this team
//
void UnitObj::SetSpyInside(U32 teamId, Bool state)
{
  ASSERT(teamId < Game::MAX_TEAMS)

  if (state)
  {
    ASSERT(!Game::TeamTest(spyingTeams, teamId))
    Game::TeamSet(spyingTeams, teamId);
  }
  else
  {
    ASSERT(Game::TeamTest(spyingTeams, teamId))
    Game::TeamClear(spyingTeams, teamId);
  }
}


//
// ModifySight
//
// Used to temporarily restrict the line of sight (construction, powerup/down)
//
void UnitObj::ModifySight(Bool restrict, Bool instant)
{
  // At what point do we ignore the sight
  const S32 ignoreRange = 2;

  // Get the maximum possible seeing range for this type
  U32 range = UnitType()->GetMaxSeeingRange();

  // Should we modify this unit
  if (range > ignoreRange)
  {
    // What permanent modifier value are we going to use
    F32 modifier = F32(S32(range) - ignoreRange);

    // Are we restoring the sight of this unit
    if (restrict)
    {
      // Add a restricting permanent modifier
      SeeingRangeIM().ChangePermanent(-modifier);

      // Fade out the sight
      if (!instant)
      {
        SeeingRangeIM().Add(modifier, 0.0F, 0.3F);
      }
    }
    else
    {
      // Remove the permanent modifier previously added
      SeeingRangeIM().ChangePermanent(modifier);

      // Fade in the sight
      if (!instant)
      {
        SeeingRangeIM().Add(-modifier, 0.0F, -0.3F);
      }
    }
  }

  // Do the same for each upgrade
  for (U32 i = 0; i < UnitType()->upgrades.GetCount(); i++)
  {
    if (GetUpgrade(i).Alive())
    {
      GetUpgrade(i)->ModifySight(restrict, instant);
    }
  }
}


//
// GetCanSee
//
// Can this unit see the given object
//
Bool UnitObj::GetCanSee(MapObj *object)
{
  ASSERT(OnMap())
  ASSERT(object)
  ASSERT(object->OnMap())

  // Is the object footprinted
  if (FootPrint::Instance *instance = object->GetFootInstance())
  {
    // Get our cell position
    Point<S32> p(cellX, cellZ);

    // Find the nearest position on the fringe of the object
    instance->ClampToFringe(p);

    // Clamp onto the map
    WorldCtrl::ClampCellPoint(p);

    // Can we see this position
    return (Sight::CanUnitSee(this, p.x, p.z));
  }
  else
  {
    // Can we see this position
    return (Sight::CanUnitSee(this, object->GetCellX(), object->GetCellZ()));
  }  
}


//
// RenderSpecialized
//
// Display specialized unit information
//
void UnitObj::RenderSpecialized()
{
  // Is this a disruptor
  if (UnitType()->GetDisruptor())
  {
    // Draw a circle around this point at the disruptor range
    Common::Display::MarkCircleMetre
    (
      Point<F32>(WorldMatrix().posit.x, WorldMatrix().posit.z), 
      UnitType()->GetDisruptionRange(), Color(0.5f, 1.0f, 0.5f, 0.6f)
    );
  }
}


//
// SetAltitude
//
// Set the altitude
//
void UnitObj::SetAltitude(F32 f)
{
  altitude = f;
}


//
// UnitObj::Info
//
// Dump information about the object
//
ostream & UnitObj::Info(ostream &o)
{
  return (
    o 
    << MapObj::Info(o)
    //<< "Team: " << (team ? team->GetName() : "<NONE>") << endl
  );
}


//
// Heuristics for finding cells
//
Bool UnitObj::DefenseAndEnemyCantSee(U32 &val, U32 x, U32 z, void *context)
{
  UnitObj *source = reinterpret_cast<UnitObj *>(context);

  val = 0;

  // Does this unit have a team
  if (team)
  {
    U32 num = 0;
    U32 visible = 0;
    U32 threat = 0;
    U32 totalThreat = 0;

    // Get the cluster this cell is on
    MapCluster *cluster = WorldCtrl::CellsToCluster(x, z);

    for (List<Team>::Iterator t(&team->RelatedTeams(Relation::ENEMY)); *t; t++)
    {
      // Accumulate the number of teams
      num++;

      // Accumulate the total threat
      totalThreat += AI::Map::GetThreat((*t)->GetId(), UnitType()->GetArmourClass());

      // Can this enemy see this position
      if (Sight::Visible(x, z, *t))
      {
        visible++;
      }

      // What is the threat to us in this cell
      threat += cluster->ai.GetThreat((*t)->GetId(), UnitType()->GetArmourClass());
    }

    // Calculate the value
    // The number of enemies that can see us is the most important
    val = (num - visible) * totalThreat;

    // The less threat the better
    val += (totalThreat - threat);

    if (source)
    {
      // The further we are from the source the better
      val += abs(x - source->cellX) + abs(z - source->cellZ);
    }
    else
    {
      // The further we are from the here the better
      val += abs(x - cellX) + abs(z - cellZ);
    }
  }

  return (TRUE);
}



//
// AwayFromPosition
//
// Maximizes for cells further away from the given position
//
Bool UnitObj::AwayFromPosition(U32 &val, U32 x, U32 z, void *context)
{
  // Get the start position from the context
  AwayFromPositionData *data = reinterpret_cast<AwayFromPositionData *>(context);

  // Get the position to evaluate
  Vector evaluate(WorldCtrl::CellToMetresX(x), 0.0F, WorldCtrl::CellToMetresZ(z));

  // Get the angle from the origin
  Vector v(evaluate - data->origin);
  F32 angle = F32(atan2(v.z, v.x));

  // Get angle offset
  angle = data->angle - angle;
  VectorDir::FixU(angle);

  ASSERT(angle >= -PI && angle <= PI)

  // Will be 1 if perfect match, 0 if opposite
  angle = 1.0f - (F32(fabs(angle)) * PIINV);

  // Get the distance from the origin to this cell
  F32 distance = Min<F32>(60.0F, Vector(evaluate - data->origin).MagnitudeXZ());

  // Value is a combination of the distance and the angle
  val = U32(distance * angle);

  return (TRUE);
}


//
// Scatter
//
// Tell the units in the given list to 'scatter'
//
void UnitObj::Scatter(const UnitObjList &list)
{
  // Are there any units in the list
  if (list.GetCount())
  {
    AwayFromPositionData data;
    data.origin.Set(0.0F, 0.0F, 0.0F);

    // Sum up the positions
    for (UnitObjList::Iterator i(&list); *i; i++)
    {
      data.origin += (**i)->Position();
    }

    // Is there more than one unit
    if (list.GetCount() > 1)
    {
      // Find the average position
      data.origin /= F32(list.GetCount());
    }
    else
    {
      // Move position by one cell
      data.origin += WorldCtrl::CellSize();
    }

    // Reset the iterator
    i.GoToHead();

    // Tell each unit to move
    for (!i; *i; i++)
    {
      UnitObj *unit = **i;

      if (unit->CanEverMove())
      {
        Point<U32> cell;

        // Calculate the angle from the origin to the position of this unit
        Vector v(unit->Position() - data.origin);
        data.angle = F32(atan2(v.z, v.x));
     
        // Find the best cell to move to
        if (PathSearch::FindConnectedCell(unit->GetCellX(), unit->GetCellZ(), cell.x, cell.z, *unit, &data, &UnitObj::AwayFromPosition))
        {
          if (unit->FlushTasks(Tasks::UnitMove::GetConfigBlockingPriority()))
          {
            unit->AppendTask
            (
              new Tasks::UnitMove
              (
                unit, Vector(WorldCtrl::CellToMetresX(cell.x), 0.0f, WorldCtrl::CellToMetresZ(cell.z))
              ), 
              Task::TF_FROM_ORDER
            );
          }
        }
      }
    }
  }
}


//
// UnitObj::EngineCallBack
//
// Engine Callback
//
Bool UnitObj::EngineCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *)
{
  ASSERT( (Promote::Object<UnitObjType, UnitObj>(mapObj)) )

  // Set the pitch and volume of the sound based on the RPMs on the engine
  //cbd.sound.pitch = mapObj->GetSpeed() * ((UnitObj *) mapObj)->UnitType()->maxSpeedInv + 1.0f;
  cbd.sound.volume = 1.0F;

  if (mapObj->GetSpeed() == 0.0f)
  {
    cbd.particle.delay = -1.0f;   // no particles
  }
  else
  {
    cbd.particle.delay = 1.0f / mapObj->GetSpeed();  // seconds
  }

  // The Engine keeps running
  return (FALSE);
}


//
// BlindCallBack
//
// Blind Target Callback
//
Bool UnitObj::BlindCallBack(MapObj *mapObj, FX::CallBackData &, void *)
{
  ASSERT( (Promote::Object<UnitObjType, UnitObj>(mapObj)) )

  return (((UnitObj*)mapObj)->blindTarget.Test());
}


#ifdef DEVELOPMENT

//
// UnitObj::RenderDebug
//
void UnitObj::RenderDebug()
{
  if (Common::Debug::data.movement && driver)
  {
    driver->RenderDebug();
  }

  if (Common::Debug::data.weapons)
  {
    // Draw weapon frustrum
    if (weapon)
    {
      F32 scale = 20;
      Vector verts[5];
  
      verts[4] = Vector(0.0f, 0.0f, 0.0f);
      verts[0] = weapon->GetType().GetBottomRight();
      verts[1] = weapon->GetType().GetBottomLeft();
      verts[2] = weapon->GetType().GetTopLeft();
      verts[3] = weapon->GetType().GetTopRight();

      verts[0] *= scale;
      verts[1] *= scale;
      verts[2] *= scale;
      verts[3] *= scale;

      Matrix m = WorldMatrix();
      m.posit = weapon->GetFiringLocation();
      Render::Pyramid(m, verts, Color(0.0f, 1.0f, 1.0f, 0.3f));

      // Draw in the current firing solution
      Render::FatLine(weapon->GetFiringLocation(), weapon->GetFiringLocation() + weapon->GetSolution() * 10.0f, Color(0.3f, 1.0f, 1.0f, 0.8f), 0.2f);

      // Draw in the front of the unit
      Render::FatLine(weapon->GetFiringLocation(), weapon->GetFiringLocation() + WorldMatrix().front * 10.0f, Color(0.3f, 1.0f, 0.3f, 0.8f), 0.2f);
      
      // Draw in the front of the weapon
      if (weapon->GetWeaponFront(m.front))
      {
        Render::FatLine(weapon->GetFiringLocation(), weapon->GetFiringLocation() + m.front * 10.0f, Color(1.0f, 0.3f, 0.3f, 0.8f), 0.2f);
      }

      // If the weapon has a target, highlight it
      const Target & target = weapon->GetTarget();
      if (target.Alive())
      {
        Common::Display::MarkPosition(target.GetLocation(), Color(1.0f, 0.3f, 0.3f, 0.8f));
      }

      // Highlight the guard position
      Common::Display::MarkPosition(weapon->GetGuardPosition(), Color(0.3f, 1.0f, 0.3f, 0.8f));
    }
  }

  if (Common::Debug::data.sight)
  {
    Sight::RenderDebug(this);
  }

  if (Common::Debug::data.pathSearch)
  {
    // Draw waypoints
    switch (pathFinder.State())
    {
      case PathSearch::FS_FOUND:
      case PathSearch::FS_CLOSEST:
      case PathSearch::FS_DIRECT:
      {
        const Color lineClr(0, 255L, 128L, 255L);
        const Color poleClr(128L, 60L, 0L, 255L);
        const F32 Height = 2.0F;
        const F32 Fatness = 0.4F;

        PathSearch::Point *prevPt = NULL;
        Vector pt0, pt1;

        for (PathSearch::PointList::Iterator i(&pathFinder.GetPointList()); *i; i++)
        {
          Area<S32> rc;
          rc.SetSize( (*i)->x, (*i)->z, 1, 1);

          // Draw the cell
          ::Terrain::RenderCellRect(rc, lineClr, TRUE, FALSE);

          // Shuffle point down
          pt0 = pt1;

          // Calculate new point
          pt1.x = WorldCtrl::CellToMetresX((*i)->x);
          pt1.z = WorldCtrl::CellToMetresZ((*i)->z);
          pt1.y = UnitType()->FindFloor(pt1.x, pt1.z) + Height;

          if (prevPt)
          {
            // Draw a line between the segments
            Render::FatLine(pt0, pt1, lineClr, Fatness);
          }

          // Render the peg
          Render::FatLine(pt1, Vector(pt1.x, pt1.y - Height, pt1.z), poleClr, Fatness);

          prevPt = *i;
        }
        break;
      }
    }
  }
}

#endif
