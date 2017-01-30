/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// AI Map Information
//
// 19-AUG-1998
//


//////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "ai_map.h"
#include "team.h"
#include "gameobjctrl.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace AI
//
namespace AI
{


  //////////////////////////////////////////////////////////////////////////////
  //
  // Definitions
  //

  // Bleed range of threats
  F32 additionalBleedRange = 200.0f;


  //////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // Resources for the entire map
  U32 Map::mapResource;

  // Inverse of resources for the entire map
  F32 Map::mapResourceInv;

  // Disruption for the entire map
  U32 Map::mapDisruption;

  // Inverse of the disruption for the entire map
  F32 Map::mapDisruptionInv;

  // Occupation for the entire map
  U32 Map::mapOccupation[Game::MAX_TEAMS];

  // Inverse of the occupation for the entire map
  F32 Map::mapOccupationInv[Game::MAX_TEAMS];

  // Team threats for the map
  Map::Info Map::teamMapThreat[Game::MAX_TEAMS];  

  // Team defense for the map
  Map::Info Map::teamMapDefense[Game::MAX_TEAMS]; 

  // Team pains for the map
  Map::Info Map::teamMapPain[Game::MAX_TEAMS];  
  

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class BleedMap
  //


  //
  // Apply a bleed map to clusters
  //
  void Map::BleedMap::ApplyThreat(const Point<S32> &cluster, U32 team, U32 armourClass, U32 threat, Bool add)
  {
    void (Cluster::*modifier)(U32 team, U32 armourClass, U32 threat);

    if (add)
    {
      modifier = AI::Map::Cluster::AddThreat;
    }
    else
    {
      modifier = AI::Map::Cluster::RemoveThreat;
    }

    // Using the bleed map ... add the threat this object 
    // imposes to this cluster and the surrounding clusters
    (WorldCtrl::GetCluster(cluster.x, cluster.z)->ai.*modifier)(team, armourClass, threat);

    S32 i, j;

    for (i = 1; i < range; i++)
    {
      U8 val = Get(i, 0);
      if (val)
      {
        U32 t = ((val + 1) * threat) >> 8;

        if (t)
        {
          if ((cluster.x + i) < (S32) WorldCtrl::ClusterMapX())
          {
            (WorldCtrl::GetCluster(cluster.x + i, cluster.z)->ai.*modifier)(team, armourClass, t);
          }

          if ((cluster.z + i) < (S32) WorldCtrl::ClusterMapZ())
          {
            (WorldCtrl::GetCluster(cluster.x, cluster.z + i)->ai.*modifier)(team, armourClass, t);
          }

          if ((cluster.x - i) >= 0)
          {
            (WorldCtrl::GetCluster(cluster.x - i, cluster.z)->ai.*modifier)(team, armourClass, t);
          }

          if ((cluster.z - i) >= 0)
          {
            (WorldCtrl::GetCluster(cluster.x, cluster.z - i)->ai.*modifier)(team, armourClass, t);
          }
        }
      }
    }

    for (j = 1; j < range; j++)
    {
      for (i = 1; i < range; i++)
      {
        U8 val = Get(i, j);
        if (val)
        {
          U32 t = ((val + 1) * threat) >> 8;

          if (t)
          {
            if (((cluster.x + i) < (S32) WorldCtrl::ClusterMapX()) && 
                ((cluster.z + j) < (S32) WorldCtrl::ClusterMapZ()))
            {
              (WorldCtrl::GetCluster(cluster.x + i, cluster.z + j)->ai.*modifier)(team, armourClass, t);
            }

            if (((cluster.x - i) >= 0) && 
                ((cluster.z + j) < (S32) WorldCtrl::ClusterMapZ()))
            {
              (WorldCtrl::GetCluster(cluster.x - i, cluster.z + j)->ai.*modifier)(team, armourClass, t);
            }

            if (((cluster.x + i) < (S32) WorldCtrl::ClusterMapX()) && 
                ((cluster.z - j) >= 0))
            {
              (WorldCtrl::GetCluster(cluster.x + i, cluster.z - j)->ai.*modifier)(team, armourClass, t);
            }

            if (((cluster.x - i) >= 0) && 
                ((cluster.z - j) >= 0))
            {
              (WorldCtrl::GetCluster(cluster.x - i, cluster.z - j)->ai.*modifier)(team, armourClass, t);
            }
          }
        }
      }          
    }
  }


  //
  // Apply a disruption bleed map to clusters
  //
  void Map::BleedMap::ApplyDisruption(const Point<S32> &cluster, U32, Bool add)
  {
    void (Cluster::*modifier)();

    if (add)
    {
      modifier = AI::Map::Cluster::AddDisruption;
    }
    else
    {
      modifier = AI::Map::Cluster::RemoveDisruption;
    }

    // Using the bleed map ... add the threat this object 
    // imposes to this cluster and the surrounding clusters
    (WorldCtrl::GetCluster(cluster.x, cluster.z)->ai.*modifier)();

    S32 i, j;

    for (i = 1; i < range; i++)
    {
      if (Get(i, 0))
      {
        if ((cluster.x + i) < (S32) WorldCtrl::ClusterMapX())
        {
          (WorldCtrl::GetCluster(cluster.x + i, cluster.z)->ai.*modifier)();
        }

        if ((cluster.z + i) < (S32) WorldCtrl::ClusterMapZ())
        {
          (WorldCtrl::GetCluster(cluster.x, cluster.z + i)->ai.*modifier)();
        }

        if ((cluster.x - i) >= 0)
        {
          (WorldCtrl::GetCluster(cluster.x - i, cluster.z)->ai.*modifier)();
        }

        if ((cluster.z - i) >= 0)
        {
          (WorldCtrl::GetCluster(cluster.x, cluster.z - i)->ai.*modifier)();
        }
      }
    }

    for (j = 1; j < range; j++)
    {
      for (i = 1; i < range; i++)
      {
        if (Get(i, j))
        {
          if (((cluster.x + i) < (S32) WorldCtrl::ClusterMapX()) && 
              ((cluster.z + j) < (S32) WorldCtrl::ClusterMapZ()))
          {
            (WorldCtrl::GetCluster(cluster.x + i, cluster.z + j)->ai.*modifier)();
          }

          if (((cluster.x - i) >= 0) && 
              ((cluster.z + j) < (S32) WorldCtrl::ClusterMapZ()))
          {
            (WorldCtrl::GetCluster(cluster.x - i, cluster.z + j)->ai.*modifier)();
          }

          if (((cluster.x + i) < (S32) WorldCtrl::ClusterMapX()) && 
              ((cluster.z - j) >= 0))
          {
            (WorldCtrl::GetCluster(cluster.x + i, cluster.z - j)->ai.*modifier)();
          }

          if (((cluster.x - i) >= 0) && 
              ((cluster.z - j) >= 0))
          {
            (WorldCtrl::GetCluster(cluster.x - i, cluster.z - j)->ai.*modifier)();
          }
        }
      }          
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Map::Cluster
  //


  //
  // Evaluate the enemy defense which is threatened
  //
  U32 Map::Cluster::EvaluateDefense(Team *team, Relation relation, U32 *threats)
  {
    U32 defense = 0;

    // Enumerate the enemy teams
    for (List<Team>::Iterator t(&team->RelatedTeams(relation)); *t; ++t)
    {
      for (U32 a = 0; a < ArmourClass::NumClasses(); a++)
      {
        if (threats[a])
        {
          defense += GetDefense((*t)->GetId(), a);
        }
      }
    }
    return (defense);
  }


  //
  // Evaluate enemy threat to the given defense table
  //
  U32 Map::Cluster::EvaluateThreat(Team *team, Relation relation, U32 *defenses)
  {
    U32 threat = 0;

    // Enumerate the enemy teams
    for (List<Team>::Iterator t(&team->RelatedTeams(relation)); *t; ++t)
    {
      for (U32 a = 0; a < ArmourClass::NumClasses(); a++)
      {
        if (defenses[a])
        {
          threat += GetThreat((*t)->GetId(), a) * defenses[a];
        }
      }
    }
    return (threat);
  }


  //
  // Evaluate enemy threat to the given defense table
  //
  U32 Map::Cluster::EvaluateThreat(Team *team, Relation relation, U32 armourClass)
  {
    U32 threat = 0;

    // Enumerate the enemy teams
    for (List<Team>::Iterator t(&team->RelatedTeams(relation)); *t; ++t)
    {
      threat += GetThreat((*t)->GetId(), armourClass);
    }
    return (threat);
  }


  //
  // Evaluate enemy threat to the given defense table
  //
  U32 Map::Cluster::EvaluateRelativeThreat(Team *team, Relation relation, U32 armourClass)
  {
    U32 threat = 0;
    U32 mapThreat = 0;

    // Enumerate the enemy teams
    for (List<Team>::Iterator t(&team->RelatedTeams(relation)); *t; ++t)
    {
      threat += GetThreat((*t)->GetId(), armourClass);
      mapThreat += Map::GetThreat((*t)->GetId(), armourClass);
    }
    return (Utils::Div(threat, 0, mapThreat));
  }


  //
  // Evaluate pain to the given defense table
  //
  U32 Map::Cluster::EvaluatePain(Team *team, U32 *defenses)
  {
    ASSERT(team)

    U32 pain = 0;

    // Enumerate the enemy teams
    for (U32 a = 0; a < ArmourClass::NumClasses(); a++)
    {
      if (defenses[a])
      {
        pain += GetPain(team->GetId(), a);
      }
    }
    return (pain);
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Map
  //


  //
  // Init: Initialize the AI Map system
  //
  void Map::Init()
  {
    mapResource = 0;
    mapResourceInv = 0.0f;

    mapDisruption = 0;
    mapDisruptionInv = 0.0f;

    for (int t = 0; t < Game::MAX_TEAMS; t++)
    {
      mapOccupation[t] = 0;
      mapOccupationInv[t] = 0.0f;
    }
  }


  //
  // Done: Shutdown the AI Map system
  //
  void Map::Done()
  {
    ASSERT(!mapResource)
    ASSERT(!mapDisruption)

    for (int t = 0; t < Game::MAX_TEAMS; t++)
    {
      // Ensure that all of the values and totals are clear at this point
      //ASSERT(!teamMapThreat[t].Total())
      //ASSERT(!teamMapDefense[t].Total())

      if (teamMapThreat[t].Total())
      {
        LOG_WARN(("PLEASE SUBMIT!! TeamMapThreat [%d] [%d]", teamMapThreat[t].Total(), GameObjCtrl::listAll.GetCount()));
      }

      if (teamMapDefense[t].Total())
      {
        LOG_WARN(("PLEASE SUBMIT!! TeamMapDefense [%d] [%d]", teamMapDefense[t].Total(), GameObjCtrl::listAll.GetCount()));
      }

//      ASSERT(!teamMapPain[t].Total())
    }
  }


  //
  // Create a bleed map
  //
  Map::BleedMap * Map::CreateBleedMap(F32 range, Bool disipate)
  {
    // Create a bleed mask map based on the range information
    range += WorldCtrl::ClusterSize();

    F32 clusterRange = range;

    if (disipate)
    {
      // If we want to disipate, add the disipate range on
      clusterRange += additionalBleedRange;
    }
    else
    {
      // Otherwise add 1/2 the bleed range just, you don't want to go near threat
      clusterRange += additionalBleedRange * 0.25f;
    }

    S32 bleedRange = (S32) (clusterRange / WorldCtrl::ClusterSize()) + 1;

    // Allocate the bleed map
    BleedMap *bleedMap = reinterpret_cast<BleedMap *>(new U8[bleedRange * bleedRange + sizeof (BleedMap)]);
    Utils::Memset(bleedMap->data, 0x00, sizeof (U8) * bleedRange * bleedRange);

    bleedMap->range = bleedRange;

    for (S32 j = 0; j < bleedRange; j++)
    {
      for (S32 i = 0; i < bleedRange; i++)
      {
        U32 ci = (U32) (i * WorldCtrl::ClusterSize());
        U32 cj = (U32) (j * WorldCtrl::ClusterSize());

        F32 dist = F32(sqrt(ci * ci + cj * cj));

        if (dist > range)
        {
          if (dist > clusterRange)
          {
            bleedMap->data[j * bleedRange + i] = 0x00;
          }
          else
          {
            dist -= range;
            dist /= clusterRange;

            dist = 1.0f - Clamp<F32>(0.0f, dist, 1.0f);
            bleedMap->data[j * bleedRange + i] = U8(dist * F32(0xE0));
          }
        }
        else
        {
          dist /= range;
          dist = 1.0f - Clamp<F32>(0.0f, dist, 1.0f);
          bleedMap->data[j * bleedRange + i] = U8(0xE0 + U8(dist * F32(0x1F)));
        }
      }
    }

    return (bleedMap);
  }

}
