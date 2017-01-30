///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-APR-1998
//


#ifndef __UNITOBJITER_H
#define __UNITOBJITER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "unitobj.h"
#include "team.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace UnitObjIter
//
namespace UnitObjIter
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct FilterData
  //
  struct FilterData
  {
    // Team to test relations against
    Team *team;

    // Relationship in with team
    Relation relation;

    // Location to perform proximity tests from
    Vector location;

    // Distance for proximity calculations
    F32 proximity;
    F32 proximity2;

    FilterData(Team *team, Relation relation, const Vector &location, F32 proximity) : 
      team(team),
      relation(relation),
      location(location),
      proximity(proximity),
      proximity2(proximity * proximity)
    {
    }

    FilterData(const Vector &location, F32 proximity, Team *team = NULL) : 
      location(location),
      proximity(proximity),
      proximity2(proximity * proximity),
      team(team)
    {
    }
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct FilterDataUnit
  //
  struct FilterDataUnit : public FilterData
  {
    // Unit who is the source of the search
    UnitObj *source;

    FilterDataUnit(Relation relation, UnitObj *source) 
    : FilterData(source->GetTeam(), relation, source->Origin(), WorldCtrl::CellSize() * source->GetSeeingRange()),
      source(source)
    {
    }

    FilterDataUnit(UnitObj *source) 
    : FilterData(source->Origin(), WorldCtrl::CellSize() * source->GetSeeingRange()),
      source(source)
    {
    }

  };

  // Filter
  typedef Bool (*Filter)(UnitObj *, const FilterData &);

  // Pre Defined Filters

  // Can be seen by the unit (requires FilterDataUnit!)
  Bool CanBeSeenBy(UnitObj *subject, const FilterData &filterData);

  // Can be seen by this team
  Bool CanBeSeenByTeam(UnitObj *subject, const FilterData &filterData);


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Tactical
  //
  class Tactical
  {
  private:

    // Cluster info
    U32 startX, endX, currentX;
    U32 startZ, endZ, currentZ;

    // Iterator for current cluster
    NList<UnitObj>::Iterator cluster;

    // Filter data
    const FilterData &filterData;

    // Filter function
    Filter filter;

    // Proximity squared of the current object
    F32 proximity2;

  public:

    // Constructor
    Tactical(Filter filter, const FilterData &filterData);

    // Iterate to the next object
    UnitObj * Next();

    // Get the proximity of the current object squard
    F32 GetProximity2()
    {
      return (proximity2);
    }
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class ClaimRect
  //
  class ClaimRect
  {
  private:

    // Area in grains
    Point<S32> g0;
    Point<S32> g1;

    // Current iterator position
    S32 x, z;

  public:

    // Constructor
    ClaimRect(const Point<S32> &p0, const Point<S32> &p1);

    // Iterate to the next object
    UnitObj *Next();
  };
}

#endif
