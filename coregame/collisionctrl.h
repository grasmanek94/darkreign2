///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Physics System
//
// 09-SEP-98
//


#ifndef __COLLISIONCTRL_H
#define __COLLISIONCTRL_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mathtypes.h"


///////////////////////////////////////////////////////////////////////////////
//
// Forward declarations
//
class ProjectileObj;


///////////////////////////////////////////////////////////////////////////////
//
// namespace CollisionCtrl - Manages the collision system
//

namespace CollisionCtrl
{

  // Initialise the collision system
  void Init();

  // Shutdown the collision system
  void Done();

  // Add the object to the collision list
  void AddObject(MapObj *obj);

  // Resolve collisions
  void Resolve();

};

#endif
