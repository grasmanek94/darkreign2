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
#include "studio_brush_objects.h"
#include "studio_brush_terrain.h"
#include "studio_brush_terraintweak.h"
#include "studio_brush_water.h"
#include "studio_brush_region.h"
#include "studio_brush_tag.h"
#include "studio_brush_curve.h"
#include "studio_brush_trail.h"
#include "studio_brush_pathsearch.h"
#include "studio_brush_overlay.h"

#include "input.h"
#include "viewer.h"
#include "common.h"
#include "iface.h"
#include "iface_types.h"
#include "mapobj.h"
#include "promote.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Studio - Mission creation environment
//
namespace Studio
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Brush - Contains all available brushes
  //
  namespace Brush
  {
    //
    // Default object selection filter 
    //
    static MapObj * SelectionFilter(MapObj *obj)
    {
      ASSERT(obj)
      return (obj);
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Base - The base brush class
    //

    //
    // Constructor
    //
    Base::Base(const char *name) : name(name)
    {
      char configName[256];

      // Generate the name of the configuration control
      Utils::Sprintf(configName, 256, "Brush::%s", Name());

      // Find the control (not required)
      config = IFace::Find<BrushConfig>(configName);

      // If we have a config control, tell it to use this brush
      if (config.Alive())
      {
        config->UseBrush(this);
      }
      else
      {
        ERR_FATAL(("Brush '%s' requires the config control '%s'", Name(), configName));
      }

      // Set default capture thresholds for each button
      for (U32 i = 0; i < MOUSE_MAX; i++)
      {
        captureThreshold[i].Set(5, 5);
      }

      // Clear local data
      hasCapture = FALSE;
      captureMouse = CM_OFF;
    }


    //
    // Destructor
    //
    Base::~Base()
    {
      varList.DisposeAll();
    }


    //
    // HasProperty
    //
    // Does brush have the given property
    //
    Bool Base::HasProperty(U32)
    {
      return (FALSE);
    }


    //
    // HasProperty
    //
    // Does brush have the given property
    //
    Bool Base::HasProperty(const char *property)
    {
      return (HasProperty(Crc::CalcStr(property)));
    }


    //
    // NotifyVar
    //
    // Notification that the given interface var has changed
    //
    void Base::NotifyVar(IFaceVar *)
    {
    }


    //
    // Cursor
    //
    // Returns the current cursor for this brush
    //
    U32 Base::Cursor()
    {
      if (captureMouse == CM_ON)
      {
         return (0x5B2A0A5F); // "Null"
      }

      return (0xE2D24FC2); // "StudioDefault"
    }


    //
    // GetSelectionFilter
    //
    // Returns the object selection filter for this brush
    //
    Common::GameWindow::SelectFilter * Base::GetSelectionFilter()
    {
      return (SelectionFilter);
    }


    //
    // MouseNotify
    //
    // Used to generate events based on the current mouse button
    //
    void Base::MouseNotify(const char *event, ::Event *e)
    {
      ASSERT(captureMouse != CM_OFF);

      char buf[256];
      char *mouseName = NULL;

      switch (button)
      {
        case MOUSE_LEFT   : mouseName = "Left"; break;
        case MOUSE_MIDDLE : mouseName = "Middle"; break;
        case MOUSE_RIGHT  : mouseName = "Right"; break;
      }

      ASSERT(mouseName);

      Utils::Sprintf(buf, 256, "Brush::%sMouse%s", mouseName, event);
      Notify(buf, e);
    }


    //
    // CaptureThreshold
    //
    // Returns TRUE if the mouse has moved past current threshold
    //
    Bool Base::CaptureThreshold(const Point<S32> &b)
    {
      ASSERT(captureMouse == CM_INIT);

      Point<S32> &a = mouseStart;
      Point<S32> &t = captureThreshold[button];

      return (abs(a.x - b.x) >= t.x || abs(a.y - b.y) >= t.y);
    }


    //
    // Notification
    //
    // Called when a brush event is generated
    //
    void Base::Notification(U32 crc, ::Event *e)
    {
      switch (crc)
      {
        //
        // Core Event Handling
        //

        case 0xD47619B1: // "Input::LeftMouseDown"
        case 0x3C955069: // "Input::MiddleMouseDown"
        case 0xA463EA4C: // "Input::RightMouseDown"
        {
          // Setup the name of the mouse button
          switch (crc)
          {
            case 0xD47619B1: // "Input::LeftMouseDown"
              button = MOUSE_LEFT; 
              SaveMousePosition(e);
              break;
            case 0x3C955069: // "Input::MiddleMouseDown"
              button = MOUSE_MIDDLE; break;
            case 0xA463EA4C: // "Input::RightMouseDown"
              button = MOUSE_RIGHT;  break;
          }

          captureMouse = CM_INIT;

          // Immediately check mouse threshold
          Notify("Brush::CheckThreshold", e);
          break;
        }

        case 0xFA75C2F5: // "Input::MouseMove"
        case 0x621B4478: // "Brush::CheckThreshold"
        {
          // Update mouse capture
          switch (captureMouse)
          {
            // Are we starting an object selection
            case CM_INIT:
            {
              // Have we moved the mouse far enough
              if (CaptureThreshold(data.cInfo.mouse))
              {
                captureMouse = CM_ON;
                MouseNotify("CaptureStart", e);
              }
              break;
            }

            // Currently doing object selection
            case CM_ON:
              MouseNotify("CaptureMove", e);
              break;
          }
          break;
        }

        case 0xC74F389F: // "Input::LeftMouseUp"
        case 0x765CF149: // "Input::MiddleMouseUp"
        case 0x0CF5E157: // "Input::RightMouseUp"
        {
          switch (captureMouse)
          {
            case CM_INIT:
              MouseNotify("Click", e);
              break;

            case CM_ON:
              MouseNotify("CaptureEnd", e);
              break;
          }

          // Clear capture mode
          captureMouse = CM_OFF;
          break;
        }

        case 0x6780062A: // "System::LostCapture"
        {
          if (captureMouse == CM_ON)
          {
            switch (button)
            {
              case MOUSE_LEFT:
                selectMode = SM_INACTIVE;
                break;

              case MOUSE_MIDDLE:
              case MOUSE_RIGHT:
                Viewer::GetCurrent()->Notify(Viewer::VN_LOSTCAPTURE);
                break;
            }
          }

          captureMouse = CM_OFF;
          break;
        }

        case 0x5AB0F116: // "System::Activated"
        {
          if (config.Alive())
          {
            IFace::Activate(config);
          }

          break;
        }

        case 0x6589C573: // "System::Deactivated"
        {
          if (config.Alive())
          {
            IFace::Deactivate(config);
          }
          break;
        }

        case 0x107BAAA6: // "System::DuplicateActivate"
        {
          if (config.Alive())
          {
            if (config->IsActive())
            {
              config->Deactivate();
            }
            else
            {
              IFace::Activate(config);
            }
          }
          break;
        }

        case 0xE6863C47: // "System::InitSimulation"
        {
          // Activate all registered interface vars
          for (List<IFaceVar>::Iterator i(&varList); *i; i++)
          {
            config->ActivateVar(*i);
          }
          break;
        }


        //
        // Group Selection Functionality
        //

        case 0xFEA6C563: // "Brush::LeftMouseCaptureStart"
          if (data.cInfo.gameWindow)
          {
            // Save the current mouse position
            SaveMousePosition(e);

            selectMode = data.cInfo.gameWindow->HasProperty("PostDrawSelect") ? SM_POSTDRAW : SM_PREDRAW;
          }
          // Intentional fall-through

        case 0xE6428DAD: // "Brush::LeftMouseCaptureMove"
          selectBox.Set(data.cInfo.mouse, mouseStart);
          selectBox.Sort();
          break;

        case 0x252BA28C: // "Brush::LeftMouseCaptureEnd"
        {
          ASSERT(data.cInfo.gameWindow);

          // Do we need to clear the current selection
          if (!Common::Input::GetModifierKey(3))
          {
            data.sList.Clear();
          }

          // Do the group selection
          data.cInfo.gameWindow->SelectGroup(selectBox, data.sList, GetSelectionFilter());

          selectMode = SM_INACTIVE;
          break;
        }

        case 0x27546BF9: // "Brush::LeftMouseClick"
        {
          // Get the object under the cursor
          MapObj * obj = ObjectUnderCursor();

          if (obj)
          {
            // Do we need to clear the current selection
            if (!Common::Input::GetModifierKey(3))
            {
              data.sList.Clear();
            }

            // Add object to selection
            data.sList.Append(obj);
          }
          break;
        }

        case 0x0ACEA1BD: // "Brush::RightMouseClick"
          data.sList.Clear();
          break;


        //
        // Camera Control Functionality
        //

        case 0xBAA10932: // "Brush::RightMouseCaptureStart"
          SaveMousePosition(e);
          Viewer::GetCurrent()->Notify(Viewer::VN_RBUTTON, TRUE);
          break;

        case 0xC255264B: // "Brush::MiddleMouseCaptureStart"
          SaveMousePosition(e);
          Viewer::GetCurrent()->Notify(Viewer::VN_MBUTTON, TRUE);
          break;

        case 0x3E3119C7: // "Brush::RightMouseCaptureMove"
        case 0xDE8C44CE: // "Brush::MiddleMouseCaptureMove"
          Viewer::GetCurrent()->Notify(Viewer::VN_MOUSEMOVE, Input::MouseDelta().x, Input::MouseDelta().y);
          break;

        case 0x8937F2D5: // "Brush::RightMouseCaptureEnd"
          RestoreMousePosition();
          Viewer::GetCurrent()->Notify(Viewer::VN_RBUTTON, FALSE);
          break;

        case 0x795EF775: // "Brush::MiddleMouseCaptureEnd"
          RestoreMousePosition();
          Viewer::GetCurrent()->Notify(Viewer::VN_MBUTTON, FALSE);
          break;

        case 0x9CAAA9E6: // "Input::MouseAxis"
          Viewer::GetCurrent()->Notify(Viewer::VN_MOUSEAXIS, S32(e->input.ch) / 120);
          break;


        //
        // Brush Display Functionality
        //

        case 0xC3C52EA3: // "System::PreDraw"
        {
          // Should we show the selected objects
          if (HasProperty("DisplaySelected"))
          {
            // Selected objects
            for (MapObjList::Iterator i(&data.sList); *i; i++)
            {
              // List has been validated at this point
              Common::Display::ObjectSelection(**i);
            }

            // Get the object under the cursor
            MapObj * obj = ObjectUnderCursor();

            // Object under mouse
            if (!HasCapture() && obj)
            {
              Common::Display::ObjectSelection(obj);
            }
          }

          // Selection window
          if (selectMode == SM_PREDRAW)
          {
            Common::Display::Rectangle(Common::Display::GroupSelectColor(), selectBox);
          }

          // Always pass system events down
          break;
        }

        case 0x8B9FFA39: // "System::PostDraw"
        {
          if (selectMode == SM_POSTDRAW)
          {
            Common::Display::Rectangle(Common::Display::GroupSelectColor(), selectBox);
          }
          break;
        }
      }
    }


    //
    // Notify
    //
    // Notify this brush after updating base data
    //
    void Base::Notify(U32 crc, ::Event *e)
    {
      // Do pre-notification base updates
      switch(crc)
      {
        case 0xD47619B1: // "Input::LeftMouseDown"
        case 0x3C955069: // "Input::MiddleMouseDown"
        case 0xA463EA4C: // "Input::RightMouseDown"
          mouseStart.Set(e->input.mouseX, e->input.mouseY);
          break;
      }

      // Pass the event to the brush
      Notification(crc, e);
     
      // Do post-notification base updates
      switch(crc)
      {
        case 0xD47619B1: // "Input::LeftMouseDown"
        case 0x3C955069: // "Input::MiddleMouseDown"
        case 0xA463EA4C: // "Input::RightMouseDown"
          hasCapture = TRUE;
          break;

        case 0x6780062A: // "System::LostCapture"
        case 0xC74F389F: // "Input::LeftMouseUp"
        case 0x765CF149: // "Input::MiddleMouseUp"
        case 0x0CF5E157: // "Input::RightMouseUp"
          hasCapture = FALSE;
          break;
      }
    }


    //
    // Notify
    //
    // Notify this brush after updating base data
    //
    void Base::Notify(const char *name, ::Event *e)
    {
      /*
      if (!Utils::Strstr(name, "Poll") && !Utils::Strstr(name, "Draw"))
      {
        LOG_DIAG(("%s->Notify(%s)", Name(), name));
      }
      */

      // Call above method
      Notify(Crc::CalcStr(name), e);
    }


    //
    // ObjectUnderCursor
    //
    // Returns the map object under the cursor, or null
    //
    MapObj * Base::ObjectUnderCursor()
    {
      if (data.cInfo.gameWindow && data.cInfo.mObject.mapObj)
      {
        return (data.cInfo.mObject.mapObj);
      }

      return (NULL);
    }


    //
    // SaveMousePosition
    //
    // Save the current mouse position
    //
    void Base::SaveMousePosition(::Event *e)
    {
      ASSERT(e);

      savedMousePosition.x = e->input.mouseX;
      savedMousePosition.y = e->input.mouseY;
    }


    //
    // RestoreMousePosition
    //
    // Restore the current mouse position
    //
    void Base::RestoreMousePosition()
    {
      Input::SetMousePos(savedMousePosition.x, savedMousePosition.y);

      // Position from start of cycle is now invalid
      data.cInfo.mObject.mapObj = NULL;
      data.cInfo.mTerrain.cell = NULL;
    }

    S32 Base::DeltaX()
    {
      return savedMousePosition.x - Input::MousePos().x;
    }

    S32 Base::DeltaY()
    {
      return savedMousePosition.y - Input::MousePos().y;
    }

    //
    // CreateInteger
    //
    // Create an integer iface var
    //
    IFaceVar * Base::CreateInteger(const char *name, S32 dVal, S32 low, S32 high)
    {
      IFaceVar *var = new IFaceVar(config, config->CreateInteger(name, dVal, low, high));
      varList.Append(var);
      return (var);     
    }


    //
    // CreateFloat
    //
    // Create a float iface var
    //
    IFaceVar * Base::CreateFloat(const char *name, F32 dVal, F32 low, F32 high)
    {
      IFaceVar *var = new IFaceVar(config, config->CreateFloat(name, dVal, low, high));
      varList.Append(var);
      return (var);     
    }


    //
    // CreateString
    //
    // Create a string iface var
    //
    IFaceVar * Base::CreateString(const char *name, const char *dVal)
    {
      IFaceVar *var = new IFaceVar(config, config->CreateString(name, dVal));
      varList.Append(var);
      return (var);
    }



    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class BrushConfig - Used to configure a brush
    //
   
    //
    // Constructor
    //
    BrushConfig::BrushConfig(IControl *parent) : ICWindow(parent), brush(NULL)
    {
    }


    //
    // Notify
    //
    // Notification that a local var has changed value
    //
    void BrushConfig::Notify(IFaceVar *var)
    {
      // Are we configured to use a brush
      if (brush)
      {
        // Notify the brush this var has changed
        brush->NotifyVar(var);
      }      
    }


    //
    // UseBrush
    //
    // Tell this control which brush to notify
    //
    void BrushConfig::UseBrush(Base *b)
    {
      brush = b;
    }


    //
    // HandleEvent
    //
    // Handle interface events
    //
    U32 BrushConfig::HandleEvent(::Event &e)
    {
      // Should we notify the brush
      if (brush && (e.type == IFace::EventID()) && (e.subType == IFace::NOTIFY))
      {
        brush->Notify(e.iface.p1, &e);
      }

      // Always notify the window
      return (ICWindow::HandleEvent(e));  
    }
 
    
    ///////////////////////////////////////////////////////////////////////////////
    //
    // System functions
    //

    // System initialized flag
    static Bool studioInit = FALSE;

    // Sim initialized flag
    static Bool simInit = FALSE;

    // Tree of all existing brushes
    static BinTree<Base> brushes;

    // The default brush
    static Brush::Base * defaultBrush;


    //
    // CreateHandler
    //
    static IControl * CreateHandler(U32 crc, IControl *parent, U32)
    {
      IControl *ctrl = NULL;

      switch (crc)
      {
        case 0x05E735DA: // "Brush::Config"
          ctrl = new BrushConfig(parent);
          break;
      }

      return (ctrl);
    }


    //
    // CreateBrush
    //
    // Create a new brush
    //
    static void CreateBrush(Base *newBrush)
    {
      brushes.Add(Crc::CalcStr(newBrush->Name()), newBrush);

      if (!defaultBrush)
      {
        defaultBrush = newBrush;
      }
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

      // Clear default brush
      defaultBrush = NULL;

      // Register the brush configuration control
      IFace::RegisterControlClass("Brush::Config", CreateHandler);

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

      // Unregister the brush configuration control
      IFace::UnregisterControlClass("Brush::Config");

      // Delete all brushes
      brushes.DisposeAll();

      // Clear init flag
      studioInit = FALSE;
    }


    //
    // CreateBrushes
    //
    // Create all available brushes
    //
    void CreateBrushes()
    {
      // Create all available brushes (first one is the default)
      CreateBrush(new Terrain("Terrain"));
      CreateBrush(new Objects("Objects"));
      CreateBrush(new TerrainTweak("TerrainTweak"));
      CreateBrush(new Water("Water"));
      CreateBrush(new Region("Region"));
      CreateBrush(new Tag("Tag"));
      CreateBrush(new Curve("Curve"));
      CreateBrush(new Trail("Trail"));
      CreateBrush(new PathSearch("PathSearch"));
      CreateBrush(new Overlay("Overlay"));

      // Must have created at least one brush
      ASSERT(defaultBrush)
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

      // Initialize the history system
      History::Base::Init();
     
      // Notify each brush
      for (BinTree<Base>::Iterator i(&brushes); *i; i++)
      {
        (*i)->Notify("System::InitSimulation");
      }

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

      // Notify each brush
      for (BinTree<Base>::Iterator i(&brushes); *i; i++)
      {
        (*i)->Notify("System::DoneSimulation");
      }

      // Shutdown the history system
      History::Base::Done();

      // Clear init flag
      simInit = FALSE;
    }


    //
    // Get
    //
    // Returns a brush, or default if !name, or NULL if no brushes
    //
    Base * Get(const char *name)
    {
      ASSERT(studioInit);

      // Try and find the requested brush
      Base * brush = name ? brushes.Find(Crc::CalcStr(name)) : NULL;

      // Should we return the default brush
      if (!brush)
      {
        // Default brush may be NULL if no brushes created yet
        brush = defaultBrush;
      }

      return (brush);
    }
  }
}
