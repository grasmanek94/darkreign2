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
#include "input.h"
#include "iface.h"
#include "iface_types.h"
#include "common_gamewindow.h"
#include "unitobj.h"
#include "promote.h"
#include "gameobjctrl.h"
#include "mapobjlist.h"
#include "unitobjlist.h"
#include "terraindata.h"
#include "common.h"
#include "environment.h"
#include "pathsearch.h"
#include "mapobjctrl.h"
#include "main.h"
#include "fx.h"
#include "particlesystem.h"
#include "sight.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Studio - Mission creation environment
//
namespace Studio
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Event - Studio event handling
  //
  namespace Event
  {
    // System initialized flag
    static Bool studioInit = FALSE;

    // Sim initialized flag
    static Bool simInit = FALSE;


    //
    // UseBrush
    // 
    // Set the current brush
    //
    void UseBrush(const char *name, Bool required)
    {
      // Get the new brush
      Brush::Base *newBrush = Brush::Get(name);

      // Do we have a current brush
      if (data.brush)
      {
        // Is this already the current brush
        if (data.brush == newBrush)
        {
          // Notify the brush so it can do funky stuff
          data.brush->Notify("System::DuplicateActivate");

          // Brush is already active
          return;
        }

        // Are we losing capture
        if (data.brush->HasCapture())
        {
          data.brush->Notify("System::LostCapture");
        }

        // Notify of deactivation
        data.brush->Notify("System::Deactivated");
      }

      // Set the new brush
      data.brush = newBrush;

      // Notify of activation
      if (data.brush)
      {
        data.brush->Notify("System::Activated");
      }
      else

      // Was a brush required
      if (required)
      {
        ERR_FATAL(("Brush::Get(%s) returned NULL (brush required)", name ? name : "NULL"));
      }
    }

  
    //
    // GetGameWindow
    //
    // Returns pointer to the game window the mouse is currently over, or NULL
    //
    static Common::GameWindow * GetGameWindow()
    {
      // Always return NULL if a modal window is active
      if (!IFace::GetModal())
      {
        // Get the appropriate window
        IControl *ctrl = data.brush->HasCapture() ? IFace::GetCapture() : IFace::GetMouseOver();

        // Promote to a game window
        return (ctrl ? IFace::Promote<Common::GameWindow>(ctrl) : NULL);
      }

      return (NULL);
    }


    //
    // SetupCommonCycle
    //
    // Setup common cycle info
    //
    static void SetupCommonCycle()
    {
      // Start the common cycle
      Common::Cycle::Start();

      // Clear the current selection
      Common::Cycle::ClearSelected();

      // Add each selected object
      for (MapObjList::Iterator i(&data.sList); *i; i++)
      {
        Common::Cycle::AddSelected(**i);
      }

      // Are we over a game window
      if (data.cInfo.gameWindow)
      {
        // Set the object under the mouse
        Common::Cycle::SetMouseOverObject(data.cInfo.mObject.mapObj);
      }

      // End the common cycle
      Common::Cycle::End();
    }


    //
    // SetupCycleInfo
    //
    // Generate info for this processing cycle
    //
    void SetupCycleInfo()
    {
      // Grab a shortcut
      CycleInfo &i = data.cInfo;

      // Should not be called more than once per cycle
      ASSERT(data.cycleCount != data.lastUpdate)

      // Save the current cycle
      data.lastUpdate = data.cycleCount;

      // Ensure all selected objects are alive and on the map
      MapObjListUtil::PurgeOffMap(data.sList);

      // Get the current screen position of the mouse
      i.mouse = Input::MousePos();

      // Setup information if over a game window 
      if ((i.gameWindow = GetGameWindow()) != NULL)
      {
        // See if mouse is over an object
        i.mObject.mapObj = i.gameWindow->PickObject
        (
          i.mouse.x, i.mouse.y, data.brush->GetSelectionFilter()
        );

        // Promote to a unit
        i.mObject.unitObj = i.mObject.mapObj ? 
          Promote::Object<UnitObjType, UnitObj>(i.mObject.mapObj) : NULL;

        // Set the Mesh::Manager hook
        if (i.mObject.mapObj)
        {
          Mesh::Manager::curEnt = &i.mObject.mapObj->Mesh();
          Mesh::Manager::curParent = Mesh::Manager::curEnt->FindMeshEnt();
        }

        // See if mouse is over the terrain
        if (i.gameWindow->TerrainPosition(i.mouse.x, i.mouse.y, i.mTerrain.pos))
        {
          ASSERT(WorldCtrl::MetreOnMap(i.mTerrain.pos.x, i.mTerrain.pos.z));

          // If so, setup additional information

          // Calculate the cell position on the terrain
          i.mTerrain.cellX = WorldCtrl::MetresToCellX(i.mTerrain.pos.x);
          i.mTerrain.cellZ = WorldCtrl::MetresToCellZ(i.mTerrain.pos.z);
          i.mTerrain.cell = Terrain::GetCell( i.mTerrain.cellX, i.mTerrain.cellZ);

          // And get a pointer to the actual map cell
          i.mTerrain.dataCell = &TerrainData::GetCell(i.mTerrain.cellX, i.mTerrain.cellZ);
        }
        else
        {
          // Otherwise clear the pointers
          i.mTerrain.cell = NULL;
        }
      }

      // Notify the brush that the cycle info is now setup
      data.brush->Notify("System::PostCycleInfoPoll");
    } 


    //
    // GameWindowHandler
    //
    // Event handler for any active game window
    //
    static Bool FASTCALL GameWindowHandler(::Event &e, Common::GameWindow *gameWindow)
    {
      ASSERT(gameWindow);

      // Input events
      if (e.type == Input::EventID())
      {
        switch (e.subType)
        {
          // A mouse button has been pressed
          case Input::MOUSEBUTTONDOWN:
          case Input::MOUSEBUTTONDBLCLK:
          {
            // If we do not already have mouse capture
            if (gameWindow != IFace::GetCapture())
            {
              // Get mouse capture
              gameWindow->GetMouseCapture();

              // Save mouse code
              data.captureCode = e.input.code;

              // Left mouse button down
              if (data.captureCode == Input::LeftButtonCode())
              {
                data.brush->Notify("Input::LeftMouseDown", &e);
              }
              else

              // Middle mouse button down
              if (data.captureCode == Input::MidButtonCode())
              {
                data.brush->Notify("Input::MiddleMouseDown", &e);
              }
              else

              // Right mouse button down
              if (data.captureCode == Input::RightButtonCode())
              {
                data.brush->Notify("Input::RightMouseDown", &e);
              }
            }

            return TRUE;
          }

          // The mouse has moved
          case Input::MOUSEMOVE:
          {
            // Notify the current brush
            data.brush->Notify("Input::MouseMove", &e);
            return (TRUE);
          }

          // A mouse button has been released
          case Input::MOUSEBUTTONUP:
          case Input::MOUSEBUTTONDBLCLKUP:
          {
            // We have capture and it's the button that got the capture
            if (gameWindow == IFace::GetCapture() && (e.input.code == data.captureCode))
            {
              // Left mouse button up
              if (data.captureCode == Input::LeftButtonCode())
              {
                data.brush->Notify("Input::LeftMouseUp", &e);
              }
              else

              // Middle mouse button up
              if (data.captureCode == Input::MidButtonCode())
              {
                data.brush->Notify("Input::MiddleMouseUp", &e);
              }
              else

              // Right mouse button up
              if (data.captureCode == Input::RightButtonCode())
              {
                data.brush->Notify("Input::RightMouseUp", &e);
              }
            
              gameWindow->ReleaseMouseCapture();
            }

            return (TRUE);
          }

          // Mouse-wheel axis has changed
          case Input::MOUSEAXIS:
          {
            // Notify the current brush
            data.brush->Notify("Input::MouseAxis", &e);
            return (TRUE);
          }
        }
      }
      else 
    
      // Interface events
      if (e.type == IFace::EventID())
      {
        switch (e.subType)
        {
          case IFace::LOSTCAPTURE:
          {
            // Are we losing brush capture
            if (data.brush->HasCapture())
            {
              data.brush->Notify("System::LostCapture");
            }
            return (TRUE);
          }
        }
      }

      return (FALSE);
    }


    //
    // Init
    //
    // Studio initialization
    //
    void Init()
    {
      ASSERT(!studioInit);
      ASSERT(!simInit);

      // Setup system data
      data.cycleCount = data.lastUpdate = 0;
      data.brush = NULL;
      data.team = NULL;

      // Setup cycle data
      data.cInfo.mouse.x = 0;
      data.cInfo.mouse.y = 0;
      data.cInfo.gameWindow = NULL;

      // Register game window event handler
      Common::GameWindow::RegisterHandler(GameWindowHandler);

      // Create the main game window
      Common::GameWindow *ctrl = new Common::GameWindow(IFace::RootWindow());
      ctrl->SetName("MainWindow");
      ctrl->SetGeometry("ParentWidth", "ParentHeight", NULL);
      ctrl->SetZPos(0xFFFFFFFF);
      ctrl->Activate(); 

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

      // Release the game window handler
      Common::GameWindow::RegisterHandler(NULL);

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

      // Set the default brush
      UseBrush(NULL);
     
      // Set init flag
      simInit = TRUE;

      // Ensure all units and particles are visible even when no team selected
      Sight::showAllUnits = TRUE;

      // set up first frame
      //
      MapObjCtrl::UpdateMapPos();
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

      // Notify current brush
      if (data.brush->HasCapture())
      {
        data.brush->Notify("System::LostCapture");
      }

      // Clear selected list
      data.sList.Clear();

      // Clear init flag
      simInit = FALSE;
    }


    //
    // Process
    //
    // Single frame of event processing
    //
    void Process()
    {
      ASSERT(studioInit);
      ASSERT(simInit);
      ASSERT(data.brush);

      // update auto mrm error factor
      //
//      Mesh::Manager::UpdateMRMFactor();

      Terrain::Simulate( Main::elapSecs);

      // prepare world matrices for this frame
      //
      MapObjCtrl::UpdateMapPos();
      MapObjCtrl::SimulateInt(Main::elapSecs);

      // Poll the current brush
      data.brush->Notify("System::PreEventPoll");

      // Process effects
      FX::Process();
      ParticleSystem::Simulate(Main::elapSecs);

      // Delete any marked objects
      GameObjCtrl::DeleteDyingObjects();

      // Poll the current brush
      data.brush->Notify("System::PostDeletionPoll");

      // Increment the cycle counter
      data.cycleCount++;

      // Process input and interface events
      IFace::Process(FALSE, SetupCycleInfo);

      // Setup common cycle info
      SetupCommonCycle();

      // Poll the current brush
      data.brush->Notify("System::PostEventPoll");

      // Update game systems
      Environment::Process();

      PathSearch::ProcessRequests();
    }
  }
}
