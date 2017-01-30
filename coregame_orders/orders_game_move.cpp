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
#include "promote.h"
#include "taskctrl.h"
#include "tasks_unitmove.h"


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
    U32 Move::orderId;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Move
    //


    //
    // Generate
    //
    void Move::Generate(Player &player, const Vector &terrainPos, Bool attack, Modifier mod)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.x = terrainPos.x;
      data.z = terrainPos.z;
      data.mod = mod;
      data.attack = attack;

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Move::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Create an iterator
      for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; ++i)
      {
        UnitObj *unit = **i;

        if (unit->CanEverMove())
        {
          Vector v(d->x, 0.0f, d->z);

          WorldCtrl::ClampMetreMap(v.x, v.z);
          IssueTask(d->mod, unit, new Tasks::UnitMove(unit, v), player, d->attack ? Task::TF_FLAG1 : 0);
        }
      }

      return (sizeof (Data));
    }
  }
}


