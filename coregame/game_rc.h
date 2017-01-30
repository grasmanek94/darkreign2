///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game RunCodes
//
// 12-AUG-1998
//

#ifndef __GAME_RUNCODES_H
#define __GAME_RUNCODES_H

///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "runcodes.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Game
//
namespace Game
{

	///////////////////////////////////////////////////////////////////////////////
	//
	// NameSpace RC
	//
  namespace RC
  {
    // Initialize GameRunCodes system
    void Init();

    // Shutdown GameRunCodes system
    void Done();

    // Set the RunCode
    void Set(const char *);

    // Check the runcode
    Bool In(const char *);

    // Clear the RunCode
    void Clear();

    // Process the RunCode
    void Process();

    // Is the current RunCode NULL ?
    Bool IsClear();

    // Get average framerate of the game
    F32 AvgFrameRate();

    // Get average display framerate of the game
    F32 AvgDisplayRate();

    // Get average triangle count
    U32 AvgTriCount();

    // Get total triangle count
    U32 TriCount();

    // Reset framerate
    void ResetFrameRate();
  }
}


#endif
