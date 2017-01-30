///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Line Of Sight System
//
// 24-NOV-1998
//

#ifndef __SIGHT_H
#define __SIGHT_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "unitobjdec.h"
#include "blockfile.h"


///////////////////////////////////////////////////////////////////////////////
//
// Forward declarations
//
class Team;


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Sight - The line of sight system
//
namespace Sight
{

  // Show all units?
  extern Bool showAllUnits;


  // Initialize system
  void Init(Bool editMode);

  // Shutdown system
  void Done();

  // To be called after the mission is loaded
  void NotifyPostMissionLoad();

  // Rescan all line of sight for all units on a team
  void RescanAll(Team *team, Bool force = FALSE);

  // Rescan all units which match the given types
  void RescanAllTypes(const NList<UnitObjType> &types);

  // Process the detached list
  void ProcessDetachedList();

  // Update fog display
  void UpdateDisplay(Team *team, Bool force = FALSE, Bool first = FALSE);

  // Sweep away fog around unit
  // Must call Unsweep between calls to Sweep
  void Sweep(UnitObj *u);

  // Let the fog fall back in around unit
  void UnSweep(UnitObj *u);

  // Detach a sight map from a unit (after it dies)
  void Detach(Map **map);

  // Return the eye position of an object
  F32 EyePosition(UnitObj *u);

  // TRUE iff top left corner of cell (x,y) has been seen by team
  Bool Seen(U32 x, U32 z, Team *team);

  // TRUE iff top left corner of cell (x,y) is currently visible by team
  Bool Visible(U32 x, U32 z, Team *team);

  // TRUE iff any cell in the range is currently visible by team
  Bool Visible(const Area<U32> &area, Team *team);

  // Return seen and visible at once
  void SeenVisible(U32 x, U32 z, Team *team, Bool &seen, Bool &vis);

  // Check if any cells are seen/visible in the given area
  void SeenVisible(const Area<U32> &area, Team *team, Bool &seen, Bool &vis);

  // Return bit mask of teams seeing this cell
  Game::TeamBitfield VisibleMask(U32 x, U32 z);

  // Test if a unit can see map cell x,z
  Bool CanUnitSee(UnitObj *u, U32 x, U32 z);

  // Return the dirty cluster map for a team
  U8 *GetDirtyClusterMap(Team *team);

  // Return maximum seeing range in cells
  U32 MaxRangeCells();

  // Return maximum seeing range in metres
  F32 MaxRangeMetres();

  // Save and load system
  void Save(BlockFile &bFile);
  void Load(BlockFile &bFile);

  // Is all unit display on (for cineractives)
  inline Bool ShowAllUnits()
  {
    return (showAllUnits);
  }

  #ifdef DEVELOPMENT

  // Return debug mode flag
  Bool DebugMode();

  // Reset the terrain display
  void ResetTerrainDisplay();

  // Render debugging info for a unit
  void RenderDebug(UnitObj *u);

  #endif
}

#endif
