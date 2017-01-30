///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Physics System
//
// 09-SEP-98
//


#include "physicsctrl.h"
#include "unitobj.h"
#include "collisionctrl.h"
#include "console.h"
#include "gametime.h"
#include "common.h"
#include "stdload.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace PhysicsCtrl
//

namespace PhysicsCtrl
{

  // Has the system been started
  static Bool sysInit = FALSE;

  // Gravity amount
  F32 gravity;

  // Console Command handler
  void CmdHandler(U32 pathCrc);


  //
  // Initialise the physics system
  //
  void Init()
  {
    ASSERT(!sysInit);

    // Set defaults for physics system
    SetGravity(PhysicsConst::EARTHGRAVITY);

    // Initialise collision system
    CollisionCtrl::Init();

    sysInit = TRUE;
  }


  //
  // Shut down the physics system
  //
  void Done()
  {
    ASSERT(sysInit);

    // Shutdown collision system
    CollisionCtrl::Done();

    sysInit = FALSE;
  }


  //
  // Set the gravity
  //
  void SetGravity(F32 down)
  {
    gravity = down;
  }


  //
  // Load configuration
  //
  void LoadConfig(FScope *fScope)
  {
    // Load gravity
    SetGravity(StdLoad::TypeF32(fScope, "Gravity", PhysicsConst::EARTHGRAVITY));
  }


  //
  // Save configuration
  //
  void SaveConfig(FScope *fScope)
  {
    // Save gravity
    StdSave::TypeF32(fScope, "Gravity", gravity);
  }
}
