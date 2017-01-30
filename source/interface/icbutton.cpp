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
#include "icbutton.h"
#include "iface.h"
#include "iface_util.h"
#include "iface_types.h"
#include "iface_sound.h"
#include "ifvar.h"
#include "input.h"
#include "debug_memory.h"


///////////////////////////////////////////////////////////////////////////////
//
// Struct ICButtonResize - Saved state info for resizing
//
struct ICButtonResize
{
  ICButtonPtr control;
  ICButton::ButtonState state;
};

static ICButtonResize resizeInfo;


///////////////////////////////////////////////////////////////////////////////
//
// Class ICButton - Standard button control
//


//
// ICButton::ICButton
//
ICButton::ICButton(IControl *parent)
: IControl(parent),
  toggleVar(NULL),
  buttonStyle(0),
  icon(NULL)
{
  // Default Control style
  controlStyle |= (STYLE_DROPSHADOW | STYLE_VGRADIENT | STYLE_TABSTOP);

  // Button state
  SetState(BS_UP);

  // Default sounds
  soundOver  = IFace::Sound::MENU_POPUP;
  soundClick = IFace::Sound::MENU_COMMAND;
}


//
// ICButton::~ICButton
//
ICButton::~ICButton()
{
  if (icon)
  {
    delete icon;
  }

  // Dispose of var
  if (toggleVar)
  {
    delete toggleVar;
  }
}


//
// ICButton::SetState
//
void ICButton::SetState(ButtonState state)
{
  // Set state
  buttonState = state;

  // Set selected state if necessary
  if (buttonStyle & STYLE_SELECTDOWN)
  {
    if (buttonState == BS_DOWN)
    {
      controlState |= STATE_SELECTED;
    }
    else
    {
      controlState &= ~STATE_SELECTED;
    }
  }

  // Move the client rect
  paintInfo.client = clientRects[buttonState];
}


//
// ICButton::Notify
//
// Var changed
//
void ICButton::Notify(IFaceVar *var)
{
  ASSERT(buttonStyle & STYLE_TOGGLE)

  if (toggleVar == var)
  {
    ASSERT(toggleVar)

    if (toggleVar->GetIntegerValue())
    {
      controlState |= STATE_SELECTED;
      SetState(BS_DOWN);
    }
    else
    {
      controlState &= ~STATE_SELECTED;
      SetState(BS_UP);
    }
  }
}


//
// ICButton::DrawSelf
//
// Draw the button control
//
void ICButton::DrawSelf(PaintInfo &pi)
{
  if (!(buttonStyle & STYLE_FLAT) ||
       (controlState & STATE_SELECTED || 
       (controlState & STATE_HILITE)))
  {
    // Fill the background
    DrawCtrlBackground(pi, GetTexture());

    // Draw the frame
    DrawCtrlFrame(pi);
  }

  // Draw the text
  if (pi.font)
  {
    if ((buttonStyle & STYLE_TOGGLE) && (textStr != NULL))
    {
      DrawCtrlText(pi, textStr);
    }
    else
    {
      DrawCtrlText(pi);
    }
  }

  // Draw the icon
  if (icon)
  {
    IFace::RenderRectangle
    (
      pi.client, 
      skin ? pi.colors->bg[ColorIndex()] : IFace::data.cgTexture->bg[ColorIndex()],
      icon,
      pi.alphaScale
    );
  }
}


//
// ICButton::AdjustGeometry
//
// Adjust geometry of the button
//
void ICButton::AdjustGeometry()
{
  // Call base class
  IControl::AdjustGeometry();

  // Calculate client rectangles when in up and down states
  if (controlStyle & STYLE_DROPSHADOW)
  {
    // Up client area is same as original
    clientRects[BS_UP] = paintInfo.client;

    // Down client area is moved down by size of dropshadow
    clientRects[BS_DOWN] = paintInfo.client + (IFace::GetMetric(IFace::DROPSHADOW_UP) - IFace::GetMetric(IFace::DROPSHADOW_DOWN));
  }
  else
  {
    clientRects[BS_UP]   = paintInfo.client;
    clientRects[BS_DOWN] = paintInfo.client;
  }
}


//
// ICButton::Activate
//
Bool ICButton::Activate()
{
  if (IControl::Activate())
  {
    // Check settings
    if (buttonStyle & STYLE_TOGGLE)
    {
      // Check and setup the var
      ActivateVar(toggleVar, VarSys::VI_INTEGER);
    }
    return (TRUE);
  }

  return (FALSE);
}


//
// ICButton::Deactivate
//
Bool ICButton::Deactivate()
{
  if (IControl::Deactivate())
  {
    // Check settings
    if (buttonStyle & STYLE_TOGGLE)
    {
      toggleVar->Deactivate();
    }
    return (TRUE);
  }

  return (FALSE);
}


//
// ICButton::SetStyleItem
//
// Change a style setting
//
Bool ICButton::SetStyleItem(const char *s, Bool toggle)
{
  U32 style;

  switch (Crc::CalcStr(s))
  {
    case 0x62A243C5: // "Toggle"
      style = STYLE_TOGGLE;
      break;

    case 0xC73B0775: // "Flat"
      style = STYLE_FLAT;
      break;

    case 0xC1B3BA0B: // "SelectWhenDown"
      style = STYLE_SELECTDOWN;
      break;

    default:
      return IControl::SetStyleItem(s, toggle);
  }

  // Toggle the style
  buttonStyle = (toggle) ? (buttonStyle | style) : (buttonStyle & ~style);

  return TRUE;
}


//
// ICButton::Setup
//
// Configure this control with an FScope
//
void ICButton::Setup(FScope *fScope)
{
  switch (fScope->NameCrc())
  {
    case 0x742EA048: // "UseVar"
    {
      ConfigureVar(toggleVar, fScope);
      break;
    }

    case 0x2598B3EB: // "Icon"
    {
      TextureInfo t;
      IFace::FScopeToTextureInfo(fScope, t);

      // If we don't have an icon, allocate one
      if (icon == NULL)
      {
        icon = new TextureInfo;
      }
      *icon = t;
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

void ICButton::SetIcon( TextureInfo & tinfo)
{
 // If we don't have an icon, allocate one
  if (icon == NULL)
  {
    icon = new TextureInfo;
  }
  *icon = tinfo;
}


//
// ICButton::HandleEvent
//
// Process input events
//
U32 ICButton::HandleEvent(Event &e)
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
          if (controlState & STATE_DISABLED)
          {
            SendNotify(this, ICButtonNotify::DisabledDown);
          }
          else
          {
            if (buttonStyle & STYLE_TOGGLE)
            {
              // Generate message
              SendNotify(this, ICButtonMsg::Toggle, FALSE);

              // Play click sound
              IFace::Sound::Play(soundClick, this);
            }
            else
            {
              SetState(BS_DOWN);
              GetMouseCapture();
              GetKeyFocus();
              SendNotify(this, ICButtonNotify::Pressing);
              return TRUE;
            }
          }
        }
        break;
      }

      case Input::MOUSEBUTTONUP:
      case Input::MOUSEBUTTONDBLCLKUP:
      {
        if (e.input.code == Input::LeftButtonCode())
        {
          if (!(buttonStyle & STYLE_TOGGLE))
          {
            // If mouse is over the button, generate a message
            if (buttonState == BS_DOWN)
            {
              // Set button state
              SetState(BS_UP);

              // Generate message
              SendNotify(this, ICButtonMsg::Press, FALSE);
            }

            // Release keyboard focus
            if (HasKeyFocus())
            {
              ReleaseKeyFocus();
            }

            // Release mouse capture
            if (HasMouseCapture())
            {
              ReleaseMouseCapture();
            }

            // Play click sound
            IFace::Sound::Play(soundClick, this);
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
          // Toggle the up state depending on the mouse position
          if (InWindow(Point<S32>(e.input.mouseX, e.input.mouseY)))
          {
            SetState(BS_DOWN);
          }
          else
          {
            SetState(BS_UP);
          }

          // Handled
          return (TRUE);
        }

        // Not handled
        break;
      }

      case Input::KEYDOWN:
      {
        switch (e.input.code)
        {
          case DIK_RETURN:
          case DIK_SPACE:
          {
            // Enter and space simulate left button clicks
            if (!(controlState & STATE_DISABLED))
            {
              // Generate message
              if (buttonStyle & STYLE_TOGGLE)
              {
                SendNotify(this, ICButtonMsg::Toggle, FALSE);
              }
              else
              {
                SendNotify(this, ICButtonMsg::Press, FALSE);
              }

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
    // Interface events
    switch (e.subType)
    {
      // Notification events
      case IFace::NOTIFY:
      {
        switch (e.iface.p1)
        {
          case ICButtonMsg::Press:
          {
            // Generate pressed notification
            SendNotify(this, ICButtonNotify::Pressed);

            // Handled
            return (TRUE);
          }

          case ICButtonMsg::Toggle:
          {
            if (!(controlState & STATE_DISABLED))
            {
              if (toggleVar)
              {
                VALIDATE(toggleVar);

                // Set the value of the var
                toggleVar->SetIntegerValue(!toggleVar->GetIntegerValue());

                // Generate toggled notification
                SendNotify(this, ICButtonNotify::Toggled);
              }
            }

            // Handled
            return (TRUE);
          }

          case IControlNotify::PreResize:
          {
            ASSERT(IsActive())

            resizeInfo.control = this;
            resizeInfo.state   = buttonState;

            // Call base class
            break;
          }

          case IControlNotify::PostResize:
          {
            if (resizeInfo.control.Alive() && (resizeInfo.control.GetData() == this))
            {
              SetState(resizeInfo.state);
            }

            // Clear reaper
            resizeInfo.control = NULL;

            // Call base class
            break;
          }

        }
        break;
      }

      case IFace::LOSTCAPTURE:
      case IFace::LOSTFOCUS:
      {
        // Clear state
        if (!(buttonStyle & STYLE_TOGGLE))
        {
          SetState(BS_UP);
        }

        // Pass through to IControl
        break;
      }

      case IFace::MOUSEIN:
      {
        // Play mouse in sound
        IFace::Sound::Play(soundOver, this);
        break;
      }
    }
  }

  // Allow IControl class to process this event
  return (IControl::HandleEvent(e));
}
