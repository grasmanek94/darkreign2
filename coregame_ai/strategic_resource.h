/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Resource
//


#ifndef __STRATEGIC_RESOURCE_H
#define __STRATEGIC_RESOURCE_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Forward Declarations
  //


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Resource
  //
  class Resource
  {
  public:

    /////////////////////////////////////////////////////////////////////////////
    //
    // Forward Declarations
    //
    class Manager;
    class Decomposition;
    struct ClusterGroup;

  private:

    // Resource manager which is managing this resource
    Manager &manager;

    // Manager node
    NBinTree<Resource, F32>::Node nodeManager;

    // The resource objects which make up this resource
    ResourceObjList resources;

    // The id of this resource
    U32 id;

    // The total amount of resource 
    U32 resource;

    // The total regeneration rate
    U32 regen;

    // Centre of this region
    Point<F32> centre;

    // Resource storage facilities assigned to this resource
    UnitObjList resourceStorers;

    // Resource transporters assigned to this resource
    UnitObjList resourceTransports;

    // Other units assigned to this resource
    UnitObjList units;

    // Units which are on order for this resource
    UnitObjTypeList onOrder;
   
    // The proximity to our base
    F32 baseProximity;

    // Last cycle we updated
    U32 updateCycle;

    // Map Clusters which this resource covers
    List<MapCluster> clusters;

  public:

    // Death track info
    DTrack::Info dTrack;

  public:

    // Constructors
    Resource(Manager &manager, const ClusterGroup &clusterGroup, F32 proximity, U32 id);
    Resource(Manager &manager, F32 proximity, U32 id);

    // Destructor
    ~Resource();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope);

    // Process
    Bool Process();

    // Add on Order
    void AddOnOrder(UnitObjType &type);

    // Remove type on Order
    void RemoveOnOrder(UnitObjType &type);

    // Add unit
    void AddUnit(UnitObj *unit);

  public:

    // Get the script manager in control of this script
    Manager & GetManager()
    {
      return (manager);
    }

    // Get the id of this resource
    U32 GetId()
    {
      return (id);
    }

    // Get the clusters this resource covers
    const List<MapCluster> & GetClusters() const
    {
      return (clusters);
    }

    // Get the resources which this resource covers
    const ResourceObjList & GetResources() const
    {
      return (resources);
    }

    // Get the total amount of resource
    U32 GetResource() const
    {
      return (resource);
    }

    // Get the total regeneration rate
    U32 GetRegen() const
    {
      return (regen);
    }

    // Get the proximity to our primary base
    F32 GetBaseProximity() const
    {
      return (baseProximity);
    }

    // Get the update cycle
    U32 GetUpdateCycle() const
    {
      return (updateCycle);
    }

    // Get the centre of this resource
    const Point<F32> & GetCentre() const
    {
      return (centre);
    }

    // Get the list of resource storers assigned to this resource
    const UnitObjList & GetResourceStorers() const
    {
      return (resourceStorers);
    }

    // Get the list of resource transporters assigned to this resource
    const UnitObjList & GetResourceTransports() const
    {
      return (resourceTransports);
    }

    // Get the list of units assigned to this resource
    const UnitObjList & GetUnits() const
    {
      return (units);
    }
    
    // Get the list of types which are on order for this resource
    const UnitObjTypeList & GetOnOrder() const
    {
      return (onOrder);
    }

  public:

    // Friends
    friend class Manager;

  };

}

#endif
