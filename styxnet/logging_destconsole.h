///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1998
// Matthew Versluys
//
// Logging System
//


#ifndef __LOGGING_DESTCONSOLE_H
#define __LOGGING_DESTCONSOLE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "logging_dest.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Logging
//
namespace Logging
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class DestConsole
  //
  class DestConsole : public Destination
  {
  public:

    // Constructor and Destructor
    DestConsole();
    ~DestConsole();

    // Write to the destination
    void Write(Level level, const char *labelName, const char *sourceModule, U32 sourceLine, U32 elapsed, const char *message);

  };

}

#endif

