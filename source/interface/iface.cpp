///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "iface.h"
#include "iface_priv.h"
#include "iface_util.h"
#include "iface_types.h"
#include "babel.h"
#include "iface_sound.h"
#include "iface_controls.h"
#include "iface_messagebox.h"
#include "multilanguage.h"
#include "vid_public.h"
#include "main.h"
#include "fontsys.h"
#include "input.h"
#include "stack.h"
#include "console.h"
#include "console_viewer.h"
#include "keybind.h"
#include "cursor.h"
#include "perfstats.h"
#include "stdload.h"
#include "meshent.h"
#include "statistics.h"
#include "icontrol.h"
#include "icroot.h"
#include <locale.h>


//#define LOGGING

#ifdef LOGGING
  #define LOG_IFACE(x) LOG_DIAG(x)
  #define PROCESS(c,e,s) DebugProcessEvent(c,e,s)
#else
  #define LOG_IFACE(x)
  #define PROCESS(c,e,s) c->HandleEvent(e)
#endif



///////////////////////////////////////////////////////////////////////////////
//
// NameSpace IFace
//
namespace IFace
{
  // Scope name
  static const char *SCOPE_NAME = "ConfigureInterface";
  static const char *CTRL_SCOPE = "CreateControl";


  // Public data
  Bool sysInit = FALSE;

  // System data
  SystemData data;

  // Identifier for IFACE events
  static U16 eventId;

  // Interface flags
  static U32 systemFlags;

  // Repaint function
  static RepaintProc *paintProc = NULL;

  // The list of registered control classes
  static BinTree<ICClass> ctrlTypes;

  // Root window, all windows in the system are descendents of this
  static ICRoot *root = NULL;

  // Overlay windows, such as tool tips
  static ICRoot *overlays = NULL;

  // Stack for modal windows
  static Stack<IControlPtr> modalStack;

  // Active controls
  static IControlPtr modal;
  static IControlPtr focus;
  static IControlPtr mouseCapture;
  static IControlPtr mouseOver;

  // Tool tip control
  static IControlPtr tipCtrl;

  // Control that will display next tool tip
  static IControlPtr nextTipOwner;

  // Time until next tool tip should appear
  static U32 nextTipDelay;

  static U32 startTime;
  static U32 elapsedMs;
  static U32 timeStepMs;
  static F32 timeStepSec;
  static U32 lastInputEvent;

  // Visibility state of the caret
  static Bool caretState;
  static S32 caretMs;
  static S32 caretSpeed;

  // Metrics table
  static S32 metricTable[MAX_METRICS];

  // Color table
  static Color colorTable[MAX_COLORS];

  // Max color groups
  const U32 MAX_COLOR_GROUPS = 32;

  // Color group table
  static BinTree<ColorGroup> stdColorGroups;
  static BinTree<ColorGroup> customColorGroups;

  // Texture skins
  static BinTree<TextureSkin> textureSkins;

  // FScope Registry 
  static BinTree<FScope> scopeRegistry;

  // Unmanaged images
  static List<Bitmap> unmanagedBitmaps;

  // Console colors
  static Color defaultConsoleColor;
  static BinTree<Color> consoleColors;

  // Wireframe mode
  static VarInteger wireFrame;

  // CRC Tables
  U32 colorTableCrc[MAX_COLORS];
  U32 metricTableCrc[MAX_CUSTOM_METRICS];

  // Fading parameters
  Bool fading;
  F32 fadeRate;

  // Default repaint function
  static void DefaultRenderProc();

  // Function to handle input events
  static Bool InputEventHandler(Event &e);

  // Function to handle interface events
  static Bool IFaceEventHandler(Event &e);

  // Callback function to instantiate a control of a specified type
  static IControl *CtrlCreateHandler(U32 classId, IControl *parent);

  // Update system metrics related to screen dimensions
  static void UpdateScreenMetrics();

  // Initailise colors
  static void InitColors();
  static void DoneColors();

  // Init color groups
  static void InitDefaultColorGroups();

  // Initailse metrics
  static void InitMetrics();

  // Register Commands and vars
  static void CmdInit();

  // Unregister Commands and vars
  static void CmdDone();

  // Handle console commands
  static void CmdHandler(U32 pathCrc);

  // Poll for tool tip
  static void PollToolTip();

  // Function to be called when the mouse under the cursor changes
  static void OnMouseOverChanged(IControl *over);

  // Function to be called on a mouse click
  static void OnMouseClick();

  // Reset fade
  static void ResetFade();

  // Read a configuration section
  static void Configure(FScope *fScope);
  static void CreateRootLevelControl(FScope *fScope);


  /////////////////////////////////////////////////////////////////////////////
  //
  // Variable Substitution 
  //
  const char VARCHAR = '$';

  struct Substitution : public VarSys::Substitution
  {
    // Constructor
    Substitution() : VarSys::Substitution(VARCHAR) { }

    // Expansion
    const char *Expand(const char *name, void *context)
    {
      IControl *ctrl = reinterpret_cast<IControl *>(context);

      switch (*name)
      {
        case VARCHAR:
          // Use the root control
          ctrl = IFace::RootWindow();
          break;

        case '\0':
          // Use the context team
          if (!ctrl)
          {
            LOG_DIAG(("Expanding '%s': Expected context for control substitution", name))
            return (name);
          }
          break;

        default:
          if ((ctrl = IFace::FindByName(name, ctrl)) == NULL)
          {
            LOG_DIAG(("Expanding '%s': Could not find control", name));
            return (name);
          }
          break;
      }

      return (ctrl->DynVarName());
    }
  };

  static Substitution substitution;


  //
  // Debug build event tracing
  //
  #ifdef LOGGING

  static U32 DebugProcessEvent(IControl *ctrl, Event &e, const char *str)
  {
    LOG_IFACE
    ((
      "%s Event: %.4X %.4X %.8X %.8X %.8X %.8X to [%s]", 
      str, e.type, e.subType, e.param1, e.param2, e.param3, e.param4, ctrl->Name()
    ))

    return (ctrl->HandleEvent(e));
  }

  #endif


  //
  // Clear all static reapers
  //
  static void ClearReapers()
  {
    modal.Clear();
    focus.Clear();
    mouseOver.Clear();
    mouseCapture.Clear();
    nextTipOwner.Clear();
  }


  //
  // Initialise the interface system
  // Creates the console and root windows
  //
  void Init()
  {
    ASSERT(!sysInit);

    data.backBuf = &Vid::backBmp;

    // Register event type
    EventSys::RegisterEvent("IFACE", eventId);

    // Set up the event handlers
    EventSys::SetHandler(IFace::EventID(), IFaceEventHandler);
    EventSys::SetHandler(Input::EventID(), InputEventHandler);

    // Register scope handler
    Main::RegisterRootScope(SCOPE_NAME, Configure);
    Main::RegisterRootScope(CTRL_SCOPE, CreateRootLevelControl);

    startTime = Clock::Time::Ms();

    // Set up color scheme
    InitColors();

    // Setup metrics
    InitMetrics();

    // Register Standard Interface control creation handlers
    Controls::Init();

    // Initalise sounds
    Sound::Init();

    // Register command handlers and scopes
    CmdInit();

    // Initialise cursor system
    CursorSys::Init();

    // Create the root window
    root = new ICRoot;
    root->SetName("Root");
    root->Activate();

    // Create overlays window
    overlays = new ICRoot;
    overlays->SetName("Overlays");
    overlays->Activate();

    // Initialise caret
    caretState = !caretState;
    caretSpeed = 400;
    caretMs = caretSpeed;

    // Initial alpha scale
    ResetFade();

    // Repaint function
    paintProc = DefaultRenderProc;

    // Initialise key bindings
    KeyBind::Init();

    // Register the var substitution for teams
    VarSys::RegisterSubstitution(substitution);

    // System is initialised
    sysInit = TRUE;
  }


  //
  // Shut it all down
  //
  void Done()
  {
    if (sysInit)
    {
      // Clean up
      DisposeAll();

      // Register the var substitution for teams
      VarSys::UnregisterSubstitution(substitution);

      // Delete key bindings
      KeyBind::Done();

      // Delete root window
      ASSERT(root);

      root->Deactivate();
      delete root;

      // Delete overlays window
      ASSERT(overlays);

      overlays->Deactivate();
      delete overlays;

      // Delete colors
      DoneColors();

      // Shutdown the cursor system
      CursorSys::Done();

      // Delete sounds
      Sound::Done();

      // Unregister controls
      Controls::Done();

      // Remove var scope
      CmdDone();

      // Unregister scope handler
      Main::UnregisterRootScope(CTRL_SCOPE, CreateRootLevelControl);
      Main::UnregisterRootScope(SCOPE_NAME, Configure);

      // Done
      sysInit = FALSE;
    }
  }


  //
  // Critical shutdown
  //
  void CriticalShutdown()
  {
    sysInit = FALSE;
  }


  //
  // Called every frame of the game to handle input and do other good stuff
  //
  void Process(Bool processCursor, CycleSetupProc *setupProc)
  {
    ASSERT(sysInit)

    PERF_S("IFace::Process");

    // Store time elapsed since game started/last frame
    elapsedMs  = Main::thisTime - startTime;
    timeStepMs = Main::elapTime;
    timeStepSec = Main::elapSecs;

    // Read input events during the last frame
    Input::ReadEvents();

    // Delete all controls that are marked
    IControl::DeleteMarkedControls();

    // Don't do any processing of controls if the interface is not visible
    IControl *over = NULL;

    if (systemFlags & DISABLE_DRAW)
    {
      mouseOver.Clear();
    }
    else
    {
      // Find the window that the mouse cursor is over
      over = root->Find(Input::MousePos().x, Input::MousePos().y);

      // Mouse over event
      if (!mouseCapture.Alive())
      {
        if (mouseOver.GetData() != over)
        {
          // mouse over has changed
          if (mouseOver.Alive())
          {
            SendEvent(mouseOver, NULL, MOUSEOUT, 0, 0);
          }

          if (!modal.Alive() || (modal.Alive() && modal->IsChild(over)))
          {
            if (over)
            {
              SendEvent(over, NULL, MOUSEIN, 0, 0);
            }
          }

          // System Hook 
          OnMouseOverChanged(over);
        }
        mouseOver = over;
      }

      // Simulate the caret
      ProcessCaret(timeStepMs);
    }

    // Call the optional event setup function before any events are processed
    if (setupProc)
    {
      setupProc();
    }

    // Process all events
    EventSys::ProcessAll();

    // Process "hold" type key bindings if no other control has keyboard focus
    if (!focus.Alive())
    {
      KeyBind::Poll();
    }

    // Process polled controls
    IControl::ProcessPollList(timeStepMs);

    // Poll for a tooltip
    PollToolTip();

    // Process the cursor
    if (!(systemFlags & DISABLE_DRAW))
    {
      if (processCursor)
      {
        ProcessCursor();
      }

      // Debugging info
      MSWRITEV(13, (19, 0, "OVER    %-32s", mouseOver.Alive()    ? mouseOver->Name()   : ""));
      MSWRITEV(13, (20, 0, "MODAL   %-32s", modal.Alive()        ? modal->Name()       : ""));
      MSWRITEV(13, (21, 0, "FOCUS   %-32s", focus.Alive()        ? focus->Name()       : ""));
      MSWRITEV(13, (22, 0, "CAPTURE %-32s", mouseCapture.Alive() ? mouseCapture->Name(): ""));
    }

    // Delete all controls that are marked
    IControl::DeleteMarkedControls();

    // Animate fading
    if (fading)
    {
      data.alphaScale += fadeRate * timeStepSec;

      if ((fadeRate < 0) && (data.alphaScale < 0.0F))
      {
        data.alphaScale = 0.0F;
        fading = FALSE;
      }
      else

      if ((fadeRate > 0) && (data.alphaScale > 1.0F))
      {
        data.alphaScale = 1.0F;
        fading = FALSE;
      }
    }

    PERF_E("IFace::Process");
  }


  //
  // Process cursor when a modal window is active
  //
  static Bool ProcessModalCursor(IControl *modalCtrl)
  {
    ASSERT(modalCtrl)
    ASSERT(mouseOver.Alive())

    // Mouse is inside modal window
    if (modalCtrl->IsChild(mouseOver))
    {
      // Mouse is over a child of modal window, or the modal, so use mouseover's cursor
      return (mouseOver->GetCursor());
    }
    else
    {
      // Mouse is outside modal window
      if (modalCtrl->IsModalClose())
      {
        if (modalStack.GetCount() > 0)
        {
          // Window is modal close style, repeat procedure with next window on the stack
          U32 cursor;
          IControlPtr *top;

          // Get top item
          top = modalStack.Pop();

          // Process it, until no more items on stack
          cursor = ProcessModalCursor(*top);

          // Push it back on
          modalStack.Push(top);

          return (cursor);
        }
        else
        {
          // Use cursor of control under mouse
          return (mouseOver->GetCursor());
        }
      }
    }

    // Mouse is out of the modal's tree, use NO cursor
    return (CursorSys::GetStandardCursor(CursorSys::NO));
  }


  //
  // Process the cursor
  //
  void ProcessCursor()
  {
    // Set the cursor
    U32 cursor = CursorSys::DefaultCursor();

    if (mouseCapture.Alive())
    {
      // Use capture control's cursor
      cursor = mouseCapture->GetCursor();
    }
    else 
      
    if (mouseOver.Alive())
    {
      if (modal.Alive())
      {
        cursor = ProcessModalCursor(modal);
      }
      else
      {
        // Mouse is over a control so use its cursor
        cursor = mouseOver->GetCursor();
      }
    }

    // Set the cursor
    CursorSys::Set(cursor);
  }


  //
  // Update system metrics related to screen dimensions
  //
  static void UpdateScreenMetrics()
  {
    metricTable[SCREEN_WIDTH]   = Vid::backBmp.Width();
    metricTable[SCREEN_HEIGHT]  = Vid::backBmp.Height();
    metricTable[SCREEN_DEPTH]   = Vid::backBmp.Depth();
  }


  //
  // Function to be called when the video mode changes
  //
  void OnModeChange()
  {
    if (sysInit)
    {
      FontSys::OnModeChange();

      U32 xres = Vid::backBmp.Width();
      U32 yres = Vid::backBmp.Height();

      // Reload unmanaged bitmaps
      for (List<Bitmap>::Iterator i(&unmanagedBitmaps); *i; i++)
      {
        (*i)->ReleaseDD();
        (*i)->Read((*i)->GetName());
      }

      // Update system metrics
      UpdateScreenMetrics();

      // Notify the root window of mode change
      SendEvent(root, NULL, DISPLAYMODECHANGED, xres, yres);
      SendEvent(overlays, NULL, DISPLAYMODECHANGED, xres, yres);
    }
  }


  //
  // Function to be called when the game is deactivated
  //
  void OnActivate(Bool)
  {
    if (sysInit)
    {
      SetMouseCapture(NULL);
    }
  }


  //
  // Poll for a tool tip
  //
  static void PollToolTip()
  {
    if (nextTipOwner.Alive())
    {
      if (IFace::LastInputEventTime() > nextTipDelay)
      {
        SendEvent(nextTipOwner, NULL, DISPLAYTIP);
        nextTipOwner.Clear();
      }
    }
  }


  //
  // Function to be called when the mouse under the cursor changes
  //
  static void OnMouseOverChanged(IControl *over)
  {
    Bool tipMode = tipCtrl.Alive();

    // If a tool tip is active delete it
    if (tipCtrl.Alive())
    {
      tipCtrl->MarkForDeletion();
      tipCtrl.Clear();
    }

    nextTipOwner.Clear();

    // If in tipmode create a new tip
    if (over && tipMode)
    {
      // Automatically pop up the new tool tip if already in tip mode
      SendEvent(over, NULL, DISPLAYTIP);
    }
    else
    {
      // Ask the control for it's tool tip time
      if (over && ((nextTipDelay = SendEvent(over, NULL, TIPDELAY)) > 0))
      {
        nextTipOwner = over;
      }
    }
  }


  //
  // Function to be called on a mouse click
  //
  static void OnMouseClick()
  {
    if (tipCtrl.Alive())
    {
      tipCtrl->MarkForDeletion();
      tipCtrl.Clear();
    }

    // Don't poll for tool tips until mouse over changes
    nextTipOwner.Clear();
  }


  //
  // Set a system flags
  //
  Bool SetFlag(U32 flag, Bool value)
  {
    Bool old = (systemFlags & flag) ? TRUE : FALSE;

    if (value)
    {
      systemFlags |= flag;
    }
    else
    {
      systemFlags &= ~flag;
    }
    return (old);
  }


  //
  // Get a system flag
  //
  Bool GetFlag(U32 flag)
  {
    return (systemFlags & flag);
  }


  //
  // Set redraw function
  //
  RepaintProc *SetRepaintProc(RepaintProc *proc)
  {
    RepaintProc *oldProc = paintProc;

    paintProc = proc;
    return (oldProc);
  }

  //
  // Set fade time
  //
  void SetFade(F32 initial, F32 time, Bool up)
  {
    ASSERT(time > 1e-4F)

    // Initial alpha scale
    data.alphaScale = initial;

    // Calculate percent per second
    fadeRate = (1.0F / time);
    if (!up)
    {
      fadeRate *= -1.0F;
    }
    fading = TRUE;
  }


  //
  // Reset fade
  //
  static void ResetFade()
  {
    data.alphaScale = 1.0F;
    fading = FALSE;
  }


  //
  // Registers a code class for a control and a function that can instantiate the control
  //
  Bool RegisterControlClass(const char *name, ICClass::CREATEPROC *proc)
  {
    // Ensure that the control class is not already registered
    if (ctrlTypes.Find(Crc::CalcStr(name)))
    {
      ERR_FATAL(("Control class [%s] already defined", name));
    }

    // Create a new entry
    ICClass *c = new ICClass;
  
    c->type = name;
    c->createProc = proc;
  
    return ctrlTypes.Add(c->type.crc, c);
  }


  //
  // Unregister a code class Interface control previously registered
  //
  Bool UnregisterControlClass(const char *name)
  {
    ASSERT(sysInit);

    U32 key = Crc::CalcStr(name);

    if (!ctrlTypes.Find(key))
    {
      ERR_FATAL(("Unable to unregister control class [%s], not found", name));
    }

    // Get rid of it
    ctrlTypes.Dispose(key);

    return (TRUE);
  }


  //
  // Find a control class
  //
  ICClass *FindControlClass(U32 id)
  {
    return (ctrlTypes.Find(id));
  }


  //
  // Create a custom control class from a DefineControlType scope
  //
  void DefineControlType(FScope *fScope)
  {
    ASSERT(sysInit);

    const char *newClass  = fScope->NextArgString();
    const char *baseClass = fScope->NextArgString();
    ICClass *base, *c;

    // The base class must already be defined
    if ((base = ctrlTypes.Find(Crc::CalcStr(baseClass))) == NULL)
    {
      ERR_FATAL(("Base control class [%s] not defined", baseClass));
    }

    // It was so create a new Control class
    c = new ICClass;
  
    c->type       = newClass;
    c->base       = base->type;
    c->derived    = TRUE;
    c->createProc = base->createProc;
    c->scope      = fScope->Dup(NULL);
  
    if (ctrlTypes.Add(c->type.crc, c))
    {
      ERR_FATAL(("Control class [%s] already defined", newClass));
    }
  }


  //
  // Creates a control of type "ctrlClass" and assigns it the name "ctrlName" 
  // and sets its parent object to "parent".
  //
  IControl *CreateControl(const char *ctrlName, const char *ctrlClass, IControl *parent, INITPROC *proc)
  {
    U32 classId = Crc::CalcStr(ctrlClass);

    IControl *ctrl = CreateControl(ctrlName, classId, parent, proc);

    if (!ctrl)
    {
      ERR_FATAL(("Control class [%s] not found", ctrlClass))
    }

    return (ctrl);
  }


  //
  // Create a controm from a name and class id, add set its parent
  //
  IControl *CreateControl(const char *ctrlName, U32 classId, IControl *parent, INITPROC *proc)
  {
    ASSERT(sysInit);
    ASSERT(ctrlName);

    ICClass *c = ctrlTypes.Find(classId);

    if (c == NULL)
    {
      return (NULL);
    }

    ASSERT(c->createProc);

    // If this control class is derived from another class (using DefineControlType) then
    // call the base class function to instantiate the control
    IControl *ctrl = NULL;

    if (parent == NULL)
    {
      parent = root;
    }

    if (c->derived)
    {
      ctrl = c->createProc(c->base.crc, parent, 0);
    }
    else
    {
      ctrl = c->createProc(c->type.crc, parent, 0);
    }

    // Couldn't create it
    if (ctrl == NULL)
    {
      ERR_FATAL(("Error creating control [%s]", ctrlName));
    }

    // Setup parameters
    ctrl->SetName(ctrlName);

    // If an init proc was passed in, call it
    if (proc)
    {
      proc(ctrl);
    }

    if (c->scope)
    {
      // Configure with the scope
      c->scope->InitIterators();
      ctrl->Configure(c->scope);
    }

    return (ctrl);
  }


  //
  // Create and configure a control from a FScope object
  //
  IControl *CreateControl(FScope *fScope, IControl *parent)
  {
    const char *ctrlName  = fScope->NextArgString();
    const char *ctrlClass = fScope->NextArgString();

    IControl *ctrl = CreateControl(ctrlName, ctrlClass, parent);

    if (ctrl != NULL)
    {
      ctrl->Configure(fScope);
    }

    return ctrl;
  }


  //
  // Create a control from registry data FScope
  //
  IControl *CreateControl(const char *name, FScope *fScope, IControl *parent)
  {
    IControl *ctrl;

    // Reset iterators as this scope will have been traversed before
    fScope->InitIterators();

    // Extract class of control from first argument of scope
    ctrl = CreateControl(name, StdLoad::TypeString(fScope), parent);

    // Apply body of configuration
    if (ctrl)
    {
      ctrl->Configure(fScope);
    }

    return (ctrl);
  }


  //
  // Create a root level scope, for Main cmds
  //
  static void CreateRootLevelControl(FScope *fScope)
  {
    CreateControl(fScope, NULL);
  }


  //
  // Create data registry entry
  //
  static void CreateRegData(FScope *fScope)
  {
    const char *name = StdLoad::TypeString(fScope);
    U32 key = Crc::CalcStr(name);

    if (scopeRegistry.Find(key))
    {
      LOG_ERR(("Registry data [%s] already defined", name))
    }
    else
    {
      scopeRegistry.Add(key, fScope->Dup());
    }
  }


  //
  // Find data in scope registry
  //
  FScope *FindRegData(const char *name, Bool subFunc, Bool warn)
  {
    FScope *p = scopeRegistry.Find(Crc::CalcStr(name));

    if (p == NULL)
    {
      if (warn)
      {
        LOG_ERR(("Registry data [%s] not found", name))
      }
      return (NULL);
    }
    else
    {
      p->InitIterators();
      return (subFunc ? p->NextFunction() : p);
    }
  }


  //
  // Configure metrics
  //
  static void ConfigureMetrics(FScope *fScope)
  {
    ASSERT(fScope)

    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      U32 key = sScope->NameCrc();

      // Search CRC table
      for (U32 i = 0; i < MAX_CUSTOM_METRICS; i++)
      {
        if (metricTableCrc[i] == key)
        {
          // If the arg is an integer, read it as an int, else
          // if its a string then use the CRC of its value.
          VNode *vNode = sScope->NextArgument(TRUE);
          
          if (vNode->aType == VNode::AT_STRING)
          {
            metricTable[i] = Crc::CalcStr(vNode->GetString());
          }
          else

          if (vNode->aType == VNode::AT_INTEGER)
          {
            metricTable[i] = vNode->GetInteger();
          }
          else
          {
            LOG_ERR(("Metric [%s] not String or Integer", sScope->NameStr()))
          }
          break;
        }
      }

      // Not found
      if (i == MAX_CUSTOM_METRICS)
      {
        LOG_ERR(("Unknown Metric Name [%s]", sScope->NameStr()))
      }
    }
  }


  //
  // Configure colors
  //
  static void ConfigureColors(FScope *fScope)
  {
    ASSERT(fScope)

    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      U32 key = sScope->NameCrc();

      // Search CRC table
      for (U32 i = 0; i < MAX_COLORS; i++)
      {
        if (colorTableCrc[i] == key)
        {
          FScopeToColor(sScope, colorTable[i]);
          break;
        }
      }

      // Not found
      if (i == MAX_COLORS)
      {
        LOG_ERR(("Unknown Color Name [%s]", sScope->NameStr()))
      }
    }

    // Update default color groups
    InitDefaultColorGroups();
  }


  //
  // Configure default console color
  //
  static void ConfigureDefaultConsoleColor(FScope *fScope)
  {
    ASSERT(fScope)
    FScopeToColor(fScope, defaultConsoleColor);
  }

  
  //
  // Configure console colors
  //
  static void ConfigureConsoleColors(FScope *fScope)
  {
    ASSERT(fScope)

    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      U32 type = sScope->NameCrc();
      Color c;
      FScopeToColor(sScope, c);
      consoleColors.Add(type, new Color(c));
    }
  }


  //
  // Configure color groups
  //
  static void ProcessCreateColorGroup(FScope *fScope)
  {
    ASSERT(fScope)

    const char *name = fScope->NextArgString();
    const char *base = fScope->GetArgCount() > 1 ? fScope->NextArgString() : NULL;
    FScope *sScope;

    // Find the color group
    ColorGroup *c = CreateColorGroup(name);
    ASSERT(c)

    // Is there a base? if so copy its ass
    if (base)
    {
      ColorGroup *c2 = FindColorGroup(Crc::CalcStr(base));

      if (c2 && (c != c2))
      {
        memcpy(c, c2, sizeof(ColorGroup));
      }
    }

    // Read in each setting
    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x7C801FA5: // "NormalBg"
          FScopeToColor(sScope, c->bg[ColorGroup::NORMAL]);
          break;

        case 0x39A43E0C: // "NormalFg"
          FScopeToColor(sScope, c->fg[ColorGroup::NORMAL]);
          break;

        case 0xA4758AEE: // "SelectedBg"
          FScopeToColor(sScope, c->bg[ColorGroup::SELECTED]);
          break;

        case 0xE151AB47: // "SelectedFg"
          FScopeToColor(sScope, c->fg[ColorGroup::SELECTED]);
          break;

        case 0xACADD3A8: // "HilitedBg"
          FScopeToColor(sScope, c->bg[ColorGroup::HILITED]);
          break;

        case 0xE989F201: // "HilitedFg"
          FScopeToColor(sScope, c->fg[ColorGroup::HILITED]);
          break;

        case 0xDC54D3B9: // "HilitedSelBg"
          FScopeToColor(sScope, c->bg[ColorGroup::SELHILITED]);
          break;

        case 0x9970F210: // "HilitedSelFg"
          FScopeToColor(sScope, c->fg[ColorGroup::SELHILITED]);
          break;

        case 0x51BB620B: // "DisabledBg"
          FScopeToColor(sScope, c->bg[ColorGroup::DISABLED]);
          break;

        case 0x149F43A2: // "DisabledFg"
          FScopeToColor(sScope, c->fg[ColorGroup::DISABLED]);
          break;

        case 0x7DC41647: // "AllBg"
        {
          Color clr;
          FScopeToColor(sScope, clr);

          for (U32 i = 0; i < ColorGroup::MAX_INDEX; i++)
          {
            c->bg[i] = clr;
          }
          break;
        }

        case 0x38E037EE: // "AllFg"
        {
          Color clr;
          FScopeToColor(sScope, clr);

          for (U32 i = 0; i < ColorGroup::MAX_INDEX; i++)
          {
            c->fg[i] = clr;
          }
          break;
        }

        case 0xE688BD45: // "NormalTexture"
          FScopeToTextureInfo(sScope, c->textures[ColorGroup::NORMAL]);
          break;

        case 0xABFE44A3: // "SelectedTexture"
          FScopeToTextureInfo(sScope, c->textures[ColorGroup::SELECTED]);
          break;

        case 0x5330A9A1: // "HilitedTexture"
          FScopeToTextureInfo(sScope, c->textures[ColorGroup::HILITED]);
          break;

        case 0x588A3B34: // "HilitedSelTexture"
          FScopeToTextureInfo(sScope, c->textures[ColorGroup::SELHILITED]);
          break;

        case 0xDCE70F6D: // "DisabledTexture"
          FScopeToTextureInfo(sScope, c->textures[ColorGroup::DISABLED]);
          break;      

        case 0xB1DA7108: // "AllTextures"
        {
          TextureInfo ti;
          FScopeToTextureInfo(sScope, ti);

          for (U32 i = 0; i < ColorGroup::MAX_INDEX; i++)
          {
            c->textures[i] = ti;
          }
          break;
        }

        default:
          LOG_ERR(("Invalid Color Group Item [%s]", sScope->NameStr()))
          break;
      }   
    }
  }


  //
  // Configure skin
  //
  static void ProcessCreateSkin(FScope *fScope)
  {
    ASSERT(fScope)

    const char *name = fScope->NextArgString();
    const char *base = fScope->GetArgCount() > 1 ? fScope->NextArgString() : NULL;

    // Find or create the skin
    TextureSkin *c = CreateTextureSkin(name);

    if (c)
    {
      // Is there a base? if so copy its ass
      if (base)
      {
        TextureSkin *c2 = FindTextureSkin(Crc::CalcStr(base));

        if (c2 && (c != c2))
        {
          memcpy(c, c2, sizeof(TextureSkin));
        }
      }

      FScopeToTextureSkin(fScope, *c);
    }
  }


  //
  // Read a configuration section
  //
  static void Configure(FScope *fScope)
  {
    FScope *sScope;

    ASSERT(fScope);

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x7BFCAF3E: // "DefineControlType"
        {
          DefineControlType(sScope);
          break;
        }

        case 0xDC817C35: // "CreateRegData"
        {
          CreateRegData(sScope);
          break;
        }

        case 0x5A948686: // "ScanCodes"
        {
          KeyBind::ProcessScanCodes(sScope);
          break;
        }

        case 0xEF9BD543: // "Modifiers"
        {
          KeyBind::ProcessModifiers(sScope);
          break;
        }

        case 0xC57E4440: // "CreateCursor"
        {
          CursorSys::ProcessCreateCursor(sScope);
          break;
        }

        case 0x390A51F7: // "CreateColorGroup"
        {
          ProcessCreateColorGroup(sScope);
          break;
        }

        case 0x2DA197F3: // "CreateFont"
        {
          // Create the font
          FontSys::Create(sScope);
          break;
        }

        case 0x291BAD22: // "CreateSkin"
        {
          ProcessCreateSkin(sScope);
          break;
        }

        case 0x2B3B14E3: // "Colors"
        {
          ConfigureColors(sScope);
          break;
        }

        case 0xFA934E14: // "DefaultConsoleColor"
        {
          ConfigureDefaultConsoleColor(sScope);
          break;
        }

        case 0x3EDD1FB9: // "ConsoleColors"
        {
          ConfigureConsoleColors(sScope);
          break;
        }

        case 0x824D2A30: // "Metrics"
        {
          ConfigureMetrics(sScope);
          break;
        }

        case 0xDCC321D2: // "Sound"
        {
          IFace::Sound::Configure(sScope);
          break;
        }

        case 0x14D3A281: // "StandardCursors"
        {
          CursorSys::ProcessStandardCursors(sScope);
          break;
        }
      }
    }
  }


  //
  // Send an event directly to a control
  //
  U32 SendEvent(IControl *ctrl, IControl *from, U32 type, U32 p1, U32 p2, U32 p3, U32 p4)
  {
    ASSERT(ctrl);

    Event e;
    e.type        = eventId;
    e.subType     = (U16)type;
    e.iface.to    = ctrl;
    e.iface.from  = from;
    e.iface.p1    = p1;
    e.iface.p2    = p2;
    e.iface.p3    = p3;
    e.iface.p4    = p4;

    // Send event directly to control
    return (PROCESS(ctrl, e, "Send"));
  }


  //
  // Send an event to a control via the event queue
  //
  U32 PostEvent(IControl *ctrl, IControl *from, U32 type, U32 p1, U32 p2, U32 p3, U32 p4)
  {
    Event *e = EventSys::PutLock();

    if (e)
    {
      // Create reapers for the to and from controls
      IControlPtr *ptrTo = NULL, *ptrFrom = NULL;

      // Setup the destination reaper, this is mandatory
      ptrTo = new IControlPtr;
      ptrTo->Setup(ctrl);

      // Setup the sender reaper, this is optional
      if (from)
      {
        ptrFrom = new IControlPtr;
        ptrFrom->Setup(from);
      }

      // Build up the event structure
      e->type      = eventId;
      e->subType   = (U16) type;
      e->iface.to  = ptrTo;
      e->iface.from= ptrFrom;
      e->iface.p1  = p1;
      e->iface.p2  = p2;
      e->iface.p3  = p3;
      e->iface.p4  = p4;

      EventSys::PutUnlock();

      return (TRUE);
    }
    else
    {
      return (FALSE);
    }
  }


  //
  // Send a notify event to the control
  //
  U32 SendNotify(IControl *ctrl, IControl *from, U32 event, U32 p1, U32 p2, U32 p3)
  {
    return (SendEvent(ctrl, from, IFace::NOTIFY, event, p1, p2, p3));
  }


  //
  // Send a notify event to the current modal control
  //
  U32 SendNotifyModal(IControl *from, U32 type, U32 p1, U32 p2, U32 p3)
  {
    IControl *ctrl = GetModal();
    if (ctrl)
    {
      return (SendNotify(ctrl, from, type, p1, p2, p3));
    }
    else
    {
      LOG_WARN(("SendNotifyModal: There is no modal control"))
      return (FALSE);
    }
  }


  //
  // Send a notify event to the current control which has focus
  //
  U32 SendNotifyFocus(IControl *from, U32 type, U32 p1, U32 p2, U32 p3)
  {
    IControl *ctrl = GetFocus();
    if (ctrl)
    {
      return (SendNotify(ctrl, from, type, p1, p2, p3));
    }
    else
    {
      LOG_WARN(("SendNotifyFocus: There is no control with focus"))
      return (FALSE);
    }
  }


  //
  // Post a notify event to the control
  //
  U32 PostNotify(IControl *ctrl, IControl *from, U32 event, U32 p1, U32 p2, U32 p3)
  {
    return (PostEvent(ctrl, from, IFace::NOTIFY, event, p1, p2, p3));
  }


  //
  // Post a notify event to the current modal control
  //
  U32 PostNotifyModal(IControl *from, U32 type, U32 p1, U32 p2, U32 p3)
  {
    IControl *ctrl = GetModal();
    if (ctrl)
    {
      return (PostNotify(ctrl, from, type, p1, p2, p3));
    }
    else
    {
      LOG_WARN(("PostNotifyModal: There is no modal control"))
      return (FALSE);
    }
  }


  //
  // Post a notify event to the current control which has focus
  //
  U32 PostNotifyFocus(IControl *from, U32 type, U32 p1, U32 p2, U32 p3)
  {
    IControl *ctrl = GetFocus();
    if (ctrl)
    {
      return (PostNotify(ctrl, from, type, p1, p2, p3));
    }
    else
    {
      LOG_WARN(("PostNotifyFocus: There is no control with focus"))
      return (FALSE);
    }
  }


  //
  // Return the Event id obtained from EventSys
  //
  U16 EventID() 
  { 
    return (eventId);
  }


  //
  // Time since last input event
  //
  U32 LastInputEventTime()
  {
    if (lastInputEvent > Main::thisTime)
    {
      return (0);
    }
    else
    {
      return (Main::thisTime - lastInputEvent);
    }
  }


  //
  // Shuts down all windows in the interface except for the root window and 
  // console.  Typically done between run code changes.
  //
  void DisposeAll()
  {
    // Clear flags
    systemFlags = 0;

    // Reset alpha scale
    ResetFade();

    // Unbind keys
    KeyBind::RemoveAll();

    // Clean up control stacks
    modalStack.DisposeAll();

    ClearReapers();

    // Delete all controls
    root->DisposeChildren();
    overlays->DisposeChildren();

    // Delete all registered templates
    BinTree<ICClass>::Iterator i(&ctrlTypes);
    ICClass *c;

    while ((c = i++) != NULL)
    {
      if (c->derived)
      {
        ctrlTypes.Unlink(c->type.crc);
        delete c;
      }
    }
  
    // Delete all cursors
    CursorSys::DeleteAll();

    // Delete fonts
    FontSys::DeleteAll();

    // Delete unmanaged bitmaps
    unmanagedBitmaps.DisposeAll();

    // Delete custom color groups
    customColorGroups.DisposeAll();

    // Delete texture skins
    textureSkins.DisposeAll();

    // Delete registry
    scopeRegistry.DisposeAll();

    // Reset cursor state
    Input::ShowCursor(TRUE);
  }


  //
  // Refresh all top level controls by deactiving/reactivating
  //
  void Refresh()
  {
    // Hide all modal windows
    if (modal.Alive())
    {
      (*modal).Deactivate();
    }

    for (;;)
    {
      IControlPtr *ctrl = modalStack.Pop();

      if (ctrl)
      {
        if (ctrl->Alive()) 
        {
          (*ctrl)->Deactivate();
        }

        // Delete the memory used by the reaper
        delete ctrl;
      }
      else
      {
        break;
      }
    }

    ClearReapers();

    root->RefreshControls();
    overlays->RefreshControls();
  }


  //
  // Delete or deactivate all top level non system controls
  //
  void PurgeNonSystem(Bool)
  {
    ASSERT(sysInit)
    ASSERT(root)
    ASSERT(overlays)

    root->PurgeNonSystem();
    overlays->PurgeNonSystem();
  }


  //
  // Activate a window
  //
  Bool Activate(const char *name)
  {
    // Find the control
    IControl *ctrl = FindByName(name, root);

    // Activate it
    if (ctrl)
    {
      return (Activate(ctrl));
    }
    return (FALSE);
  }


  //
  // Activate a window
  //
  Bool Activate(IControl *ctrl)
  {
    ASSERT(ctrl)

    if (systemFlags & DISABLE_ACTIVATE)
    {
      return (FALSE);
    }

    if (ctrl->Activate())
    {
      // Bring it to the top
      ctrl->SetZPos(0);
      return (TRUE);
    }
    else
    {
      return (FALSE);
    }
  }


  //
  // Deactivate a window
  //
  Bool Deactivate(const char *name)
  {
    // Find the control
    IControl *ctrl = root->FindByName(name, NULL);

    // Deactivate it
    if (ctrl)
    {
      return (Deactivate(ctrl));
    }
    else
    {
      return (FALSE);
    }
  }


  //
  // Deactivate a window
  //
  Bool Deactivate(IControl *ctrl)
  {
    return (ctrl->Deactivate());
  }


  // 
  // Toggle active state
  //
  Bool ToggleActive(const char *name)
  {
    // Find the control
    IControl *ctrl = root->FindByName(name, NULL);

    // (De)activate it
    if (ctrl)
    {
      if (ctrl->IsActive())
      {
        Deactivate(ctrl);
      }
      else
      {
        Activate(ctrl);
      }
    }

    return FALSE;
  }


  //
  // Setup current tip control
  //
  void SetTipControl(IControl *c)
  {
    if (tipCtrl.Alive())
    {
      LOG_WARN(("Tip Control [%s] already alive", tipCtrl->Name()))
    }

    tipCtrl = c;
  }


  //
  // Find a root level control by name
  //
  IControl *FindByName(const char *name, IControl *base)
  {
    ASSERT(sysInit);
    ASSERT(root);

    return (root->FindByName(name, base));
  }


  //
  // Return a pointer to the root window
  //
  IControl *RootWindow()
  {
    return (root);
  }


  //
  // Return a pointer to the overlays window
  //
  IControl *OverlaysWindow()
  {
    return (overlays);
  }


  //
  // Return a pointer to the game window control, always succeeds
  //
  IControl *GameWindow()
  {
    return (root);
  }


  //
  // SetGameHandler
  //
  EventSys::HANDLERPROC SetGameHandler(EventSys::HANDLERPROC proc)
  {
    return (root->SetGameHandler(proc));
  }


  //
  // Draw ALL the interface, including 2d/3d and page flipping
  //
  void PaintAll()
  {
    // Render interface
    Vid::RenderBegin();
    Vid::RenderClear();

    Render();

    Vid::RenderEnd();
    Vid::RenderFlush();
  }
  

  //
  // Default repaint function
  //
  static void DefaultRenderProc()
  {
    // Delete all controls that are marked
    IControl::DeleteMarkedControls();

    // Render all top level controls

    // turn off all unnecessary vid services
    //
    Vid::SetPerspectiveStateI( FALSE);
    Bool antialias = Vid::SetAntiAliasStateI( FALSE);
    Bool specular  = Vid::SetSpecularStateI( FALSE);
    Bool fog       = Vid::SetFogStateI( FALSE);

    // turn off wireframe; don't notify vars (I)
    //
    Bool shade = Vid::SetShadeStateI( wireFrame ? Vid::shadeWIRE : Vid::shadeGOURAUD);

    // turn off texture filtering; don't notify vars (I)
    //
    U32 filter = Vid::SetFilterStateI( 0);

    // turn on texturing
    //
    Bool tex = Vid::SetTextureStateI(TRUE);

    // Draw root window
    {
      PaintInfo p = root->GetPaintInfo();
      root->Draw(p);
    }

    // Draw overlays
    {
      PaintInfo p = overlays->GetPaintInfo();
      overlays->Draw(p);
    }

    // draw the cursor
    if (Input::CursorVisible())
    {
      CursorSys::Display(Input::MousePos().x, Input::MousePos().y);
    }

    // Flush buckets now, but keep them valid
    DoneBuckets();
    InitBuckets();

    // Restore texturing
    //
    if (!tex)
    {
      Vid::SetTextureStateI(tex);
      Vid::SetTexture( NULL);
    }

    // Restore texture filtering; don't notify vars (I)
    //
    Vid::SetFilterStateI( filter);
    Vid::SetFogStateI( fog);
    Vid::SetSpecularStateI( specular);
    Vid::SetAntiAliasStateI( antialias);
    Vid::SetPerspectiveStateI( TRUE);

    // Restore wireframe
    Vid::SetShadeStateI( shade);
  }

  // Previous alpha value
  static U32 prevAlpha;

  //
  // Begin rendering
  //
  void InitRender()
  {
    ASSERT (!(systemFlags & DISABLE_DRAW))

#ifdef DOSTATISTICS
    Statistics::tempTris = 0;
#endif

    // Call render function
    PERF_S("IFace::Render3D");

    PERF_S("Flush buckets");

    Vid::FlushBuckets();
    Vid::FlushTranBuckets();

    PERF_E("Flush buckets");

    // Setup defaults
    Vid::SetTexture(NULL);
    Vid::SetMaterial(NULL);
    prevAlpha = Vid::SetAlphaState( TRUE);

    // Setup bucket pointers
    InitBuckets();
  }


  //
  // Finish Rendering
  //
  void DoneRender()
  {
    ASSERT (!(systemFlags & DISABLE_DRAW))

    // Flush the buckets
    DoneBuckets();

    // Restore settings
    Vid::SetAlphaState(prevAlpha);

#ifdef DOSTATISTICS
    Statistics::ifaceTris = Statistics::tempTris;
#endif

    PERF_E("IFace::Render3D");
  }


  //
  // Just call the render function
  //
  void CallRenderProc()
  {
    ASSERT(paintProc)
    ASSERT (!(systemFlags & DISABLE_DRAW))

    paintProc();
  }


  //
  // Render Top level controls that require 3d rendering
  //
  void Render()
  {
    if (systemFlags & DISABLE_DRAW)
    {
      return;
    }

    InitRender();
    CallRenderProc();
    DoneRender();
  }


  //
  // SetMouseCapture
  //
  // Set "ctrl" to have mouse capture
  //
  void SetMouseCapture(IControl *ctrl)
  {
    // If capture has changed, redraw control with capture
    if (ctrl != mouseCapture.GetData())
    {
      // Notify new capture control
      if (ctrl)
      {
        SendEvent(ctrl, NULL, GOTCAPTURE);
      }

      // Notify previous capture control
      if (mouseCapture.Alive())
      {
        SendEvent(mouseCapture, NULL, LOSTCAPTURE);
      }
    }

    // Point the reaper to the new control
    mouseCapture.Setup(ctrl);
  }


  //
  // ReleaseMouseCapture
  //
  // Release mouse capture from "ctrl"
  //
  void ReleaseMouseCapture(IControl *ctrl)
  {
    ASSERT(ctrl);

    // The control releasing the capture really should already have it
    if (ctrl != mouseCapture.GetData())
    {
      //LOG_DIAG(("PANIC: capture[%s]!=ctrl[%s]", mouseCapture.Alive() ? (*mouseCapture).Name() : "", ctrl->Name()));
      return;
    }

    // Notify control that it has lost focus
    if (mouseCapture.Alive())
    {
      SendEvent(mouseCapture, NULL, LOSTCAPTURE);
    }

    // Clear the capture reaper
    mouseCapture.Clear();
  }


  //
  // Return the control that has mouse capture
  //
  IControl *GetCapture() 
  { 
    return mouseCapture.Alive() ? mouseCapture.GetData() : NULL; 
  }


  //
  // Return the control that is beneath the mouse
  //
  IControl *GetMouseOver() 
  { 
    return mouseOver.Alive() ? mouseOver.GetData() : NULL;
  }


  //
  // Set "ctrl" to have keyboard focus
  //
  void SetFocus(IControl *ctrl)
  {
    // If focus has changed, redraw control with capture
    if (ctrl != focus.GetData())
    {
      // Notify new focus control
      if (ctrl)
      {
        SendEvent(ctrl, NULL, GOTFOCUS);
      }

      // Notify previous focus control
      if (focus.Alive())
      {
        SendEvent(focus, NULL, LOSTFOCUS);
      }
    }

    // Point the reaper to the new control
    focus.Setup(ctrl);
  }


  //
  // ReleaseFocus
  //
  // Release keyboard focus from "ctrl"
  //
  void ReleaseFocus(IControl *ctrl)
  {
    ASSERT(ctrl);

    // The control releasing the capture really should already have it
    if (ctrl != focus.GetData())
    {
      //LOG_DIAG(("PANIC: focus[%s]!=ctrl[%s]", focus.Alive() ? (*focus).Name() : "", ctrl->Name()));
      return;
    }

    // Notify control that it has lost focus
    if (focus.Alive())
    {
      SendEvent(focus, NULL, LOSTFOCUS);
    }

    // Otherwise clear the focus reaper
    focus.Clear();
  }


  //
  // Return the control that has keyboard focus
  //
  IControl *GetFocus() 
  { 
    return focus.Alive() ? focus.GetData() : NULL;
  }


  //
  // SetModal
  //
  // Set "ctrl" to be modal
  //
  void SetModal(IControl *ctrl)
  {
    ASSERT(ctrl);

    // If a control is already modal, push it onto the stack
    if (modal.Alive())
    {
      // Create a new reaper
      IControlPtr *ptr = new IControlPtr(modal);

      // Add it to the stack
      modalStack.Push(ptr);
    }

    // Point reaper to the new control
    modal.Setup(ctrl);

    // Clear Capture and focus
    SetMouseCapture(NULL);
    SetFocus(NULL);
  }


  //
  // UnsetModal
  //
  // Unset modal state of "ctrl"
  //
  void UnsetModal(IControl *ctrl)
  {
    ASSERT(ctrl);

    if (ctrl != modal.GetData())
    {
      //LOG_DIAG(("PANIC: modal[%s]!=ctrl[%s]", modal.Alive() ? (*modal).Name() : "", ctrl->Name()));

      modalStack.DisposeAll();
    }

    // Point reaper to control on top of stack
    IControlPtr *ptr = modalStack.Pop();

    // Keep popping till we find an active control
    while (ptr)
    {
      if (ptr->Alive() && (*ptr)->IsActive())
      {
        // Ok, we have an alive and active control
        break;
      }
      else
      {
        // throw that one away and get the next one
        //LOG_DIAG(("UnsetModal: throwing away [%s]", ptr->Alive() ? (*ptr)->Name() : "DEAD"));

        // Delete current reaper
        delete ptr;

        // Get the next one
        ptr = modalStack.Pop();
      }
    }

    if (ptr)
    {
      // Set up the modal reaper from the popped reaper
      modal.Setup(*ptr);

      // Dispose of the popped reaper
      delete ptr;
    }
    else
    {
      // Clear the modal reaper
      modal.Clear();
    }
  }


  //
  // Return the current modal control
  //
  IControl *GetModal() 
  { 
    return modal.Alive() ? modal.GetData() : NULL;
  }


  //
  // Deactivate all modal controls
  //
  void DeactivateModals()
  {
    U32 count = 0;

    while (modal.Alive())
    {
      modal->Deactivate();

      if (count++ > 100)
      {
        LOG_ERR(("DeactivateModals infinite loop?"))
      }
    }
  }


  //
  // Simulate the caret
  //
  void ProcessCaret(S32 timeStep)
  {
    if ((caretMs -= S32(timeStep)) < 0)
    {
      caretState = !caretState;
      caretMs = caretSpeed;

      if (focus.Alive())
      {
        IFace::SendEvent(focus, NULL, CARETCHANGED, caretState, 0);
      }
    }
  }


  //
  // The caret has moved - stops it flashing while moving
  //
  void CaretMoved()
  {
    caretState = TRUE;
    caretMs = caretSpeed;
  }


  //
  // HandleThisEvent
  //
  // Ask a control to handle an event event
  //
  static Bool HandleThisEvent(IControl *ctrl, Event &e, Bool hookCheck = FALSE)
  {
    ASSERT(ctrl)

    // Check if any controls up the hierarchy have input hooks
    if (hookCheck && ctrl->HasInputHook())
    {
      IControl *p = ctrl;

      for (;;)
      {
        IControl *p2 = p->Parent();

        if (p2->HasInputHook())
        {
          p = p2;
        }
        else
        {
          break;
        }
      }

      ASSERT(p)

      if (p != ctrl)
      {
        // Does this control want to hook this event?
        if (SendEvent(p, NULL, HOOKCHECK, e.subType))
        {
          // If it returns TRUE, then don't pass to the original control
          if (PROCESS(p, e, "Hook"))
          {
            //return (TRUE);
          }
        }
      }
    }

    // Pass to the original control
    return (PROCESS(ctrl, e, "Send"));
  }


  //
  // InputEventHandler
  //
  // Function to handle input events
  //
  Bool InputEventHandler(Event &e)
  {
    Bool passToControlUnderCursor = FALSE;

    // Store time of last input event
    lastInputEvent = Main::thisTime;

    // Process Console key first
    if (e.subType == Input::KEYDOWN)
    {
      if (!GetFlag(DISABLE_CONSOLE))
      {
        if ((e.input.code == DIK_F10) && (e.input.state == (Input::CTRLDOWN | Input::ALTDOWN)))
        {        
          // Activate or deactivate the console
          ToggleActive("Sys::Console");
          return (TRUE);
        }
      }

      if (e.input.code == DIK_SYSRQ)
      {
        // Take a screenshot
        ScreenDump();
        return (TRUE);
      }

      if (e.input.code == DIK_RETURN && (e.input.state & Input::ALTDOWN))
      {
        // Toggle fullscreen/windowed mode
        Vid::ToggleWindowedMode();
        return (TRUE);
      }
    }

    // Process the event normally
    switch (e.subType)
    {
      case Input::KEYDOWN:
      case Input::KEYUP:
      case Input::KEYCHAR:
      case Input::KEYREPEAT:
      {
        // Focus control has first bite at focus event
        if (focus.Alive())
        {
          if (HandleThisEvent(focus, e, TRUE))
          {
            // Event was handled by the control
            return (TRUE);
          }
        }
        else 
          
        if (!modal.Alive())
        {
          // Else let the key binding system have a go at it
          if (KeyBind::HandleEvent(e))
          {
            // Event was handled by the key binding system
            return (TRUE);
          }
        }

        // Do default behavior
        break;
      }

      case Input::MOUSEBUTTONDOWN:
      case Input::MOUSEBUTTONDBLCLK:
      {
        // Call hook function
        OnMouseClick();

        // Handle mouse clicks outside modal window
        while (modal.Alive() && !modal->InWindow(Point<S32>(e.input.mouseX, e.input.mouseY)))
        {
          if (modal->IsModalClose())
          {
            // Deactivate the modal control
            modal->Deactivate();
          }
          else
          {
            break;
          }
        }
      }
      // Intentional fall through

      case Input::MOUSEBUTTONUP:
      case Input::MOUSEBUTTONDBLCLKUP:
      case Input::MOUSEMOVE:
      case Input::MOUSEAXIS:
      {
        // Mouse movement events get passed to control under the cursor
        passToControlUnderCursor = TRUE;

        // Capture control has first bite at mouse events
        if (mouseCapture.Alive())
        {
          HandleThisEvent(mouseCapture, e, TRUE);
          return (TRUE);
        }

        // Do default behavior
        break;
      }
    }

    // Next, pass event to modal control
    if (modal.Alive())
    {
      if (passToControlUnderCursor && mouseOver.Alive() && modal->IsChild(mouseOver))
      {
        // Control under cursor is a child of the modal window
        HandleThisEvent(mouseOver, e, TRUE);
      }
      else
      {
        // Just hand it to the modal window
        HandleThisEvent(modal, e, TRUE);
      }
      return (TRUE);
    }

    // Finally, pass event to control under the mouse
    if (passToControlUnderCursor && mouseOver.Alive())
    {
      if (HandleThisEvent(mouseOver, e, TRUE))
      {
        return (TRUE);
      }
    }

    // Event was not handled
    return (FALSE);
  }


  //
  // IFaceEventHandler
  //
  // Function to handle interface events
  //
  Bool IFaceEventHandler(Event &e)
  {
    IControlPtr *ptrTo   = (IControlPtr *)e.iface.to;
    IControlPtr *ptrFrom = (IControlPtr *)e.iface.from;

    ASSERT(ptrTo);

    // Is the control still alive?
    if (ptrTo->Alive())
    {
      PROCESS((*ptrTo), e, "Send");
    }
    else
    {
      LOG_DIAG(("IFace Event to dead control"));
    }

    // Dispose of the reapers
    delete ptrTo;

    if (ptrFrom)
    {
      delete ptrFrom;
    }

    return FALSE;
  }


  //
  // Elapsed time since game started
  //
  U32 ElapsedMs()
  {
    return (elapsedMs);
  }


  //
  // Elapsed time since last frame processing
  //
  U32 TimeStepMs()
  {
    return (timeStepMs);
  }


  //
  // Elapsed time since last frame processing
  //
  F32 TimeStepSec()
  {
    return (timeStepSec);
  }


  //
  // Current screen width
  //
  S32 ScreenWidth()
  {
    ASSERT(data.backBuf);
    return (data.backBuf->Width());
  }

  
  //
  // Current screen height
  //
  S32 ScreenHeight()
  {
    ASSERT(data.backBuf);
    return (data.backBuf->Height());
  }

  
  //
  // Current screen depth
  //
  S32 ScreenDepth()
  {
    ASSERT(data.backBuf);
    return (data.backBuf->Depth());
  }

  
  //
  // Current pixel format
  //
  const Pix * ScreenPixelFormat()
  {
    ASSERT(data.backBuf);
    return (data.backBuf->PixelFormat());
  }


  //
  // Current state of the caret
  //
  Bool CaretState()
  {
    return (caretState);
  }


  //
  // Register an unmanaged bitmap to delete on DisposeAll
  //
  void RegisterUnmanagedBitmap(Bitmap *bitmap)
  {
    ASSERT(bitmap)
    unmanagedBitmaps.Append(bitmap);
  }


  //
  // Return a system metric
  //
  S32 GetMetric(U32 type)
  {
    if (type >= MAX_METRICS)
    {
      ERR_FATAL(("Metric out of range [%d]", type))
    }
    return (metricTable[type]);
  }


  //
  // Return a system color
  //
  const Color &GetColor(U32 type)
  {
    if (type >= MAX_COLORS)
    {
      ERR_FATAL(("Color out of range [%d]", type))
    }
    return (colorTable[type]);
  }


  //
  // Return a console color
  //
  const Color &GetConsoleColor(U32 type)
  {
    const Color *color = consoleColors.Find(type);

    if (color)
    {
      return (*color);
    }
    else
    {
      return (defaultConsoleColor);
    }
  }


  //
  // Set a console color
  //
  void SetConsoleColor(U32 type, const Color &color)
  {
    Color *c = consoleColors.Find(type);

    if (c)
    {
      *c = color;
    }
    else
    {
      consoleColors.Add(type, new Color(color));
    }
  }


  //
  // Create a system color group
  //
  static ColorGroup *CreateSystemColorGroup(const char *name)
  {
    ColorGroup *c;
    U32 key = Crc::CalcStr(name);

    if ((c = stdColorGroups.Find(key)) == NULL)
    {
      c = new ColorGroup;

      // Copy default settings
      if (data.cgDefault && (c != data.cgDefault))
      {
        memcpy(c, data.cgDefault, sizeof(ColorGroup));
      }

      // Add to list
      stdColorGroups.Add(key, c);
    }
    return (c);
  }


  //
  // Create a color group
  //
  ColorGroup *CreateColorGroup(const char *name)
  {
    ColorGroup *c;
    U32 key = Crc::CalcStr(name);

    // Find color group in standard list first, then custom group
    if ((c = FindColorGroup(key)) != NULL)
    {
      return (c);
    }

    // Wasn't found, so create a new one
    c = new ColorGroup;

    // Copy default settings
    if (data.cgDefault && (c != data.cgDefault))
    {
      memcpy(c, data.cgDefault, sizeof(ColorGroup));
    }

    // Add to list
    customColorGroups.Add(key, c);

    return (c);
  }

  
  //
  // Find a color group
  //
  ColorGroup *FindColorGroup(U32 id)
  {
    ColorGroup *c = NULL;

    if ((c = stdColorGroups.Find(id)) != NULL)
    {
      return (c);
    }
    else
    {
      return (customColorGroups.Find(id));
    }
  }


  //
  // Create a texture skin
  //
  TextureSkin *CreateTextureSkin(const char *name)
  {
    TextureSkin *t;
    U32 key = Crc::CalcStr(name);

    // Find texture skin
    if ((t = FindTextureSkin(key)) != NULL)
    {
      return (t);
    }

    // Wasn't found, so create a new one
    t = new TextureSkin;

    // Add to list
    textureSkins.Add(key, t);

    return (t);
  }


  //
  // Find a texture skin
  //
  TextureSkin *FindTextureSkin(U32 id)
  {
    return (textureSkins.Find(id));
  }


  //
  // Initailise colors
  //
  static void InitColors()
  {
    // Basic control color
    colorTable[CLR_STD_BG].Set(192L, 192L, 192L, 192L);
    colorTable[CLR_STD_FG].Set(  0L,   0L,   0L, 255L);

    // Client color
    colorTable[CLR_CLIENT_BG].Set(128L, 128L, 128L, 192L);
    colorTable[CLR_CLIENT_FG].Set(  0L,   0L,   0L, 255L);

    // Highlighted control color
    colorTable[CLR_HILITE_BG] = colorTable[CLR_STD_BG];
    colorTable[CLR_HILITE_FG].Set(0L, 96L, 144L, 255L);

    // Selected control color
    colorTable[CLR_SELECTED_BG].Set(  0L,  96L, 144L, 192L);
    colorTable[CLR_SELECTED_FG].Set(255L, 255L, 255L, 255L);

    // Disabled control color
    colorTable[CLR_DISABLED_BG].Set(128L, 128L, 128L, 128L);
    colorTable[CLR_DISABLED_FG].Set( 32L,  32L,  32L, 255L);

    // Titlebar color
    colorTable[CLR_TITLE_FG ].Set( 220L, 220L, 220L, 192L);
		colorTable[CLR_TITLE_BG1].Set(  16L,   0L,  72L, 192L);
    colorTable[CLR_TITLE_BG2].Set( 144L, 168L, 255L, 192L);
    colorTable[CLR_TITLEHI_FG].Set(255L, 255L, 255L, 192L);

    // Inactive Titlebar color
    colorTable[CLR_TITLELO_FG ].Set(192L, 192L, 192L, 128L);
		colorTable[CLR_TITLELO_BG1].Set( 16L,   0L,  72L,  80L);
    colorTable[CLR_TITLELO_BG2].Set(144L, 168L, 255L,  80L);

    // CRC Lookup table
    memset(colorTableCrc, 0, sizeof(colorTableCrc));

    colorTableCrc[CLR_STD_BG]       = 0x6C3706C0; // "StdBg"
    colorTableCrc[CLR_STD_FG]       = 0x29132769; // "StdFg"
    colorTableCrc[CLR_CLIENT_BG]    = 0xBDA5B3AD; // "ClientBg"
    colorTableCrc[CLR_CLIENT_FG]    = 0xF8819204; // "ClientFg"
    colorTableCrc[CLR_HILITE_BG]    = 0xACADD3A8; // "HilitedBg"
    colorTableCrc[CLR_HILITE_FG]    = 0xE989F201; // "HilitedFg"
    colorTableCrc[CLR_SELECTED_BG]  = 0xA4758AEE; // "SelectedBg"
    colorTableCrc[CLR_SELECTED_FG]  = 0xE151AB47; // "SelectedFg"
    colorTableCrc[CLR_DISABLED_BG]  = 0x51BB620B; // "DisabledBg"
    colorTableCrc[CLR_DISABLED_FG]  = 0x149F43A2; // "DisabledFg"
    colorTableCrc[CLR_TITLE_BG1]    = 0x85973C67; // "TitleBg1"
    colorTableCrc[CLR_TITLE_BG2]    = 0x88D41ABE; // "TitleBg2"
    colorTableCrc[CLR_TITLE_FG]     = 0xE22FDAA3; // "TitleFg"
    colorTableCrc[CLR_TITLEHI_FG]   = 0xC3DE8C80; // "TitleHiFg"
    colorTableCrc[CLR_TITLELO_BG1]  = 0x0838536F; // "TitleLoBg1"
    colorTableCrc[CLR_TITLELO_BG2]  = 0x057B75B6; // "TitleLoBg2"
    colorTableCrc[CLR_TITLELO_FG]   = 0xBBF82B97; // "TitleLoFg"

    // Init default color group
    InitDefaultColorGroups();
  }


  //
  // Shutdown colors
  // 
  static void DoneColors()
  {
    // Delete standard colors
    stdColorGroups.DisposeAll();

    // Delete console colors
    consoleColors.DisposeAll();
  }


  //
  // Init default color groups
  //
  static void InitDefaultColorGroups()
  {
    // Initialise default color group
    data.cgDefault = CreateSystemColorGroup("Sys::Default");

    data.cgDefault->bg[ColorGroup::NORMAL]      = colorTable[CLR_STD_BG];
    data.cgDefault->fg[ColorGroup::NORMAL]      = colorTable[CLR_STD_FG];
    data.cgDefault->bg[ColorGroup::SELECTED]    = colorTable[CLR_SELECTED_BG];
    data.cgDefault->fg[ColorGroup::SELECTED]    = colorTable[CLR_SELECTED_FG];
    data.cgDefault->bg[ColorGroup::HILITED]     = colorTable[CLR_HILITE_BG];
    data.cgDefault->fg[ColorGroup::HILITED]     = colorTable[CLR_HILITE_FG];
    data.cgDefault->bg[ColorGroup::SELHILITED]  = colorTable[CLR_SELECTED_BG];
    data.cgDefault->fg[ColorGroup::SELHILITED]  = colorTable[CLR_SELECTED_FG];
    data.cgDefault->bg[ColorGroup::DISABLED]    = colorTable[CLR_DISABLED_BG];
    data.cgDefault->fg[ColorGroup::DISABLED]    = colorTable[CLR_DISABLED_FG];

    // Client color group
    data.cgClient = CreateSystemColorGroup("Sys::Client");

    data.cgClient->bg[ColorGroup::NORMAL]      = colorTable[CLR_CLIENT_BG];
    data.cgClient->fg[ColorGroup::NORMAL]      = colorTable[CLR_CLIENT_FG];
    data.cgClient->bg[ColorGroup::SELECTED]    = colorTable[CLR_SELECTED_BG];
    data.cgClient->fg[ColorGroup::SELECTED]    = colorTable[CLR_SELECTED_FG];
    data.cgClient->bg[ColorGroup::HILITED]     = colorTable[CLR_CLIENT_BG];
    data.cgClient->fg[ColorGroup::HILITED]     = colorTable[CLR_CLIENT_FG];
    data.cgClient->bg[ColorGroup::SELHILITED]  = colorTable[CLR_SELECTED_BG];
    data.cgClient->fg[ColorGroup::SELHILITED]  = colorTable[CLR_SELECTED_FG];
    data.cgClient->bg[ColorGroup::DISABLED]    = colorTable[CLR_DISABLED_BG];
    data.cgClient->fg[ColorGroup::DISABLED]    = colorTable[CLR_DISABLED_FG];

    // Title bar color group
    data.cgTitle = CreateSystemColorGroup("Sys::Titlebar");

    data.cgTitle->bg[ColorGroup::NORMAL]      = colorTable[CLR_TITLE_BG1];
    data.cgTitle->fg[ColorGroup::NORMAL]      = colorTable[CLR_TITLE_FG];
    data.cgTitle->bg[ColorGroup::SELECTED]    = colorTable[CLR_TITLE_BG1];
    data.cgTitle->fg[ColorGroup::SELECTED]    = colorTable[CLR_TITLE_FG];
    data.cgTitle->bg[ColorGroup::HILITED]     = colorTable[CLR_TITLE_BG1];
    data.cgTitle->fg[ColorGroup::HILITED]     = colorTable[CLR_TITLEHI_FG];
    data.cgTitle->bg[ColorGroup::SELHILITED]  = colorTable[CLR_TITLE_BG1];
    data.cgTitle->fg[ColorGroup::SELHILITED]  = colorTable[CLR_TITLEHI_FG];
    data.cgTitle->bg[ColorGroup::DISABLED]    = colorTable[CLR_TITLELO_BG1];
    data.cgTitle->fg[ColorGroup::DISABLED]    = colorTable[CLR_TITLELO_FG];

    // List item color group
    data.cgListItem = CreateSystemColorGroup("Sys::ListItem");

    data.cgListItem->bg[ColorGroup::NORMAL]      = Color(0L, 0L, 0L, 0L);
    data.cgListItem->fg[ColorGroup::NORMAL]      = colorTable[CLR_STD_FG];
    data.cgListItem->bg[ColorGroup::SELECTED]    = colorTable[CLR_SELECTED_BG];
    data.cgListItem->fg[ColorGroup::SELECTED]    = colorTable[CLR_SELECTED_FG];
    data.cgListItem->bg[ColorGroup::HILITED]     = Color(0L, 0L, 0L, 0L);
    data.cgListItem->fg[ColorGroup::HILITED]     = colorTable[CLR_HILITE_FG];
    data.cgListItem->bg[ColorGroup::SELHILITED]  = colorTable[CLR_SELECTED_BG];
    data.cgListItem->fg[ColorGroup::SELHILITED]  = colorTable[CLR_HILITE_FG];
    data.cgListItem->bg[ColorGroup::DISABLED]    = Color(0L, 0L, 0L, 0L);
    data.cgListItem->fg[ColorGroup::DISABLED]    = colorTable[CLR_DISABLED_FG];

    // Opaque texture color group
    data.cgTexture = CreateSystemColorGroup("Sys::Texture");

    data.cgTexture->bg[ColorGroup::NORMAL]      = Color(255L, 255L, 255L, 255L);
    data.cgTexture->fg[ColorGroup::NORMAL]      = Color(255L, 255L, 255L, 255L);
    data.cgTexture->bg[ColorGroup::SELECTED]    = Color(255L, 255L, 255L, 255L);
    data.cgTexture->fg[ColorGroup::SELECTED]    = Color(255L, 255L, 255L, 255L);
    data.cgTexture->bg[ColorGroup::HILITED]     = Color(255L, 255L, 255L, 255L);
    data.cgTexture->fg[ColorGroup::HILITED]     = Color(255L, 255L, 255L, 255L);
    data.cgTexture->bg[ColorGroup::SELHILITED]  = Color(255L, 255L, 255L, 255L);
    data.cgTexture->fg[ColorGroup::SELHILITED]  = Color(255L, 255L, 255L, 255L);
    data.cgTexture->bg[ColorGroup::DISABLED]    = Color(192L, 192L, 192L, 255L);
    data.cgTexture->fg[ColorGroup::DISABLED]    = Color(192L, 192L, 192L, 255L);

    // Translucent texture color group
    data.cgTransTexture = CreateSystemColorGroup("Sys::TranslucentTexture");
    memcpy(data.cgTransTexture, data.cgTexture, sizeof ColorGroup);

    data.cgTransTexture->bg[ColorGroup::NORMAL].a     = 160;
    data.cgTransTexture->bg[ColorGroup::SELECTED].a   = 160;
    data.cgTransTexture->bg[ColorGroup::HILITED].a    = 160;
    data.cgTransTexture->bg[ColorGroup::SELHILITED].a = 160;
    data.cgTransTexture->bg[ColorGroup::DISABLED].a   = 160;
  }


  //
  // Initailse metrics
  //
  static void InitMetrics()
  {
    memset(metricTable, 0, sizeof(metricTable));

    metricTable[SLIDER_WIDTH]       = 12;
    metricTable[BUTTON_WIDTH]       = 80;
    metricTable[BUTTON_HEIGHT]      = 24;
    metricTable[TITLE_HEIGHT]       = 16;
    metricTable[THIN_TITLE_HEIGHT]  = 11;
    metricTable[DROPSHADOW_UP]      = 2;
    metricTable[DROPSHADOW_DOWN]    = 1;
    metricTable[SHADOW_ALPHA]       = 88;
    metricTable[BORDER_THIN]        = 1;
    metricTable[BORDER_THICK]       = 3;
    metricTable[VGRADIENT]          = 150;
    metricTable[HGRADIENT]          = 110;
    metricTable[TITLE_FONT]         = Crc::CalcStr("System");
    metricTable[THIN_TITLE_FONT]    = Crc::CalcStr("Small");
    metricTable[POLL_DELAY]         = 500;
    metricTable[TIP_DELAY]          = 800;

    // CRC Lookup table
    memset(metricTableCrc, 0, sizeof(metricTableCrc));

    metricTableCrc[SLIDER_WIDTH]      = 0x841B6B65; // "SliderWidth"
    metricTableCrc[BUTTON_WIDTH]      = 0x858BB44A; // "ButtonWidth"
    metricTableCrc[BUTTON_HEIGHT]     = 0xBE6F9EAE; // "ButtonHeight"
    metricTableCrc[TITLE_HEIGHT]      = 0x73940B81; // "TitleHeight"
    metricTableCrc[THIN_TITLE_HEIGHT] = 0x0F4DBD9D; // "ThinTitleHeight"
    metricTableCrc[DROPSHADOW_UP]     = 0xCF2E0EE7; // "DropShadowUp"
    metricTableCrc[DROPSHADOW_DOWN]   = 0xC02281C3; // "DropShadowDown"
    metricTableCrc[SHADOW_ALPHA]      = 0xED3EA652; // "ShadowAlpha"
    metricTableCrc[BORDER_THIN]       = 0x9EC9A0D7; // "BorderThin"
    metricTableCrc[BORDER_THICK]      = 0xFB5FA20E; // "BorderThick"
    metricTableCrc[VGRADIENT]         = 0x74B064A2; // "VGradient"
    metricTableCrc[HGRADIENT]         = 0xC736BA70; // "HGradient"
    metricTableCrc[TITLE_FONT]        = 0x351225BF; // "TitleFont"
    metricTableCrc[THIN_TITLE_FONT]   = 0x88DB892E; // "ThinTitleFont"
    metricTableCrc[POLL_DELAY]        = 0x1F446A4E; // "PollDelay"
    metricTableCrc[TIP_DELAY]         = 0x5590A3C2; // "TipDelay"

    UpdateScreenMetrics();
  }


  //
  // List controls on the console
  //
  Bool EnumChildrenCallback(IControl *control, U32 context)
  {
    char buf[256];
    char *s = buf;

    U32 indent = (context & 0xFFFF) << 1;

    // Build up string
    *s = '\0';

    // Verbose mode
    if (context & 0x40000000)
    {
      U32 state = control->GetControlState();

      Utils::Sprintf
      (
        s, 256, "[size:%4d,%4d pos:%4d,%4d %c%c%c%c]  ", 
        control->GetSize().x, 
        control->GetSize().y, 
        control->GetPos().x, 
        control->GetPos().y,
        state & IControl::STATE_SELECTED ? 'S' : '.',
        state & IControl::STATE_HILITE   ? 'H' : '.',
        state & IControl::STATE_DISABLED ? 'D' : '.',
        state & IControl::STATE_ACTIVE   ? 'A' : '.');

      s += strlen(s);
    }

    for (U32 i = 0; i < indent; i++)
    {
      *s++ = ' ';
    }

    // Control name
    Utils::Strmcpy(s, control->Name(), sizeof(buf) - (s - buf));

    CON_DIAG((buf))

    // Enumerate its children if recursion is on
    if (context & 0x80000000)
    {
      if (!Promote<ConsoleViewer>(control))
      {
        control->EnumChildren(EnumChildrenCallback, context+1);
      }
    }

    return TRUE;
  }


  //
  // Register Commands and vars
  //
  void CmdInit()
  {
    // Register command handlers and scopes
    VarSys::RegisterHandler("iface", CmdHandler);
    VarSys::RegisterHandler(DYNAMICDATA_SCOPE, CmdHandler);

    // Register commands
    VarSys::CreateCmd("iface.activate");
    VarSys::CreateCmd("iface.deactivate");
    VarSys::CreateCmd("iface.toggleactive");
    VarSys::CreateCmd("iface.closedialog");
    VarSys::CreateCmd("iface.resize");

    VarSys::CreateCmd("iface.listclasses");
    VarSys::CreateCmd("iface.listcontrols");
    VarSys::CreateCmd("iface.listfonts");
    VarSys::CreateCmd("iface.writefonts");
    VarSys::CreateCmd("iface.screenshot");
    VarSys::CreateCmd("iface.setlocale");
    VarSys::CreateCmd("iface.disabledraw");
    VarSys::CreateCmd("iface.disableactivate");
    VarSys::CreateCmd("iface.sendnotifyevent");
    VarSys::CreateCmd("iface.ifconsolemsg");
    VarSys::CreateCmd("iface.deactivatemodals");

    // System variables
    const char *language = MultiLanguage::GetLanguage();
    if (language == NULL)
    {
      ERR_FATAL(("No language configured"))
    }

    VarSys::CreateString("iface.language", language, VarSys::NOEDIT);

    #ifdef DEVELOPMENT
      VarSys::CreateCmd("iface.setalpha");
      VarSys::CreateCmd("iface.fadeup");
      VarSys::CreateCmd("iface.testmodechange");
      VarSys::CreateCmd("iface.testmsgbox");
    #endif

    VarSys::CreateInteger("iface.wireframe", FALSE, VarSys::DEFAULT, &wireFrame);
  }


  //
  // Unregister Commands and vars
  //
  void CmdDone()
  {
    // Remove var scope
    VarSys::DeleteItem("iface");
  }


  //
  // Test callback for iface.testmsgbox
  //
  static void TestCallBack(U32 id, U32)
  {
    LOG_DIAG(("Button 0x%.8X pressed", id))
  }



  //
  // Message hook callback for iface.ifmessage
  //
  static Bool IfMessageHook(const CH *, U32 &, void *context)
  {
    ASSERT(context)
    ASSERT(*(U32 *)context == 0)

    // Set the flag to true
    *(U32 *)context = TRUE;

    // Stop enumerating
    return (FALSE);
  }


  //
  // Handle console commands
  //
  void CmdHandler(U32 pathCrc)
  {
    switch (pathCrc)
    {
      case 0x9FD05ECF: // "iface.disabledraw"
      {
        S32 flag;
        if (Console::GetArgInteger(1, flag))
        {
          SetFlag(DISABLE_DRAW, flag ? TRUE : FALSE);
        }
        break;
      }

      case 0xB6138D00: // "iface.disableactivate"
      {
        S32 flag;
        if (Console::GetArgInteger(1, flag))
        {
          SetFlag(DISABLE_ACTIVATE, flag ? TRUE : FALSE);
        }
        break;
      }


      // Process args
      case 0x9EE90A99: // "iface.listcontrols"
      {
        const char *args;
        U32 argc = 1;
        U32 context = 0;
        IControl *base = root;

        // Process args
        for (;;)
        {
          if (!Console::GetArgString(argc++, args))
          {
            break;
          }

          switch (Crc::CalcStr(args))
          {
            case 0x8B5AB605: // "/r"
            {
              // Recursive listing
              context |= 0x80000000;
              break;
            }

            case 0x985EC0D9: // "/v"
            {
              // Verbose listing
              context |= 0x40000000;
              break;
            }

            default:
            {
              IControl *ctrl = IFace::FindByName(args);
              if (ctrl)
              {
                base = ctrl;
              }
              break;
            }
          }
        }

        // Enum the children
        ASSERT(base);
        base->EnumChildren(EnumChildrenCallback, context);

        break;
      }

      case 0x0301E78C: // "iface.listclasses"
      {
        for (BinTree<ICClass>::Iterator i(&ctrlTypes); *i; i++)
        {
          if ((*i)->derived)
          {
            CON_DIAG(("%s (%s)", (*i)->type.str, (*i)->base.str))
          }
          else
          {
            CON_DIAG(((*i)->type.str))
          }
        }
        break;
      }

      case 0x971AD37F: // "iface.activate"
      {
        const char *name;
    
        // Check for one string argument
        if (!Console::GetArgString(1, name))
        {
          CON_ERR((Console::ARGS))
        }
        else
        {
          Activate(name);
        }
        break;
      }

      case 0x76F6033F: // "iface.deactivate"
      {
        const char *name;
    
        // Check for one string argument
        if (!Console::GetArgString(1, name))
        {
          CON_ERR((Console::ARGS))
        }
        else
        {
          Deactivate(name);
        }
        break;
      }

      case 0x1B48EEF0: // "iface.toggleactive"
      {
        const char *name;
    
        // Check for one string argument
        if (!Console::GetArgString(1, name))
        {
          CON_ERR((Console::ARGS))
        }
        else
        {
          ToggleActive(name);
        }
        break;
      }

      case 0x7C2B3DA1: // "iface.closedialog"
      {
        if (modal.Alive())
        {
          (*modal).Deactivate();
        }
        break;
      }

      case 0xA0718455: // "iface.resize"
      {
        const char *name;
        Point<S32> size;
    
        // Check for one string argument
        if (!(Console::GetArgString(1, name) && Console::GetArgInteger(2, size.x) && Console::GetArgInteger(3, size.y)))
        {
          CON_ERR((Console::ARGS))
        }
        else
        {
          // Find the control
          IControl *ctrl = FindByName(name, root);

          // Resize it
          if (ctrl)
          {
            Point<S32> pos = ctrl->GetPos();
            ctrl->Resize(size);
            ctrl->SetPos(pos.x, pos.y);
          }
        }
        break;
      }

      case 0x3FC8ECAA: // "iface.listfonts"
      {
        FontSys::Report();
        break;
      }

      case 0x51723C76: // "iface.writefonts"
      {
        FontSys::WriteFonts();
        break;
      }

      case 0x4DE07566: // "iface.screenshot"
      {
        ScreenDump();
        break;
      }

      case 0x0D5B5622: // "iface.setlocale"
      {
        // Remove this when ML system is written
        //FIXME(915843011, "aiarossi"); // Fri Jan 08 16:50:11 1999

        const char *locale;
    
        // Check for one string argument
        if (!Console::GetArgString(1, locale))
        {
          CON_ERR((Console::ARGS))
        }
        else
        {
          const char *ret = setlocale(LC_CTYPE, locale);

          if (ret)
          {
            CON_DIAG(("Locale now '%s'", ret))
          }
          else
          {
            CON_ERR(("Unable to set locale"))
          }
        }
        break;
      }

      case 0x9D767A92: // "iface.sendnotifyevent"
      {
        const char *name;
        const char *event;
    
        // Check for two string arguments
        if (!Console::GetArgString(1, name) || !Console::GetArgString(2, event))
        {
          CON_ERR((Console::ARGS))
        }
        else
        {
          const char *arg1 = "";
          Console::GetArgString(3, arg1);

          const char *arg2 = "";
          Console::GetArgString(4, arg2);

          const char *arg3 = "";
          Console::GetArgString(5, arg3);

          // Find the control
          IControl *ctrl = FindByName(name, root);

          if (ctrl)
          {
            // Send the event to the control
            SendEvent(ctrl, NULL, IFace::NOTIFY, Crc::CalcStr(event), Crc::CalcStr(arg1), Crc::CalcStr(arg2), Crc::CalcStr(arg3));
          }
          else
          {
            // Couldn't find the control
            CON_ERR(("Could not find control '%s'", name))
          }
        }
        break;
      }

      case 0x91AC6DBA: // "iface.ifconsolemsg"
      {
        const char *msgs, *cmd;

        if (Console::GetArgString(1, msgs) && Console::GetArgString(2, cmd))
        {
          BinTree<U32> filters;
          U32 flag = FALSE;

          // FIXME: Split msgs up into ';' delimited values
          filters.Add(Crc::CalcStr(msgs));

          if (filters.GetCount())
          {
            Console::EnumStrings(&filters, IfMessageHook, &flag);

            if (flag)
            {
              Console::ProcessCmd(cmd);
            }
            filters.DisposeAll();
          }
        }
        break;
      }

      case 0x04EFD033: // "iface.deactivatemodals"
      {
        DeactivateModals();
        break;
      }


      #ifdef DEVELOPMENT
        // 
        // Debugging only
        //
        case 0x09D5D144: // "iface.setalpha"
        {
          F32 f;

          if (Console::GetArgFloat(1, f))
          {
            data.alphaScale = Clamp<F32>(0.0F, f, 1.0F);
          }
          break;
        }

        case 0x5E0670DE: // "iface.fadeup"
        {
          F32 f;

          if (Console::GetArgFloat(1, f) && f > 1e-4F)
          {
            SetFade(0.0F, f);
          }
          break;
        }

        case 0x650D4B05: // "iface.testmodechange"
        {
          // Test the mode change function without actually changing modes
          OnModeChange();
          break;
        }

        case 0xD54EF697: // "iface.testmsgbox"
        {
          const char *title, *str;

          if (Console::GetArgString(1, title) && Console::GetArgString(2, str))
          {
            const char *caption;
            MBEvent *b1 = NULL, *b2 = NULL, *b3 = NULL;

            if (!Console::GetArgString(3, caption))
            {
              caption = "Ok";
            }
            
            b1 = new MBEventCallback(caption, TRANSLATE((caption)), TestCallBack);
            LOG_DIAG(("Added [%s]=0x%.8X", caption, Crc::CalcStr(caption)))

            // Button 2 and 3 are optional
            if (Console::GetArgString(4, caption))
            {
              b2 = new MBEventCallback(caption, TRANSLATE((caption)), TestCallBack);
              LOG_DIAG(("Added [%s]=0x%.8X", caption, Crc::CalcStr(caption)))
            }
            if (Console::GetArgString(5, caption))
            {
              b3 = new MBEventCallback(caption, TRANSLATE((caption)), TestCallBack);
              LOG_DIAG(("Added [%s]=0x%.8X", caption, Crc::CalcStr(caption)))
            }

            MsgBox(TRANSLATE((title)), TRANSLATE((str)), 0, b1, b2, b3);
          }
          else
          {
            CON_ERR((Console::ARGS))
          }
          break;
        }
      #endif
    }
  }
}


