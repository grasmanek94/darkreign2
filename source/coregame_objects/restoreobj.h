///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 06-JAN-1999
//

#ifndef __RESTOREOBJ_H
#define __RESTOREOBJ_H


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

// Forward delcarations
class RestoreObj;

// Reapers
typedef Reaper<RestoreObj> RestoreObjPtr;


/////////////////////////////////////////////////////////////////////////////
//
// Namespace RestoreObjNotify - Notification CRC's
//
namespace RestoreObjNotify
{
  // Does a task need to block hitpoint restoration
  const U32 BlockHitPoints = 0x729E044A; // "RestoreObj::BlockHitPoints"
}


///////////////////////////////////////////////////////////////////////////////
//
// Class RestoreObjType - A unit that restores hitpoints on other units
//
class RestoreObjType : public UnitObjType
{
  PROMOTE_LINK(RestoreObjType, UnitObjType, 0xD2ADB9B9); // "RestoreObjType"

  // Target properties
  PropertyList<16> propertyList;

  // Number of hitpoints to add each process
  U32 addHitPoints;

  // Reload rate
  F32 reloadRate;

  // The distance at which to start restoration
  F32 distance;

  // Is this a static restorer
  Bool isStatic;

  // Restore Callback
  static Bool RestoreCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *context);

public:

  // Constructor and destructor
  RestoreObjType(const char *typeName, FScope *fScope);
  ~RestoreObjType();

  // Called after all types are loaded
  void PostLoad();

  // Create a new instance using this type
  GameObj* NewInstance(U32 id);

  // Can the given type be restored by this restorer
  Bool CheckRestore(GameObjType *type);

  // Start the restoration FX
  void StartRestoreFX(UnitObj *obj);

  // Get the max number of hitpoints to add per cycle
  U32 GetAddHitPoints()
  {
    return (addHitPoints);
  }

  // Get the reloading rate
  F32 GetReloadRate()
  {
    return (reloadRate);
  }

  // Is this a static restorer
  Bool IsStatic()
  {
    return (isStatic);
  }

  // Restoration distance
  F32 Distance()
  {
    return (distance);
  }
};


///////////////////////////////////////////////////////////////////////////////
//
// Class RestoreObj - Instance class for above type
//

class RestoreObj : public UnitObj
{
public:

  // Does the given object require restoration
  static Bool AnyRestoreRequired(UnitObj *unit);

  // Constructor and destructor
  RestoreObj(RestoreObjType *objType, U32 id);
  ~RestoreObj();

  // Can the given object ever be restored by this object
  Bool CanRestore(UnitObj *obj);

  // Does the given object require restoring by this object
  Bool RestoreRequired(UnitObj *obj);

  // Do one restoration process on the given unit
  void Restore(UnitObj *obj);

  // Start restore FX for this object
  void StartRestoreFX()
  {
    RestoreType()->StartRestoreFX(this);
  }

  // Get pointer to type
  RestoreObjType * RestoreType()
  {
    // This is a safe cast
    return ((RestoreObjType *)type);
  }

  // Is this a static restorer
  Bool IsStatic()
  {
    return (RestoreType()->IsStatic());
  }
};

#endif