///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 14-NOV-1999
//

#ifndef __SPYOBJ_H
#define __SPYOBJ_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "unitobj.h"
#include "propertylist.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

OBJDEC(SpyObj)


///////////////////////////////////////////////////////////////////////////////
//
// Class SpyObjType - A unit that spies
//
class SpyObjType : public UnitObjType
{
  PROMOTE_LINK(SpyObjType, UnitObjType, 0x16897910) // "SpyObjType"

protected:

  // Resources to steal per cycle
  U32 resourceRate;

  // Power to steal per cycle
  U32 powerRate;

  // Percentage of facility's power to reduce by
  F32 powerLevel;

  // Time delay from being compromised to being executed
  U32 surrenderCycles;

  // What units can this spy morph into
  PropertyList<16> properties;

public:

  // Constructor and destructor
  SpyObjType(const char *typeName, FScope *fScope);
  ~SpyObjType();

  // Called after all types are loaded
  void PostLoad();

  // Initialized type specific resources
  Bool InitializeResources();

  // Create a new instance using this type
  GameObj* NewInstance(U32 id);

  // Calculate power to steal for a given unit type
  U32 CalcPowerLeak(UnitObjType *type);

  // Resources to steal per cycle
  U32 GetResourceRate()
  {
    return (resourceRate);
  }

  // Time delay from being compromised to being executed
  U32 GetSurrenderCycles()
  {
    return (surrenderCycles);
  }

  // GetPowerRate
  U32 GetPowerRate()
  {
    return (powerRate);
  }

  // GetPowerLevel
  F32 GetPowerLevel()
  {
    return (powerLevel);
  }

  // True if this spy can morph into the given type
  Bool CanMorph(UnitObjType *type)
  {
    return (!properties.GetCount() || properties.Test(type));
  }
};


///////////////////////////////////////////////////////////////////////////////
//
// Class SpyObj - Instance class for above type
//

class SpyObj : public UnitObj
{
public:

  // Spy list node
  NList<SpyObj>::Node spyNode;

  // List of all spies
  static NList<SpyObj> allSpies;

protected:

  // The team this spy is morphed into
  Team *morphTeam;

  // The unit type this spy is morphed into
  UnitObjTypePtr morphType;

public:
  
  // Constructor and destructor
  SpyObj(SpyObjType *objType, U32 id);
  ~SpyObj();

  // Called to before deleting the object
  void PreDelete();

  // Load and save state configuration
  void LoadState(FScope *fScope);
  virtual void SaveState(FScope *fScope, MeshEnt * theMesh = NULL);

  // Send an Event
  Bool SendEvent(const Task::Event &event, Bool idle);

  // Can the spy morph into another unit right now
  Bool CanMorph(UnitObj *unit);

  // Set or clear the morph target data
  void SetMorphTarget(UnitObj *unit = NULL);

  // Get the morph team of this spy, or NULL if not morphed
  Team * GetMorphTeam();

  // Get pointer to type
  SpyObjType * SpyType()
  {
    // This is a safe cast
    return ((SpyObjType *)type);
  }

};

#endif
