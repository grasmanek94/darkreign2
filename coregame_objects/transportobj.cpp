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
#include "transportobj.h"
#include "mapobjctrl.h"
#include "stdload.h"
#include "resolver.h"
#include "taskctrl.h"
#include "task.h"
#include "fx.h"
#include "movement_pathfollow.h"
#include "sight.h"
#include "tasks_transportpad.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Config scope name for type and world files
#define SCOPE_CONFIG   "TransportObj"


///////////////////////////////////////////////////////////////////////////////
//
// Class TransportObjType - A unit that transports other units
//


//
// Constructor
//
TransportObjType::TransportObjType(const char *name, FScope *fScope) : UnitObjType(name, fScope)
{
  // Get specific config scope
  fScope = fScope->GetFunction(SCOPE_CONFIG);

  // Load config
  propertyList.Load(fScope, "Properties", FALSE);
  spaces = StdLoad::TypeU32(fScope, "Spaces", 0);
  distance = StdLoad::TypeF32(fScope, "Distance", 10.0F);
  chargeTime = StdLoad::TypeF32(fScope, "ChargeTime", 10.0F);
  StdLoad::TypeReaperObjType(fScope, "PortalType", portalType);
  portalTime = StdLoad::TypeF32(fScope, "PortalTime", chargeTime);
}


//
// Destructor
//
TransportObjType::~TransportObjType()
{
}


//
// PostLoad
//
// Called after all types are loaded
//
void TransportObjType::PostLoad()
{
  // Call parent scope first
  UnitObjType::PostLoad();

  Resolver::Type(portalType);
}


//
// InitializeResources
//
Bool TransportObjType::InitializeResources()
{
  // Are we allowed to initialize resources ?
  if (UnitObjType::InitializeResources())
  {
    if (portalType.Alive())
    {
      portalType->InitializeResources();
    }

    return (TRUE);
  }

  return (FALSE);
}


//
// NewInstance
//
// Create a new map object instance using this type
//
GameObj* TransportObjType::NewInstance(U32 id)
{
  // Allocate new object instance
  return (new TransportObj(this, id));
}


//
// CheckTransport
//
// Can the given type be transported by this transport
//
Bool TransportObjType::CheckTransport(UnitObjType *type)
{
  return (propertyList.Test(type));
}



///////////////////////////////////////////////////////////////////////////////
//
// Class TransportObj - Instance class for above type
//

//
// Constructor
//
TransportObj::TransportObj(TransportObjType *objType, U32 id) : UnitObj(objType, id)
{
}


//
// Destructor
//
TransportObj::~TransportObj()
{
  cargo.Clear();
}


//
// PreDelete
//
// Pre deletion cleanup
//
void TransportObj::PreDelete()
{
  // Delete all passengers
  for (UnitObjList::Iterator i(&cargo); *i; i++)
  {
    if ((*i)->Alive())
    {
      (**i)->MarkForDeletion();
    }
  }

  // Call parent scope last
  UnitObj::PreDelete();
}


//
// SaveState
//
// Save a state configuration scope
//
void TransportObj::SaveState(FScope *fScope, MeshEnt * theMesh) // = NULL)
{
  // Call parent scope first
  UnitObj::SaveState(fScope);

  // Create specific config scope
  fScope = fScope->AddFunction(SCOPE_CONFIG);

  cargo.PurgeDead();

  if (cargo.GetCount())
  {
    StdSave::TypeReaperList(fScope, "Cargo", cargo);
  }

  StdSave::TypeReaper(fScope, "TelepadLink", telepadLink);
}


//
// LoadState
//
// Load a state configuration scope
//
void TransportObj::LoadState(FScope *fScope)
{
  // Call parent scope first
  UnitObj::LoadState(fScope);

  // Get the config scope
  fScope = fScope->GetFunction(SCOPE_CONFIG);
  FScope *sScope;

  while ((sScope = fScope->NextFunction()) != NULL)
  {
    switch (sScope->NameCrc())
    {
      case 0x1005DD72: // "Cargo"
        StdLoad::TypeReaperList(sScope, cargo);
        break;

      case 0x56257E63: // "TelepadLink"
        StdLoad::TypeReaper(sScope, telepadLink);
        break;
    }
  }
}


//
// PostLoad
//
// Called after all objects are loaded
//
void TransportObj::PostLoad()
{
  // Call parent scope first
  UnitObj::PostLoad();

  Resolver::Object<TransportObj, TransportObjType>(telepadLink);
  Resolver::ObjList<UnitObj, UnitObjType, UnitObjListNode>(cargo);

  // If no telepad link, try and use linked unit
  if (telepadLink.Dead() && GetLinkedUnit())
  {
    telepadLink = Promote::Object<TransportObjType, TransportObj>(GetLinkedUnit());
  }
}


//
// GetUsedSpaces
//
// Get the number of spaces currently used
//
U32 TransportObj::GetUsedSpaces()
{
  cargo.PurgeDead();
  return (cargo.GetCount());
}


//
// GetFreeSpaces
//  
// Get the number of free spaces
//
U32 TransportObj::GetFreeSpaces()
{
  ASSERT(TransportType()->GetSpaces() >= GetUsedSpaces())
  
  return (TransportType()->GetSpaces() - GetUsedSpaces());
}


//
// CheckType
//
// Can the given unit be transported by this type
//
Bool TransportObj::CheckType(UnitObj *unit)
{
  // Can never transport footprinted objects
  if (unit->UnitType()->GetFootPrintType())
  {
    return (FALSE);
  }

  // Does this transport allow this type
  if (TransportType()->CheckTransport(unit->UnitType()))
  {
    // Is there a telepad link
    if (telepadLink.Alive())
    {
      // Telepad is active and ready for zap-o-rama
      return (telepadLink->GetPadTask() && telepadLink->GetPadTask()->CheckPortalEntry(unit));
    }

    // Is this a death machine
    if (GetPadTask() && !GetTeam())
    {
      return (TRUE);
    }
 
    // Is the unit on the transporters team
    if (unit->GetTeam() == GetTeam())
    {
      return (TRUE);
    }
  }

  return (FALSE);
}


//
// CheckSpace
//
// Can the given unit currently fit inside this transport
//
Bool TransportObj::CheckSpace(UnitObj *)
{
  // Death machine, telepad link, or spaces available
  return (!TransportType()->GetSpaces() || telepadLink.Alive() || GetFreeSpaces());
}


//
// CheckCargo
//
// Checks both type and space
//
Bool TransportObj::CheckCargo(UnitObj *unit)
{
  return (CheckType(unit) && CheckSpace(unit) && !cargo.Find(unit));
}


//
// PickupCargo
//
// Pickup the given cargo item
//
void TransportObj::PickupCargo(UnitObj *unit)
{
  ASSERT(unit)
   
  if (CheckCargo(unit))
  {
    // Trigger transport enter fx
    StartGenericFX(0xE9BB373F); // "Transport::Pickup"

    // Is the unit on the map
    if (unit->OnMap())
    {
      if (telepadLink.Dead())
      {
        MapType()->StartGenericFX(unit, 0x8CE85CE5, NULL, TRUE); // "Transport::Cargo::Board"
      }

      // Remove the object from the map
      MapObjCtrl::RemoveFromMap(unit);
    }

    // Is this a portal transfer
    if (telepadLink.Alive() && telepadLink->GetPadTask())
    {
      // Attempt to transfer the unit
      if (!telepadLink->GetPadTask()->TransferPortalUnit(unit))
      {
        unit->MarkForDeletion();
      }
    }
    else

    // Is this a death machine
    if (!TransportType()->GetSpaces())
    {
      unit->MarkForDeletion();
    }
    else
    {
      // Add this unit to our cargo
      AddCargo(unit);
    }
  }
  else
  {
    // Unit is no longer welcome
    unit->MarkForDeletion();
  }
}


//
// AddCargo
//
// Add the given cargo
//
void TransportObj::AddCargo(UnitObj *unit)
{
  ASSERT(unit)
  ASSERT(CheckSpace(unit))

  // This ain't for portal transfers
  ASSERT(!telepadLink.Alive())

  // Make sure that its not on the map
  ASSERT(!unit->OnMap())

  // Add it to the cargo list
  cargo.Append(unit);

  // Set the cargo flag in the unit
  unit->SetFlag(FLAG_CARGO, TRUE);
}


//
// UnloadAvailable
//
// Is this transporter able to unload right now
//
Bool TransportObj::UnloadAvailable()
{
  // Is there any carge
  if (GetUsedSpaces())
  {
    // Is there an active telepad task
    Tasks::TransportPad *pad = GetPadTask();

    if (!pad || pad->UnloadAvailable())
    {
      return (TRUE);
    }
  }

  return (FALSE);
}


//
// CheckUnload
//
// Check if this transport can currently unload at the given location
//
Bool TransportObj::CheckUnload(U32 x, U32 z)
{
  // Can see the destination and have cargo
  if (Sight::Visible(x, z, GetTeam()))
  {
    // Is there an active telepad task
    Tasks::TransportPad *pad = GetPadTask();

    // No telepad, or telepad is cool with cell
    if (!pad || pad->CheckUnload(x, z))
    {
      // Iterate the current cargo
      for (UnitObjList::Iterator i(&cargo); *i; i++)
      {
        // Is unit alive
        if ((*i)->Alive())
        {
          // Can this unit move on the location
          if (PathSearch::CanMoveToCell((**i)->UnitType()->GetTractionIndex((**i)->GetCurrentLayer()), x, z))
          {
            return (TRUE);
          }     
        }
      }
    }
  }

  // Unable to unload here
  return (FALSE);
}


//
// Unload
//
// Unload a single unit at the given location
//
Bool TransportObj::Unload(const Vector &destination, UnitObj *unit)
{
  Vector closest;
 
  // Get the closest position this type can move on
  if (unit->UnitType()->FindClosestPos(destination, closest, 5))
  {
    Vector linked;

    // Get the closest linked location
    if (unit->UnitType()->FindLinkedPos(closest, linked))
    {
      // Generate new position
      Matrix m = unit->WorldMatrix();
      m.posit = linked;

      // Move unit
      unit->SetSimCurrent(m);

      // Add the object to the map
      MapObjCtrl::AddToMap(unit);

      // Trigger boarding effects on the unit
      MapType()->StartGenericFX(unit, 0xDF24FD46); // "Transport::Cargo::Unload" 

      return (TRUE);
    }
  }

  return (FALSE);
}


//
// Unload
//
// Unload cargo at the given location (TRUE if any unloaded)
//
Bool TransportObj::Unload(const Vector &destination, Bool single)
{
  // Remove all dead reapers
  cargo.PurgeDead();

  // Have any objects been unloaded
  Bool success = FALSE;

  // Unload each unit in the cargo bay
  for (UnitObjList::Iterator i(&cargo); *i; ++i)
  {
    if (Unload(destination, **i))
    {
      // Clear the cargo flag in the unit
      (**i)->SetFlag(FLAG_CARGO, FALSE);

      // Remove from cargo
      (*i)->Clear();

      // Successfully dropped off unit
      success = TRUE;

      // Are we done
      if (single)
      {
        break;
      }
    }
  }

  if (success)
  {
    StartGenericFX(0x8DBAE7D8); // "Transport::Unload"
  }

  return (success);
}


//
// GetPadTask
//
// Get the transport pad task, or NULL
//
Tasks::TransportPad * TransportObj::GetPadTask()
{
  return (TaskCtrl::Promote<Tasks::TransportPad>(this));
}
