///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game Orders
// 29-JUL-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "orders_game.h"
#include "unitobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Orders
//
namespace Orders
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Game
  //
  namespace Game
  {
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Internal Data
    //
    U32 SelfDestruct::orderId;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class SelfDestruct
    //

    //
    // Generate
    //
    void SelfDestruct::Generate(Player &player)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 SelfDestruct::Execute(const U8 *, Player &player)
    {
      // Tell each object to SelfDestruct
      for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
      {
        // Get the unit
        UnitObj *unit = **i;

        // Ensure it can self destruct
        if (unit->HasProperty(0x54D4152A) && !unit->UnderConstruction()) // "Ability::SelfDestruct"
        {
          unit->SelfDestruct(TRUE, unit->GetTeam());
        }
      }

      return (sizeof (Data));
    }
  }
}


