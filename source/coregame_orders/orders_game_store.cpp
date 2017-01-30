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
#include "resolver.h"
#include "taskctrl.h"
#include "tasks_unitcollect.h"


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
    // Class OrdersGame::Store
    //
    U32 Store::orderId;


    //
    // Generate
    //
    void Store::Generate(Player &player, U32 object, Bool search, Bool update, Modifier mod)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.search = search;
      data.update = update;
      data.object = object;
      data.mod = mod;

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Store::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Create an iterator
      for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
      {
        // Get the unit
        UnitObj *unit = **i;

        // Is this a collector ?
        if (Tasks::UnitCollect *task = TaskCtrl::PromoteIdle<Tasks::UnitCollect>(unit))
        {
          // Should we search for a place to store the resource
          if (d->search)
          {
            task->Store();
          }
          else
          {
            // Convert ID into a pointer
            if (UnitObj *storeObj = Resolver::Object<UnitObj, UnitObjType>(d->object))
            {
              // If not updating the position, flush tasks incase moving or something
              if (!d->update && unit->GetActiveTask())
              {
                unit->FlushTasks();
              }

              // Tell this task about the new storage point
              task->SetStorageObject(storeObj, d->update);
            }
          }
        }
      }

      return (sizeof (Data));
    }
  }
}