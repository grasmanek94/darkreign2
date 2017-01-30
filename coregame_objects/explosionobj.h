///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 30-SEP-1998
//


#ifndef __EXPLOSIONOBJ_H
#define __EXPLOSIONOBJ_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "explosionobjdec.h"
#include "mapobj.h"
#include "damage.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Forward Declarations
class Team;


///////////////////////////////////////////////////////////////////////////////
//
// Class ExplosionObjType
//
class ExplosionObjType : public MapObjType
{
  PROMOTE_LINK(ExplosionObjType, MapObjType, 0x601EBAAA); // "ExplosionObjType"

public:

  // Damage information for this weapon
  Damage::Type damage;

  // Time for unit to blindly target any unit
  F32 blindTargetTime;

  // Area effect

  // Area where 100 % damage is done
  F32 areaInner;                        

  // Area where 100 % damage is done squared
  F32 areaInner2;               
  
  // Area where 0 % damage is done
  F32 areaOuter;                        

  // Area where partial damage is done squared inverted
  F32 areaDiff2Inv;                     

  // How long does the explosion persist for
  F32 persist;

  // Action scope to execute when detonated
  FScope *action;

public:

  // Constructor and Destructor
  ExplosionObjType(const char *typeName, FScope *fScope);
  ~ExplosionObjType();

  // PostLoad
  void PostLoad();

  // Initialized type specific resources
  Bool InitializeResources();

  // Create a new instance using this type
  GameObj* NewInstance(U32 id);

  // Detonate
  void Detonate(const Vector &location, UnitObj *sourceUnit, Team *sourceTeam);

  // Get the threat of this explosion to the given armour class
  U32 GetThreat(U32 armourClass);

private:

  // Apply
  void Apply(const Vector &location, UnitObj *unit, Team *team);

public:

  // Are instances of this type on the primitive list by default
  Bool DefaultPrimitiveProcessing()
  {
    return (TRUE);
  }

  // Range squared of minimum area efect
  F32 GetAreaInner2()
  {
    return (areaInner2);
  }

  // Range which objects are going to be damaged in
  F32 GetAreaOuter()
  {
    return (areaOuter);
  }

  // Range of difference between min squared and max squared inverted
  F32 GetAreaDiff2Inv()
  {
    return (areaDiff2Inv);
  }

  // Get the damage
  const Damage::Type & GetDamage()
  {
    return (damage);
  }

  //
  // Friends of ExplosionObjType
  //
  friend class ExplosionObj;
};


///////////////////////////////////////////////////////////////////////////////
//
// Class ExplosionObj - Instance class for above type
//
class ExplosionObj : public MapObj
{
protected:

  // Damage
  Damage::Object damage;
  
  // Unit which started this explosion
  UnitObjPtr sourceUnit;

  // Team which started this explosion (only used if unit is invalid)
  Team *sourceTeam;

public:

  // Constructor and destructor
  ExplosionObj(ExplosionObjType *objType, U32 id);
  ~ExplosionObj();

  // Called to before deleting the object
  void PreDelete();

  // Load and save state configuration
  void LoadState(FScope *fScope);
  virtual void SaveState(FScope *fScope, MeshEnt * theMesh = NULL);

  // Called after all objects are loaded
  void PostLoad();

  // AddToMapHook
  void AddToMapHook();

  // Per-cycle processing
  void ProcessCycle();

  // Render debug
  void RenderDebug();

public:

  // Get pointer to type
  ExplosionObjType* ExplosionType()
  {
    // This is a safe cast
    return ((ExplosionObjType *)type);
  }

public:

  // FX Callbacks
  static Bool ExplosionCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *context);

public:

  //
  // Friends of ExplosionObj
  //
  friend class ExplosionObjType;

};

#endif