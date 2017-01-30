/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//

#include "icsystembutton.h"
#include "iface.h"
#include "iface_util.h"
#include "iface_types.h"
#include "icdroplist.h"
#include "fontsys.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ICButton - Standard button control
//

//
// ICButton::ICButton
//
ICSystemButton::ICSystemButton(Function func, IControl *parent) 
: function(func), 
  ICButton(parent)
{
  // Default button style
  sysButtonStyle = STYLE_CODEDRAWN;

  // Default control style
  controlStyle |= STYLE_DROPSHADOW;

  // Disable tab stop
  controlStyle &= ~STYLE_TABSTOP;

  // Set up the event notification
  U32 event = ICButtonNotify::Pressing;

  switch (function)
  {
    case CLOSE:
      AddEventTranslation(ICButtonNotify::Pressed, ICWindowMsg::Close, parent);
      break;

    case HELP:
      AddEventTranslation(ICButtonNotify::Pressed, ICWindowMsg::Help, parent);
      break;

    case SLIDER_LEFT:
    case SLIDER_UP:
      AddEventTranslation(event, ICSliderMsg::Decrement, parent);
      break;

    case SLIDER_RIGHT:
    case SLIDER_DOWN:
      AddEventTranslation(event, ICSliderMsg::Increment, parent);
      break;

    case MINIMIZE:
      AddEventTranslation(event, ICWindowMsg::Minimize, parent);
      break;

    case MAXIMIZE:
      AddEventTranslation(event, ICWindowMsg::Maximize, parent);
      break;

    case DROPLIST:
      AddEventTranslation(event, ICDropListMsg::Drop, parent);
      break;
  }
}


//
// ICSystemButton::Poll
//
// Poll callback
//
void ICSystemButton::Poll()
{
  if ((pollDelay += pollInterval) < IFace::GetMetric(IFace::POLL_DELAY))
  {
    return;
  }

  switch (function)
  {
    case SLIDER_LEFT:
    case SLIDER_UP:
    case SLIDER_RIGHT:
    case SLIDER_DOWN:
    {
      // Generate a notification
      SendNotify(this, ICButtonNotify::Pressing, TRUE);
      return;
    }
  }
}


//
// ICSystemButton::Activate
//
// Activation
//
Bool ICSystemButton::Activate()
{
  if (ICButton::Activate())
  {
    U32 fontId =  IFace::GetMetric(IFace::TITLE_FONT);
    Font *font = FontSys::GetFont(fontId);

    if (font == NULL)
    {
      ERR_FATAL(("Title font not found"));
    }
    return (TRUE);
  }
  return (FALSE);
}


//
// ICSystemButton::SetStyleItem
//
// Change a style setting
//
Bool ICSystemButton::SetStyleItem(const char *s, Bool toggle)
{
  U32 style;

  switch (Crc::CalcStr(s))
  {
    case 0xE0A8897B: // "CodeDrawn"
      style = STYLE_CODEDRAWN;
      break;

    default:
      return ICButton::SetStyleItem(s, toggle);
  }

  // Toggle the style
  sysButtonStyle = (toggle) ? (sysButtonStyle | style) : (sysButtonStyle & ~style);

  return (TRUE);
}


//
// ICSystemButton::HandleEvent
//
// Event handling
//
U32 ICSystemButton::HandleEvent(Event &e)
{
  if (e.type == IFace::EventID())
  {
    // Interface events
    switch (e.subType)
    {
      case IFace::GOTCAPTURE:
      {
        if (!pollNode.InUse())
        {
          // Make it poll
          pollInterval = 50;
          pollDelay = 0;
          AddToPollList();
        }
        break;
      }

      case IFace::LOSTCAPTURE:
      {
        if (pollNode.InUse())
        {
          // Stop it from polling
          RemoveFromPollList();
          pollInterval = 0;
        }
        break;
      }

    }
  }

  return (ICButton::HandleEvent(e));
}


//
// ICSystemButton::DrawSelf
//
// Draw the button control into the specified bitmap
//
void ICSystemButton::DrawSelf(PaintInfo &pi)
{
  // Draw basic button style
  ICButton::DrawSelf(pi);

  // Draw code-drawn component
  if (sysButtonStyle & STYLE_CODEDRAWN)
  {
    // Halve the alpha
    Color c = pi.colors->fg[ColorIndex()];
    c.a >>= 1;

    // Make the client area square
    ClipRect r = pi.client;

    if (r.Width() > r.Height())
    {
      r.p0.x += (r.Width() - r.Height()) / 2;
      r.p1.x = r.p0.x + r.Height();
    }
    else
    if (r.Width() < r.Height())
    {
      r.p0.y += (r.Height() - r.Width()) / 2;
      r.p1.y = r.p0.y + r.Width();
    }

    // Draw custom icon
    switch (function)
    {
      case CLOSE:
        DrawCloseIcon(r, c);
        break;

      case HELP:
        DrawCharacterIcon(r, c, L'?');
        break;

      case SLIDER_LEFT:
        DrawLeftIcon(r, c);
        break;

      case SLIDER_RIGHT:
        DrawRightIcon(r, c);
        break;

      case SLIDER_UP:
        DrawUpIcon(r, c);
        break;

      case SLIDER_DOWN:
        DrawDownIcon(r, c);
        break;

      case DROPLIST:
        DrawDownIcon(r, c);
        break;
    }
  }
}


//
// Draw close icon
//
void ICSystemButton::DrawCloseIcon(const ClipRect &r, Color c)
{
  DrawDownIcon(r, c);
}


//
// ICSystemButton::DrawLeftIcon
//
void ICSystemButton::DrawLeftIcon(const ClipRect &r, Color c)
{
  Point<S32> pt[3];

  pt[0].Set(r.p0.x + 2, r.p0.y + (r.Height() >> 1));
  pt[1].Set(r.p1.x - 2, r.p0.y + 2);
  pt[2].Set(r.p1.x - 2, r.p1.y - 2);

  IFace::RenderTriangle(pt, c);
}


//
// ICSystemButton::DrawRightIcon
//
void ICSystemButton::DrawRightIcon(const ClipRect &r, Color c)
{
  Point<S32> pt[3];

  pt[0].Set(r.p1.x - 2, r.p0.y + (r.Height() >> 1));
  pt[1].Set(r.p0.x + 2, r.p0.y + 2);
  pt[2].Set(r.p0.x + 2, r.p1.y - 2);

  IFace::RenderTriangle(pt, c);
}


//
// ICSystemButton::DrawUpIcon
//
void ICSystemButton::DrawUpIcon(const ClipRect &r, Color c)
{
  Point<S32> pt[3];

  pt[0].Set(r.p0.x + (r.Width() >> 1), r.p0.y + 1);
  pt[1].Set(r.p0.x + 1, r.p1.y - 2);
  pt[2].Set(r.p1.x - 2, r.p1.y - 2);

  IFace::RenderTriangle(pt, c);
}


//
// ICSystemButton::DrawDownIcon
//
void ICSystemButton::DrawDownIcon(const ClipRect &r, Color c)
{
  Point<S32> pt[3];

  pt[0].Set(r.p0.x + (r.Width() >> 1), r.p1.y - 2);
  pt[1].Set(r.p0.x + 2, r.p0.y + 2);
  pt[2].Set(r.p1.x - 2, r.p0.y + 2);

  IFace::RenderTriangle(pt, c);
}


//
// Draw character icon
//
void ICSystemButton::DrawCharacterIcon(const ClipRect &r, Color c, CH ch)
{
  CH s[2];
  s[0] = ch;
  s[1] = L'\0';

  font->Draw(r.p0.x, r.p0.y, s, 1, c);
}
