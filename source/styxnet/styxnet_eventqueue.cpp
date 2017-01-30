////////////////////////////////////////////////////////////////////////////////
//
// StyxNet
//
// Copyright 1999-2000
// Matthew Versluys
//


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "styxnet_eventqueue.h"


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


  //
  // EventQueue::~EventQueue
  //
  EventQueue::~EventQueue()
  {
    CRC event;
    Event::Data *data;
    while (GetEvent(event, data))
    {
      if (data)
      {
        delete [] data;
      }
    }
  }


  //
  // EventQueue::GetEvent
  //
  // Get an event from the server
  //
  Bool EventQueue::GetEvent(CRC &event, Event::Data *&data)
  {
    // Are there any events in the event queue ?
    if (Event *e = events.RemovePre(0))
    {
      // We got one
      event = e->message;
      data = e->data;
      events.RemovePost();
      return (TRUE);
    }
    else
    {
      // There were no messages
      return (FALSE);
    }
  }


  //
  // EventQueue::SendEvent
  //
  // Send an event
  //
  void EventQueue::SendEvent(CRC message, Event::Data *data)
  {
    Event *e = events.AddPre();
    e->message = message;
    e->data = data;
    events.AddPost();
  }

}

