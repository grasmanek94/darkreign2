///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Game Level Sound Control
//
// 28-JAN-2000
//


#ifndef __GAMESOUND_H
#define __GAMESOUND_H

///////////////////////////////////////////////////////////////////////////////
//
// Includes
//

#include "trackplayer.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace GameSound
//
namespace GameSound
{
  // Initialize and shutdown system
  void Init();
  void Done();

  // Get the track player
  TrackPlayer & GetPlayer();

  // Reset gamesound state
  void Reset();

  // Polling for sound events
  void Poll();
};

#endif