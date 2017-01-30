///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Console viewer
//
// 16-MAR-1999
//


#ifndef __COMMON_MESSAGEWINDOW_H
#define __COMMON_MESSAGEWINDOW_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Common
//
namespace Common
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class MessageWindow
  //
  class MessageWindow : public IControl
  {
    PROMOTE_LINK(MessageWindow, IControl, 0x1974C5DE) // "MessageWindow"

  protected:

    // Message structure
    struct Message;

    // Messages 
    NList<Message> messages;

    // Incoming messages
    NList<Message> incoming;

    // Characters incoming
    U32 incomingCharacters;

    // The message being typed
    Message *typing;

    // The current length of the message being types
    U32 typingLength;

    // Typing speed in characters per frame
    U32 typingSpeedMin;
    U32 typingSpeedMax;

    // Message filters
    BinTree<U32> filters;

    // Maximum number of messages
    U32 messagesMax;

    // Fade speed
    U32 fadeSpeed;

    // Life time of messages
    U32 lifeTime;

  public:

    // Constructor and Destructor
    MessageWindow(IControl *parent);
    ~MessageWindow();

    // Setup this control from one scope function
    void Setup(FScope *fScope);

    // Control draws itself
    void DrawSelf(PaintInfo &pi);

    // Activation
    Bool Activate();

    // Deactivate
    Bool Deactivate();

    // CleanUp
    void CleanUp();

    // Console Hook
    static Bool ConsoleHook(const CH *text, U32 &type, void *context);

  };

}

#endif
