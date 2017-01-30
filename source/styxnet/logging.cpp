///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1998
// Matthew Versluys
//
// Logging System
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "std.h"
#include "logging.h"
#include "logging_dest.h"
#include "clock.h"
#include "debug.h"
#include "win32.h"


///////////////////////////////////////////////////////////////////////////////
//
// Global logging
//
LOG_DEFLOCAL("Global")


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Logging
//
namespace Logging
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Definitions
  //
  #define LOG_BUFFERSIZE 1024


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //
  static Bool               initialized = FALSE;
  static char               buffer[LOG_BUFFERSIZE];
  static NList<Destination> destinations(&Destination::node);
  static U32                startTime;

  const char *levelDescShort[5] =   { " ",        "!",    "?",    "-",    "+"    };
  const char *levelDescVerbose[5] = { "DISABLED", "ERR ", "WARN", "DIAG", "DEV " };

  Win32::Mutex mutex;


  //
  // Initialize Logging System
  //
  void Init()
  {
    ASSERT(!initialized)

    // Grab the start time
    startTime = Clock::Time::Ms();

    // Set the initialized flag
    initialized = TRUE;
  }


  //
  // Shutdown Logging System
  //
  void Done()
  {
    ASSERT(initialized)

    // Shutdown the destinations
    destinations.DisposeAll();

    // Clear the initialized flag
    initialized = FALSE;
  }


  //
  // AddDestination
  //
  // Add a global destination
  //
  void AddDestination(Destination *destination)
  {
    ASSERT(destination)
    destinations.Append(destination);
  }


  //
  // RemoveDestination
  //
  // Remove a global destination
  //
  void RemoveDestination(Destination *destination)
  {
    ASSERT(destination)
    destinations.Unlink(destination);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Client
  //

  //
  // Client::Client
  //
  // Constructor
  //
  Client::Client(const char *name) 
  : name(name)
  {
  }


  //
  // Client::~Client
  //
  // Destructor
  //
  Client::~Client()
  {
  }


  //
  // Client::Write
  //
  void Client::Write()
  {
    // Get the message text
    const char *message = stream.str();

    // Get the filename
    const char *fileName = Utils::Strrchr(file, '\\') + 1;

    // Get the time
    U32 time = Clock::Time::Ms() - startTime;

    /*
    // Is this an warning or error ?
    if (level == WARN || level == ERR)
    {
      // Compose buffer into exception information
      U32 arguments[4];
      
      arguments[0] = (U32) message;
      arguments[1] = (U32) file;
      arguments[2] = (U32) line;
      arguments[3] = level == WARN ? 1 : 0;

      RaiseException(1, level == ERR ? EXCEPTION_NONCONTINUABLE_EXCEPTION : 0, 4, arguments);
    }
    else
    */
    {
      // Write to the destinations
      NList<Destination>::Iterator dests(&destinations);
      for (!dests; dests.IsValid(); ++dests)
      {
        (*dests)->Write(level, name, fileName, line, time, message);
      }
    }

    stream.freeze(0);
    stream.seekp(0);
  }

}
