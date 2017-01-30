///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game Orders
//
// 15-NOV-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "orders_game.h"
#include "spyobj.h"
#include "promote.h"
#include "resolver.h"
#include "taskctrl.h"
#include "tasks_spyidle.h"


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
    U32 Morph::orderId;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Morph
    //


    //
    // Generate
    //
    void Morph::Generate(Player &player, U32 target)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.target = target;

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Morph::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Convert ID into a pointer
      if (UnitObj *target = Resolver::Object<UnitObj, UnitObjType>(d->target))
      {
        // Check each selected object
        for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
        { 
          // Is this a spy?
          if (Tasks::SpyIdle *task = TaskCtrl::PromoteIdle<Tasks::SpyIdle>(**i))
          {
            // Flush tasks so idle task can process
            (**i)->FlushTasks();

            // Perform the morph
            task->Morph(target);
          }
        }
      }

      return (sizeof (Data));
    }
  }
}
