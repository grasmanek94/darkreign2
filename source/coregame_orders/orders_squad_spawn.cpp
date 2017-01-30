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
    // Class Spawn
    //

    U32 Spawn::orderId;


    //
    // Generate
    //
    void Spawn::Generate(Player &player, U32 squad, U32 type, const Vector &location, F32 orientation)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);

      // Pack the squad
      data.squad = squad;

      // Pack the type
      data.type = type;

      // Pack the location
      location.Convert(data.location);

      // Pack the orientation
      data.orientation = orientation;

      // Add the order
      Add(data, sizeof(Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Spawn::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Resolve the type
      if (UnitObjType *type = GameObjCtrl::FindType<UnitObjType>(d->type))
      {
        // Add this unit to the squad
        if (SquadObj * squadObj = Resolver::Object<SquadObj, SquadObjType>(d->squad))
        {
          UnitObj &unit = type->Spawn(d->location, player.GetTeam());

          // Quaternion
          unit.SetSimTarget(Quaternion(d->orientation, Matrix::I.Up()));

          squadObj->AddUnitObj(&unit);
          squadObj->NotifyPlayer(0x883F0FF3, unit.Id()); // "Squad::Spawn"
        }
      }
  
      return (sizeof (Data));
    }
  }
}
