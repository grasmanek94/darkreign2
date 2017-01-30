///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game Orders
// 29-JUL-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "orders_game.h"
#include "formation.h"


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
    U32 Formation::orderId;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Formation
    //


    //
    // Generate
    //
    void Formation::Generate(Player &player, U32 formation, const Vector &location, F32 direction)
    {
      Data  data;

      // Setup data structure
      data.Setup(orderId, player);
      data.formation = formation;
      location.Convert(data.location);
      data.direction = direction;

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Formation::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Apply the formation to the group
      ::Formation::Apply(d->formation, d->location, d->direction, player.GetSelectedList(), Task::TF_FROM_ORDER | Task::TF_FLAG1);

      return (sizeof (Data));
    }
  }
}


