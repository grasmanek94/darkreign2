///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 06-JAN-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "restoreobj.h"
#include "stdload.h"
#include "resolver.h"
#include "taskctrl.h"
#include "task.h"
#include "tasks_restorestatic.h"
#include "fx.h"
#include "team.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Config scope name for type and world files
#define SCOPE_CONFIG   "RestoreObj"


///////////////////////////////////////////////////////////////////////////////
//
// Class RestoreObjType - A unit that restores hitpoints on other units
//


//
// Constructor
//
RestoreObjType::RestoreObjType(const char *name, FScope *fScope) : UnitObjType(name, fScope)
{
  // Get specific config scope
  fScope = fScope->GetFunction(SCOPE_CONFIG);

  // Load config
  propertyList.Load(fScope, "TargetProperties", FALSE);
  addHitPoints = StdLoad::TypeU32(fScope, "AddHitPoints", 0);
  reloadRate = StdLoad::TypeF32(fScope, "ReloadRate", 0.0F);
  distance = StdLoad::TypeF32(fScope, "Distance", 4.0F);
  isStatic = (idleTask.crc == 0x1DFD760D); // "Tasks::RestoreStatic"
}


//
// Destructor
//
RestoreObjType::~RestoreObjType()
{
}


//
// PostLoad
//
// Called after all types are loaded
//
void RestoreObjType::PostLoad()
{
  // Call parent scope first
  UnitObjType::PostLoad();
}


//
// NewInstance
//
// Create a new map object instance using this type
//
GameObj* RestoreObjType::NewInstance(U32 id)
{
  // Allocate new object instance
  return (new RestoreObj(this, id));
}


//
// CheckRestore
//
// Can the given type be restored by this restorer
//
Bool RestoreObjType::CheckRestore(GameObjType *type)
{
  return (propertyList.Test(type));
}


//
// RestoreCallBack
//
// Returns FALSE when the restoration FX should stop
//
Bool RestoreObjType::RestoreCallBack(MapObj *obj, FX::CallBackData &, void *)
{
  return (!obj->SendEvent(Task::Event(0x2F20A733))); // "RestoreObjType::IsRestoring"
}


//
// StartRestoreFX
//
// Start the restoration FX
//
void RestoreObjType::StartRestoreFX(UnitObj *obj)
{
  obj->StartGenericFX(0x653C7E7A, RestoreCallBack); // "Restore::Process"
}



///////////////////////////////////////////////////////////////////////////////
//
// Class RestoreObj - Instance class for above type
//


//
// AnyRestoreRequired
//
// Does the given object require restoration
//
Bool RestoreObj::AnyRestoreRequired(UnitObj *unit)
{
  // Ignore if already dying
  if (!unit->Dying())
  {
    // Does it need hitpoints
    if (unit->GetHitPoints() < unit->MapType()->GetHitPoints())
    {
      // Ignore if the object has not yet been damaged (under construction)
      if (unit->GetNegativeModifyHitPoints())
      {
        // Ignore if the task is saying we should not restore
        if (!unit->SendEvent(Task::Event(RestoreObjNotify::BlockHitPoints)))
        {
          return (TRUE);
        }
      }
    }

    // Does it need ammunition
    if (unit->GetAmmunition() < unit->GetMaximumAmmunition())
    {
      return (TRUE);
    }
  }

  return (FALSE);
}


//
// Constructor
//
RestoreObj::RestoreObj(RestoreObjType *objType, U32 id) : UnitObj(objType, id)
{
}


//
// Destructor
//
RestoreObj::~RestoreObj()
{
}


//
// CanRestore
//
// Can the given object ever be restored by this object
//
Bool RestoreObj::CanRestore(UnitObj *obj)
{
  return
  (  
    // Unable to restore self
    (obj != this)

    && 

    // Must be in the property list
    RestoreType()->CheckRestore(obj->GameType()) 
    
    && 

    // Must be an ally
    Team::TestRelation(GetTeam(), obj->GetTeam(), Relation::ALLY)
  );
}


//
// RestoreRequired
//
// Does the given object require restoring by this object
//
Bool RestoreObj::RestoreRequired(UnitObj *obj)
{
  ASSERT(obj)
 
  // Does the object require any restoration by us
  if (AnyRestoreRequired(obj) && CanRestore(obj))
  {
    // Does it need hitpoints
    if (obj->GetHitPoints() < obj->MapType()->GetHitPoints())
    {
      // Can we supply hitpoints
      if (RestoreType()->GetAddHitPoints())
      {
        return (TRUE);
      }
    }

    // Does it need ammunition
    if (obj->GetAmmunition() < obj->GetMaximumAmmunition())
    {
      // Can we supply ammo
      if (RestoreType()->GetReloadRate() > 0.0F)
      {
        return (TRUE);
      }
    }
  }

  return (FALSE);
}


//
// Restore
//
// Do one restoration process on the given unit
//
void RestoreObj::Restore(UnitObj *obj)
{
  ASSERT(obj)

  // Add hitpoints
  if (RestoreType()->GetAddHitPoints())
  {
    S32 add = Clamp<S32>
    (
      0, 
      Utils::FtoLNearest(F32(RestoreType()->GetAddHitPoints()) * GetEfficiency()), 
      obj->MapType()->GetHitPoints() - obj->GetHitPoints()
    );

    // Modify the current hitpoint value
    if (add)
    {
      obj->ModifyHitPoints(add);
    }
  }

  // Give ammo to the target
  if (RestoreType()->GetReloadRate() > 0.0F)
  {
    // Get the rate, and modify for current efficiency
    F32 r = RestoreType()->GetReloadRate() * GetEfficiency();

    if (r)
    {
      obj->ReloadAmmunition(r);
    }
  }

  // Trigger an effect on the provider
  StartGenericFX(0xAD72D6D0); // "Restore::Source::Process"

  // Trigger an effect on the recipient
  obj->StartGenericFX(0x53551B19); // "Restore::Target::Process"
}
