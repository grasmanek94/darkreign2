/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Menu Control
//
// 11-NOV-1998
//

#include "icmenu.h"
#include "iface.h"
#include "iface_types.h"
#include "iface_util.h"
#include "input.h"
#include "stdload.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class MenuButton - Cascading sub menu button
//
class MenuButton : public ICButton
{
  PROMOTE_LINK(MenuButton, ICButton, 0x988DFBA8); // "MenuButton"

public:

  // Constructor
  MenuButton(IControl *parent);

  // Destructor
  ~MenuButton();

  // Draw control
  void DrawSelf(PaintInfo &pi);

};


///////////////////////////////////////////////////////////////////////////////
//
// Class SubMenuButton - Cascading sub menu button
//
class SubMenuButton : public MenuButton
{
  PROMOTE_LINK(SubMenuButton, ICButton, 0xD2C9588D) // "SubMenuButton"

public:

  // Sub menu class id
  GameIdent menuClass;

  // Sub menu
  IControlPtr subMenu;

public:

  // Constructor
  SubMenuButton(IControl *parent) : MenuButton(parent), menuClass("") {}

  // Configure
  void Setup(FScope *fScope);

  // Deactivate
  Bool Deactivate();

  // Create the submenu
  void CreateSubMenu();

  // Event handler
  U32 HandleEvent(Event &e);

  // Draw control
  void DrawSelf(PaintInfo &pi);

};


///////////////////////////////////////////////////////////////////////////////
//
// Class ICMenu - Container for menu items
//


// Data store for the init proc
BinTree<void> *ICMenu::initAttrib;
ICMenu * ICMenu::initParentMenu;


//
// Constructor
//
ICMenu::ICMenu(IControl *parent) 
: IControl(parent),
  attrib(NULL)
{
  // Clear data
  menuStyle = 0;
  menuEdge = 3;
  itemConfig = NULL;
  callBack = NULL;
  context = NULL;
  geom.flags |= GEOM_KEEPVISIBLE;
  controlStyle |= STYLE_DROPSHADOW;
}


//
// Destructor
//
ICMenu::~ICMenu()
{
  if (itemConfig)
  {
    delete itemConfig;
  }

  if (attrib && !parentMenu.Alive())
  {
    attrib->DisposeAll();
    delete attrib;
  }
}


//
// DrawSelf
//
// Draw this control
//
void ICMenu::DrawSelf(PaintInfo &pi)
{
  DrawCtrlBackground(pi, GetTexture());
  DrawCtrlFrame(pi);
}


//
// SetStyleItem
//
// Change a style setting
//
Bool ICMenu::SetStyleItem(const char *s, Bool toggle)
{
  U32 style;

  switch (Crc::CalcStr(s))
  {
    case 0x2942B3CD: // "Horizontal"
      style = STYLE_HORIZONTAL;
      break;

    case 0x3897458F: // "NoAutoSize"
      style = STYLE_NOAUTOSIZE;
      break;

    default:
      return IControl::SetStyleItem(s, toggle);
  }

  // Toggle the style
  menuStyle = (toggle) ? (menuStyle | style) : (menuStyle & ~style);

  return TRUE;
}


//
// Setup
//
// Configure control from an FScope
//
void ICMenu::Setup(FScope *fScope)
{
  switch (fScope->NameCrc())
  {
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

    case 0xBE279E99: // "AddItem"
    {

      // The first parameter is the identifier
      GameIdent ident = StdLoad::TypeString(fScope);

      // The first optional param is the visible param
      GameIdent visible = StdLoad::TypeStringD(fScope, "");

      // The second optional param is the enabled param
      GameIdent enabled = StdLoad::TypeStringD(fScope, "");

      // If a visible param was supplied, make sure that the attrib is set
      if (attrib && !visible.Null() && !attrib->Exists(visible.crc))
      {
        // Failed, don't add it
        break;
      }

      // Create a new item
      IControl *ctrl = new MenuButton(this);
      ctrl->SetName(ident.str);

      // Apply any configuration
      if (itemConfig)
      {
        itemConfig->InitIterators();
        ctrl->Configure(itemConfig);
      }

      // If the enable param was supplied, make sure that the attrib is set
      if (attrib && !enabled.Null() && !attrib->Exists(enabled.crc))
      {
        ctrl->SetEnabled(FALSE);
      }

      // Apply configuration
      ctrl->Configure(fScope);
      break;
    }

    case 0x461BC0C0: // "AddSubMenu"
    {
      // Create a new item
      SubMenuButton *ctrl = new SubMenuButton(this);

      // Setup name
      ctrl->SetName(StdLoad::TypeString(fScope));

      // Apply any configuration
      if (itemConfig)
      {
        itemConfig->InitIterators();
        ctrl->Configure(itemConfig);
      }

      // Apply configuration
      ctrl->Configure(fScope);
      break;
    }

    case 0x50E0BF04: // "MenuEdge"
    {
      menuEdge = StdLoad::TypeU32(fScope);
      break;
    }

    // Pass it to the previous level in the hierarchy
    default:
      IControl::Setup(fScope);
  }
}


//
// Setup
//
// Configure control from the code
//
void ICMenu::Setup(EventCallBack *func, void *cText)
{
  ASSERT(func);

  // Save callback and context
  callBack = func;
  context = cText;

  // Setup fixed styles for code-generated menus
  SetStyle("ModalClose", NULL);
  SetGeometry("KeepVisible", NULL);
}

  
//
// AddItem
//
// Used to add a menu item from within the code
//
IControl * ICMenu::AddItem(const char *name, const CH *text, const char *event, Bool enabled)
{
  // Create a new item
  IControl *ctrl = IFace::CreateControl(name, "Button", this);

  // Set it up
  ctrl->SetTextFont("System");
  ctrl->SetStyle("TabStop", NULL);
  ctrl->SetEnabled(enabled);
  ctrl->SetTextString(text, TRUE);

  // Add some spacing
  ctrl->SetGeomSize(8, 6);

  if (event)
  {
    // Setup notification
    ctrl->AddEventTranslation(ICButtonNotify::Pressed, Crc::CalcStr(event), this);
  }

  return (ctrl);
}


//
// Activate
//
// Configure menu using current items, then activate
// 
Bool ICMenu::Activate()
{
  // Only adjust if not already active
  if (CanActivate())
  {
    NList<IControl>::Iterator i(&children);
    S32 offset = menuEdge, xMax = 0, yMax = 0;

    // Step though all children
    for (i.GoToTail(); *i; i--)
    {
      // Is auto-size turned off
      if (menuStyle & STYLE_NOAUTOSIZE)
      {
        (*i)->AdjustGeometry();
      }
      else
      {
        // Turn on autosizing
        (*i)->SetGeometryItem("AutoSize", TRUE);

        // Tell control to autosize itself
        (*i)->AdjustGeometry();

        // Turn off autosizing
        (*i)->SetGeometryItem("AutoSize", FALSE);  
      }

      // Update maximums
      xMax = Max<S32>(xMax, (*i)->GetSize().x);
      yMax = Max<S32>(yMax, (*i)->GetSize().y);
    }

    // If we don't have dimension, don't activate
    if (!xMax || !yMax)
    {
      return (FALSE);
    }

    // Adjust position of each child
    for (i.GoToTail(); *i; i--)
    {
      // Set the final size and position
      if (menuStyle & STYLE_HORIZONTAL)
      {
        (*i)->SetSize((*i)->GetSize().x, yMax);
        (*i)->SetPos(offset, menuEdge);
        offset += (*i)->GetSize().x;
      }
      else
      {
        (*i)->SetSize(xMax, (*i)->GetSize().y);
        (*i)->SetPos(menuEdge, offset);
        offset += (*i)->GetSize().y;
      }
    }

    // Update the size of the menu
    if (menuStyle & STYLE_HORIZONTAL)
    {
      SetSize(offset + menuEdge, yMax + (menuEdge * 2));
    }
    else
    {
      SetSize(xMax + (menuEdge * 2), offset + menuEdge);
    }
  }

  return (IControl::Activate());
}


//
// Deactivate
//
// This menu is being deactivated
//
Bool ICMenu::Deactivate()
{
  if (IControl::Deactivate())
  {          
    // Delete menus that are generated in the code
    if (callBack)
    {
      MarkForDeletion();
    }

    return (TRUE);
  }
  return (FALSE);
}


//
// HandleEvent
//
// Event handler
//
U32 ICMenu::HandleEvent(Event &e)
{
  if (e.type == IFace::EventID())
  {
    switch (e.subType)
    {
      case IFace::NOTIFY:
      {
        // If no callback, or callback didn't handle this event
        if (!callBack || !callBack(context, e.iface.p1, e.iface.p2, e.iface.p3)) 
        {
          // Allow IControl class to process this event
          IControl::HandleEvent(e);
        }

        // Do specific handling
        switch (e.iface.p1)
        {
          case 0x3EBABB6C: // "Control::PrevTab"
          case 0xEC827715: // "Control::NextTab"
            break;

          default : 
          {
            // Deactivate modal menus on events
            if (IsModalClose())
            {
              ICMenu *menu = this;
              while (menu)
              {
                menu->Deactivate();

                if (menu->parentMenu.Alive())
                {
                  menu = parentMenu;
                }
                else
                {
                  menu = NULL;
                }
              }
            }
          }
        }
        return (TRUE);
      }
    }
  }
  else

  if (e.type == Input::EventID())
  {
    // Input events
    switch (e.subType)
    {
      case Input::KEYDOWN:
      case Input::KEYREPEAT:
      {
        switch (e.input.code)
        {
          case DIK_ESCAPE:
          {
            // If modal then issue the Window::Close notification
            if (controlStyle & STYLE_MODAL)
            {
              // Close menu
              Deactivate();

              // Handled
              return (TRUE);
            }

            // Not handled
            break;
          }
        }

        // Not handled
        break;
      }
    }
  }

  // Allow IControl class to process this event
  return (IControl::HandleEvent(e));
}


//
// New
//
// Create a new menu from the code
//
ICMenu * ICMenu::New(const char *name, EventCallBack *func, void *cText, const char *tplate, ICMenu *parent, BinTree<void> *attributes)
{
  // Store the attributes in the transfer area
  initAttrib = attributes;
  initParentMenu = parent;

  // Create a context menu
  ICMenu *menu = IFace::Promote<ICMenu>(IFace::CreateControl(name, tplate, NULL, InitProc));

  if (menu == NULL)
  {
    ERR_FATAL(("Class [%s] is not a menu", tplate))
  }

  // Set it up
  menu->Setup(func, cText);

  return (menu);
}


//
// Find
//
// Find an ICMenu control
//
ICMenu *ICMenu::Find(const char *path)
{
  IControl *ctrl = IFace::FindByName(path);

  // This currently does not check for correct type
  if (ctrl && ctrl->DerivedFrom(ICMenu::ClassId()))
  {
    return (ICMenu *)ctrl;
  }

  return NULL;
}


//
// Initialization proc
//
void ICMenu::InitProc(IControl *ctrl)
{
  // Create a context menu
  ICMenu *menu = IFace::Promote<ICMenu>(ctrl);

  if (menu)
  {
    menu->parentMenu = initParentMenu;

    if (initAttrib)
    {
      menu->attrib = new BinTree<void>;
      initAttrib->Transfer(*menu->attrib);
      initAttrib = NULL;
    }
    else
    {
      menu->attrib = NULL;
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
// Class MenuButton
//



//
// MenuButton::MenuButton
//
// Constructor
//
MenuButton::MenuButton(IControl *parent) 
: ICButton(parent)
{
  // The default button style is flat
  //buttonStyle |= STYLE_FLAT;
}


//
// MenuButton::~MenuButton
//
// Destructor
//
MenuButton::~MenuButton()
{
  if (icon)
  {
    delete icon;
    icon = NULL;
  }
}


//
// DrawSelf
//
void MenuButton::DrawSelf(PaintInfo &pi)
{
  // If we're not over the item, use the down client rect
  if (!(controlState & STATE_SELECTED || controlState & STATE_HILITE))
  {
    // Move the client rect
    pi.client = clientRects[BS_DOWN];
    pi.client += ClientToScreen(pi.client.p0);
  }

  ICButton::DrawSelf(pi);
}


///////////////////////////////////////////////////////////////////////////////
//
// Class SubMenuButton
//


//
// Dummy callback
//
static Bool DummyCallBack(void *, U32, U32, U32)
{
  // Used in creation of submenu
  return (FALSE);
}


//
// Deactivate the control
//
Bool SubMenuButton::Deactivate()
{
  if (MenuButton::Deactivate())
  {
    // Deactivate the menu we're pointing at
    if (subMenu.Alive())
    {
      subMenu->Deactivate();
      subMenu->MarkForDeletion();
    }
    return (TRUE);
  }
  else
  {
    return (FALSE);
  }
}


//
// Configure control from an FScope
//
void SubMenuButton::Setup(FScope *fScope)
{
  switch (fScope->NameCrc())
  {
    case 0x1884A86E: // "SubMenuClass"
    {
      // Read class id
      menuClass = StdLoad::TypeString(fScope);
      break;
    }

    // Pass it to the previous level in the hierarchy
    default:
      MenuButton::Setup(fScope);
  }
}


//
// Generate the submenu
//
void SubMenuButton::CreateSubMenu()
{
  // If parent is a menu, inherit its settings
  if (!subMenu.Alive())
  {
    if (menuClass.str[0] == '\0')
    {
      ERR_FATAL(("Submenu class not specified for [%s]", Name()))
    }

    // Promote parent to a menu
    ICMenu *menu = IFace::Promote<ICMenu>(parent);

    // Inherit settings from our parent if we are on a menu
    ICMenu::EventCallBack *cb;
    void *context;

    if (menu && menu->GetCallBack())
    {
      cb = menu->GetCallBack();
      context = menu->GetContext();
    }
    else
    {
      cb = DummyCallBack;
      context = this;
    }

    // Create the sub menu
    subMenu = ICMenu::New("SubMenu", cb, context, menuClass.str, menu, menu->attrib);

    // Activate it and position it
    subMenu->Activate();

    // Using the position of this button calculate the position the menu will take
   
    // Is it going to fit on the right ?
    if (subMenu->GetSize().x < IFace::ScreenWidth() - (GetScreenPos().x + GetSize().x))
    {
      // Can we draw it down ?
      if (subMenu->GetSize().y < IFace::ScreenHeight() - GetScreenPos().y)
      {
        // Move it to right down
        subMenu->MoveTo(Point<S32>(GetScreenPos().x + GetSize().x, GetScreenPos().y));
        return;
      }
      // Can we draw it up ?
      else if (subMenu->GetSize().y < GetScreenPos().y + GetSize().y)
      {
        // Move it to right up
        subMenu->MoveTo(Point<S32>(GetScreenPos().x + GetSize().x, GetScreenPos().y + GetSize().y - subMenu->GetSize().y));
        return;
      }
    }
    // Is it going to fit on the left ?
    else if (subMenu->GetSize().x < GetScreenPos().x)
    {
      // Can we draw it down ?
      if (subMenu->GetSize().y < IFace::ScreenHeight() - GetScreenPos().y)
      {
        // Move it to left down
        subMenu->MoveTo(Point<S32>(GetScreenPos().x - subMenu->GetSize().x, GetScreenPos().y));
        return;
      }
      // Can we draw it up ?
      else if (subMenu->GetSize().y < GetScreenPos().y + GetSize().y)
      {
        // Move it to left up
        subMenu->MoveTo(Point<S32>(GetScreenPos().x - subMenu->GetSize().x, GetScreenPos().y + GetSize().y - subMenu->GetSize().y));
        return;
      }
    }

    // Just use the mouse position
    subMenu->MoveTo(Input::MousePos());
  }
}


//
// Event handler
//
U32 SubMenuButton::HandleEvent(Event &e)
{
  if (e.type == IFace::EventID())
  {
    // Interface events
    switch (e.subType)
    {
      case IFace::NOTIFY:
      {
        switch (e.iface.p1)
        {
          case ICButtonMsg::Press:
          {
            // Create submenu
            CreateSubMenu();

            // Default processing
            break;
          }
        }
      }

      case IFace::MOUSEIN:
        break;

      case IFace::MOUSEOUT:
        break;
    }
  }

  // Allow IControl class to process this event
  return (MenuButton::HandleEvent(e));
}


//
// Draw control
//
void SubMenuButton::DrawSelf(PaintInfo &pi)
{
  // Draw the button first
  MenuButton::DrawSelf(pi);

  ClipRect r = pi.client;

  // If we're not over the item, use the down client rect
  if (!(controlState & STATE_SELECTED || controlState & STATE_HILITE))
  {
    // Move the client rect
    r = clientRects[BS_DOWN];
    r += ClientToScreen(r.p0);
  }

  // Draw a triangle indicating this a sub menu
  Point<S32> pt[3];
  S32 h = r.Height();
  S32 h2 = h >> 1;
  S32 h4 = h >> 2;
  S32 y = r.p0.y + h2;

  pt[0].Set(r.p1.x - h4, y);
  pt[1].Set(r.p1.x - h4 - h2, y - h4);
  pt[2].Set(r.p1.x - h4 - h2, y + h4);
  IFace::RenderTriangle(pt, pi.colors->fg[ColorIndex()]);

  pt[0].Set(r.p0.x + h4, y);
  pt[1].Set(r.p0.x + h4 + h2, y - h4);
  pt[2].Set(r.p0.x + h4 + h2, y + h4);
  IFace::RenderTriangle(pt, pi.colors->fg[ColorIndex()]);

}
