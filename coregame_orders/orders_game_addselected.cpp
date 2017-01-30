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
#include "unitobj.h"


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
    // Class AddSelected
    //

    U32 AddSelected::orderId;

    //
    // Generate
    //
    void AddSelected::Generate(Player &player, const UnitObjList &objList)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);

      // Pack the list of object ID's
      UnitObjList::Iterator i(&objList);
      U32 index = 0;

      while (*i)
      {
        UnitObjListNode *obj = *i;

        // Only alive objects should be sent
        if (obj->Alive())
        {
          // Save this object's ID into current slot
          data.idList[index] = (*obj)->Id();

          // Move to next available slot
          index++;
        }

        // Move to the next object
        i++;

        // If array is full or we are at the end of the objects list, fire off order
        if ((*i == NULL) || (index == MAX))
        {
          // Pack the number of unit id's in this order
          ASSERT(index < U8_MAX);
          data.idCount = (U8)index;

          Add(data, sizeof(Data) - (sizeof(U32) * (MAX - index)), player.IsRoute());

          index = 0;
        }
      }
    }


    //
    // Generate
    //
    void AddSelected::Generate(Player &player, UnitObj *obj)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.idList[0] = obj->Id();
      data.idCount = 1;

      Add(data, sizeof(Data) - (sizeof(U32) * (MAX - 1)), player.IsRoute());
    }


    //
    // Execute
    //
    U32 AddSelected::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Extract each object from the list and add it to the player's selected list
      U8 count = d->idCount;
      U32 index = 0;

      ASSERT(count <= MAX);

      // Convert each object id to an object pointer
      while (index < count)
      {
        UnitObj *unitObj = Resolver::Object<UnitObj, UnitObjType>(d->idList[index]);

        if (unitObj)
        {
          // Promotion was successful, add it to the selected objects list
          player.AddToSelectedList(unitObj);
        }

        // Move to next object
        index++;
      }

      return (sizeof(Data) - (sizeof(U32) * (MAX - d->idCount)));
    }

  }

}
  


