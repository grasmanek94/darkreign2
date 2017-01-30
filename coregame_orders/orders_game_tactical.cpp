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
#include "tactical.h"
#include "unitobj.h"
#include "promote.h"


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
    U32 Tactical::orderId;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Tactical
    //


    //
    // Generate
    //
    void Tactical::Generate(Player &player, U8 mIndex, U8 sIndex)
    {
      Data data;

      ASSERT(mIndex < ::Tactical::GetNumModifiers())

      // Setup data structure
      data.Setup(orderId, player);
      data.mIndex = mIndex;
      data.sIndex = sIndex;
      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Tactical::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Iterate over selected objects
      for (UnitObjList::Iterator i(&player.GetSelectedList()); *i; i++)
      {
        // Set the tactical modifier setting of the unit
        (**i)->SetTacticalModifierSetting(d->mIndex, d->sIndex);
      }

      return (sizeof (Data));
    }
  }
}


