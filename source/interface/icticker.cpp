/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Ticker Text Control
//
// 29-DEC-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icticker.h"
#include "iface.h"
#include "babel.h"
#include "iface_types.h"
#include "ifvar.h"
#include "input.h"
#include "perfstats.h"
#include "stdload.h"
#include "font.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ICTicker
//


//
// ICTicker::ICTicker
//
ICTicker::ICTicker(IControl *parent) :
  IControl(parent),
  speed(1),
  messageIterator(&messages),
  currentMessage(NULL)
{
  // Default style
  controlStyle |= IControl::STYLE_INERT;

  // Set the poll interval to 100x per second (more likely the frame rate)
  SetPollInterval(10);
}


//
// ICTicker::~ICTicker
//
ICTicker::~ICTicker()
{
  // Clear any messages
  Clear();

  // If there's a current message, delete it
  if (currentMessage)
  {
    delete currentMessage;
  }
}


//
// ICTicker::Activate
//
// Activate the control
//
Bool ICTicker::Activate()
{
  if (IControl::Activate())
  {
    // Ensure that the first message is setup correctly
    messageIterator.GoToHead();
    NextMessage();

    return (TRUE);
  }
  else
  {
    return (FALSE);
  }
}


//
// ICTicker::Deactivate
//
// Deactivate the control
//
Bool ICTicker::Deactivate()
{
  if (IControl::Deactivate())
  {
    return (TRUE);
  }
  else
  {
    return (FALSE);
  }
}


//
// ICTicker::Setup
//
// setup this control using a 'DefineControl' function
//
void ICTicker::Setup(FScope *fScope)
{
  switch (fScope->NameCrc())
  {
    case 0x2B96BEE1: // "Speed"
      speed = StdLoad::TypeU32(fScope);
      break;

    case 0xC9FCFE2A: // "Clear"
      Clear();
      break;

    case 0x6FEF8A3E: // "AddText"
    {
      const CH *text = TRANSLATE((StdLoad::TypeString(fScope)));

      // Get the optional argument 
      // (why is this MUCH more complicated than its needs to be?)
      VNode *vNode = fScope->NextArgument(VNode::AT_STRING, FALSE);
      const char *dir = "Up";
      if (vNode)
      {
        dir = vNode->GetString();
      }
      
      switch (Crc::CalcStr(dir))
      {
        case 0xBA190163: // "Left"
          AddText(text, DIR_LEFT);
          break;

        case 0xE2DDD72B: // "Right"
          AddText(text, DIR_RIGHT);
          break;

        case 0xF975A769: // "Up"
          AddText(text, DIR_UP);
          break;

        case 0xEF54F336: // "Down"
          AddText(text, DIR_DOWN);
          break;

        case 0x59D598FC: // "Alpha"
          AddText(text, DIR_ALPHA);
          break;

        default:
          fScope->ScopeError("Unkown Direction '%s'", dir);
          break;
      }
      break;
    }

    default:
    {
      IControl::Setup(fScope);
      break;
    }
  }
}


//
// ICTicker::DrawSelf
//
// Draw the control
//
void ICTicker::DrawSelf(PaintInfo &pi)
{
  // Draw background
  DrawCtrlBackground(pi, GetTexture());

  // Draw frame
  DrawCtrlFrame(pi);

  // If there is a font then draw the control's text
  if (pi.font)
  {
    // Is there any text to be drawn
    if (currentMessage)
    {
      Color c = pi.colors->fg[ColorIndex()];
      c.a = U8(alpha);

      // Display the text
      pi.font->Draw
      (
        pi.client.p0.x + offsetX, 
        pi.client.p0.y + offsetY, 
        currentMessage->text, 
        currentMessage->length, 
        c,
        &pi.client
      );

      // Calcalate the width of the text
      //int width = pi.font->Width(currentMessage->text, currentMessage->length);
      
      switch (currentMessage->direction)
      {
        /*
        case DIR_LEFT:
          // Pause in the middle
          if (counter && offsetX <= ((pi.client.Width() - width) / 2))
          {
            counter--;
            break;
          }
           
          offsetX -= speed;
          if (offsetX < -width)
          {
            NextMessage();
          }
          break;

        case DIR_RIGHT:
          // Pause in the middle
          if (counter && offsetX >= ((pi.client.Width() - width) / 2))
          {
            counter--;
            break;
          }

          offsetX += speed;
          if (offsetX > pi.client.Width())
          {
            NextMessage();
          }
          break;

        case DIR_UP:
          // Pause in the middle
          if (counter && offsetY <= ((pi.client.Height() - S32(pi.font->Height())) / 2))
          {
            counter--;
            break;
          }

          offsetY -= speed;
          if (offsetY < -pi.font->Height())
          {
            NextMessage();
          }
          break;

        case DIR_DOWN:
          // Pause in the middle
          if (counter && offsetY >= ((pi.client.Height() - S32(pi.font->Height())) / 2))
          {
            counter--;
            break;
          }

          offsetY += speed;
          if (offsetY > pi.client.Height())
          {
            NextMessage();
          }
          break;
      */
        case DIR_ALPHA:
        default:
        {
          // Pause in the middle
          if (counter && alpha >= 255)
          {
            counter--;
            alphaDir = -S32(speed);
            break;
          }

          alpha += alphaDir;
          if (alpha > 255)
          {
            alpha = 255;
          }

          if (alpha <= 0)
          {
            NextMessage();
          }
          break;
        }
      }
    }
  }
}


//
// ICTicker::HandleEvent
//
U32 ICTicker::HandleEvent(Event &e)
{
  // Pass all input events to the parent
  if (e.type == Input::EventID())
  {
    if (parent)
    {
      return parent->HandleEvent(e);
    }
  }

  // Allow IControl class to process this event
  return IControl::HandleEvent(e);
}


//
// ICTicker::Clear
//
// Clear the ticker
//
void ICTicker::Clear()
{
  // Destroy all messages
  messages.DisposeAll();

  // Reset the iterator
  messageIterator.GoToHead();
}


//
// ICTicker::AddText
//
// Add text to the ticker
//
void ICTicker::AddText(const CH *text, Direction direction)
{
  // Append the message
  messages.Append(new Message(text, Utils::Strlen(text), direction));

  // Reset the iterator
  messageIterator.GoToHead();
}


//
// ICTicker::NextMessage
//
// Setup the next message
//
void ICTicker::NextMessage()
{
  // If we have a current message ... delete it
  if (currentMessage)
  {
    delete currentMessage;
    currentMessage = NULL;
  }

  // Is the iterator on a message ?
  if (!*messageIterator)
  {
    // Reset the iterator and see if theres anything
    messageIterator.GoToHead();

    if (!*messageIterator)
    {
      // No messages in the list
      return;
    }
  }

  // Make a copy of the current message
  currentMessage = new Message(**messageIterator);

  // Initialize the offsets bassed upon the direction of travel
  switch (currentMessage->direction)
  {
    /*
    case DIR_LEFT:
      offsetX = paintInfo.client.Width();
      offsetY = (paintInfo.client.Height() - paintInfo.font->Height()) / 2;
      counter = 0;
      break;

    case DIR_RIGHT:
      offsetX = - (int) paintInfo.font->Width(currentMessage->text, currentMessage->length);
      offsetY = (paintInfo.client.Height() - paintInfo.font->Height()) / 2;
      counter = 0;
      break;

    case DIR_UP:
      offsetX = (paintInfo.client.Width() - paintInfo.font->Width(currentMessage->text, currentMessage->length)) / 2;
      offsetY = paintInfo.client.Height();
      counter = 100;
      break;

    case DIR_DOWN:
      offsetX = (paintInfo.client.Width() - paintInfo.font->Width(currentMessage->text, currentMessage->length)) / 2;
      offsetY = -S32(paintInfo.font->Height());
      counter = 100;
      break;
    */

    case DIR_LEFT:
    case DIR_RIGHT:
    case DIR_UP:
    case DIR_DOWN:
    case DIR_ALPHA:
      offsetX = (paintInfo.client.Width() - paintInfo.font->Width(currentMessage->text, currentMessage->length)) / 2;
      offsetY = (paintInfo.client.Height() - paintInfo.font->Height()) / 2;
      counter = 100;
      alpha = 0;
      alphaDir = speed;
      break;

    default:
      ERR_FATAL(("Unkown direction %d", currentMessage->direction))
      break;
  }

  // Set the iterator to the next message
  messageIterator++;
}


//
// ICTicker::FindTicker
//
// Find a ICTicker control
//
ICTicker *ICTicker::FindTicker(const char *path)
{
  IControl *ctrl = IFace::FindByName(path);

  // This currently does not check for correct type
  if (ctrl && ctrl->DerivedFrom(ICTicker::ClassId()))
  {
    return (ICTicker *)ctrl;
  }

  return NULL;
}
