/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Ticker Text Control
//
// 29-DEC-1998
//


#ifndef __ICTICKER_H
#define __ICTICKER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"
#include "varsys.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ICTicker
//
class ICTicker : public IControl
{
  PROMOTE_LINK(ICTicker, IControl, 0x1F8FDFCF); // "ICTicker"

public:

  enum Direction
  {
    DIR_LEFT,
    DIR_RIGHT,
    DIR_UP,
    DIR_DOWN,
    DIR_ALPHA,
  };

protected:

  //
  // Struct Message
  //
  struct Message
  {
    // Text to be displayed
    CH *text;

    // Length of the text
    U32 length;

    // Direction to scroll
    Direction direction;

    // Initializing Constructor
    Message(const CH *text, U32 length, Direction direction) :
      text(Utils::Strdup(text)),
      length(length),
      direction(direction)
    {
    }

    // Copy Constructor
    Message(const Message &message) :
      text(Utils::Strdup(message.text)),
      length(message.length),
      direction(message.direction)
    {
    }

    // Destructor
    ~Message()
    {
      delete [] text;
    }
  };

  // Text to be displayed
  List<Message> messages;

  // Current text item
  List<Message>::Iterator messageIterator;

  // Copy of current item
  Message *currentMessage;

  // X Offset within current
  int offsetX;

  // Y Offset within current
  int offsetY;

  // Counter used for pauses
  U32 counter;

  // Alpha animation level
  S32 alpha;
  S32 alphaDir;

  // Speed of scrolling
  U32 speed;

  // Draw control
  void DrawSelf(PaintInfo &pi);

public:

  ICTicker(IControl *parent);
  ~ICTicker();

  // Setup this control using a 'DefineControl' function
  void Setup(FScope *fScope);

  // Activate the control
  Bool Activate();

  // Deactivate the control
  Bool Deactivate();

  // Event handler
  U32 HandleEvent(Event &e);

  // Clear the ticker
  void Clear();

  // Add text to the ticker
  void AddText(const CH *text, Direction direction);

  // Setup the next message
  void NextMessage();

  // Find a ICTicker control
  static ICTicker *FindTicker(const char *path);
};


//
// Type definitions
//
typedef Reaper<ICTicker> ICTickerPtr;


#endif
