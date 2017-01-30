///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// WON Stuff
//

#ifndef __WON_PRIVATE_H
#define __WON_PRIVATE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "won.h"
#include "console.h"
#include "icontrol.h"
#include "iclistbox.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Won
//
namespace Won
{

  #define CON_WON(x) CONSOLE(0x516DBDE5, x ) // "Won"

  // Control to send WON messages to
  extern IControlPtr wonConnectCtrl;

  // Player list
  extern ICListBoxPtr wonPlayersCtrl;

  // Chat window
  extern IControlPtr wonChatCtrl;

  // Games window
  extern IControlPtr wonGamesCtrl;


  // Abort
  void Abort();

  // Load config
  void LoadConfig();

}

#endif
