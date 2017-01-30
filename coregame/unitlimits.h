///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Configurable Unit Construction Limitations
//
// 6-MAR-2000
//

#ifndef __UNITLIMITS_H
#define __UNITLIMITS_H


///////////////////////////////////////////////////////////////////////////////
//
// Namespace UnitLimits
//
namespace UnitLimits
{
  // Initialize and shutdown system
  void Init();
  void Done();

  // Returns the per team command point limit
  U32 GetLimit();
}

#endif
