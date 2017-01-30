///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1998
// Matthew Versluys
//
// Logging System
//


#ifndef __LOGGING_H
#define __LOGGING_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "stream.h"
#include "win32_mutex.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Client Definition Macro
#define LOG_DEC          static ::Logging::Client logClient;
#define LOG_EXTERN       extern ::Logging::Client logClient;

// Client Declaration Macro
#define LOG_DEF(c, n)    ::Logging::Client c::logClient(n);
#define LOG_DEFLOCAL(n)  ::Logging::Client logClient(n);

// Log Macro
#define LOGGING(t, x)                     \
{                                         \
  Logging::mutex.Wait();                  \
  logClient.Set(__FILE__, __LINE__, t);   \
  logClient.stream << x;                  \
  logClient.stream << ends << flush;      \
  logClient.Write();                      \
  Logging::mutex.Signal();                \
}

#define LERR(y)  LOGGING(::Logging::ERR, y)
#define LWARN(y) LOGGING(::Logging::WARN, y)
#define LDIAG(y) LOGGING(::Logging::DIAG, y)

#define LOG_ADDDEST(d) Logging::AddDestination(d);
#define LOG_REMOVEDEST(d) Logging::RemoveDestination(d);


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Logging
//
namespace Logging
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Forward Declarations
  //
  struct Destination;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Enum Level
  //
  enum Level
  {
    ERR,
    WARN,
    DIAG
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Client
  //
  class Client 
  {
  private:                      

    // Source file
    const char *file;

    // Line number of source
    U32 line;

    // Log level
    Level level;
    
    // Name of the label
    const char *name;

  public:

    // The stream
    ostrstream stream;

    // Constructor and Destructor
    Client(const char *name);
    ~Client();

    // Set
    void Set(const char *fileIn, U32 lineIn, Level levelIn)
    {
      file = fileIn;
      line = lineIn;
      level = levelIn;
    }

    // Write
    void Write();

  };


  extern const char *levelDescShort[5];
  extern const char *levelDescVerbose[5];
  extern Win32::Mutex mutex;


  // Initialize Logging System
  void Init();

  // Shutdown Logging System
  void Done();

  // Add a destination
  void AddDestination(Destination *destination);

  // Remove a destination
  void RemoveDestination(Destination *destination);

}

// Global Logging
LOG_EXTERN

#endif
