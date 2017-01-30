///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
// 1-JUL-1999
//

#ifndef __MULTIPLAYER_H
#define __MULTIPLAYER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//

#include "fscope.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{
  LOGEXTERN

  // PreInitialize multiplayer
  void PreInit();

  // Initialize multiplayer
  void Init();

  // Shutdown multiplayer
  void Done();

  // Terminate multiplayer
  void Terminate();

  // Process multiplayer
  void Process();

  // Reset multiplayer
  void Reset();

  // Is multiplayer ready to enter the game
  Bool IsReady();

  // Set the readyness of this player
  void SetReady();

  // Clear the readyness of this player
  void ClearReady();

  // Apply multiplayer data setup to the mission
  void SetupPreMission();

  // Apply multiplayer data setup to the mission
  void SetupMission();

  // Apply multiplayer data setup to the mission
  void SetupPostMission();

  // Save and load multiplayer system
  void Save(FScope *scope);
  void Load(FScope *scope);

  // Is this the host
  Bool IsHost();

  // Get data crc
  U32 GetDataCrc();

  // Set data crc
  void SetDataCrc(U32 crc);

}


#endif
