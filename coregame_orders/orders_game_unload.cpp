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
#include "tasks_transportpad.h"
#include "tasks_transportunload.h"


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
    U32 Unload::orderId;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Unload
    //


    //
    // Generate
    //
    void Unload::Generate(Player &player, const Vector &location, Modifier mod)
    {
      Data  data;

      // Setup data structure
      data.Setup(orderId, player);
      data.x = location.x;
      data.z = location.z;
      data.mod = mod;

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Unload::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Get the metre position
      Vector location(d->x, TerrainData::FindFloor(d->x, d->z), d->z);

      // Create an iterator
      for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
      {
        // Is this a transport
        if (TransportObj *t = Promote::Object<TransportObjType, TransportObj>(**i))
        {
          // Can it unload right now
          if (t->UnloadAvailable())
          {
            // Is this a telepad
            if (Tasks::TransportPad *pad = TaskCtrl::PromoteIdle<Tasks::TransportPad>(t))
            {
              // Send an event to the task telling it to unload
              pad->SetUnloadDestination(location);
            }
            else
            {
              // Move to location and unload
              IssueTask(d->mod, t, new Tasks::TransportUnload(t, location), player);
            }
          }
        }
      }

      return (sizeof (Data));
    }
  }
}


