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
#include "vid_private.h"

#include "studio_private.h"
#include "iface.h"
#include "gameobjctrl.h"
#include "mapobjctrl.h"
#include "main.h"
#include "viewer.h"
#include "mapobj.h"
#include "common.h"
#include "sight.h"
#include "environment.h"
#include "particlesystem.h"
#include "common_mapwindow.h"
#include "perfstats.h"

///////////////////////////////////////////////////////////////////////////////
//
// Namespace Studio - Mission creation environment
//
namespace Studio
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Display - Studio rendering
  //
  namespace Display
  {
    // System initialized flag
    static Bool studioInit = FALSE;

    // Sim initialized flag
    static Bool simInit = FALSE;


    //
    // Init
    //
    // Studio initialization
    //
    void Init()
    {
      ASSERT(!studioInit);
      ASSERT(!simInit);

      // Set the correct video mode 
      /*
      if (Vid::isStatus.fullScreen)
      {
        Vid::SetMode(Vid::CurDD().gameMode);
      }
      */

      // Initialize Vid Resources
      Vid::InitResources();

      // Load render resources
      //Render::LoadResources();

      // Clear the screen
      Vid::ClearBack();

      // Initialize the viewer
      Viewer::Init();

      // Set init flag
      studioInit = TRUE;
    }


    //
    // Done
    //
    // Studio shutdown
    //
    void Done()
    {
      ASSERT(!simInit);
      ASSERT(studioInit);

      // Shutdown the viewer
      Viewer::Done();

      // Remove all interface items
      IFace::DisposeAll();

      // Clear all Vid data
      Vid::DisposeAll();

      // Load render resources
      //Render::DeleteResources();

      // Clear init flag
      studioInit = FALSE;
    }


    //
    // InitSim
    //
    // Simulation initialization
    //
    void InitSim()
    {
      ASSERT(studioInit);
      ASSERT(!simInit);

      // Initialise external systems
      Common::InitSim();

      // Initialise map window
      Common::MapWindow::Init();

      // Refresh the interface components (rebind vars etc)
      IFace::Refresh();
   
      // Turn off the shroud
      Vid::Var::Terrain::shroud = FALSE;

      // Set init flag
      simInit = TRUE;
    }


    //
    // DoneSim
    //
    // Simulation shutdown
    //
    void DoneSim()
    {
      ASSERT(studioInit);
      ASSERT(simInit);

      // Clear game window event handler
      Common::MapWindow::Done();

      // Shutdown external systems
      Common::DoneSim();

      // Clear init flag
      simInit = FALSE;
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
        // test for clip against mirror plane 
        //
        MapObjCtrl::BuildMirrorList();

        Terrain::Sky::Render();

//     if (val > 1)
        {
          PERF_S("Terrain::Render");
          if (Vid::renderState.status.mirTerrain)
          {
            Terrain::Render();
          }
          if (Vid::renderState.status.mirEnvironment)
          {
            Environment::Render();
          }
          Vid::FlushBuckets();
          PERF_E("Terrain::Render");

          PERF_S("MapObjCtrl::Render");
          if (Vid::renderState.status.mirObjects)
          {
            MapObjCtrl::RenderMirror();
          }
          PERF_E("MapObjCtrl::Render");
        }

        Vid::Mirror::LoopStop();
      }

      Vid::Mirror::Stop();
    }


    //
    // Display
    //
    void Display()
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

      MapObjCtrl::BuildDisplayList( NULL, TRUE);

      PERF_S("Terrain::Render");
      Terrain::Sky::Render();
      Terrain::Render();
      Environment::Render();
      Vid::FlushBuckets();
      PERF_E("Terrain::Render");

      if (Vid::Config::TrilinearOff())
      {
        // turn trilinear filtering off for models

        Vid::SetFilterStateI( Vid::renderState.status.filter & ~Vid::filterMIPFILTER);
      }

      MapObjCtrl::Render();

      PERF_S("Flush buckets");
      Vid::FlushBuckets();    
      PERF_E("Flush buckets");

      ParticleSystem::Render(Vid::CurCamera());

      // Display debug information
      #ifdef DEVELOPMENT

      Common::Display::RenderDebug();

      #endif

      // Notify the brush to do all predrawing
      data.brush->Notify("System::PreDraw");

      PERF_S("Flush buckets");
      Vid::FlushBuckets();
      Vid::FlushTranBuckets();
      PERF_E("Flush buckets");

      // Render interface
      IFace::Render();

      // Notify the brush to do all postdrawing
      data.brush->Notify("System::PostDraw");

      // Finish 3D rendering
      Vid::RenderEnd();

    }

    //
    // Process
    //
    // Single frame of display processing
    //
    void Process()
    {
      ASSERT(studioInit);
      ASSERT(simInit);

      // Are we over a game window
      if (data.cInfo.gameWindow)
      {
        // Set the cursor using the current brush
        data.cInfo.gameWindow->SetCursor(data.brush->Cursor());
      }

      // Setup the cursor based on current control
      IFace::ProcessCursor();

      Viewer::Simulate();

      // render
      //
      Display();

      // Page flip
      Vid::RenderFlush();
    }
  }
}
