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
#include "trapobj.h"
#include "unitobjiter.h"
#include "resolver.h"
#include "weapon.h"
#include "savegame.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Config scope name for type and world files
#define SCOPE_CONFIG   "TrapObj"


///////////////////////////////////////////////////////////////////////////////
//
// Class TrapObjType - Fixed objects that trigger on detection of enemy
//

//
// Constructor
//
TrapObjType::TrapObjType(const char *name, FScope *fScope) : UnitObjType(name, fScope)
{
  // Get specific config scope
  fScope = fScope->GetFunction(SCOPE_CONFIG);

  // Load config
  distance = StdLoad::TypeF32(fScope, "Distance", 10.0F);
  chargeTime = StdLoad::TypeF32(fScope, "ChargeTime", 10.0F);
  selfDestruct = StdLoad::TypeU32(fScope, "SelfDestruct", 0);
  weaponSpeed = StdLoad::TypeF32(fScope, "WeaponSpeed", 0.0F);
  StdLoad::TypeReaperObjType(fScope, "Parasite", parasite);
  properties.Load(fScope, "Properties");
}


//
// Destructor
//
TrapObjType::~TrapObjType()
{
}


//
// PostLoad
//
// Called after all types are loaded
//
void TrapObjType::PostLoad()
{
  // Call parent scope first
  UnitObjType::PostLoad();

  // Resolve reapers
  Resolver::Type(parasite);
}


//
// NewInstance
//
// Create a new map object instance using this type
//
GameObj* TrapObjType::NewInstance(U32 id)
{
  // Allocate new object instance
  return (new TrapObj(this, id));
}



///////////////////////////////////////////////////////////////////////////////
//
// Class TrapObj - Instance class for above type
//


// Static data
NList<TrapObj> TrapObj::allTraps(&TrapObj::node);


//
// Constructor
//
TrapObj::TrapObj(TrapObjType *objType, U32 id) : UnitObj(objType, id)
{
  // Add to global trap list
  allTraps.Append(this);
}


//
// Destructor
//
TrapObj::~TrapObj()
{
  // Remove from global trap list
  allTraps.Unlink(this);
}


//
// PreDelete
//
// Pre deletion cleanup
//
void TrapObj::PreDelete()
{

  // Call parent scope last
  UnitObj::PreDelete();
}


//
// SaveState
//
// Save a state configuration scope
//
void TrapObj::SaveState(FScope *fScope, MeshEnt * theMesh) // = NULL)
{
  // Call parent scope first
  UnitObj::SaveState(fScope);

  if (SaveGame::SaveActive())
  {
    fScope = fScope->AddFunction(SCOPE_CONFIG);
    recharge.SaveState(fScope->AddFunction("Recharge"));
  }
}


//
// LoadState
//
// Load a state configuration scope
//
void TrapObj::LoadState(FScope *fScope)
{
  // Call parent scope first
  UnitObj::LoadState(fScope);

  // Get the optional config scope
  if ((fScope = fScope->GetFunction(SCOPE_CONFIG, FALSE)) != NULL)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xCFB61F0B: // "Recharge"
          recharge.LoadState(sScope);
          break;
      }
    } 
  }
}


//
// Poll
//
// Attempt to trigger this trap
//
Bool TrapObj::Poll()
{
  // Did we find a valid target
  Bool valid = FALSE;

  // Has recharging finished
  if (recharge.Test())
  {
    UnitObj *target;

    // Find an enemy within the configured range
    UnitObjIter::Tactical i
    (
      NULL, UnitObjIter::FilterData(GetTeam(), Relation::ENEMY, Position(), TrapType()->GetDistance())
    );
    
    // Step through each possible target
    while ((target = i.Next()) != NULL)
    {
      // Can this trap trigger on this target
      if (TrapType()->Test(target))
      {
        // Does this trap self destruct
        if (TrapType()->GetSelfDestruct())
        {
          SelfDestruct(TRUE, GetTeam());
          valid = TRUE;
        }

        // Does this trap attach a parasite
        if (TrapType()->GetParasite() && TrapType()->GetParasite()->Infect(target, GetTeam()))
        {
          valid = TRUE;
        }

        // Should we fire a weapon
        if (TrapType()->GetWeaponSpeed() > 0.0F)
        {
          if (GetWeapon())
          {
            Vector pos = Origin();
            pos.y += 20.0f;
            GetWeapon()->SetTarget(Target(pos));
            valid = TRUE;
          }
          else
          {
            LOG_WARN(("Trap %s configured to fire a weapon, but has none!", TypeName()));
          }
        }

        if (valid)
        {
          // Signal team radio that we were triggered
          if (GetTeam())
          {
            // "Trap::Triggered"
            GetTeam()->GetRadio().Trigger(0xA0FF29A5, Radio::Event(this));
          }

          // Signal the target that it has been trapped
          if (target->GetTeam())
          {
            // "Trap::Triggered::Target"
            target->GetTeam()->GetRadio().Trigger(0x3070E869, Radio::Event(target));
          }

          // Start charge time
          recharge.Start(TrapType()->GetChargeTime());
        }

        // Only affect one target for now
        break;
      }
    }
  }

  return (valid);
}
