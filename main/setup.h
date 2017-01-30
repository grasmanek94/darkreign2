///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Engine Setup Management
//
// 18-DEC-1997
//

#ifndef __SETUP_H
#define __SETUP_H


//
// Includes
//
#include "ptree.h"
#include "filesys.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Sub key for the root directory
#define SETUP_ROOTKEY       "@rootdir"

// Name of the stream pointing to the root directory
#define SETUP_ROOTSTREAM    "root"



///////////////////////////////////////////////////////////////////////////////
//
// Namespace Setup - Manages basic engine configuration
//

namespace Setup
{
  // Check current media type (FALSE if illegal)
  Bool CheckMediaType();

  // Does startup configuration
  void StartupConfiguration();

  // Process configuration of mono
  void ProcessMonoConfiguration(FScope *fScope);

  // Find the original CD using the Activision SDK
  Bool FindOriginalCD();
};


#endif