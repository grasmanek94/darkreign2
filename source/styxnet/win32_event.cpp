///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1999
// Matthew Versluys
//
// Win32 Event
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "win32_event.h"


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


  //
  // Event
  //
  // Create a new event
  //
  Event::Event(const char *name)
  {
    event = CreateEvent(NULL, FALSE, FALSE, name);
    primary = name && (GetLastError() == ERROR_ALREADY_EXISTS) ? FALSE : TRUE;
  }

  
  //
  // ~Event
  //
  // Destroy Event
  //
  Event::~Event()
  {
    CloseHandle(event);
  }


  //
  // GetHandle
  //
  // Returns the event handle if you need it
  //
  HANDLE Event::GetHandle() const
  {
    return (event);
  }


  //
  // Wait
  //
  // Wait for event to become signaled (with optional timeout)
  //
  Bool Event::Wait(U32 timeout)
  {
    return ((WaitForSingleObject(event, timeout) == WAIT_OBJECT_0) ? TRUE : FALSE);
  }

  
  //
  // Signal
  //
  // Signal event
  //
  void Event::Signal()
  {
    SetEvent(event);
  }


  //
  // Pulse
  //
  // Pulse the event
  //
  void Event::Pulse()
  {
    PulseEvent(event);
  }


  //
  // IsPrimary
  //
  // Test to see if this is the primary event (only applies to named event)
  //
  Bool Event::IsPrimary()
  {
    return (primary);
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class EventIndex
  //
  EventIndex::EventIndex(const char *name)
  : Event(name)
  {
  }


}

