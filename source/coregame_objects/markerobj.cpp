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
#include "markerobj.h"
#include "stdload.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class MarkerObjType
//


//
// MarkerObjType::MarkerObjType
//
// Constructor
//
MarkerObjType::MarkerObjType(const char *name, FScope *fScope) 
: MapObjType(name, fScope)
{
  // Disable saving
  save = FALSE;
}


//
// MarkerObjType::NewInstance
//
// Create a new map object instance using this type
//
GameObj* MarkerObjType::NewInstance(U32 id)
{
  // Allocate new object instance
  return (new MarkerObj(this, id));
}


///////////////////////////////////////////////////////////////////////////////
//
// Class MarkerObj - Instance class for above type
//


//
// MarkerObj::MarkerObj
//
// Constructor
//
MarkerObj::MarkerObj(MarkerObjType *objType, U32 id) : MapObj(objType, id)
{
}


//
// MarkerObj::MarkerObj
//
// Destructor
//
MarkerObj::~MarkerObj()
{
}


//
// MarkerObj::PreDelete
//
// Pre deletion cleanup
//
void MarkerObj::PreDelete()
{
  // Call parent scope last
  MapObj::PreDelete();
}
