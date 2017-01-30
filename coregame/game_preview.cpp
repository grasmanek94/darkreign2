///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game Preview Loading and Saving
//
// 14-AUG-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "game_preview.h"
#include "game.h"
#include "sides.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
#define GAME_PREVIEW_STREAM "Preview"
#define GAME_PREVIEW_BITMAP "terrain.bmp"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Game
//
namespace Game
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class TeamInfo - Contains preview information for a team
  //

  //
  // Constructor
  //
  Preview::TeamInfo::TeamInfo(FScope *scope)
  {
    // The team name
    name = StdLoad::TypeString(scope);

    // The team id
    teamId = StdLoad::TypeU32(scope);

    // Start point
    StdLoad::TypePoint<F32>
    (
      scope, "StartPoint", startPoint, 
      Point<F32>((teamId + 1) * 0.1f, (teamId + 1) * 0.1f)
    );

    // Is the side fixed
    sideFixed = StdLoad::TypeU32(scope, "SideFixed", TRUE);

    // Get the side
    side = StdLoad::TypeString(scope, "Side", Sides::GetSide().GetName().str);
  }


  //////////////////////////////////////////////////////////////////////////////
  //
  // Class Preview
  //

  //
  // Preview::Preview
  //
  Preview::Preview(const Missions::Mission *missionIn) 
  : teams(&TeamInfo::node),
    terrain(NULL),
    ruleSetFixed(FALSE)
  {
    if (missionIn)
    {
      name = missionIn->GetName().str;
      path = missionIn->GetGroup().GetPath().str;
    }
    else
    {
      name = "";
      path = "";
    }

    // Open mission stream
    const char *oldStream = OpenMissionStream();

    // Open the game configuration file
    PTree pTree;

    // Parse the file
    if (pTree.AddFile(FILENAME_MISSION_CONFIG))
    {
      // Get the global scope
      FScope *gScope = pTree.GetGlobalScope();
      FScope *sScope;

      // Process each function
      while ((sScope = gScope->NextFunction()) != NULL)
      {
        switch (sScope->NameCrc())
        {
          case 0xEDF7E07D: // "DefaultRule"
            ruleSet = StdLoad::TypeString(sScope);
            break;

          case 0x7F6A7C11: // "FixedRule"
            ruleSetFixed = StdLoad::TypeU32(sScope);
            break;

          case 0x1D4A8250: // "WorldInfo"
            // Load map size
            size = StdLoad::TypeU32(sScope, "CellMapX");
            break;

          case 0xCA519158: // "DefineTeams"
          {
            ASSERT(!teams.GetCount())

            // Count the number of teams we find
            FScope *fScope;

            // Process each function
            while ((fScope = sScope->NextFunction()) != NULL)
            {
              switch (fScope->NameCrc())
              {
                case 0xB884B9E8: // "CreateTeam"
                {
                  // Is this team available for play
                  if (StdLoad::TypeU32(fScope, "AvailablePlay", 1))
                  {
                    // Create new team info
                    TeamInfo *info = new TeamInfo(fScope);

                    // Add to the tree
                    teams.Add(info->GetName().crc, info);
                  }
                  break;
                }
              }
            }

            break;
          }
        }
      }
    }
    else
    {
      size = 0;
    }

    // Load preview texture
    ReloadTextures();

    // Restore old stuff
    if (oldStream)
    {
      CloseMissionStream(oldStream);
    }
  }


  //
  // Preview::~Preview
  //
  Preview::~Preview()
  {
    ReleaseTextures();
    teams.DisposeAll();
  }


  //
  // Preview::OpenMissionStream
  //
  const char *Preview::OpenMissionStream()
  {
    if (*path.str && *name.str)
    {
      // Save the currently active stream
      const char * oldStream = FileSys::GetActiveStream();

      // Find the mission
      const Missions::Mission *mission = Missions::FindMission(name.str, path.str);

      if (mission)
      {
        // Setup the preview stream
        mission->SetupStream(GAME_PREVIEW_STREAM);

        // Set the preview stream to be the current stream
        FileSys::SetActiveStream(GAME_PREVIEW_STREAM);

        return (oldStream);
      }
      else
      {
        LOG_ERR(("Preview: Mission [%s][%s] is gone!", path.str, name.str))
        return (NULL);
      }
    }
    else
    {
      return (NULL);
    }
  }


  //
  // Preview::CloseMissionStream
  //
  void Preview::CloseMissionStream(const char *oldStream)
  {
    ASSERT(oldStream)

    // Restore the old active stream
    FileSys::SetActiveStream(oldStream);

    // Delete the preview stream
    FileSys::DeleteStream(GAME_PREVIEW_STREAM);
  }


  //
  // Preview::LoadTextures
  //
  void Preview::LoadTextures()
  {
    // Does the map texture exist
    if (FileSys::Exists(GAME_PREVIEW_BITMAP))
    {
      terrain = new Bitmap(bitmapTEXTURE | bitmapNORELOAD);
      terrain->SetName("GamePreview::Terrain");
      terrain->ReadBMP(GAME_PREVIEW_BITMAP);
    }
  }


  //
  // Preview::ReleaseTextures
  //
  void Preview::ReleaseTextures()
  {
    terrain = NULL;
  }


  //
  // Preview::ReloadTextures
  //
  // Reload textures on mode change
  //
  void Preview::ReloadTextures()
  {
    const char *oldStream = OpenMissionStream();

    // Delete current textures
    ReleaseTextures();

    // Create new ones
    LoadTextures();

    // Restore old stream
    if (oldStream)
    {
      CloseMissionStream(oldStream);
    }
  }


  //
  // GetMission
  //
  // Get the mission
  //
  const Missions::Mission * Preview::GetMission()
  {
    return (Missions::FindMission(name.str, path.str));
  }


  //
  // Preview::FindId
  //
  Preview::TeamInfo *Preview::FindTeamId(U32 id)
  {
    for (NBinTree<TeamInfo>::Iterator i(&teams); *i; i++)
    {
      if (id == (*i)->GetTeamId())
      {
        return (*i);
      }
    }
    return (NULL);
  }
}

