///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-APR-1998
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "squadobj.h"
#include "squadobjctrl.h"
#include "stdload.h"
#include "unitobj.h"
#include "resolver.h"
#include "player.h"
#include "sync.h"
#include "tasks_squadattack.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Config scope name for type and world files
#define SCOPE_CONFIG   "SquadObj"


///////////////////////////////////////////////////////////////////////////////
//
// Class Type - A list of objects that forms a squad
//


//
// SquadObjType::SquadObjType
//
// Constructor
//
SquadObjType::SquadObjType(const char *name, FScope *fScope) 
: GameObjType(name, fScope)
{
  thinkInterval = 10;
}


//
// SquadObjType::PostLoad
//
// Called after all types are loaded
//
void SquadObjType::PostLoad()
{
  // Call parent scope first
  GameObjType::PostLoad();
}


//
// SquadObjType::NewInstance
//
// Create a new map object instance using this type
//
GameObj* SquadObjType::NewInstance(U32 id)
{
  // Allocate new object instance
  return (new SquadObj(this, id));
}


///////////////////////////////////////////////////////////////////////////////
//
// Class SquadObj
//


//
// SquadObj::SquadObj
//
// Constructor
//
SquadObj::SquadObj(SquadObjType *objType, U32 id) 
: GameObj(objType, id)
{
  team = NULL;
}


//
// SquadObj::SquadObj
//
// Destructor
//
SquadObj::~SquadObj()
{
  // Make sure we clear the reaper list
  list.Clear();
}


//
// SquadObj::PreDelete
//
// Pre deletion cleanup
//
void SquadObj::PreDelete()
{

  // Call parent scope last
  GameObj::PreDelete();
}


//
// SquadObj::SaveState
//
// Save a state configuration scope
//
void SquadObj::SaveState(FScope *fScope, MeshEnt * theMesh) // = NULL)
{
  // Call parent scope first
  GameObj::SaveState(fScope);

  // Create our specific config scope
  fScope = fScope->AddFunction(SCOPE_CONFIG);

  // Save the team
  if (team)
  {
    StdSave::TypeString(fScope, "Team", team->GetName());
  }

  // Save Tactical Modifier Settings
  settings.SaveState(fScope->AddFunction("ModifierSettings"));

  // Save each list node
  for (UnitList::Iterator i(&list); *i; i++)
  {
    if (FScope *sScope = StdSave::TypeReaper(fScope, "Node", **i))
    {
      (*i)->SaveState(sScope);
    }
  }
}


//
// SquadObj::LoadState
//
// Load a state configuration scope
//
void SquadObj::LoadState(FScope *fScope)
{
  // Call parent scope first
  GameObj::LoadState(fScope);

  // Get specific config scope
  fScope = fScope->GetFunction(SCOPE_CONFIG);
  FScope *sScope;

  while ((sScope = fScope->NextFunction()) != NULL)
  {
    switch (sScope->NameCrc())
    {
      case 0xEDF0E1CF: // "Team"
        SetTeam(Team::Name2Team(StdLoad::TypeString(sScope)));
        break;

      case 0x2F382D90: // "ModifierSettings"
        settings.LoadState(sScope);
        break;

      case 0xE3554C44: // "Node"
      {
        ListNode *node = list.Append();
        StdLoad::TypeReaper(sScope, *node);
        node->LoadState(sScope);
        break;
      }
    }
  }
}


//
// SquadObj::PostLoad
//
// Called after all objects are loaded
//
void SquadObj::PostLoad()
{
  // Call parent scope first
  GameObj::PostLoad();

  // Resolve the object list
  Resolver::ObjList<UnitObj, UnitObjType, ListNode>(list);
}


//
// SquadObj::SetTeam
//
// Set the team this squad belongs to
//
void SquadObj::SetTeam(Team *t)
{
  team = t;
}


//
// SquadObj::GetTeam
//
// Get the team this squad belongs to
//
Team * SquadObj::GetTeam()
{
  return (team);
}


//
// SquadObj::AddUnitObj
//
// Add a single map object
//
void SquadObj::AddUnitObj(UnitObj *obj)
{
  ASSERT(obj)

  // Check to see if the unit is alread in a squad
  if (obj->GetSquad())
  {
    // Remove it from that squad
    obj->ClearSquad();
  }

  // Force our tactical settings upon this unit
  obj->settings = settings;

  // Set the squad of the object
  obj->SetSquad(this);

  // Append the given object
  list.Append(obj);
}


//
// SquadObj::AddUnitObj
//
// Add a list of map objects
//
void SquadObj::AddUnitObjList(UnitObjList *objList)
{
  ASSERT(objList)

  // Add each unit in the list
  for (UnitObjList::Iterator u(objList); *u; u++)
  {
    if ((*u)->Alive())
    {
      AddUnitObj(**u); 
    }
  }
}


//
// SquadObj::RemoveUnitObj
//
// Remove a single unit from the squad
//
void SquadObj::RemoveUnitObj(UnitObj *obj)
{
  // Remove the unit from the list
  list.Remove(obj);
}


//
// SquadObj::Empty
//
// Empty the squad
//
void SquadObj::Empty()
{
  // Remove each of the objects from the squad
  UnitList::Iterator u(&list); 
  
  ListNode *ptr;

  while ((ptr = u++) != NULL)
  {
    if (ptr->Alive())
    {
      (*ptr)->ClearSquad();
    }
  }

  // Clear out the list
  list.Clear();
}


//
// SquadObj::GetList
//
// Get the unit list inside the squad
//
const SquadObj::UnitList & SquadObj::GetList()
{
  // Purge dead from the list
  list.PurgeDead();

  // Return whats left
  return (list);
}


//
// SquadObj::GetLocation
//
// Get the central location of the squad
//
Bool SquadObj::GetLocation(Vector &location)
{
  // Purge dead from the list
  list.PurgeDead();

  location = Vector(0.0f, 0.0f, 0.0f);
  F32 count = 0.0f;

  for (UnitList::Iterator u(&list); *u; u++)
  {
    location += (**u)->Origin();
    count += 1.0f;
  }

  if (count)
  {
    location /= count;
    return (TRUE);
  }
  else
  {
    return (FALSE);
  }
}


//
// SquadObj::GetHitPoints
//
// Get the overall health of the squad
//
S32 SquadObj::GetHitPoints()
{
  S32 total = 0;
  for (UnitList::Iterator u(&list); *u; u++)
  {
    if ((*u)->Alive())
    {
      total += (**u)->GetHitPoints();
    }
  }
  return (total);
}


//
// SquadObj::GetMaxHitPoints
//
// Get the maximum health of the squad
//
S32 SquadObj::GetMaxHitPoints()
{
  S32 total = 0;
  for (UnitList::Iterator u(&list); *u; u++)
  {
    if ((*u)->Alive())
    {
      total += (**u)->MapType()->GetHitPoints();
    }
  }
  return (total);
}


//
// SquadObj::GetAverageHealth
//
// Get the average health of the squad
//
F32 SquadObj::GetAverageHealth()
{
  F32 count = 0.0f;
  F32 total = 0.0f;
  for (UnitList::Iterator u(&list); *u; u++)
  {
    if ((*u)->Alive())
    {
      count += 1.0f;
      total += F32((**u)->GetHitPoints()) * (**u)->MapType()->GetHitPointsInv();
    }
  }
  if (count > 0.0f)
  {
    total /= count;
  }
  else
  {
    total = 0.0f;
  }
  return (total);
}


//
// SquadObj::GetThreat
//
// Get the threat of the squad to the given armour class
//
U32 SquadObj::GetThreat(U32 ac)
{
  U32 total = 0;
  for (UnitList::Iterator u(&list); *u; u++)
  {
    if ((*u)->Alive())
    {
      total += (**u)->UnitType()->GetThreat(ac);
    }
  }
  return (total);
}


//
// SquadObj::Notify
//
// Notification sent to the squad
//
void SquadObj::PostEvent(const Task::Event &event, Bool idle)
{
  // Are we interested in the message
  switch (event.message)
  {
    case 0x2E537947: // "Tactical::TargetFound"
    {
      // Someone in the squad found a target
      UnitObj *unit = Resolver::Object<UnitObj, UnitObjType>(event.param1);

      if (unit)
      {
        Target target(unit);

        // Suggest this target to the other squad members
        for (UnitList::Iterator u(&list); *u; u++)
        {
          if ((*u)->Alive())
          {
            UnitObj *unit = **u;
            Task *task = unit->GetCurrentTask();

            // If the unit has a task from an order then do not suggest the target
            if (task && !(task->GetFlags() & Task::TF_FROM_ORDER))
            {
              unit->SuggestTarget(target, FALSE, FALSE);
            }
          }
        }
      }
      break;
    }

    default:
      GameObj::PostEvent(event, idle);
      break;
  }
}


//
// SquadObj::Notify
//
// Notification for the squad to send to the player
//
Bool SquadObj::NotifyPlayer(U32 message, U32 param1, U32 param2)
{
  return (team ? team->NotifyPlayer(this, message, param1, param2) : FALSE);
}


//
// Set tactical
//
void SquadObj::SetTacticalModifierSetting(U8 modifier, U8 setting)
{
  // Apply the modifer setting to the actual squad
  settings.Set(modifier, setting);

  // Apply the modifer settings to all of the units in the squad
  for (UnitList::Iterator i(&GetList()); *i; i++)
  {
    // Change the tactical AI settings
    (**i)->settings.Set(modifier, setting);
  }
}
