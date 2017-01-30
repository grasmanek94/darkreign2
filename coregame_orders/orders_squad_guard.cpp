///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Squad Orders
// 26-MAR-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "orders_squad.h"
#include "resolver.h"
#include "gameobjctrl.h"
#include "squadobj.h"
#include "tasks_squadguard.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Orders
//
namespace Orders
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Squad
  //
  namespace Squad
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Guard
    //

    U32 Guard::orderId;


    //
    // Generate
    //
    void Guard::Generate(Player &player, U32 squad, U32 object, Modifier mod)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);

      // Pack the squad
      data.squad = squad;

      // Set the type
      data.type = Target::OBJECT;

      // Pack the object
      data.object = object;

      // Pack the modifier
      data.mod = mod;

      // Add the order
      Add(data, sizeof(Data), player.IsRoute());
    }


    //
    // Generate
    //
    void Guard::Generate(Player &player, U32 squad, Vector &location, Modifier mod)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);

      // Pack the squad
      data.squad = squad;

      // Set the type
      data.type = Target::LOCATION;

      // Pack the location
      location.Convert(data.location);

      // Pack the modifier
      data.mod = mod;

      // Add the order
      Add(data, sizeof(Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Guard::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Resolve the squad
      if (SquadObj * squadObj = Resolver::Object<SquadObj, SquadObjType>(d->squad))
      {
        switch (d->type)
        {
          case Target::OBJECT:
          {
            // Convert ID into a pointer
            if (MapObj *mapObj = Resolver::Object<MapObj, MapObjType>(d->object))
            {
              IssueTask(d->mod, squadObj, new Tasks::SquadGuard(squadObj, Target(mapObj)), player);
            }
            break;
          }

          case Target::LOCATION:
          {
            IssueTask(d->mod, squadObj, new Tasks::SquadGuard(squadObj, Target(d->location)), player);
            break;
          }
        }
      }

      return (sizeof (Data));
    }
  }
}
