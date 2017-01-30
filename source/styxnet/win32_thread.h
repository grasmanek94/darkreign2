///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1998
// Matthew Versluys
//
// Win32 Thread
//


#ifndef __WIN32_THREAD_H
#define __WIN32_THREAD_H


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
  // Class Thread
  //
  class Thread
  {
  private:

    // Thread ID
    U32 threadID;

    // Thread handle
    HANDLE threadHandle;

  public:

    enum Priority
    {
      // Indicates a base priority level of 15 for IDLE_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, or HIGH_PRIORITY_CLASS processes, and a base priority level of 31 for REALTIME_PRIORITY_CLASS processes.
      TIME_CRITICAL = THREAD_PRIORITY_TIME_CRITICAL,
      // Indicates 2 points above normal priority for the priority class.
      HIGHEST = THREAD_PRIORITY_HIGHEST,
      // Indicates 1 point above normal priority for the priority class.
      ABOVE_NORMAL = THREAD_PRIORITY_ABOVE_NORMAL,
      // Indicates normal priority for the priority class.
      NORMAL = THREAD_PRIORITY_NORMAL,
      // Indicates 1 point below normal priority for the priority class.
      BELOW_NORMAL = THREAD_PRIORITY_BELOW_NORMAL,
      // Indicates 2 points below normal priority for the priority class.
      LOWEST = THREAD_PRIORITY_LOWEST,
      // Indicates a base-priority level of 1 for IDLE_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, or HIGH_PRIORITY_CLASS processes, and a base-priority level of 16 for REALTIME_PRIORITY_CLASS processes.
      IDLE = THREAD_PRIORITY_IDLE
    };

  public:

    // Thread
    Thread();

    // Thread
    Thread(U32 (STDCALL *func)(void *), void *arg = NULL);

    // ~Thread
    ~Thread();

    // Start
    void Start(U32 (STDCALL *func)(void *), void *arg = NULL);

    // Stop
    void Stop();

    // Clear
    void Clear();

    // SetPriority
    void SetPriority(Priority priority);

    // GetPriority
    Priority GetPriority();

    // GetID
    U32 GetId();

    // IsAlive
    Bool IsAlive();

    // Suspend
    Bool Suspend();
    
    // Resume
    Bool Resume();

    // GetCurrntId
    static U32 GetCurrentId();

  };

}

#endif