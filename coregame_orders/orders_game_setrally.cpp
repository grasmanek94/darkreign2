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
    U32 SetRally::orderId;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class SetRally
    //


    //
    // Generate
    //
    void SetRally::Generate(Player &player, U32 x, U32 z)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.x = x;
      data.z = z;

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 SetRally::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Create an iterator
      for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
      {
        (**i)->SetRallyPoint(Point<U32>(d->x, d->z));
      }

      return (sizeof (Data));
    }
  }
}


