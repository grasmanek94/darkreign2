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
#include "tasks_squadboard.h"
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
    // Class Board
    //

    U32 Board::orderId;


    //
    // Generate
    //
    void Board::Generate(Player &player, U32 squad, Bool attack, Modifier mod)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);

      // Pack the squad
      data.squad = squad;

      // Pack the modifier
      data.mod = mod;

      // Pack the attack flag
      data.attack = attack;

      // Add the order
      Add(data, sizeof(Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Board::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Resolve the squad
      if (SquadObj * squadObj = Resolver::Object<SquadObj, SquadObjType>(d->squad))
      {
        // Get the selected units and pass them to the board task
        TransportObjList transports;

        for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
        {
          if (TransportObj *transport = Promote::Object<TransportObjType, TransportObj>(**i))
          {
            transports.Append(transport);
          }
        }

        // Issue the task for the squad to board the transports
        IssueTask(d->mod, squadObj, new Tasks::SquadBoard(squadObj, transports), player, d->attack ? Task::TF_FLAG1 : 0);

        // Clear out the temp list
        transports.Clear();
      }
  
      return (sizeof (Data));
    }
  }
}
