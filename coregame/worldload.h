///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 03-JUN-1998
//

#ifndef __WORLDLOAD_H
#define __WORLDLOAD_H


//
// Includes
//
#include "ptree.h"
#include "gameobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace WorldLoad - Processes a world definition file
//

namespace WorldLoad
{

  // Save all objects, false if unable to create file
  Bool SaveObjectFile(const char *name);

  // Load a object definition file into current state, false if not found
  Bool LoadObjectFile(const char *name);
};

#endif