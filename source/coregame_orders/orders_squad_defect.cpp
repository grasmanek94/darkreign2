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
#include "mapobjctrl.h"


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
    // Class Defect
    //

    U32 Defect::orderId;


    //
    // Generate
    //
    void Defect::Generate(Player &player, U32 squad, U32 team)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);

      // Pack the squad
      data.squad = squad;

      // Pack the team
      data.team = team;

      // Add the order
      Add(data, sizeof(Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 Defect::Execute(const U8 *data, Player &)
    {
      const Data *d = (Data *) data;

      // Resolve the squad
      if (SquadObj * squadObj = Resolver::Object<SquadObj, SquadObjType>(d->squad))
      {
        // Resolve the team
        Team * team = Team::Id2Team(d->team);

        for (SquadObj::UnitList::Iterator i(&squadObj->GetList()); *i; i++)
        {
          (**i)->SetTeam(team);
        }
      }
  
      return (sizeof (Data));
    }
  }
}
