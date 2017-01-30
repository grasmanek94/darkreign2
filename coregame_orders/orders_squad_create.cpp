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
#include "squadobjctrl.h"
#include "squadobj.h"


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
    // Class Create
    //

    U32 Create::orderId;

    //
    // Generate
    //
    void Create::Generate(Player &player, U32 id)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);

      // Pack the id
      data.id = id;

      // Add the order
      Add(data, sizeof(Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Create::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Create the squad
      SquadObj *squadObj = SquadObjCtrl::Create(player.GetTeam());
      squadObj->NotifyPlayer(0x8AA808B7, d->id); // "Squad::Created"

      return (sizeof (Data));
    }
  }
}
