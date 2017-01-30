///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 14-NOV-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "spyobj.h"
#include "team.h"
#include "taskctrl.h"
#include "tasks_spyidle.h"
#include "resolver.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Config scope name for type and world files
#define SCOPE_CONFIG   "SpyObj"


///////////////////////////////////////////////////////////////////////////////
//
// Class SpyObjType - A unit that spies
//


//
// Constructor
//
SpyObjType::SpyObjType(const char *name, FScope *fScope) : UnitObjType(name, fScope)
{
  // Get specific config scope
  fScope = fScope->GetFunction(SCOPE_CONFIG);

  // Resources to steal per second, convert to per cycle
  resourceRate = Utils::FtoL(StdLoad::TypeF32(fScope, "ResourceRate", F32(GameTime::CYCLESPERSECOND), Range<F32>::positive) * GameTime::INTERVAL);

  // Level to reduce power to
  powerLevel = StdLoad::TypeF32(fScope, "PowerLevel", 0.5F, Range<F32>::percentage);

  // Power units to steal per second, convert to per cycle
  powerRate = Utils::FtoL(StdLoad::TypeF32(fScope, "PowerRate", F32(GameTime::CYCLESPERSECOND), Range<F32>::positive) * GameTime::INTERVAL);

  // Time delay from being compromised to being executed
  surrenderCycles = Utils::FtoL(StdLoad::TypeF32(fScope, "SurrenderDelay", 6.0F, Range<F32>::positive) * F32(GameTime::CYCLESPERSECOND));

  // Load the morphing properties
  properties.Load(fScope, "Properties", FALSE);
}


//
// Destructor
//
SpyObjType::~SpyObjType()
{
}


//
// PostLoad
//
// Called after all types are loaded
//
void SpyObjType::PostLoad()
{
  // Call parent scope first
  UnitObjType::PostLoad();
}


//
// InitializeResources
//
Bool SpyObjType::InitializeResources()
{
  // Are we allowed to initialize resources ?
  if (UnitObjType::InitializeResources())
  {
    return (TRUE);
  }

  return (FALSE);
}


//
// NewInstance
//
// Create a new map object instance using this type
//
GameObj* SpyObjType::NewInstance(U32 id)
{
  // Allocate new object instance
  return (new SpyObj(this, id));
}


//
// CalcPowerLeak
//
// Calculate power to steal for a given unit type
U32 SpyObjType::CalcPowerLeak(UnitObjType *type)
{
  // Use larger of day/night power production
  U32 produced = Max<U32>(type->GetPower().GetProducedDay(), type->GetPower().GetProducedNight());
  return (Utils::FtoL(F32(produced) * (1.0F - powerLevel)));
}


///////////////////////////////////////////////////////////////////////////////
//
// Class SpyObj - Instance class for above type
//


// Static data
NList<SpyObj> SpyObj::allSpies(&SpyObj::spyNode);


//
// Constructor
//
SpyObj::SpyObj(SpyObjType *objType, U32 id) : UnitObj(objType, id), morphTeam(NULL)
{
  allSpies.Append(this);
}


//
// Destructor
//
SpyObj::~SpyObj()
{
  allSpies.Unlink(this);
}


//
// PreDelete
//
// Pre deletion cleanup
//
void SpyObj::PreDelete()
{
  // Notify the idle task
  SendEvent(Task::Event(0x97824A27), TRUE); // "SpyObj::PreDelete"

  // Call parent scope last
  UnitObj::PreDelete();
}


//
// SaveState
//
// Save state configuration
//
void SpyObj::SaveState(FScope *fScope, MeshEnt * theMesh) // = NULL)
{
  // Notify the spy task that we've been found out
  Tasks::SpyIdle * task = TaskCtrl::PromoteIdle<Tasks::SpyIdle>(this);

  MeshEnt * originalMesh = NULL;

  if (task && task->IsMorphed())
  {
    originalMesh = task->oldMesh;
  }

  // Call parent scope first
  UnitObj::SaveState(fScope, originalMesh);

  // Create specific config scope
  fScope = fScope->AddFunction(SCOPE_CONFIG);

  // Save the morph team
  if (morphTeam)
  {
    StdSave::TypeString(fScope, "MorphTeam", morphTeam->GetName());
  }

  // Save the morph type
  StdSave::TypeReaperObjType(fScope, "MorphType", morphType);
}


//
// LoadState
//
// Load state configuration
//
void SpyObj::LoadState(FScope *fScope)
{
  // Call parent scope first
  UnitObj::LoadState(fScope);

  // Get specific config scope
  if ((fScope = fScope->GetFunction(SCOPE_CONFIG, FALSE)) != NULL)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x95B674EF: // "MorphTeam"
          morphTeam = Team::Name2Team(StdLoad::TypeString(sScope));
          break;

        case 0x65DBDDCC: // "MorphType"
          StdLoad::TypeReaperObjType(sScope, morphType);
          Resolver::Type(morphType);
          break;
      }
    }
  }
}


//
// SendEvent
//
// Send an Event
//
Bool SpyObj::SendEvent(const Task::Event &event, Bool idle)
{
  switch (event.message)
  {
    case 0xF874D787: // "AttackTarget"
    {
      // Notify the spy task that we've been found out
      Tasks::SpyIdle *task = TaskCtrl::PromoteIdle<Tasks::SpyIdle>(this);

      if (task && task->IsMorphed())
      {
        task->MorphDetected();
      }
      return (TRUE);
    }

    default:
      // If we don't want the event pass it down
      return (MapObj::SendEvent(event, idle));
  }
}


//
// CanMorph
//
// Can the spy morph into another unit right now
//
Bool SpyObj::CanMorph(UnitObj *unit)
{
  if (!SpyType()->CanMorph(unit->UnitType()))
  {
    // Not configured to morph into this type
    return (FALSE);
  }

  /*
  if (unit->UnitType()->GetGrainSize() != UnitType()->GetGrainSize())
  {
    // Other unit is a different size
    return (FALSE);
  }
  */

  if (!unit->CanEverMove())
  {
    // Other unit can't move
    return (FALSE);
  }

  if (GetTeam())
  {
    if (GetTeam()->GetTeamsByRelation(Relation::ENEMY) & GetTeamsCanSee())
    {
      // Spy is seen by enemy teams
      return (FALSE);
    }
  }

  return (TRUE);
}


//
// SetMorphTarget
//
// Set or clear the morph target data
//
void SpyObj::SetMorphTarget(UnitObj *unit)
{
  if (unit)
  {
    morphTeam = unit->GetTeam();
    morphType = unit->UnitType();
  }
  else
  {
    morphTeam = NULL;
    morphType = NULL;
  }
}


//
// GetMorphTeam
//
// Get the morph team of this spy, or NULL if not morphed
//
Team * SpyObj::GetMorphTeam()
{
  return (morphTeam);
}
