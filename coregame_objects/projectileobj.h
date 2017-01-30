///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 30-SEP-1998
//


#ifndef __PROJECTILEOBJ_H
#define __PROJECTILEOBJ_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "projectileobjdec.h"
#include "explosionobjdec.h"
#include "unitobjdec.h"
#include "mapobj.h"
#include "promote.h"
#include "target.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Forward Declarations
class Team;


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace ProjectileModel
//
namespace ProjectileModel
{
  const U32 StraightTrajectory  = 0x9C75500F; // "StraightTrajectory"
  const U32 ArcTrajectory       = 0x39869B34; // "ArcTrajectory"
  const U32 GuidedTrajectory    = 0xE7C98DAD; // "GuidedTrajectory"
}


///////////////////////////////////////////////////////////////////////////////
//
// Class ProjectileObjType - Base type class for all projectile objects
//
class ProjectileObjType : public MapObjType
{
  PROMOTE_LINK(ProjectileObjType, MapObjType, 0x5050D296); // "ProjectileObjType"

public:

  // Projectile model
  U32 model;

  // Projectile explodes on impact
  U32 impact : 1;
  
  // Proximity type
  enum ProximityType { PROXIMITY_NORMAL, PROXIMITY_XZ, PROXIMITY_Y } proximityType;

  // Length of the fuse (s)
  F32 fuse;                             

  // Distance at which it explodes anyway
  F32 proximity;                        

  // Proximity squared
  F32 proximity2;                       

  // Model specific settings
  union
  {
    struct
    {
      // Top speed in m/s
      F32 topSpeed;

      // Acceleration in m/s/s
      F32 acceleration;

      // Homing rate in rads/sec
      F32 homingRate;
      F32 cosHomingRate;

      // Waver rate in rads/sec
      F32 waverTurn;
      F32 waverRate;

    } missile;
  };

  // Explosion
  ExplosionObjTypePtr explosionType;

  // Pointer to the simulation function
  void (ProjectileObjType::*projectilePhysicsProc)(ProjectileObj &, Matrix &, Vector &);

protected:

  // Simulate a bullet, no effects of friction or gravity
  void BulletPhysics(ProjectileObj &obj, Matrix &m, Vector &s);

  // Simulate a grenade, which includes gravity effects
  void GrenadePhysics(ProjectileObj &obj, Matrix &m, Vector &s);

  // Simulate a missile, which includes acceleration, turning etc
  void MissilePhysics(ProjectileObj &obj, Matrix &m, Vector &s);

  // Simulation function
  void ProcessProjectilePhysics(ProjectileObj &obj, Matrix &m, Vector &s)
  {
    (this->*projectilePhysicsProc)(obj, m, s);
  }

public:

  // Constructor and Destructor
  ProjectileObjType(const char *typeName, FScope *fScope);
  ~ProjectileObjType();

  // PostLoad
  void PostLoad();

  // Initialized type specific resources
  Bool InitializeResources();

  // Create a new instance using this type
  GameObj* NewInstance(U32 id);

public:

  // Are instances of this type on the primitive list by default
  Bool DefaultPrimitiveProcessing()
  {
    return (TRUE);
  }

  // Get the projectile model
  U32 GetModel()
  {
    return (model);
  }

  // Does the projectile explode on impact ?
  Bool GetImpact()
  {
    return (impact);
  }

  // What's the length of the projectiles fuse ?
  F32 GetFuse()
  {
    return (fuse);
  }

  // What's the distance that the projectile explodes at anyhow ?
  F32 GetProximity()
  {
    return (proximity);
  }
  F32 GetProximity2()
  {
    return (proximity2);
  }

  // Returns the proximity type
  ProximityType GetProximityType()
  {
    return (proximityType);
  }

  // Returns the explosion type
  ExplosionObjType * GetExploisionType()
  {
    return (explosionType.GetPointer());
  }

  //
  // Friends of ProjectileObjType
  //
  friend class ProjectileObj;
};


///////////////////////////////////////////////////////////////////////////////
//
// Class ProjectileObj - Instance class for above type
//
class ProjectileObj : public MapObj
{
protected:

  // The unit that fired this projectile
  UnitObjPtr source;

  // The position where this projectile started
  Vector startPosit;

  // Team which fired this projectils
  Team *sourceTeam;

  // The weapon type that fired the projectile
  Weapon::Type *weaponType;

  // Target of the projectile
  Target target;

  // Distance travelled
  F32 distTravelled;

  // Current damage value of weapon
  S32 damage;

  // Weapon Id of firing weapon
  U32 firingWeaponId;

  // Has this projectile detonated
  U32 detonated : 1,

  // Was target behind homing missile last cycle
      wasBehind : 1;

  // Angle last used for homing missile
  F32 lastAngle;

  // Waver pitch and yaw
  F32 waverPitch;
  F32 waverYaw;

public:

  // Constructor and destructor
  ProjectileObj(ProjectileObjType *objType, U32 id);
  ~ProjectileObj();

  // Called to before deleting the object
  void PreDelete();

  // Per-cycle processing
  void ProcessCycle();

  // Load and save state configuration
  void LoadState(FScope *fScope);
  virtual void SaveState(FScope *fScope, MeshEnt * theMesh = NULL);

  // Called after all objects are loaded
  void PostLoad();

  // Setup a created projectile
  void Setup(UnitObj *sourceIn, Team *teamIn, Weapon::Type *weaponType, const Target &targetIn, S32 damageIn, F32 speed);

  // AddToMapHook
  void AddToMapHook();

  // The projectile has collided with another object or the terrain
  void Collided(MapObj *with, const Vector *veloc);

  // Detonate the projectile
  virtual void Detonate();

public:

  // FX Callbacks

  // Fire Trailback
  static Bool TrailCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *context);

public:

  // Get the weapon type associated with this projectile
  Weapon::Type * GetWeaponType()
  {
    return (weaponType);
  }

  // Get the object which fired this projectile
  UnitObj * GetSourceUnit()
  {
    return (source.Alive() ? source.GetData() : NULL);
  }

  // Returns the source team
  Team * GetSourceTeam()
  {
    return (sourceTeam);
  }

  // Get pointer to type
  ProjectileObjType* ProjectileType()
  {
    // This is a safe cast
    return ((ProjectileObjType *)type);
  }

  // GetDamage: Amount of damage this weapon does 
  // against the specifed armour class per second
  S32 GetDamage(U32 armourClass)
  {
    return (damage * ArmourClass::Lookup(firingWeaponId, armourClass) >> 16);
  }

  // Get the target of the projectile
  const Target & GetTarget()
  {
    return (target);
  }

  //
  // Friends of ProjectileObj
  //
  friend class ProjectileObjType;

};

#endif