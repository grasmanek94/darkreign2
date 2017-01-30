// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000
//
// Definition of LastError (ImageMagick error handler)
//
// This class serves to collect the last error/warning
// reported by the ImageMagick error callback.
//
// This class is a Magick++ implementation class and is not intended
// for use by end-users.
// 
#if !defined (LastError_header)
#define LastError_header

#include "Thread.h"

namespace Magick
{
  //
  // Pure virtual base class to represent last error
  //
  class LastErrorBase
  {
  public:
    LastErrorBase( void ) {};
    virtual ~LastErrorBase( void ) {};

    // Test to see if error or warning is set
    virtual bool           isError( void ) const = 0;
    
    // Clear out existing error info
    virtual void           clear ( void ) = 0;
    
    // Error code
    virtual void           error ( MagickLib::ErrorType error_ ) = 0;
    virtual MagickLib::ErrorType error ( void ) const = 0;

    // Warning code
    virtual void           warning ( MagickLib::WarningType warning_ ) = 0;
    virtual MagickLib::WarningType warning ( void ) const = 0;
    
    // System errno
    virtual void           syserror ( int syserror_ ) = 0;
    virtual int            syserror ( void ) const = 0;
    
    // Error message
    virtual void           message ( std::string message_ ) = 0;
    virtual std::string    message ( void ) const = 0;
    
    // Error qualifier
    virtual void           qualifier ( std::string qualifier_ ) = 0;
    virtual std::string    qualifier ( void ) const = 0;
    
    // Throw exception corresponding to error (if any)
    virtual void           throwException( void ) = 0;
  };

  //
  // Interface class to access singleton.
  //
  class LastError : public LastErrorBase
  {
    friend class LastErrorGuard;
  public:
    // Default constructor
    LastError( void );

    // Destructor
    ~LastError( void );

    // Assignment operator
    LastError& LastError::operator=(const LastError& value_);

    // Copy constructor
    LastError( const LastError& value_ );

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

    //
    // No user-serviceable parts beyond this point
    //

    static MutexLock*      _mutexLock; // Mutex lock

  private:

    // Access error data
    LastErrorBase*         instance();
    const LastErrorBase*   instance() const;

    // Indicate that destruction of error data is desired
    void doDelete();

    static unsigned        _refCount;  // References
    static bool            _doDelete;  // Deletion desired
  };
}

#endif // LastError_header
