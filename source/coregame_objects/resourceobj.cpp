///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-APR-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "resourceobj.h"
#include "stdload.h"
#include "team.h"
#include "gameobjctrl.h"
#include "savegame.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Config scope name for type and world files
#define SCOPE_CONFIG   "ResourceObj"


///////////////////////////////////////////////////////////////////////////////
//
// Class ResourceObjType
//


// List of all resource types
NList<ResourceObjType> ResourceObjType::list(&ResourceObjType::node);


//
// ResourceObjType::ResourceObjType
//
// Constructor
//
ResourceObjType::ResourceObjType(const char *name, FScope *fScope) : 
  MapObjType(name, fScope)
{
  // Get specific config scope
  fScope = fScope->GetFunction(SCOPE_CONFIG);

  // Load the resource max
  resourceMax = StdLoad::TypeU32(fScope, "ResourceMax", 0, Range<U32>::positive);
  resourceMaxInv = resourceMax ? 1.0f / F32(resourceMax) : 0.0f;

  // Load the resource change
  resourceChange = StdLoad::TypeU32(fScope, "ResourceChange", resourceMax / 10, Range<U32>(0, resourceMax));

  // Load the resource regen rate
  resourceRate = StdLoad::TypeU32(fScope, "ResourceRate", 0, Range<U32>::positive);

  // Add to the type list
  list.Append(this);
}


//
// ResourceObjType::~ResourceObjType
//
// Destructor
//
ResourceObjType::~ResourceObjType()
{
  // Remove from the type list
  list.Unlink(this);
}


//
// ResourceObjType::NewInstance
//
// Create a new map object instance using this type
//
GameObj* ResourceObjType::NewInstance(U32 id)
{
  // Allocate new object instance
  return (new ResourceObj(this, id));
}


//
// EnableAllRegen
//
// Enable h@X0r multiplayer resource option
//
void ResourceObjType::EnableAllRegen(U32 multiplier, U32 absolute)
{
  for (NList<ResourceObjType>::Iterator i(&list); *i; ++i)
  {
    ResourceObjType &r = **i;

    if (r.resourceRate)
    {
      r.resourceRate *= multiplier;
    }
    else
    {
      r.resourceRate = absolute;
    }
  }
}



///////////////////////////////////////////////////////////////////////////////
//
// Class ResourceObj - Instance class for above type
//

//
// ResourceObj::ResourceObj
//
// Constructor
//
ResourceObj::ResourceObj(ResourceObjType *objType, U32 id) : 
  MapObj(objType, id),
  teamsCanSee(0),
  teamsHaveSeen(0),
  resource(ResourceType()->GetResourceMax())
{
}


//
// ResourceObj::~ResourceObj
//
// Destructor
//
ResourceObj::~ResourceObj()
{
}


//
// ResourceObj::PreDelete
//
// Pre deletion cleanup
//
void ResourceObj::PreDelete()
{

  // Call parent scope last
  MapObj::PreDelete();
}


//
// ResourceObj::SaveState
//
// Save a state configuration scope
//
void ResourceObj::SaveState(FScope *fScope, MeshEnt * theMesh) // = NULL)
{
  // Call parent scope first
  MapObj::SaveState(fScope);

  if (SaveGame::SaveActive())
  {
    // Create our specific config scope
    fScope = fScope->AddFunction(SCOPE_CONFIG);

    // Save the sight bitfield
    StdSave::TypeU32(fScope, "TeamHaveSeen", teamsHaveSeen);

    // Save the amount of resource
    StdSave::TypePercentage(fScope, "ResourcePercent", ResourceType()->GetResourceMax(), resource);
  }
}


//
// ResourceObj::LoadState
//
// Load a state configuration scope
//
void ResourceObj::LoadState(FScope *fScope)
{
  // Call parent scope first
  MapObj::LoadState(fScope);

  if ((fScope = fScope->GetFunction(SCOPE_CONFIG, FALSE)) != NULL)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x5457F5AB: // "TeamHaveSeen"
          teamsHaveSeen = Game::TeamBitfield(StdLoad::TypeU32(sScope));
          break;

        case 0x7C8A86BB: // "ResourcePercent"
          resource = StdLoad::TypePercentage(sScope, ResourceType()->GetResourceMax());
          AdjustResource();
          break;
      }
    }
  }
}


//
// ResourceObj::Equip
//
// Equip the object
//
void ResourceObj::Equip()
{
  // Default the amount of resource to the maximum
  resource = ResourceType()->GetResourceMax();

  AdjustResource();

  // Call parent scope first
  MapObj::Equip();
}


//
// ResourceObj::CaptureMapHooks
//
// Capture and release map hooks
//
void ResourceObj::CaptureMapHooks(Bool capture)
{
  // Perform Map Object Processing
  MapObj::CaptureMapHooks(capture);

  if (capture)
  {
    ASSERT(currentCluster)

    // Hook the resource list
    currentCluster->resourceList.Append(this);

    // Update AI
    currentCluster->ai.AddResource(resource);
  }
  else
  {
    // Was the object in a cluster ?
    if (currentCluster)
    {
      // Unhook it from the resource list
      currentCluster->resourceList.Unlink(this);

      // Update AI
      currentCluster->ai.RemoveResource(resource);
    }
  }
}


//
// ResourceObj::SetCanSee
//
// This resource can now be seen by the given team
//
void ResourceObj::SetCanSee(U32 id)
{
  // Has this team seen us before ?
  if (!TestHaveSeen(id))
  {
    // Convert the id into a team
    Team *t = Team::Id2Team(id);
    ASSERT(t)

    // Signal team radio that we've been spotted
    t->GetRadio().Trigger(0xC3C503C8, Radio::Event(this)); // "ResourceSighted"
  }

  Game::TeamSet(teamsCanSee, id);
  Game::TeamSet(teamsHaveSeen, id);
}


//
// ResourceObj::ClearCanSee
//
// This resource cannot be seen by the given team
//
void ResourceObj::ClearCanSee(U32 id)
{
  Game::TeamClear(teamsCanSee, id);
}


//
// ResourceObj::TakeResource
//
// Take some resource
//
U32 ResourceObj::TakeResource(U32 want)
{
  if (want > resource)
  {
    want = resource;

    // If this resource doesn't regenerate then mark it for deletion
    if (!ResourceType()->resourceRate)
    {
      GameObjCtrl::MarkForDeletion(this);
    }
  }

  // Is this object in a cluster
  if (currentCluster)
  {
    currentCluster->ai.RemoveResource(want);
  }

  // Remove from the resource
  resource -= want;

  AdjustResource();

  // Return the amount that was available
  return (want);
}


//
// ResourceObj::GiveResource
//
// Give resource
//
U32 ResourceObj::GiveResource(U32 amount)
{
  // How much space is left
  ASSERT(resource <= ResourceType()->GetResourceMax())
  U32 left = ResourceType()->GetResourceMax() - resource;

  if (amount > left)
  {
    amount = left;
  }

  // Is this object in a cluster
  if (currentCluster)
  {
    currentCluster->ai.AddResource(amount);
  }

  // Add to the resource
  resource += amount;

  AdjustResource();

  // Return the amount actually added
  return (amount);
}


//
// ResourceObj::SetResource
//
// Set resource
//
void ResourceObj::SetResource(F32 percentage)
{
  // Is this object in a cluster
  if (currentCluster)
  {
    currentCluster->ai.RemoveResource(resource);
  }
  
  resource = U32(percentage * F32(ResourceType()->GetResourceMax()));

  AdjustResource();

  // Is this object in a cluster
  if (currentCluster)
  {
    currentCluster->ai.AddResource(resource);
  }
}


//
// Adjust resource display
//
void ResourceObj::AdjustResource()
{
  if (Mesh().curCycle)
  {
    Mesh().SetFrame((Mesh().curCycle->maxFrame - 1) * (1.0f - (F32(resource) * ResourceType()->GetResourceMaxInv())));
  }
}
