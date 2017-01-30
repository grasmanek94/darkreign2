///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 15-JUN-1999
//

#ifndef __OFFMAPOBJ_H
#define __OFFMAPOBJ_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "offmapobjdec.h"
#include "unitobj.h"
  

///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Forward delcarations
class OffMapObj;

// Reapers
typedef Reaper<OffMapObj> OffMapObjPtr;


///////////////////////////////////////////////////////////////////////////////
//
// Class OffMapObjType - A unit that exists off the map
//
class OffMapObjType : public UnitObjType
{
  PROMOTE_LINK(OffMapObjType, UnitObjType, 0xD1FE616C); // "OffMapObjType"

  // Should the traction type be enforced for launching
  Bool checkTraction;

public:

  // Constructor and destructor
  OffMapObjType(const char *typeName, FScope *fScope);
  ~OffMapObjType();

  // Called after all types are loaded
  void PostLoad();

  // Create a new instance using this type
  GameObj* NewInstance(U32 id);

  // Get the check traction flag
  Bool GetCheckTraction()
  {
    return (checkTraction);
  }
};


///////////////////////////////////////////////////////////////////////////////
//
// Class OffMapObj - Instance class for above type
//

class OffMapObj : public UnitObj
{
protected:

  // Has this object been executed yet
  Bool executed;

  // Facility that is holding this object
  UnitObjPtr facility;

public:
  
  // Constructor and destructor
  OffMapObj(OffMapObjType *objType, U32 id);
  ~OffMapObj();

  // Called to before deleting the object
  void PreDelete();

  // Load and save state configuration
  void LoadState(FScope *fScope);
  virtual void SaveState(FScope *fScope, MeshEnt * theMesh = NULL);

  // Called after all objects are loaded
  void PostLoad();

  // Setup the facility
  void SetFacility(UnitObj *unit);

  // Check execution position
  virtual Bool Check(const Vector &pos);

  // Execute an operation (TRUE if accepted)
  virtual Bool Execute(U32 operation, const Vector &pos);

  // Called when the object has been executed and can be deleted (returns TRUE)
  Bool Done();

  // Get pointer to type
  OffMapObjType * OffMapType()
  {
    // This is a safe cast
    return ((OffMapObjType *)type);
  }

  // Return the assigned facility, or NULL
  UnitObj * GetFacility()
  {
    return (facility.GetPointer());
  }

  // Has this object been executed yet
  Bool Executed()
  {
    return (executed);
  }
};

#endif