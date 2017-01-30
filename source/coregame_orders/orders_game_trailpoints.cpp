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
#include "gameobjctrl.h"
#include "resolver.h"
#include "trailobj.h"


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
    // Class TrailPoints
    //

    U32 TrailPoints::orderId;

    //
    // Generate
    //
    void TrailPoints::Generate(Player &player, const TrailObj::WayPointList &list)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);

      // Pack the list of object ID's
      TrailObj::WayPointList::Iterator i(&list);
      U32 index = 0;

      while (*i)
      {
        // Get the waypoint
        TrailObj::WayPoint *p = i++;

        // Save this waypoint
        data.list[index++].Set(p->x, p->z);

        // If array is full or we are at the end of the list, fire off order
        if ((*i == NULL) || (index == MAX))
        {
          // Pack the number of unit id's in this order
          ASSERT(index < U8_MAX);
          data.count = (U8)index;

          // Submit the order
          Add(data, sizeof(Data) - (sizeof(Point<U32>) * (MAX - index)), player.IsRoute());

          // Reset count and continue with remaining points
          index = 0;
        }
      }
    }


    //
    // Execute
    //
    U32 TrailPoints::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      if (TrailObj *trail = player.GetLastTrail())
      {
        U32 index = 0;
        TrailObj::WayPointList list;

        ASSERT(d->count <= MAX)

        while (index < d->count)
        {
          // Get the waypoint
          const Point<U32> &p = d->list[index++];

          // Add to the list
          list.AppendPoint(p.x, p.z);
        }

        // Append the list to the trail
        trail->GetList().AppendList(list);

        // Clear the local list
        list.DisposeAll();
      }

      return (sizeof(Data) - (sizeof(Point<U32>) * (MAX - d->count)));
    }
  }
}
  


