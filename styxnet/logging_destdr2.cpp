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
#include "logging_destdr2.h"
#include "debug.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Logging
//
namespace Logging
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class DestDR2
  //

  LOGDEFLOCAL("StyxNet")


  //
  // DestDR2::DestDR2
  //
  DestDR2::DestDR2() 
  {
  }


  //
  // DestDR2::~DestDR2
  //
  DestDR2::~DestDR2()
  {
  }


  //
  // DestDR2::Write
  //
  // Write to the destination
  //
  void DestDR2::Write(
    Level level, const char *, const char *sourceModule, U32 sourceLine, U32,
    const char *message)
  {
    switch (level)
    {
      case ERR:
        ERR_INDIRECT(sourceModule, sourceLine, Debug::Error::FATAL, (message))
        break;

      case WARN:
        LOG_INDIRECT(sourceModule, sourceLine, Log::WARN, (message))
        break;

      case DIAG:
        LOG_INDIRECT(sourceModule, sourceLine, Log::DIAG, (message))
        break;
    }
  }
}

