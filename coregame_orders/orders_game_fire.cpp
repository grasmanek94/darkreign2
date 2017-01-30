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
#include "tasks_unitfire.h"


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
    // Class OrdersGame::Fire
    //
    U32 Fire::orderId;


    //
    // Generate
    //
    void Fire::Generate(Player &player, F32 horizontal, F32 vertical, F32 speed)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.horizontal = horizontal;
      data.vertical = vertical;
      data.speed = speed;

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Fire::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Create an iterator
      for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
      {
        UnitObj *unit = **i;
        
        if (unit->CanEverFire())
        {
          IssueTask(FLUSH, unit, new Tasks::UnitFire(unit, Target(NULL), d->horizontal, d->vertical, d->speed), player);
        }
      }

      return (sizeof (Data));
    }
  }
}