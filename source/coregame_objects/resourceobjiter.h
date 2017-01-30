///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-APR-1998
//


#ifndef __RESOURCEOBJITER_H
#define __RESOURCEOBJITER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "resourceobjdec.h"
#include "unitobj.h"
#include "team.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace ResourceObjIter
//
namespace ResourceObjIter
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct FilterData
  //
  struct FilterData
  {
    // Location to perform proximity tests from
    Vector location;

    // Distance for proximity calculations
    F32 proximity;
    F32 proximity2;

    FilterData(const Vector &location, F32 proximity) : 
      location(location),
      proximity(proximity),
      proximity2(proximity * proximity)
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

    FilterDataUnit(UnitObj *source) :
      FilterData(source->Origin(), WorldCtrl::CellSize() * source->GetSeeingRange()),
      source(source)
    {
    }

  };


  // Filter
  typedef Bool (*Filter)(ResourceObj *, const FilterData &);

  // Pre Defined Filters

  // Can be seen by the unit (requires FilterDataUnit!)
  Bool CanBeSeenBy(ResourceObj *subject, const FilterData &filterData);

  // Has been seen by the unit's team (requires FilterDataUnit!)
  Bool HasBeenSeenByTeam(ResourceObj *subject, const FilterData &filterData);

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Resource
  //
  class Resource
  {
  private:

    // Cluster info
    U32 startX, endX, currentX;
    U32 startZ, endZ, currentZ;

    // Iterator for current cluster
    NList<ResourceObj>::Iterator cluster;

    // Filter data
    const FilterData &filterData;

    // Filter function
    Filter filter;

    // Proximity squared of the current object
    F32 proximity2;

  public:

    // Constructor
    Resource(Filter filter, const FilterData &filterData);

    // Iterate to the next object
    ResourceObj * Next();

    // Get the proximity of the current object squared
    F32 GetProximity2()
    {
      return (proximity2);
    }
  };
}

#endif