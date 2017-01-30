///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
// 1-JUL-1999
//


#ifndef __MULTIPLAYER_CONTROLS_HOSTCONFIG_H
#define __MULTIPLAYER_CONTROLS_HOSTCONFIG_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "game_missionselection.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Controls
  //
  namespace Controls
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class HostConfig
    //
    class HostConfig : public Game::MissionSelection
    {
    public:

      // Constructor
      HostConfig(IControl *parent);

      // Destructor
      ~HostConfig();

    protected:

      // Notification that a local var has changed value
      void Notify(IFaceVar *var);

    };

  }

}


#endif
