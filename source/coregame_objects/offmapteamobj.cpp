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
#include "offmapteamobj.h"
#include "stdload.h"
#include "resolver.h"
#include "team.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Config scope name for type and world files
#define SCOPE_CONFIG   "OffMapTeamObj"


///////////////////////////////////////////////////////////////////////////////
//
// Class OffMapTeamObjType - De La OffMapTeam
//


//
// Constructor
//
OffMapTeamObjType::OffMapTeamObjType(const char *name, FScope *fScope) : OffMapObjType(name, fScope)
{
  // All objects of this type are triggered immediately
  AddProperty("OffMap::Immediate");

  // Get specific config scope
  fScope = fScope->GetFunction(SCOPE_CONFIG);

  // Load the modifiers
  modifiers.Load(fScope->GetFunction("Modifiers"));
}


//
// Destructor
//
OffMapTeamObjType::~OffMapTeamObjType()
{
  // Delete instant modifiers
  modifiers.DisposeAll();
}


//
// PostLoad
//
// Called after all types are loaded
//
void OffMapTeamObjType::PostLoad()
{
  // Call parent scope first
  OffMapObjType::PostLoad();

}


//
// NewInstance
//
// Create a new map object instance using this type
//
GameObj* OffMapTeamObjType::NewInstance(U32 id)
{
  // Allocate new object instance
  return (new OffMapTeamObj(this, id));
}



///////////////////////////////////////////////////////////////////////////////
//
// Class OffMapTeamObj - Instance class for above type
//

//
// Constructor
//
OffMapTeamObj::OffMapTeamObj(OffMapTeamObjType *objType, U32 id) : OffMapObj(objType, id)
{
}


//
// Destructor
//
OffMapTeamObj::~OffMapTeamObj()
{
}


//
// Execute
//
// Execute an operation (TRUE if accepted)
//
Bool OffMapTeamObj::Execute(U32 operation, const Vector &)
{
  switch (operation)
  {
    case 0x9B29F069: // "Trigger::Immediate"
    {
      if (GetTeam())
      {
        // Apply the modifiers to each object on this team
        for (NList<UnitObj>::Iterator i(&GetTeam()->GetUnitObjects()); *i; i++)
        {
          if ((*i)->OnMap())
          {
            OffMapTeamType()->modifiers.Apply(*i);
          }
        }
       
        GetTeam()->GetRadio().Trigger(0x971FE8D0, Radio::Event(this)); // "OffMapTeam::Executed"
      }

      return (Done());
    }
  }

  return (FALSE);
}
