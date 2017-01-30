///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1999
// Matthew Versluys
//
// Win32 Mutex
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "win32_mutex.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Win32
//
namespace Win32
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Mutex
  //


  //
  // Mutex
  //
  // Create a new mutex
  //
  Mutex::Mutex(const char *name)
  {
    mutex = CreateMutex(NULL, 0, name);
    primary = name && (GetLastError() == ERROR_ALREADY_EXISTS) ? FALSE : TRUE;
  }

  
  //
  // ~Mutex
  //
  // Destroy Mutex
  //
  Mutex::~Mutex()
  {
    CloseHandle(mutex);
  }


  //
  // GetHandle
  //
  // Returns the mutex handle if you need it
  //
  HANDLE Mutex::GetHandle() const
  {
    return (mutex);
  }


  //
  // Wait
  //
  // Wait for mutex to become signaled (with optional timeout)
  //
  Bool Mutex::Wait(U32 timeout)
  {
    return ((WaitForSingleObject(mutex, timeout) == WAIT_OBJECT_0) ? TRUE : FALSE);
  }

  
  //
  // Signal
  //
  // Signal mutex
  //
  void Mutex::Signal()
  {
    ReleaseMutex(mutex);
  }


  //
  // IsPrimary
  //
  // Test to see if this is the primary mutex (only applies to named mutexes)
  //
  Bool Mutex::IsPrimary()
  {
    return (primary);
  }

}

