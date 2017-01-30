///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1998
// Matthew Versluys
//
// Logging System
//


#ifndef __LOGGING_DEST_H
#define __LOGGING_DEST_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "logging.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Logging
//
namespace Logging
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Destination
  //
  struct Destination
  {
    // List Node
    NList<Destination>::Node node;

    // Destructor
    virtual ~Destination() { };

    // Write to the destination
    virtual void Write(
      Level level, 
      const char *labelName, 
      const char *sourceModule, 
      U32 sourceLine,
      U32 elapsed,
      const char *message) = 0;
  };

}

#endif