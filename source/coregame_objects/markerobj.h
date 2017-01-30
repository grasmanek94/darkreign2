///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-AUG-1998
//

#ifndef __MARKEROBJ_H
#define __MARKEROBJ_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "markerobjdec.h"
#include "mapobj.h"
#include "promote.h"
#include "mathtypes.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Forward delcarations
class MarkerObj;

// Map object reapers
typedef Reaper<MarkerObj> MarkerObjPtr;


///////////////////////////////////////////////////////////////////////////////
//
// Class MarkerObjType
//

class MarkerObjType : public MapObjType
{
  PROMOTE_LINK(MarkerObjType, MapObjType, 0x26E352A1); // "MarkerObjType"

public:

  // Constructor
  MarkerObjType(const char *name, FScope *fScope);

  // Create a new instance using this type
  GameObj* NewInstance(U32 id);
};


///////////////////////////////////////////////////////////////////////////////
//
// Class MarkerObj - Instance class for above type
//

class MarkerObj : public MapObj
{
public:

  // Constructor and destructor
  MarkerObj(MarkerObjType *objType, U32 id);
  ~MarkerObj();

  // Called to before deleting the object
  void PreDelete();

};

#endif  