///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1999
// Matthew Versluys
//
// Win32 Mutex
//


#ifndef __WIN32_MUTEX_H
#define __WIN32_MUTEX_H


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
  // Class Mutex
  //
  class Mutex
  {
  private:

    // Mutex handle
    HANDLE mutex;

    // Is this the primary mutex (only relevent to named mutexes)
    Bool primary;

  public:

    // Create a new mutex
    Mutex(const char *name = NULL);

    // ~Mutex
    ~Mutex();

    // Returns the mutex handle if you need it
    HANDLE GetHandle() const;

    // Wait for mutex to become signaled (with optional timeout)
    Bool Wait(U32 timeout = INFINITE);

    // Signal mutex
    void Signal();

    // IsPrimary
    Bool IsPrimary();

  };

}

#endif