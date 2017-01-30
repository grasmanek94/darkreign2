///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 06-JAN-1999
//

#ifndef __PARASITEOBJ_H
#define __PARASITEOBJ_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "unitobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Forward delcarations
class ParasiteObj;

// Reapers
typedef Reaper<ParasiteObj> ParasiteObjPtr;


///////////////////////////////////////////////////////////////////////////////
//
// Class ParasiteObjType - An object that attaches to another to do evil
//
class ParasiteObjType : public UnitObjType
{
  PROMOTE_LINK(ParasiteObjType, UnitObjType, 0x46588AF7); // "ParasiteObjType"

protected:

  // Generic timer values
  F32 time1, time2;

  // Should radio events be triggered
  Bool useRadio;

public:

  // Constructor and destructor
  ParasiteObjType(const char *typeName, FScope *fScope);
  ~ParasiteObjType();

  // Called after all types are loaded
  void PostLoad();

  // Create a new instance using this type
  GameObj* NewInstance(U32 id);

  // Infect the given object with this parasite (FALSE if already infected)
  Bool Infect(UnitObj *target, Team *team);

  F32 GetTime1()
  {
    return (time1);
  }

  F32 GetTime2()
  {
    return (time2);
  }

  Bool GetUseRadio()
  {
    return (useRadio);
  }
};


///////////////////////////////////////////////////////////////////////////////
//
// Class ParasiteObj - Instance class for above type
//

class ParasiteObj : public UnitObj
{
  // The target unit
  UnitObjPtr target;

public:
  
  // Constructor and destructor
  ParasiteObj(ParasiteObjType *objType, U32 id);
  ~ParasiteObj();

  // Called to before deleting the object
  void PreDelete();

  // Load and save state configuration
  void LoadState(FScope *fScope);
  virtual void SaveState(FScope *fScope, MeshEnt * theMesh = NULL);

  // Called after all objects are loaded
  void PostLoad();

  // Sets the target
  void SetTarget(UnitObj *t);

  // Returns the target, or NULL if not alive
  UnitObj * GetTarget()
  {
    return (target.GetPointer());
  }

  // Get pointer to type
  ParasiteObjType * ParasiteType()
  {
    // This is a safe cast
    return ((ParasiteObjType *)type);
  }
};

#endif