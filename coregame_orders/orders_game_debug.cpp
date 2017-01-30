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
#include "team.h"


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
    U32 Debug::orderId;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Debug
    //

    //
    // Generate
    //
    void Debug::Generate(Player &player, U32 type, U32 id, U32 u, F32 f)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.type = type;
      data.id = id;
      data.u = u;
      data.f = f;
      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Debug::Execute(const U8 *data, Player &player)
    {
      #ifdef DEVELOPMENT

        const Data *d = (Data *) data;

        // Convert ID into a pointer
        MapObj *object = d->id ? Resolver::Object<MapObj, MapObjType>(d->id) : NULL;

        // Promote to a unit
        UnitObj *unit = object ? Promote::Object<UnitObjType, UnitObj>(object) : NULL;

        // Type based debugging
        switch (d->type)
        {
          case 0x94EDF0DD: // "resourceobj"
          {
            if (player.GetTeam())
            {
              player.GetTeam()->AddResourceStore(d->u);
              player.GetTeam()->ReportResource(d->u, "resource.development");
            }
            break;
          }

          case 0x9FEA8AAD: // "weaponobj"
          {
            if (unit && unit->GetTeam())
            {
              unit->GetTeam()->SetDifficulty(d->f);
            }
            break;
          }

          case 0x485FC1BB: // "restoreobj"
          {
            for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
            {
              (**i)->ModifyHitPoints(1000);
              (**i)->ReloadAmmunition(1000);
            }

            if (unit)
            {
              unit->ModifyHitPoints(1000);
              unit->ReloadAmmunition(1000);
            }
            break;
          }

          case 0x962FE484: // "explosionobj"
          {
            if (object)
            {
              object->SelfDestruct(FALSE, player.GetTeam());
            }
            break;
          }

          case 0x322EFA23: // "immediate"
          {
            for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
            {
              (**i)->SendEvent(Task::Event(0x834E6A12)); // "Debug::Immediate"
            }
          }
        }

      #else

        data;
        player;

      #endif

      return (sizeof (Data));
    }
  }
}


