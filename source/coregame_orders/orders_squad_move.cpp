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
    // Class Move
    //

    U32 Move::orderId;


    //
    // Generate
    //
    void Move::Generate(Player &player, U32 squad, const Vector &destination, Bool attack, Bool turn, Modifier mod)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);

      // Pack the squad
      data.squad = squad;

      // Pack the flags
      data.attack = attack;
      data.turn = turn;

      // Pack the destination
      destination.Convert(data.destination);

      // Pack the modifier
      data.mod = mod;

      // Add the order
      Add(data, sizeof(Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Move::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Resolve the squad
      if (SquadObj * squadObj = Resolver::Object<SquadObj, SquadObjType>(d->squad))
      {
        U32 flags = (d->attack ? Task::TF_FLAG1 : 0) | (d->turn ? Task::TF_FLAG3 : 0);
        IssueTask(d->mod, squadObj, new Tasks::SquadMove(squadObj, d->destination), player, flags);
      }
  
      return (sizeof (Data));
    }
  }
}
