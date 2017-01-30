/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Listbox Control
//
// 20-JUL-98
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "iclistbox.h"
#include "icstatic.h"
#include "iclistslider.h"
#include "iface.h"
#include "iface_types.h"
#include "iface_util.h"
#include "iface_priv.h"
#include "babel.h"
#include "iface_sound.h"
#include "ifvar.h"
#include "input.h"
#include "filesys.h"
#include "fscope.h"
#include "font.h"
#include "perfstats.h"
#include "stdload.h"


//#define LOG_LIST(x) LOG_DIAG(x)
#define LOG_LIST(x)


///////////////////////////////////////////////////////////////////////////////
//
// Struct ICListBox::Rebuild
//
struct ICListBox::Rebuild
{
  enum
  {
    TOP = (1 <<  0),
    SEL = (1 <<  1)
  };


  // Flags
  U32 flags;

  // Visible top
  S32 top;

  // Selected item
  GameIdent sel;


  // Constructor
  Rebuild() : flags(0) {}
};


///////////////////////////////////////////////////////////////////////////////
//
// Class ICListBox::ListContainer
//
struct ICListBox::ListContainer : public IControl
{
  // Constructor
  ListContainer(IControl *parent) : IControl(parent) 
  {
    geom.flags |= GEOM_PARENTWIDTH | GEOM_PARENTHEIGHT;
    SetName("[Container]");
  }

  void DrawChildren(PaintInfo &pi)
  {
    const U32 DISPLAY_MASK = STATE_ACTIVE | STATE_VISIBLE;

    NList<IControl>::Iterator i(&children);

    // Draw children from lowest Z-pos to highest
    for (i.GoToTail(); *i; i--)
    {
      IControl *child = *i;

      if ((child->GetControlState() & DISPLAY_MASK) == DISPLAY_MASK)
      {
        PaintInfo p = child->GetPaintInfo();

        // Convert to screen coordinates before drawing
        Point<S32> origin = child->GetPos() + pi.client.p0;
        p.window += origin;
        p.client += origin;

        // Clamp rectangles to client area of listbox
        if (p.window.p1.x > pi.client.p1.x)
        {
          p.window.p1.x = pi.client.p1.x;
          p.client.p1.x = pi.client.p1.x;
        }

        // Apply global alpha scale
        p.alphaScale *= IFace::data.alphaScale;

        child->Draw(p);
      }
    }
  }


  friend class ICListBox;
};


///////////////////////////////////////////////////////////////////////////////
//
// Class ICListBox
//

// Control names
const char *SliderCtrlName = "[ListSlider]";


//
// ICListBox::ICListBox
//
ICListBox::ICListBox(IControl *parent)
: IControl(parent), 
  listBoxStyle(0),
  selectedVar(NULL), 
  indexVar(NULL),
  slider(NULL), 
  itemConfig(NULL),
  sliderCfg(NULL),
  desiredTop(-1),
  reposition(FALSE),
  cellPad(2),
  updateVar(FALSE)
{
  // Default color
  SetColorGroup(IFace::data.cgClient);

  // Default styles
  controlStyle |= (STYLE_DROPSHADOW | STYLE_TABSTOP);

  // Default list bos styles
  listBoxStyle |= STYLE_VSLIDER;

  // Hook mouse click messages
  inputHook = TRUE;

  // Create container control
  container = new ListContainer(this);

  // Default sounds
  soundClick    = IFace::Sound::MENU_POPUP;
  soundDblClick = IFace::Sound::MENU_COMMAND;

  // Create local variables
  topVar = new IFaceVar(this, VarSys::CreateInteger(DynVarName("top"), 0, VarSys::DEFAULT, &top));
  VarSys::CreateInteger(DynVarName("count"), 0, VarSys::DEFAULT, &count);
  VarSys::CreateInteger(DynVarName("vis"), 0, VarSys::DEFAULT, &vis);

  // Update the range
  UpdateRange(TRUE);

  ClearSelected(FALSE);
}


//
// ICListBox::~ICListBox
//
ICListBox::~ICListBox()
{
  // Clear selected list
  selectedList.Clear();

  // Dispose of item config
  if (itemConfig)
  {
    delete itemConfig;
    itemConfig = NULL;
  }

  // Dispose of slider name
  if (slider)
  {
    delete slider;
    slider = NULL;
  }

  // Dispose of vars
  if (selectedVar)
  {
    delete selectedVar;
    selectedVar = NULL;
  }
  if (indexVar)
  {
    delete indexVar;
    indexVar = NULL;
  }
  if (topVar)
  {
    delete topVar;
    topVar = NULL;
  }
}


//
// ICListBox::SetSelectedItemVar
//
// Set the var to use for the selected item
//
void ICListBox::SetSelectedItemVar(const char *name)
{
  ConfigureVar(selectedVar, name);
}


//
// Compare function for sorting icontrols via qsort
//
static int CDECL CompareControls(const void *e1, const void *e2)
{
	IControl *c1 = *(IControl **) e1;
	IControl *c2 = *(IControl **) e2;

  int sort = Utils::Stricmp(c1->Name(), c2->Name());

  return sort;
}


//
// Compare function for sorting icontrols via qsort
//
static int CDECL CompareControlsDescription(const void *e1, const void *e2)
{
	IControl *c1 = *(IControl **) e1;
	IControl *c2 = *(IControl **) e2;

  int sort = Utils::Stricmp(c1->GetTextString(), c2->GetTextString());

  return sort;
}


//
// ICListBox::Sort
//
void ICListBox::Sort(Bool useDesc)
{
  if (container->children.GetCount())
  {
    IControl **array = new IControl*[container->children.GetCount()];
 
    U32 n = 0;

    for (NList<IControl>::Iterator li(&container->children); *li; li++, n++)
    {
      array[n] = *li;
    }
    container->children.UnlinkAll();

    qsort
    (
      (void *) array, (size_t) n, sizeof(IControl *), 
      useDesc ? CompareControlsDescription : CompareControls
    );

    for (U32 j = 0; j < n; j++)
    {
      container->children.Append( array[j]);
    }

    delete [] array;
  }
}


//
// PreRebuild
//
ICListBox::Rebuild *ICListBox::PreRebuild()
{
  Rebuild *r = new Rebuild;

  // Save the top
  r->top = top;
  r->flags |= Rebuild::TOP;

  // Save selected item
  const char *s;

  if (GetSelectedItem(s))
  {
    r->flags |= Rebuild::SEL;
    r->sel = s;
  }

  return (r);
}


//
// PostRebuild
//
void ICListBox::PostRebuild(ICListBox::Rebuild **rebuild)
{
  ASSERT(rebuild)
  ASSERT(*rebuild)

  Rebuild *r = *rebuild;

  UpdateRange();

  // Restore the top
  if (r->flags & Rebuild::TOP)
  {
    desiredTop = r->top;
    top = r->top;
  }

  // Restore selected item
  if (r->flags & Rebuild::SEL)
  {
    SetSelectedItem(r->sel.str);
  }

  delete r;
  r = NULL;
}


//
// ICListBox::DrawSelf
//
void ICListBox::DrawSelf(PaintInfo &pi)
{
  // Sync vars with actual item count
  if (count != container->children.GetCount())
  {
    UpdateRange(FALSE);
  }

  // Reposition all controls
  if (reposition)
  {
    reposition = FALSE;

    SetupCellSize();
    UpdateRange(FALSE);
  }

  // Redraw background
  DrawCtrlBackground(pi, GetTexture());

  // Redraw frame
  DrawCtrlFrame(pi);
}


//
// ICListBox::UpdateRange
//
// Update var ranges
//
void ICListBox::UpdateRange(Bool clear)
{
  count = container->children.GetCount();

  if (clear)
  {
    // Reset the range, and top
    top->SetIntegerRange(0, 0);
    top = 0;
  }
  else
  {
    // Set the range to the number of non-visible items in the list
    S32 range = count > vis ? count - vis : 0;
    top->SetIntegerRange(0, range);
  }

  // Apply the deferred top variable value
  if (desiredTop >= 0)
  {
    top = desiredTop;
    desiredTop = -1;
  }
}


//
// ICListBox::SetupCellSize
//
// Setup cell size if needed
//
Bool ICListBox::SetupCellSize()
{
  ASSERT(IsActive())

#ifdef LOGGING
  Clock::CycleWatch t;
  t.Start();
#endif

  S32 idx = 0;
  S32 visible = 0;
  NList<IControl>::Iterator i(&container->children);

  for (; idx < top; idx++, i++)
  {
    if (*i)
    {
      (*i)->MoveTo(Point<S32>(0, paintInfo.client.Height()));
      (*i)->SetVisible(FALSE);
    }
  }

  for (S32 y = 0; *i;)
  {
    IControl *p = *i;
    S32 h = p->GetPaintInfo().window.Height();

    // No more room
    if (y + h > GetPaintInfo().client.Height())
    {
      break;
    }

    // Move it
    p->MoveTo(Point<S32>(0, y));

    // And make it visible
    p->SetVisible(TRUE);

    /*
    // If it's bigger than the client window, setup tool tip
    if (p->GetPaintInfo().window.p1.x > paintInfo.client.p1.x)
    {
      if (p->textStr)
      {
        p->SetTipText(p->textStr, TRUE);
      }
    }
    else
    {
      p->SetTipText(NULL, FALSE);
    }
    */


    idx++;
    i++;
    visible++;
    y += h;
  }

  vis = visible;

  for (; *i; i++)
  {
    (*i)->MoveTo(Point<S32>(0, paintInfo.client.Height()));
    (*i)->SetVisible(FALSE);
  }

#ifdef LOGGING
  t.Stop();
  LOG_IFACE(("SetupCellSize [%s] %d cycles", Name(), U32(t.GetTotal())))
#endif

  return (TRUE);
}



//
// ICListBox::Select
//
// Select an item
//
Bool ICListBox::Select(IControl *item, Bool toggle, Bool clear, Bool notify)
{
  ASSERT(item)

  if (item == NULL)
  {
    return (FALSE);
  }

  // Ignore if there is no selection
  if (listBoxStyle & STYLE_NOSELECTION)
  {
    return (FALSE);
  }

  // If the item is in the list
  if (selectedList.Exists(item) && !toggle)
  {
    // Generate click notification
    if (IsActive())
    {
      SendNotify(this, ICListBoxNotify::SelectionClick);
    }
  }

  // Clear all selected items if necessary
  if (clear)
  {
    selectedList.Clear();
  }
  else
  {
    // This is already done in ClearSelected
    selectedList.PurgeDead();
  }

  if (toggle && (listBoxStyle & STYLE_MULTISELECT))
  {
    // Toggle selected state
    if (selectedList.Exists(item))
    {
      selectedList.Remove(item);
    }
    else
    {
      selectedList.Append(item);
    }
  }
  else
  {
    // Force selected state
    if (!selectedList.Exists(item))
    {
      selectedList.Append(item);

      // Set the var for single select boxes only
      if (selectedVar && (IsActive() || selectedVar->IsActive()))
      {
        selectedVar->SetValue(item->Name());
      }
      else
      {
        // Update the var next time we're activiated
        updateVar = TRUE;
      }

      // Set the index var for singled select boxes only
      if (indexVar && (IsActive() || indexVar->IsActive()))
      {
        S32 index = GetSelectedIndex();
        if (index != indexVar->GetIntegerValue())
        {
          indexVar->SetIntegerValue(index);
        }
      }
      else
      {
        // Update the var next time we're activated
        updateVar = TRUE;
      }
    }
  }

  // Update selected flags - FIXME: less than optimal
  {
    for (NList<IControl>::Iterator i(&container->children); *i; i++)
    {
      (*i)->controlState &= ~STATE_SELECTED;
    }
  }
  {
    for (SelectedList::Iterator i(&selectedList); *i; i++)
    {
      (**i)->controlState |= STATE_SELECTED;
    }
  }

  // If single select and the item is not visible, then modify the top variable
  if (!(listBoxStyle & STYLE_MULTISELECT) && IsActive() && (selectedList.GetCount() == 1))
  {
    IControl *sel = selectedList.GetFirst();

    if (!(sel->GetControlState() & STATE_VISIBLE))
    {
      top = GetSelectedIndex()-1;
    }
  }

  // Generate selection change notification
  if (IsActive() && notify)
  {
    if (selectedList.GetCount())
    {
      SendNotify(this, ICListBoxNotify::ChangeSelection, TRUE, item->NameCrc(), item->controlState & STATE_SELECTED);
    }
    else
    {
      SendNotify(this, ICListBoxNotify::ClearSelection, TRUE, item->NameCrc(), FALSE);
    }
  }

  return (TRUE);
}


//
// ICListBox::ItemCount
//
// Return the number of items in the list
//
U32 ICListBox::ItemCount()
{
  ASSERT(container)
  return (container->children.GetCount());
}


//
// ICListBox::GetItems
//
// Get the list of items
const NList<IControl> & ICListBox::GetItems()
{
  ASSERT(container)
  return (container->children);
}


//
// ICListBox::ClearSelected
//
// Clear the selected item
//
Bool ICListBox::ClearSelected(Bool notify)
{
  // Clear current list
  selectedList.PurgeDead();

  for (SelectedList::Iterator i(&selectedList); *i; i++)
  {
    (**i)->controlState &= ~STATE_SELECTED;
  }
  selectedList.Clear();

  // Generate selection cleared notification
  if (notify && IsActive())
  {
    SendNotify(this, ICListBoxNotify::ClearSelection);
  }

  // Set the var for single select boxes only
  if (!(listBoxStyle & STYLE_MULTISELECT))
  {
    if (selectedVar && (IsActive() || selectedVar->IsActive()))
    {
      selectedVar->SetValue("");
    }
  }

  return (TRUE);
}


//
// ICListBox::EnumSelected
//
// Enumerate list of selected items
//
void ICListBox::EnumSelected(Bool (*proc)(IControl *, U32), U32 context)
{
  selectedList.PurgeDead();

  for (SelectedList::Iterator i(&selectedList); *i; i++)
  {
    if (!proc(**i, context))
    {
      return;
    }
  }
}


//
// Enumerate list of selected itmes
//
void ICListBox::EnumSelected(void (*proc)(const char *key, const CH *display, void *context), void *context)
{
  selectedList.PurgeDead();

  for (SelectedList::Iterator i(&selectedList); *i; i++)
  {
    proc((**i)->Name(), (**i)->GetTextString(), context);
  }
}


//
// Enumerate list of non-selected items
//
void ICListBox::EnumNonSelected(void (*proc)(const char *key, const CH *display, void *context), void *context)
{
  for (NList<IControl>::Iterator i(&container->children); *i; i++)
  {
    if (!((*i)->controlState & STATE_SELECTED))
    {
      proc((*i)->Name(), (*i)->GetTextString(), context);
    }
  }
}


//
// ICListBox::GetSelectedItem
//
// Return the key of the selected item
//
Bool ICListBox::GetSelectedItem(const char * &s)
{
  selectedList.PurgeDead();

  if (selectedList.GetCount())
  {
    s = selectedList.GetFirst()->Name();
    return (TRUE);
  }
  else
  {
    s = NULL;
    return (FALSE);
  }
}


//
// ICListBox::GetSelectedItem
//
// Return the control of the selected item
//
IControl * ICListBox::GetSelectedItem()
{
  selectedList.PurgeDead();

  if (selectedList.GetCount())
  {
    return (selectedList.GetFirst());
  }
  else
  {
    return (NULL);
  }
}


//
// ICListBox::GetSelectedIndex
// 
// Returh the index of the selected item
//
U32 ICListBox::GetSelectedIndex()
{
  selectedList.PurgeDead();

  if (selectedList.GetCount())
  {
    U32 index = 1;
    for (NList<IControl>::Iterator i(&container->children); *i; i++)
    {
      if (*i == selectedList.GetFirst())
      {
        return (index);
      }
      index++;
    }
  }

  // No selection
  return (0);
}


//
// ICListBox::SetSelectedItem
//
// Set the selected item by key 
// (for multiple selection lists this adds the item to the selected list)
//
Bool ICListBox::SetSelectedItem(const char *name, Bool force, Bool notify)
{
  for (NList<IControl>::Iterator i(&container->children); *i; i++)
  {
    if (!Utils::Strcmp(name, (*i)->Name()))
    {
      Bool toggle = listBoxStyle & STYLE_MULTISELECT ? TRUE : FALSE;
      Select(*i, force ? FALSE : toggle, !toggle, notify);
      return (TRUE);
    }
  }

  return (FALSE);
}


//
// ICListBox::SetSelectedItem
//
// Set the selected item by index
// (for multiple selection lists this adds the item to the selected list)
//
Bool ICListBox::SetSelectedItem(U32 index, Bool force, Bool notify)
{
  IControl *ctrl = container->children[index - 1];

  if (ctrl)
  {
    Bool toggle = listBoxStyle & STYLE_MULTISELECT ? TRUE : FALSE;
    Select(ctrl, force ? FALSE : toggle, !toggle, notify);
    return (TRUE);
  }

  return (FALSE);
}


//
// ICListBox::SetSelectedItem
//
// Set the selected item by index
// (for multiple selection lists this adds the item to the selected list)
//
Bool ICListBox::SetSelectedItem(IControl *ctrl, Bool force, Bool notify)
{
  selectedList.PurgeDead();

  //if (selectedList.GetCount())
  {
    for (NList<IControl>::Iterator i(&container->children); *i; i++)
    {
      if (*i == ctrl)
      {
        Bool toggle = listBoxStyle & STYLE_MULTISELECT ? TRUE : FALSE;
        Select(*i, force ? FALSE : toggle, !toggle, notify);
        return (TRUE);
      }
    }
  }

  return (FALSE);
}


//
// ICListBox::AddItem
//
// Adds a control to the listbox
//
void ICListBox::AddItem(const char *id, IControl *ctrl, Bool configured)
{
  ctrl->SetName(id);

  // Add this item to the listbox
  ctrl->SetParent(container);
  ctrl->SetZPos(0xFFFFFFFF);
  ctrl->controlState &= ~STATE_SELECTED;

  // Apply any configuration
  if (!configured && itemConfig)
  {
    itemConfig->InitIterators();
    ctrl->Configure(itemConfig);
  }

  // If the list box is active also activate the item
  if (controlState & STATE_ACTIVE)
  {
    ctrl->Activate();

    // Reposition visible controls on next redraw
    ForceReposition();
  }

  // If this is the first item in the list, select it
  if ((container->children.GetCount() == 1) && !(listBoxStyle & (STYLE_MULTISELECT | STYLE_NOSELECTION | STYLE_CANCLEAR)))
  {
    Select(ctrl, FALSE, FALSE);
  }

  // If autoscroll is on, scroll to the newly added item
  if (listBoxStyle & STYLE_AUTOSCROLL)
  {
    desiredTop = S32_MAX;
  }

  // If smart scroll is on, scroll to the newly 
  // added item if we're at the bottom of the scroll
  if (listBoxStyle & STYLE_SMARTSCROLL && top + vis >= count)
  {
    desiredTop = S32_MAX;
  }

  // Update the count
  count = container->children.GetCount();
}


//
// Remove a control from the using the given name
//
void ICListBox::RemoveItem(const char *id)
{
  if (IControl *ctrl = IFace::Find<IControl>(id, container))
  {
    RemoveItem(ctrl);
  }
}


//
// Remove a control from the list (marks it for deletion)
//
void ICListBox::RemoveItem(IControl *ctrl)
{
  ASSERT(ctrl)
  ASSERT(ctrl->parent == container)

  // If this ctrl is selected
  if (selectedList.Exists(ctrl))
  {
    selectedList.Remove(ctrl);
    if (!(listBoxStyle & (STYLE_MULTISELECT | STYLE_CANCLEAR)))
    {
      // Is there a next control ?
      if (IControl *next = container->children.GetNext(ctrl))
      {
        Select(next, FALSE, FALSE);
      }
      else if (IControl *prev = container->children.GetPrev(ctrl))
      {
        Select(prev, FALSE, FALSE);
      }
      else
      {
        ClearSelected();
      }
    }
    else
    {
      // We should update the var or something here ?
    }
  }
  ctrl->MarkForDeletion();
  ForceReposition();
}


//
// ICListBox::AddTextItem
//
// Creates a static text control and adds it to the listbox
//
void ICListBox::AddTextItem(const char *id, const CH *display, const Color *textClr, const Point<S32> &indent)
{
  const CH *ptr = display ? display : Utils::Ansi2Unicode(id);

  do
  {
    // Create a new static
    ICStatic *ctrl = new ICStatic(container);

    // Apply default color group
    ctrl->SetColorGroup(IFace::data.cgListItem);
    ctrl->controlStyle &= ~STYLE_INERT;

    // Optional text color override
    if (textClr)
    {
      ctrl->OverrideTextColor(textClr);
    }

    // Apply any configuration
    if (itemConfig)
    {
      itemConfig->InitIterators();
      ctrl->Configure(itemConfig);
    }

    if (listBoxStyle & STYLE_WRAP)
    {
      ASSERT(paintInfo.client.Width() > 3)

      U32 length;
      const CH *next = IFace::BreakText(ptr, ctrl->GetPaintInfo().font, paintInfo.client.Width() - 3, length);
      ctrl->SetTextString(Utils::Strndup(ptr, length), FALSE, TRUE);
      ptr = next;
    }
    else
    {
      // Setup
      ctrl->SetTextString(ptr, TRUE);
      ptr = NULL;
    }

    ctrl->SetTextJustify(JUSTIFY_LEFT);

    if (listBoxStyle & STYLE_AUTOTIPTEXT)
    {
      // Could be dangerous, but just use the display string for tip text
      ctrl->SetTipText(ctrl->textStr, FALSE);
    }

    // Setup the indent
    ctrl->SetIndent(indent);

    // Add it the the end of the listbox
    AddItem(id, ctrl, TRUE);
  }
  while (ptr);
}


//
// ICListBox::DeleteAllItems
//
// Delete all items from the listbox
//
void ICListBox::DeleteAllItems()
{
  container->DisposeChildren();
  ClearSelected();

  // Reset display parameters
  UpdateRange(TRUE);
}


//
// ICListBox::GetAdjustmentRect
//
// Calculate an adjustment rect based on the style
//
ClipRect ICListBox::GetAdjustmentRect()
{
  ClipRect r(0, 0, 0, 0);

  if (skin == NULL)
  {
    // Adjust geometry to compensate for slider
    if (listBoxStyle & STYLE_VSLIDER)
    {
      r.Set(0, 0, -(IFace::GetMetric(IFace::SLIDER_WIDTH) + 0), 0);
    }
  }

  return (r + IControl::GetAdjustmentRect());
}


//
// ICListBox::PostConfigure
//
// Post configuration hook
//
void ICListBox::PostConfigure()
{
  // Post configure base class first
  IControl::PostConfigure();

  // Create slider if needed
  if (listBoxStyle & STYLE_VSLIDER)
  {
    ICListSlider *sliderCtrl = NULL;

    if (sliderCfg)
    {
      // Create a custom slider
      IControl *ctrl = IFace::CreateControl(SliderCtrlName, sliderCfg, this);
      if (ctrl)
      {
        sliderCtrl = IFace::Promote<ICListSlider>(ctrl);
      }
      if (sliderCtrl == NULL)
      {
        ERR_FATAL(("Error creating list slider for [%s]", Name()))
      }
    }
    else
    {
      // Create generic slider
      sliderCtrl = new ICListSlider(this);
      sliderCtrl->SetName(SliderCtrlName);
      sliderCtrl->SetGeometry("WinParentHeight", "WinRight", "WinTop", NULL);
      sliderCtrl->SetSize(IFace::GetMetric(IFace::SLIDER_WIDTH), 0);
      sliderCtrl->SetOrientation("Vertical");
    }

    sliderCtrl->AddListBox(this);
  }

  // Attach slider if one is specified
  if (slider)
  {
    ICListSlider *sliderCtrl = IFace::Find<ICListSlider>(slider, this);

    if (sliderCtrl == NULL)
    {
      ERR_FATAL(("Slider [%s] not found", slider));
    }

    sliderCtrl->AddListBox(this);
  }
}


//
// ICListBox::Activate
//
// Activate the control
//
Bool ICListBox::Activate()
{
  if (IControl::Activate())
  {
    LOG_LIST(("[%s] activate", Name()))

    if (!(listBoxStyle & (STYLE_NOAUTOREBUILD)))
    {
      // Generate a list rebuild message
      SendNotify(this, ICListBoxMsg::Rebuild, FALSE);
    }

    // Activate vars
    ActivateVar(topVar);

    // Validate selectedVar
    if (!(listBoxStyle & (STYLE_MULTISELECT | STYLE_NOSELECTION)))
    {
      ActivateVar(selectedVar);
    }

    // Validete indexVar
    if (!(listBoxStyle & (STYLE_MULTISELECT | STYLE_NOSELECTION)) && indexVar)
    {
      ActivateVar(indexVar);
    }

    // Purge dead things from selected list
    selectedList.PurgeDead();

    // Reposition controls
    SetupCellSize();
    UpdateRange(FALSE);

    // Are we right and the var is wrong ?
    if (updateVar)
    {
      if (selectedVar && selectedVar->IsActive())
      {
        const char *val;
        GetSelectedItem(val);
        if (val)
        {
          selectedVar->SetValue(val ? val : "");
        }
      }

      if (indexVar && indexVar->IsActive())
      {
        indexVar->SetIntegerValue(GetSelectedIndex());
      }

      updateVar = FALSE;
    }
    else
    {
      // If there's an index var set the selected item from the index
      if (indexVar && indexVar->IsActive())
      {
        SetSelectedItem(indexVar->GetIntegerValue());
      }

      // If there's a selection var set the selected item from the string
      if (selectedVar && selectedVar->IsActive())
      {
        SetSelectedItem(selectedVar->GetStringValue());
      }
    }

    // If autoscroll is on, scroll to the newly added item
    if (listBoxStyle & STYLE_AUTOSCROLL)
    {
      desiredTop = S32_MAX;
    }

    // If smart scroll is on, scroll to the newly 
    // added item if we're at the bottom of the scroll
    if (listBoxStyle & STYLE_SMARTSCROLL && top + vis >= count)
    {
      desiredTop = S32_MAX;
    }

    // Force reposition on next redraw
    ForceReposition();

    LOG_LIST(("[%s] Activated, #selected=%d, index=%d, sel='%s'", Name(), selectedList.GetCount(), indexVar ? indexVar->GetIntegerValue() : -1, selectedVar ? selectedVar->GetStringValue() : ""))

    return (TRUE);
  }
  else
  {
    return (FALSE);
  }
}


//
// ICListBox::Deactivate
//
// Deactivate the listbox
//
Bool ICListBox::Deactivate()
{
  if (IControl::Deactivate())
  {
    // Reset state
    vis = 0;
    count = 0;

    // Deactivate vars
    topVar->Deactivate();

    return (TRUE);
  }
  else
  {
    return (FALSE);
  }
}


//
// ICListBox::Setup
//
// Configure the control from an fscope
//
void ICListBox::Setup(FScope *fScope)
{
  switch (fScope->NameCrc())
  {
    case 0x742EA048: // "UseVar"
    {
      ConfigureVar(selectedVar, fScope);
      break;
    }

    case 0xF379ABC4: // "IndexVar"
    {
      ConfigureVar(indexVar, fScope);
      break;
    }

    case 0x8DFC9692: // "UseSlider"
    {
      // Ensure slider has not already been setup
      if (slider)
      {
        delete slider;
      }

      // Dup the slider string
      slider = Utils::Strdup(fScope->NextArgString());

      break;
    }

    case 0xF4E44C64: // "ItemConfig"
    {
      // If we don't already have a config
      if (!itemConfig)
      {
        // Copy this one
        itemConfig = fScope->Dup();
      }
      break;
    }

    case 0x83AB76D4: // "SliderConfig"
    {
      sliderCfg = IFace::FindRegData(fScope->NextArgString());
      break;
    }

    case 0x52AC1340: // "CellPadding"
    {
      cellPad = fScope->NextArgInteger();
      break;
    }

    case 0x2E5E6E1D: // "AddTextItem"
    {
      const char *id  = fScope->NextArgString();
      const CH *str = NULL;

      // Optional text string
      VNode *vNode = fScope->NextArgument(VNode::AT_STRING, FALSE);
      if (vNode)
      {
        str = TRANSLATE((vNode->GetString()));
      }

      // Optional text color override
      FScope *sScope;

      if ((sScope = fScope->GetFunction("Color", FALSE)) != NULL)
      {
        Color c;
        
        StdLoad::TypeColor(sScope, c);
        AddTextItem(id, str, &c);
      }
      else
      {
        AddTextItem(id, str);
      }
      return;
    }

    case 0x9F1D54D0: // "Add"
    {
      IControl *ctrl = IFace::CreateControl(fScope, container);

      if (ctrl == NULL)
      {
        LOG_ERR(("Error creating control [%s]", ctrl->Name()));
      }
      else
      {
        AddItem(ctrl->Name(), ctrl);
      }
      break;
    }

    case 0x1237D9DA: // "SetSelected"
    {
      VNode *vNode = fScope->NextArgument();

      switch (vNode->aType)
      {
        case VNode::AT_STRING:
        {
          SetSelectedItem(vNode->GetString());
          break;
        }

        case VNode::AT_INTEGER:
        {
          IControl *ctrl = container->children[vNode->GetInteger()];

          if (ctrl)
          {
            Select(ctrl, FALSE, TRUE);
          }
          break;
        }

        default:
          fScope->ScopeError("SetSelected: Expected a String or an Integer");
          break;
      }
      break;
    }
    
    default:
    {
      // Pass it to the previous level in the hierarchy
      IControl::Setup(fScope);
      break;
    }
  }
}


//
// ICListBox::Notify
//
// Handle notifications
//
void ICListBox::Notify(IFaceVar *var)
{
  if (IsActive())
  {
    if (indexVar && indexVar == var)
    {
      SetSelectedItem(indexVar->GetIntegerValue());
    }
    else
    if (selectedVar && selectedVar == var)
    {
      SetSelectedItem(selectedVar->GetStringValue());
    }
    else
    if (topVar == var)
    {
      ForceReposition();
    }
  }
  else
  {
    IControl::Notify(var);
  }
}


//
// ICListBox::HandleEvent
//
// Event handler
//
U32 ICListBox::HandleEvent(Event &e)
{
  // Input events
  if (e.type == Input::EventID())
  {
    // Client coordinates
    Point<S32> p = container->ScreenToClient(Point<S32>(e.input.mouseX, e.input.mouseY));
    IControl *child = container->Find(p.x, p.y, TRUE);

    // Filter out references to self
    if (child == container)
    {
      child = NULL;
    }

    // If this control isn't in the container, try to go up until we hit the container
    while (child && child->parent != container)
    {
      child = child->parent;
    }

    // Otherwise process it internally
    switch (e.subType)
    {
      case Input::MOUSEBUTTONDOWN:
      {
        if (e.input.code == Input::LeftButtonCode())
        {
          // If left button is pressed on a child select it
          if (child)
          {
            Bool toggle = listBoxStyle & STYLE_MULTISELECT 
                          ? TRUE
                          : FALSE; 

            // Play click sound
            IFace::Sound::Play(soundClick, this);

            if (!(listBoxStyle & STYLE_NOSELECTIONINPUT))
            {
              // Select the item
              Select(child, toggle, !toggle);
            }
          }

          // Handled
          return (TRUE);
        }
        else 
          
        if (e.input.code == Input::RightButtonCode())
        {
          if (!(listBoxStyle & STYLE_NOSELECTIONINPUT))
          {
            // Right button clears list
            if (listBoxStyle & STYLE_CANCLEAR)
            {
              ClearSelected();
            }
          }

          // Handled
          return (TRUE);
        }

        // Not handled
        break;
      }

      case Input::MOUSEBUTTONDBLCLK:
      {
        if (e.input.code == Input::LeftButtonCode())
        {
          // If left button is double clicked on a child then select it
          if (child)
          {
            if (!(listBoxStyle & STYLE_NOSELECTIONINPUT))
            {
              // and notify the parent of a double click event
              if (Select(child))
              {
                if (!(listBoxStyle & STYLE_MULTISELECT))
                {
                  // Play click sound
                  IFace::Sound::Play(soundClick, this);

                  // Generate a double click notification
                  SendNotify(this, ICListBoxNotify::DoubleClick);
                }

                // Handled
                return (TRUE);
              }
            }
          }
        }

        // Not handled
        break;
      }

      case Input::MOUSEAXIS:
      {
        S16 amount = S16(e.input.ch / -120);

        // Generate scroll event
        SendNotify(this, ICListBoxMsg::ScrollLine, FALSE, amount);

        // Handled
        return (TRUE);
      }

      case Input::KEYDOWN:
      {
        switch (e.input.code)
        {
          case DIK_RETURN:
          {
            if (!(listBoxStyle & STYLE_NOSELECTIONINPUT))
            {
              // and notify the parent of a double click event
              if (!(listBoxStyle & STYLE_MULTISELECT))
              {
                if (selectedList.GetCount() == 1)
                {
                  // Play click sound
                  IFace::Sound::Play(soundClick, this);

                  // Generate a double click notification
                  SendNotify(this, ICListBoxNotify::DoubleClick);
                }

                // Handled
                return (TRUE);
              }
            }

            // Handled
            return (TRUE);
          }
        }

        // Not handled
        break;
      }
    }
  }
  else

  if (e.type == IFace::EventID())
  {
    switch (e.subType)
    {
      case IFace::NOTIFY:
      {
        // Do specific handling
        switch (e.iface.p1)
        {
          case ICListBoxMsg::ClearSelection:
          {
            if (listBoxStyle & STYLE_CANCLEAR)
            {
              ClearSelected();
            }

            // Handled
            return (TRUE);
          }

          case ICListBoxMsg::DeleteAll:
          {
            // Delete all items in the list
            DeleteAllItems();

            // Handled
            return (TRUE);
          }

          case ICListBoxMsg::SetSelected:
          {
            // Set a selected item
            for (NList<IControl>::Iterator i(&container->children); *i; i++)
            {
              if (e.iface.p2 == (*i)->NameCrc())
              {
                Bool toggle = listBoxStyle & STYLE_MULTISELECT ? TRUE : FALSE;
                Select(*i, toggle, !toggle);
                break;
              }
            }

            // Handled
            return (TRUE);
          }

          case ICListBoxMsg::ScrollLine:
          {
            // Scroll by n lines
            S32 n = S32(e.iface.p2);
            top = top + n;

            // Handled
            return (TRUE);
          }

          case ICListBoxMsg::ScrollPage:
          {
            // Scroll by n pages
            if (vis > 1)
            {
              S32 n = S32(e.iface.p2) * vis;
              top = top + ((n > 1) ? n-1 : ((n < 1) ? n+1 : n));
            }

            // Handled
            return (TRUE);
          }

          case ICListBoxMsg::MoveUp:
          {
            // Take the selected item and move it up one place
            IControl *selected = GetSelectedItem();
            if (selected)
            {
              IControl *prev = container->children.GetPrev(selected);

              if (prev)
              {
                container->children.Unlink(selected);
                container->children.InsertBefore(&container->children.GetNode(*prev), selected);

                SetupCellSize();
                UpdateRange(FALSE);
              }
            }

            // Handled
            return (TRUE);
          }

          case ICListBoxMsg::MoveDown:
          {
            // Take the selected item and move it down one place
            IControl *selected = GetSelectedItem();
            if (selected)
            {
              IControl *next = container->children.GetNext(selected);

              if (next)
              {
                container->children.Unlink(selected);
                container->children.InsertAfter(&container->children.GetNode(*next), selected);

                UpdateRange(FALSE);
                SetupCellSize();
              }
            }

            // Handled
            return (TRUE);
          }

          default:
          {
            // Not handled
            break;
          }
        }

        // Not handled
        break;
      }

      // Check if we are hooking into this message
      case IFace::HOOKCHECK:
      {
        switch (e.iface.p1)
        {
          case Input::MOUSEBUTTONDOWN:
          case Input::MOUSEBUTTONDBLCLK:
          case Input::MOUSEAXIS:
            return (TRUE);

          default:
            return (FALSE);
        }
      }
    }
  }

  // Allow IControl class to process this event
  return IControl::HandleEvent(e);
}


//
// ICListBox::SetStyleItem
//
// Change a style setting
//
Bool ICListBox::SetStyleItem(const char *s, Bool toggle)
{
  U32 style;

  switch (Crc::CalcStr(s))
  {
    case 0x6F942D71: // "VSlider"
      style = STYLE_VSLIDER;
      break;

    case 0x7348AC3A: // "NoSelection"
      style = STYLE_NOSELECTION;
      break;

    case 0x74DA85D3: // "MultiSelect"
      style = STYLE_MULTISELECT;
      break;

    case 0xC216D0BE: // "CanClear"
      style = STYLE_CANCLEAR;
      break;

    case 0x6F8E6C70: // "AutoScroll"
      style = STYLE_AUTOSCROLL;
      break;

    case 0x00079F36: // "SmartScroll"
      style = STYLE_SMARTSCROLL;
      break;

    case 0xE8DA81E7: // "Wrap"
      style = STYLE_WRAP;
      break;

    case 0xF94FEE72: // "AutoTipText"
      style = STYLE_AUTOTIPTEXT;
      break;

    case 0xF5037B67: // "NoAutoRebuild"
      style = STYLE_NOAUTOREBUILD;
      break;

    case 0x43E9E615: // "NoSelectionInput"
      style = STYLE_NOSELECTIONINPUT;
      break;

    default:
      return (IControl::SetStyleItem(s, toggle));
  }

  // Toggle the style
  listBoxStyle = (toggle) ? (listBoxStyle | style) : (listBoxStyle & ~style);

  return (TRUE);
}



//
// ICListBox::FindListBox
//
ICListBox *ICListBox::FindListBox(const char *path)
{
  IControl *ctrl = IFace::FindByName(path);

  // This currently does not check for correct type
  if (ctrl && ctrl->DerivedFrom(ICListBox::ClassId()))
  {
    return ((ICListBox *)ctrl);
  }

  return (NULL);
}


//
// ICListBox::FillFromPath
//
// fill a list box with files matching 'filter' from the folder 'path'
// returns the number of files added
//
U32 ICListBox::FillFromPath( const char *path, const char *filter, Bool doClear, Bool doSubDirs) // = "*.*", = TRUE, = FALSE
{
  Dir::Find find;

  if (doClear)
  {
    // Clear the list
    DeleteAllItems();
  }

  U32 n = 0;

  // Add each file
  if (Dir::FindFirst(find, path, filter))
  {
    do
    {
      // Don't add directories
      if (*find.finddata.name != '.')
      {
        AddTextItem(find.finddata.name, NULL);
      }
    } 
    while (Dir::FindNext(find));
  }  

  // Finish find operation
  Dir::FindClose(find);

  if (doSubDirs && Dir::FindFirst(find, path, "*."))
  {
    do
    {
      U32 len = strlen( find.finddata.name);
      if (!((len == 1 || len == 2) && *find.finddata.name == '.'))
      {
        char buffer[MAX_BUFFLEN + 1];
        Utils::MakePath( buffer, MAX_BUFFLEN, path, "\\", find.finddata.name);

        FillFromPath( buffer, filter, FALSE, TRUE);
      }
    } 
    while (Dir::FindNext(find));

    // Finish find operation
    Dir::FindClose(find);
  }  

  return n;
}


///////////////////////////////////////////////////////////////////////////////
//
// Class ICListBoxSpacer
//


//
// Redraw self
//
void ICListBoxSpacer::DrawSelf(PaintInfo &pi)
{
  ClipRect rect(pi.client);
  rect.p0.x += 2;
  rect.p0.y = (pi.client.p1.y + pi.client.p0.y) / 2;
  rect.p1.x -= 2;
  rect.p1.y = rect.p0.y + 1;

  // Draw a filled rectangle
  IFace::RenderRectangle(rect, pi.colors->fg[ColorIndex()]);
}
