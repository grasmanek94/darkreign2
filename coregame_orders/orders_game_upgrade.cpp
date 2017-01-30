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
#include "resolver.h"
#include "taskctrl.h"
#include "tasks_unitupgrade.h"


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
    // Class Upgrade
    //
    U32 Upgrade::orderId;


    //
    // Generate
    //
    void Upgrade::Generate(Player &player)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Upgrade::Execute(const U8 *, Player &player)
    {
      // Upgrade each selected unit
      for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
      { 
        if ((**i)->CanUpgradeNow())
        {
          (**i)->UpgradeNow();
        }
      }

      return (sizeof (Data));
    }
  }
}