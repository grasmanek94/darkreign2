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
#include "resolver.h"
#include "taskctrl.h"
#include "tasks_unitattack.h"
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
    // Class OrdersGame::Attack
    //
    U32 Attack::orderId;


    //
    // Generate
    //
    void Attack::Generate(Player &player, U32 object, Bool move, Modifier mod)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.type = Target::OBJECT;
      data.move = move;
      data.object = object;
      data.mod = mod;

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Generate
    //
    void Attack::Generate(Player &player, const Vector &location, Bool move, Modifier mod)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.type = Target::LOCATION;
      data.move = move;
      location.Convert(data.location);
      data.move = move;
      data.mod = mod;

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Attack::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Create an iterator
      for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
      {
        // Get the unit
        UnitObj *unit = **i;

        if (unit->CanEverFire())
        {
          // If no ammo, try and restore
          if (unit->GetWeapon() && !unit->GetWeapon()->HaveAmmunition())
          {
            unit->RestoreNow();
          }
          else
          {
            switch (d->type)
            {
              case Target::OBJECT:
              {
                // Convert ID into a pointer
                if (MapObj *mapObj = Resolver::Object<MapObj, MapObjType>(d->object))
                {
                  // Can we ever damage this object
                  if ((mapObj != unit) && !unit->ForceAttacking(mapObj) && unit->CanDamageNow(mapObj))
                  {
                    IssueTask(d->mod, unit, new Tasks::UnitAttack(unit, Target(mapObj)), player, d->move ? 0 : Task::TF_FLAG2);

                    // Notify the object that it's being attacked
                    mapObj->SendEvent(Task::Event(0xF874D787)); // "AttackTarget"
                  }
                }
                break;
              }

              case Target::LOCATION:
              {
                Vector v;
                v.Set(d->location.x, d->location.y, d->location.z);
                IssueTask(d->mod, unit, new Tasks::UnitAttack(unit, Target(v)), player, d->move ? 0 : Task::TF_FLAG2);
                break;
              }
            }
          }
        }
      }

      return (sizeof (Data));
    }
  }
}