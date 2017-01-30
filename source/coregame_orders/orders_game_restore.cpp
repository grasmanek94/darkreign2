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
#include "resolver.h"


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
    // Class OrdersGame::Restore
    //
    U32 Restore::orderId;


    //
    // Generate
    //
    void Restore::Generate(Player &player, Modifier mod)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.mod = mod;

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Restore::Execute(const U8 *, Player &player)
    {
      // Create an iterator
      for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
      {
        // Tell each of the selected units to restore
        (**i)->RestoreNow();
      }

      return (sizeof (Data));
    }
  }
}