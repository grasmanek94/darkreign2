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
#include "parasiteobj.h"
#include "resolver.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Config scope name for type and world files
#define SCOPE_CONFIG   "ParasiteObj"


///////////////////////////////////////////////////////////////////////////////
//
// Class ParasiteObjType - Fixed objects that trigger on detection of enemy
//

//
// Constructor
//
ParasiteObjType::ParasiteObjType(const char *name, FScope *fScope) : UnitObjType(name, fScope)
{
  // Get specific config scope
  fScope = fScope->GetFunction(SCOPE_CONFIG);

  // Load config
  time1 = StdLoad::TypeF32(fScope, "Time1", 0.0F);
  time2 = StdLoad::TypeF32(fScope, "Time2", 0.0F);
  useRadio = StdLoad::TypeU32(fScope, "UseRadio", FALSE, Range<U32>::flag);
}


//
// Destructor
//
ParasiteObjType::~ParasiteObjType()
{
}


//
// PostLoad
//
// Called after all types are loaded
//
void ParasiteObjType::PostLoad()
{
  // Call parent scope first
  UnitObjType::PostLoad();
}


//
// NewInstance
//
// Create a new map object instance using this type
//
GameObj* ParasiteObjType::NewInstance(U32 id)
{
  // Allocate new object instance
  return (new ParasiteObj(this, id));
}


//
// Infect
//
// Infect the given object with this parasite (FALSE if already infected)
//
Bool ParasiteObjType::Infect(UnitObj *target, Team *team)
{
  ASSERT(target)

  // Ignore if already infected or null mesh
  if (!target->GetFlag(UnitObj::FLAG_PARASITE) && !target->MapType()->IsNullObj())
  {
    // Create the parasite
    ParasiteObj *p = (ParasiteObj*)(&Spawn(target->WorldMatrix(), team));

    // Attach to the target
    p->Attach(target, target->Mesh());

    // Set the target pointer
    p->SetTarget(target);

    // Set the clandestine flag in the parasite
    p->SetFlag(UnitObj::FLAG_CLANDESTINE, TRUE);

    // Trigger the effect
    StartGenericFX(target, 0x424516F6); // "Parasite::Infected"

    return (TRUE);
  }

  return (FALSE);
}



///////////////////////////////////////////////////////////////////////////////
//
// Class ParasiteObj - Instance class for above type
//

//
// Constructor
//
ParasiteObj::ParasiteObj(ParasiteObjType *objType, U32 id) : UnitObj(objType, id)
{
}


//
// Destructor
//
ParasiteObj::~ParasiteObj()
{
}


//
// PreDelete
//
// Pre deletion cleanup
//
void ParasiteObj::PreDelete()
{
  if (target.Alive())
  {
    target->SetFlag(UnitObj::FLAG_PARASITE, FALSE);
  }

  // Call parent scope last
  UnitObj::PreDelete();
}


//
// SaveState
//
// Save a state configuration scope
//
void ParasiteObj::SaveState(FScope *fScope, MeshEnt * theMesh) // = NULL)
{
  // Call parent scope first
  UnitObj::SaveState(fScope);

  // Create specific config scope
  fScope = fScope->AddFunction(SCOPE_CONFIG);

  StdSave::TypeReaper(fScope, "Target", target);
}


//
// LoadState
//
// Load a state configuration scope
//
void ParasiteObj::LoadState(FScope *fScope)
{
  // Call parent scope first
  UnitObj::LoadState(fScope);

  // Get the config scope
  fScope = fScope->GetFunction(SCOPE_CONFIG);

  StdLoad::TypeReaper(fScope, "Target", target);
}


//
// PostLoad
//
// Called after all objects are loaded
//
void ParasiteObj::PostLoad()
{
  // Call parent scope first
  UnitObj::PostLoad();

  Resolver::Object<UnitObj, UnitObjType>(target);
}


//
// SetTarget
//
// Sets the target
//
void ParasiteObj::SetTarget(UnitObj *t)
{
  target = t;

  // Set the infection flag
  if (t)
  {
    t->SetFlag(UnitObj::FLAG_PARASITE, TRUE);
  }
}

