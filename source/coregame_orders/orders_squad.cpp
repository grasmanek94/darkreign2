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

    //
    // Init
    //
    void Init()
    {
      // Register the squad orders
      Create::Register();
      Destroy::Register();
      Delete::Register();
      AddSelected::Register();
      RemoveSelected::Register();
      Empty::Register();
      Stop::Register();
      Move::Register();
      Trail::Register();
      Board::Register();
      FollowTag::Register();
      Formation::Register();
      Restore::Register();
      Attack::Register();
      Guard::Register();
      Tactical::Register();
      Notify::Register();
      Explore::Register();
      Spawn::Register();
      TransportSpawn::Register();
      Defect::Register();
    }

  }

}
