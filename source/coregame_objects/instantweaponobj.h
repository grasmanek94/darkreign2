///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-APR-1998
//


#ifndef __INSTANTWEAPONOBJ_H
#define __INSTANTWEAPONOBJ_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "weaponobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class WeaponObjType - Base type class for all weapon objects
//
class InstantWeaponObjType : public WeaponObjType
{
  PROMOTE_LINK(InstantWeaponObjType, WeaponObjType, 0x4AD85F18); // "InstantWeaponObjType"

private:

  // Delay between firing (s)
  InstanceModifierType delay;

  // Fire rate (1/s) (16/16)
  U32 rate;

  // Particles which get released when the weapon is firing
  FX::Type *fireFX;

  // Particles on the target
  FX::Type *targetFX;

  // Barrel length
  F32 barrelLength;

public:

  // Constructor and Destructor
  InstantWeaponObjType(const char *typeName, FScope *fScope);
  ~InstantWeaponObjType();

  // Initialize Resources
  InitializeResources();

  // Create a new instance using this type
  GameObj* NewInstance(U32 id);

  // GetThreat: Amount of damage this weapon does against the specifed armour class per second
  U32 GetThreat(U32 armourClass) 
  {
    return ((damage.GetInteger() * ArmourClass::Lookup(weaponId, armourClass) >> 16) * rate >> 16);
  }

  // Get the delay between firing for this weapon
  F32 GetDelay()
  {
    return (delay.GetFPoint());
  }

  // Instance modifiers
  const InstanceModifierType &DelayIM()
  {
    return (delay);
  }

  // Get the length of the barrel
  F32 GetBarrelLength()
  {
    return (barrelLength);
  }

public:

  //
  // Friends of InstantWeaponObjType
  //
  friend class InstantWeaponObj;

};


///////////////////////////////////////////////////////////////////////////////
//
// Class InstantWeaponObj - Instance class for above type
//
class InstantWeaponObj : public WeaponObj
{
private:

  // The target
  Vector destination;

  // Last time the weapon released
  F32 lastReleaseTime;

  // Instance modified values
  InstanceModifier delay;

public:

  // Constructor and destructor
  InstantWeaponObj(InstantWeaponObjType *objType, U32 id);
  ~InstantWeaponObj();

  // Calculate a firing solution
  WeaponObj::Solution FiringSolution(Target &target, const Vector &unitFront, Vector &solution, F32 &fParam);

  // Fire the weapon
  void Fire(const Vector &solution, UnitObj *source, const Target &target, WeaponObjType::FirePoint *firePoint);

  // Get the delay between firing for this weapon
  F32 GetDelay();

  // Ready to fire ?
  Bool ReadyToFire();

public:

  // Stop firing the weapon
  void HaltFire() { };

  // Get pointer to type
  InstantWeaponObjType * InstantWeaponType()
  {
    // This is a safe cast
    return ((InstantWeaponObjType *)type);
  }

  // Instance modifiers
  InstanceModifier &DelayIM()
  {
    return (delay);
  }

public:

  // FX Callbacks
  static Bool FireCallBack(MapObj *mapObj, FX::CallBackData &cbd);

};

#endif