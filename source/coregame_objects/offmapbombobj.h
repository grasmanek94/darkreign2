///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 15-JUN-1999
//

#ifndef __OffMapBombOBJ_H
#define __OffMapBombOBJ_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "offmapobj.h"
#include "explosionobj.h"
  

///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Forward delcarations
class OffMapBombObj;

// Reapers
typedef Reaper<OffMapBombObj> OffMapBombObjPtr;


///////////////////////////////////////////////////////////////////////////////
//
// Class OffMapBombObjType - De La OffMapBomb
//
class OffMapBombObjType : public OffMapObjType
{
  PROMOTE_LINK(OffMapBombObjType, OffMapObjType, 0x5C18E5D0); // "OffMapBombObjType"

public:

  // Explosion
  ExplosionObjTypePtr explosion;

  // Height to detonate above ground
  F32 height;

  // Constructor and destructor
  OffMapBombObjType(const char *typeName, FScope *fScope);
  ~OffMapBombObjType();

  // Called after all types are loaded
  void PostLoad();

  // Create a new instance using this type
  GameObj* NewInstance(U32 id);
};


///////////////////////////////////////////////////////////////////////////////
//
// Class OffMapBombObj - Instance class for above type
//

class OffMapBombObj : public OffMapObj
{
public:
  
  // Constructor and destructor
  OffMapBombObj(OffMapBombObjType *objType, U32 id);
  ~OffMapBombObj();

  // Execute an operation (TRUE if accepted)
  Bool Execute(U32 operation, const Vector &pos);

  // Get pointer to type
  OffMapBombObjType * OffMapBombType()
  {
    // This is a safe cast
    return ((OffMapBombObjType *)type);
  }
};

#endif