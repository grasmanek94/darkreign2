///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Network Game Interface
// 15-JUN-1998
//

#ifndef __MULTIPLAYER_DATA_PRIVATE_H
#define __MULTIPLAYER_DATA_PRIVATE_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "multiplayer_network_private.h"
#include "multiplayer_data.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Data
  //
  namespace Data
  {

    // Session data has arrived
    void SessionData(StyxNet::EventMessage::Data::SessionData *sessionData);

    // Session private data has arrived
    void SessionPrivateData(StyxNet::EventMessage::Data::SessionPrivateData *sessionPrivateData);

    // Session sync data has arrived
    void SessionSyncData(StyxNet::EventMessage::Data::SessionSyncData *sessionSyncData);

    // Reset
    void Reset();

  };

}

#endif