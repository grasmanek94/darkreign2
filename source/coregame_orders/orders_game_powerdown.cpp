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
#include "resolver.h"
#include "taskctrl.h"
#include "tasks_unitpowerdown.h"


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
    // Class PowerDown
    //
    U32 PowerDown::orderId;


    //
    // Generate
    //
    void PowerDown::Generate(Player &player, Bool down)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.down = down;

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 PowerDown::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Are we powering down or up
      if (d->down)
      {
        // Check each selected object
        for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
        { 
          UnitObj *subject = **i;

          // Can this unit power down ?
          if (subject->UnitType()->GetPower().GetRequired())
          {
            IssueTask(FLUSH, subject, new Tasks::UnitPowerDown(subject), player);
          }
        }
      }
      else
      {
        // Check each selected object
        for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
        { 
          UnitObj *subject = **i;

          // Is this unit powered down ?
          if (Tasks::UnitPowerDown *task = TaskCtrl::Promote<Tasks::UnitPowerDown>(subject))
          {
            task->ProcessEvent(Task::Event(0x57BE223A)); // "PowerUp"
          }
        }
      }

      return (sizeof (Data));
    }
  }
}