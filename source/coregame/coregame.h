///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 10-JUN-1998
//

#ifndef __COREGAME_H
#define __COREGAME_H


#include "filesys.h"
#include "console.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace CoreGame - Controls all core game systems
//

namespace CoreGame
{

  // Initialise and shutdown various systems
  void Init();
  void Done();

  // Loads the active mission
  Bool OpenMission();

  // Release an open mission if one exists
  void CloseMission();

  // Save currently loaded mission
  Bool SaveMission();

  // Save mission in binary format
  void SetBinarySave();

  // Save mission in text format
  void SetTextSave();

  // Is mission saving in binary
  Bool IsBinarySave();

  // Save the given tree
  Bool WriteTree(PTree &pTree, const char *name);

  // Is the game being simulated
  void SetInSimulation(Bool flag);
  Bool GetInSimulation();
};

#endif