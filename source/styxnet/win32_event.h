///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1999
// Matthew Versluys
//
// Win32 Event
//


#ifndef __WIN32_EVENT_H
#define __WIN32_EVENT_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "win32.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Win32
//
namespace Win32
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Event
  //
  class Event
  {
  private:

    // Event handle
    HANDLE event;

    // Is this the primary event (only relevent to named events)
    Bool primary;

  public:

    // Index this event is assigned (used by event lists)
    U32 index;

    // Create a new event
    Event(const char *name = NULL);

    // ~Event
    ~Event();

    // Returns the event handle if you need it
    HANDLE GetHandle() const;

    // Wait for event to become signaled (with optional timeout)
    Bool Wait(U32 timeout = INFINITE);

    // Signal event
    void Signal();

    // Pulse
    void Pulse();

    // IsPrimary
    Bool IsPrimary();

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class EventIndex
  //
  class EventIndex : public Event
  {
  private:

    // Index assigned to this event (for event lists)
    U32 Index;

    // Nlist node
    NList<EventIndex>::Node node;

  public:

    template <U32 MAX_EVENTS> class List;

    // Create a new event
    EventIndex(const char *name = NULL);

  public:

    friend class List;

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Template EventIndex::List
  //
  template <U32 MAX_EVENTS> class EventIndex::List
  {
  private:

    // Number of events
    U32 numEvents;

    // Event array
    HANDLE handles[MAX_EVENTS];

    // Event contexts
    void *contexts[MAX_EVENTS];

    // The actual events
    Event *events[MAX_EVENTS];

  public:

    // Constructor
    List()
    : numEvents(0)
    {
    }

    // Destructor
    ~List()
    {
      ClearEvents();
    }

    // Add an event to the list
    void AddEvent(Event &event, void *context)
    {
      ASSERT(numEvents < MAX_EVENTS)

      event.index = numEvents;
      handles[numEvents] = event.GetHandle();
      contexts[numEvents] = context;
      events[numEvents] = &event;
      numEvents++;
    }

    // Remove an event from the list
    void RemoveEvent(Event &event)
    {
      U32 e;
      for (e = event.index; e < (numEvents - 1); e++)
      {
        handles[e] = handles[e + 1];
        contexts[e] = contexts[e + 1];
        events[e] = events[e + 1];
        events[e]->index = e;
      }
      numEvents--;
    }

    // Clear the event list
    void ClearEvents()
    {
      numEvents = 0;
    }

    // Wait for multiple events
    Bool Wait(void *&context, Bool all = FALSE, U32 timeout = INFINITE)
    {
      U32 i = WaitForMultipleObjects(numEvents, handles, all, timeout);
      switch (i)
      {
        case WAIT_TIMEOUT: 
          context = NULL;
          return (FALSE);
          break;

        case WAIT_FAILED:
          context = NULL;
          return (FALSE);
          break;

        default:

          if (i >= WAIT_OBJECT_0 && i < WAIT_OBJECT_0 + numEvents)
          {
            context = contexts[i - WAIT_OBJECT_0];
            return (TRUE);
          }
          else
          if (i >= WAIT_ABANDONED_0 && i < WAIT_ABANDONED_0 + numEvents)
          {
            context = contexts[i - WAIT_ABANDONED_0];
            return (FALSE);
          }
          else
          {
       //   LERR("Got Unknown Result from WaitForMultipleObjects: " << i)
            return (FALSE);
          }
          break;
      }
    }
  };
}

#endif