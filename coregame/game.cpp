///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// De La Game
//
// 12-AUG-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes a lot
//
#include "vid_public.h"
#include "game.h"
#include "coregame.h"
#include "team.h"
#include "player.h"
#include "terrain.h"
#include "worldctrl.h"
#include "random.h"
#include "gametime.h"
#include "game_config.h"
#include "game_rc.h"
#include "gamegod.h"
#include "ai.h"
#include "meshent.h"
#include "footprint.h"
#include "gameobjctrl.h"
#include "mapobjctrl.h"
#include "unitobjctrl.h"
#include "mapobjiter.h"
#include "pathsearch.h"
#include "physicsctrl.h"
#include "particlesystem.h"
#include "regionobj.h"
#include "trailobj.h"
#include "bookmarkobj.h"
#include "unitobj.h"
#include "gamebabel.h"
#include "fx.h"
#include "message.h"
#include "terraingroup.h"
#include "tagobj.h"
#include "environment.h"
//#include "lensflare.h"
#include "squadobjctrl.h"
#include "taskctrl.h"
#include "common.h"
#include "terrain.h"
#include "main.h"
#include "movement.h"
#include "environment.h"
#include "environment_quake.h"
#include "sound.h"
#include "mesheffect_system.h"
#include "weapon.h"
#include "savegame.h"
#include "sides.h"
#include "sight.h"
#include "campaigns.h"
#include "difficulty.h"
#include "sideplacement.h"
#include "unitlimits.h"
#include "multiplayer.h"
#include "multiplayer_data.h"
#include "mods.h"
#include "user.h"
#include "missionvar.h"
#include "gamesound.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Game
//
namespace Game
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Constants
  //
  static const char *TYPEFILE = "types.cfg";
  static const char *STUDIOFILE = "types_studio.cfg";


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Static data
  //

  // Is the system intialized
  static Bool initialized = FALSE;

  // Has a mission been preloaded
  static Bool missionPreLoaded = FALSE;

  // Has a mission been loaded
  static Bool missionLoaded = FALSE;

  // Was the mission online ?
  static Bool missionOnline = FALSE;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Prototypes
  //

  // Register code-generated types
  static void RegisterCodeTypes();


  //
  // ReportMissionStart
  //
  // Report that an existing mission has been started
  //
  static void ReportMissionStart()
  {
    // If we're online record that we are online
    missionOnline = MultiPlayer::Data::Online();

    // Is there an active mission
    if (const Missions::Mission *mission = Missions::GetActive())
    {
      // If in a campaign, record it as the last played
      if (Campaigns::Campaign *campaign = Campaigns::Find(mission))
      {
        Campaigns::GetProgress().SetLastCampaign(campaign);
      }
    }
  }


  //
  // ReportMissionEnd
  //
  // Report a mission result for the display team
  //
  static void ReportMissionEnd()
  {
    // Is there an active mission
    if (const Missions::Mission *mission = Missions::GetActive())
    {
      // Get the current display team
      if (Team *team = Team::GetDisplayTeam())
      {
        // Get the end game stats
        const Team::EndGame &endGame = team->GetEndGame();

        // Was there actually a result for a valid mission
        if (endGame.result != Team::EndGame::NR)
        {
          // Get the users record for this mission
          Campaigns::Progress::Mission &record = Campaigns::GetProgress().GetMission
          (
            mission->GetGroup().GetPath().str, mission->GetName().str
          );

          // Remember the current difficulty level
          record.SetDifficulty(Difficulty::GetCurrentSetting().GetName().str);

          // Update mission outcome stats
          switch (endGame.result)
          {
            case Team::EndGame::WIN:
              record.RecordWin();
              break;

            case Team::EndGame::LOSE:
              record.RecordLoss();
              break;
          }
        }
      }
    }
  }


  //
  // Init
  //
  void Init()
  {
    ASSERT(!initialized);
    ASSERT(!missionLoaded);
    ASSERT(!missionPreLoaded);
    ASSERT(!missionOnline);

    // Initialize systems
    Player::Init();
    Common::Init();

    initialized = TRUE;
  }


  //
  // Done
  //
  void Done()
  {
    ASSERT(initialized);

    if (missionLoaded)
    {
      CloseMission();
    }

    if (missionPreLoaded)
    {
      PostCloseMission();
    }

    // Shutdown systems
    Common::Done();
    Player::Done();

    // Destroy Commands
    VarSys::DeleteItem("game");

    // We're definitely not online now
    missionOnline = FALSE;

    initialized = FALSE;
  }


  //
  // PreOpenMission
  //
  // Pre-Open an existing mission or new mission
  //
  void PreOpenMission(const char *missionExecFile)
  {
    ASSERT(initialized);

    // Is there a selected mission
    const Missions::Mission *mission = Missions::GetSelected();

    // Setup the active mission
    Missions::SetActive(mission);

    // Initialize user addons
    User::InitializeAddons();

    // Log diagnostics to show the flow of a run
    if (mission)
    {
      LOG_DIAG(("PreOpenMission: [%s][%s]", mission->GetGroup().GetPath().str, mission->GetName().str));
    }
    else
    {
      LOG_DIAG(("PreOpenMission: New mission"));
    }

    // PostClose any pre-open mission
    PostCloseMission();

    // Invalidate old load errors
    ClearLoadErrors();

    if (mission)
    {
      // Setup read access
      Missions::OpenActiveStream();
    }

    GameGod::SetModeChangeCallback();

    // Initialize Systems which must be initialized before "game.cfg"

    Team::Init();
    Environment::Init();
    MoveTable::Init();
    SidePlacement::Init();
    UnitLimits::Init();
    TerrainGroup::Init();
    AI::Init();
    TaskCtrl::Init();
    Config::Init();
    MissionVar::Init();

    if (mission)
    {
      // Load the game configuration for the mission
      if (!Config::Load(FILENAME_MISSION_CONFIG))
      {
        ERR_FATAL(("Failed to load '%s'", FILENAME_MISSION_CONFIG))
      }
    }

    // Activate default terrain group
    TerrainGroup::SetActiveGroup(TerrainGroup::GetDefaultGroup());

    // Exec optional mission specific load screen after world stream is added
    if (missionExecFile)
    {
      Main::Exec(missionExecFile, Main::ScopeHandler, FALSE);
    }

    // Initialize Core Game Systems
    GameBabel::Init();
    ArmourClass::Init();
    WorldCtrl::Init();
    FootPrint::Init();
    GameObjCtrl::Init();
    MapObjCtrl::Init();
    UnitObjCtrl::Init();
    SquadObjCtrl::Init();
    MapObjIter::Init();
    PathSearch::Init();
    PhysicsCtrl::Init();
    Movement::Init(Common::InStudio());
//    LensFlare::InitResources();
    ParticleSystem::Init();
    MeshEffectSystem::Init();
    FX::Init();
    Message::Init();
    Weapon::Manager::Init();

    // Register code-generated types
    RegisterCodeTypes();

    // Load type definitions
    ProcessTypeFile(TYPEFILE);

    // If in the studio, add the studio file
    if (Common::InStudio())
    {
      ProcessTypeFile(STUDIOFILE);

      // If we're entering the studio and we have a fixed ruleset, load it
      if (Config::GetFixedRule())
      {
        Mods::Mod *mod = Mods::GetMod(Mods::Types::RuleSet, Config::GetDefaultRule().crc);
        if (mod)
        {
          mod->Load();
        }
      }
    }

    // Allow type-resolution within other types
    GameObjCtrl::PostLoadTypes();
    Weapon::Manager::PostLoad();

    // Load user addon configs
    User::LoadAddons();

    // Load AI Configuration
    AI::ConfigLoad();

    // Team preload setup
    Team::PreLoadAll();

    // Report that a mission has been started
    ReportMissionStart();

    // Mission now pre-loaded
    missionPreLoaded = TRUE;
  }


  //
  // PostCloseMission
  //
  // Post Close a mission
  //
  void PostCloseMission()
  {
    ASSERT(initialized);

    // Do we have a mission pre-loaded
    if (missionPreLoaded)
    {
      // Report the result of the mission just played
      ReportMissionEnd();

      // Post game.cfg systems
      Weapon::Manager::Done();
      Message::Done();
      FX::Done();
      MeshEffectSystem::Done();
      ParticleSystem::Done();
      Mesh::Manager::DisposeAll();

      AI::Done();
      Movement::Done();
      PhysicsCtrl::Done();
      PathSearch::Done();
      SquadObjCtrl::Done();
      UnitObjCtrl::Done();
      MapObjCtrl::Done();
      GameObjCtrl::Done();
      FootPrint::Done();
      WorldCtrl::Done();
      ArmourClass::Done();
      GameBabel::Done();

      // Pre game.cfg systems
      Config::Done();
      TaskCtrl::Done();
      TerrainGroup::Done();
      UnitLimits::Done();
      SidePlacement::Done();
      MoveTable::Done();
      Environment::Done();
      Team::Done();   
      MissionVar::Done();

      // Close read access
      Missions::CloseActiveStream();

      // Clear the active mission
      Missions::SetActive(NULL);

      // Mission now not pre-loaded
      missionPreLoaded = FALSE;
    }
  }


  //
  // OpenMission
  //
  // Load a new mission
  //
  void OpenMission()
  {
    ASSERT(initialized);
    ASSERT(missionPreLoaded);
    ASSERT(!missionLoaded);

    // Is there an active mission
    const Missions::Mission *mission = Missions::GetActive();

    // Initialize terrain system
    Terrain::Sky::Load("engine_sky.xsi", "engine_cloud.tga");

    // If multiplayer hasn't set it up
    if (!MultiPlayer::Data::HaveData())
    {
      // Sync random number services
      Random::sync.SetSeed(0x7FFFFFFF);
    }

    // Initialize core game system
    CoreGame::Init();

    // Save the current position
    Vector pos = Vid::CurCamera().WorldMatrix().Position();

    // Initalize timing
    GameTime::Reset();

    // Load the objects for the mission
    if (!CoreGame::OpenMission())
    {
      ERR_FATAL(("CoreGame::OpenMission failed"));
    }

    if (mission)
    {
      // Perform Game File Post Loading

      // Team Post Load processing
      Team::PostLoadAll();
    }

    // If required, create side based units for all teams
    if (!SaveGame::LoadActive() && !Common::InStudio())
    {
      Team::CreateSideBasedUnits();
    }

    if (mission)
    {
      // Player Post Load processing
      Player::PostLoadAll();

      // Multiplayer Post Load proccessing
//      MultiPlayer::PostLoad();

      // AI Post Load processing
      AI::ConfigPostLoad();
    }

    // Now that the client team is set, apply the difficulty setting
    Team::ApplyDifficultyLevels();

    // Now that the client team is set, update the sight display
    Sight::UpdateDisplay(Team::GetDisplayTeam(), TRUE);

    // Should we add the side resource
    if (!Common::InStudio() && mission && !mission->IsSystem())
    {
      // Do we have a display team
      if (Team *team = Team::GetDisplayTeam())
      {
        // Open the interface resource
        Sides::OpenResource(Sides::GetSide(team->GetSide()));
      }
    }

    // Environment Post Load processing
    Environment::PostLoad();

    // Make sure the camera is within and above the terrain
    //
    if (pos.x >= Terrain::MeterWidth() || pos.z >= Terrain::MeterHeight())
    {
      pos.x = (F32) (Terrain::MeterWidth()  >> 1);
      pos.z = (F32) (Terrain::MeterHeight() >> 1);
    }

    GameGod::Loader::Done();

    // Notify save game of load position
    SaveGame::Notify(0xEA3E0DCC); // "Load::Game::OpenMission"

    // Mission now loaded
    missionLoaded = TRUE;
  }


  //
  // CloseMission
  //
  // Close an open mission if one exists
  //
  void CloseMission()
  {
    ASSERT(initialized);
    ASSERT(missionLoaded);

    // Reset game sound state before mission and sides streams are closed
    GameSound::Reset();

    // Close core mission data
    CoreGame::CloseMission();

    // Close the side interface resource
    Sides::CloseResource();

    // Close any mod resources
    Mods::FreeResources();

    // Close display related stuff (AFTER CoreGame::CloseMission!)
    FX::CloseMission();
    ParticleSystem::CloseMission();

    // Core game system
    CoreGame::Done();

    // Shutdown terrain manager
//    Terrain::Done();    // in vid.cpp; only once per app run

    // No mission loaded
    missionLoaded = FALSE;
  }


  //
  // SaveMission
  //
  // Save currently loaded mission (use NULL arguments to save as current name)
  //
  Bool SaveMission(const char *path, const char *mission)
  {
  #ifdef DEMO
    #pragma message("Save-game disabled")
    return (FALSE);
  #else
    ASSERT(initialized);

    ClearLoadErrors();

    // Check we have a mission loaded
    if (!missionLoaded)
    {
      return (FALSE);
    }

    // If this is a mission save, setup write access
    if (!SaveGame::SaveActive())
    {
      Bool result = FALSE;

      // Do we have a specific location
      if (path && mission)
      {
        // Ensure not empty strings
        if (*path && *mission)
        {
          result = Missions::SetupWriteAccess(path, mission);
        }
      }
      else

      if (Missions::GetActive())
      {
        result = Missions::SetupWriteAccess(Missions::GetActive());
      }

      if (!result)
      {
        CON_ERR(("Unable to setup write access"))
        return (FALSE);
      }
    }

    // Save the game file
    if (!Config::Save(FILENAME_MISSION_CONFIG))
    { 
      CON_ERR(("Unable to write '%s'", FILENAME_MISSION_CONFIG))
      return (FALSE);
    }

    return (CoreGame::SaveMission());
  #endif
  }


  //
  // MissionPreLoaded
  //
  // Is a mission currenly preloaded
  //
  Bool MissionPreLoaded()
  {
    return (missionPreLoaded);
  }


  //
  // MissionLoaded
  //
  // Is a mission currently loaded
  //
  Bool MissionLoaded()
  {
    return (missionLoaded);
  }


  //
  // MissionOnline
  //
  // Is the mission online
  //
  Bool MissionOnline()
  {
    return (missionOnline);
  }


  //
  // RegisterCodeTypes
  //
  // Register code-generated types
  //
  void RegisterCodeTypes()
  {
    GameObjCtrl::AddObjectType(new RegionObjType("Region", NULL));
    GameObjCtrl::AddObjectType(new TrailObjType("Trail", NULL));
    GameObjCtrl::AddObjectType(new TagObjType("Tag", NULL));
    GameObjCtrl::AddObjectType(new BookmarkObjType("Bookmark", NULL));
  }


  //
  // ProcessTypeFile
  //
  // Process a type definition file
  //
  void ProcessTypeFile(const char *typeFile)
  {
    ASSERT(initialized);

    PTree tFile(TRUE, MultiPlayer::GetDataCrc());
    FScope *sScope, *fScope;

    // Parse the file
    tFile.AddFile(typeFile);

    // Get the global scope
    fScope = tFile.GetGlobalScope();

    // Step through each function in this scope
    while ((sScope = fScope->NextFunction()) != 0)
    {
      switch (sScope->NameCrc())
      {
        default:
          if (!MeshEffectSystem::ProcessTypeFile( sScope))
          {
            LOG_WARN(("Unknown type creation function '%s'", sScope->NameStr()));
            sScope->DumpScope();
          }
          break;

        case 0x25C3A645: // "CreateEffectType"
          FX::ProcessCreate( sScope);
          break;

        case 0x4C29B238: // "CreateObjectType"
          GameObjCtrl::ProcessCreateObjectType(sScope);
          break;

        case 0x743D0B6C: // "CreateTractionType"
          MoveTable::ProcessCreateTractionType(sScope);
          break;

        case 0xB0416B86: // "CreateParticleType"
          ParticleSystem::ProcessCreateParticleType(sScope);
          break;

        case 0xEE3D4236: // "CreateWeaponType"
          Weapon::Manager::ProcessCreateWeaponType(sScope);
          break;

        case 0xF346B480: // "CreateParticleRenderType"
          ParticleSystem::ProcessCreateParticleRenderType(sScope);
          break;

        case 0x6E57DD5F: // "CreatePhysicsModel"
          Movement::ProcessCreateMovementModel(sScope, StdLoad::TypeString(sScope));
          break;

        case 0x8B9A0806: // "CreateQuakeType"
          Environment::Quake::ProcessCreate( sScope);
          break;

        case 0x4FE3EBF8: // "ConfigureRadioEvent"
          Radio::ProcessConfigureRadioEvent(sScope);
          break;

        case 0x3E3F2563: // "ConfigureGameMessage"
          Message::ProcessConfigureGameMessage(sScope);
          break;

        case 0xE56A2DBA: // "ConfigureLocationMessage"
          Message::ProcessConfigureLocationMessage(sScope);
          break;

        case 0xB7B228D8: // "ConfigureObjMessage"
          Message::ProcessConfigureObjMessage(sScope);
          break;
      }
    } 

    LOG_DIAG(("Type Stream Crc: %08X", tFile.GetCrc()))
    MultiPlayer::SetDataCrc(tFile.GetCrc());

    if (GameGod::CheckTypes())
    {
      FSCOPE_CHECK(fScope)
    }
  }


  //
  // Converts all Mission::LoadError to Mission::OldLoadError
  //
  void ClearLoadErrors()
  {
    Console::ConvertMessages(
      0xA1D5DDD2, // "Mission::LoadError"
      0x269A3144); // "Mission::OldLoadError"
  }
}
