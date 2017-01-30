////////////////////////////////////////////////////////////////////////////////
//
// StyxNet
//
// Copyright 1999-2000
// Matthew Versluys
//

#ifndef __STYXNET_EVENTQUEUE_H
#define __STYXNET_EVENTQUEUE_H


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "styxnet_private.h"
#include "queue.h"


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class EventQueue
  //
  class EventQueue
  {
  private:

    // Queue for slapping events into
    SafeQueue<Event, eventQueueSize> events;

  public:

    // Destructor
    virtual ~EventQueue();

    // Get events
    Bool GetEvent(CRC &event, Event::Data *&data);

    // Send an update event
    void SendEvent(CRC message, Event::Data *data = NULL);

  };

}

#endif
