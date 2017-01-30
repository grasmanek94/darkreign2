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
#include "tasks_squadfollowtag.h"


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
    // Class FollowTag
    //

    U32 FollowTag::orderId;


    //
    // Generate
    //
    void FollowTag::Generate(Player &player, U32 squad, U32 tag, Bool attack, Modifier mod)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);

      // Pack the squad
      data.squad = squad;

      // Pack the tag
      data.tag = tag;

      // Pack the attack flag
      data.attack = attack;

      // Pack the modifier
      data.mod = mod;

      // Add the order
      Add(data, sizeof(Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 FollowTag::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Resolve the squad
      if (SquadObj * squadObj = Resolver::Object<SquadObj, SquadObjType>(d->squad))
      {
        // Resolve the tag
        if (TagObj * tagObj = Resolver::Object<TagObj, TagObjType>(d->tag))
        {
          IssueTask(d->mod, squadObj, new Tasks::SquadFollowTag(squadObj, tagObj), player, d->attack ? Task::TF_FLAG1 : 0);
        }
      }
  
      return (sizeof (Data));
    }
  }
}
