///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game Orders
//
// 22-OCT-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "orders_game.h"
#include "unitobj.h"
#include "promote.h"
#include "taskctrl.h"
#include "tasks_unitmove.h"
#include "movement_pathfollow.h"
#include "weapon.h"


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
    // Internal Data
    //
    U32 Turn::orderId;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Turn
    //


    //
    // Generate
    //
    void Turn::Generate(Player &player, const Vector &terrainPos, Modifier mod)
    {
      Data  data;

      // Setup data structure
      data.Setup(orderId, player);
      data.x = terrainPos.x;
      data.z = terrainPos.z;
      data.mod = mod;

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Turn::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Send the move order to all of the selected objects
      for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
      {
        UnitObj *unit = **i;

        // Calculate desired front vector
        Vector v(d->x - unit->Position().x, 0, d->z - unit->Position().z);
        v.Normalize();

        // Can the unit move
        if (unit->CanEverMove())
        {
          if (!unit->GetDriver()->IsBoarded())
          {
            // Convert the given task Id into a move type
            Tasks::UnitMove *task = new Tasks::UnitMove(unit);
            task->SetDir(v);
            IssueTask(d->mod, unit, task, player);
          }
        }
      }

      return (sizeof (Data));
    }
  }
}


