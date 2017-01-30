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
#include "offmapspawnobj.h"
#include "stdload.h"
#include "resolver.h"
#include "team.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Config scope name for type and world files
#define SCOPE_CONFIG   "OffMapSpawnObj"


///////////////////////////////////////////////////////////////////////////////
//
// Class OffMapSpawnObjType - De La OffMapSpawn
//


//
// Constructor
//
OffMapSpawnObjType::OffMapSpawnObjType(const char *name, FScope *fScope) : OffMapObjType(name, fScope)
{
  // Get specific config scope
  fScope = fScope->GetFunction(SCOPE_CONFIG);
  
  StdLoad::TypeReaperObjType(fScope, "SpawnType", spawnType); 
  StdLoad::TypeReaperObjType(fScope, "ParasiteType", parasiteType); 
}


//
// Destructor
//  
OffMapSpawnObjType::~OffMapSpawnObjType()
{
}


//
// PostLoad
//
// Called after all types are loaded
//
void OffMapSpawnObjType::PostLoad()
{
  // Call parent scope first
  OffMapObjType::PostLoad();

  // Resolve the reapers
  Resolver::Type(spawnType);
  Resolver::Type(parasiteType);
}


//
// InitializeResources
//
Bool OffMapSpawnObjType::InitializeResources()
{
  // Are we allowed to initialize resources ?
  if (OffMapObjType::InitializeResources())
  {
    if (spawnType.Alive())
    {
      spawnType->InitializeResources();
    }

    if (parasiteType.Alive())
    {
      parasiteType->InitializeResources();
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
GameObj* OffMapSpawnObjType::NewInstance(U32 id)
{
  // Allocate new object instance
  return (new OffMapSpawnObj(this, id));
}



///////////////////////////////////////////////////////////////////////////////
//
// Class OffMapSpawnObj - Instance class for above type
//

//
// Constructor
//
OffMapSpawnObj::OffMapSpawnObj(OffMapSpawnObjType *objType, U32 id) : OffMapObj(objType, id)
{
}


//
// Destructor
//
OffMapSpawnObj::~OffMapSpawnObj()
{
}


//
// Check
//
// Check execution position
//
Bool OffMapSpawnObj::Check(const Vector &pos)
{
  // Call base function first
  if (OffMapObj::Check(pos))
  {
    // Have a type to spawn
    if (OffMapSpawnType()->GetSpawnType())
    {
      Vector closest;

      // Get the closest linked location
      if (OffMapSpawnType()->GetSpawnType()->FindLinkedPos(pos, closest))
      {
        return (TRUE);
      }
    }
  }

  return (FALSE);
}


//
// Execute
//
// Execute an operation (TRUE if accepted)
//
Bool OffMapSpawnObj::Execute(U32 operation, const Vector &pos)
{
  switch (operation)
  {
    case 0x63417A92: // "Trigger::Positional"
    {
      // Get the type to be spawned
      UnitObjType *s = OffMapSpawnType()->GetSpawnType();

      // On a team, and have a type to spawn
      if (GetTeam() && s)
      {
        Vector closest;

        // Get the closest linked location
        if (s->FindLinkedPos(pos, closest))
        {
          // Create the object
          UnitObj *obj = &s->Spawn(closest, GetTeam());

          // Is there a parasite to attach
          if (OffMapSpawnType()->GetParasiteType())
          {
            OffMapSpawnType()->GetParasiteType()->Infect(obj, GetTeam());
          }
      
          // "OffMapSpawn::Executed"
          GetTeam()->GetRadio().Trigger(0x774C5EDE, Radio::Event(this, closest)); 

          return (Done());
        }
      }
    }
  }

  return (FALSE);
}
