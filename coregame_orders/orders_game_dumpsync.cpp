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
#include "sync.h"
#include "main.h"
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

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Internal Data
    //
    U32 DumpSync::orderId;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class DumpSync
    //

    //
    // Generate
    //
    void DumpSync::Generate(U32 gameCycle, Player &player)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.gameCycle = gameCycle;

      Add(data, sizeof (Data));
    }


    //
    // Execute
    //
    U32 DumpSync::Execute(const U8 *data, Player &)
    {
      const Data *d = (Data *) data;

      LOG_DIAG(("Dumping Sync : cycle %d", d->gameCycle))

      // Dump Sync Strings
      Sync::Dump();

      Main::runCodes.Set("QUIT");
 
      return (sizeof (Data));
    }
  }
}