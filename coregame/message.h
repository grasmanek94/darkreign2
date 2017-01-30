///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game Messages
// 16-MAR-1999
//

#ifndef __MESSAGE_H
#define __MESSAGE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mapobjdec.h"
#include "mathtypes.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Message
//
namespace Message
{

  // Initialization and Shutdown
  void Init();
  void Done();

  // Process configruation
  void ProcessConfigureGameMessage(FScope *fScope);
  void ProcessConfigureLocationMessage(FScope *fScope);
  void ProcessConfigureObjMessage(FScope *fScope);

  // Trigger a Game Message
  void CDECL TriggerGameMessage(U32 message, U32 params = 0, ...);

  // Trigger a Location Message
  void CDECL TriggerLocationMessage(U32 message, const Vector &location, U32 params = 0, ...);

  // Trigger a Obj Message
  void CDECL TriggerObjMessage(U32 message, GameObj *gameobj, const Vector *position, U32 params = 0, ...);
};

#endif