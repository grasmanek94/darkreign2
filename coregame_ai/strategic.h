/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic AI
//
// 31-AUG-1998
//


#ifndef __STRATEGIC_H
#define __STRATEGIC_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "ai.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{
  LOGEXTERN


  /////////////////////////////////////////////////////////////////////////////
  //
  // Forward Declarations
  //


  // Initialize Strategic AI
  void Init();

  // Shutdown Strategic AI
  void Done();

  // Create a new Strategic AI object
  class Object * Create(U32 personality = 0, Bool route = FALSE);

  // Save and load
  void Save(FScope *scope);
  void Load(FScope *scope);

  // Reset the Strategic AI after a mission has been loaded
  void Reset();

  // Process Strategic AI
  void Process();

}


#endif
