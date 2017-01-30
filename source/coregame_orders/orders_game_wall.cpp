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
#include "wallobj.h"
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
    // Class Wall
    //
    U32 Wall::orderId;


    //
    // Generate
    //
    void Wall::Generate(Player &player, U32 target)
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
    U32 Wall::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Convert ID into a pointer
      if (WallObj *target = Resolver::Object<WallObj, WallObjType>(d->target))
      {
        // Check each selected object
        for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
        { 
          WallObj *subject = Promote::Object<WallObjType, WallObj>(**i);

          if (subject && subject != target)
          {
            subject->ToggleLink(target, TRUE, FALSE);
          }
        }
      }

      return (sizeof (Data));
    }
  }
}