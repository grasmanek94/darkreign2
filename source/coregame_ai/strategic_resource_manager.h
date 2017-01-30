/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Resource
// 25-MAR-1999
//


#ifndef __STRATEGIC_RESOURCE_MANAGER_H
#define __STRATEGIC_RESOURCE_MANAGER_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_resource.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Resource::Manager
  //
  class Resource::Manager
  {
  private:

    // Strategic Object the manager belongs to
    Object *strategic;

    // The resource id counter
    U32 resourceId;

    // Resources (sorted by distance to our base vs distance to enemy base)
    NBinTree<Resource, F32> resources;

    // Transports which are currently unassigned
    UnitObjList resourceTransports;

  public:

    // Constructor and destructor
    Manager();
    ~Manager();

    // Reset
    void Reset();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope);

    // Process resource
    void Process();

    // Find a resource
    Resource * FindResource(U32 id);

  public:

    // Setup
    void Setup(Object *object)
    {
      strategic = object;
    }

    // Get the strategic object from the manager
    Object & GetObject()
    {
      ASSERT(strategic)
      return (*strategic);
    }

    // Get the resources
    const NBinTree<Resource, F32> & GetResources()
    {
      return (resources);
    }

  };

}

#endif
