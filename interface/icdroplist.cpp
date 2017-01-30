/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icdroplist.h"
#include "iclistbox.h"
#include "stdload.h"
#include "babel.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class 
//
class ICDropList::Container : public ICWindow
{
private:

  // Drop list associated with this container
  ICDropListPtr dropList;

public:

  // Is the drop list in the process of being activated
  Bool activating;

public:

  // Constructor
  Container(IControl *parent, ICDropList *list)
  : ICWindow(parent),
    dropList(list),
    activating(FALSE)
  {
    // Default style
    controlStyle |= STYLE_MODAL | STYLE_MODALCLOSE;
    controlStyle &= ~STYLE_DROPSHADOW;
  }

  // Activate
  Bool Activate()
  {
    activating = TRUE;
    if (ICWindow::Activate())
    {
      activating = FALSE;
      return (TRUE);
    }
    activating = FALSE;
    return (FALSE);
  }

  // Deactivate
  Bool Deactivate()
  {
    if (ICWindow::Deactivate())
    {
      if (dropList.Alive())
      {
        dropList->state = CLOSING;
      }
      return (TRUE);
    }
    return (FALSE);
  }
};


///////////////////////////////////////////////////////////////////////////////
//
// Class ICDropList
//

// Name of code generated list box
const char *ListBoxCtrlName = "[ListBox]";

// Name of code generated window for listbox
const char *ContainerCtrlName = "[Drop%08X:%s]";

// Name of code generated current button
const char *CurrentCtrlName = "";

// Name of code generated drop button
const char *DropBtnCtrlName = "[DropButton]";


//
// ICDropList::ICDropList
//
ICDropList::ICDropList(IControl *parent)
: IControl(parent),
  selectedVar(NULL), 
  selectedVarName(NULL),
  state(CLOSED),
  height(50),
  dropListStyle(0),
  containerCfg(NULL),
  listBoxCfg(NULL),
  currentCfg(NULL),
  dropButtonCfg(NULL)
{
  controlStyle |= TRANSPARENT;
}


//
// ICDropList::~ICDropList
//
ICDropList::~ICDropList()
{
  // Dispose of var
  if (selectedVar)
  {
    delete selectedVar;
    selectedVar = NULL;
  }

  // Dispose of var name
  if (selectedVarName)
  {
    delete selectedVarName;
    selectedVarName = NULL;
  }

  if (containerCfg)
  {
    delete containerCfg;
    containerCfg = NULL;
  }
  if (listBoxCfg)
  {
    delete listBoxCfg;
    listBoxCfg = NULL;
  }
  if (currentCfg)
  {
    delete currentCfg;
    currentCfg = NULL;
  }
  if (dropButtonCfg)
  {
    delete dropButtonCfg;
    dropButtonCfg = NULL;
  }

}


//
// ICDropList::Notify
//
// Var changed
//
void ICDropList::Notify(IFaceVar *var)
{
  if (var == selectedVar)
  {
    ICListBox *listBox;
    IControl *ctrl;

    // Set the text on the button
    if (current.Alive() && 
        (listBox = GetListBox()) != NULL)
    {
      // If the list box isn't active, set the value of the var
      if (!listBox->IsActive())
      {
        listBox->SetSelectedItem(var->GetStringValue());
      }

      if ((ctrl = listBox->GetSelectedItem()) != NULL)
      {
        current->SetTextString(ctrl->GetTextString(), TRUE);

        // Force the text color of the button to the same as the list item
        if (dropListStyle & STYLE_ITEMCOLOR)
        {
          current->OverrideTextColor(ctrl->GetOverrideTextColor());
        }
      }

      SendNotify(this, ICDropListNotify::ChangeSelection);
    }
  }
  else
  {
    IControl::Notify(var);
  }
}


//
// ICDropList::PostConfigure
//
// Post configuration hook
//
void ICDropList::PostConfigure()
{
  // Post configure base class first
  IControl::PostConfigure();

  // Check that var has been setup
  if (!selectedVar || !selectedVarName)
  {
    ERR_FATAL(("DropList var has not been configured for [%s]", Name()))
  }

  // Create container
  char name[256];
  Utils::Sprintf(name, sizeof (name), ContainerCtrlName, this, Name());

  container = new Container(IFace::RootWindow(), this);
  container->SetName(name);
  container->SetAlignTo(this);
  container->SetSize(0, height);

  if (containerCfg)
  {
    // Apply custom style
    containerCfg->InitIterators();
    container->Configure(containerCfg);
  }
  else
  {
    // Apply generic style
    container->SetGeometry("Left", "Bottom", "VExternal", "HInternal", "AlignToWidth", NULL);
  }

  // Create list box
  if (listBoxCfg)
  {
    // Create custom listbox
    IControl *ctrl = IFace::CreateControl(ListBoxCtrlName, listBoxCfg, container);
    listBox = IFace::Promote<ICListBox>(ctrl);
    if (!listBox.Alive())
    {
      ERR_FATAL(("Error creating Listbox for [%s]", Name()))
    }
  }
  else
  {
    // Create generic listbox
    listBox = new ICListBox(container);
    listBox->SetName(ListBoxCtrlName);
  }

  // Default style
  listBox->SetGeometry("ParentWidth", "ParentHeight", NULL);
  listBox->SetStyle("NoAutoRebuild", NULL);

  // Have the list box use the same var as this item
  listBox->SetSelectedItemVar(selectedVarName);

  // Add an event translation to notify this when selection is changed
  listBox->AddEventTranslation(ICListBoxNotify::ChangeSelection, ICDropListMsg::Selected, this);

  // Create current button
  if (currentCfg)
  {
    // Create custom control
    IControl *ctrl = IFace::CreateControl(CurrentCtrlName, currentCfg, this);
    current = IFace::Promote<ICButton>(ctrl);
    if (!current.Alive())
    {
      ERR_FATAL(("Error creating button for [%s]", Name()))
    }
  }
  else
  {
    // Create generic button
    current = new ICButton(this);
    current->SetName(CurrentCtrlName);
    current->SetGeometry("ParentWidth", "ParentHeight", NULL);
    current->SetSize(-IFace::GetMetric(IFace::SLIDER_WIDTH), 0);
    current->SetStyle("!VGradient", NULL);
    current->SetColorGroup(IFace::data.cgClient);
    current->SetTextFont("System");
    current->SetTextJustify(JUSTIFY_LEFT);
  }

  // Convert button presses to droplist
  current->AddEventTranslation(ICButtonNotify::Pressing, ICDropListMsg::Drop, this);

  // Create drop down button
  if (dropButtonCfg)
  {
    // Create custom control
    IControl *ctrl = IFace::CreateControl(DropBtnCtrlName, dropButtonCfg, this);
    if (ctrl == NULL)
    {
      ERR_FATAL(("Error creating drop button for [%s]", Name()))
    }
  }
  else
  {
    // Create generic control
    IControl *ctrl = new ICSystemButton(ICSystemButton::DROPLIST, this);
    ctrl->SetName(DropBtnCtrlName);
    ctrl->SetGeometry("ParentHeight", "Right", NULL);
    ctrl->SetSize(IFace::GetMetric(IFace::SLIDER_WIDTH), 0);
  }
}


//
// ICDropList::SetStyleItem
//
// Change a style setting
//
Bool ICDropList::SetStyleItem(const char *s, Bool toggle)
{
  U32 style;

  switch (Crc::CalcStr(s))
  {
    case 0xD766346C: // "UseItemColor"
      style = STYLE_ITEMCOLOR;
      break;

    case 0x09DA21A5: // "SafePulldown"
      style = STYLE_SAFEPULLDOWN;
      break;

    case 0x5F4EA42D: // "GoUp"
      style = STYLE_GO_UP;
      break;

    default:
      return (IControl::SetStyleItem(s, toggle));
  }

  // Toggle the style
  dropListStyle = (toggle) ? (dropListStyle | style) : (dropListStyle & ~style);

  return (TRUE);
}


//
// ICDropList::DrawSelf
//
// Draw the droplist control
//
void ICDropList::DrawSelf(PaintInfo &pi)
{
  if (state == CLOSING)
  {
    state = CLOSED;
  }

  // Fill the background
  DrawCtrlBackground(pi, GetTexture());

  // Draw the frame
  DrawCtrlFrame(pi);
}


//
// ICDropList::Activate
//
Bool ICDropList::Activate()
{
  if (IControl::Activate())
  {
    ActivateVar(selectedVar);

    // Make sure that the list box is alive
    if (!container.Alive())
    {
      ERR_FATAL(("ListBox is not alive"))
    }

    // Tell the list box to rebuild at this point
    SendNotify(listBox, ICListBoxMsg::Rebuild, FALSE);
    
    if (!(dropListStyle & STYLE_SAFEPULLDOWN))
    {
      container->Activate();
      container->Deactivate();
    }

    return (TRUE);
  }

  return (FALSE);
}


//
// ICDropList::Deactivate
//
Bool ICDropList::Deactivate()
{
  if (IControl::Deactivate())
  {
    selectedVar->Deactivate();
    return (TRUE);
  }

  return (FALSE);
}


//
// ICDropList::Setup
//
// Configure this control with an FScope
//
void ICDropList::Setup(FScope *fScope)
{
  switch (fScope->NameCrc())
  {
    case 0x742EA048: // "UseVar"
    {
      selectedVarName = Utils::Strdup(FindVarName(fScope->NextArgString()));
      selectedVar = new IFaceVar(this, FindVarName(selectedVarName));
      break;
    }

    case 0x86010476: // "Height"
    {
      height = StdLoad::TypeU32(fScope);
      break;
    }

    case 0x6556609C: // "Container"
    {
      if (containerCfg)
      {
        delete containerCfg;
      }
      containerCfg = fScope->Dup();
      break;
    }

    case 0xAB7D6BE2: // "ListBox"
    {
      if (listBoxCfg)
      {
        delete listBoxCfg;
      }
      listBoxCfg = fScope->Dup();
      break;
    }

    case 0x587C9FAF: // "Current"
    {
      if (currentCfg)
      {
        delete currentCfg;
      }
      currentCfg = fScope->Dup();
      break;
    }

    case 0xED325082: // "DropButton"
    {
      if (dropButtonCfg)
      {
        delete dropButtonCfg;
      }
      dropButtonCfg = fScope->Dup();
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
// ICDropList::HandleEvent
//
// Process input events
//
U32 ICDropList::HandleEvent(Event &e)
{
  if (e.type == IFace::EventID())
  {
    switch (e.subType)
    {
      case IFace::NOTIFY:
      {
        // Do specific handling
        switch (e.iface.p1)
        {
          case ICDropListMsg::ClearSelected:
          {
            selectedVar->SetStringValue("");

            ICListBox *listBox;

            // Set the text on the button
            if ((listBox = GetListBox()) != NULL)
            {
              listBox->ClearSelected(FALSE);
            }
            
            // Handled
            return (TRUE);
          }

          case ICDropListMsg::Selected:
            if (!container->activating)
            {
              container->Deactivate();
              state = CLOSED;
            }

            // Handled
            return (TRUE);

          case ICDropListMsg::Drop:
            switch (state)
            {
              case OPEN:
                container->Deactivate();
                state = CLOSED;
                break;

              case CLOSED:
                container->SetZPos(0);
                container->Activate();
                state = OPEN;
                break;

              case CLOSING:
                state = CLOSED;
                break;
            }

            // Handled
            return (TRUE);
        }
      }
    }
  }

  // Allow IControl class to process this event
  return (IControl::HandleEvent(e));
}


//
// Set the selected item in the drop list
//
void ICDropList::SetSelected(const char *item)
{
  if (selectedVar)
  {
    selectedVar->SetStringValue(item);
  }
}


//
// ICDropList::GetListBox
//
// GetListBox
//
ICListBox * ICDropList::GetListBox()
{
  if (container.Alive())
  {
    return (IFace::Find<ICListBox>(ListBoxCtrlName, container));
  }
  else
  {
    return (NULL);
  }
}
