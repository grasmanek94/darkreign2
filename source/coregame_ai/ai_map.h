/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// AI Map Information
//
// 19-AUG-1998
//

#ifndef __AI_MAP_H
#define __AI_MAP_H


//////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "gameconstants.h"
#include "armourclass.h"
#include "relation.h"


//////////////////////////////////////////////////////////////////////////////
//
// Forward Declarations
//
class Team;


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace AI
//
namespace AI
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Map
  //
  class Map
  {
  public:

    ///////////////////////////////////////////////////////////////////////////
    //
    // Class Info
    //
    class Info
    {
    private:

      U32 *values;      // Array of values by armour class
      U32 total;        // Total

    public:

      // Info
      Info() : 
        values(NULL), 
        total(0) 
      { 
      }

      // ~Info
      ~Info()
      {
        ASSERT(!total);
        ASSERT(!values);
      }

      // Value
      U32 Value(U32 index)
      {
        return (values ? values[index] : 0);
      }

      // Total
      U32 Total()
      {
        return (total);
      }

      // Add
      void Add(U32 index, U32 value)
      {
        ASSERT(U32_MAX - total > value)

        if (!values && value)
        {
          ASSERT(!total)
          values = new U32[ArmourClass::NumClasses()];
          Utils::Memset(values, 0x00, ArmourClass::NumClasses() * sizeof (U32));
        }

        values[index] += value;
        total += value;
      }

      // Remove
      void Remove(U32 index, U32 value)
      {
        ASSERT(total >= value)

        if (value)
        {
          ASSERT(values)
          ASSERT(values[index] >= value)

          values[index] -= value;
          total -= value;

          if (total == 0)
          {
            delete [] values;
            values = NULL;
          }
        }
      }

      // GetValues
      U32 * GetValues()
      {
        return (values);
      }

    };


    ///////////////////////////////////////////////////////////////////////////
    //
    // Class Cluster
    //
    class Cluster
    {
    private:

      // Amount of resource in this cluster
      U32 resource;

      // Amount of disruption in this cluster
      U32 disruption;

      // Occupation count
      U32 occupation[Game::MAX_TEAMS];

      // Team threats for this cluster
      Info teamThreat[Game::MAX_TEAMS];

      // Team defense for this cluster
      Info teamDefense[Game::MAX_TEAMS];

      // Team pain for this cluster
      Info teamPain[Game::MAX_TEAMS];

    public:

      // Constructor
      Cluster()
      {
        resource = 0;
        disruption = 0;
        for (int t = 0; t < Game::MAX_TEAMS; t++)
        {
          occupation[t] = 0;
        }
      }

      // Destructor
      ~Cluster()
      {
        ASSERT(!resource)
        ASSERT(!disruption)

        // Clean up any remaining pain
        for (int t = 0; t < Game::MAX_TEAMS; t++)
        {
          for (U32 ac = 0; ac < ArmourClass::NumClasses(); ac++)
          {
            teamPain[t].Remove(ac, teamPain[t].Value(ac));
            teamMapPain[t].Remove(ac, teamMapPain[t].Value(ac));
          }
        }
      }

      //
      // Cluster Resource Management
      //

      // AddResource: Adds the specified amount of resource to a cluster
      void AddResource(U32 res)
      {
        ASSERT(U32_MAX - resource > res)
        ASSERT(U32_MAX - mapResource > res)
        resource += res;
        mapResource += res;
        mapResourceInv = mapResource ? 1.0f / F32(mapResource) : 0.0f;
      }

      // RemoveResource: Removes the specified amount of resource from a cluster
      void RemoveResource(U32 res)
      {
        ASSERT(resource >= res)
        ASSERT(mapResource >= res)
        resource -= res;
        mapResource -= res;
        mapResourceInv = mapResource ? 1.0f / F32(mapResource) : 0.0f;
      }

      // GetResource: Returns the speicifed amount of resource from a cluster
      U32 GetResource()
      {
        return (resource);
      }

      //
      // Disruption management
      //

      // AddDisruption: Adds disruption to the cluster
      void AddDisruption()
      {
        ASSERT(U32_MAX - disruption > 1)
        ASSERT(U32_MAX - mapDisruption > 1)
        disruption++;
        mapDisruption++;
        mapDisruptionInv = mapDisruption ? 1.0f / F32(mapDisruption) : 0.0f;
      }

      // RemoveDisruption: Removes disruption from the cluster
      void RemoveDisruption()
      {
        ASSERT(disruption >= 1)
        ASSERT(mapDisruption >= 1)
        disruption--;
        mapDisruption--;
        mapDisruptionInv = mapDisruption ? 1.0f / F32(mapDisruption) : 0.0f;
      }

      // GetDisruption: Gets the amount of disruption for the cluster
      U32 GetDisruption()
      {
        return (disruption);
      }

      //
      // Occupation Management
      //

      // IncOccupation: Increment the occupation of this cluster
      void IncOccupation(U32 team)
      {
        ASSERT(team < Game::MAX_TEAMS)
        occupation[team]++;
        mapOccupation[team] += 1;
        mapOccupationInv[team] = 1.0f / F32(mapOccupation[team]);
      }

      // GetOccupation: Get the occupation count of this cluster
      U32 GetOccupation(U32 team)
      {
        ASSERT(team < Game::MAX_TEAMS)
        return (occupation[team]);
      }

      //
      // Cluster Threat Management
      //

      // AddThreat: Adds the specified amount of threat to a cluster
      void AddThreat(U32 team, U32 armourClass, U32 threat)
      {
        ASSERT(team < Game::MAX_TEAMS)
        ASSERT(armourClass < ArmourClass::NumClasses())

        teamThreat[team].Add(armourClass, threat);
        teamMapThreat[team].Add(armourClass, threat);
      }  

      // RemoveThreat: Removes the specified amount of threat from a cluster
      void RemoveThreat(U32 team, U32 armourClass, U32 threat)
      {
        ASSERT(team < Game::MAX_TEAMS)
        ASSERT(armourClass < ArmourClass::NumClasses())

        teamThreat[team].Remove(armourClass, threat);
        teamMapThreat[team].Remove(armourClass, threat);
      }

      // GetThreat: Returns the threat to this armour class for the team
      U32 GetThreat(U32 team, U32 armourClass)
      {
        ASSERT(team < Game::MAX_TEAMS)
        ASSERT(armourClass < ArmourClass::NumClasses())

        return (teamThreat[team].Value(armourClass));
      }

      // GetTotalThreat: Returns the total threat for the team
      U32 GetTotalThreat(U32 team)
      {
        ASSERT(team < Game::MAX_TEAMS)

        return (teamThreat[team].Total());
      }

      // GetThreats: Returns the threats for this team
      U32 * GetThreats(U32 team)
      {
        return (teamThreat[team].GetValues());
      }

      //
      // Cluster Defense Management
      //

      // AddDefense: Adds the specified amount of defense to a cluster
      void AddDefense(U32 team, U32 armourClass, U32 defense)
      {
        ASSERT(team < Game::MAX_TEAMS)
        ASSERT(armourClass < ArmourClass::NumClasses())

        teamDefense[team].Add(armourClass, defense);
        teamMapDefense[team].Add(armourClass, defense);
      }

      // RemoveDefense: Removes the specified amount of defense from a cluster
      void RemoveDefense(U32 team, U32 armourClass, U32 defense)
      {
        ASSERT(team < Game::MAX_TEAMS)
        ASSERT(armourClass < ArmourClass::NumClasses())

        teamDefense[team].Remove(armourClass, defense);
        teamMapDefense[team].Remove(armourClass, defense);
      }

      // GetDefense: Returns the defense by armour class for the team
      U32 GetDefense(U32 team, U32 armourClass)
      {
        ASSERT(team < Game::MAX_TEAMS)
        ASSERT(armourClass < ArmourClass::NumClasses())

        return (teamDefense[team].Value(armourClass));
      }

      // GetTotalDefense: Returns the total defense for the team
      U32 GetTotalDefense(U32 team)
      {
        ASSERT(team < Game::MAX_TEAMS)

        return (teamDefense[team].Total());
      }

      // GetDefenses: Returns the defenses for this team
      U32 * GetDefenses(U32 team)
      {
        return (teamDefense[team].GetValues());
      }

      //
      // Cluster Pain Management
      //

      // AddPain: Adds the specified amount of pain to a cluster
      void AddPain(U32 team, U32 armourClass, U32 pain)
      {
        ASSERT(team < Game::MAX_TEAMS)
        ASSERT(armourClass < ArmourClass::NumClasses())

        teamPain[team].Add(armourClass, pain);
        teamMapPain[team].Add(armourClass, pain);
      }  

      // ReducePain: Reduces the pain in this cluster by a fraction of 256
      void ReducePain(U32 team, U32 fraction)
      {
        ASSERT(team < Game::MAX_TEAMS)

        for (U32 ac = 0; ac < ArmourClass::NumClasses(); ac++)
        {
          U32 value = teamPain[team].Value(ac);

          if (value)
          {
            U32 reduction = value * fraction >> 8;
            if (reduction == 0)
            {
              reduction = value;
            }
            teamPain[team].Remove(ac, reduction);
            teamMapPain[team].Remove(ac, reduction);
          }
        }
      }

      // GetPain: Returns the pain to this armour class for the team
      U32 GetPain(U32 team, U32 armourClass)
      {
        ASSERT(team < Game::MAX_TEAMS)
        ASSERT(armourClass < ArmourClass::NumClasses())

        return (teamPain[team].Value(armourClass));
      }

      // GetTotalPain: Returns the total pain for the team
      U32 GetTotalPain(U32 team)
      {
        ASSERT(team < Game::MAX_TEAMS)

        return (teamPain[team].Total());
      }

      // GetPains: Returns the pains for this team
      U32 * GetPains(U32 team)
      {
        return (teamPain[team].GetValues());
      }

      //
      // Evaluators
      //

      U32 EvaluateDefense(Team *team, Relation relation, U32 *threats);

      U32 EvaluateThreat(Team *team, Relation relation, U32 *defenses);
      U32 EvaluateThreat(Team *team, Relation relation, U32 armourClass);
      U32 EvaluateRelativeThreat(Team *team, Relation relation, U32 armourClass);

      U32 EvaluatePain(Team *team, U32 *defenses);

    };

#pragma warning(push)
#pragma warning(disable : 4200)

    ///////////////////////////////////////////////////////////////////////////
    //
    // Class BleedMap
    //
    struct BleedMap
    {
      S32 range;
      U8 data[];

      // Get the value of the bleed map at the given offset
      U8 Get(S32 x, S32 y)
      {
        ASSERT(x < range)
        ASSERT(y < range)
        return (*(data + (y * range) + x));
      }

      // Apply a bleed map to clusters
      void ApplyThreat(const Point<S32> &cluster, U32 team, U32 armourClass, U32 threat, Bool add);

      // Apply a disruption bleed map to clusters
      void ApplyDisruption(const Point<S32> &cluster, U32 team, Bool add);

    };

#pragma warning(pop)

  private:

    // Resources for the entire map
    static U32 mapResource;

    // Inverse of the resources for the entire map
    static F32 mapResourceInv;

    // Disruption for the entire map
    static U32 mapDisruption;

    // Inverse of the disruption for the entire map
    static F32 mapDisruptionInv;

    // Occupation for the entire map
    static U32 mapOccupation[Game::MAX_TEAMS];

    // Inverse of the occupation for the entire map
    static F32 mapOccupationInv[Game::MAX_TEAMS];

    // Team threats for the map
    static Info teamMapThreat[Game::MAX_TEAMS];

    // Team defense for the map
    static Info teamMapDefense[Game::MAX_TEAMS]; 

    // Team pain for the map
    static Info teamMapPain[Game::MAX_TEAMS];

  public:

    // Init: Initialize the AI Map system
    static void Init();

    // Done: Shutdown the AI Map system
    static void Done();


    // Create a bleed map
    static BleedMap * CreateBleedMap(F32 range, Bool disipate = FALSE);


    // GetResource: Returns the amount of resource on the entire map
    static U32 GetResource()
    {
      return (mapResource);
    }

    // GetResourceInv: Returns the inverse of resource on the entire map
    static F32 GetResourceInv()
    {
      return (mapResourceInv);
    }

    // GetDisruption: Returns the amount of disruption on the entire map
    static U32 GetDisruption()
    {
      return (mapDisruption);
    }

    // GetDisruptionInv: Returns the inverse of disruption on the entire map
    static F32 GetDisruptionInv()
    {
      return (mapDisruptionInv);
    }

    // GetOccupation: Returns the amount of occupation for the entire map
    static U32 GetOccupation(U32 team)
    {
      ASSERT(team < Game::MAX_TEAMS)
      return (mapOccupation[team]);
    }

    // GetOccupationInv: Returns the inverse of occupation for the entire map
    static F32 GetOccupationInv(U32 team)
    {
      ASSERT(team < Game::MAX_TEAMS)
      return (mapOccupationInv[team]);
    }

    // GetThreat: Returns the threat to this armour class for the team for the entire map
    static U32 GetThreat(U32 team, U32 armourClass)
    {
      ASSERT(team < Game::MAX_TEAMS)
      ASSERT(armourClass < ArmourClass::NumClasses())

      return (teamMapThreat[team].Value(armourClass));
    }

    // GetTotalThreat: Returns the total threat for the team for the entire map
    static U32 GetTotalThreat(U32 team)
    {
      ASSERT(team < Game::MAX_TEAMS)

      return (teamMapThreat[team].Total());
    }

    // GetDefense: Returns the defense for the team for the entire map
    static U32 GetDefense(U32 team, U32 armourClass)
    {
      ASSERT(team < Game::MAX_TEAMS)
      ASSERT(armourClass < ArmourClass::NumClasses())

      return (teamMapDefense[team].Value(armourClass));
    }

    // GetTotalDefense: Returns the total defense for the team for the entire map
    static U32 GetTotalDefense(U32 team)
    {
      ASSERT(team < Game::MAX_TEAMS)

      return (teamMapDefense[team].Total());
    }

    // GetPain: Returns the pain to this armour class for the team for the entire map
    static U32 GetPain(U32 team, U32 armourClass)
    {
      ASSERT(team < Game::MAX_TEAMS)
      ASSERT(armourClass < ArmourClass::NumClasses())

      return (teamMapPain[team].Value(armourClass));
    }

    // GetTotalPain: Returns the total pain for the team for the entire map
    static U32 GetTotalPain(U32 team)
    {
      ASSERT(team < Game::MAX_TEAMS)

      return (teamMapPain[team].Total());
    }

    //
    // Friends of AIMap
    //
    friend Map::Cluster;

  };

}


#endif