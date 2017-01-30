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
#include "strategic_resource_manager.h"
#include "strategic_resource_decomposition.h"
#include "strategic_object.h"
#include "savegame.h"
#include "orders_game.h"


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


  //
  // Constructor
  //
  Resource::Manager::Manager()
  : resourceId(1),
    resources(&Resource::nodeManager)
  {
  }


  //
  // Destructor
  //
  Resource::Manager::~Manager()
  {
    resources.DisposeAll();
    resourceTransports.Clear();
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Resource::Manager::SaveState(FScope *scope)
  {
    StdSave::TypeU32(scope, "ResourceId", resourceId);
    StdSave::TypeReaperList(scope, "ResourceTransports", resourceTransports);

    for (NBinTree<Resource, F32>::Iterator i(&resources); *i; i++)
    {
      // Create a new function
      FScope *sScope = scope->AddFunction("Resource");

      // Save the proximity key
      sScope->AddArgFPoint(i.GetKey());

      // Save the id
      sScope->AddArgInteger((*i)->GetId());

      // Save resource data
      (*i)->SaveState(sScope);
    }

  }


  //
  // LoadState
  //
  // Load state information
  //
  void Resource::Manager::LoadState(FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x5C0AABC4: // "ResourceId"
          resourceId = StdLoad::TypeU32(sScope);
          break;

        case 0x4CD1BE27: // "Resource"
        {
          // Load the proximity and the id
          F32 proximity = StdLoad::TypeF32(sScope);
          U32 id = StdLoad::TypeU32(sScope);

          ASSERT(id < resourceId);

          // Create a new resource
          Resource *resource = new Resource(*this, proximity, id);

          // Load resource data
          resource->LoadState(sScope);

          // Add to the tree
          resources.Add(proximity, resource);         
          break;
        }

        case 0x89777833: // "ResourceTransports"
          StdLoad::TypeReaperList(sScope, resourceTransports);
          Resolver::ObjList<UnitObj, UnitObjType, UnitObjListNode>(resourceTransports);
          break;
      }
    }
  }


  //
  // Reset
  //
  void Resource::Manager::Reset()
  {
    // If we are loading a save game, don't perform analysis of decomposition
    if (!SaveGame::LoadActive())
    {
      Team *myTeam = GetObject().GetTeam();

      if (myTeam->GetStartRegion())
      {
        const Point<F32> &myTeamPos = myTeam->GetStartRegion()->GetMidPoint();

        for (NList<ClusterGroup>::Iterator g(&Decomposition::GetClusterGroups()); *g; ++g)
        {
          // Get the distance to the middle of the cluster
          F32 x = (*g)->midAvg.x;
          F32 z = (*g)->midAvg.z;

          // How far is this resource away from start location
          F32 myTeamDistance = 
              (myTeamPos.x - x) * (myTeamPos.x - x) + 
              (myTeamPos.z - z) * (myTeamPos.z - z);

          F32 otherTeamDistance = 0.0f;
          U32 numTeams = 0;

          for (U32 t = 0; t < Game::MAX_TEAMS; t++)
          {
            Team *team = Team::Id2Team(t);
            if (team && myTeam != team && team->GetStartRegion())
            {
              const Point<F32> &pos = team->GetStartRegion()->GetMidPoint();
              F32 distance = (pos.x - x) * (pos.x - x) + (pos.z - z) * (pos.z - z);

              numTeams++;

              // Add the distance to the other team
              otherTeamDistance += distance;
            }
          }

          if (numTeams)
          {
            // Add to the distance the average distance the resource is to the other teams
            // Since we want closer distances to enemies to score higher, we subtact the
            // average distance from the map diagonal distance 
            myTeamDistance += WorldCtrl::MetreMapDiag2() - (otherTeamDistance / F32(numTeams));
          }

          // Add to the list of resources
          resources.Add(myTeamDistance, new Resource(*this, **g, myTeamDistance, resourceId++));
        }
      }
    }
  }


  //
  // Process Resource
  //
  void Resource::Manager::Process()
  {
    NBinTree<Resource, F32>::Iterator r(&resources);

    resourceTransports.PurgeDead();

    while (Resource *resource = r++)
    {
      if (resource->Process())
      {
        // Remove the resource
        resources.Unlink(resource);

        // Assign all of the resource transports to 
        // the unassigned resource transports pile
        resource->resourceTransports.PurgeDead();
        for (UnitObjList::Iterator t(&resource->resourceTransports); *t; ++t)
        {
          resourceTransports.AppendNoDup(**t);
        }

        // Delete it
        delete resource;
      }
      else
      {
        // Check to see if this resource is currently understaffed

        resource->resourceTransports.PurgeDead();
        resource->resourceStorers.PurgeDead();

        if (resource->resourceStorers.GetCount())
        {
          S32 under = resource->resourceTransports.GetCount() - 3;

          while (under++ < 0)
          {
            UnitObj *closest = NULL;
            F32 minDistance = F32_MAX;

            // Find the closest unassigned transport 
            // (which is in the same) connected region
            for (UnitObjList::Iterator u(&resourceTransports); *u; ++u)
            {
              Point<F32> p((**u)->Origin().x, (**u)->Origin().z);

              F32 distance = (resource->centre - p).GetMagnitude2();

              if (distance < minDistance)
              {
                closest = **u;
                minDistance = distance;
              }
            }

            if (closest)
            {
              resource->AddUnit(closest);
              resourceTransports.Remove(closest);
            }
            else
            {
              break;
            }
          }
        }
      }
    }
  }


  //
  // Find a resource
  //
  Resource * Resource::Manager::FindResource(U32 id)
  {
    for (NBinTree<Resource, F32>::Iterator i(&resources); *i; ++i)
    {
      if ((*i)->GetId() == id)
      {
        return (*i);
      }
    }
    
    return (NULL);
  }
}
