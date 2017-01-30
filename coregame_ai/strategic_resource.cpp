/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Resource
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_private.h"
#include "strategic_resource.h"
#include "strategic_resource_manager.h"
#include "strategic_resource_decomposition.h"
#include "strategic_object.h"
#include "orders_game.h"
#include "resolver.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Resource
  //

  // Number of cycles between updates
  const U32 updateInterval = 450;


  //
  // Constructor
  //
  Resource::Resource(Manager &manager, const ClusterGroup &clusterGroup, F32 proximity, U32 id)
  : manager(manager),
    id(id),
    resource(clusterGroup.resource),
    regen(clusterGroup.regen),
    centre(clusterGroup.midAvg),
    baseProximity(proximity),
    updateCycle(GameTime::SimCycle() + updateInterval)
  {
    resources.AppendList(clusterGroup.resources);

    for (List<MapCluster>::Iterator c(&clusterGroup.clusters); *c; ++c)
    {
      clusters.Append(*c);
    }

    // Register our construction
    RegisterConstruction(dTrack);
  }


  //
  // Constructor
  //
  Resource::Resource(Manager &manager, F32 proximity, U32 id)
  : manager(manager),
    id(id),
    resource(0),
    regen(0),
    baseProximity(proximity),
    updateCycle(GameTime::SimCycle() + updateInterval)
  {
    // Register our construction
    RegisterConstruction(dTrack);
  }


  //
  // Destructor
  //
  Resource::~Resource()
  {
    // Unlink all clusters
    clusters.UnlinkAll();

    // Clear on order types
    onOrder.Clear();

    // Clear units assigned
    resourceStorers.Clear();
    resourceTransports.Clear();
    units.Clear();

    // Clear resources
    resources.Clear();

    // Register our demise
    RegisterDestruction(dTrack);
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Resource::SaveState(FScope *scope)
  {
    StdSave::TypeReaperList(scope, "Resources", resources);
    StdSave::TypeU32(scope, "Resource", resource);
    StdSave::TypeU32(scope, "Regen", regen);
    StdSave::TypeReaperList(scope, "ResourceStorers", resourceStorers);
    StdSave::TypeReaperList(scope, "ResourceTransports", resourceTransports);
    StdSave::TypeReaperList(scope, "Units", units);
    StdSave::TypeReaperListObjType(scope, "OnOrder", onOrder);
    StdSave::TypeF32(scope, "BaseProximity", baseProximity);
    StdSave::TypePoint(scope, "Centre", centre);
    StdSave::TypeU32(scope, "UpdateCycle", updateCycle);
    
    FScope *fScope = scope->AddFunction("Clusters");
    
    for (List<MapCluster>::Iterator i(&clusters); *i; ++i)
    {
      FScope *sScope = fScope->AddFunction("Cluster");
      sScope->AddArgInteger((*i)->xIndex);
      sScope->AddArgInteger((*i)->zIndex);
    }
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Resource::LoadState(FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x8C34B766: // "Resources"
          StdLoad::TypeReaperList(sScope, resources);
          Resolver::ObjList<ResourceObj, ResourceObjType, ResourceObjListNode>(resources);
          break;

        case 0x4CD1BE27: // "Resource"
          resource = StdLoad::TypeU32(sScope);
          break;

        case 0x192CADA5: // "Regen"
          regen = StdLoad::TypeU32(sScope);
          break;

        case 0x41DB0A3A: // "ResourceStorers"
          StdLoad::TypeReaperList(sScope, resourceStorers);
          Resolver::ObjList<UnitObj, UnitObjType, UnitObjListNode>(resourceStorers);
          break;

        case 0x89777833: // "ResourceTransports"
          StdLoad::TypeReaperList(sScope, resourceTransports);
          Resolver::ObjList<UnitObj, UnitObjType, UnitObjListNode>(resourceTransports);
          break;

        case 0xCED02493: // "Units"
          StdLoad::TypeReaperList(sScope, units);
          Resolver::ObjList<UnitObj, UnitObjType, UnitObjListNode>(units);
          break;

        case 0x189FFF75: // "OnOrder"
          StdLoad::TypeReaperListObjType(sScope, onOrder);
          Resolver::TypeList<UnitObjType, UnitObjTypeListNode>(onOrder);
          break;

        case 0x511695C1: // "BaseProximity"
          baseProximity = StdLoad::TypeF32(sScope);
          break;

        case 0x03633B25: // "Centre"
          StdLoad::TypePoint(sScope, centre);
          break;

        case 0x87843E5F: // "UpdateCycle"
          updateCycle = StdLoad::TypeU32(sScope);
          break;

        case 0x6ECAAA17: // "Clusters"
        {
          FScope *ssScope;

          while ((ssScope = sScope->NextFunction()) != NULL)
          {
            switch (ssScope->NameCrc())
            {
              case 0xF5ACB747: // "Cluster"
              {
                // Get cluster location
                U32 x = StdLoad::TypeU32(ssScope);
                U32 z = StdLoad::TypeU32(ssScope);

                // Get the cluster
                clusters.Append(WorldCtrl::GetCluster(x, z));
                break;
              }
            }
          }
        }
      }
    }
  }


  //
  // Add on Order
  //
  void Resource::AddOnOrder(UnitObjType &type)
  {
    // On order types
    onOrder.Append(&type);
  }


  //
  // Remove on order
  //
  void Resource::RemoveOnOrder(UnitObjType &type)
  {
    // Find this type in the on order types
    if (!onOrder.Remove(&type, TRUE))
    {
      ERR_FATAL(("Could not find type '%s' in the on order types", type.GetName()))
    }
  }


  //
  // Add unit
  //
  void Resource::AddUnit(UnitObj *unit)
  {
    ASSERT(unit)

    // Is this unit a resource transporter ...

    // If this unit is a resource transporter, order it to collect from our resources!
    if (unit->UnitType()->GetResourceTransport())
    {
      // Add to the resource transporters
      resourceTransports.Append(unit);

      ResourceObj *resource = NULL;
      U32 amount = 0;

      resources.PurgeDead();

      // Pick the most succulent resource we have
      for (ResourceObjList::Iterator r(&resources); *r; ++r)
      {
        if ((**r)->GetResource() > amount)
        {
          resource = **r;
          amount = resource->GetResource();
        }
      }

      if (resource)
      {
        Orders::Game::ClearSelected::Generate(manager.GetObject());
        Orders::Game::AddSelected::Generate(manager.GetObject(), unit);
        Orders::Game::Collect::Generate(manager.GetObject(), resource->Id(), FALSE, Orders::FLUSH);
      }
      else
      {
        // The're no resource, this can happen but we
        // expect this resource to be removed soon
      }
    }
    else if (unit->HasProperty(0xAE95DF36)) // "Ability::StoreResource"
    {
      // Add to the resource storers
      resourceStorers.Append(unit);
    }
    else
    {
      // Add to other units
      units.Append(unit);
    }
  }


  //
  // Process
  //
  Bool Resource::Process()
  {
    // How long since we last processed ?
    if (GameTime::SimCycle() > updateCycle)
    {
      updateCycle += updateInterval;

      // Update the total quantity of resource at this site
      resource = 0;
      regen = 0;
      for (ResourceObjList::Iterator r(&resources); *r; ++r)
      {
        if ((*r)->Alive())
        {
          resource += (**r)->GetResource();
          regen += (**r)->ResourceType()->GetResourceRate();
        }
      }

      // Is there zero resource ?
      if (!resource && !regen)
      {
        // Recycle all of the refineries and all of the units
        resourceStorers.PurgeDead();
        units.PurgeDead();
        if (resourceStorers.GetCount())
        {
          Orders::Game::ClearSelected::Generate(manager.GetObject());
          for (UnitObjList::Iterator s(&resourceStorers); *s; ++s)
          {
            Orders::Game::AddSelected::Generate(manager.GetObject(), **s);
          }
          for (UnitObjList::Iterator u(&units); *u; ++u)
          {
            Orders::Game::AddSelected::Generate(manager.GetObject(), **u);
          }
          Orders::Game::Recycle::Generate(manager.GetObject());
        }
        return (TRUE);
      }
      else
      {
        // Check all of our resource transporters and make sure that 
        // we evenly distribute them to resource storage facilities
        resourceTransports.PurgeDead();
        resourceStorers.PurgeDead();
        units.PurgeDead();

        LOG_AI(("Performing transport assignment for resource"))

        if (resourceStorers.GetCount() && resourceTransports.GetCount())
        {
          UnitObjList::Iterator s(&resourceStorers);

          // Iterate the storers and assign transports
          for (UnitObjList::Iterator t(&resourceTransports); *t; ++t)
          {
            LOG_AI(("Storer [%d] <- Transport [%d]", (*s)->Id(), (*t)->Id()))
            Orders::Game::ClearSelected::Generate(manager.GetObject());
            Orders::Game::AddSelected::Generate(manager.GetObject(), **t);
            Orders::Game::Store::Generate(manager.GetObject(), (*s)->Id(), FALSE, TRUE, Orders::FLUSH);

            if (!++s)
            {
              !s;
            }
          }
        }
      }
    }

    return (FALSE);
  }
}

