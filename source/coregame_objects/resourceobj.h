///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-APR-1998
//


#ifndef __RESOURCEOBJ_H
#define __RESOURCEOBJ_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes 
//
#include "mapobj.h"
#include "resourceobjdec.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Forward delcarations
class PropObj;


///////////////////////////////////////////////////////////////////////////////
//
// Class ResourceObjType
//
class ResourceObjType : public MapObjType
{
  PROMOTE_LINK(ResourceObjType, MapObjType, 0x4AE3488F) // "ResourceObjType"

public:

  // List of all resource types
  NList<ResourceObjType>::Node node;

private:

  // Maximum amount of resource this resource can have
  U32 resourceMax;
  F32 resourceMaxInv;

  // Amount of resource it contains which makes it more viable
  U32 resourceChange;

  // Rate at which resource regenerates
  U32 resourceRate;

protected:

  // List of all resource types
  static NList<ResourceObjType> list;

public:

  // Constructor and destructor
  ResourceObjType(const char *name, FScope *fScope);
  ~ResourceObjType();

  // Create a new instance using this type
  GameObj* NewInstance(U32 id);

public:

  // Enable h@X0r multiplayer resource option
  static void EnableAllRegen(U32 multiplier, U32 absolute);

public:

  U32 GetResourceMax()
  {
    return (resourceMax);
  }

  F32 GetResourceMaxInv()
  {
    return (resourceMaxInv);
  }

  U32 GetResourceChange()
  {
    return (resourceChange);
  }

  U32 GetResourceRate()
  {
    return (resourceRate);
  }

  // Friends of ResourceObjType
  friend ResourceObj;

};


///////////////////////////////////////////////////////////////////////////////
//
// Class ResourceObj
//
class ResourceObj : public MapObj
{
protected:

  // Amount of resource
  U32 resource;

  // Teams which can see this resource
  Game::TeamBitfield teamsCanSee;

  // Teams which have seen this resource
  Game::TeamBitfield teamsHaveSeen;

public:

  NList<ResourceObj>::Node resourceNode;

public:

  // Constructor and destructor
  ResourceObj(ResourceObjType *objType, U32 id);
  ~ResourceObj();

  // Called to before deleting the object
  void PreDelete();

  // Load and save state configuration
  virtual void SaveState(FScope *fScope, MeshEnt * theMesh = NULL);
  void LoadState(FScope *fScope);

  // Equip the object
  void Equip();

  // Capture and release map hooks
  void CaptureMapHooks(Bool capture);

  // This resource can now be seen by the given team
  void SetCanSee(U32 team);

  // This resource cannot be seen by the given team
  void ClearCanSee(U32 team);

  // Take some resource
  U32 TakeResource(U32 amount);

  // Give resource
  U32 GiveResource(U32 amount);

  // Set resource
  void SetResource(F32 percentage);

  // Adjust resource display
  void AdjustResource();

public:

  ResourceObjType * ResourceType()
  {
    // This is a safe cast
    return ((ResourceObjType *) type);
  }

  // Test to see if a team can see this resource
  Bool TestCanSee(U32 id)
  {
    return (Game::TeamTest(teamsCanSee, id));
  }

  // Test to see if a team has seen this resource
  Bool TestHaveSeen(U32 id)
  {
    return (Game::TeamTest(teamsHaveSeen, id));
  }

  // Get the amount of resource
  U32 GetResource()
  {
    return (resource);
  }

};

#endif
