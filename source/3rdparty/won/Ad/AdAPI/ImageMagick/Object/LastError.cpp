// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000
//
// Implementation of LastError (ImageMagick error handler)
//

#define MAGICK_IMPLEMENTATION

#include <string>

#include <errno.h>

#include "Thread.h"
#include "Include.h"

using namespace std;

#include "LastError.h"
#include "Exception.h"

// Magick++ error/warning callback functions
namespace Magick
{
  extern "C" {
  void LastErrorHandler( const MagickLib::ErrorType error_,
			 const char *message_ ,
			 const char *qualifier_ );
  void LastWarningHandler( const MagickLib::WarningType warning_,
			   const char *message_ ,
			   const char *qualifier_ );
#if defined(HasPTHREADS)
  void LastErrorThreadSpecificDestructor ( void * tsd_ );
  void LastErrorOnce( void );
#endif
  }

  // Format and throw exception
  void throwException( LastErrorBase &lastError_ );

  //
  // Class to contain actual error data
  //
  class LastErrorData : public LastErrorBase
  {
    // Accessed only via LastError
    friend class LastError;

  public:
    // Destructor
    ~LastErrorData( void );

  private:

    // Constructor
    LastErrorData( void );

    // Test to see if error or warning is set
    bool                   isError( void ) const;
    
    // Clear out existing error info
    void                   clear ( void );
    
    // Error code
    void                   error ( MagickLib::ErrorType error_ );
    MagickLib::ErrorType   error ( void ) const;

    // Warning code
    void                   warning ( MagickLib::WarningType warning_ );
    MagickLib::WarningType warning ( void ) const;
    
    // System errno
    void                   syserror ( int syserror_ );
    int                    syserror ( void ) const;
    
    // Error message
    void                   message ( std::string message_ );
    std::string            message ( void ) const;
    
    // Error qualifier
    void                   qualifier ( std::string qualifier_ );
    std::string            qualifier ( void ) const;
    
    // Throw exception corresponding to error (if any)
    void                   throwException( void );

  private:

    // Don't support copy constructor
    LastErrorData ( const LastErrorData& original_ );
    
    // Don't support assignment
    LastErrorData& operator = ( const LastErrorData& original_ );
    
    MagickLib::ErrorType   _error;
    MagickLib::WarningType _warning;
    int                    _syserror;
    std::string            _message;
    std::string            _qualifier;
  };

  // Destruction guard class.  Calls doDelete() if program exits
  class LastErrorGuard
  {
  public:
    LastErrorGuard( void )
      {
      }
    ~LastErrorGuard( void )
      {
	_singleton.doDelete();
      }
  private:
    LastError _singleton;
  };
}

// Allocate static guard class in this translation unit
// Static guard class is destroyed when program exits.
static Magick::LastErrorGuard LastErrorGuard_inst;

// Constructor
Magick::LastErrorData::LastErrorData( void )
  : _error(MagickLib::UndefinedError),
    _warning(MagickLib::UndefinedWarning),
    _syserror(0),
    _message(),
    _qualifier()
{
  // Register error callback function with ImageMagick
  MagickLib::SetErrorHandler( LastErrorHandler );
  MagickLib::SetWarningHandler( LastWarningHandler );
}

// Destructor
Magick::LastErrorData::~LastErrorData( void )
{
  // Nothing to do
}

//
// LastErrorData (Single Thread) implementation
//

// Test to see if object contains error
bool Magick::LastErrorData::isError( void ) const
{
  return ( _error   != MagickLib::UndefinedError ||
	   _warning != MagickLib::UndefinedWarning );
}

// Clear out existing error info
void Magick::LastErrorData::clear ( void )
{
  _error     = MagickLib::UndefinedError;
  _warning   = MagickLib::UndefinedWarning;
  _syserror  = 0;
  _message.erase();
  _qualifier.erase();
}

// Error code
void Magick::LastErrorData::error ( MagickLib::ErrorType error_ )
{
  _error = error_;
}
MagickLib::ErrorType Magick::LastErrorData::error ( void ) const
{
  return _error;
}

// Warning code
void Magick::LastErrorData::warning ( MagickLib::WarningType warning_ )
{
  _warning = warning_;
}
MagickLib::WarningType Magick::LastErrorData::warning ( void ) const
{
  return _warning;
}

// System error
void Magick::LastErrorData::syserror ( int syserror_ )
{
  _syserror = syserror_;
}
int Magick::LastErrorData::syserror ( void ) const
{
  return _syserror;
}
  
// Error message
void Magick::LastErrorData::message ( std::string message_ )
{
  _message = message_;
}
std::string Magick::LastErrorData::message ( void ) const
{
  return _message;
}

// Error qualifier
void  Magick::LastErrorData::qualifier ( std::string qualifier_ )
{
  _qualifier = qualifier_;
}
std::string Magick::LastErrorData::qualifier ( void ) const
{
  return _qualifier;
}

// Throw exception corresponding to error (if any)
// Clears error info before throw.
void Magick::LastErrorData::throwException( void )
{
  if ( !isError() )
    return;

  Magick::throwException( *this );
}

//
// LastError (interface class) implementation
//

// Assignment operator
Magick::LastError& Magick::LastError::operator=(const Magick::LastError& value_)
{
  // Nothing to assign.
  return *this;
}

// Copy constructor
Magick::LastError::LastError( const Magick::LastError& value_ )
{
  // Increase reference count
  value_._mutexLock->lock();
  ++value_._refCount;
  value_._mutexLock->unlock();
}

//
// Pass-through methods to singleton implementation
//

// Test to see if error or warning is set
bool Magick::LastError::isError( void ) const
{
  return instance()->isError();
}
    
// Clear out existing error info
void Magick::LastError::clear ( void )
{
  instance()->clear();
}
    
// Error code
void Magick::LastError::error ( MagickLib::ErrorType error_ )
{
  instance()->error(error_);
}
MagickLib::ErrorType Magick::LastError::error ( void ) const 
{
  return instance()->error();
}

// Warning code
void Magick::LastError::warning ( MagickLib::WarningType warning_ )
{
  instance()->warning( warning_ );
}
MagickLib::WarningType Magick::LastError::warning ( void ) const
{
  return instance()->warning();
}
    
// System errno
void Magick::LastError::syserror ( int syserror_ )
{
  instance()->syserror( syserror_ );
}
int Magick::LastError::syserror ( void ) const
{
  return instance()->syserror();
}
    
// Error message
void Magick::LastError::message ( std::string message_ )
{
  instance()->message( message_ );
}
std::string Magick::LastError::message ( void ) const
{
  return instance()->message();
}
    
// Error qualifier
void Magick::LastError::qualifier ( std::string qualifier_ )
{
  instance()->qualifier( qualifier_ );
}
std::string Magick::LastError::qualifier ( void ) const
{
  return instance()->qualifier();
}

// Throw exception corresponding to error (if any)
void Magick::LastError::throwException( void )
{
  instance()->throwException();
}

namespace Magick
{
#if defined(HasPTHREADS)
  // Pthreads TSD
  static pthread_key_t tsd_key;
  static pthread_once_t key_once = PTHREAD_ONCE_INIT;
#else
  // Not threaded
  static LastErrorBase* LastErrorDataInst = 0;
#endif
}
// Initialize LastError static members
Magick::MutexLock*     Magick::LastError::_mutexLock = 0;    // Mutex lock
unsigned               Magick::LastError::_refCount = 0;     // Reference counter
bool                   Magick::LastError::_doDelete = false; // Object is to be deleted

#if defined(HasPTHREADS)
// Destroy thread specific data for a thread
void Magick::LastErrorThreadSpecificDestructor ( void * tsd_ )
{
  Magick::LastErrorData *p = (Magick::LastErrorData *)tsd_;
  delete p;
}
// Called just once to initialize thread specific data key
void Magick::LastErrorOnce( void )
{
  // Pthreads TSD
  int status = pthread_key_create( &Magick::tsd_key, LastErrorThreadSpecificDestructor );
  if ( status != 0 )
    {
      switch ( status )
	{
	case EAGAIN :
	case ENOMEM :
	  throw Magick::ErrorResourceLimit( "pthread_key_create() failed" );
	case EINVAL :
	  throw Magick::ErrorOption( "pthread_key_create() failed" );
	default :
	  {
	    throw Magick::ErrorResourceLimit( "pthread_key_create() failed" );
	  } 
	}
    }
  Magick::LastError::_mutexLock = new MutexLock;
}
#endif

// Default constructor
Magick::LastError::LastError( void )
{
#if defined(HasPTHREADS)
  // Pthreads TSD
  int status = pthread_once ( &key_once, LastErrorOnce );
  if ( status != 0 )
    throw Magick::ErrorOption( "pthread_once() failed" );
  LastErrorBase* data = (LastErrorBase*) pthread_getspecific( tsd_key );
  if ( data == 0 )
    {
      data = new LastErrorData;
      status = pthread_setspecific( tsd_key, data );
      if ( status != 0 )
	{
	  switch ( status )
	    {
	    case ENOMEM :
	      throw Magick::ErrorResourceLimit( "pthread_key_create() failed" );
	    case EINVAL :
	      throw Magick::ErrorOption( "pthread_key_create() failed" );
	    default :
	      {
		throw Magick::ErrorResourceLimit( "pthread_key_create() failed" );
	      } 
	    }
	}
    }
  _mutexLock->lock();
  ++_refCount;
  _mutexLock->unlock();
#else
  // Not threaded
  if ( Magick::LastErrorDataInst == 0 )
    {
      Magick::LastErrorDataInst = new LastErrorData;
      _mutexLock = new MutexLock;
    }
  ++_refCount;
#endif
}

// Destructor
Magick::LastError::~LastError( void )
{
#if defined(HasPTHREADS)
  // Pthreads TSD
  bool doDelete = false;
  {
    _mutexLock->lock();
    if ( --_refCount == 0 && _doDelete )
      doDelete = true;
    _mutexLock->unlock();
  }
  if ( doDelete )
    {
      pthread_key_delete( tsd_key );
      delete _mutexLock;
      _mutexLock = 0;
    }
#else
  // Not threaded
  if ( --_refCount == 0 && _doDelete )
    {
      delete Magick::LastErrorDataInst;
      delete _mutexLock;
      _mutexLock = 0;
    }
#endif
}

// Access error data
Magick::LastErrorBase* Magick::LastError::instance()
{
#if defined(HasPTHREADS)
  // Pthreads TSD
  LastErrorBase* data = (LastErrorBase*) pthread_getspecific( tsd_key );
  return (Magick::LastErrorBase*)data;
#else
  // Not threaded
  return Magick::LastErrorDataInst;
#endif
}
const Magick::LastErrorBase* Magick::LastError::instance() const
{
#if defined(HasPTHREADS)
  // Pthreads TSD
  LastErrorBase* data = (LastErrorBase*) pthread_getspecific( tsd_key );
  return (Magick::LastErrorBase*)data;
#else
  // Not threaded
  return Magick::LastErrorDataInst;
#endif
}

// Indicate that destruction is desired
void Magick::LastError::doDelete()
{
#if defined(HasPTHREADS)
  // Pthreads TSD
  _doDelete = true;
  if ( _refCount == 0 )
    {
      ::pthread_key_delete( tsd_key );
      delete _mutexLock;
      _mutexLock = 0;
    }
#else
  // Not threaded
  _doDelete = true;
  if ( _refCount == 0 )
    {
      delete Magick::LastErrorDataInst;
      delete _mutexLock;
      _mutexLock = 0;
    }
#endif
}

//
// Magick++ error callback function
//
void Magick::LastErrorHandler( const MagickLib::ErrorType error_,
			       const char *message_ ,
			       const char *qualifier_)
{
  LastError err;
  err.error( error_ );
  err.syserror( errno );
  if ( message_ )
    err.message( message_ );
  else
    err.message( "" );
  if ( qualifier_ )
    err.qualifier( qualifier_ );
  else
    err.qualifier( "" );

  // Clear out system errno now that it has been collected.
  errno = 0;
}

//
// Magick++ warning callback function
//
void Magick::LastWarningHandler( const MagickLib::WarningType warning_,
				 const char *message_ ,
				 const char *qualifier_)
{
  LastError err;
  err.warning( warning_ );
  err.syserror( errno );
  if ( message_ )
    err.message( message_ );
  else
    err.message( "" );
  if ( qualifier_ )
    err.qualifier( qualifier_ );
  else
    err.qualifier( "" );

  // Clear out system errno now that it has been collected.
  errno = 0;
}

// Format and throw exception
void Magick::throwException( Magick::LastErrorBase &lastError_ )
{
  // Format error message ImageMagick-style
  std::string message = MagickLib::SetClientName((char *) NULL);
  if ( lastError_.message().length() > 0 )
    message += std::string(": ");

  if ( lastError_.message().length() > 0 )
    message += lastError_.message();

  if ( lastError_.qualifier().length() > 0 )
    message += " (" + lastError_.qualifier() + ")";

  if ( lastError_.syserror() )
    message += std::string(" [") + strerror(lastError_.syserror()) + std::string("]");

  if ( lastError_.warning() != MagickLib::UndefinedWarning )
    {
      MagickLib::WarningType warningVal = lastError_.warning();
      lastError_.clear();
      
      switch ( warningVal )
	{
	  // Warnings
	case MagickLib::ResourceLimitWarning :
	  throw WarningResourceLimit( message );
	case MagickLib::XServerWarning :
	  throw WarningXServer( message );
	case MagickLib::OptionWarning :
	  throw WarningOption( message );
	case MagickLib::DelegateWarning :
	  throw WarningDelegate( message );
	case MagickLib::MissingDelegateWarning :
	  throw WarningMissingDelegate( message );
	case MagickLib::CorruptImageWarning :
	  throw WarningCorruptImage( message );
	case MagickLib::FileOpenWarning :
	  throw WarningFileOpen( message );
	case MagickLib::BlobWarning :
	  throw WarningBlob ( message );
	case MagickLib::CacheWarning :
	  throw WarningCache ( message );
	case MagickLib::UndefinedWarning :
	default :
	  throw WarningUndefined( message );
	}
    }
  else if ( lastError_.error() != MagickLib::UndefinedError )
    {
      MagickLib::ErrorType errorVal = lastError_.error();
      lastError_.clear();
      
      switch ( errorVal )
	{
	  // Errors
	case MagickLib::ResourceLimitError :
	  throw ErrorResourceLimit( message );
	case MagickLib::XServerError :
	  throw ErrorXServer( message );
	case MagickLib::OptionError :
	  throw ErrorOption( message );
	case MagickLib::DelegateError :
	  throw ErrorDelegate( message );
	case MagickLib::MissingDelegateError :
	  throw ErrorMissingDelegate( message );
	case MagickLib::CorruptImageError :
	  throw ErrorCorruptImage( message );
	case MagickLib::FileOpenError :
	  throw ErrorFileOpen( message );
	case MagickLib::BlobError :
	  throw ErrorBlob ( message );
	case MagickLib::CacheWarning :
	  throw ErrorCache ( message );
	case MagickLib::UndefinedError :
	default :
	  throw ErrorUndefined( message );
	}
    }
}
