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
#include "tasks_unitexplore.h"


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
    U32 Explore::orderId;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Explore
    //


    //
    // Generate
    //
    void Explore::Generate(Player &player, Modifier mod)
    {
      Data  data;

      // Setup data structure
      data.Setup(orderId, player);
      data.mod = mod;

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Explore::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Create an iterator
      for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; ++i)
      {
        UnitObj *unit = **i;

        if (unit->CanEverMove())
        {
          IssueTask(d->mod, unit, new Tasks::UnitExplore(unit), player, Task::TF_FLAG1);
        }
      }

      return (sizeof (Data));
    }
  }
}


