///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Unit Information Generation
//
// 24-NOV-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "unitobjinfo.h"
#include "tactical.h"
#include "restoreobj.h"
#include "taskctrl.h"
#include "transportobj.h"
#include "tasks_unitpowerdown.h"
#include "tasks_unitconstruct.h"
#include "tasks_restorestatic.h"
#include "tasks_unitguard.h"
#include "team.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class UnitObjInfo::TacticalInfo
//


//
// Constructor
//
UnitObjInfo::TacticalInfo::TacticalInfo()
{
  // Get the number of modifiers
  count = Tactical::GetNumModifiers();
  
  if (count)
  {
    // Allocate info for each modifier
    modifiers = new Info[count];

    // Setup each item
    for (U8 m = 0; m < count; m++)
    {
      // Get the modifier info
      Info &info = modifiers[m];

      // Get the number of settings for this modifier
      info.count = Tactical::GetNumSettings(m);

      if (info.count)
      {
        // Allocate info for each setting
        info.settings = new Bool[info.count];
      }
    }
  }
}


//
// Destructor
//
UnitObjInfo::TacticalInfo::~TacticalInfo()
{
  if (count)
  {
    for (U8 m = 0; m < count; m++)
    {
      Info &info = modifiers[m];

      if (info.count)
      {
        delete [] info.settings;
      }
    }

    delete [] modifiers;
  }
}


//
// Set
//
// Set the flag for the given setting
//
void UnitObjInfo::TacticalInfo::Set(U8 modifier, U8 setting, Bool value)
{
  // Do we know about this modifier
  if (modifier < count)
  {
    // Get the info
    Info &info = modifiers[modifier];

    // Do we know about this setting
    if (setting < info.count)
    {
      // Store the new value
      info.settings[setting] = value;
    }
  }
}


//
// Reset
//
// Reset all information
//
void UnitObjInfo::TacticalInfo::Reset()
{
  // For each modifier
  for (U8 m = 0; m < count; m++)
  {
    // Get the modifier info
    Info &info = modifiers[m];

    // For each setting
    for (U8 s = 0; s < info.count; s++)
    {
      info.settings[s] = FALSE;
    }

    // Clear used count
    info.countUsed = 0;
  }
}


//
// Add
//
// Add the settings from the given unit
//
void UnitObjInfo::TacticalInfo::Add(UnitObj *unit)
{
  // For each known modifier
  for (U8 m = 0; m < count; m++)
  {
    // Add the setting from this unit
    Set(m, unit->settings.Get(m), TRUE);
  }
}


//
// Done
//
// Finished adding units
//
void UnitObjInfo::TacticalInfo::Done()
{
  // For each known modifier
  for (U8 m = 0; m < count; m++)
  {
    // Get the modifier info
    Info &info = modifiers[m];

    // For each setting
    for (U8 s = 0; s < info.count; s++)
    {
      if (info.settings[s])
      {
        info.countUsed++;
      }
    }
  } 
}


//
// GetUsedSettings
//
// Get the number of settings used for the given modifier
//
U32 UnitObjInfo::TacticalInfo::GetUsedSettings(U8 modifier) const
{
  return ((modifier < count) ? modifiers[modifier].countUsed : 0);
}


//
// Get
//
// Get the flag for the given setting
//
Bool UnitObjInfo::TacticalInfo::Get(U8 modifier, U8 setting) const
{
  // Do we know about this modifier
  if (modifier < count)
  {
    // Get the info
    Info &info = modifiers[modifier];

    // Do we know about this setting
    if (setting < info.count)
    {
      return (info.settings[setting]);
    }
  }

  return (FALSE);
}



///////////////////////////////////////////////////////////////////////////////
//
// Class UnitObjInfo
//


//
// Constructor
//
UnitObjInfo::UnitObjInfo()
{
  // Initialize all data to default values
  Reset();
}


//
// Destructor
//
UnitObjInfo::~UnitObjInfo()
{
  attributes.DisposeAll();
}


//
// AddAttributesOnce
//
// Add attributes once per update
//
void UnitObjInfo::AddAttributesOnce()
{
}


//
// AddAttributes
//
// Add the attributes for the given unit
//
void UnitObjInfo::AddAttributes(UnitObj *unit)
{
  if (unit->GetFootInstance())
  {
    attributes.AddNoDup(0x2FC81A55); // "FootPrinted"
  }
  else
  {
    attributes.AddNoDup(0x6E81A3CE); // "NonFootPrinted"
  }

  if (unit->CanEverMove())
  {
    attributes.AddNoDup(0x7EE0F119); // "CanEverMove"
  }

  if (unit->CanEverFire())
  {
    attributes.AddNoDup(0xA68A11C6); // "CanEverFire"
  }

  if (Tasks::UnitGuard::CanGuard(unit))
  {
    attributes.AddNoDup(0x033E9C3D); // "CanGuard"
  }

  if (unit->GetActiveTask() && !unit->Blocked())
  {
    attributes.AddNoDup(0xFF62DA04); // "Stop"
  }

  if 
  (
    unit->UnitType()->GetResourceTransport() && 
    unit->UnitType()->GetResourceTransport() != unit->GetResourceTransport()
  )
  {
    attributes.AddNoDup(0xCA9760D8); // "CollectResource"
  }

  if (unit->GetResourceTransport())
  {
    attributes.AddNoDup(0x96C38977); // "StoreResource"
  }

  if (unit->CanEverMove() && RestoreObj::AnyRestoreRequired(unit))
  {
    attributes.AddNoDup(0xE4F151B8); // "RestoreRequired"

    if (Tasks::RestoreStatic::Find(unit))
    {
      attributes.AddNoDup(0xF2DE2398); // "RestoreAvailable"
    }
  }

  if (unit->HasProperty(0x54D4152A) && !unit->UnderConstruction()) // "Ability::SelfDestruct"
  {
    attributes.AddNoDup(0x3E6F6C6C); // "SelfDestruct"
  }

  if (unit->CanRecycleNow() && (!unit->UnitType()->CanBoard() || !unit->GetBoardManager()->InUse()))
  {
    attributes.AddNoDup(0xF8AB3A7A); // "Recycle"
  }

  if (unit->GetNextUpgrade())
  {
    attributes.AddNoDup(0xE5CA04D2); // "UpgradeAvailable"
  }

  if (unit->CanUpgradeNow())
  {
    attributes.AddNoDup(0xE1977EAE); // "Upgrade"
  }

  if (unit->UnitType()->GetPower().GetRequired())
  {
    if (TaskCtrl::Promote<Tasks::UnitPowerDown>(unit))
    {
      if (unit->SendEvent(Task::Event(0xDC8CB3E0))) // "UnitPowerDown::CanPowerUp"
      {
        attributes.AddNoDup(0x57BE223A); // "PowerUp"
      }
    }
    else

    if (!unit->Blocked(Tasks::UnitPowerDown::GetConfigBlockingPriority()))
    {
      attributes.AddNoDup(0xD177BA84); // "PowerDown"
    }
  }

  // Is the unit a transporter
  if (TransportObj *transport = Promote::Object<TransportObjType, TransportObj>(unit))
  {
    // Does it have something to unload
    if (transport->UnloadAvailable())
    {
      attributes.AddNoDup(0x13DF72D7); // "UnloadAvailable"
    }
  }

  if (unit->CanUseRallyPoint())
  {
    attributes.AddNoDup(0x67DEDB99); // "RallyPoint"
  }

  // Are there any spies inside?
  if (unit->UnitType()->CanBeInfiltrated() && unit->GetSpyingTeams())
  {
    attributes.AddNoDup(0xA72CC165); // "Infiltrated"
  } 

  if (unit->GetSquad())
  {
    attributes.AddNoDup(0x32F479DF); // "InSquad"
  }

  // Can the current task be paused
  if (unit->SendEvent(Task::Event(TaskNotify::CanPause)))
  {
    attributes.AddNoDup(0x37345010); // "Pause"
  }
  else

  // Can the current task be unpaused
  if (unit->SendEvent(Task::Event(TaskNotify::CanUnpause)))
  {
    attributes.AddNoDup(0x0642D599); // "Unpause"
  }

  // Can the current task be cancelled
  if (unit->SendEvent(Task::Event(TaskNotify::CanCancel)))
  {
    attributes.AddNoDup(0x3E412225); // "Cancel"
  }
}


//
// Reset
//
// Reset all information
//
void UnitObjInfo::Reset()
{
  // No units added
  numberAdded = 0;

  // Reset the tactical information
  tacticalInfo.Reset();

  // Clear the attributes tree
  attributes.DisposeAll();

  // Add the once only attributes
  AddAttributesOnce();
}


//
// Add
//
// Add information for the given object
//
void UnitObjInfo::Add(UnitObj *unit)
{
  // Record that this unit was added
  numberAdded++;

  // Add the tactical information
  tacticalInfo.Add(unit);

  // Add the unit attributes
  AddAttributes(unit);
}


//
// Add
//
// Add information for each unit in the given list
//
void UnitObjInfo::Add(const UnitObjList &list)
{
  // Add each unit in the list
  for (UnitObjList::Iterator i(&list); *i; i++)
  {
    Add(**i);
  }
}


//
// Done
//
// Finished adding units
//
void UnitObjInfo::Done()
{
  tacticalInfo.Done();
}


//
// FindAttribute
//
// True if the given attribute is present
//
Bool UnitObjInfo::FindAttribute(U32 crc)
{
  return (attributes.Exists(crc) ? TRUE : FALSE);
}
