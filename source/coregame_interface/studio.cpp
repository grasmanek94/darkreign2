///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 11-FEB-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//

#include "studio_private.h"
#include "studio_controls.h"
#include "terrain.h"
#include "iface.h"
#include "game.h"
#include "runcodes.h"
#include "common.h"
#include "environment.h"
#include "perfstats.h"

///////////////////////////////////////////////////////////////////////////////
//
// Namespace Studio - Mission creation environment
//
namespace Studio
{
  // System initialized flag
  static Bool initialized = FALSE;

  // Run code manager
  static RunCodes runCodes("Studio");

  // Has a request been made to load a new mission
  Bool missionRequested;

  // System-wide private data
  SystemData data;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Load - Loads the currently setup mission
  //
  namespace Load
  {
    //
    // Init
    //
    // Open the currently setup mission
    //
    void Init()
    {
      // Clear mission flag
      missionRequested = FALSE;
    }


    //
    // Process
    //
    // Load the game in segments
    //
    void Process()
    {
      // Repaint the interface
      IFace::PaintAll();

      // Ensure mission is pre-loaded
      if (!Game::MissionPreLoaded())
      {
        Game::PreOpenMission();
      }

      // Load currently selected mission
      Game::OpenMission();

      // Enter the sim runcode
      runCodes.Set("Sim");
    }


    //
    // Done
    //
    // Close this runcode
    //
    void Done()
    {
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Sim - Simulation of the studio
  //
  namespace Sim
  {
    //
    // Init
    //
    // Initialize the studio simulation
    //
    void Init()
    {
      // Initialize sub-systems
      Cmd::InitSim();
      Brush::InitSim();
      Event::InitSim();
      Display::InitSim();

      // Clear the display team so we can see everything
      if (Team::GetDisplayTeam())
      {
        Team::ClearDisplayTeam();
      }

      // Start up movies
      Bitmap::Manager::MovieFirstStart();
    }


    //
    // Process
    //
    void Process()
    {
      PERF_S("Studio::Sim")

      // Is a new mission requested
      if (missionRequested)
      {
        // Enter load runcode
        runCodes.Set("Load");
      }
      else
      {
        // Is the main window active
        if (Vid::isStatus.active)
        {
          // Process sub-systems
          PERF_S("Event")
          Event::Process();
          PERF_E("Event")

          PERF_S("Display")
          Display::Process();
          PERF_E("Display")
        }
      }

      PERF_E("Studio::Sim")
    }


    //
    // Done
    //
    void Done()
    {
      // Shutdown sub-systems
      Display::DoneSim();
      Event::DoneSim();
      Brush::DoneSim();
      Cmd::DoneSim();

      // Close the mission
      Game::CloseMission();
      Game::PostCloseMission();
    }
  }

  
  //
  // Init
  //
  // Initialize system
  //
  void Init()
  {
    ASSERT(!initialized);

    // Initialize sub-systems
    Cmd::Init();
    Brush::Init();
    Event::Init();
    Display::Init();
    Controls::Init();

    // Initialize the game
    Game::Init();

    // Setup the available runcodes
    runCodes.Register("Load", Load::Process, Load::Init, Load::Done);
    runCodes.Register("Sim", Sim::Process, Sim::Init, Sim::Done);

    // Enter load runcode
    runCodes.Set("Load");

    // Set init flag
    initialized = TRUE;
  }


  //
  // PostInit
  //
  void PostInit()
  {
    // Create all available brushes (after interface config)
    Brush::CreateBrushes();
  }


  //
  // Done
  //
  void Done()
  {
    ASSERT(initialized);

    // Ensure subruncode is shutdown
    runCodes.Clear();

    // Clean up the runcodes
    runCodes.Cleanup();

    // Shutdown the game
    Game::Done();

    // Shutdown sub-systems
    Controls::Done();
    Display::Done();
    Event::Done();
    Brush::Done();
    Cmd::Done();

    // Clear init flag
    initialized = FALSE;
  }


  //
  // Process
  //
  // Do a single frame of processing
  //
  void Process()
  {
    // Process the current runcode
    runCodes.Process();
  }
}
