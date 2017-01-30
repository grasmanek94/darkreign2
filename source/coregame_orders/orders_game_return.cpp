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
#include "promote.h"
#include "taskctrl.h"
#include "tasks_unitmove.h"


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
    U32 Return::orderId;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Return
    //


    //
    // Generate
    //
    void Return::Generate(Player &player, Modifier mod)
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
    U32 Return::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Send the move order to all of the selected objects
      for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
      {
        if ((**i)->CanEverMove())
        {
          // To return, move to where you are
          IssueTask(d->mod, **i, new Tasks::UnitMove(**i, (**i)->WorldMatrix().Position()), player);
        }
      }

      return (sizeof (Data));
    }
  }
}


