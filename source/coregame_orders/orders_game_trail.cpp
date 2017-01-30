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
#include "trailobj.h"
#include "resolver.h"
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
    // Class Trail - Use an existing trail
    //
    U32 Trail::orderId;

 
    //
    // Create
    //
    void Trail::Create(Player &player, TrailObj::Mode mode)
    {   
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.op = OP_CREATE;
      data.mode = U8(mode);

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Delete
    //
    void Trail::Delete(Player &player, U32 trail)
    {   
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.op = OP_DELETE;
      data.trail = trail;

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Apply
    //
    void Trail::Apply(Player &player, U32 trail, U32 index, Bool attack, Modifier mod)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.op = OP_APPLY;
      data.trail = trail;
      data.index = index;
      data.mod = mod;
      data.attack = attack;

      Add(data, sizeof (Data), player.IsRoute());
    }



    //
    // Execute
    //
    U32 Trail::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      switch (d->op)
      {
        case OP_CREATE:
        {
          player.SetLastTrail
          (
            TrailObj::Create(player.GetTeam(), NULL, TrailObj::Mode(d->mode))
          );
          break;
        }

        case OP_DELETE:
        {
          TrailObj *trail = Resolver::Object<TrailObj, TrailObjType>(d->trail);

          if (trail)
          {
            trail->MarkForDeletion();
          }
          break;
        }
        
        case OP_APPLY:
        {
          TrailObj *trail = Resolver::Object<TrailObj, TrailObjType>(d->trail);

          if (trail)
          {
            for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
            { 
              UnitObj *subject = **i;

              if ((**i)->CanEverMove())
              {
                IssueTask(d->mod, subject, new Tasks::UnitMove(subject, trail, d->index), player, d->attack ? Task::TF_FLAG1 : 0);
              }
            }
          }
          break;
        }
      }

      return (sizeof (Data));
    }
  }
}