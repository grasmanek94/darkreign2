///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1998
// Matthew Versluys
//
// Logging System
//


#ifndef __LOGGING_DESTDR2_H
#define __LOGGING_DESTDR2_H


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
  // Class DestDR2
  //
  class DestDR2 : public Destination
  {
  public:

    // Constructor and Destructor
    DestDR2();
    ~DestDR2();

    // Write to the destination
    void Write(Level level, const char *labelName, const char *sourceModule, U32 sourceLine, U32 elapsed, const char *message);

  };

}

#endif

