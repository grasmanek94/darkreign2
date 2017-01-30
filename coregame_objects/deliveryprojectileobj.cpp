///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 18-AUG-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "deliveryprojectileobj.h"
#include "stdload.h"
#include "unitobj.h"
#include "mapobjctrl.h"
#include "gameobjctrl.h"
#include "resolver.h"
#include "taskctrl.h"
#include "tasks_unitattack.h"
#include "fx.h"

///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Config scope name for type and world files
#define SCOPE_CONFIG "DeliveryProjectileObj"


///////////////////////////////////////////////////////////////////////////////
//
// Class ProjectileObjType - Base type class for all map object types
//


//
// DeliveryProjectileObjType::DeliveryProjectileObjType
//
// Constructor
//
DeliveryProjectileObjType::DeliveryProjectileObjType(const char *name, FScope *fScope) 
: ProjectileObjType(name, fScope)
{
  fScope = fScope->GetFunction(SCOPE_CONFIG);

  // Load configuration
  StdLoad::TypeReaperObjType(fScope, "DeliveryType", deliveryType);
  count = StdLoad::TypeU32(fScope, "Count", 1);
  velocity = StdLoad::TypeF32(fScope, "Velocity", 10.0F);
  StdLoad::TypeReaperObjType(fScope, "Parasite", parasite);
}


//
// DeliveryProjectileObjType::~DeliveryProjectileObjType
//
// Destructor
//
DeliveryProjectileObjType::~DeliveryProjectileObjType()
{
}


//
// DeliveryProjectileObjType::PostLoad
//
// PostLoad
//
void DeliveryProjectileObjType::PostLoad()
{
  // Call Parent First
  ProjectileObjType::PostLoad();

  // Resolve reapers
  Resolver::Type<UnitObjType>(deliveryType, TRUE);
  Resolver::Type(parasite);
}


//
// DeliveryProjectileObjType::InitializeResources
//
Bool DeliveryProjectileObjType::InitializeResources()
{
  // Are we allowed to initialize resources ?
  if (ProjectileObjType::InitializeResources())
  {
    return (TRUE);
  }
  return (FALSE);
}


//
// DeliveryProjectileObjType::NewInstance
//
// Create a new map object instance using this type
//
GameObj* DeliveryProjectileObjType::NewInstance(U32 id)
{
  // Allocate new object instance
  return (new DeliveryProjectileObj(this, id));
}



///////////////////////////////////////////////////////////////////////////////
//
// Class DeliveryProjectileObj - Instance class for above type
//

//
// DeliveryProjectileObj::DeliveryProjectileObj
//
// Constructor
//
DeliveryProjectileObj::DeliveryProjectileObj(DeliveryProjectileObjType *objType, U32 id) 
: ProjectileObj(objType, id)
{
}


//
// DeliveryProjectileObj::~DeliveryProjectileObj
//
// Destructor
//
DeliveryProjectileObj::~DeliveryProjectileObj()
{
}


//
// DeliveryProjectileObj::PreDelete
//
// Pre deletion cleanup
//
void DeliveryProjectileObj::PreDelete()
{

  // Call parent scope last
  ProjectileObj::PreDelete();
}


//
// DeliveryProjectileObj::Detonate
//
// The projectile is detonated
//
void DeliveryProjectileObj::Detonate()
{
  if (!detonated)
  {
    StartGenericFX(0xFFF9084C); // "DeliveryProjectileObj::Detonate"
 
    // Get the type to spawn
    UnitObjType *spawnType = DeliveryProjectileType()->deliveryType;

    // Spawn the number required
    for (U32 i = 0; i < DeliveryProjectileType()->count; i++)
    {
      // Attempt to create near our location
      if (UnitObj *unit = spawnType->SpawnClosest(Position(), GetSourceTeam()))
      {
        // Does this projectile attach a parasite
        if (ParasiteObjType *parasite = DeliveryProjectileType()->GetParasite())
        {
          parasite->Infect(unit, unit->GetTeam());
        }
      }
    }

    detonated = TRUE;
  }

  // Stop the projectile
  SetSpeed(0.0F);
  SetVelocity(Matrix::I.posit);

  // Delete the projectile
  MarkForDeletion();
}
