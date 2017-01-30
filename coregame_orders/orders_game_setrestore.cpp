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
#include "restoreobj.h"
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
    // Class OrdersGame::SetRestore
    //
    U32 SetRestore::orderId;


    //
    // Generate
    //
    void SetRestore::Generate(Player &player, U32 restorer, Modifier mod)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.restorer = restorer;
      data.mod = mod;

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 SetRestore::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Resolve the restorer
      if (RestoreObj *r = d->restorer ? Resolver::Object<RestoreObj, RestoreObjType>(d->restorer) : NULL)
      {
        for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
        {
          if (r->CanRestore(**i))
          {
            // Can this object be restored by the target
            (**i)->SetPrimaryRestore(r);
          }
        }
      }

      return (sizeof (Data));
    }
  }
}