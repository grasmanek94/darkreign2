///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 11-FEB-1999
//

#ifndef __STUDIO_BRUSH_H
#define __STUDIO_BRUSH_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio_history.h"
#include "utiltypes.h"
#include "event.h"
#include "ifvar.h"
#include "icwindow.h"
#include "mapobjdec.h"
#include "common_gamewindow.h"


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
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Forward declarations
    //
    class BrushConfig;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Base - The base brush class
    //
    class Base
    {
    protected:

      // Capture modes
      enum CaptureMode { CM_OFF, CM_INIT, CM_ON };

      // Mouse buttons
      enum MouseButton { MOUSE_LEFT, MOUSE_MIDDLE, MOUSE_RIGHT, MOUSE_MAX };

      // Mouse capture thresholds for each button
      Point<S32> captureThreshold[MOUSE_MAX];

      // Used to capture mouse buttons
      CaptureMode captureMouse;

      // Which button has capture
      MouseButton button;

      // Starting position of a mouse click
      Point<S32> mouseStart;

      // The interface configuration control
      Reaper<BrushConfig> config;

      // Called when a brush event is generated
      virtual void Notification(U32 crc, ::Event *e = NULL);

      // Returns the map object under the cursor, or null
      MapObj * ObjectUnderCursor();

      // Save/Restore the current mouse position
      void SaveMousePosition(::Event *e);
      void RestoreMousePosition();

      // Create vars thru the interface config
      IFaceVar * CreateInteger(const char *name, S32 dVal, S32 low = S32_MIN, S32 high = S32_MAX);
      IFaceVar * CreateFloat(const char *name, F32 dVal, F32 low = F32_MIN, F32 high = F32_MAX);
      IFaceVar * CreateString(const char *name, const char *dVal);

    private:

      // The text identifier for this brush
      GameIdent name;

      // Does this brush currently have capture
      Bool hasCapture;

      // Current group selection mode
      enum { SM_INACTIVE, SM_PREDRAW, SM_POSTDRAW } selectMode;

      // Current group select box
      Area<S32> selectBox;

      // Saved mouse position
      Point<S32> savedMousePosition;

      // List of all registered iface vars
      List<IFaceVar> varList;

      // Used to generate events based on the current mouse button
      void MouseNotify(const char *event, ::Event *e);

      // Returns TRUE if the mouse has moved past current threshold
      Bool CaptureThreshold(const Point<S32> &b);
  
    public:

      // Constructor and destructor
      Base(const char *name);
      virtual ~Base();

      // Does brush have given property (required)
      virtual Bool HasProperty(U32 property);
      Bool HasProperty(const char *property);

      // Interface var notification (required)
      virtual void NotifyVar(IFaceVar *var);

      // Returns the current cursor for this brush (not required)
      virtual U32 Cursor();

      // Returns the object selection filter for this brush
      virtual Common::GameWindow::SelectFilter * GetSelectionFilter();

      // Notify this brush after updating base data
      void Notify(U32 crc, ::Event *e = NULL);
      void Notify(const char *name, ::Event *e = NULL);

      // Returns the name of this brush
      const char * Name()
      {
        return (name.str);
      }

      // Does this brush currently have capture
      Bool HasCapture()
      {
        return (hasCapture);
      }

      S32 DeltaX();
      S32 DeltaY();
    };

    
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class BrushConfig - Used to configure a brush
    //
    class BrushConfig : public ICWindow
    {
      PROMOTE_LINK(BrushConfig, ICWindow, 0x05E735DA); // "Brush::Config"

    protected:

      // The current brush being configured
      Base *brush;

      // Notification that a local var has changed value
      void Notify(IFaceVar *var);

    public:

      // Constructor 
      BrushConfig(IControl *parent);

      // Tell this control which brush to notify
      void UseBrush(Base *b);

      // Event handling
      U32 HandleEvent(::Event &e);
    };


    ///////////////////////////////////////////////////////////////////////////////
    //
    // System functions
    //

    // Studio initialization
    void Init();
    void Done();

    // Create all available brushes
    void CreateBrushes();

    // Simulation initialization
    void InitSim();
    void DoneSim();

    // Returns a brush, or default if !name, or NULL if no brushes
    Base * Get(const char *name = NULL);
  }
}

#endif