///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 27-APR-1998
//

#ifndef __GAMEBABEL_H
#define __GAMEBABEL_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "fscope.h"
#include "gameobjdec.h"
#include "utiltypes.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace GameBabel - Translates between identifiers and types
//
namespace GameBabel
{
  // Babel callback type
  typedef GameObjType* (BabelCallBack)(GameIdent &, const char *, FScope *);

  // Register a babel callback
  void RegisterBabel(BabelCallBack *callBack);

  // Returns a new object type instance, or NULL if the class id is not recognized
  GameObjType* NewGameObjType(GameIdent &classId, const char *name, FScope *fScope);

  // Initialize and shutdown the system
  void Init();
  void Done();
};

#endif