/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Slider control
//
// 06-JUL-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icslider.h"
#include "icbutton.h"
#include "iface.h"
#include "iface_util.h"
#include "iface_priv.h"
#include "iface_types.h"
#include "ifvar.h"
#include "input.h"
#include "debug_memory.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ICSlider implementation
//

// Control names
static const char *DecBtnCtrlName = "[Decrement]";
static const char *IncBtnCtrlName = "[Increment]";
static const char *ThumbCtrlName  = "[Thumb]";


//
// ICSlider::ICSlider
//
ICSlider::ICSlider(IControl *parent) 
: IControl(parent), 
  sliderVar(NULL),
  sliderStyle(0),
  horizontal(TRUE),
  minVal(0.0F),
  maxVal(0.0F),
  rangeLo(0.0F),
  rangeHi(100.0F),
  useRange(FALSE),
  moveKnob(TRUE),
  decBtnCfg(NULL),
  incBtnCfg(NULL),
  thumbCfg(NULL)
{
  // Default slider style
  sliderStyle = STYLE_SQUARETHUMB;

  // Default style
  controlStyle |= STYLE_TABSTOP | STYLE_TRANSPARENT;

  // Default color
  SetColorGroup(IFace::data.cgClient);
}


//
// ICSlider::~ICSlider
//
ICSlider::~ICSlider()
{
  // Dispose of var
  if (sliderVar)
  {
    delete sliderVar;
    sliderVar = NULL;
  }
}


//
// ICSlider::GetAdjustmentRect
//
// Calculate an adjustment rect based on the style
//
ClipRect ICSlider::GetAdjustmentRect()
{
  ClipRect r(0, 0, 0, 0);

  // Adjust geometry to compensate for slider
  if (skin == NULL)
  {
    if (sliderStyle & STYLE_BUTTONS)
    {
      if (horizontal)
      {
        r.Set(size.y, 0, -size.y, 0);
      }
      else
      {
        r.Set(0, size.x, 0, -size.x);
      }
    }
  }

  return (r + IControl::GetAdjustmentRect());
}


//
// ICSlider::PostConfigure
//
// Post configure notification
//
void ICSlider::PostConfigure()
{
  // PostConfigure base class first
  IControl::PostConfigure();

  // Create buttons
  if (sliderStyle & STYLE_BUTTONS)
  {
    if (decBtnCfg && incBtnCfg)
    {
      // Create customised buttons
      IFace::CreateControl(DecBtnCtrlName, decBtnCfg, this);
      IFace::CreateControl(IncBtnCtrlName, incBtnCfg, this);
    }
    else
    {
      IControl *dec, *inc;

      // Create generic buttons
      if (horizontal)
      {
        // Left
        dec = new ICSystemButton(ICSystemButton::SLIDER_LEFT, this);
        dec->SetGeometry("WinParentHeight", "WinParentWidth","WinLeft", "Square", NULL);

        // Right
        inc = new ICSystemButton(ICSystemButton::SLIDER_RIGHT, this);
        inc->SetGeometry("WinParentHeight", "WinParentWidth", "WinRight", "Square", NULL);
      }
      else
      {
        // Up
        dec = new ICSystemButton(ICSystemButton::SLIDER_UP, this);
        dec->SetGeometry("WinParentHeight", "WinParentWidth", "WinTop", "Square", NULL);

        // Down
        inc = new ICSystemButton(ICSystemButton::SLIDER_DOWN, this);
        inc->SetGeometry("WinParentHeight", "WinParentWidth", "WinBottom", "Square", NULL);
      }

      dec->SetName(DecBtnCtrlName);
      inc->SetName(IncBtnCtrlName);
    }
  }

  if (thumbCfg)
  {
    // Create custom thumb track
    thumbBtn = IFace::CreateControl(ThumbCtrlName, thumbCfg, this);
  }
  else
  {
    // Create generic thumb track
    thumbBtn = new ICSliderThumb(this);
    thumbBtn->SetName(ThumbCtrlName);

    if (sliderStyle & STYLE_SQUARETHUMB)
    {
      thumbBtn->SetGeometry("ParentHeight", "ParentWidth", "Square", NULL);
    }
    else
    {
      thumbBtn->SetGeometry(horizontal ? "ParentHeight" : "ParentWidth", NULL);
    }
  }
}


//
// ICSlider::GetSliderValue
//
// Retrieve the slider value from the var
//
void ICSlider::GetSliderValue()
{
  VALIDATE(sliderVar)

  // Read current var value
  if (sliderVar->Type() == VarSys::VI_INTEGER)
  {
    curVal = F32(sliderVar->GetIntegerValue());
  }
  else
  {
    curVal = F32(sliderVar->GetFloatValue());
  }

  curVal = Clamp<F32>(minVal, curVal, maxVal);
}


//
// ICSlider::SetSliderValue
//
// Store the slider value in the var
//
void ICSlider::SetSliderValue(F32 value)
{
  VALIDATE(sliderVar)

  value = Clamp<F32>(minVal, value, maxVal);

  // Update internal value
  if (sliderVar->Type() == VarSys::VI_INTEGER)
  {
    sliderVar->SetIntegerValue(Utils::FtoL(value));
  }
  else
  {
    sliderVar->SetFloatValue(value);
  }
}


//
// ICSlider::Notify
//
// Var notification
//
void ICSlider::Notify(IFaceVar *)
{
  // Flag knob as dirty
  moveKnob = TRUE;
}


//
// ICSlider::DrawSelf
//
void ICSlider::DrawSelf(PaintInfo &pi)
{
  // Knob position was changed
  if (moveKnob)
  {
    VALIDATE(sliderVar)

    // Update slider value
    GetSliderValue();

    // Update the knob position
    if (thumbBtn.Alive())
    {
      ClipRect thumbRange = GetThumbRange();
      S32 thumbW = thumbBtn->GetPaintInfo().window.Width();
      S32 thumbH = thumbBtn->GetPaintInfo().window.Height();

      if (horizontal)
      {
        S32 pos = range 
                  ? Utils::FtoL(((curVal - minVal) / range) * F32(thumbRange.Width() - thumbW)) 
                  : 0;

        thumbBtn->MoveTo(thumbRange.p0 + Point<S32>(pos, 0));
      }
      else
      {
        S32 pos = range 
                  ? Utils::FtoL(((curVal - minVal) / range) * F32(thumbRange.Height() - thumbH)) 
                  : 0;

        thumbBtn->MoveTo(thumbRange.p0 + Point<S32>(0, pos));
      }
    }

    moveKnob = FALSE;
  }

  // Draw frame
  DrawCtrlFrame(pi);

  // Draw background
  DrawCtrlBackground(pi, GetTexture());

  // Draw client area
  if (!(sliderStyle & STYLE_NODRAWCLIENT))
  {
    if (thumbBtn.Alive())
    {
      const Point<S32> &tPos  = thumbBtn->GetPos();
      const Point<S32> &tSize = thumbBtn->GetSize();

      S32 shadow = (controlStyle & STYLE_DROPSHADOW) ? IFace::GetMetric(IFace::DROPSHADOW_UP) : 0;

      // Draw left side
      if 
      (
        (horizontal && (tPos.x > shadow + 1)) 
        ||
        (!horizontal && (tPos.y > shadow + 1))
      )
      {
        ClipRect r = pi.client;

        if (horizontal)
        {
          r.p1.x = r.p0.x + tPos.x;
        }
        else
        {
          r.p1.y = r.p0.y + tPos.y;
        }

        // Adjust for shadow
        r.p1 -= shadow;

        // Render client
        IFace::RenderRectangle(r, pi.colors->bg[ColorIndex()]);

        // Render shadow
        if (shadow)
        {
          IFace::RenderShadow(r, r + shadow, Color(0, 0, 0, IFace::GetMetric(IFace::SHADOW_ALPHA)), 0);
        }
      }

      // Draw right side
      if 
      (
        (horizontal && (tPos.x + tSize.x < pi.client.Width()))
        ||
        (!horizontal && (tPos.y + tSize.x < pi.client.Height()))
      )
      {
        ClipRect r = pi.client;

        if (horizontal)
        {
          r.p0.x += (tPos.x + tSize.x);
        }
        else
        {
          r.p0.y += (tPos.y + tSize.y);
        }

        // Adjust for shadow
        r.p1 -= shadow;

        // Render client
        IFace::RenderRectangle(r, pi.colors->bg[ColorIndex()]);

        // Render shadow
        if (shadow)
        {
          IFace::RenderShadow(r, r + shadow, Color(0, 0, 0, IFace::GetMetric(IFace::SHADOW_ALPHA)), 0);
        }
      }
    }
  }
}


//
// ICSlider::InitRange
//
// Initialise range parameters, only to be done when active
//
void ICSlider::InitRange()
{
  ASSERT(sliderVar);
  ASSERT(IsActive());

  // Set up ranges
  VarSys::VarItem &item = sliderVar->GetItem();

  if (useRange || (!(item.flags & VarSys::CLAMP)))
  {
    // Var override, or no var clamping, so use rangeLo and rangeHi
    minVal = rangeLo;
    maxVal = rangeHi;
  }
  else
  {
    switch (sliderVar->Type())
    {
      case VarSys::VI_INTEGER:
      {
        minVal = F32(item.integer.lo);
        maxVal = F32(item.integer.hi);
        break;
      }

      case VarSys::VI_FPOINT:
      {
        minVal = F32(item.fpoint.lo);
        maxVal = F32(item.fpoint.hi);
        break;
      }
    }
  }

  // Incremental step
  range = maxVal - minVal;

  // Ensure range is ok
  ClipRect thumbRange = GetThumbRange();

  if ((thumbRange.Width() <= 0) || (thumbRange.Height() <= 0))
  {
    ERR_FATAL(("Slider [%s] is too small, make it bigger", Name()))
  }

  // Calculate increment step for one notch
  if (sliderVar->Type() == VarSys::VI_INTEGER)
  {
    // Ensure increment step is always 1 for integral vars
    incStep = 1.0F;
  }
  else
  {
    // Otherwise make the increment smooth
    incStep = range / F32(horizontal ? thumbRange.Width() - size.y : thumbRange.Height() - size.x);
  }
}


//
// ICSlider::Activate
//
// Activate Control, makes it visible and active and creates resources
//
Bool ICSlider::Activate()
{
  if (IControl::Activate())
  {
    // Setup and check the var
    ActivateVar(sliderVar);

    if ((sliderVar->Type() != VarSys::VI_INTEGER) && (sliderVar->Type() != VarSys::VI_FPOINT))
    {
      ERR_FATAL(("Integer or Float var expected for slider [%s]", Name()));
    }

    // Initialise sliding range
    InitRange();

    // Get current values of the var
    GetSliderValue();

    return (TRUE);
  }
  else
  {
    return (FALSE);
  }
}


//
// ICSlider::Deactivate
//
// Deactive control
//
Bool ICSlider::Deactivate()
{
  if (IControl::Deactivate())
  {
    // Unlink from slider var
    sliderVar->Deactivate();
    return (TRUE);
  }
  else
  {
    return (FALSE);
  }
}


//
// ICSlider::Setup
//
void ICSlider::Setup(FScope *fScope)
{
  switch (fScope->NameCrc())
  {
    case 0x742EA048: // "UseVar"
    {
      ConfigureVar(sliderVar, fScope);
      break;
    }

    case 0xEE2D2689: // "Orientation"
    {
      SetOrientation(fScope->NextArgString());
      break;
    }

    case 0xE17D7C71: // "Range"
    {
      // Override range
      rangeLo = fScope->NextArgFPoint();
      rangeHi = fScope->NextArgFPoint();
      useRange = TRUE;
      break;
    }

    case 0x87AC33D3: // "DecButtonConfig"
      decBtnCfg = IFace::FindRegData(fScope->NextArgString());
      break;

    case 0x2F0229B7: // "IncButtonConfig"
      incBtnCfg = IFace::FindRegData(fScope->NextArgString());
      break;

    case 0x1B1A0247: // "ThumbConfig"
      thumbCfg = IFace::FindRegData(fScope->NextArgString());
      break;

    default:
    {
      // Pass it to the previous level in the hierarchy
      IControl::Setup(fScope);
      break;
    }
  }
}


//
// ICSlider::SetStyleItem
//
// Change a style setting
//
Bool ICSlider::SetStyleItem(const char *s, Bool toggle)
{
  U32 style;

  switch (Crc::CalcStr(s))
  {
    case 0x7619B2C8: // "Buttons"
      style = STYLE_BUTTONS;
      break;

    case 0xDB928D52: // "NoDrawClient"
      style = STYLE_NODRAWCLIENT;
      break;
    
    default:
      return (IControl::SetStyleItem(s, toggle));
  }

  // Toggle the style
  sliderStyle = (toggle) ? (sliderStyle | style) : (sliderStyle & ~style);

  return (TRUE);
}


//
// ICSlider::SetOrientation
//
// Set the orientation of the slider
//
void ICSlider::SetOrientation(ICSlider::Orientation o)
{
  horizontal = (o == HORIZONTAL) ? TRUE : FALSE;
}


//
// UseVar
//
// Setup the var for this slider to use
//
void ICSlider::UseVar(VarSys::VarItem *item)
{
  ASSERT(!sliderVar);

  sliderVar = new IFaceVar(this, item);
}


//
// ICSlider::SetOrientation
//
// Set the orientation of the slider using the string representation
//
void ICSlider::SetOrientation(const char *s)
{
  switch (Crc::CalcStr(s))
  {
    case 0x2942B3CD: // "Horizontal"
      SetOrientation(HORIZONTAL);
      break;

    case 0xC8F18F06: // "Vertical"
      SetOrientation(VERTICAL);
      break;

    default:
      ERR_FATAL(("Invalid orientation [%s]", s));
      break;
  }
}


//
// ICSlider::HandleEvent
//
// Event handler
//
U32 ICSlider::HandleEvent(Event &e)
{
  VALIDATE(sliderVar);

  if (e.type == Input::EventID())
  {
    switch (e.subType)
    {
      case Input::MOUSEAXIS:
      {
        S16 amount = S16(e.input.ch);

        // Increment or decrement
        SetSliderValue(curVal - F32(amount / 120) * incStep);

        // Handled
        return TRUE;
      }

      case Input::KEYDOWN:
      case Input::KEYREPEAT:
      {
        switch (e.input.code)
        {
          case DIK_UP:
          case DIK_LEFT:
          {
            // Decrement
            SetSliderValue(curVal - incStep);

            // Handled
            return TRUE;
          }

          case DIK_DOWN:
          case DIK_RIGHT:
          {
            // Increment
            SetSliderValue(curVal + incStep);

            // Handled
            return TRUE;
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
        U32 event = e.iface.p1;

        switch (event)
        {
          case ICSliderMsg::Decrement:
          {
            // Decrement button was pressed
            SetSliderValue(curVal - incStep);

            // Handled
            return (TRUE);
          }

          case ICSliderMsg::Increment:
          {
            // Increment button was pressed
            SetSliderValue(curVal + incStep);

            // Handled
            return (TRUE);
          }
        }
      }
    }
  }

  // Allow IControl class to process this event
  return (IControl::HandleEvent(e));
}


//
// ICSlider::GetThumbRange
//
// Return range of movement of thumb
//
ClipRect ICSlider::GetThumbRange()
{
  return (paintInfo.client - paintInfo.client.p0);
}


///////////////////////////////////////////////////////////////////////////////
//
// ICSliderThumb implementation
//


//
// Constructor
//
ICSliderThumb::ICSliderThumb(IControl *parent)
: ICButton(parent), 
  slider(NULL) 
{
}


//
// ICSliderThumb::PostConfigure
//
void ICSliderThumb::PostConfigure()
{
  // PostConfigure base class first
  ICButton::PostConfigure();

  // Ensure thumb's parent is a slider
  slider = IFace::Promote<ICSlider>(parent);

  if (slider == NULL)
  {
    ERR_FATAL(("SliderThumb [%s] must be placed on a slider", Name()))
  }
}


//
// ICSliderThumb::HandleEvent
//
U32 ICSliderThumb::HandleEvent(Event &e)
{
  ASSERT(slider)

  if (e.type == Input::EventID())
  {
    // Input events
    switch (e.subType)
    {
      case Input::MOUSEBUTTONDOWN:
      {
        if (e.input.code == Input::LeftButtonCode())
        {
          // Offset of cursor within thumb
          dragPos = ScreenToWindow(Point<S32>(e.input.mouseX, e.input.mouseY));
        }
        break;
      }

      case Input::MOUSEAXIS:
      {
        if (!HasMouseCapture())
        {
          // Pass event to parent
          parent->HandleEvent(e);

          return (TRUE);
        }
        break;
      }

      case Input::MOUSEMOVE:
      {
        if (HasMouseCapture())
        {
          // Get movement rectangle from parent
          ClipRect moveRect = slider->GetThumbRange();

          // Track the mouse
          Point<S32> newPos = slider->ScreenToClient(Point<S32>(e.input.mouseX, e.input.mouseY)) - dragPos;

          // Update slider value
          F32 fSize;
          F32 fPos;

          if (slider->horizontal)
          {
            fSize = F32(moveRect.Width() - GetPaintInfo().window.Width());
            fPos = F32(newPos.x);
          }
          else
          {
            fSize = F32(moveRect.Height() - GetPaintInfo().window.Height());
            fPos = F32(newPos.y);
          }

          if (fSize > 1e-4F)
          {
            F32 fVal = (fPos * slider->range / fSize) + slider->minVal;
            slider->SetSliderValue(fVal);
          }

          // Don't allow button to process this
          return (TRUE);
        }
        break;
      }
    }
  }

  // This event can't be handled by this control, so pass it to the parent class
  return (ICButton::HandleEvent(e));
}
