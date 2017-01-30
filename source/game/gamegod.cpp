///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// GameGod, the creator and the destroyer
//
// 24-JUL-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "gamegod.h"
#include "main.h"
#include "orders.h"
#include "iface.h"
#include "input.h"
#include "vid_public.h"
#include "console.h"
#include "gametime.h"
#include "iclistbox.h"
#include "game_rc.h"
#include "sound.h"
#include "gamesound.h"
#include "multiplayer.h"
#include "sync.h"
#include "user.h"
#include "demo.h"
#include "win32reg.h"
#include "worldctrl.h"
#include "iface_types.h"
#include "terraingroup.h"
#include "savegame.h"
#include "sides.h"
#include "mods.h"
#include "campaigns.h"
#include "difficulty.h"
#include "missions.h"
#include "woniface.h"
#include "setup.h"


#include "game_login.h"
#include "game_missionselection.h"
#include "game_campaignselection.h"
#include "game_saveload.h"
#include "game_difficultylist.h"
#include "game_addonlist.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace GameGod
//

namespace GameGod
{
  // Has the system been initialized
  static Bool initialized = FALSE;

  // The label of the original CD
  static const char * CD_LABEL_ORIGINAL = "DarkReign2";

  // Have we seen the original CD yet
  static Bool seenOriginal;

  // Should we check types for fScope dirtyness ?
  static VarInteger checkTypes;

  // Should we check objects for fScope dirtyness ?
  static VarInteger checkObjects;

  // Current flow action
  static VarString flowAction;

  // Current flow state
  static VarString flowState;

  // Is the studio enabled
  static VarInteger studioEnabled;


  //
  // Prototypes
  //
  IControl *CreateHandler(U32 crc, IControl *parent, U32);


  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Loader - Mission loading progress
  //
  namespace Loader
  {

    // Max Redraw rate
    static const MaxRedrawFps = 20;
    static const MaxRedrawUs = 1000000 / MaxRedrawFps;

    // Vars
    static VarString system;
    static VarFloat percent;

    // Total number of items
    static U32 total;
    static U32 current;

    // Percent per item
    static F32 rate;

    // Last redraw time
    static U32 lastRedraw;


    //
    // Repaint the screen
    //
    void Repaint(Bool force = FALSE)
    {
      // Ping the watchdog
      Debug::Watchdog::Poll();

      U32 now = Clock::Time::UsLwr();

      if (force || (now - lastRedraw > MaxRedrawUs))
      {
        IFace::PaintAll();

        lastRedraw = now;
      }
    }


    //
    // Specify a new subsystem
    //
    void SubSystem(const char *name, U32 items)
    {
      current = 0;
      total = items;
      rate = items ? 1.0F / F32(total) : 1.0F;

      // Setup vars
      system = name;
      percent = 0.0F;

      // Force a repaint now
      Repaint(TRUE);
    }


    //
    // Update progress of previous system
    //
    void Update(U32 count)
    {
      current = count;

      if (current > total)
      {
//        LOG_DEV(("Loader::Update out of range for %s (%d vs %d)", *system, current, total))
        current = total;
      }
      percent = F32(current) * rate;

      // Force repaint if this is the last in this system
      Repaint(current == total);
    }


    //
    // Advance counter
    //
    void Advance(U32 count)
    {
      Update(current + count);
    }


    //
    // Finished loading
    //
    void Done()
    {
      system = "";
      percent = 0.0F;
    }
  }


  //
  // FindOriginalCD
  //
  // Is the original CD in a drive
  //
  static Bool FindOriginalCD()
  {
    #ifdef DEVELOPMENT

      seenOriginal = TRUE;

    #else

      // Have we seen the original, or we can see it now
      if (!seenOriginal && Setup::FindOriginalCD())
      {
        // Remember that we've seen it
        seenOriginal = TRUE;
      }

    #endif

    return (seenOriginal);
  }


  //
  // CmdHandler
  //
  // Handles var system commands 
  //
  void CmdHandler(U32 pathCrc)
  {
    switch (pathCrc)
    {
      case 0x41878DEC: // "gamegod.verify"
      {
        char *controlName;

        if (Console::GetArgString(1, controlName))
        {
          if (IControl *control = IFace::FindByName(controlName))
          {
            if (FindOriginalCD())
            {
              IFace::PostEvent(control, NULL, IFace::NOTIFY, 0x9FF2D5A9); // "Verify::Valid"
            }
            else
            {
              IFace::PostEvent(control, NULL, IFace::NOTIFY, 0x412CB189); // "Verify::Invalid"
            }
          }
        }
        break;
      }

      case 0xC9FB8050: // "gamegod.dosafeload"
      {
        Missions::SetSafeLoad(TRUE);
        CON_DIAG(("Filtering objects for next load only..."))
        break;
      }

      case 0x05A5D04A: // "gamegod.setterraingroup"
      {
        const char *group;

        if (Console::GetArgString(1, group))
        {
          TerrainGroup::SetDefaultGroup(group);
        }
        break;
      }

      case 0xC04E77FC: // "gamegod.setnewmapsize"
      {
        F32 metreX;
        F32 metreZ;
        char *controlName = "|DlgStudioNewMap";

        if (Console::GetArgFloat(1, metreX) && Console::GetArgFloat(2, metreZ))
        {
          // Get optional control name
          Console::GetArgString(3, controlName);

          // Calculate the closest cell values
          S32 cellX = U32(metreX / WorldCtrl::CellSize());
          S32 cellZ = U32(metreZ / WorldCtrl::CellSize());

          // If using default values, check to see they are valid
          if (WorldCtrl::SetDefaultMapSize(cellX, cellZ))
          {
            if (IControl *ic = IFace::FindByName(controlName))
            {
              IFace::PostEvent(ic, NULL, IFace::NOTIFY, 0x07984B08); // "New"
            }
            break;
          }
          else
          {
            CON_ERR(("Map size (%.0f x %.0f) is invalid", metreX, metreZ))
          }
        }
        break;
      }

      case 0x68946F46: // "gamegod.missions.select"
      {
        const char *missionName;

        if (Console::GetArgString(1, missionName))
        {
          // Get optional group path
          const char *groupPath = NULL;
          Console::GetArgString(2, groupPath);

          if (const Missions::Mission *mission = Missions::FindMission(missionName, groupPath))
          {
            Missions::SetSelected(mission);
          }
          else
          {
            CON_ERR(("Unable to find mission [%s] in any group", missionName))
          }
        }
        else
        {
          Missions::SetSelected();
        }

        break;
      }
      
      case 0xC19B91E1: // "gamegod.missions.launch"
      {
        Missions::LaunchMission(TRUE);
        break;
      }

      case 0x9F954BF1: // "gamegod.missions.replay"
      {
        Missions::ReplayMission(TRUE);
        break;
      }

      case 0x7B8211C1: // "gamegod.missions.next"
      {
        Missions::NextMission(TRUE);
        break;
      }

      case 0xA9CD40A3: // "gamegod.missions.current"
      {
        if (const Missions::Mission *m = Missions::GetSelected())
        {
          CON_DIAG(("Selected : [%s] [%s]", m->GetName().str, m->GetGroup().GetPath().str));
        }
        else
        {
          CON_DIAG(("Selected : None"));
        }

        if (const Missions::Mission *m = Missions::GetActive())
        {
          CON_DIAG(("Active   : [%s] [%s]", m->GetName().str, m->GetGroup().GetPath().str));
        }
        else
        {
          CON_DIAG(("Active   : None"));
        }
        break;
      }

      case 0xC9AEF64F: // "gamegod.missions.progress"
      {
        const char *name;

        // Are we completing a campaign
        if (Console::GetArgString(1, name))
        {
          // Find the campaign
          if (Campaigns::Campaign *campaign = Campaigns::Find(name))
          {
            // Complete each mission
            Campaigns::GetProgress().CompleteCampaign(campaign);
          }
        }
        else
        {
          // Display each mission record
          for (NBinTree<Campaigns::Progress::Mission>::Iterator i(&Campaigns::GetProgress().GetMissions()); *i; ++i)
          {
            // Get the record
            Campaigns::Progress::Mission &r = **i;

            CON_DIAG(("[%s] [%s] %s", r.GetPath().str, r.GetMission().str, r.MissionCompleted(TRUE) ? "Completed" : ""));
          }
        }
        break;
      }
    }
  }


  //
  // ScopeHandler
  //
  // Scope handler for game specific configuration
  //
  void ScopeHandler(FScope *fScope)
  {
    // This once did custom stuff...
    Main::ScopeHandler(fScope);
  }


  //
  // ExecGameConfig
  //
  // Execute game specific configuration scope
  //
  void ExecGameConfig()
  {
    PTree pTree;

    // Attempt to open the file
    if (pTree.AddFile(APPLICATION_CONFIGFILE))
    {
      // Find our config scope
      FScope *fScope = pTree.GetGlobalScope()->GetFunction("GameConfig");

      // And execute it using custom handler
      Main::ProcessCmdScope(fScope, ScopeHandler);
    }
    else
    {
      // The file was found at startup, so should have been here now
      ERR_FATAL(("Unexpected failure while trying to execute '%s'", APPLICATION_CONFIGFILE));
    }
  }


  //
  // Video Mode change callback function
  //
  void OnModeChange()
  {
    // Notify input system of mode change
    Input::OnModeChange();

    // Notify interface of mode change
    IFace::OnModeChange();
  }


  // 
  // Init
  //
  // Game specific, core system initialization
  //
  void Init()
  {
    ASSERT(!initialized)

    // Never seen original CD
    seenOriginal = FALSE;

    // See if it's there now
    FindOriginalCD();

    // Register command handler
    VarSys::RegisterHandler("gamegod", CmdHandler);
    VarSys::RegisterHandler("gamegod.missions", CmdHandler);
    VarSys::RegisterHandler("gamegod.flow", CmdHandler);
    VarSys::RegisterHandler("gamegod.loader", CmdHandler);

    // Create items
    VarSys::CreateCmd("gamegod.verify");
    VarSys::CreateCmd("gamegod.dosafeload");
    VarSys::CreateCmd("gamegod.setterraingroup");
    VarSys::CreateCmd("gamegod.setnewmapsize");

    VarSys::CreateInteger("gamegod.check.types", 0, VarSys::DEFAULT, &checkTypes);
    VarSys::CreateInteger("gamegod.check.objects", 0, VarSys::DEFAULT, &checkObjects);

    VarSys::CreateCmd("gamegod.missions.select");
    VarSys::CreateCmd("gamegod.missions.launch");
    VarSys::CreateCmd("gamegod.missions.replay");
    VarSys::CreateCmd("gamegod.missions.next");
    VarSys::CreateCmd("gamegod.missions.current");
    VarSys::CreateCmd("gamegod.missions.progress");

    VarSys::CreateInteger("gamegod.studiomode", 0, VarSys::DEFAULT, &studioEnabled);

    VarSys::CreateString("gamegod.flow.action", "", VarSys::DEFAULT, &flowAction);
    VarSys::CreateString("gamegod.flow.state", "", VarSys::DEFAULT, &flowState);

    VarSys::CreateString("gamegod.loader.system", "", VarSys::DEFAULT, &Loader::system);
    VarSys::CreateFloat ("gamegod.loader.percent", 0.0F, VarSys::DEFAULT, &Loader::percent)->SetFloatRange(0.0F, 1.0F);

    // System now initialized
    initialized = TRUE;

    // Initialize game-specific core systems
    Sound::Init();
    Sound::Digital::Claim();
    Sound::Redbook::SetVolumeLabel(CD_LABEL_ORIGINAL);
    GameSound::Init();
    Orders::Init();
    ExecGameConfig();
    IFace::Init();
    GameTime::Init();
    Game::RC::Init();
    Sync::Init();
    Difficulty::Init();
    Campaigns::Init();
    Missions::Init();
    User::Init();
    Demo::Init();
    SaveGame::Init();
    Sides::Init();
    Mods::Init();
    MultiPlayer::Init();
 
    //Win32Reg::Update();

    Vid::InitIFace();
    Mesh::Manager::InitIFace();

    // Register controls
    IFace::RegisterControlClass("Game::Login", CreateHandler);
    IFace::RegisterControlClass("Game::MissionSelection", CreateHandler);
    IFace::RegisterControlClass("Game::CampaignSelection", CreateHandler);
    IFace::RegisterControlClass("Game::SaveLoad", CreateHandler);
    IFace::RegisterControlClass("Game::DifficultyList", CreateHandler);
    IFace::RegisterControlClass("Game::AddonList", CreateHandler);

    // Set the initial run code
    Main::runCodes.Set(0xA521E37B); // "Intro"
  }


  //
  // Done
  //
  // Game specific, core system shutdown
  //
  void Done()
  {
    ASSERT(initialized)

    // Flush every log from now on
    Log::SetFlush( TRUE);

    // Un-register controls
    IFace::UnregisterControlClass("Game::Login");
    IFace::UnregisterControlClass("Game::MissionSelection");
    IFace::UnregisterControlClass("Game::CampaignSelection");
    IFace::UnregisterControlClass("Game::SaveLoad");
    IFace::UnregisterControlClass("Game::DifficultyList");
    IFace::UnregisterControlClass("Game::AddonList");

    Mesh::Manager::DoneIFace();
    Vid::DoneIFace();

    // Shutdown game-specific core systems
    MultiPlayer::Done();
    Mods::Done();
    Sides::Done();
    SaveGame::Done();
    Demo::Done();
    User::Done();
    Missions::Done();
    Campaigns::Done();
    Difficulty::Done();
    Sync::Done();
    Game::RC::Done();
    GameTime::Done();
    IFace::Done();
    Orders::Done();
    GameSound::Done();
    Sound::Done();

    // Delete var scope
    VarSys::DeleteItem("gamegod");

    // System now shutdown
    initialized = FALSE;
  }


  //
  // setup mode change callback
  //
  void SetModeChangeCallback()
  {
    // Register the function to be called upon mode change
    Vid::SetModeChangeCallback(OnModeChange);
  }


  //
  // Should we check types
  //
  Bool CheckTypes()
  {
    return (checkTypes);
  }


  //
  // Should we check objects
  //
  Bool CheckObjects()
  {
    return (checkObjects);
  }


  //
  // Set flow action
  //
  void SetFlowAction(const char *action)
  {
    flowAction = action;
  }


  //
  // Start the App
  //
  void CDECL Start()
  {
    Utils::FP::Reset();

    // By default, prevent all exceptions from being raised
    Utils::FP::MaskException(Utils::FP::EX_ALL);

    #ifdef DEVELOPMENT

    if (Main::fpuExceptions)
    {
      Utils::FP::UnmaskException(Utils::FP::EX_ZERODIVIDE|Utils::FP::EX_OVERFLOW);
    }

    #endif

    SetModeChangeCallback();

    // Setup Multiplayer for command line
    MultiPlayer::PreInit();

    // Initialize generic core systems
    Main::CoreSystemInit();

    // Initialize game specific core systems
    Init();

    // Start the message pump
    Main::MessagePump();

    // Shutdown game specific systems
    Done();

    // Shutdown generic core systems
    Main::CoreSystemDone();

    #ifdef DEVELOPMENT

    Log::CheckErrors();

    if (Main::fpuExceptions)
    {
      Utils::FP::MaskException(Utils::FP::EX_ALL);
    }

    #endif
  }


  //
  // Control creation callback
  //
  IControl *CreateHandler(U32 crc, IControl *parent, U32)
  {
    IControl *ctrl = NULL;

    switch (crc)
    {
      case 0xE337E09C: // "Game::Login"
        ctrl = new Game::Login(parent);
        break;

      case 0x736F39B6: // "Game::MissionSelection"
        ctrl = new Game::MissionSelection(parent);
        break;

      case 0x0E88D05B: // "Game::CampaignSelection"
        ctrl = new Game::CampaignSelection(parent);
        break;

      case 0xA436C580: // "Game::SaveLoad"
        ctrl = new Game::SaveLoad(parent);
        break;

      case 0x6CEDB7EF: // "Game::DifficultyList"
        ctrl = new Game::DifficultyList(parent);
        break;

      case 0x1331C1AA: // "Game::AddonList"
        ctrl = new Game::AddonList(parent);
        break;

    }

    return ctrl;
  }
}

