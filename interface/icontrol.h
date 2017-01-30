/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//

#ifndef __ICONTROL_H
#define __ICONTROL_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "varsys.h"
#include "event.h"
#include "iftypes.h"
#include "iface_types.h"
#include "promotelink.h"


///////////////////////////////////////////////////////////////////////////////
//
// Forward declarations
//
class FScope;
class IControl;


///////////////////////////////////////////////////////////////////////////////
//
// Type definitions
//
typedef Reaper<IControl> IControlPtr;
typedef StrCrc<64>       IControlName;


///////////////////////////////////////////////////////////////////////////////
//
// IControl Messages
//
namespace IControlMsg
{
  const U32 PrevTabStop = 0x6BC23246; // "Control::Message::PrevTabStop"
  const U32 NextTabStop = 0xB1B89D1B; // "Control::Message::NextTabStop"
}

namespace IControlNotify
{
  const U32 Activating    = 0x4DAD7AC7; // "Window::Notify::Activating"
  const U32 Activated     = 0xF2ED9F61; // "Window::Notify::Activated"
  const U32 Deactivating  = 0x606286E1; // "Window::Notify::Deactivating"
  const U32 Deactivated   = 0x6AC3454B; // "Window::Notify::Deactivated"
  const U32 PreResize     = 0x4F2E858C; // "Control::Notify::PreResize"
  const U32 PostResize    = 0x980620DB; // "Control::Notify::PostResize"
}


///////////////////////////////////////////////////////////////////////////////
//
// Class IControl - Base class for all interface controls
//
class IControl
{
  PROMOTE_BASE(IControl, 0x05313784); // "IControl"

protected:

  LOGDEC

public:

  // Control styles
  enum
  {
    // Transparent
    STYLE_TRANSPARENT   = 0x00000001,

    // Thin border
    STYLE_THINBORDER    = 0x00000008,

    // Thick border
    STYLE_THICKBORDER   = 0x00000010,

    // Invert border
    STYLE_SUNKENBORDER  = 0x00000020,

    // Drop shadow
    STYLE_DROPSHADOW    = 0x00000040,  

    // Modal
    STYLE_MODAL         = 0x00000080,

    // Shutdown modal window when clicks outside window
    STYLE_MODALCLOSE    = 0x00000100,

    // Does not respond to mouse clicks
    STYLE_INERT         = 0x00000200,

    // Draw with vertical gradient
    STYLE_VGRADIENT     = 0x00000400,

    // Draw with horizontal gradient
    STYLE_HGRADIENT     = 0x00000800,      

    // Draw with titlebar gradient color scheme
    STYLE_TITLEGRADIENT = 0x00001000,

    // Is control a tab stop?
    STYLE_TABSTOP       = 0x00002000,

    // Don't automatically activate when parent is activated
    STYLE_NOAUTOACTIVATE= 0x00008000,

    // Fade in on activation
    STYLE_FADEIN        = 0x00010000,

    // Set tab stop on activation, even if not modal
    STYLE_SETTABSTOP    = 0x00020000,

    // Text is split over multiple lines
    STYLE_MULTILINE     = 0x00040000,

    // Should the activation state of this control be saved
    STYLE_SAVESTATE     = 0x00080000,

    // System wide control (i.e. important)
    STYLE_SYSTEMWIDE    = 0x40000000,
  };

  // Control state
  enum 
  {
    // NOTE: Lowest 3 bits are used as an index into color mask

    // Selected
		STATE_SELECTED    = 0x00000001,

    // Highlited
		STATE_HILITE      = 0x00000002,

    // Disabled
		STATE_DISABLED    = 0x00000004,

    // Color mask
    STATE_MASK_COLOR  = STATE_SELECTED |
                        STATE_HILITE |
                        STATE_DISABLED,

    // Active
    STATE_ACTIVE      = 0x00000008,

    // Drawn
    STATE_VISIBLE     = 0x00000010,
  };

  // Geometry settings
  enum
  {
    // Client based
    GEOM_RIGHT            = (1 <<  0),
    GEOM_BOTTOM           = (1 <<  1),
    GEOM_HCENTRE          = (1 <<  2),
    GEOM_VCENTRE          = (1 <<  3),
    GEOM_PARENTWIDTH      = (1 <<  4),
    GEOM_PARENTHEIGHT     = (1 <<  5),

    // Window based
    GEOM_WINLEFT          = (1 <<  6),
    GEOM_WINRIGHT         = (1 <<  7),
    GEOM_WINTOP           = (1 <<  8),
    GEOM_WINBOTTOM        = (1 <<  9),
    GEOM_WINHCENTRE       = (1 << 10),
    GEOM_WINVCENTRE       = (1 << 11),
    GEOM_WINPARENTWIDTH   = (1 << 12),
    GEOM_WINPARENTHEIGHT  = (1 << 13),

    GEOM_SQUARE           = (1 << 14),
    GEOM_AUTOSIZEX        = (1 << 15),
    GEOM_AUTOSIZEY        = (1 << 16),
    GEOM_AUTOSIZE         = GEOM_AUTOSIZEX | GEOM_AUTOSIZEY,

    // Alignment
    GEOM_HINTERNAL        = (1 << 17),
    GEOM_VINTERNAL        = (1 << 18),
    GEOM_ALIGNTOWIDTH     = (1 << 19),
    GEOM_ALIGNTOHEIGHT    = (1 << 20),

    GEOM_KEEPVISIBLE      = (1 << 21),

    // Mask for size related geometry flags
    GEOM_MASK_SIZE_X  = GEOM_PARENTWIDTH | 
                        GEOM_WINPARENTWIDTH |
                        GEOM_SQUARE | 
                        GEOM_AUTOSIZEX |
                        GEOM_ALIGNTOWIDTH,

    GEOM_MASK_SIZE_Y  = GEOM_PARENTHEIGHT | 
                        GEOM_WINPARENTHEIGHT |
                        GEOM_SQUARE | 
                        GEOM_AUTOSIZEY |
                        GEOM_ALIGNTOHEIGHT,

    GEOM_MASK_SIZE    = GEOM_MASK_SIZE_X | 
                        GEOM_MASK_SIZE_Y,

    // Mask for position related geometry flags
    GEOM_MASK_POS_X   = GEOM_RIGHT | 
                        GEOM_HCENTRE | 
                        GEOM_WINRIGHT |
                        GEOM_WINLEFT |
                        GEOM_WINHCENTRE |
                        GEOM_HINTERNAL | 
                        GEOM_KEEPVISIBLE,

    GEOM_MASK_POS_Y   = GEOM_BOTTOM | 
                        GEOM_VCENTRE | 
                        GEOM_WINTOP |
                        GEOM_WINBOTTOM |
                        GEOM_WINVCENTRE |
                        GEOM_VINTERNAL | 
                        GEOM_KEEPVISIBLE,

    GEOM_MASK_POS     = GEOM_MASK_POS_X | 
                        GEOM_MASK_POS_Y,
  };

  // Text justification settings
  enum
  {
    JUSTIFY_LEFT      = 0x00000001,
    JUSTIFY_RIGHT     = 0x00000004,

    JUSTIFY_TOP       = 0x00000010,
    JUSTIFY_BOTTOM    = 0x00000040,

    JUSTIFY_HORZ      = JUSTIFY_LEFT | JUSTIFY_RIGHT,
    JUSTIFY_VERT      = JUSTIFY_TOP | JUSTIFY_BOTTOM,
  };

public:

  // Geometry setting structure
  struct Geometry
  {
    U32 flags;
    Point<S32> size;
    Point<S32> pos;
  };

  // Multi line text
  struct MultiLine
  {
    struct Item
    {
      const CH *text;
      U32 length;
    };

    Item *items;
    U32 count;

    MultiLine(U32 n);
    ~MultiLine();
  };

  // Death track info
  DTrack::Info dTrack;

protected:

  // Identifier of this control
  IControlName ident;

  // Current status
  U32 controlState;

  // Display style
  U32 controlStyle;

  // Position and size relative to parent
  Geometry geom;
  Point<S32> size;
  Point<S32> pos;

  // Parent of this control
  IControl *parent;

  // Optional irregular region
  Array<Point<S32> > *region;
  
  // Display information
  PaintInfo paintInfo;

  // Control to align geometry with
  IControlPtr alignTo;

  // List node for deletion list
  NList<IControl>::Node deleteNode;

  // List node for children list
  NList<IControl>::Node childNode;

  // List of child controls
  NList<IControl> children;

  // Texture
  TextureInfo *texture;

  // Skin
  TextureSkin *skin;

  // Text strings
  CH *textStr;
  char *formatStr;
  MultiLine *multiLine;

  // Text Position and dimension
  Point<S32> textPos;
  Point<S32> textSize;

  // Tool tip text
  CH *tipTextStr;

  // Text justification
  U32 textJustify;

  // Cursor information
  U32 cursor;

  // Free the text string?
  U32 freeText : 1,

  // Free tip text string?
      freeTipText : 1,
                    
  // Has been post-configured?
      postConfigured : 1,

  // Has an input hook installed
      inputHook : 1,

  // Is there a text color override?
      forceTextClr : 1;

  // Tree of input hooks
  BinTree<U32> inputHooks;

  // Event translations
  struct EventTranslation
  {
    U32 id;
    U32 param1;
    U32 param2;
    U32 param3;
    IControl *ctrl;
    NBinTree<EventTranslation>::Node node;

    // Constructor
    EventTranslation(U32 id, U32 param1, U32 param2, U32 param3, IControl *ctrl) 
    : id(id), param1(param1), param2(param2), param3(param3), ctrl(ctrl) {}
  };

  NBinTree<EventTranslation> translations;

  // Notification Actions
  BinTree<FScope> actions;

  // Polling list node
  NList<IControl>::Node pollNode;

  // All control list
  NList<IControl>::Node allNode;

  // Poll interval
  S32 pollInterval;
  S32 nextPollTime;

  // Alpha scale
  F32 alphaScale;

  // Forced text color
  Color forcedTextColor;

  // Var alias
  char *varAlias;

  // Context control (mainly for modal windows)
  IControlPtr context;

  // Friends of IControl
  friend class IFaceVar;
  friend class ICRoot;
  friend class ICListBox;

protected:

  // List of all controls
  static NList<IControl> allList;

  // List of all controls to be polled
  static NList<IControl> pollList;

  // List of all controls to be deleted
  static NList<IControl> deleteList;

  // Previously created contorl
  static IControlPtr previous;

protected:

  // Setup alignment to another control
  void SetupAlignment();

  // Finds and sets the tab stop
  Bool SetTabStop(IControl *from, Bool forward = TRUE);

  // Draw this control
  virtual void DrawSelf(PaintInfo &pi);

  // Draw all children of this control
  virtual void DrawChildren(PaintInfo &pi);

  // Draws the text of the object
  virtual void DrawCtrlText(const PaintInfo &pi, const CH *str = NULL, Color *clr = NULL, const Point<S32> &indent = Point<S32>(0, 0));

  // Redraws the control's background image
  virtual void DrawCtrlBackground(const PaintInfo &pi, const TextureInfo *tex = NULL);

  // Draws frame around control
  virtual void DrawCtrlFrame(const PaintInfo &pi);

  // Callback function for controls that are on the poll list
  virtual void Poll();

  // Add a control to the poll list
  void AddToPollList();

  // Remove a control from the poll list
  void RemoveFromPollList();

  // Function called when a var being watched changes value
  virtual void Notify(IFaceVar *var);

  // Configure an iface var
  void ConfigureVar(IFaceVar * &var, FScope *fScope);

  // Configure an iface var
  void ConfigureVar(IFaceVar * &var, const char *name);

  // Automatically resize geometry
  virtual void AutoSize();

  // Color mask for index into colorgroup array
  U32 ColorIndex() const
  {
    return (Min<U32>(controlState & STATE_MASK_COLOR, ColorGroup::MAX_INDEX-1));
  }

  // Return the control's texture info
  TextureInfo *GetTexture();

  // Generate multiline text
  void GenerateMultiLine(const CH *str);

  // Validate multiline text
  Bool ValidateMultiLine();

public:

  IControl(IControl *parent);

  virtual ~IControl();

  // Activate and optionally check the type of the var
  void ActivateVar(IFaceVar *var, VarSys::VarItemType type = VarSys::VI_NONE);
 
  // Mark this control for deletion next processing cycle
  void MarkForDeletion();

  // IsDying
  Bool IsDying()
  {
    return (deleteNode.InUse());
  }

  // Master Draw function, calls DrawSelf and DrawChildren
  void Draw(PaintInfo &pi);

  // Get PaintInfo data
  const PaintInfo &GetPaintInfo() const
  {
    return (paintInfo);
  }

  //
  // Child/parent functions
  //

  // Set this control's parent
  void SetParent(IControl *ctrl);

  // Add a child to this control
  void AddChild(IControl *ctrl);

  // Removes a child from this control
  void RemoveChild(IControl *ctrl);

  // Dispose of all children
  void DisposeChildren();

  // Enumerate list of children
  void EnumChildren(Bool (*proc)(IControl *, U32), U32 context);

  // Number of children
  U32 ChildCount();

  // Get the list of children
  const NList<IControl> & GetChildren()
  {
    return (children);
  }


  //
  // Basic configuration settings
  //

  // Setup this control from one scope function
  virtual void Setup(FScope *fScope);

  // Configure this control from a scope
  void Configure(FScope *fScope);

  // Find and read confuration of a control template
  Bool ReadTemplate(const char *name, Bool warn = FALSE);

  // Called after Configure() is completed
  virtual void PostConfigure();

  // Set the name of this control
  void SetName(const char *name);

  // Set the style for this control, arg list must be NULL terminated
  void CDECL SetStyle(const char *s, ...);

  // Style configuration
  virtual Bool SetStyleItem(const char *s, Bool toggle);

  // Set state
  Bool SetStateItem(const char *s, Bool toggle);

  // Set the geometry properties of this control
  void CDECL SetGeometry(const char *s, ...);

  // Geometry configuration
  Bool SetGeometryItem(const char *s, Bool toggle);

  // Set the control to align to
  Bool SetAlignTo(const char *name);

  // Set the control to align to
  Bool SetAlignTo(IControl *ctrl);

  // Adjust geometry of control
  virtual void AdjustGeometry();

  // Calculate an adjustment rect based on the style
  virtual ClipRect GetAdjustmentRect();

  // Set the size of this control
  void SetSize(S32 width, S32 height);

  // Set the geometry size of this control
  void SetGeomSize(S32 width, S32 height);

  // Set the position of this control
  void SetPos(S32 x, S32 y);

  // Set the color group
  void SetColorGroup(ColorGroup *c);

  // Force the text color of this control
  void OverrideTextColor(const Color *c);

  // Get the forced text color, or NULL if not applicable
  const Color *GetOverrideTextColor();

  // Set the texture skin
  void SetTextureSkin(TextureSkin *skin);

  // Set the background image of this control
  void SetImage(const TextureInfo *t);

  // Set the cursor that is displayed when over this control
  void SetCursor(U32 id);

  // Return the cursor
  U32 GetCursor()
  {
    return (cursor);
  }

  // Set the visible (drawn) state
  Bool SetVisible(Bool vis);

  // Set the context
  void SetContext(IControl *ctrl)
  {
    context = ctrl;
  }

  // Get the context
  IControl *GetContext()
  {
    return (context.Alive() ? context : NULL);
  }

  //
  // Text configuration options
  //

  // Set the text to display on this control
  virtual void SetTextString(const CH *s, Bool dup, Bool cleanup = FALSE);

  // Get the text being displayed on this control
  void GetTextString(CH *buf, U32 bufSize);

  // Get a pointer to the text being displayed on this control
  const CH * GetTextString();

  // Set the font to display text
  void SetTextFont(const char *fontName);

  // Set the font directly
  void SetFont(Font *font)
  {
    ASSERT(font)
    paintInfo.font = font;
  }

  // Set text justification via a JUSTIFY_* flag
  void SetTextJustify(U32 flag);

  // Set the sprintf style format string for text display
  void SetFormatSpec(const char *spec);

  // Set the tip text
  void SetTipText(const CH *s, Bool dup);

  // Find a child control at a position x,y
  IControl *Find(S32 x, S32 y, Bool all = FALSE);

  // Find a child control by CRC of name
  IControl *Find(U32 crc, Bool descend = FALSE);

  // Test if a control a child of this control
  Bool IsChild(IControl *ctrl);

  // Test if a point in screen coordinates lies within control or its children
  Bool InWindow(const Point<S32> &p) const;

  // Test if a point in screen coordinates lies within a control's client area
  Bool InClient(const Point<S32> &p) const;

  // Test if a point in window coordinates lies withing a control's client area
  Bool InClientFromWindow(const Point<S32> &p) const;

  // Convert screen coordinates to window coordinates
  Point<S32> ScreenToWindow(const Point<S32> &p) const;

  // Convert screen coordinates to client coordinates
  Point<S32> ScreenToClient(const Point<S32> &p) const;

  // Convert client coordinates to screen coordinates
  Point<S32> ClientToScreen(const Point<S32> &p) const;

  // Ask a control if it can activate
  virtual Bool CanActivate();

  // Activate Control, makes it visible and active and creates resources
  virtual Bool Activate();

  // Deactive control
  virtual Bool Deactivate();

  // Toggle the active/inactive state of this control
  Bool ToggleActive();

  // If flag is true, call Activate, otherwise call Deactivate
  Bool ChangeActiveState(Bool flag);

  // Move control to position
  void MoveTo(const Point<S32> &p);

  // Resize control
  void Resize(const Point<S32> &p);

  // Set Z-position of this control
  void SetZPos(U32 flags);

  // Activate the tip using the supplied text
  void ActivateTip(const CH *text);

  //
  // Event passing and handling
  //

  // Create a new event handler called "name"
  void AddEventScope(const char *name, FScope *scope);

  // Execute the fscope
  void ExecScope(FScope *fScope);

  // Execute a scope item
  virtual void ExecItem(FScope *fScope);

  // Add a notification translation entry
  Bool AddEventTranslation(U32 event, U32 id, IControl *ctrl, U32 param1 = 0, U32 param2 = 0, U32 param3 = 0);

  // Sends a notification event to another control
  Bool SendNotify(IControl *ctrl, U32 event, Bool translate = TRUE, U32 param1 = 0, U32 param2 = 0, U32 param3 = 0);

  // Sends a notification to all children
  Bool NotifyAllChildren(U32 event, Bool translate = TRUE, U32 param1 = 0, U32 param2 = 0, U32 param3 = 0, Bool propagate = TRUE);

  // Handle input and interface events
  virtual U32 HandleEvent(Event &e);

  //
  // Mouse capture
  //

  // Capture the mouse
  void GetMouseCapture();

  // Release mouse
  void ReleaseMouseCapture();

  // Test if this control has capture
  Bool HasMouseCapture();

  //
  // Keyboard focus
  //
  
  // Get keyboard focus
  void GetKeyFocus();

  // Release keyboard focus
  void ReleaseKeyFocus();

  // Test if this control has focus
  Bool HasKeyFocus();

  //
  // Modal state
  //

  // Set modal state
  void SetModal();

  // Unset modal state
  void UnsetModal();

  // Test if this control is modal
  Bool IsModal();


  //
  // Mouse over state
  //

  // Is the cursor over this control
  Bool IsMouseOver();

  //
  // Member variable access
  //

  // Return the name of the control
  const char *Name()
  {
    return (ident.str); 
  }

  // Return the crc identifier of the control
  U32 NameCrc()
  {
    return (ident.crc);
  }

  // Get tool tip string
  const CH *GetTipTextString()
  {
    return (tipTextStr);
  }

  // Return the control style
  U32 GetControlStyle() const
  {
    return (controlStyle);
  }

  // Return the control state
  U32 GetControlState() const
  {
    return (controlState);
  }

  // Is this control currently active
  Bool IsActive() const
  {
    return (controlState & STATE_ACTIVE);
  }

  // Is this control visible
  Bool IsVisible() const
  {
    return (controlState & STATE_VISIBLE);
  }

  // Is the control a tab stop?
  Bool IsTabStop() const
  {
    return (controlStyle & STYLE_TABSTOP);
  }

  // Does the control have the modal close style?
  Bool IsModalClose() const
  {
    return (controlStyle & STYLE_MODALCLOSE);
  }

  // Get position
  const Point<S32> &GetPos() const
  {
    return (pos);
  }

  // Get screen position
  Point<S32> GetScreenPos() const
  {
    return (ClientToScreen(Point<S32>(0, 0)));
  }

  // Get size
  const Point<S32> &GetSize() const
  {
    return (size);
  }

  // Return the CRC of the control's name
  U32 ID()
  {
    return (ident.crc); 
  }

  // Return the parent
  IControl *Parent()
  {
    return (parent);
  }

  // Does the control have an input hook?
  Bool HasInputHook()
  {
    return (inputHook);
  }

  // Set the poll interval for the control
  void SetPollInterval(S32 interval)
  {
    pollInterval = interval;
  }

  // Set the enabled state
  void SetEnabled(Bool flag, Bool recurse = FALSE);

  // Set the var alias
  void SetVarAlias(const char *alias);

  // Construct a name of a dynamic variable
  const char *DynVarName(const char *var = NULL);

  // Find a var name
  const char *FindVarName(const char *var);

  // Functions for creating local vars
  VarSys::VarItem * CreateInteger(const char *name, S32 dVal, S32 low = S32_MIN, S32 high = S32_MAX);
  VarSys::VarItem * CreateFloat(const char *name, F32 dVal, F32 low = F32_MIN, F32 high = F32_MAX);
  VarSys::VarItem * CreateString(const char *name, const char *dVal);

  // Process and "IFaceCmd" command
  void ProcessCmd(FScope *fScope);

public:

  // Update all polled objects
  static void ProcessPollList(U32 timeStepMs);

  // Delete all marked items
  static void DeleteMarkedControls();

  // Find control by dtrack id
  static IControl * FindById(U32 id);

  // Save and load control states
  static void Save(FScope *scope);
  static void Load(FScope *scope);
};

#endif
