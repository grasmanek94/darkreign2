///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game Orders
//
// 28-OCT-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "orders_game.h"
#include "promote.h"
#include "resolver.h"


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
    U32 Constructor::orderId;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Constructor
    //


    //
    // Generate
    //
    void Constructor::Generate(Player &player, U32 id, U32 operation, U32 value)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.id = id;
      data.operation = operation;
      data.value = value;

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Constructor::Execute(const U8 *data, Player &)
    {
      const Data *d = (Data *) data;

      // Convert ID into a pointer
      if (UnitObj *obj = Resolver::Object<UnitObj, UnitObjType>(d->id))
      {
        // Pass order to the object
        obj->PostEvent(Task::Event(d->operation, d->value), TRUE);
      }

      return (sizeof (Data));
    }
  }
}


