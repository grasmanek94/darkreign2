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
    U32 EjectSpy::orderId;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class EjectSpy
    //


    //
    // Generate
    //
    void EjectSpy::Generate(Player &player)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 EjectSpy::Execute(const U8 *, Player &player)
    {
      // Eject spies from all selected buildings
      for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
      {
        UnitObj *obj = **i;

        // Check for all spies that are in this building
        for (NList<SpyObj>::Iterator j(&SpyObj::allSpies); *j; j++)
        { 
          Tasks::SpyIdle *task = TaskCtrl::PromoteIdle<Tasks::SpyIdle>(*j);

          if (task && task->IsInfiltrating(obj))
          {
            // Eject spy from building
            task->Eject();
          }
        }
      }

      return (sizeof (Data));
    }
  }
}
