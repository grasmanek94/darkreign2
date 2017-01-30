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
#include "restoreobj.h"
#include "resolver.h"
#include "taskctrl.h"
#include "tasks_restoremobile.h"


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
    // Class Restore
    //
    U32 RestoreMobile::orderId;


    //
    // Generate
    //
    void RestoreMobile::Generate(Player &player, U32 target, Modifier mod)
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
    U32 RestoreMobile::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Convert ID into a pointer
      if (UnitObj *target = Resolver::Object<UnitObj, UnitObjType>(d->target))
      {
        // Check each selected object
        for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
        { 
          // Promote to a restore object
          RestoreObj *subject = Promote::Object<RestoreObjType, RestoreObj>(**i);

          // Can the subject restore the target
          if (subject && subject->RestoreRequired(target))
          {
            IssueTask(d->mod, subject, new Tasks::RestoreMobile(subject, target), player);
          }
        }
      }

      return (sizeof (Data));
    }
  }
}