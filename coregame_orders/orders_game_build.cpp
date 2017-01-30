///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game Orders
//
// 28-OCT-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "orders_game.h"
#include "tasks_unitbuild.h"
#include "resolver.h"
#include "wallobj.h"


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
    U32 Build::orderId;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Build
    //


    //
    // Generate
    //
    void Build::Generate(Player &player, MapObjType *build, const Vector &d, WorldCtrl::CompassDir dir, Modifier mod)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.build = build->GetNameCrc();
      data.dest = d;
      data.dir = dir;
      data.mod = mod;

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Build::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Should never be more than one unit in the selected list
      if (UnitObj *unitObj = player.GetSelectedList().GetFirst())
      {
        // Ensure unit can move (sync filter)
        if (unitObj->CanEverMove())
        {
          // Get the type to build
          if (UnitObjType *build = GameObjCtrl::FindType<UnitObjType>(d->build))
          {
            // Setup the destination
            Vector dest(d->dest.x, TerrainData::FindFloorWithWater(d->dest.x, d->dest.z), d->dest.z);

            // Send the task
            IssueTask(d->mod, unitObj, new Tasks::UnitBuild(unitObj, build, dest, d->dir), player);
          }
        }
      }

      return (sizeof (Data));
    }
  }
}


