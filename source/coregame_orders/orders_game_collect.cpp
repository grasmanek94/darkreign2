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
#include "resourceobj.h"
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
    // Class OrdersGame::Collect
    //
    U32 Collect::orderId;


    //
    // Generate
    //
    void Collect::Generate(Player &player, U32 object, Bool search, Modifier mod)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.search = search;
      data.object = object;
      data.mod = mod;

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Collect::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Create an iterator
      for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; ++i)
      {
        UnitObj *unit = **i;

        if (d->search)
        {
          // Is this a collector ?
          if (Tasks::UnitCollect *task = TaskCtrl::PromoteIdle<Tasks::UnitCollect>(unit))
          {
            // Get some resource
            task->Resource();
          }
        }
        else
        {
          // Convert ID into a pointer
          if (ResourceObj *resourceObj = Resolver::Object<ResourceObj, ResourceObjType>(d->object))
          {
            // Is this a collector ?
            if (Tasks::UnitCollect *task = TaskCtrl::PromoteIdle<Tasks::UnitCollect>(unit))
            {
              // Tell this task about the resource object
              task->SetResourceObject(resourceObj);

              // For human controlled units, reset the storage object
              if (!unit->IsAI())
              {
                task->ClearStorageObject();
              }
            }
          }
        }
      }

      return (sizeof (Data));
    }
  }
}