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
    // Class ClearSelected
    //

    U32 ClearSelected::orderId;

    //
    // Generate
    //
    void ClearSelected::Generate(Player &player)
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
    U32 ClearSelected::Execute(const U8 *, Player &player)
    {
      // Clear the list of selected objects for this player
      player.ClearSelectedList();

      return (sizeof(Data));
    }
  }
}