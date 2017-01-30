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
#include "gametime.h"
#include "mapobj.h"


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
    // Class Pause
    //

    U32 Pause::orderId;

    //
    // Generate
    //
    void Pause::Generate(Player &player)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);

      // Add the order
      Add(data, sizeof(Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Pause::Execute(const U8 *, Player &)
    {
      // Toggle the pause in gametime
      GameTime::Pause();

      return (sizeof(Data));
    }
  }
}
