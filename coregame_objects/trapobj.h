///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 06-JAN-1999
//

#ifndef __TRAPOBJ_H
#define __TRAPOBJ_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "unitobj.h"
#include "parasiteobj.h"
#include "propertylist.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Forward delcarations
class TrapObj;

// Reapers
typedef Reaper<TrapObj> TrapObjPtr;


///////////////////////////////////////////////////////////////////////////////
//
// Class TrapObjType - Fixed objects that trigger on detection of enemy
//
class TrapObjType : public UnitObjType
{
  PROMOTE_LINK(TrapObjType, UnitObjType, 0x21CF8E02); // "TrapObjType"

protected:

  // Distance to target before triggering
  F32 distance;

  // Recharge time used in some traps
  F32 chargeTime;

  // Does this trap self destruct
  Bool selfDestruct;

  // The speed at which to fire a weapon
  F32 weaponSpeed;

  // The parasite to attach
  Reaper<ParasiteObjType> parasite;

  // Target properties
  PropertyList<16> properties;

public:

  // Constructor and destructor
  TrapObjType(const char *typeName, FScope *fScope);
  ~TrapObjType();

  // Called after all types are loaded
  void PostLoad();

  // Create a new instance using this type
  GameObj* NewInstance(U32 id);

  F32 GetDistance()
  {
    return (distance);
  }

  F32 GetChargeTime()
  {
    return (chargeTime);
  }

  Bool GetSelfDestruct()
  {
    return (selfDestruct);
  }

  F32 GetWeaponSpeed()
  {
    return (weaponSpeed);
  }

  ParasiteObjType * GetParasite()
  {
    return (parasite.GetPointer());
  }

  Bool Test(UnitObj *obj)
  {
    return (properties.Test(obj->GameType()));
  }
};


///////////////////////////////////////////////////////////////////////////////
//
// Class TrapObj - Instance class for above type
//

class TrapObj : public UnitObj
{
protected:

  // The recharge timer
  GameTime::Timer recharge;

  // Trap list node
  NList<TrapObj>::Node node;

public:

  // All traps in existence
  static NList<TrapObj> allTraps;

public:
  
  // Constructor and destructor
  TrapObj(TrapObjType *objType, U32 id);
  ~TrapObj();

  // Called to before deleting the object
  void PreDelete();

  // Load and save state configuration
  void LoadState(FScope *fScope);
  virtual void SaveState(FScope *fScope, MeshEnt * theMesh = NULL);

  // Attempt to trigger this trap
  Bool Poll();

  // Get pointer to type
  TrapObjType * TrapType()
  {
    // This is a safe cast
    return ((TrapObjType *)type);
  }
};

#endif