// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000
//
// Implementation of thread support
//

#define MAGICK_IMPLEMENTATION

#include "Exception.h"
#include "Thread.h"

// Default constructor
Magick::MutexLock::MutexLock(void)
{
#if defined(HasPTHREADS)
  ::pthread_mutexattr_t attr;
  int sysError = 0;
  if ( (sysError = ::pthread_mutexattr_init( &attr )) == 0 )
    if ( (sysError = ::pthread_mutex_init( &_mutex, &attr )) == 0 )
      return;
  throw Magick::ErrorOption( "mutex initialization failed" );
#endif
#if defined(_MT) && defined(_VISUALC_)
	SECURITY_ATTRIBUTES security;

	/* Allow the semaphore to be inherited */
	security.nLength = sizeof(security);
	security.lpSecurityDescriptor = NULL;
	security.bInheritHandle = TRUE;

	/* Create the semaphore, with initial value signaled */
	_mutex.id = ::CreateSemaphore(&security, 1, MAXSEMLEN, NULL);
	if ( _mutex.id != NULL )
        return;
  throw Magick::ErrorOption( "mutex initialization failed" );
#endif
}

// Destructor
Magick::MutexLock::~MutexLock(void)
{
#if defined(HasPTHREADS)
  if ( ::pthread_mutex_destroy( &_mutex ) == 0 )
    return;
  throw Magick::ErrorOption( "mutex destruction failed" );
#endif
#if defined(_MT) && defined(_VISUALC_)
  if ( ::CloseHandle(_mutex.id) != 0 )
    return;
  throw Magick::ErrorOption( "mutex destruction failed" );
#endif
}

// Lock mutex
void Magick::MutexLock::lock(void)
{
#if defined(HasPTHREADS)
  if ( ::pthread_mutex_lock( &_mutex ) == 0)
    return;
  throw Magick::ErrorOption( "mutex lock failed" );
#endif
#if defined(_MT) && defined(_VISUALC_)
  if ( WaitForSingleObject(_mutex.id, INFINITE) != WAIT_FAILED )
    return;
  throw Magick::ErrorOption( "mutex lock failed" );
#endif
}

// Unlock mutex
void Magick::MutexLock::unlock(void)
{
#if defined(HasPTHREADS)
  if ( ::pthread_mutex_unlock( &_mutex ) == 0)
    return;
  throw Magick::ErrorOption( "mutex unlock failed" );
#endif
#if defined(_MT) && defined(_VISUALC_)
  if ( ReleaseSemaphore(_mutex.id, 1, NULL) == TRUE )
    return;
  throw Magick::ErrorOption( "mutex unlock failed" );
#endif
}
