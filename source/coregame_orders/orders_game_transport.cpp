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
#include "transportobj.h"
#include "resolver.h"
#include "taskctrl.h"
#include "tasks_unitmove.h"
#include "tasks_unitboard.h"
#include "tasks_transportpad.h"


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
    // Class Transport
    //
    U32 Transport::orderId;


    //
    // Generate
    //
    void Transport::Generate(Player &player, U32 target, Modifier mod)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.target = target;
      data.mod = mod;

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Transport::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Should never be more than one unit in the selected list
      UnitObj *u = player.GetSelectedList().GetFirst();

      // Promote to a transport
      if (TransportObj *transport = u ? Promote::Object<TransportObjType, TransportObj>(u) : NULL)
      {
        // Convert ID into a pointer
        if (UnitObj *target = Resolver::Object<UnitObj, UnitObjType>(d->target))
        {
          // Do we have a valid passenger
          if (target != transport && transport->CheckCargo(target))
          {
            // Is this a telepad
            if (TaskCtrl::Promote<Tasks::TransportPad>(transport->GetIdleTask()))
            {
              // Tell the target to board the transport using exit points
              IssueTask(d->mod, target, new Tasks::UnitMove(target, transport), player);
            }
            else
            {
              // Tell the transport to move closer
              IssueTask(Orders::FLUSH, transport, new Tasks::UnitMove(transport, target->Position()), player);

              // Tell the target to board the transport
              IssueTask(d->mod, target, new Tasks::UnitBoard(target, transport), player);
            }
          }
        }
      }

      return (sizeof (Data));
    }
  }
}