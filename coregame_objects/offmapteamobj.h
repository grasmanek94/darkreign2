///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 15-JUN-1999
//

#ifndef __OffMapTeamOBJ_H
#define __OffMapTeamOBJ_H


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
class OffMapTeamObj;

// Reapers
typedef Reaper<OffMapTeamObj> OffMapTeamObjPtr;


///////////////////////////////////////////////////////////////////////////////
//
// Class OffMapTeamObjType - De La OffMapTeam
//
class OffMapTeamObjType : public OffMapObjType
{
  PROMOTE_LINK(OffMapTeamObjType, OffMapObjType, 0xE9BC7F91); // "OffMapTeamObjType"

public:

  // Instance modifier list
  ApplyModifierList modifiers;

  // Constructor and destructor
  OffMapTeamObjType(const char *typeName, FScope *fScope);
  ~OffMapTeamObjType();

  // Called after all types are loaded
  void PostLoad();

  // Create a new instance using this type
  GameObj* NewInstance(U32 id);
};


///////////////////////////////////////////////////////////////////////////////
//
// Class OffMapTeamObj - Instance class for above type
//

class OffMapTeamObj : public OffMapObj
{
public:
  
  // Constructor and destructor
  OffMapTeamObj(OffMapTeamObjType *objType, U32 id);
  ~OffMapTeamObj();

  // Execute an operation (TRUE if accepted)
  Bool Execute(U32 operation, const Vector &pos);

  // Get pointer to type
  OffMapTeamObjType * OffMapTeamType()
  {
    // This is a safe cast
    return ((OffMapTeamObjType *)type);
  }
};

#endif