///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Color Editor
//
// 1-MAR-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "iface.h"
#include "ifvar.h"
#include "iface_util.h"
#include "iface_types.h"
#include "input.h"
#include "coloreditor.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ColorBar
//


//
// Constructor
//
ColorBar::ColorBar(const char *name, const ClipRect &rect, IControl *parent) :
  IControl(parent)
{
  // Clear internal data
  var = NULL;

  // Add dropshadow
//    controlStyle |= STYLE_DROPSHADOW;

  // Set control values
  SetName(name);
  SetPos(rect.p0.x, rect.p0.y);
  SetSize(rect.p1.x, rect.p1.y);
}


//
// HandleEvent
//
// Passes events through to the user
//
U32 ColorBar::HandleEvent(Event &e)
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
          Point<S32> mouse(e.input.mouseX, e.input.mouseY);
          if (InClient(mouse))
          {
            GetMouseCapture();

            if (var)
            {
              var->SetIntegerValue(ScreenToWindow(mouse).y);
            }
          }
        }
        break;
      }

      case Input::MOUSEMOVE:
      {
        if (HasMouseCapture()) //  || !IFace::GetCapture())
        {
          if (var)
          {
            Point<S32> mouse(e.input.mouseX, e.input.mouseY);
            var->SetIntegerValue(ScreenToWindow(mouse).y);
          }
          return (TRUE);
        }
        break;
      }

      case Input::MOUSEBUTTONUP:
      case Input::MOUSEBUTTONDBLCLKUP:
      {
        if (e.input.code == Input::LeftButtonCode())
        {
          if (HasMouseCapture())
          {
            ReleaseMouseCapture();
            return (TRUE);
          }
        }
        break;
      }
    }
  }

  // Allow parent class to process this event
  return IControl::HandleEvent(e);
}


//
// SetTopColor
//
// Set Top Color
//
void ColorBar::SetTopColor(const Color &color)
{
  // Set all colors
  topColor = color;
  middleColor = color;
  bottomColor = color;
}


//
// SetBottomColor
//
// Set Bottom Color
//
void ColorBar::SetBottomColor(const Color &color)
{
  bottomColor = color;

  // Set middle color to average color
  middleColor = Color(
    (S32) ((topColor.r + bottomColor.r) >> 1),
    (S32) ((topColor.g + bottomColor.g) >> 1),
    (S32) ((topColor.b + bottomColor.b) >> 1),
    (S32) ((topColor.a + bottomColor.a) >> 1));
}


//
// SetMiddleColor
//
// Set Middle Color
//
void ColorBar::SetMiddleColor(const Color &color)
{
  middleColor = color;
}


//
// SetVar
//
// Setup the var for this slider to use
//
void ColorBar::SetVar(IFaceVar *item)
{
  var = item;
}


//
// DrawSelf
//
void ColorBar::DrawSelf(PaintInfo &pi)
{
  // Draw a frame
  DrawCtrlFrame(pi);

  // Draw the graduated bar
  S32 half = (pi.client.p0.y + pi.client.p1.y) >> 1;
  IFace::RenderGradient(ClipRect(pi.client.p0, pi.client.p1.x, half), topColor, middleColor);
  IFace::RenderGradient(ClipRect(pi.client.p0.x, half, pi.client.p1), middleColor, bottomColor);

  // Draw a single line indicating the actual value
  if (var)
  {
    S32 val = var->GetIntegerValue();
    Color bar;

    if (val < 128)
    {
      bar.Set(255l, 255l, 255l, 255l);
    }
    else
    {
      bar.Set(0l, 0l, 0l, 255l);
    }

    IFace::RenderRectangle(ClipRect(pi.client.p0.x, pi.client.p0.y + val, pi.client.p1.x, pi.client.p0.y + val + 1), bar);
  }

  // Render the drop shadow
  IFace::RenderShadow(pi.client, pi.client + IFace::GetMetric(IFace::DROPSHADOW_UP), Color(0, 0, 0, IFace::GetMetric(IFace::SHADOW_ALPHA)), 0);
}


///////////////////////////////////////////////////////////////////////////////
//
// Class ColorMap
//


//
// Constructor
//
ColorMap::ColorMap(const char *name, const Point<S32> &point, IControl *parent) :
  IControl(parent)
{
  // Add dropshadow
//    controlStyle |= STYLE_DROPSHADOW;

  // Clear internal data
  hueVar = NULL;
  saturationVar = NULL;

  // Set control values
  SetName(name);
  SetPos(point.x, point.y);
  SetSize(360, 256);
}


//
// HandleEvent
//
// Passes events through to the user
//
U32 ColorMap::HandleEvent(Event &e)
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
          Point<S32> mouse(e.input.mouseX, e.input.mouseY);
          if (InClient(mouse))
          {
            GetMouseCapture();

            if (hueVar)
            {
              hueVar->SetIntegerValue(ScreenToWindow(mouse).x);
            }
            if (saturationVar)
            {
              saturationVar->SetIntegerValue(255 - ScreenToWindow(mouse).y);
            }
          }
        }
        break;
      }

      case Input::MOUSEMOVE:
      {
        if (HasMouseCapture()) //  || !IFace::GetCapture())
        {
          Point<S32> mouse(e.input.mouseX, e.input.mouseY);
          if (hueVar)
          {
            hueVar->SetIntegerValue(ScreenToWindow(mouse).x);
          }
          if (saturationVar)
          {
            saturationVar->SetIntegerValue(255 - ScreenToWindow(mouse).y);
          }
          return (TRUE);
        }
        break;
      }

      case Input::MOUSEBUTTONUP:
      case Input::MOUSEBUTTONDBLCLKUP:
      {
        if (e.input.code == Input::LeftButtonCode())
        {
          if (HasMouseCapture())
          {
            ReleaseMouseCapture();
            return (TRUE);
          }
        }
        break;
      }
    }
  }

  // Allow parent class to process this event
  return IControl::HandleEvent(e);
}


//
// SetHueVar
//
void ColorMap::SetHueVar(IFaceVar *item)
{
  hueVar = item;
}


//
// SetSaturationVar
//
void ColorMap::SetSaturationVar(IFaceVar *item)
{
  saturationVar = item;
}


//
// SetLuminosityVar
//
void ColorMap::SetLuminosityVar(IFaceVar *item)
{
  luminosityVar = item;
}


//
// DrawSelf
//
void ColorMap::DrawSelf(PaintInfo &pi)
{
  // Draw a frame
  DrawCtrlFrame(pi);

  // Draw the hue/saturation map
  Point<S32> point = pi.client.p0;

  // Set the colors of each of the bars before they are drawn
  Color color1;
  Color color2;

  // Draw Hue first
  CalculateRGB(color1, 0, 255, luminosityVar->GetIntegerValue());
  CalculateRGB(color2, 59, 255, luminosityVar->GetIntegerValue());
  IFace::RenderGradient(
    ClipRect(point, point + Point<S32>(60, 256)), color1, color2, FALSE);
  point += Point<S32>(60, 0);

  CalculateRGB(color1, 60, 255, luminosityVar->GetIntegerValue());
  CalculateRGB(color2, 119, 255, luminosityVar->GetIntegerValue());
  IFace::RenderGradient(
    ClipRect(point, point + Point<S32>(60, 256)), color1, color2, FALSE);
  point += Point<S32>(60, 0);

  CalculateRGB(color1, 120, 255, luminosityVar->GetIntegerValue());
  CalculateRGB(color2, 179, 255, luminosityVar->GetIntegerValue());
  IFace::RenderGradient(
    ClipRect(point, point + Point<S32>(60, 256)), color1, color2, FALSE);
  point += Point<S32>(60, 0);

  CalculateRGB(color1, 180, 255, luminosityVar->GetIntegerValue());
  CalculateRGB(color2, 239, 255, luminosityVar->GetIntegerValue());
  IFace::RenderGradient(
    ClipRect(point, point + Point<S32>(60, 256)), color1, color2, FALSE);
  point += Point<S32>(60, 0);

  CalculateRGB(color1, 240, 255, luminosityVar->GetIntegerValue());
  CalculateRGB(color2, 299, 255, luminosityVar->GetIntegerValue());
  IFace::RenderGradient(
    ClipRect(point, point + Point<S32>(60, 256)), color1, color2, FALSE);
  point += Point<S32>(60, 0);

  CalculateRGB(color1, 300, 255, luminosityVar->GetIntegerValue());
  CalculateRGB(color2, 359, 255, luminosityVar->GetIntegerValue());
  IFace::RenderGradient(
    ClipRect(point, point + Point<S32>(60, 256)), color1, color2, FALSE);
  point += Point<S32>(60, 0);

  // Overlay the saturation
  CalculateRGB(color2, 0, 0, luminosityVar->GetIntegerValue());
  color1.Set(color2.R(), color2.G(), color2.B(), 0.0f);
  IFace::RenderGradient(
    ClipRect(pi.client.p0, pi.client.p0 + Point<S32>(360, 256)), color1, color2);

  // Draw the hue and saturation
  if (hueVar)
  {
    S32 val = hueVar->GetIntegerValue();
    IFace::RenderRectangle(ClipRect(pi.client.p0.x + val, pi.client.p0.y, pi.client.p0.x + val + 1, pi.client.p1.y), Color(255l, 255l, 255l, 255l));
  }
  if (saturationVar)
  {
    S32 val = 255 - saturationVar->GetIntegerValue();
    IFace::RenderRectangle(ClipRect(pi.client.p0.x, pi.client.p0.y + val, pi.client.p1.x, pi.client.p0.y + val + 1), Color(255l, 255l, 255l, 255l));
  }

  // Render the drop shadow
  IFace::RenderShadow(pi.client, pi.client + IFace::GetMetric(IFace::DROPSHADOW_UP), Color(0, 0, 0, IFace::GetMetric(IFace::SHADOW_ALPHA)), 0);
}


///////////////////////////////////////////////////////////////////////////////
//
// Class ColorEditor
//

//
// Constructor
//
ColorEditor::ColorEditor(IControl *parent) : ICWindow(parent)
{
  // Clear color var
  color = NULL;

  // Create local vars
  hue = new IFaceVar(this, CreateInteger("Hue", 0, 0, 359));
  luminosity = new IFaceVar(this, CreateInteger("Luminosity", 0, 0, 255));
  saturation = new IFaceVar(this, CreateInteger("Saturation", 127, 0, 255));
  red = new IFaceVar(this, CreateInteger("Red", 0, 0, 255));
  green = new IFaceVar(this, CreateInteger("Green", 0, 0, 255));
  blue = new IFaceVar(this, CreateInteger("Blue", 0, 0, 255));

  // Create color bars
  hueSatMap = new ColorMap("HueSaturation", Point<S32>(110, 5), this);
  lumBar = new ColorBar("Luminosity", ClipRect(480, 5, 20, 256), this);
  redBar = new ColorBar("Red", ClipRect(5, 5, 20, 256), this);
  greenBar = new ColorBar("Green", ClipRect(35, 5, 20, 256), this);
  blueBar = new ColorBar("Blue", ClipRect(65, 5, 20, 256), this);

  colorDisp = new ColorBar("ColorDisp", ClipRect(300, 286, 110, 35), this);
  redDisp = new ColorBar("RedDisp", ClipRect(5, 271, 20, 50), this);
  greenDisp = new ColorBar("GreenDisp", ClipRect(35, 271, 20, 50), this);
  blueDisp = new ColorBar("BlueDisp", ClipRect(65, 271, 20, 50), this);

  // Setup the connections between vars and color bars
  hueSatMap->SetHueVar(hue);
  hueSatMap->SetSaturationVar(saturation);
  hueSatMap->SetLuminosityVar(luminosity);
  lumBar->SetVar(luminosity);
  redBar->SetVar(red);
  greenBar->SetVar(green);
  blueBar->SetVar(blue);
}


//
// Destructor
//
ColorEditor::~ColorEditor()
{
  // Delete local vars
  delete hue;
  delete luminosity;
  delete saturation;
  delete red;
  delete green;
  delete blue;
}


//
// Notify
//
// Notification that a local var has changed value
//
void ColorEditor::Notify(IFaceVar *var)
{
  static flag = FALSE;

  // Prevent recursion
  if (!flag)
  {
    flag = TRUE;
    
    if (var == hue || var == luminosity || var == saturation)
    {
      // Recalculate the values for RGB from HLS
      S32 r, g, b;
      CalculateRGB(r, g, b, hue->GetIntegerValue(), saturation->GetIntegerValue(), luminosity->GetIntegerValue());
      red->SetIntegerValue(r);
      green->SetIntegerValue(g);
      blue->SetIntegerValue(b);
    }
    else
    {
      // Recalculate the values for HLS from RGB
      S32 h, s, l;
      CalculateHLS(red->GetIntegerValue(), green->GetIntegerValue(), blue->GetIntegerValue(), h, s, l);
      hue->SetIntegerValue(h);
      saturation->SetIntegerValue(s);
      luminosity->SetIntegerValue(l);
    }

    flag = FALSE;
  }
}


//
// HandleEvent
//
// Pass any events to the registered handler
//
U32 ColorEditor::HandleEvent(Event &e)
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
          case 0xA4C4F136: // "OK"
            if (color)
            {
              Color c(red->GetIntegerValue(), green->GetIntegerValue(), blue->GetIntegerValue());
              color->SetIntegerValue(c);
            }
            IFace::Deactivate(this);
            color = NULL;
            break;

          case 0x3E412225: // "Cancel"
            IFace::Deactivate(this);
            color = NULL;
            break;

          default : 
            ICWindow::HandleEvent(e);
            break;
        }

        return (TRUE);
      }
    }
  }

  return (ICWindow::HandleEvent(e));  
}



//
// DrawSelf
//
// Control draws itself
//
void ColorEditor::DrawSelf(PaintInfo &pi)
{
  ICWindow::DrawSelf(pi);

  // Set the colors of each of the bars before they are drawn
  S32 r, g, b;
  CalculateRGB(r, g, b, hue->GetIntegerValue(), saturation->GetIntegerValue(), 0);
  lumBar->SetTopColor(Color(r, g, b, 255));

  // Calculate the RGB values for Full Luminosity
  CalculateRGB(r, g, b, hue->GetIntegerValue(), saturation->GetIntegerValue(), 255);
  lumBar->SetBottomColor(Color(r, g, b, 255));

  // Calculate the RGB values for Half Luminsotiy
  CalculateRGB(r, g, b, hue->GetIntegerValue(), saturation->GetIntegerValue(), 127);
  lumBar->SetMiddleColor(Color(r, g, b, 255));

  // Set the red bar
  redBar->SetTopColor(Color(0, green->GetIntegerValue(), blue->GetIntegerValue()));
  redBar->SetBottomColor(Color(255, green->GetIntegerValue(), blue->GetIntegerValue()));

  // Set the green bar
  greenBar->SetTopColor(Color(red->GetIntegerValue(), 0, blue->GetIntegerValue()));
  greenBar->SetBottomColor(Color(red->GetIntegerValue(), 255, blue->GetIntegerValue()));

  // Set the blue bar
  blueBar->SetTopColor(Color(red->GetIntegerValue(), green->GetIntegerValue(), 0));
  blueBar->SetBottomColor(Color(red->GetIntegerValue(), green->GetIntegerValue(), 255));

  // Set the color display bar
  colorDisp->SetTopColor(Color(red->GetIntegerValue(), green->GetIntegerValue(), blue->GetIntegerValue()));

  // Set the red display bar
  redDisp->SetTopColor(Color(red->GetIntegerValue(), 0, 0));

  // Set the green display bar
  greenDisp->SetTopColor(Color(0, green->GetIntegerValue(), 0));

  // Set the blue display bar
  blueDisp->SetTopColor(Color(0, 0, blue->GetIntegerValue()));

}


//
// Activate
//
// Activate this control
//
Bool ColorEditor::Activate()
{
  if (ICWindow::Activate())
  {
    ActivateVar(hue);
    ActivateVar(luminosity);
    ActivateVar(saturation);
    ActivateVar(red);
    ActivateVar(green);
    ActivateVar(blue);
    ActivateVar(color, VarSys::VI_INTEGER);

    Color c(color->GetIntegerValue());
    red->SetIntegerValue(c.r);
    green->SetIntegerValue(c.g);
    blue->SetIntegerValue(c.b);

    return (TRUE);
  }

  return (FALSE);
}


//
// Deactivate
//
// Deactivate this control
//
Bool ColorEditor::Deactivate()
{
  if (ICWindow::Deactivate())
  {
    hue->Deactivate();
    luminosity->Deactivate();
    saturation->Deactivate();
    red->Deactivate();
    green->Deactivate();
    blue->Deactivate();
    color->Deactivate();
    color = NULL;
    return (TRUE);
  }

  return (FALSE);
}

//
// SetColorVar
//
// Set the var to be used for color
//
void ColorEditor::SetColorVar(IFaceVar *colorIn)
{
  color = colorIn;
/*
  Color c(color->GetIntegerValue());
  red->SetIntegerValue(c.r);
  green->SetIntegerValue(c.g);
  blue->SetIntegerValue(c.b);
*/
}


