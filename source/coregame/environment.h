///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Environment control
//
// 1-MAR-1999
//


#ifndef __ENVIRONMENT_H
#define __ENVIRONMENT_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "fscope.h"
#include "varsys.h"
#include "bitmap.h"

///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Environment
//
namespace Environment
{
  extern VarFloat     waterSpeed;   // water u,v animation speed

  // Initialization and Shutdown
  void Init();
  void Done();

  // Process Environment
  void Process();

  // Render Environment
  void Render();

  // Load information
  void LoadInfo(FScope *fScope);

  // Save information
  void SaveInfo(FScope *fScope);

  // Post Load
  void PostLoad();
}


#endif