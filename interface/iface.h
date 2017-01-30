///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//


#ifndef __IFACE_H
#define __IFACE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"
#include "icclass.h"


///////////////////////////////////////////////////////////////////////////////
//
// Forward declarations
//
class FScope;
class MBEvent;
class Bitmap;
struct PixFormat;


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace IFace
//
namespace IFace
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Definitions
  //
  typedef void (INITPROC)(IControl *);


  // Interface Flags
  enum
  {
    // Enable drawing
    DISABLE_DRAW      = 0x0001,

    // Enable window activations
    DISABLE_ACTIVATE  = 0x0002,

    // Disable console
    DISABLE_CONSOLE   = 0x0004,
  };

  // Repaint function
  typedef void (RepaintProc)(void);

  // Cycle setup function
  typedef void (CycleSetupProc)(void);


  //
  // Initialisation
  //

  // Initialise the interface system
  void Init();

  // Shutdown the interface system
  void Done();

  // Emergency shutdown
  void CriticalShutdown();

  //
  // Core Processing
  //

  // Process input events and perform control processing
  // An optional function can be passed in to be called before events are dished out
  void Process(Bool processCursor = TRUE, CycleSetupProc *proc = NULL);

  // Process the cursor
  void ProcessCursor();

  // Render the interface
  void Render();


  // Begin rendering
  void InitRender();

  // Finish Rendering
  void DoneRender();

  // Call the Render function only
  void CallRenderProc();

  // Draw ALL the interface, including 2d/3d and page flipping
  void PaintAll();

  // Remove all controls, resources, except the core stuff
  void DisposeAll();

  // Refresh all top level controls by deactiving/reactivating
  void Refresh();

  // Delete or deactivate all top level non system controls
  void PurgeNonSystem(Bool del = TRUE);

  // Function to be called when the video mode changes
  void OnModeChange();

  // Function to be called when the game is deactivated
  void OnActivate(Bool active);

  // Set a system flags
  Bool SetFlag(U32 flag, Bool value);

  // Get a system flag
  Bool GetFlag(U32 flag);

  // Set redraw functoin
  RepaintProc *SetRepaintProc(RepaintProc *proc);

  // Set fade time
  void SetFade(F32 initial, F32 time, Bool up = TRUE);


  //
  // Control management
  //

  // Register a code class Interface control and a function that can instantiate the control
  Bool RegisterControlClass(const char *name, ICClass::CREATEPROC *proc);

  // Unregister a code class Interface control previously registered
  Bool UnregisterControlClass(const char *name);

  // Find a control class
  ICClass *FindControlClass(U32 id);

  // Create a control class definition from a FScope "DefineControlType"
  void DefineControlType(FScope *fScope);

  // Create a control from a name and class, and set its parent
  IControl *CreateControl(const char *ctrlName, const char *ctrlClass, IControl *parent, INITPROC *proc = NULL);

  // Create a controm from a name and class id, add set its parent
  IControl *CreateControl(const char *ctrlName, U32 classId, IControl *parent, INITPROC *proc = NULL);

  // Create a control from a FScope "CreateControl" 
  IControl *CreateControl(FScope *fScope, IControl *parent = NULL);

  // Create a control from registry data FScope
  IControl *CreateControl(const char *name, FScope *fScope, IControl *parent);

  // Find data in scope registry
  FScope *FindRegData(const char *name, Bool subFunc = TRUE, Bool warn = TRUE);

  //
  // Events
  //

  // Send an IFace event to the control
  U32 SendEvent(IControl *ctrl, IControl *from, U32 type, U32 p1 = 0, U32 p2 = 0, U32 p3 = 0, U32 p4 = 0);

  // Post an IFace event to a control via the event queue
  U32 PostEvent(IControl *ctrl, IControl *from, U32 type, U32 p1 = 0, U32 p2 = 0, U32 p3 = 0, U32 p4 = 0);


  // Send a notify event to the control
  U32 SendNotify(IControl *ctrl, IControl *from, U32 event, U32 p1 = 0, U32 p2 = 0, U32 p3 = 0);

  // Send a notify event to the current modal control
  U32 SendNotifyModal(IControl *from, U32 event, U32 p1 = 0, U32 p2 = 0, U32 p3 = 0);

  // Send a notify event to the current control which has focus
  U32 SendNotifyFocus(IControl *from, U32 event, U32 p1 = 0, U32 p2 = 0, U32 p3 = 0);


  // Post a notify event to the control
  U32 PostNotify(IControl *from, U32 event, U32 p1 = 0, U32 p2 = 0, U32 p3 = 0);

  // Post a notify event to the current modal control
  U32 PostNotifyModal(IControl *from, U32 event, U32 p1 = 0, U32 p2 = 0, U32 p3 = 0);

  // Post a notify event to the current control which has focus
  U32 PostNotifyFocus(IControl *from, U32 event, U32 p1 = 0, U32 p2 = 0, U32 p3 = 0);


  // Return the Event id obtained from EventSys
  U16 EventID();

  // Time since last input event
  U32 LastInputEventTime();

  //
  //
  // Console
  //

  // Show or hide the console
  void ShowConsole(Bool vis = TRUE);

  // Display a string on the console
  void CDECL ConsoleWrite(const char *str, U32 color);

  // Clear the console
  void ClearConsole();

  // Returns a value for the console state
  S32 GetConsoleState();

  //
  // Mouse capture functions
  //

  // Set "ctrl" to have mouse capture
  void SetMouseCapture(IControl *ctrl);

  // Release mouse capture from "ctrl"
  void ReleaseMouseCapture(IControl *ctrl);

  // Return the control that has mouse capture
  IControl *GetCapture();

  // Return the control that is beneath the mouse
  IControl *GetMouseOver();

  //
  // Keyboard focus functions
  //

  // Set "ctrl" to have keyboard focus
  void SetFocus(IControl *ctrl);

  // Release keyboard focus from "ctrl"
  void ReleaseFocus(IControl *ctrl);

  // Return the control that has keyboard focus
  IControl *GetFocus();

  //
  // Modal window functions
  //

  // Set "ctrl" to be modal
  void SetModal(IControl *ctrl);

  // Unset modal state of "ctrl"
  void UnsetModal(IControl *ctrl);

  // Return the current modal control
  IControl *GetModal();

  // Deactivate all modal controls
  void DeactivateModals();

  //
  // Other window functions
  //

  // Return a pointer to the root window
  IControl *RootWindow();

  // Return a pointer to the overlays window
  IControl *OverlaysWindow();

  // Activate a window
  Bool Activate(const char *name);

  // Activate a window
  Bool Activate(IControl *ctrl);

  // Deactivate a window
  Bool Deactivate(const char *name);

  // Deactivate a window
  Bool Deactivate(IControl *ctrl);

  // Toggle active state
  Bool ToggleActive(const char *name);

  // Find a control by name
  IControl *FindByName(const char *name, IControl *base = NULL);

  // Find a control by name crc
  IControl *FindByName(U32 crc, IControl *base = NULL);

  // Setup current tip control
  void SetTipControl(IControl *c);


  //
  // Game window functions
  //

  // Set handler proc for the game window
  EventSys::HANDLERPROC SetGameHandler(EventSys::HANDLERPROC proc);

  // Return a pointer to the game window control, always succeeds
  IControl *GameWindow();

  //
  // Resource management
  //

  // Register an unmanaged bitmap to delete on DisposeAll
  void RegisterUnmanagedBitmap(Bitmap *bitmap);

  // Stock metrics
  S32 GetMetric(U32 type);

  // Stock interface colors
  const Color &GetColor(U32 type);

  // Get console color from type
  const Color &GetConsoleColor(U32 type);

  // Set console color from type
  void SetConsoleColor(U32 type, const Color &color);

  // Create a color group
  ColorGroup *CreateColorGroup(const char *name);

  // Find a color group
  ColorGroup *FindColorGroup(U32 id);

  // Create a texture skin
  TextureSkin *CreateTextureSkin(const char *name);

  // Find a texture skin
  TextureSkin *FindTextureSkin(U32 id);

  // Elapsed time since game started
  U32 ElapsedMs();

  // Elapsed time since last frame processing
  U32 TimeStepMs();
  F32 TimeStepSec();

  // Current screen width
  S32 ScreenWidth();

  // Current screen height
  S32 ScreenHeight();

  // Current screen depth
  S32 ScreenDepth();

  // Current pixel format
  const Pix * ScreenPixelFormat();

  // Current state of the caret
  Bool CaretState();

  // Control promoter
  template <class CONTROL> CONTROL * Promote(IControl *control, Bool required = FALSE)
  {
    ASSERT(control);

    // Can we safely cast this control
    if (control->DerivedFrom(CONTROL::ClassId()))
    {
      return ((CONTROL *) control);
    }

    if (required)
    {
      ERR_FATAL(("Could not promote '%s' to '%s'", control->Name(), CONTROL::ClassName()))
    }

    // Not able to cast
    return (NULL);
  }

  // Find a specific type
  template <class CONTROL> CONTROL * Find(const char *name, IControl *base = NULL, Bool required = FALSE)
  {
    // Find the control
    IControl *ctrl = FindByName(name, base);

    if (required && !ctrl)
    {
      ERR_FATAL(("Could not find control '%s' in control '%s'", name, base ? base->Name(): "Root"))
    }

    CONTROL *control = ctrl ? Promote<CONTROL>(ctrl, required) : NULL;

    // If found, attempt to promote
    return (control);
  }

  // Find a specific type
  template <class CONTROL> CONTROL * Find(U32 crc, IControl *base)
  {
    // Find the control
    IControl *ctrl = base->Find(crc, TRUE);

    // If found, attempt to promote
    return (ctrl ? Promote<CONTROL>(ctrl) : NULL);
  }

}

#endif
