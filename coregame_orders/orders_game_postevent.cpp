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
    // Class PostEvent
    //
    U32 PostEvent::orderId;


    //
    // Generate
    //
    void PostEvent::Generate(Player &player, U32 event, U32 param1, U32 param2)
    {
      Data data;
     
      // Setup data structure
      data.Setup(orderId, player);
      data.event = event;
      data.param1 = param1;
      data.param2 = param2;

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 PostEvent::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Create the event
      Task::Event event(d->event, d->param1, d->param2);

      // Send event to each selected object
      for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
      {
        (**i)->PostEvent(event);
      }

      return (sizeof (Data));
    }
  }
}