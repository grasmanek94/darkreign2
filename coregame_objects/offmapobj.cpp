///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 15-JUN-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "offmapobj.h"
#include "stdload.h"
#include "resolver.h"
#include "client.h"
#include "team.h"
#include "sight.h"
#include "savegame.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Config scope name for type and world files
#define SCOPE_CONFIG   "OffMapObj"


///////////////////////////////////////////////////////////////////////////////
//
// Class OffMapObjType - A unit that can build footprinted objects
//


//
// Constructor
//
OffMapObjType::OffMapObjType(const char *name, FScope *fScope) : UnitObjType(name, fScope)
{
  thinkInterval = 10;

  // Get specific config scope
  if ((fScope = fScope->GetFunction(SCOPE_CONFIG, FALSE)) != NULL)
  {
    checkTraction = StdLoad::TypeU32(fScope, "CheckTraction", TRUE);
  }
}


//
// Destructor
//
OffMapObjType::~OffMapObjType()
{
}


//
// PostLoad
//
// Called after all types are loaded
//
void OffMapObjType::PostLoad()
{
  // Call parent scope first
  UnitObjType::PostLoad();

  // If we have a constructor type, set some
  if (GetConstructorType())
  {
    GetConstructorType()->AddProperty("Provide::Offmap");
  }
}


//
// NewInstance
//
// Create a new map object instance using this type
//
GameObj* OffMapObjType::NewInstance(U32 id)
{
  // Allocate new object instance
  return (new OffMapObj(this, id));
}



///////////////////////////////////////////////////////////////////////////////
//
// Class OffMapObj - Instance class for above type
//

//
// Constructor
//
OffMapObj::OffMapObj(OffMapObjType *objType, U32 id) 
: UnitObj(objType, id), 
  executed(FALSE)
{
}


//
// Destructor
//
OffMapObj::~OffMapObj()
{
}


//
// PreDelete
//
// Pre deletion cleanup
//
void OffMapObj::PreDelete()
{

  // Call parent scope last
  UnitObj::PreDelete();
}


//
// SaveState
//
// Save a state configuration scope
//
void OffMapObj::SaveState(FScope *fScope, MeshEnt * theMesh) // = NULL)
{
  // Call parent scope first
  UnitObj::SaveState(fScope);

  if (SaveGame::SaveActive())
  {
    // Create specific config scope
    fScope = fScope->AddFunction(SCOPE_CONFIG);

    // Save data
    StdSave::TypeU32(fScope, "Executed", executed);
    StdSave::TypeReaper(fScope, "Facility", facility);
  }
}


//
// LoadState
//
// Load a state configuration scope
//
void OffMapObj::LoadState(FScope *fScope)
{
  // Call parent scope first
  UnitObj::LoadState(fScope);

  if ((fScope = fScope->GetFunction(SCOPE_CONFIG, FALSE)) != NULL)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xEC456344: // "Executed"
          executed = StdLoad::TypeU32(sScope);
          break;

        case 0x2B4A9F8C: // "Facility"
          StdLoad::TypeReaper(sScope, facility);
          break;
      }
    }
  }
}


//
// PostLoad
//
// Called after all objects are loaded
//
void OffMapObj::PostLoad()
{
  // Call parent scope first
  UnitObj::PostLoad();

  Resolver::Object<UnitObj, UnitObjType>(facility);
}


//
// SetFacility
//
// Setup the facility
//
void OffMapObj::SetFacility(UnitObj *unit)
{
  facility = unit;
}


//
// Check
//
// Check execution position
//
Bool OffMapObj::Check(const Vector &pos)
{
  // Check traction
  if (OffMapType()->GetCheckTraction())
  {
    // Convert to cells
    U32 x = WorldCtrl::MetresToCellX(pos.x);
    U32 z = WorldCtrl::MetresToCellZ(pos.z);

    if (!PathSearch::CanMoveToCell(UnitType()->GetTractionIndex(Claim::LAYER_LOWER), x, z))
    {
      return (FALSE);
    }
  }

  // Must be on a team that can see the location
  if (GetTeam())
  {
    if (GetTeam()->IsAI())
    {
      return (TRUE);
    }

    if (Sight::Visible(WorldCtrl::MetresToCellX(pos.x), WorldCtrl::MetresToCellZ(pos.z), GetTeam()))
    {
      // No disruptors in the area
      return (UnitObjType::FindDisruptor(GetTeam(), pos) == NULL);
    }
  }

  return (FALSE);
}


//
// Execute
//
// Execute an operation (TRUE if accepted)
//
Bool OffMapObj::Execute(U32, const Vector &)
{
  return (FALSE);  
}


//
// Done
//
// Called when the object has been executed and can be deleted
//
Bool OffMapObj::Done()
{
  // Notify the facility
  if (facility.Alive())
  {
    facility->PostEvent(Task::Event(0xC44A7A13, Id())); // "Order::UnitConstructor::ClearOffMap"   
  }

  // Delete this object
  MarkForDeletion();

  // Mark as completed incase of duplicate orders
  return (executed = TRUE);
}

