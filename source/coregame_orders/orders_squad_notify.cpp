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
    // Class Notify
    //

    U32 Notify::orderId;


    //
    // Generate
    //
    void Notify::Generate(Player &player, U32 squad, U32 message, U32 param)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);

      // Pack the squad
      data.squad = squad;

      // Pack the message
      data.message = message;

      // Pack the data
      data.param = param;

      // Add the order
      Add(data, sizeof(Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Notify::Execute(const U8 *data, Player &)
    {
      const Data *d = (Data *) data;

      // Resolve the squad
      if (SquadObj * squadObj = Resolver::Object<SquadObj, SquadObjType>(d->squad))
      {
        // Send the notification to the player
        squadObj->NotifyPlayer(d->message, d->param);
      }
  
      return (sizeof (Data));
    }
  }
}
