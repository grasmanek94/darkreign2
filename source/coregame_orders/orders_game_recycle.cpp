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
#include "taskctrl.h"
#include "tasks_unitrecycle.h"
#include "tasks_unitconstruct.h"


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
    // Class Recycle
    //
    U32 Recycle::orderId;


    //
    // Generate
    //
    void Recycle::Generate(Player &player)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Recycle::Execute(const U8 *, Player &player)
    {
      // Check each selected object
      for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
      { 
        (**i)->RecycleNow();
      }

      return (sizeof (Data));
    }
  }
}