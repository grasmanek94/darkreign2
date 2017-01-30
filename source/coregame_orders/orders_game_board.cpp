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
#include "tasks_unitboard.h"
#include "tasks_unitmove.h"
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
    // Class Board
    //
    U32 Board::orderId;


    //
    // Generate
    //
    void Board::Generate(Player &player, U32 target, Modifier mod)
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
    U32 Board::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Convert ID into a pointer
      if (TransportObj *target = Resolver::Object<TransportObj, TransportObjType>(d->target))
      {
        // Check each selected object
        for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
        { 
          UnitObj *subject = **i;

          // Can this unit type board this transport
          if (target != subject && target->CheckType(subject))
          {
            if (target->UnitType()->CanBoard())
            {
              // Board using entry points
              IssueTask(d->mod, subject, new Tasks::UnitMove(subject, target), player);
            }
            else
            {
              // Board using warping 
              IssueTask(d->mod, subject, new Tasks::UnitBoard(subject, target), player);
            }
          }
        }
      }

      return (sizeof (Data));
    }
  }
}