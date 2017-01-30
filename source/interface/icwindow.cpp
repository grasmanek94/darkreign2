/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//


#include "icwindow.h"
#include "icbutton.h"
#include "icstatic.h"
#include "iface.h"
#include "iface_priv.h"
#include "iface_util.h"
#include "iface_types.h"
#include "fontsys.h"
#include "input.h"
#include "stdload.h"


// Name of code generated title bar control
static const char *TitleBarCtlName = "[Titlebar]";

// Name of code generated close button
static const char *CloseBtnCtlName = "[Close]";


///////////////////////////////////////////////////////////////////////////////
//
// Class ICWindow - Standard window control
//


//
// ICWindow::ICWindow
//
ICWindow::ICWindow(IControl *parent) 
: IControl(parent),
  windowStyle(0),
  titleBarConfig(NULL),
  closeBtnConfig(NULL)
{ 
  // Default control style
  controlStyle |= STYLE_DROPSHADOW;
}


//
// ICWindow::~ICWindow
//
ICWindow::~ICWindow()
{
}


//
// ICWindow::PostConfigure
//
// Post configuration
//
void ICWindow::PostConfigure()
{
  // Adjust window so that client size == configured size
  if (windowStyle & STYLE_ADJUSTWINDOW)
  {
    ClipRect r = GetAdjustmentRect();

    size.x += (r.p0.x - r.p1.x);
    size.y += (r.p0.y - r.p1.y);
    geom.size.x += (r.p0.x - r.p1.x);
    geom.size.y += (r.p0.y - r.p1.y);
  }

  // Post configure IControl
  IControl::PostConfigure();

  // If a title bar is specified create a control for it
  if (windowStyle & STYLE_TITLEBAR)
  {
    IControl *closeBtn = NULL;

    if (titleBarConfig)
    {
      // Use custom defined title bar
      titleBar = IFace::CreateControl(TitleBarCtlName, titleBarConfig, this);

      if (!(windowStyle & STYLE_NOSYSBUTTONS) && (windowStyle & STYLE_CLOSEBUTTON) && closeBtnConfig)
      {
        // Use custom defined close button
        closeBtn = IFace::CreateControl(CloseBtnCtlName, closeBtnConfig, this);
      }
    }
    else
    {
      // Otherwise use the default code style
      Bool thinTitle;
      S32 titleHeight;

      if (windowStyle & STYLE_THINTITLEBAR)
      {
        thinTitle = TRUE;
        titleHeight = IFace::GetMetric(IFace::THIN_TITLE_HEIGHT);
      }
      else
      {
        thinTitle = FALSE;
        titleHeight = IFace::GetMetric(IFace::TITLE_HEIGHT);
      }

      // Find the title font
      Font *font = FontSys::GetFont(IFace::GetMetric(thinTitle ? IFace::THIN_TITLE_FONT : IFace::TITLE_FONT));
      if (font == NULL)
      {
        ERR_FATAL(("Title font not found"));
      }

      // Create title bar
      titleBar = new ICWindowTitle(this);

      // Hard coded defaults
      titleBar->SetName(TitleBarCtlName);
      titleBar->SetSize(0, titleHeight);
      titleBar->SetPos(0, -titleHeight);
      titleBar->SetGeometry("WinParentWidth", NULL);
      titleBar->SetStyle("TitleGradient", "DropShadow", NULL);
      titleBar->SetFont(font);
      titleBar->SetColorGroup(IFace::data.cgTitle);
      titleBar->SetTextJustify(JUSTIFY_LEFT);

      // Create close button after titlebar
      if (!(windowStyle & STYLE_NOSYSBUTTONS) && (windowStyle & STYLE_CLOSEBUTTON))
      {
        closeBtn = new ICSystemButton(ICSystemButton::CLOSE, this);

        // Hard coded defaults
        closeBtn->SetName(CloseBtnCtlName);
        closeBtn->SetSize(titleHeight - 2, titleHeight - 2);
        closeBtn->SetPos(-2, -titleHeight+1);
        closeBtn->SetGeometry("Right", NULL);
      }
    }

    // Common configuration
    SetTextString(textStr ? textStr : Utils::Ansi2Unicode(ident.str), TRUE);
  }
}


//
// ICWindow::GetAdjustmentRect
//
// Calculate an adjustment rect based on the style
//
ClipRect ICWindow::GetAdjustmentRect()
{
  ClipRect r(0, 0, 0, 0);

  if (skin == NULL)
  {
    if (windowStyle & STYLE_TITLEBAR)
    {
      r.Set(0, IFace::GetMetric(IFace::TITLE_HEIGHT), 0, 0);
    }
    else

    if (windowStyle & STYLE_THINTITLEBAR)
    {
      r.Set(0, IFace::GetMetric(IFace::THIN_TITLE_HEIGHT), 0, 0);
    }
  }

  return (r + IControl::GetAdjustmentRect());
}


//
// ICWindow::Setup
//
// Configure the control
//
void ICWindow::Setup(FScope *fScope)
{
  switch (fScope->NameCrc())
  {
    case 0xC09E206D: // "TitleBarConfig"
    {
      titleBarConfig = IFace::FindRegData(fScope->NextArgString());
      break;
    }

    case 0x06B57762: // "CloseButtonConfig"
    {
      closeBtnConfig = IFace::FindRegData(fScope->NextArgString());
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
// ICWindow::DrawSelf
//
// Draw the window
//
void ICWindow::DrawSelf(PaintInfo &pi)
{
  // Redraw the background image
  DrawCtrlBackground(pi, GetTexture());

  // Redraw the frame
  DrawCtrlFrame(pi);
}


//
// ICWindow::HandleEvent
//
// Handle input events
//
U32 ICWindow::HandleEvent(Event &e)
{
  if (e.type == Input::EventID())
  {
    // Input events
    switch (e.subType)
    {
      case Input::MOUSEBUTTONDOWN:
      {
        if (e.input.code == Input::LeftButtonCode())
        {
          if (InWindow(Point<S32>(e.input.mouseX, e.input.mouseY)))
          {
            // Raise window
            SendNotify(this, ICWindowMsg::Raise, FALSE);

            // Handled
            return (TRUE);
          }
        }
        break;
      }

      case Input::KEYDOWN:
      case Input::KEYREPEAT:
      {
        switch (e.input.code)
        {
          case DIK_TAB:
          {
            // If nothing has the focus then activate the first tab stop control
            if (IFace::GetFocus() == NULL)
            {
              SetTabStop(NULL, TRUE);

              // Handled
              return (TRUE);
            }

            // Not handled
            break;
          }

          case DIK_ESCAPE:
          {
            // If modal then issue the Window::Close notification
            if ((controlStyle & STYLE_MODAL) && !(windowStyle & STYLE_NOSYSBUTTONS))
            {
              // Close window
              SendNotify(this, ICWindowMsg::Close, FALSE);

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
  else

  if (e.type == IFace::EventID())
  {
    switch (e.subType)
    {
      // Notification events
      case IFace::NOTIFY:
      {
        switch (e.iface.p1)
        {
          case ICWindowMsg::Raise:
          {
            // Raise the window
            SetZPos(0);

            // Handled
            return (TRUE);
          }

          case ICWindowMsg::Close:
          {
            // Close window
            Deactivate();

            // Handled
            return (TRUE);
          }

          case ICWindowMsg::Maximize:
          case ICWindowMsg::Minimize:
          {
            // Not implemented
            break;
          }

          case ICWindowMsg::Destroy:
          {
            // Mark for deletion
            MarkForDeletion();

            // Handled
            return (TRUE);
          }
        }

        // Not handled
        break;
      }
    }
  }

  // This event can't be handled by this control, so pass it to the parent class
  return (IControl::HandleEvent(e));
}


//
// Set the text to display on this control
//
void ICWindow::SetTextString(const CH *s, Bool dup, Bool cleanup)
{
  if (titleBar.Alive())
  {
    titleBar->SetTextString(s, dup, cleanup);
  }
  else
  {
    IControl::SetTextString(s, dup, cleanup);
  }
}


//
// ICWindow::GetWindowTitle
//
// Return window title control
//
IControl *ICWindow::GetWindowTitle()
{
  if (titleBar.Alive())
  {
    return (titleBar);
  }
  else
  {
    return (NULL);
  }
}


//
// ICWindow::SetStyleItem
//
// Change a style setting
//
Bool ICWindow::SetStyleItem(const char *s, Bool toggle)
{
  U32 style;

  switch (Crc::CalcStr(s))
  {
    case 0x5984D56F: // "TitleBar"
      style = STYLE_TITLEBAR | STYLE_CLOSEBUTTON;
      break;

    case 0xC03EC808: // "ThinTitleBar"
      style = STYLE_TITLEBAR | STYLE_THINTITLEBAR | STYLE_CLOSEBUTTON;
      break;

    case 0x4DA2A66E: // "CloseButton"
      style = STYLE_CLOSEBUTTON;
      break;

    case 0x24F45607: // "MaxButton"
      style = STYLE_MAXBUTTON;
      break;

    case 0xB1381124: // "MinButton"
      style = STYLE_MINBUTTON;
      break;

    case 0xD2558D4B: // "HelpButton"
      style = STYLE_HELPBUTTON;
      break;

    case 0xFF5F1F2A: // "NoSysButtons"
      style = STYLE_NOSYSBUTTONS;
      break;

    case 0x5AA830EF: // "AdjustWindow"
      style = STYLE_ADJUSTWINDOW;
      break;

    case 0xAD6CFFC7: // "Immovable"
      style = STYLE_IMMOVABLE;
      break;

    default:
      return IControl::SetStyleItem(s, toggle);
  }

  // Toggle the style
  windowStyle = (toggle) ? (windowStyle | style) : (windowStyle & ~style);

  return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//
// Class ICWindowTitle - Window titlebar control
//


//
// ICWindowTitle::ICWindowTitle
//
ICWindowTitle::ICWindowTitle(IControl *parent) 
: IControl(parent)
{
}


//
// ICWindowTitle::DrawSelf
//
void ICWindowTitle::DrawSelf(PaintInfo &pi)
{
  DrawCtrlBackground(pi, GetTexture());
  DrawCtrlFrame(pi);
  DrawCtrlText(pi);
}


//
// ICWindowTitle::HandleEvent
//
U32 ICWindowTitle::HandleEvent(Event &e)
{
  if (e.type == Input::EventID())
  {
    // Input events
    switch (e.subType)
    {
      case Input::MOUSEBUTTONDOWN:
      case Input::MOUSEBUTTONDBLCLK:
      {
        if (e.input.code == Input::LeftButtonCode())
        {
          // Don't move window if it has immovable style
          ICWindow *parentWnd = IFace::Promote<ICWindow>(parent);

          if (parentWnd && !(parentWnd->GetWinStyle() & ICWindow::STYLE_IMMOVABLE))
          {
            // Take mouse capture
            GetMouseCapture();

            // Raise window
            SendNotify(parent, ICWindowMsg::Raise, FALSE);

            // Start dragging
            dragPos = Point<S32>(e.input.mouseX, e.input.mouseY);
          }
        }
        break;
      }

      case Input::MOUSEBUTTONUP:
      case Input::MOUSEBUTTONDBLCLKUP:
      {
        if (e.input.code == Input::LeftButtonCode())
        {
          // Release mouse capture
          if (HasMouseCapture())
          {
            ReleaseMouseCapture();
          }

          // Handled
          return (TRUE);
        }

        // Not handled
        break;
      }

      case Input::MOUSEMOVE:
      {
        if (HasMouseCapture())
        {
          ASSERT(parent);

          // Track the mouse
          Point<S32> mouse = Point<S32>(e.input.mouseX, e.input.mouseY);
          Point<S32> newPos = parent->GetPos() + mouse - dragPos;

          parent->MoveTo(newPos);
          dragPos = mouse;

          // Don't allow button to process this
          return (TRUE);
        }
        break;
      }
    }
  }

  // This event can't be handled by this control, so pass it to the parent class
  return (IControl::HandleEvent(e));
}
