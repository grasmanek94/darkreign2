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
    // Class RemoveSelected
    //

    U32 RemoveSelected::orderId;


    //
    // Generate
    //
    void RemoveSelected::Generate(Player &player)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);

      // Add the order
      Add(data, sizeof(Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 RemoveSelected::Execute(const U8 *, Player &player)
    {
      for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
      {
        // Get this unit
        UnitObj *unit = **i;

        // Remove from its squad
        if (unit->GetSquad())
        {
          unit->ClearSquad();
        }
      }
     
      return (sizeof (Data));
    }
  }
}
