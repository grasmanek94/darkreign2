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
#include "offmapobj.h"
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
    U32 OffMap::orderId;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class OffMap
    //


    //
    // Generate
    //
    void OffMap::Generate(Player &player, U32 id, U32 operation, const Vector *pos)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.id = id;
      data.operation = operation;

      if (pos)
      {
        data.pos = *pos;
      }
      else
      {
        data.pos.ClearData();
      }

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 OffMap::Execute(const U8 *data, Player &)
    {
      const Data *d = (Data *) data;

      // Convert ID into a pointer
      if (OffMapObj *obj = Resolver::Object<OffMapObj, OffMapObjType>(d->id))
      {
        if (!obj->Executed() && obj->Check(d->pos))
        {
          // Pass order to the object
          obj->Execute(d->operation, d->pos);
        }
      }

      return (sizeof (Data));
    }
  }
}


