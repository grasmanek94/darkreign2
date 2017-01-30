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
#include "mapobj.h"


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

    //
    // Init
    //
    void Init()
    {
      // Register the game orders
      Move::Register();
      Explore::Register();
      Return::Register();
      Turn::Register();
      Stop::Register();
      Delete::Register();
      Attack::Register();
      Guard::Register();
      Collect::Register();
      Store::Register();
      AddSelected::Register();
      ClearSelected::Register();
      Pause::Register();
      StepOnce::Register();
      DumpSync::Register();
      Tactical::Register();
      Build::Register();
      Formation::Register();
      Constructor::Register();
      OffMap::Register();
      Restore::Register();
      SetRestore::Register();
      Fire::Register();
      RestoreMobile::Register();
      RestoreStatic::Register();
      SelfDestruct::Register();
      Board::Register();
      Transport::Register();
      Unload::Register();
      Recycle::Register();
      PowerDown::Register();
      Upgrade::Register();
      Trail::Register();
      TrailPoints::Register();
      SetRally::Register();
      Wall::Register();
      Infiltrate::Register();
      Morph::Register();
      EjectSpy::Register();
      RevealSpy::Register();
      Scatter::Register();
      GiveUnits::Register();
      GiveResource::Register();
      PlayerMarker::Register();
      Debug::Register();
      PostEvent::Register();
      PlayerLeft::Register();
    }
  }
}
