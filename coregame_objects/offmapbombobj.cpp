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
#include "offmapbombobj.h"
#include "stdload.h"
#include "resolver.h"
#include "terrain.h"
#include "unitobjiter.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Config scope name for type and world files
#define SCOPE_CONFIG   "OffMapBombObj"


///////////////////////////////////////////////////////////////////////////////
//
// Class OffMapBombObjType - De La OffMapBomb
//


//
// Constructor
//
OffMapBombObjType::OffMapBombObjType(const char *name, FScope *fScope) : OffMapObjType(name, fScope)
{
  // Get specific config scope
  fScope = fScope->GetFunction(SCOPE_CONFIG);

  // Load config
  StdLoad::TypeReaperObjType(fScope, "Explosion", explosion);
  height = StdLoad::TypeF32(fScope, "Height", 0.0F);
}


//
// Destructor
//
OffMapBombObjType::~OffMapBombObjType()
{
}


//
// PostLoad
//
// Called after all types are loaded
//
void OffMapBombObjType::PostLoad()
{
  // Call parent scope first
  OffMapObjType::PostLoad();

  Resolver::Type(explosion);
}


//
// NewInstance
//
// Create a new map object instance using this type
//
GameObj* OffMapBombObjType::NewInstance(U32 id)
{
  // Allocate new object instance
  return (new OffMapBombObj(this, id));
}



///////////////////////////////////////////////////////////////////////////////
//
// Class OffMapBombObj - Instance class for above type
//

//
// Constructor
//
OffMapBombObj::OffMapBombObj(OffMapBombObjType *objType, U32 id) : OffMapObj(objType, id)
{
}


//
// Destructor
//
OffMapBombObj::~OffMapBombObj()
{
}


//
// Execute
//
// Execute an operation (TRUE if accepted)
//
Bool OffMapBombObj::Execute(U32 operation, const Vector &pos)
{
  switch (operation)
  {
    case 0x63417A92: // "Trigger::Positional"
    {
      if (GetTeam() && OffMapBombType()->explosion.Alive())
      {
        // Detonate explosion
        Vector p(pos.x, pos.y + OffMapBombType()->height, pos.z);
        OffMapBombType()->explosion->Detonate(p, NULL, GetTeam());
    
        GetTeam()->GetRadio().Trigger(0xF7F0CF04, Radio::Event(this, p)); // "OffMapBomb::Executed"
      }

      return (Done());
    }
  }

  return (FALSE);
}
