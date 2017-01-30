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
#include "transportobj.h"


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
    // Class TransportSpawn
    //

    U32 TransportSpawn::orderId;


    //
    // Generate
    //
    void TransportSpawn::Generate(Player &player, U32 squad, U32 type, U32 transport)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);

      // Pack the squad
      data.squad = squad;

      // Pack the type
      data.type = type;

      // Pack the transport
      data.transport = transport;

      // Add the order
      Add(data, sizeof(Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 TransportSpawn::Execute(const U8 *data, Player &)
    {
      const Data *d = (Data *) data;

      // Resolve the squad
      if (SquadObj * squadObj = Resolver::Object<SquadObj, SquadObjType>(d->squad))
      {
        // Resolve the type
        if (UnitObjType *type = GameObjCtrl::FindType<UnitObjType>(d->type))
        {
          // Resolve the transport
          if (TransportObj * transportObj = Resolver::Object<TransportObj, TransportObjType>(d->transport))
          {
            UnitObj &unit = type->Spawn(transportObj);
            squadObj->AddUnitObj(&unit);
          }
        }
      }
  
      return (sizeof (Data));
    }
  }
}
