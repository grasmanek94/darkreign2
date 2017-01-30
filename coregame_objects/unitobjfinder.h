///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-APR-1998
//


#ifndef __UNITOBJFINDER_H
#define __UNITOBJFINDER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "unitobjiter.h"
#include "restoreobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace UnitObjFinder
//
namespace UnitObjFinder
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct HeuristicData
  //
  struct HeuristicData
  {
    UnitObj *winner;

    union
    {
      struct
      {
        UnitObj *subject;
      } random;

      struct
      {
        UnitObj *subject;
        U32 threat;
        S32 defense;
        F32 distance;
      } maxThreatMinDefense;

      struct
      {
        UnitObj *subject;
        MapObj *guard;
        U32 threat;
        S32 defense;
        F32 distance;
      } maxDanger;

      struct
      {
        UnitObj *subject;
        UnitObj *healee;
        U32 threat;
        S32 defense;
        F32 distance;
      } healer;

      struct
      {
        UnitObj *subject;
        F32 value;
        F32 distance;
      } minHitPointPercentage;

      struct
      {
        RestoreObj *subject;
        F32 distance;
      } restore;
    };

    // Constructors
    HeuristicData() :
      winner(NULL)
    {
    }

    //
    // Type specific setup functions
    //
    void SetupRandom(UnitObj *subject)
    {
      ASSERT(subject)
      random.subject = subject;
    }

    void SetupMaxThreatMinDefense(UnitObj *subject)
    {
      ASSERT(subject)
      maxThreatMinDefense.subject = subject;
      maxThreatMinDefense.threat = 0;
      maxThreatMinDefense.defense = 0;
      maxThreatMinDefense.distance = F32_MAX;
    }

    void SetupMaxDanger(UnitObj *subject, MapObj *guard = NULL)
    {
      ASSERT(subject)
      maxDanger.subject = subject;
      maxDanger.guard = guard ? guard : subject;
      maxDanger.threat = 0;
      maxDanger.defense = 0;
      maxDanger.distance = F32_MAX;
    }

    void SetupHealer(UnitObj *subject, UnitObj *healee)
    {
      ASSERT(subject)
      healer.subject = subject;
      healer.healee = healee;
      healer.threat = 0;
      healer.defense = 0;
      healer.distance = F32_MAX;
    }

    void SetupMinHitPointPercentage(UnitObj *subject)
    {
      ASSERT(subject)
      minHitPointPercentage.subject = subject;
      minHitPointPercentage.value = F32_MAX;
      minHitPointPercentage.distance = F32_MAX;
    }

    void SetupRestore(RestoreObj *subject)
    {
      ASSERT(subject)
      restore.subject = subject;
      restore.distance = F32_MAX;
    }

  };

  // Heuristic Definition
  typedef void (*Heuristic)(UnitObj *obj, F32 distance2, HeuristicData &heuristicData);

  // Pre defined Heuristics
  void Random(UnitObj *obj, F32 distance2, HeuristicData &heuristicData);
  void MaxThreatMinDefense(UnitObj *obj, F32 distance2, HeuristicData &heuristicData);
  void MaxDanger(UnitObj *obj, F32 distance2, HeuristicData &heuristicData);
  void Healer(UnitObj *obj, F32 distance2, HeuristicData &heuristicData);
  void MinHitPointPercentage(UnitObj *obj, F32 distance2, HeuristicData &heuristicData);
  void Restore(UnitObj *obj, F32 distance2, HeuristicData &heuristicData);

  // Find an object using the given heuristic, heuristic data, filter, filter data
  UnitObj * Find(Heuristic heuristic, HeuristicData &heuristicData, UnitObjIter::Filter filter, const UnitObjIter::FilterData &filterData); 

}

#endif