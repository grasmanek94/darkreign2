///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 15-JUN-1999
//

#ifndef __OffMapStrikeOBJ_H
#define __OffMapStrikeOBJ_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "offmapobj.h"
  

///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Forward delcarations
class OffMapStrikeObj;

// Reapers
typedef Reaper<OffMapStrikeObj> OffMapStrikeObjPtr;


///////////////////////////////////////////////////////////////////////////////
//
// Class OffMapStrikeObjType - De La OffMapStrike
//
class OffMapStrikeObjType : public OffMapObjType
{
  PROMOTE_LINK(OffMapStrikeObjType, OffMapObjType, 0xDD9107D7); // "OffMapStrikeObjType"

public:

  GameIdent weaponName;
  Weapon::Type *weapon;
  F32 height;

  // Constructor and destructor
  OffMapStrikeObjType(const char *typeName, FScope *fScope);
  ~OffMapStrikeObjType();

  // Called after all types are loaded
  void PostLoad();

  // Create a new instance using this type
  GameObj* NewInstance(U32 id);
};


///////////////////////////////////////////////////////////////////////////////
//
// Class OffMapStrikeObj - Instance class for above type
//

class OffMapStrikeObj : public OffMapObj
{
public:
  
  // Constructor and destructor
  OffMapStrikeObj(OffMapStrikeObjType *objType, U32 id);
  ~OffMapStrikeObj();

  // Execute an operation (TRUE if accepted)
  Bool Execute(U32 operation, const Vector &pos);

  // Get pointer to type
  OffMapStrikeObjType * OffMapStrikeType()
  {
    // This is a safe cast
    return ((OffMapStrikeObjType *)type);
  }
};

#endif