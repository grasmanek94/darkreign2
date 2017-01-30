///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// System Utilities
// 23-NOV-1997
//


#ifndef __SYSTEM_H
#define __SYSTEM_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#pragma pack(push, _save_system_h_)
#include <process.h>
#pragma pack(pop, _save_system_h_)


///////////////////////////////////////////////////////////////////////////////
//
// Class System
//
class System
{
public:

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Thread
  //
  // Thread management
  //
  class Thread
  {
  private:
    U32         threadid;
    HANDLE      threadh;

  public:

    enum Priority
    {
      TIME_CRITICAL = THREAD_PRIORITY_TIME_CRITICAL, // Indicates a base priority level of 15 for IDLE_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, or HIGH_PRIORITY_CLASS processes, and a base priority level of 31 for REALTIME_PRIORITY_CLASS processes.
      HIGHEST       = THREAD_PRIORITY_HIGHEST,       // Indicates 2 points above normal priority for the priority class.
      ABOVE_NORMAL  = THREAD_PRIORITY_ABOVE_NORMAL,  // Indicates 1 point above normal priority for the priority class.
      NORMAL        = THREAD_PRIORITY_NORMAL,	       // Indicates normal priority for the priority class.
      BELOW_NORMAL  = THREAD_PRIORITY_BELOW_NORMAL,  // Indicates 1 point below normal priority for the priority class.
      LOWEST        = THREAD_PRIORITY_LOWEST,        // Indicates 2 points below normal priority for the priority class.
      IDLE          = THREAD_PRIORITY_IDLE           // Indicates a base-priority level of 1 for IDLE_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, or HIGH_PRIORITY_CLASS processes, and a base-priority level of 16 for REALTIME_PRIORITY_CLASS processes.
    };

    //
    // Thread
    // 
    // Creates a new thread
    //
    Thread(U32 (STDCALL *func)(void *), void *arg = NULL)
    {
      threadh = CreateThread(NULL, 0, func, arg, 0, &threadid);
    }

    //
    // Start
    // 
    static void Start(U32 (STDCALL *func)(void *), void *arg = NULL)
    {
      U32 threadid;
      CreateThread(NULL, 0, func, arg, 0, &threadid);
    }

    //
    // ~Thread
    //
    // Destroys the thread
    //
    ~Thread()
    {
      // Wait for the thread handle to become signalled
      WaitForSingleObject(threadh, INFINITE);
    }

    //
    // SetPriority
    //
    // Change the priority of the thread
    //
    void SetPriority(Priority priority)
    {
      SetThreadPriority(threadh, priority);
    }

    //
    // GetPriority
    //
    // Retrieve the priority of the thread
    Priority GetPriority()
    {
      return ((Thread::Priority) GetThreadPriority(threadh));
    }

    //
    // GetID
    //
    // Retrieve the ID of the thread
    //
    inline U32 GetId()
    {
      return (threadid);
    }

    //
    // IsAlive
    //
    // Determines if the thread is alive or not
    //
    Bool IsAlive()
    {
      U32 code;
      GetExitCodeThread(threadh, &code);
      return ((code == STILL_ACTIVE) ? TRUE : FALSE);
    }

    //
    // Suspend
    //
    // Suspends execution of the thread
    // Multiple suspends will require multiple resumes
    //
    Bool Suspend()
    {
      return ((SuspendThread(threadh) == 0xFFFFFFFF) ? FALSE : TRUE);
    }

    //
    // Resume
    //
    // Resumes execution of the thread
    //
    Bool Resume()
    {
      return ((ResumeThread(threadh) == 0xFFFFFFFF) ? FALSE : TRUE);
    }

    //
    // GetCurrntId
    //
    // Returns the id of the current thread
    static U32 GetCurrentId()
    {
      return (GetCurrentThreadId());
    }

    // Returns the id of the current process
    static U32 GetProcessId()
    {
      return (GetCurrentProcessId());
    }

    // Collects the name of the current process
    static void GetProcessName(char *name, U32 len)
    {
      GetModuleFileName(NULL, name, len);
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class CritSect
  //
  // Critical Section management
  //
  class CritSect
  {
  private:
    CRITICAL_SECTION  critSect;

  public:

    //
    // CritSect
    //
    // Creates a new critical section
    //
    CritSect()
    {
      InitializeCriticalSection(&critSect);
    }
    
    //
    // ~CritSect
    //
    // Deletes a critical section
    //
    ~CritSect()
    {
      DeleteCriticalSection(&critSect);
    }

    //
    // Wait
    //
    // Enter the critical section
    //
    void Wait()
    {
      EnterCriticalSection(&critSect);
    }

    //
    // Signal
    //
    // Leave the critical section
    //
    void Signal()
    {
      LeaveCriticalSection(&critSect);
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class CriticalSection
  //
  // Mutual Exclusion management
  //
  class CriticalSection
  {
  private:

    CRITICAL_SECTION criticalSection;

  public:

    //
    // CriticalSection
    //
    // Create a new CriticalSection
    //
    CriticalSection()
    {
      InitializeCriticalSection(&criticalSection);
    }

    //
    // ~CriticalSection
    //
    // Destroy CriticalSection
    //
    ~CriticalSection()
    {
      DeleteCriticalSection(&criticalSection);
    }

    //
    // Wait
    //
    void Wait()
    {
      EnterCriticalSection(&criticalSection);
    }

    //
    // Signal
    //
    void Signal()
    {
      LeaveCriticalSection(&criticalSection);
    }


  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Mutex
  //
  // Mutual Exclusion management
  //
  class Mutex
  {
  private:

    HANDLE mutex;
    Bool   primary;

  public:

    //
    // Mutex
    //
    // Create a new mutex
    //
    Mutex(const char *name = NULL)
    {
      mutex = CreateMutex(NULL, 0, name);
      primary = name && (GetLastError() == ERROR_ALREADY_EXISTS) ? FALSE : TRUE;
    }

    //
    // ~Mutex
    //
    // Destroy mutex
    //
    ~Mutex()
    {
      CloseHandle(mutex);
    }

    //
    // GetHandle
    //
    HANDLE GetHandle()
    {
      return (mutex);
    }

    //
    // Wait
    //
    // Wait for mutex to become signaled (with optional timeout)
    //
    Bool Wait(U32 timeout = INFINITE)
    {
      return (WaitForSingleObject(mutex, timeout) == WAIT_OBJECT_0) ? TRUE : FALSE;
    }

    //
    // Signal
    //
    // Signal Mutex
    //
    void Signal()
    {
      ReleaseMutex(mutex);
    }

    //
    // IsPrimary
    //
    // Test to see if this is the primary mutex (only applies to named mutexes)
    //
    Bool IsPrimary()
    {
      return (primary);
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Semaphore
  //
  // Semaphore management
  //
  class Semaphore
  {
  private:

    HANDLE sem;
    Bool   primary;

  public:

    //
    // Semaphore
    //
    // Create a new semaphore
    //
    Semaphore(int init, int max, const char *name = NULL)
    {
      sem = CreateSemaphore(NULL, init, max, name);
      primary = name && (GetLastError() == ERROR_ALREADY_EXISTS) ? FALSE : TRUE;
    }

    //
    // ~Semaphore
    //
    // Destroy semaphore
    //
    ~Semaphore()
    {
      CloseHandle(sem);
    }

    //
    // GetHandle
    //
    HANDLE GetHandle()
    {
      return (sem);
    }

    //
    // Wait
    //
    // Wait for semaphore to become signaled (with optional timeout)
    //
    Bool Wait(U32 timeout = INFINITE)
    {
      return ((WaitForSingleObject(sem, timeout) == WAIT_OBJECT_0) ? TRUE : FALSE);
    }

    //
    // Test
    //
    // Test to see if the semaphore is signaled
    //
    Bool Test()
    {
      if (WaitForSingleObject(sem, 0) == WAIT_OBJECT_0)
      {
        ReleaseSemaphore(sem, 1, NULL);    
        return (TRUE);
      }
      else
      {
        return (FALSE);
      }
    }

    //
    // Signal
    //
    // Signal the semaphore
    //
    void Signal(int step = 1)
    {
      ReleaseSemaphore(sem, step, NULL);
    }

    // Test to see if this is the primary semaphore (only applies to named semaphores)
    Bool IsPrimary()
    {
      return (primary);
    }
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Event
  //
  // Event management
  //
  class Event
  {
  private:

    HANDLE event;
    Bool   primary;

  public:

    //
    // Event
    //
    // Create a new event
    //
    Event(const char *name = NULL)
    {
      event = CreateEvent(NULL, FALSE, FALSE, name);
      primary = (GetLastError() == ERROR_ALREADY_EXISTS) ? FALSE : TRUE;
    }

    //
    // ~Event
    //
    // Destroy Event
    //
    ~Event()
    {
      CloseHandle(event);
    }

    //
    // GetHandle
    //
    // Returns the event handle if you need it
    //
    HANDLE GetHandle()
    {
      return (event);
    }

    //
    // Wait
    //
    // Wait for event to become signaled (with optional timeout)
    //
    Bool Wait(U32 timeout = INFINITE)
    {
      return ((WaitForSingleObject(event, timeout) == WAIT_OBJECT_0) ? TRUE : FALSE);
    }

    //
    // Test
    //
    // Test an event
    //
    Bool Test()
    {
      return (Wait(0));
    }

    //
    // Signal
    //
    // Signal event
    //
    void Signal()
    {
      SetEvent(event);
    }

    //
    // Pulse
    //
    // Pulse the event
    //
    void Pulse()
    {
      PulseEvent(event);
    }

    //
    // IsPrimary
    //
    // Test to see if this is the primary event (only applies to named event)
    //
    Bool IsPrimary()
    {
      return (primary);
    }

  };




  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class SharedMem
  //
  // Shared Memory management
  //
  class SharedMem
  {
  private:

    HANDLE handle;
    U8     *ptr;
    Bool   primary;

  public:

    // Create new block of shared memory
    U8 *Create(const char *name, U32 size)
    {
      //
      // Create File mapping
      //
      handle = CreateFileMapping(
        (HANDLE) 0xFFFFFFFF,
        NULL,
        PAGE_READWRITE,
        0,
        size,
        name);

      //
      // Was it successful ?
      //
      if (handle == NULL)
      {
        return (NULL);
      }

      //
      // Get pointer to the shared memory
      //
      ptr = (U8 *) MapViewOfFile(
        handle,
        FILE_MAP_WRITE,
        0,
        0,
        0);

      //
      // Did we get the pointer ?
      //
      if (ptr == NULL)
      {
        return (NULL);
      }

      //
      // Is this the primary ?
      //
      primary = (GetLastError() == ERROR_ALREADY_EXISTS) ? FALSE : TRUE;

      return (ptr);
    }

    //
    // Destroy
    //
    // Destroy block of shared memory
    //
    void Destroy()
    {
      if (ptr && handle)
      {
        //
        // Unmap shared memory from the process's address space.
        //
        UnmapViewOfFile(ptr);

        //
        // Close the process's handle to the file-mapping object.
        //
        CloseHandle(handle);
      }

    }

    //
    // IsPrimary
    //
    // Is this the first to use this shared memory
    //
    inline Bool IsPrimary()
    {
      return (primary);
    }
  };

  //
  // WaitMultiple
  //
  // Waits on mutiple objects
  //
  // Returns '0' if it timed out
  // Returns the index of the object which was signlaled
  // Returns num if there was a problem
  //
  static int WaitMultiple(HANDLE *handles, int num, U32 timeout)
  {
    U32 val = WaitForMultipleObjects(num, handles, FALSE, timeout);

    if ((val >= WAIT_OBJECT_0) && (val < (WAIT_OBJECT_0 + num)))
    {
      return (val - WAIT_OBJECT_0 + 1);
    }
    if ((val >= WAIT_ABANDONED_0) && (val < (WAIT_ABANDONED_0 + num)))
    {
      return (num);
    }
    return (0);
  }
};

#endif
