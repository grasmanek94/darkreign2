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
#include "tasks_squadmove.h"
#include "trailobj.h"
#include "taskctrl.h"


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
    // Class Trail
    //

    U32 Trail::orderId;


    //
    // Generate
    //
    void Trail::Generate(Player &player, U32 squad, U32 trail, U32 index, Bool attack, Modifier mod)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);

      // Pack the squad
      data.squad = squad;

      // Pack the trail
      data.trail = trail;

      // Pack the index
      data.index = index;

      // Pack the attack flag
      data.attack = attack;

      // Pack the modifier
      data.mod = mod;

      // Add the order
      Add(data, sizeof(Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Trail::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Resolve the squad
      if (SquadObj * squadObj = Resolver::Object<SquadObj, SquadObjType>(d->squad))
      {
        if (TrailObj *trail = Resolver::Object<TrailObj, TrailObjType>(d->trail))
        {
          IssueTask(d->mod, squadObj, new Tasks::SquadMove(squadObj, trail, d->index), player, d->attack ? Task::TF_FLAG1 : 0);
        }
      }
  
      return (sizeof (Data));
    }
  }
}
