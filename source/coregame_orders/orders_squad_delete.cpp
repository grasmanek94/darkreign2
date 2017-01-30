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
    // Class Delete
    //

    U32 Delete::orderId;

    //
    // Generate
    //
    void Delete::Generate(Player &player, U32 squad)
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
    U32 Delete::Execute(const U8 *data, Player &)
    {
      const Data *d = (Data *) data;

      // Resolve the squad
      if (SquadObj * squadObj = Resolver::Object<SquadObj, SquadObjType>(d->squad))
      {
        // Notify the player that the deletion has taken place
        squadObj->NotifyPlayer(0xE145C9A6); // "Squad::Deleted"

        // Delete all of the units in the squad
        for (SquadObj::UnitList::Iterator i(&squadObj->GetList()); *i; i++)
        {
          if ((*i)->Alive())
          {
            GameObjCtrl::MarkForDeletion(**i);
          }
        }
      }
  
      return (sizeof (Data));
    }
  }
}
