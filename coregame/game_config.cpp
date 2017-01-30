///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game Loading
//
// 14-AUG-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "game_config.h"
#include "filesys.h"
#include "missions.h"
#include "worldctrl.h"
#include "team.h"
#include "environment.h"
#include "terraingroup.h"
#include "physicsctrl.h"
#include "coregame.h"
#include "gamegod.h"
#include "missionvar.h"
#include "viewer.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Game
//
namespace Game
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Config
  //
  namespace Config
  {
    // Is the system initialized
    static Bool initialized = FALSE;

    // The default rule
    static GameIdent defaultRule;

    // Is the default rule fixed
    static Bool fixedRule;

    // List of required addons
    static GameIdentList requiredAddons;


    //
    // Init
    //
    // Initialize the system
    //
    void Init()
    {
      ASSERT(!initialized)

      // Setup defaults
      defaultRule = "None";
      fixedRule = FALSE;

      initialized = TRUE;
    }


    //
    // Done
    //
    // Shutdown the system
    //
    void Done()
    {
      ASSERT(initialized)

      requiredAddons.DisposeAll();

      initialized = FALSE;   
    }


    //
    // GetDefaultRule
    //
    // Get the default rule
    //
    GameIdent & GetDefaultRule()
    {
      return (defaultRule);
    }


    //
    // GetFixedRule
    //
    // Get the fixed rule flag
    //
    Bool & GetFixedRule()
    {
      return (fixedRule);
    }


    //
    // GetRequiredAddons
    //
    // Get the list of required addons
    //
    GameIdentList & GetRequiredAddons()
    {
      return (requiredAddons);
    }


    //
    // Save
    //
    // Save the Game Configuration for a mission, false if unable to create file
    //
    Bool Save(const char *name)
    {
      PTree pTree;
      FilePath path;

      // Get the global scope of the parse tree
      FScope *gScope = pTree.GetGlobalScope();

      // Save local info
      StdSave::TypeString(gScope, "DefaultRule", defaultRule.str);
      StdSave::TypeU32(gScope, "FixedRule", fixedRule);

      // Required addons
      FScope *fScope = gScope->AddFunction("RequiredAddons");
      for (List<GameIdent>::Iterator a(&requiredAddons); *a; ++a)
      {
        StdSave::TypeString(fScope, "Add", (*a)->str);
      }

      // Save World Information
      WorldCtrl::SaveInfo(gScope->AddFunction("WorldInfo"));

      // Save Environment Information
      Environment::SaveInfo(gScope->AddFunction("Environment"));

      // Teams
      Team::SaveAll(gScope->AddFunction("DefineTeams"));

      // Terrain group
      TerrainGroup::SaveConfig(gScope->AddFunction("TerrainGroup"));

      // Camera
      Viewer::SaveConfig(gScope->AddFunction("Camera"));

      // Physics
      PhysicsCtrl::SaveConfig(gScope->AddFunction("Physics"));

      // Mission vars
      MissionVar::Save(gScope->AddFunction("MissionVar"));

      // Work out save path
      Dir::PathMake(path, Missions::GetWritePath(), name);
  
      // Write the file
      if (!CoreGame::WriteTree(pTree, path.str))
      {
        LOG_WARN(("Unable to write to file [%s]", path.str));
        return (FALSE);
      }

      // Success
      return (TRUE);
    }
    
    
    //
    // Load
    //
    // Load the Game Configuration for a mission, false if not found
    //
    Bool Load(const char *name)
    {
      ASSERT(name);

      PTree pTree;

      // Parse the file
      if (pTree.AddFile(name))
      {
        // Get the global scope
        FScope *gScope = pTree.GetGlobalScope();
        FScope *sScope;

        GameGod::Loader::SubSystem("#game.loader.configuration", gScope->GetBodyCount());

        // Process each function
        while ((sScope = gScope->NextFunction()) != NULL)
        {
          GameGod::Loader::Advance();

          switch (sScope->NameCrc())
          {
            case 0xEDF7E07D: // "DefaultRule"
              defaultRule = StdLoad::TypeString(sScope);
              break;

            case 0x7F6A7C11: // "FixedRule"
              fixedRule = StdLoad::TypeU32(sScope);
              break;

            case 0x91667FF6: // "RequiredAddons"
              StdLoad::TypeStrCrcList(sScope, requiredAddons);
              break;

            case 0x1D4A8250: // "WorldInfo"
              WorldCtrl::LoadInfo(sScope);
              break;

            case 0x388B26A8: // "Environment"
              Environment::LoadInfo(sScope);
              break;

            case 0x4CB618E3: // "Camera"
              Viewer::LoadConfig(sScope);
              break;

            case 0xCA519158: // "DefineTeams"
              Team::Configure(sScope);
              break;

            case 0x0F7BC4DE: // "TerrainGroup"
              TerrainGroup::LoadConfig(sScope);
              break;

            case 0x3B78591F: // "Physics"
              PhysicsCtrl::LoadConfig(sScope);
              break;

            case 0x3FC0106B: // "MissionVar"
              MissionVar::Load(sScope);
              break;

            default:
              LOG_ERR(("Unknown command '%s' in '%s'", sScope->NameStr(), name))
              break;
          }
        }

        return (TRUE);
      }

      // Didn't find the file
      return (FALSE);
    }
  }
}
