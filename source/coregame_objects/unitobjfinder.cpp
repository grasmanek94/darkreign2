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
#include "unitobjfinder.h"
#include "random.h"
#include "weapon.h"
#include "tasks_restoremobile.h"
#include "taskctrl.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace UnitObjFinder
//
namespace UnitObjFinder
{

  //
  // Pre defined Heuristics
  //

  //
  // Random
  //
  void Random(UnitObj *obj, F32, HeuristicData &heuristicData)
  {
    // Ignore if this is the subject
    if (heuristicData.random.subject != obj)
    {
      // Randomly modify the winner
      if (!heuristicData.winner || (Random::sync.Float() < 0.1F))
      {
        heuristicData.winner = obj;
      }
    }
  }


  //
  // MaxThreatMinDefense
  //
  void MaxThreatMinDefense(UnitObj *obj, F32 distance2, HeuristicData &heuristicData)
  {
    // Ignore if this is the subject
    if (heuristicData.maxThreatMinDefense.subject != obj)
    {
      // Does the subject's threat to this object exceed the current maximum ?
      U32 threat = heuristicData.maxThreatMinDefense.subject->UnitType()->GetThreat(obj->UnitType()->GetArmourClass());

      if 
      (
        // Is there more threat ?
        threat > heuristicData.maxThreatMinDefense.threat ||
        (
          // Is there equal threat but lest defense ?
          threat == heuristicData.maxThreatMinDefense.threat && 
          (
            obj->GetHitPoints() < heuristicData.maxThreatMinDefense.defense ||
            (
              // Is there equal threat and defense but less distance ?
              (obj->GetHitPoints() == heuristicData.maxThreatMinDefense.defense) &&
              (distance2 < heuristicData.maxThreatMinDefense.distance)
            )
          )
        )
      )
      {
        heuristicData.winner = obj;
        heuristicData.maxThreatMinDefense.threat = threat;
        heuristicData.maxThreatMinDefense.defense = obj->GetHitPoints();
        heuristicData.maxThreatMinDefense.distance = distance2;
      }
    }
  }

  
  //
  // MaxDanger
  //
  void MaxDanger(UnitObj *obj, F32 distance2, HeuristicData &heuristicData)
  {
    // Ignore if this is the subject or guard object
    if 
    (
      heuristicData.maxDanger.subject != obj &&
      heuristicData.maxDanger.guard != obj
    )
    {
      // Can we actually damage this object ? (don't target objects which we pose no threat to)
      if (heuristicData.maxDanger.subject->UnitType()->GetThreat(obj->UnitType()->GetArmourClass()))
      {
        // Get the threat of this unit to our guard object
        U32 threat = obj->UnitType()->GetThreat(heuristicData.maxDanger.guard->MapType()->GetArmourClass());

        // Is this object targetting our guard object ?
        if 
        (
          obj->GetWeapon() &&
          obj->GetWeapon()->GetTarget().CheckTarget(*heuristicData.maxDanger.guard)
        )
        {
          // Boost its threat
          threat *= TargetThreatBoost;
        }

        // Is this unit more threatening ?
        if 
        (
          threat > heuristicData.maxDanger.threat ||
          (
            // Is there the same threat but less defense ?
            threat == heuristicData.maxDanger.threat &&
            (
              !heuristicData.maxDanger.defense ||
              obj->GetHitPoints() < heuristicData.maxDanger.defense ||
              (
                // Is there the same threat and defense but less distance ?
                obj->GetHitPoints() == heuristicData.maxDanger.defense &&
                distance2 < heuristicData.maxDanger.distance
              )
            )
          )
        )
        {
          heuristicData.winner = obj;
          heuristicData.maxDanger.threat = threat;
          heuristicData.maxDanger.defense = obj->GetHitPoints();
          heuristicData.maxDanger.distance = distance2;
        }
      }
    }
  }


  //
  // Healer
  //
  void Healer(UnitObj *obj, F32 distance2, HeuristicData &heuristicData)
  {
    // Ignore if this is the subject
    if (heuristicData.healer.subject != obj)
    {
      // Is this object in the process of healing the healee ?
      Tasks::RestoreMobile *restoreMobileTask = TaskCtrl::Promote<Tasks::RestoreMobile>(obj->GetCurrentTask());

      if (restoreMobileTask && restoreMobileTask->GetTarget() == heuristicData.healer.healee)
      {
        // Does the subject's threat to this object exceed the current maximum ?
        U32 threat = heuristicData.healer.subject->UnitType()->GetThreat(obj->UnitType()->GetArmourClass());

        if 
        (
          threat > heuristicData.healer.threat ||
          (
            threat == heuristicData.healer.threat && 
            (
              obj->GetHitPoints() < heuristicData.healer.defense ||
              (
                obj->GetHitPoints() == heuristicData.healer.defense &&
                distance2 < heuristicData.healer.distance
              )
            )
          )
        )
        {
          heuristicData.winner = obj;
          heuristicData.healer.threat = threat;
          heuristicData.healer.defense = obj->GetHitPoints();
          heuristicData.healer.distance = distance2;
        }
      }
    }
  }


  //
  // MinHitPointPercentage
  //
  void MinHitPointPercentage(UnitObj *obj, F32 distance2, HeuristicData &heuristicData)
  {
    // Ignore if this is the subject
    if (heuristicData.minHitPointPercentage.subject != obj)
    {
      // Get this objects hit point percentage
      F32 p = obj->GetHitPointPercentage();

      if 
      (
        p < heuristicData.minHitPointPercentage.value ||
        (
          p == heuristicData.minHitPointPercentage.value &&
          distance2 < heuristicData.minHitPointPercentage.distance
        )
      )
      {
        heuristicData.winner = obj;
        heuristicData.minHitPointPercentage.value = p;
        heuristicData.minHitPointPercentage.distance = distance2;
      }
    }
  }


  //
  // Restore
  //
  void Restore(UnitObj *obj, F32 distance2, HeuristicData &heuristicData)
  {
    // Ignore if this is the subject
    if (heuristicData.restore.subject != obj)
    {
      // Does this object require restoration by this object
      if (heuristicData.restore.subject->RestoreRequired(obj))
      {
        if (distance2 < heuristicData.restore.distance)
        {
          heuristicData.winner = obj;
          heuristicData.restore.distance = distance2;
        }
      }
    }
  }


  //
  // Find
  //
  // Find an object using the given heuristic, 
  // heuristic data, filter, filter data
  //
  UnitObj * Find(Heuristic heuristic, HeuristicData &heuristicData, UnitObjIter::Filter filter, const UnitObjIter::FilterData &filterData)
  {
    ASSERT(heuristic)

    // Build the UnitObjIter
    UnitObjIter::Tactical i(filter, filterData);
    UnitObj *obj;

    while ((obj = i.Next()) != NULL)
    {
      heuristic(obj, i.GetProximity2(), heuristicData);
    }   

    return (heuristicData.winner);
  }

}
