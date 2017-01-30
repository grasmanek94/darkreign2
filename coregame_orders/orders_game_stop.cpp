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
    U32 Stop::orderId;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Stop
    //

    //
    // Generate
    //
    void Stop::Generate(Player &player)
    {
      Data data;
      
      // Setup data structure
      data.Setup(orderId, player);

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Stop::Execute(const U8 *, Player &player)
    {
      // Tell each object to stop
      for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
      {
        // Stop movement
        (**i)->Stop();
      }

      return (sizeof (Data));
    }
  }
}


