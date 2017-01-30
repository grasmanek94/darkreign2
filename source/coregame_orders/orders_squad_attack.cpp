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
#include "tasks_squadattack.h"


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
    // Class Attack
    //

    U32 Attack::orderId;


    //
    // Generate
    //
    void Attack::Generate(Player &player, U32 squad, U32 object, Bool move, Modifier mod)
    {
      Data  data;

      // Setup data structure
      data.Setup(orderId, player);

      // Pack the squad
      data.squad = squad;

      // Set the type
      data.type = Target::OBJECT;

      // Pack the object
      data.object = object;

      // Pack the move flag
      data.move = move;

      // Pack the modifier
      data.mod = mod;

      // Add the order
      Add(data, sizeof(Data), player.IsRoute());
    }


    //
    // Generate
    //
    void Attack::Generate(Player &player, U32 squad, Vector &location, Bool move, Modifier mod)
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

      // Pack the move flag
      data.move = move;

      // Pack the modifier
      data.mod = mod;

      // Add the order
      Add(data, sizeof(Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Attack::Execute(const U8 *data, Player &player)
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
            MapObj *mapObj = Resolver::Object<MapObj, MapObjType>(d->object);
        
            if (mapObj)
            {
              IssueTask(d->mod, squadObj, new Tasks::SquadAttack(squadObj, Target(mapObj)), player, d->move ? 0 : Task::TF_FLAG2);
            }
            break;
          }

          case Target::LOCATION:
          {
            IssueTask(d->mod, squadObj, new Tasks::SquadAttack(squadObj, Target(d->location)), player, d->move ? 0 : Task::TF_FLAG2);
            break;
          }

          default:
            ERR_FATAL(("Unknown Target type %d", d->type))
        }
      }

      return (sizeof (Data));
    }
  }
}
