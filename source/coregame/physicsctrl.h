///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Physics System
//
// 09-SEP-98
//


#ifndef __PHYSICSCTRL_H
#define __PHYSICSCTRL_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "utiltypes.h"
#include "mathtypes.h"
#include "physics_const.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace PhysicsCtrl - Manages the physics system
//

namespace PhysicsCtrl
{

  // Gravity amount
  extern F32 gravity;


  // Initialise the physics system
  void Init();

  // Shut down the physics system
  void Done();


  // Get the gravity value
  inline F32 GetGravity()
  {
    return (gravity);
  }

  // Set the gravity value
  void SetGravity(F32 down);

  // Load configuration
  void LoadConfig(FScope *fScope);

  // Save configuration
  void SaveConfig(FScope *fScope);

}

#endif
