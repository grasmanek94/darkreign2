///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-APR-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "propobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class PropObjType - An object that just sits and does nout
//


//
// PropObjType::PropObjType
//
// Constructor
//
PropObjType::PropObjType(const char *name, FScope *fScope) : MapObjType(name, fScope)
{
  // If the showSeen was not configured externally
  if (!showSeenConfig)
  {
    // Set all props to appear under fog
    showSeen = TRUE;
  }
}


//
// PropObjType::NewInstance
//
// Create a new map object instance using this type
//
GameObj* PropObjType::NewInstance(U32 id)
{
  // Allocate new object instance
  return (new PropObj(this, id));
}


///////////////////////////////////////////////////////////////////////////////
//
// Class PropObj - Instance class for above type
//

//
// PropObj::PropObj
//
// Constructor
//
PropObj::PropObj(PropObjType *objType, U32 id) : MapObj(objType, id)
{
}


//
// PropObj::~PropObj
//
// Destructor
//
PropObj::~PropObj()
{
}
