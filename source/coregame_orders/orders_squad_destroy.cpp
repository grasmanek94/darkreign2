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
#include "gameobjctrl.h"
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
    // Class Destroy
    //

    U32 Destroy::orderId;

    //
    // Generate
    //
    void Destroy::Generate(Player &player, U32 squad)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);

      // Pack the squad
      data.squad = squad;

      // Add the order
      Add(data, sizeof(Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Destroy::Execute(const U8 *data, Player &)
    {
      const Data *d = (Data *) data;

      // Resolve the squad
      if (SquadObj * squadObj = Resolver::Object<SquadObj, SquadObjType>(d->squad))
      {
        // Mark it for deletion
        GameObjCtrl::MarkForDeletion(squadObj);
      }

      return (sizeof (Data));
    }
  }
}
