///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 06-JAN-1999
//

#ifndef __TRANSPORTOBJ_H
#define __TRANSPORTOBJ_H


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

OBJDEC(TransportObj)

namespace Tasks
{
  class TransportPad;
}


///////////////////////////////////////////////////////////////////////////////
//
// Class TransportObjType - A unit that transports other units
//
class TransportObjType : public UnitObjType
{
  PROMOTE_LINK(TransportObjType, UnitObjType, 0x3F369977); // "TransportObjType"

  // Target properties
  PropertyList<16> propertyList;

  // Number of cargo spaces available
  U32 spaces;

  // Distance to unit before being sucked in
  F32 distance;

  // Recharge time for transport pads
  F32 chargeTime;

  // The portal type
  Reaper<TransportObjType> portalType;

  // The time a portal is open
  F32 portalTime;

public:

  // Constructor and destructor
  TransportObjType(const char *typeName, FScope *fScope);
  ~TransportObjType();

  // Called after all types are loaded
  void PostLoad();

  // Initialized type specific resources
  Bool InitializeResources();

  // Create a new instance using this type
  GameObj* NewInstance(U32 id);

  // Can the given type be transported by this transport
  Bool CheckTransport(UnitObjType *type);

  U32 GetSpaces()
  {
    return (spaces);
  }

  F32 GetDistance()
  {
    return (distance);
  }

  F32 GetChargeTime()
  {
    return (chargeTime);
  }

  F32 GetPortalTime()
  {
    return (portalTime);
  }

  TransportObjType * GetPortalType()
  {
    return (portalType.GetPointer());
  }

};


///////////////////////////////////////////////////////////////////////////////
//
// Class TransportObj - Instance class for above type
//

class TransportObj : public UnitObj
{
  // List of units being transported
  UnitObjList cargo;

  // The telepad if this is a portal
  TransportObjPtr telepadLink;

public:
  
  // Constructor and destructor
  TransportObj(TransportObjType *objType, U32 id);
  ~TransportObj();

  // Called to before deleting the object
  void PreDelete();

  // Load and save state configuration
  void LoadState(FScope *fScope);
  virtual void SaveState(FScope *fScope, MeshEnt * theMesh = NULL);

  // Called after all objects are loaded
  void PostLoad();

  // Get the number of spaces currently used
  U32 GetUsedSpaces();

  // Get the number of free spaces
  U32 GetFreeSpaces();

  // Can the given unit be transported by this type
  Bool CheckType(UnitObj *unit);

  // Can the given unit currently fit inside this transport
  Bool CheckSpace(UnitObj *unit);

  // Checks both type and space
  Bool CheckCargo(UnitObj *unit);

  // Pickup the given cargo item
  void PickupCargo(UnitObj *unit);

  // Add the given cargo
  void AddCargo(UnitObj *unit);

  // Is this transporter able to unload right now
  Bool UnloadAvailable();

  // Check if this transport can currently unload at the given location
  Bool CheckUnload(U32 x, U32 z);

  // Unload a single unit at the given location
  Bool Unload(const Vector &destination, UnitObj *unit);

  // Unload cargo at the given location (TRUE if any unloaded)
  Bool Unload(const Vector &destination, Bool single = FALSE);

  // Get the transport pad task, or NULL
  Tasks::TransportPad * GetPadTask();

  // Get the pad pointer
  TransportObj * GetTelepadLink()
  {
    return (telepadLink.GetPointer());
  }

  // Set the pad reaper
  void SetTelepadLink(TransportObj *p)
  {
    telepadLink = p;
  }

  // Get pointer to type
  TransportObjType * TransportType()
  {
    // This is a safe cast
    return ((TransportObjType *)type);
  }
};

#endif