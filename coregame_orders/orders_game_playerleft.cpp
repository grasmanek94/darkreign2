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
    // Class PlayerLeft
    //
    U32 PlayerLeft::orderId;


    //
    // Generate
    //
    void PlayerLeft::Generate(Player &player)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 PlayerLeft::Execute(const U8 *, Player &player)
    {
      Team::PlayerDeparted(&player);

      return (sizeof (Data));
    }
  }
}