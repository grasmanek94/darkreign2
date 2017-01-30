///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Console viewer
//
// 16-MAR-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "common_messagewindow.h"
#include "console.h"
#include "stdload.h"
#include "iface.h"
#include "font.h"
#include "iface_util.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Common
//
namespace Common
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct MessageWindow::Message
  //
  struct MessageWindow::Message
  {
    // The actual message
    CH *message;

    // Length of the message
    U32 length;

    // Message color
    Color color;

    // Time at which the message was added to the visible messages
    U32 startTime;

    // List Node
    NList<Message>::Node node;

    // Constructor and Destructor
    Message(const CH *message, U32 length, const Color &color);
    ~Message();

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class MessageWindow
  //


  //
  // MessageWindow::MessageWindow
  //
  // Constructor
  //
  MessageWindow::MessageWindow(IControl *parent) :
    IControl(parent),
    messages(&Message::node),
    incoming(&Message::node),
    incomingCharacters(0),
    typing(NULL),
    typingLength(0),
    typingSpeedMin(1),
    typingSpeedMax(10),
    messagesMax(10),
    fadeSpeed(10),
    lifeTime(5000)
  {
    controlStyle |= STYLE_INERT;
  }


  //
  // MessageWindow::MessageWindow
  //
  MessageWindow::~MessageWindow()
  {
    // Delete the filters
    filters.DisposeAll();

    // CleanUp
    CleanUp();
  }


  //
  // Setup
  //
  // Setup this control from one scope function
  //
  void MessageWindow::Setup(FScope *fScope)
  {
    switch (fScope->NameCrc())
    {
      case 0x5C5656F8: // "Filters"
      {
        List<GameIdent> idents;
        StdLoad::TypeStrCrcList(fScope, idents);
        for (List<GameIdent>::Iterator i(&idents); *i; i++)
        {
          filters.Add((*i)->crc);
        }
        idents.DisposeAll();
        break;
      }

      case 0xBC65BFCE: // "MessagesMax"
        messagesMax = StdLoad::TypeU32(fScope);
        break;

      case 0x8200CA7E: // "FadeSpeed"
        fadeSpeed = StdLoad::TypeU32(fScope);
        break;

      case 0x641F52F1: // "TypingSpeedMax"
        typingSpeedMax = StdLoad::TypeU32(fScope);
        break;

      case 0xB8C08761: // "TypingSpeedMin"
        typingSpeedMin = StdLoad::TypeU32(fScope);
        break;

      case 0x12CAD0FD: // "LifeTime"
        lifeTime = StdLoad::TypeU32(fScope);
        break;

      default:
        IControl::Setup(fScope);
        break;
    }
  }


  //
  // MessageWindow::DrawSelf
  //
  // Control draws itself
  //
  void MessageWindow::DrawSelf(PaintInfo &pi)
  {
    if (!pi.font)
    {
      ERR_CONFIG(("MessageWindow must have a font"))
    }

    // Draw all of the messages
    U32 yStep = pi.font->Height();
    U32 x = pi.client.p0.x;
    U32 y = pi.client.p0.y;

    // If there's no message being typed check the incoming messages
    if (!typing && incoming.GetCount())
    {
      // Pop the first item off the list and place it in the typing queue
      typing = incoming.GetHead();
      incoming.Unlink(typing);
      incomingCharacters -= typing->length;

      // Reset the typing length to 0
      typingLength = 0;
    }

    NList<Message>::Iterator m(&messages);
    Message *message;
    while ((message = m++) != NULL)
    {
      // Has the life of this message expired ?
      if (IFace::ElapsedMs() - message->startTime > lifeTime)
      {
        // Reduce Alpha
        if (message->color.a < fadeSpeed)
        {
          // Delete this item we won't be displaying it
          messages.Dispose(message);
          continue;
        }
        else
        {
          // Reduce the alpha
          message->color.a = U8(message->color.a - fadeSpeed);
        }
      }

      // Display the message
      pi.font->Draw(x + 1, y + 1, message->message, message->length, Color(0L, 0L, 0L, message->color.a), &pi.client);
      pi.font->Draw(x, y, message->message, message->length, message->color, &pi.client);

      // Move Down
      y += yStep;
    }

    // Is there a message being typed ?
    if (typing)
    {
      // Has the message being typed been completed ?
      if (typingLength >= typing->length)
      {
        // Display the message
        pi.font->Draw(x + 1, y + 1, typing->message, typing->length, Color(0L, 0L, 0L, typing->color.a), &pi.client);
        pi.font->Draw(x, y, typing->message, typing->length, typing->color, &pi.client);

        // Mark the time of the message and add it to the rest of the messages
        typing->startTime = IFace::ElapsedMs();
        messages.Append(typing);
        typing = NULL;

        // Does adding this message exceed the maximum number of messages ?
        if (messages.GetCount() > messagesMax)
        {
          // Delete the oldest message
          messages.Dispose(messages.GetHead());
        }
      }
      else
      {
        // Display the message being typed
        pi.font->Draw(x + 1, y + 1, typing->message, typingLength, Color(0.0f, 0.0f, 0.0f, 1.0f), &pi.client);
        pi.font->Draw(x, y, typing->message, typingLength, typing->color, &pi.client);

        // Work out the speed based on the number of incoming characters
        // We need to be able to display all of the remaining characters in the next 10 frames
        U32 speed = incomingCharacters / 10;
        speed = Clamp(typingSpeedMin, speed, typingSpeedMax);

        // Increment the typing length according to the typing speed
        typingLength += speed;

        // Move down
        y += yStep;
      }
    }
  }


  //
  // MessageWindow::Activate
  //
  Bool MessageWindow::Activate()
  {
    if (IControl::Activate())
    {
      BinTree<U32> *pf = filters.GetCount() ? &filters : NULL;

      // Register console notification hook
      Console::RegisterMsgHook(pf, ConsoleHook, this);

      return (TRUE);
    }
    else
    {
      return (FALSE);
    }
  }


  //
  // MessageWindow::Deactivate
  //
  Bool MessageWindow::Deactivate()
  {
    if (IControl::Deactivate())
    {
      // Unregister console notification hook
      Console::UnregisterMsgHook(ConsoleHook, this);

      // Clean up
      CleanUp();

      return (TRUE);
    }
    else
    {
      return (FALSE);
    }
  }


  //
  // MessageWindow::CleanUp
  //
  void MessageWindow::CleanUp()
  {
    // Delete all messages
    messages.DisposeAll();

    // Delete all incoming messages
    incoming.DisposeAll();
    incomingCharacters = 0;

    // Delete message being typed
    if (typing)
    {
      delete typing;
      typing = NULL;
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct MessageWindow::Message
  //

  //
  // Message::Message
  //
  // Constructor
  //
  MessageWindow::Message::Message(const CH *message, U32 length, const Color &color) :
    message(Utils::Strdup(message)),
    length(length),
    color(color),
    startTime(0)
  {
    // Force full alpha
    Message::color.a = 255;
  }


  //
  // Message::~Messsge
  //
  // Destructor
  //
  MessageWindow::Message::~Message()
  {
    // Delete the string
    delete message;
  }


  //
  // MessageWindow::ConsoleHook
  //
  Bool MessageWindow::ConsoleHook(const CH *text, U32 &type, void *context)
  {
    MessageWindow *window = (MessageWindow *) context;
    const CH *ptr = text;

    // Wrap the text into the window width
    do
    {
      ASSERT(window->paintInfo.client.Width() > 3)

      U32 length;
      const CH *next = IFace::BreakText(ptr, window->GetPaintInfo().font, window->paintInfo.client.Width() - 3, length);

      window->incomingCharacters += length;
      window->incoming.Append(new Message(ptr, length, IFace::GetConsoleColor(type)));
      ptr = next;
    }
    while (ptr);

    return (TRUE);
  }

}
