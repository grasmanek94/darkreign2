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
#include "offmapstrikeobj.h"
#include "stdload.h"
#include "resolver.h"
#include "team.h"
#include "weapon.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Config scope name for type and world files
#define SCOPE_CONFIG   "OffMapStrikeObj"


///////////////////////////////////////////////////////////////////////////////
//
// Class OffMapStrikeObjType - De La OffMapStrike
//


//
// Constructor
//
OffMapStrikeObjType::OffMapStrikeObjType(const char *name, FScope *fScope) : OffMapObjType(name, fScope)
{
  // Get specific config scope
  fScope = fScope->GetFunction(SCOPE_CONFIG);

  // Load config
  weaponName = StdLoad::TypeString(fScope, "Weapon");
  height = StdLoad::TypeF32(fScope, "Height", 20.0F);
}


//
// Destructor
//
OffMapStrikeObjType::~OffMapStrikeObjType()
{
}


//
// PostLoad
//
// Called after all types are loaded
//
void OffMapStrikeObjType::PostLoad()
{
  // Call parent scope first
  OffMapObjType::PostLoad();

  // Resolve the weapon
  weapon = Weapon::Manager::FindType(weaponName);

  if (!weapon)
  {
    ERR_CONFIG(("Could not find weapon '%s' in OffMapStrike '%s'", weaponName.str, typeId.str))
  }

  if (weapon->GetStyle() != Weapon::Style::Projectile)
  {
    ERR_CONFIG(("OffMapStrike '%s' must use a Projectile style weapon, '%s' is not", typeId.str, weaponName.str))
  }
}


//
// NewInstance
//
// Create a new map object instance using this type
//
GameObj* OffMapStrikeObjType::NewInstance(U32 id)
{
  // Allocate new object instance
  return (new OffMapStrikeObj(this, id));
}



///////////////////////////////////////////////////////////////////////////////
//
// Class OffMapStrikeObj - Instance class for above type
//

//
// Constructor
//
OffMapStrikeObj::OffMapStrikeObj(OffMapStrikeObjType *objType, U32 id) : OffMapObj(objType, id)
{
}


//
// Destructor
//
OffMapStrikeObj::~OffMapStrikeObj()
{
}


//
// Execute
//
// Execute an operation (TRUE if accepted)
//
Bool OffMapStrikeObj::Execute(U32 operation, const Vector &pos)
{
  switch (operation)
  {
    case 0x63417A92: // "Trigger::Positional"
    {
      if (GetTeam())
      {
        Matrix m;
        m.ClearData();
        m.Set(Vector(pos.x, pos.y + OffMapStrikeType()->height, pos.z));

        OffMapStrikeType()->weapon->CreateProjectile(m, GetTeam(), Target(pos));

        GetTeam()->GetRadio().Trigger(0x0B886702, Radio::Event(this, pos)); // "OffMapStrike::Executed"
      }

      return (Done());
    }
  }

  return (FALSE);
}
