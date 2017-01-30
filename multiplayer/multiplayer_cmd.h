///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Commands
// 1-JUL-1999
//


#ifndef __MULTIPLAYER_CMD_H
#define __MULTIPLAYER_CMD_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "multiplayer_private.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Cmd
  //
  namespace Cmd
  {

    // Initialization and shutdown
    void Init();
    void Done();

    // Reset after leaving a session
    void Reset();

    // Upload options
    void UploadOptions(const Options *options);

    // Upload filtering
    void UploadFiltering(Bool on);

    // Upload map infomation
    void UploadMapInfo(const char *folder, const char *mission, U32 size, U32 teams);

    // Extract the player name from a string
    Bool ExtractPlayerName(const char *str, U32 len, const char **playerName, const char **rest);

  }

}


#endif