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
#include "gameobjctrl.h"


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
    U32 Delete::orderId;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Delete
    //

    //
    // Generate
    //
    void Delete::Generate(Player &player)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Delete::Execute(const U8 *, Player &player)
    {
      // Tell each object to SelfDestruct
      for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
      {
        GameObjCtrl::MarkForDeletion(**i);
      }

      return (sizeof (Data));
    }
  }
}


