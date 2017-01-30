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
#include "tasks_unitguard.h"


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
    // Class OrdersGame::Guard
    //
    U32 Guard::orderId;


    //
    // Generate
    //
    void Guard::Generate(Player &player, U32 object, Modifier mod)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.type = Target::OBJECT;
      data.object = object;
      data.mod = mod;

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Generate
    //
    void Guard::Generate(Player &player, const Vector &terrainPos, Modifier mod)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.type = Target::LOCATION;
      data.location.x = terrainPos.x;
      data.location.y = terrainPos.y;
      data.location.z = terrainPos.z;
      data.mod = mod;

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Guard::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Create an iterator
      for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
      {
        // Get the unit
        UnitObj *unit = **i;

        // Can it ever guard
        if (Tasks::UnitGuard::CanGuard(unit))
        {
          switch (d->type)
          {
            case Target::OBJECT:
            {
              // Convert ID into a pointer
              if (MapObj *mapObj = Resolver::Object<MapObj, MapObjType>(d->object))
              {
                IssueTask(d->mod, unit, new Tasks::UnitGuard(unit, Target(mapObj)), player);
              }
              break;
            }

            case Target::LOCATION:
            {
              Vector v;
              v.Set(d->location.x, d->location.y, d->location.z);
              IssueTask(d->mod, unit, new Tasks::UnitGuard(unit, Target(v)), player);
              break;
            }
          }
        }
      }

      return (sizeof (Data));
    }
  }
}