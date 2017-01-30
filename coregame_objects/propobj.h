///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-APR-1998
//

#ifndef __PROPOBJ_H
#define __PROPOBJ_H


#include "mapobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Forward delcarations
class PropObj;


///////////////////////////////////////////////////////////////////////////////
//
// Class PropObjType - An object that just sits and does nout
//

class PropObjType : public MapObjType
{
  PROMOTE_LINK(PropObjType, MapObjType, 0x61FD726C); // "PropObjType"

public:

  // Constructor
  PropObjType(const char *name, FScope *fScope);

  // Create a new instance using this type
  GameObj* NewInstance(U32 id);

};


///////////////////////////////////////////////////////////////////////////////
//
// Class PropObj - Instance class for above type
//

class PropObj : public MapObj
{
public:

  // Constructor and destructor
  PropObj(PropObjType *objType, U32 id);
  ~PropObj();

};

#endif