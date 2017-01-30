///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 15-JUN-1999
//

#ifndef __OffMapSpawnOBJ_H
#define __OffMapSpawnOBJ_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "offmapobj.h"
#include "gametime.h"
#include "parasiteobj.h"
  

///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Forward delcarations
class OffMapSpawnObj;

// Reapers
typedef Reaper<OffMapSpawnObj> OffMapSpawnObjPtr;


///////////////////////////////////////////////////////////////////////////////
//
// Class OffMapSpawnObjType - De La OffMapSpawn
//
class OffMapSpawnObjType : public OffMapObjType
{
  PROMOTE_LINK(OffMapSpawnObjType, OffMapObjType, 0xA6908E6E); // "OffMapSpawnObjType"

  // The object type to spawn
  UnitObjTypePtr spawnType;

  // The optional parasite to attach
  Reaper<ParasiteObjType> parasiteType;

public:

  // Constructor and destructor
  OffMapSpawnObjType(const char *typeName, FScope *fScope);
  ~OffMapSpawnObjType();

  // Called after all types are loaded
  void PostLoad();

  // Initialized type specific resources
  Bool InitializeResources();

  // Create a new instance using this type
  GameObj* NewInstance(U32 id);

  UnitObjType * GetSpawnType()
  {
    return (spawnType.GetPointer());
  }

  ParasiteObjType * GetParasiteType()
  {
    return (parasiteType.GetPointer());
  }
};


///////////////////////////////////////////////////////////////////////////////
//
// Class OffMapSpawnObj - Instance class for above type
//

class OffMapSpawnObj : public OffMapObj
{
public:

  // Constructor and destructor
  OffMapSpawnObj(OffMapSpawnObjType *objType, U32 id);
  ~OffMapSpawnObj();

  // Initialized type specific resources
  Bool InitializeResources();

  // Check execution position
  Bool Check(const Vector &pos);

  // Execute an operation (TRUE if accepted)
  Bool Execute(U32 operation, const Vector &pos);

  // Get pointer to type
  OffMapSpawnObjType * OffMapSpawnType()
  {
    // This is a safe cast
    return ((OffMapSpawnObjType *)type);
  }
};

#endif