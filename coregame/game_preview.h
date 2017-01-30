///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game Preview Loading and Saving
//
// 14-AUG-1998
//

#ifndef __GAME_PREVIEW_H
#define __GAME_PREVIEW_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "bitmap.h"
#include "gameconstants.h"
#include "missions.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Game
//
namespace Game
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Preview
  //
  class Preview
  {
  public:

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class TeamInfo - Contains preview information for a team
    //
    class TeamInfo
    {
    public:

      // Tree node
      NBinTree<TeamInfo>::Node node;

    protected:

      // Team names
      GameIdent name;

      // Identifier
      U32 teamId;

      // Start location
      Point<F32> startPoint;

      // Is the side fixed
      Bool sideFixed;

      // The fixed side
      GameIdent side;

    public:

      // Constructor
      TeamInfo(FScope *scope);

      // Get the name of the team
      const GameIdent & GetName()
      {
        return (name);
      }

      // Get the team id
      U32 GetTeamId()
      {
        return (teamId);
      }

      // Get the starting point
      const Point<F32> & GetStartPoint()
      {
        return (startPoint);
      }

      // Is the side fixed
      Bool GetSideFixed()
      {
        return (sideFixed);
      }

      // Get the side (only valid if its fixed!)
      const GameIdent & GetSide()
      {
        return (side);
      }

    };

  private:

    // Bitmap of the terrain
    Bitmap *terrain;

    // Size of the map
    U32 size;

    // List of teams on map
    NBinTree<TeamInfo> teams;

    // Location of current mission
    PathIdent path;
    FileIdent name;

    // Default ruleset
    GameIdent ruleSet;

    // Is the ruleset fixed
    Bool ruleSetFixed;


    // Open mission stream
    const char *OpenMissionStream();

    // Close mission stream
    void CloseMissionStream(const char *oldStream);

    // Load textures
    void LoadTextures();

    // Release textures
    void ReleaseTextures();

  public:

    // Constructor and destructor
    Preview(const Missions::Mission *mission);
    ~Preview();

    // Reload textures on mode change
    void ReloadTextures();

    // Get the mission
    const Missions::Mission * GetMission();

    // Get the minimap bitmap
    Bitmap * GetTerrainTexture()
    {
      return (terrain);
    }

    // Get the size of the map
    U32 GetSize()
    {
      return (size);
    }

    // Get the number of teams on the map
    U32 GetTeams()
    {
      return (teams.GetCount());
    }

    // Find a team by name
    TeamInfo *FindTeamCrc(U32 nameCrc)
    {
      return (teams.Find(nameCrc));
    }

    // Find a team by id
    TeamInfo *FindTeamId(U32 id);

    // Get the ruleset
    const GameIdent & GetRuleSet()
    {
      return (ruleSet);
    }

    // Get the fixed status of the ruleset
    Bool IsRuleSetFixed()
    {
      return (ruleSetFixed);
    }

  };

}

#endif
