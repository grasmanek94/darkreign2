///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// De La Game
//
// 12-AUG-1998
//


#ifndef __GAME_H
#define __GAME_H


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Game
//

namespace Game
{
  // Initialize and shutdown
  void Init();
  void Done();

  // Pre-Open an existing mission or new mission
  void PreOpenMission(const char *missionExecFile = NULL);

  // Post Close a mission
  void PostCloseMission();

  // Open an existing or new mission
  void OpenMission();

  // Close an open mission if one exists
  void CloseMission();

  // Save currently loaded mission (use NULL arguments to save as current name)
  Bool SaveMission(const char *path = NULL, const char *mission= NULL);

  // Is a mission currenly preloaded
  Bool MissionPreLoaded();

  // Is a mission currently loaded
  Bool MissionLoaded();

  // Is the mission online ?
  Bool MissionOnline();

  // Invalidate old load errors 
  void ClearLoadErrors();

  // Process a type file
  void ProcessTypeFile(const char *fileName);

};

#endif

