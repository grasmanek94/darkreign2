///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game RunCodes
//
// 12-AUG-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_private.h"

#include "sync.h"
#include "game_rc.h"
#include "varsys.h"
#include "console.h"
#include "babel.h"
#include "iface.h"
#include "iface_messagebox.h"
#include "game.h"
#include "viewer.h"
#include "client.h"
#include "gametime.h"
#include "perfstats.h"
#include "mapobjctrl.h"
#include "gameobjctrl.h"
#include "physicsctrl.h"
#include "particlesystem.h"
#include "pathsearch.h"
#include "main.h"
#include "team.h"
#include "multiplayer.h"
#include "multiplayer_data.h"
#include "orders.h"
#include "sight.h"
#include "user.h"
#include "ai.h"
#include "fx.h"
#include "mapobj.h"
#include "environment.h"
#include "message.h"
#include "demo.h"
#include "sound.h"
#include "common.h"
#include "environment_time.h"
#include "collisionctrl.h"
#include "movement_pathfollow.h"
#include "savegame.h"
#include "missions.h"
#include "mods.h"
#include "random.h"
#include "coregame.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Game
//
namespace Game
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace RC
  //
  namespace RC
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Internal Data
    //

    // Is the system initialized
    static Bool initialized = FALSE;

    // The runcode manager
    static RunCodes runCodes("Game");

    // Average framerate of the game
    static Clock::Watch frameRate;
    static Clock::Watch displayRate;
    static Clock::Watch triCount;

    // Migration state
    static enum { NORMAL, MIGRATING, RESYNC } migration = NORMAL;


    //
    // Prototypes
    //
    void CmdHandler(U32 pathCrc);
    void Display( Bool simFrame);


	  ///////////////////////////////////////////////////////////////////////////////
	  //
	  // NameSpace Load
	  //
    namespace Load
    {
      void Init();
      void Post();
      void Done();
      Bool Loaded();
    }


	  ///////////////////////////////////////////////////////////////////////////////
	  //
	  // NameSpace SimInit
	  //
    namespace SimInit
    {
      void Init();
      void Post();
      void Process();
      void Done();
    }


	  ///////////////////////////////////////////////////////////////////////////////
	  //
	  // NameSpace Sim
	  //
    namespace Sim
    {
      void Init();
      void Process();
      void Done();
    }


    //
    // Init
    //
    // Initialize Game RunCodes
    //
    void Init()
    {
      ASSERT(!initialized)

      // Setup the runcodes
      runCodes.Register("Load",    NULL,             Load::Init,     Load::Done,    Load::Post);
      runCodes.Register("SimInit", SimInit::Process, SimInit::Init,  SimInit::Done, SimInit::Post);
      runCodes.Register("Sim",     Sim::Process,     Sim::Init,      Sim::Done);

      // Register the command handler
      VarSys::RegisterHandler("sys.game", CmdHandler);
      
      VarSys::CreateCmd("sys.game.runcode");

      VarSys::RegisterHandler("sys.game.migrate", CmdHandler);
      VarSys::CreateCmd("sys.game.migrate.start");
      VarSys::CreateCmd("sys.game.migrate.resync");
      VarSys::CreateCmd("sys.game.migrate.end");

      // System now initialized
      initialized = TRUE;
    }


    //
    // Done
    //
    // Shutdown Game RunCodes
    //
    void Done()
    {
      ASSERT(initialized)

      // Destroy Commands
      VarSys::DeleteItem("sys.game");

      // Clean up the runcodes
      runCodes.Cleanup();

      // System now shutdown
      initialized = FALSE;
    }


    //
    // Set
    //
    // Set the runcode
    //
    void Set(const char *rc)
    {
      runCodes.Set(rc);
    }


    //
    // In
    //
    // Check the runcode
    //
    Bool In(const char *rc)
    {
      return (!Utils::Strcmp(rc, runCodes.GetCurrent()));
    }


    //
    // Clear
    //
    // Clear the runcode
    //
    void Clear()
    {
      runCodes.Clear();
    }


    //
    // Process
    //
    // Process the runcode
    //
    void Process()
    {
      runCodes.Process();
    }


    //
    // IsClear
    //
    // Is the current runcode NULL
    //
    Bool IsClear()
    {
      return (runCodes.IsClear());
    }


    //
    // CmdHandler
    //
    void CmdHandler(U32 pathCrc)
    {
      switch (pathCrc)
      {
        case 0x43382C8E: // "sys.game.runcode"
        {
          const char *s;
          if (Console::ArgCount() == 2 && Console::GetArgString(1, s))
          {
            Set(s);
          }
          else
          {
            CON_ERR((Console::ARGS))
          }
          break;
        }

        case 0xBB563A77: // "sys.game.migrate.start"
          SYNC("Migration commencing")

          IFace::Activate("|Game::Resync");
          migration = MIGRATING;
          LOG_DIAG(("Switching to MIGRATING"))
          break;

        case 0x7CE02766: // "sys.game.migrate.resync"
          if (migration == MIGRATING)
          {
            migration = RESYNC;
            LOG_DIAG(("Switching to RESYNC"))
            MultiPlayer::SetReady();
          }
          break;

        case 0x7D690127: // "sys.game.migrate.end"
        {
          SYNC("Migration completed")

          // Change runcode to the sim
          IControl *ctrl = IFace::FindByName("|Game::Resync");
          if (ctrl)
          {
            IFace::SendNotify(ctrl, NULL, 0x334DAB78); // "ResyncComplete"
            IFace::Deactivate(ctrl);
          }
          MultiPlayer::ClearReady();
          migration = NORMAL;
          LOG_DIAG(("Switching to NORMAL"))
          break;
        }

        default:
          break;
      }
    }


    //
    // Display
    //
    void DisplayMirror()
    {
      if (!Vid::Mirror::Start())
      {
        return;
      }

      while (U32 val = Vid::Mirror::LoopStart())
      {
        Terrain::Sky::Render();

//      if (val > 1)
        {
          // test for clip against mirror plane 
          //
          MapObjCtrl::BuildMirrorList();

          // Render
          PERF_S("Terrain::Render");

          if (Vid::renderState.status.mirTerrain)
          {
            Terrain::Render();
          }
          Vid::FlushBuckets();
          PERF_E("Terrain::Render");

          PERF_S("MapObjCtrl::Render");
          if (Vid::renderState.status.mirObjects)
          {
            MapObjCtrl::RenderMirror();
          }
          Vid::FlushBuckets();
          PERF_E("MapObjCtrl::Render");

          PERF_S("Particles::Render");
          if (Vid::renderState.status.mirParticles)
          {
            ParticleSystem::Render(Vid::CurCamera());
          }
          PERF_E("Particles::Render");
        }

        Vid::Mirror::LoopStop();
      }
      Vid::Mirror::Stop();
    }


    //
    // Display
    //
    void Display( Bool simFrame)
    {
      Vid::RenderBegin();
      Vid::RenderClear();

      if (Vid::Config::TrilinearOff())
      {
        // turn trilinear back on for terrain

        U32 filter = *Vid::Var::varFilter ? Vid::filterFILTER : 0;
        if (*Vid::Var::varMipmap)
        {
          filter |= Vid::filterMIPMAP;
        }
        if (*Vid::Var::varMipfilter)
        {
          filter |= Vid::filterMIPFILTER;
        }
        Vid::SetFilterStateI( filter);
      }

      if (Vid::renderState.status.mirror)
      {
        DisplayMirror();
      }

      // Inform client
      PERF_S("BuildDisplayList")
      MapObjCtrl::BuildDisplayList(Team::GetDisplayTeam(), simFrame);
      PERF_E("BuildDisplayList")

      // Render
      PERF_S("Terrain::Render");
      Terrain::Sky::Render();
      Client::Display::PreTerrain();
      Terrain::Render();
      Environment::Render();
      Vid::FlushBuckets();
      PERF_E("Terrain::Render");

      if (Vid::Config::TrilinearOff())
      {
        // turn trilinear filtering off for models

        Vid::SetFilterStateI( Vid::renderState.status.filter & ~Vid::filterMIPFILTER);
      }

      PERF_S("MapObjCtrl::Render");
      MapObjCtrl::Render();
      Vid::FlushBuckets();
      PERF_E("MapObjCtrl::Render");

      PERF_S("Particles::Render");
      ParticleSystem::Render(Vid::CurCamera());
      PERF_E("Particles::Render");

      PERF_S("Client::Render");
      Client::Display::Render();
      PERF_E("Client::Render");
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // NameSpace Load
    //
    namespace Load
    {

      ///////////////////////////////////////////////////////////////////////////////
      //
      // Internal Data
      //
      static Bool loaded = FALSE;


      //
      // Init
      //
      void Init()
      {
        /*
        // Set the correct video mode
        if (Vid::isStatus.fullScreen)
        {
          Vid::SetMode(Vid::shellMode);
        }
        */

        // Increment Games Played
        User::SampleStatCount("GamesPlayed");

        // Initialize Vid Resources
        Vid::InitResources();

        // Clear the screen
        Vid::ClearBack();

      }


      //
      // Post
      //
      void Post()
      {
        // Repaint the interface
        //IFace::PaintAll();

        // Reset FPU
        Utils::FP::Reset();

        // Notify save game of position
        SaveGame::Notify(0x84191523); // "Load::Game::RC::Load::Pre"

        // Apply settings from multiplayer setup
        if (MultiPlayer::Data::HaveData())
        {
          MultiPlayer::SetupPreMission();
        }

        // Load currently selected mission
        if (!MissionPreLoaded())
        {
          PreOpenMission("interface_game_load.cfg");
        }
        else
        {
          Main::Exec("interface_game_load.cfg", Main::ScopeHandler, FALSE);
        }

        // Notify save game of position
        SaveGame::Notify(0xC1280824); // "Load::Game::RC::Load::Post"

        // Apply settings from multiplayer setup
        if (MultiPlayer::Data::HaveData())
        {
          MultiPlayer::SetupMission();
        }

        OpenMission();

        // Apply post load settings from multiplayer setup
        if (MultiPlayer::Data::HaveData())
        {
          MultiPlayer::SetupPostMission();
        }

        // Set loaded flag
        loaded = TRUE;

        // Setup demo mode if necessary
        Demo::PostLoad();

        // Flush the memory cache
        //Debug::Memory::FlushCache();

        // reset file system for new saved god files
        FileSys::BuildIndexes();

        // Change runcode to the sync
        RC::Set("SimInit");
      }


      //
      // Done
      //
      void Done()
      {
        // Kill all interface controls
        IFace::DisposeAll();
      }


      //
      // Loaded
      //
      Bool Loaded()
      {
        if (loaded)
        {
          loaded = FALSE;
          return (TRUE);
        }
        else
        {
          return (FALSE);
        }
      }
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // NameSpace SimInit
    //
    namespace SimInit
    {

      //
      // Message box window
      //
      static IControlPtr windowPtr;


      //
      // Init
      //
      void Init()
      {
        /*
        // Set the correct video mode
        if (Vid::isStatus.fullScreen)
        {
          Vid::SetMode(Vid::gameMode);
        }
        */

        // Clear the migration status
        migration = NORMAL;
        LOG_DIAG(("Switching to NORMAL"))

        // Bring the client online
        Client::Init();

        // Tell ai we're starting the sim
        AI::InitSimulation();

        // Process Environment
        //Environment::Process();

        // Perform first rendering of sight
        //Sight::UpdateDisplay(Team::GetDisplayTeam());

        //Terrain::Simulate(GameTime::SimTime());

//        MapObjCtrl::SimulateSim(GameTime::SimTime());
        MapObjCtrl::UpdateMapPos();

        // Do display
        if (Vid::isStatus.active)
        {
          // These need to be done or the initial render gets incorrect values
          Client::Display::PreRender();
          Display( TRUE);

          // Post Render
          Vid::RenderEnd();

          // Blank out the first frame
          Vid::ClearBack();
          Vid::RenderFlush();
        }

        // If we're in multiplayer, create the synchronizing control
        if (!MultiPlayer::Data::Online())
        {
          RC::Set("Sim");
        }

        // Notify demo that sim is starting
        Demo::InitSimulation();
      }


      //
      // Post
      //
      void Post()
      {
        // Notify the client that configs are processed
        Client::Prepare();

        // If we're in multiplayer, create the synchronizing control
        if (MultiPlayer::Data::Online())
        {
          IFace::Activate("|Game::Synchronizing");
        }

        // start up movies
        Bitmap::Manager::MovieFirstStart();
      }


      //
      // Process
      //
      void Process()
      {
        // If we're online proceed
        if (MultiPlayer::Data::Online())
        {
          // Process pending messages
          GameTime::Cycle(TRUE);

          // Process network messages
          MultiPlayer::Process();

          // Are we stil online
          if (MultiPlayer::Data::Online())
          {
            // Repaint interface only
            IFace::PaintAll();

            // If all players are ready to start then start the simulation
            if (MultiPlayer::IsReady())
            {
              // Change runcode to the sim
              RC::Set("Sim");
            }
            else

            // If there is nothing in the queue then send an order indicating that we are ready
            if (!MultiPlayer::Data::GetLag())
            {
              MultiPlayer::SetReady();
            }

            // Process input events
            if (Main::active)
            {
              IFace::Process();
            }
            return;
          }
        }
        RC::Set("Sim");
      }


      //
      // Done
      //
      void Done()
      {
        IFace::Deactivate("|Game::Synchronizing");

        // If the next runcode isn't Sim then go through the motions of Sim
        if (runCodes.GetNextCrc() != 0xBE9A9686) // "Sim"
        {
          LOG_DIAG(("Leaving SimInit and not going to Sim, cycling Sim::Init & Sim::Done"))
          Sim::Init();
          Sim::Done();
        }
      }
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // NameSpace Sim
    //
    namespace Sim
    {

      // Was game paused by modal control?
      static Bool modalPause;

      // Is modal pause allowed
      static Bool allowModalPause;


      //
      // Init
      //
      void Init()
      {
        modalPause = FALSE;

        // Only allow modal pausing if the session started in multiplayer
        allowModalPause = !MultiPlayer::Data::Online() && (Missions::GetActive() ? !Missions::GetActive()->IsSystem() : TRUE);

        #ifdef DEVELOPMENT

          // Turn on watchdog (if enabled)
          Debug::Watchdog::Create();

        #endif

        if (!Load::Loaded())
        {
          ERR_FATAL(("Attempt to load the Simulation without loading a map"))
        }

        // Exec mod files
        Mods::ExecFiles();

        // Reset the game time
        GameTime::Reset();

        // Ensure the shroud is being displayed
        Vid::Var::Terrain::shroud = TRUE;

        // Notify save game (before environment and team processing)
        SaveGame::Notify(0x4685E44B); // "Load::Game::RC::Sim::Init"

        // Update environment
        Environment::Process();

        // Perform first rendering of sight
        Sight::UpdateDisplay(Team::GetDisplayTeam(), TRUE, TRUE);

        // Setup initial object fogging values
        MapObjCtrl::SetObjectFogging(Team::GetDisplayTeam());

        Terrain::Simulate(GameTime::SimTime());

        // Generate time message
        //Environment::Time::GenerateMessage();

        // Reset sync
        Sync::Reset();

        // Force processing of all teams, will allow objectives/cineractives to
        // start on the first game cycle
        //
        Team::ProcessAll(TRUE);

        // set up first frame
        //
        MapObjCtrl::UpdateMapPos();

        // Notify save game that the load is completed
        SaveGame::Notify(0x0272D3DA); // "Load::Completed"

        // We are now in the simulation
        CoreGame::SetInSimulation(TRUE);
      }


      //
      // Process
      //
      void Process()
      {
        switch (migration)
        {
          case NORMAL:
          {

            // Sample game speed
            frameRate.Sample(Main::elapTime);

            // Process the client
            Client::Events::Process();

            // Process real time camera things independent of display
            Viewer::GameTimeSim();

            if (Viewer::movie)
            {
              GameTime::Test();
              PERF_S("RenderFlush");
              Vid::RenderFlush();
              PERF_E("RenderFlush");
              return;
            }

            // Should we display or skip this frame
            Bool display = Vid::isStatus.active && GameTime::DisplayCount();
            Bool disPost = *Vid::Var::varRenderPostSim && display;
            U32 displayTime = 0;

            if (display)
            {
              Client::Display::PreRender();

              if (!*Vid::Var::varRenderPostSim)
              {
                PERF_S("Render");
                {
                  U32 start = Clock::Time::UsLwr();
                  Display( TRUE);
                  displayTime += Clock::Time::UsLwr() - start;

                  // Post render parallel code - NON SYNC
                  if (!GameTime::Paused())
                  {
                    Terrain::Simulate( Main::elapSecs);
                  }
                }
                PERF_E("Render");
              }
            }

            // update auto mrm error factor
            //
            //Mesh::Manager::UpdateMRMFactor();

            // Pause when a modal control is active
            if (allowModalPause)
            {
              if (IFace::GetModal())
              {
                if (!GameTime::Paused())
                {
                  GameTime::Pause(FALSE);
                  modalPause = TRUE;
                }
              }
              else
              {
                if (GameTime::Paused() && modalPause)
                {
                  GameTime::Pause(FALSE);
                  modalPause = FALSE;
                }
              }
            }

            // Reset mode BEFORE GameTime::Test
            Utils::FP::Reset();

            // Is it time to do the next game processing
            U32 gameTest = GameTime::Test();

            if (gameTest)
            {
              SYNC_BRUTAL("Mode: " << Utils::FP::GetState() << " Random: " << Random::sync.Raw())

              // Blip an order
              #ifdef SYNC_BRUTAL_ACTIVE
                if (!Missions::ShellActive())
                {
                  Orders::SendBlip();
                }
              #endif

              #ifdef SYNC_BRUTAL_ACTIVE
                SYNC(" === SIM START ===")
                Sync::SyncObjects(TRUE);
                SYNC(" ___ SIM START ___")
              #endif

              PERF_S("Simulation");

              // prepare state0 world matrices and state1 anim targets for this frame
              //
              PERF_S("UpdateMapPos");
              MapObjCtrl::UpdateMapPos();
              PERF_E("UpdateMapPos");

              PERF_S("SimulateInt");
              MapObjCtrl::SimulateInt( GameTime::SimTime());
              PERF_E("SimulateInt");

              SYNC_BRUTAL("Mode: " << Utils::FP::GetState())
            }

            if (disPost)
            {
              PERF_S("Render");
              U32 start = Clock::Time::UsLwr();
              Display( gameTest);
              displayTime += Clock::Time::UsLwr() - start;

              // Post render parallel code - NON SYNC
              if (!GameTime::Paused())
              {
                Terrain::Simulate( Main::elapSecs);
              }
              PERF_E("Render");
            }

            if (gameTest)
            {
              // Reset mode
              Utils::FP::Reset();

              SYNC_BRUTAL("Mode: " << Utils::FP::GetState())

              // Sync vars which should be synced
              SYNC
              (
                Vid::Var::Terrain::shroud
              )

              // Process Environment
              PERF_S("Environment");
              Environment::Process();
              PERF_E("Environment");

              // Perform AI Processing
              PERF_S("AI");
              AI::Process();
              PERF_E("AI");

              // Perform Game Object Thinking
              PERF_S("ObjectThought");
              GameObjCtrl::ProcessObjectThought();
              PERF_E("ObjectThought");

              // FX processing
              // MUST be before particle and mesheffect processing
              // MUST be before GameObjCtrl::DeleteDyingObjects
              // MUST be after ProcessObjectThought
              //
              PERF_S("FX");
              FX::Process();
              PERF_E("FX");

              GameObjCtrl::DeleteDyingObjects();

              // Perform Path Searching
              PERF_S("PathSearching");
              PathSearch::ProcessRequests();
              PERF_E("PathSearching");

              // Movement collision resolution
              PERF_S("Movement::Mediate");
              Movement::Mediator::Process();
              PERF_E("Movement::Mediate");

              // Do all per-cycle object processing
              MapObjCtrl::ProcessObjects();

              // Perform collision fixups
              //
              // this resets the world matrices
              // should be after all object position updates
              //
              PERF_S("Collision Resolve");
              CollisionCtrl::Resolve();
              PERF_E("Collision Resolve");

              // Perform Particle Processing
              PERF_S("ParticleSimulate");
              ParticleSystem::Simulate( GameTime::SimTime());
              ParticleSystem::SimulateInt( Main::elapSecs);
              PERF_E("ParticleSimulate");

              // Perform Team Processing
              PERF_S("Team");
              Team::ProcessAll();
              PERF_E("Team");

              #ifdef SYNC_BRUTAL_ACTIVE
                SYNC(" === SIM END ===")
                Sync::SyncObjects(TRUE);
                SYNC(" ___ SIM END ___")
              #endif

              SYNC_BRUTAL("Mode: " << Utils::FP::GetState())

              // Update line of sight display
              PERF_S("Sight::DetachedList");
              Sight::ProcessDetachedList();
              PERF_E("Sight::DetachedList");

              // Update line of sight display
              PERF_S("Sight Display");
              Sight::UpdateDisplay( Team::GetDisplayTeam());
              PERF_E("Sight Display");

              // Tell game time to recompute the display cycles per game cycle
              GameTime::Compute();

              // NON-SYNC client side processing that is done at the sim rate
              PERF_S("Demo");
              Demo::Process();
              PERF_E("Demo");

              PERF_E("Simulation");
            }
            else 
        
            // Process interpolation
            if (Vid::renderState.status.interpMesh && !GameTime::Paused() && !GameTime::IsStalled())
            {
              PERF_S("Interpolate");
              MapObjCtrl::SimulateInt( Main::elapSecs);
              PERF_E("Interpolate");

              if (disPost)
              {
                PERF_S("Render");

                U32 start = Clock::Time::UsLwr();
                Display( FALSE);
                displayTime += Clock::Time::UsLwr() - start;

                // Post render parallel code - NON SYNC
                if (!GameTime::Paused())
                {
                  Terrain::Simulate( Main::elapSecs);
                }
                PERF_E("Render");
              }

              ParticleSystem::SimulateInt( Main::elapSecs);
            }

            // Main rendering code
            if (display)
            {
              PERF_S("PostRender");

              U32 start = Clock::Time::UsLwr();

              // Finish render
              Vid::RenderEnd();

              PERF_S("RenderFlush");
              Vid::RenderFlush();
              PERF_E("RenderFlush");

              displayTime += Clock::Time::UsLwr() - start;

              PERF_E("PostRender");

              // Notify client that rendering is finished
              Client::Display::PostRender();

              // Report display time to GameTime
              GameTime::DisplaySample(displayTime);

              displayRate.Sample(displayTime / 1000);
              triCount.Sample(Main::triCount);
            }
            break;
          }

          case MIGRATING:

            if (!MultiPlayer::Data::Online())
            {
              migration = NORMAL;
              LOG_DIAG(("Switching to NORMAL"))
              return;
            }

            // Process input events
            if (Main::active)
            {
              IFace::Process();
            }

            if (!MultiPlayer::Data::Online())
            {
              migration = NORMAL;
              LOG_DIAG(("Switching to NORMAL"))
              return;
            }

            // Process network messages
            MultiPlayer::Process();

            if (!MultiPlayer::Data::Online())
            {
              migration = NORMAL;
              LOG_DIAG(("Switching to NORMAL"))
              return;
            }

            // Repaint interface only
            IFace::PaintAll();

            break;

          case RESYNC:

            if (!MultiPlayer::Data::Online())
            {
              migration = NORMAL;
              LOG_DIAG(("Switching to NORMAL"))
              return;
            }

            // Process input events
            if (Main::active)
            {
              IFace::Process();
            }

            if (!MultiPlayer::Data::Online())
            {
              migration = NORMAL;
              LOG_DIAG(("Switching to NORMAL"))
              return;
            }

            // Process network messages
            MultiPlayer::Process();

            if (!MultiPlayer::Data::Online())
            {
              migration = NORMAL;
              LOG_DIAG(("Switching to NORMAL"))
              return;
            }

            // Repaint interface only
            IFace::PaintAll();
            break;
        }
      }


      //
      // Done
      //
      void Done()
      {
        LOG_DIAG(("Shutting down simulation"))

        // No longer in simulation
        CoreGame::SetInSimulation(FALSE);

        // Turn off watchdog
        Debug::Watchdog::Delete();

        // Tell demo the sim is finishing
        Demo::DoneSimulation();

        // Shutdown the client
        Client::Done();

        // Tell AI we're stopping the sim
        AI::DoneSimulation();

        CloseMission();

        // Sample the amount of time we played
        User::SampleStatStat("TimePlayed", GameTime::SimTotalTime());

        // Reset Orders system
        Orders::Reset();

        // Shutdown Interface
        IFace::DisposeAll();

        // Dispose of all Vid resources
        Vid::DisposeAll();
      }
    }


    //
    // Get average framerate of the Game
    //
    F32 AvgFrameRate()
    {
      return (frameRate.GetSamples() ? (1000.0F / F32(frameRate.GetAvg())) : 0.0F);
    }


    //
    // Get average display rate of the game
    //
    F32 AvgDisplayRate()
    {
      return (displayRate.GetSamples() ? (1000.0F / F32(displayRate.GetAvg())) : 0.0F);
    }


    //
    // Get average triangle count
    //
    U32 AvgTriCount()
    {
      return (triCount.GetAvg());
    }

    //
    // Get total triangle count
    U32 TriCount()
    {
      return U32(triCount.GetSum());
    }

    //
    // Reset framerate
    //
    void ResetFrameRate()
    {
      displayRate.Reset();
      frameRate.Reset();
      triCount.Reset();
    }
  }
}
