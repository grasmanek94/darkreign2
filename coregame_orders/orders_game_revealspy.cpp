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
    U32 RevealSpy::orderId;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class RevealSpy
    //


    //
    // Generate
    //
    void RevealSpy::Generate(Player &player, U32 id)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.id = id;

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 RevealSpy::Execute(const U8 *data, Player &)
    {
      const Data *d = (Data *) data;

      // Convert ID into a pointer
      if (UnitObj *unit = Resolver::Object<UnitObj, SpyObjType>(d->id))
      {
        // Notify the object that it's being attacked (this demorphs spies)
        unit->SendEvent(Task::Event(0xF874D787)); // "AttackTarget"
      }

      return (sizeof (Data));
    }
  }
}
