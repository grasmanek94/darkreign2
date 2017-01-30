///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 10-JUL-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "squadobjctrl.h"
#include "gameobjctrl.h"
#include "squadobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace SquadObjCtrl
//
namespace SquadObjCtrl
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // Initialized Flag
  static Bool initialized;

  // Squad type
  static SquadObjTypePtr type;


  //
  // Init
  //
  // Initialize system
  //
  void Init()
  {
    ASSERT(!initialized)

    // Create the type
    GameObjCtrl::AddObjectType(type = new SquadObjType("Squad", NULL));

    initialized = TRUE;
  }


  //
  // Done
  //
  // Shutdown system
  //
  void Done()
  {
    ASSERT(initialized)

    initialized = FALSE;
  }


  //
  // CreateSquad
  //
  // Create a squad
  //
  SquadObj * Create(Team *team)
  {
    ASSERT(initialized)
    ASSERT(type.Alive())

    // Safe Cast
    SquadObj *squad = (SquadObj *) type->NewInstance(0);
    squad->SetTeam(team);
    squad->Equip();

    return (squad);
  }

}
