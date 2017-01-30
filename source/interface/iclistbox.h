/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Listbox Control
//
// 20-JUL-98
//


#ifndef __ICLISTBOX_H
#define __ICLISTBOX_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"
#include "reaperlist.h"


///////////////////////////////////////////////////////////////////////////////
//
// ICListBox messages
//
namespace ICListBoxMsg
{
  const U32 ClearSelection  = 0x19A88BD2; // "ListBox::Message::ClearSelected"
  const U32 Rebuild         = 0xCF92FAFC; // "ListBox::Message::Rebuild"
  const U32 Sort            = 0x5332FA74; // "ListBox::Message::Sort"
  const U32 DeleteAll       = 0xB7378729; // "ListBox::Message::DeleteAll"
  const U32 SetSelected     = 0x62DA10CA; // "ListBox::Message::SetSelected"

  const U32 ScrollLine      = 0xCB437AB8; // "ListBox::Message::ScrollLine"
  const U32 ScrollPage      = 0xEFDC65A2; // "ListBox::Message::ScrollPage"
  const U32 ScrollHome      = 0xC18E827C; // "ListBox::Message::ScrollHome"
  const U32 ScrollEnd       = 0x1EBB7D74; // "ListBox::Message::ScrollEnd"

  const U32 MoveUp          = 0xA81BFF0E; // "ListBox::Message::MoveUp"
  const U32 MoveDown        = 0xA416A328; // "ListBox::Message::MoveDown"
}

namespace ICListBoxNotify
{
  const U32 ChangeSelection = 0xBE40C402; // "ListBox::SelChange"
  const U32 ClearSelection  = 0xCB12B5E5; // "ListBox::SelClear"
  const U32 SelectionClick  = 0x0C9FD6B1; // "ListBox::SelClick"
  const U32 DoubleClick     = 0xCF867F7B; // "ListBox::DblClick"
}


///////////////////////////////////////////////////////////////////////////////
//
// Class ICListBoxSpacer
//
class ICListBoxSpacer : public IControl
{
  PROMOTE_LINK(ICListBoxSpacer, IControl, 0x23FFE602); // "ICListBoxSpacer"

public:

  // Constructor
  ICListBoxSpacer(IControl *parent) 
  : IControl(parent) 
  { 
  }

  // Sizing constructor
  ICListBoxSpacer(IControl *parent, S32 height)
  : IControl(parent)
  {
    SetGeomSize(0, height);
  }

  // Redraw self
  void DrawSelf(PaintInfo &pi);

};


///////////////////////////////////////////////////////////////////////////////
//
// Class ICListBox
//
class ICListBox : public IControl
{
  PROMOTE_LINK(ICListBox, IControl, 0x8000AF9C); // "ICListBox"

public:

  // ICListBox styles
  enum 
  {
    // Create a slider
    STYLE_VSLIDER           = 0x00000001,

    // No selection
    STYLE_NOSELECTION       = 0x00000002,   

    // Multiple selection
    STYLE_MULTISELECT       = 0x00000004,

    // Auto scroll to last item added
    STYLE_AUTOSCROLL        = 0x00000008,

    // Selection can be cleared
    STYLE_CANCLEAR          = 0x00000010,

    // Wrap text added to the slider
    STYLE_WRAP              = 0x00000020,

    // Automatic tool tips for text items
    STYLE_AUTOTIPTEXT       = 0x00000040,

    // Don't rebuild automatically on activation
    STYLE_NOAUTOREBUILD     = 0x00000080,

    // Don't allow select changes via input
    STYLE_NOSELECTIONINPUT  = 0x00000100,

    // Smart scrolling
    STYLE_SMARTSCROLL       = 0x00000200

  };

  // Rebuild structure
  struct Rebuild;

  // Control container
  struct ListContainer;

protected:

  ///////////////////////////////////////////////////////////////////////////////
  //
  // List of selected items
  //
  struct SelectNode : public IControlPtr
  {
    // List node
    NList<SelectNode>::Node node;
  };

  typedef ReaperList<IControl, SelectNode> SelectedList;

protected:

  // Current List box style
  U32 listBoxStyle;

  // Container for list items
  ListContainer *container;

  // Configuration scope for listbox items
  FScope *itemConfig;

  // Configuration for list slider
  FScope *sliderCfg;

  // List of selected items
  SelectedList selectedList;

  // Value of selected item, in single selection listboxes
  IFaceVar *selectedVar;

  // Index of the selected item, in singled selection listboxes
  IFaceVar *indexVar;

  // Index of first item
  IFaceVar *topVar;

  // Desired index of first item, to be set on next redraw
  S32 desiredTop;

  // Padding between cells
  S32 cellPad;

  // Associated slider
  char *slider;

  // Number of elements in list
  VarInteger count;

  // First visible element
  VarInteger top;
  
  // Number of Visible elements in list
  VarInteger vis;

  // Do children need to be repositioned?
  U32 reposition,

  // Update the var at the next oportunity
      updateVar : 1;

  // Sounds
  U8 soundClick;
  U8 soundDblClick;

protected:

  // Draw the control
  void DrawSelf(PaintInfo &pi);

  // Calculate an adjustment rect based on the style
  ClipRect GetAdjustmentRect();

  // PostConfigure
  void PostConfigure();

  // Setup cell size if needed, only done once per activation
  Bool SetupCellSize();

  // Setup the selected item
  Bool Select(IControl *item, Bool clear = FALSE, Bool toggle = FALSE, Bool notify = TRUE);

  // Update var ranges
  void UpdateRange(Bool clear = FALSE);

public:

  ICListBox(IControl *parent);
  ~ICListBox();

  // Activate the listbox
  Bool Activate();

  // Deactivate the listbox
  Bool Deactivate();

  // Return the number of items in the list
  U32 ItemCount();

  // Get the list of items
  const NList<IControl> & GetItems();

  // Clear the selected item
  Bool ClearSelected(Bool notify = TRUE);

  // Set the var to use for the selected item
  void SetSelectedItemVar(const char *name);

  // Return the key of the selected item
  Bool GetSelectedItem(const char * &);

  // Return the control of the selected item
  IControl * GetSelectedItem();

  // Returh the index of the selected item
  U32 GetSelectedIndex();

  // Get the selected list
  const SelectedList & GetSelectedList()
  {
    return (selectedList);
  }

  // Set the selected item by key 
  // (for multiple selection lists this adds the item to the selected list)
  Bool SetSelectedItem(const char *, Bool force = FALSE, Bool notify = TRUE);

  // Set the selected item by index
  // (for multiple selection lists this adds the item to the selected list)
  Bool SetSelectedItem(U32 index, Bool force = FALSE, Bool notify = TRUE);

  // Set the selected item by control
  // (for multiple selection lists this adds the item to the selected list)
  Bool SetSelectedItem(IControl *ctrl, Bool force = FALSE, Bool notify = TRUE);

  // Enumerate list of selected items
  void EnumSelected(Bool (*proc)(IControl *, U32), U32 context);

  // Enumerate list of selected itmes
  void EnumSelected(void (*proc)(const char *key, const CH *display, void *context), void *context);

  // Enumerate list of non-selected items
  void EnumNonSelected(void (*proc)(const char *key, const CH *display, void *context), void *context);

  // Construct a text control with id and optional name and add it to the list
  void AddTextItem(const char *id, const CH *display, const Color *textColor = NULL, const Point<S32> &indent = Point<S32>(0, 0));

  // Construct a bitmap control with id and image and add it to the list
  IControl *AddBitmapItem(const char *id, const char *file);

  // Add a pre-constructed control to the list
  void AddItem(const char *id, IControl *ctrl, Bool configured = FALSE);

  // Remove a control from the using the given name
  void RemoveItem(const char *id);

  // Remove a control from the list (marks it for deletion)
  void RemoveItem(IControl *ctrl);

  // Delete all items from the list
  void DeleteAllItems();

  // Configure this control with an FScope
  void Setup(FScope *fScope);

  // Handle notifications
  void Notify(IFaceVar *var);

  // Style configuration
  Bool SetStyleItem(const char *s, Bool toggle);

  // Event handling function
  U32 HandleEvent(Event &e);

  U32 FillFromPath( const char *path, const char *filter = "*.*", Bool doClear = TRUE, Bool doSubDirs = FALSE);

  // Find a ICListBox control
  static ICListBox *FindListBox(const char *path);

  // Sort the contents of the list, optionally using the description
  void Sort(Bool useDesc = FALSE);

  // PreRebuild
  Rebuild *PreRebuild();

  // PostRebuild - NOTE this deletes rebuild
  void PostRebuild(Rebuild **rebuild);

public:

  // Force a reposition
  void ForceReposition()
  {
    reposition = TRUE;
  }

};


///////////////////////////////////////////////////////////////////////////////
//
// Type definitions
//
typedef Reaper<ICListBox> ICListBoxPtr;


#endif
