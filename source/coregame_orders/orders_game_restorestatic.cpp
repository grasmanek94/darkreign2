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
    U32 RestoreStatic::orderId;


    //
    // Generate
    //
    void RestoreStatic::Generate(Player &player, U32 target, Modifier mod)
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
    U32 RestoreStatic::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Convert ID into a pointer
      RestoreObj *target = Resolver::Object<RestoreObj, RestoreObjType>(d->target);

      if (target && target->IsStatic())
      {
        // Check each selected object
        for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
        { 
          // Get the unit
          UnitObj *unit = **i;

          // Can the target restore this unit
          if (target->RestoreRequired(unit))
          {
            // Set it as the primary restore facility
            unit->SetPrimaryRestore(target);

            // Tell the unit to restore now
            unit->RestoreNow();
          }
        }
      }

      return (sizeof (Data));
    }
  }
}