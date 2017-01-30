///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Squad Orders
// 26-MAR-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "orders_squad.h"
#include "resolver.h"
#include "squadobj.h"
#include "gameobjctrl.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Orders
//
namespace Orders
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Squad
  //
  namespace Squad
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class AddSelected
    //

    U32 AddSelected::orderId;


    //
    // Generate
    //
    void AddSelected::Generate(Player &player, U32 squad)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.squad = squad;

      // Add the order
      Add(data, sizeof(Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 AddSelected::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Resolve the squad
      if (SquadObj * squadObj = Resolver::Object<SquadObj, SquadObjType>(d->squad))
      {
        // Add this players selected units to the squad
        squadObj->AddUnitObjList(&player.GetSelectedList());

        // Notify the player that units were added to the squad
        squadObj->NotifyPlayer(0x2EC81EFB, d->pid); // "Squad::UnitsAdded"
      }
     
      return (sizeof (Data));
    }
  }
}
