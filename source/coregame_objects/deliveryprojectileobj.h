///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 30-SEP-1998
//


#ifndef __DELIVERYPROJECTILEOBJ_H
#define __DELIVERYPROJECTILEOBJ_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "unitobjdec.h"
#include "projectileobj.h"
#include "promote.h"
#include "target.h"
#include "parasiteobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Forward Declarations
class Team;


///////////////////////////////////////////////////////////////////////////////
//
// Class DeliveryProjectileObjType - Base type class for all projectile objects
//
class DeliveryProjectileObjType : public ProjectileObjType
{
  PROMOTE_LINK(DeliveryProjectileObjType, ProjectileObjType, 0xE6394477); // "DeliveryProjectileObjType"

private:

  // Get the type to be delivered
  UnitObjTypePtr deliveryType;

  // The number of objects to deliver
  U32 count;

  // The velocity to throw them at
  F32 velocity;

  // The parasite to attach
  Reaper<ParasiteObjType> parasite;

public:

  // Constructor and Destructor
  DeliveryProjectileObjType(const char *typeName, FScope *fScope);
  ~DeliveryProjectileObjType();

  // PostLoad
  void PostLoad();

  // Initialized type specific resources
  Bool InitializeResources();

  // Create a new instance using this type
  GameObj* NewInstance(U32 id);

public:

  //
  // Friends of ProjectileObjType
  //
  friend class DeliveryProjectileObj;

  // Get the parasite to attach, or NULL
  ParasiteObjType * GetParasite()
  {
    return (parasite.GetPointer());
  }
};


///////////////////////////////////////////////////////////////////////////////
//
// Class DeliveryProjectileObj - Instance class for above type
//
class DeliveryProjectileObj : public ProjectileObj
{
protected:


public:

  // Constructor and destructor
  DeliveryProjectileObj(DeliveryProjectileObjType *objType, U32 id);
  ~DeliveryProjectileObj();

  // Called to before deleting the object
  void PreDelete();

public:

  // Detonate the projectile
  void Detonate();

public:

  // Get pointer to type
  DeliveryProjectileObjType* DeliveryProjectileType()
  {
    // This is a safe cast
    return ((DeliveryProjectileObjType *)type);
  }

};

#endif