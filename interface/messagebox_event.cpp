///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//


#include "messagebox_event.h"
#include "iface.h"
#include "iface_types.h"


//
// Constructor
//
MBEvent::MBEvent(const char *ident, const CH *caption)
: ident(ident)
{
  // Duplicate the caption
  this->caption = Utils::Strdup(caption);
}


//
// Destructor
//
MBEvent::~MBEvent()
{
  // Delete the caption
  delete[] caption;
  caption = NULL;
}


//
// MBEventNotify::MBEventNotify
//
MBEventNotify::MBEventNotify(const char *ident, const CH *caption, IControl *control, U32 notifyId) 
: MBEvent(ident, caption),
  ctrl(control), 
  notifyId(notifyId)
{
}


//
// MBEventNotify::Process
//
// Called when a MessageBox item is pressed
//
void MBEventNotify::Process()
{
  if (ctrl.Alive())
  {
    // Control is still alive so send it the event
    IFace::SendEvent(ctrl, NULL, IFace::NOTIFY, notifyId);
  }
}


//
// MBEventCallback::MBEventCallback
//
MBEventCallback::MBEventCallback(const char *ident, const CH *caption, Proc *proc, U32 context)
: MBEvent(ident, caption),
  proc(proc),
  context(context)
{
  ASSERT(proc)
}


//
// Process the callback when the messagebox item is pressed
//
void MBEventCallback::Process()
{
  proc(ident.crc, context);
}
