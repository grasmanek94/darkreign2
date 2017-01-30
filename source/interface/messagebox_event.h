///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//


#ifndef __MESSAGEBOX_EVENT_H
#define __MESSAGEBOX_EVENT_H


#include "dtrack.h"
#include "utiltypes.h"


//
// Forward declarations
//
class IControl;


///////////////////////////////////////////////////////////////////////////////
//
// class MBEvent
//

class MBEvent
{
public:

  // Identifier of this event
  GameIdent ident;

  // Caption of this message box button
  CH *caption;

  // NList node
  NBinTree<MBEvent>::Node listNode;

public:

  // Constructor
  MBEvent(const char *ident, const CH *caption);

  // Destructor
  virtual ~MBEvent();

  // Process the callback when the messagebox item is pressed
  virtual void Process() { };

};


///////////////////////////////////////////////////////////////////////////////
//
// class MBEventNotify - Sends a notification event when message box button is pressed
//

class MBEventNotify : public MBEvent
{
protected:

  // Control to send the event to
  Reaper<IControl> ctrl;

  // Id of the notification event
  U32 notifyId;

public:

  MBEventNotify(const char *ident, const CH *caption, IControl *control, U32 notifyId);

  // Process the callback when the messagebox item is pressed
  void Process();

};


///////////////////////////////////////////////////////////////////////////////
//
// class MBEventCallback - Calls the callback function when message box button is pressed
//

class MBEventCallback : public MBEvent
{
public:

  typedef void (Proc)(U32 event, U32 context);
  U32 context;

protected:

  // Function to call
  Proc *proc;

public:

  MBEventCallback(const char *ident, const CH *caption, Proc *proc, U32 context = 0);

  // Process the callback when the messagebox item is pressed
  void Process();

};

#endif
