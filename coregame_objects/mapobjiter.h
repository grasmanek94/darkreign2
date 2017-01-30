///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-APR-1998
//


#ifndef __MAPOBJITER_H
#define __MAPOBJITER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mapobj.h"
#include "team.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MapObjIter
//
namespace MapObjIter
{

  // Iterator ticker to filter out duplicated
  extern U32 iterTicker;


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

    // Location to perform proximity tests fromt
    Vector location;

    // Distance for proximity calculations
    F32 proximity;            
    F32 proximity2;           

    FilterData(Team *team, Relation relation, const Vector &location, F32 proximity) 
    : team(team),
      relation(relation),
      location(location),
      proximity(proximity),
      proximity2(proximity * proximity)
    {
    }

    FilterData(const Vector &location, F32 proximity) 
    : location(location),
      proximity(proximity),
      proximity2(proximity * proximity),
      team(NULL)
    {
    }

  };

  // Filter
  typedef Bool (*Filter)(MapObj *, const FilterData &);

  // Pre Defined Filters
  //Bool IsFatBoy(MapObj *subject, const FilterData &filterData);


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class All 
  //
  // Avoid using this iterator at all costs.
  //
  // If you use this iterator and give a team, each object will need to be
  // promoted to a unit to test its team, just so you know.
  //
  // Also, this iterator cannot be used recursively!
  // 
  //
  class All
  {
  private:

    // Cluster info
    U32 startX, endX, currentX;           
    U32 startZ, endZ, currentZ;

    // Iterator for current cluster
    NList<MapObj>::Iterator cluster;      

    // Filter data
    const FilterData &filterData;  

    // Filter function
    Filter filter;       

    // Proximity squared of the current object
    F32 proximity2;

  public:

    // Constructor
    All(Filter filter, const FilterData &filterData);

    // Iterate to the next object
    MapObj * Next();

    // Get the proximity of the current object squard
    F32 GetProximity2()
    {
      return (proximity2);
    }
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Line
  //
  // Iterate all objects that occur on the line given
  //
  class Line
  {
  private:

    // Pointers to major and minor axes
    F32 Point<F32>::* major;
    F32 Point<F32>::* minor;

    // Ray tests to use
    U32 tests;

    // Current offset within a cluster
    F32 offset;
    Point<U32> cluster;

    // Iterator for the current cluster
    NList<MapObj>::Iterator iterator;

  public:

    // Constructor
    Line(const Vector &src, const Vector &dst, U32 tests);

    // Iterate to the next object
    MapObj * Next();

  };

  // Initialise
  void Init();

  // Return value of ticker
  inline U32 GetIterTicker()
  {
    return (iterTicker);
  }

  // Increment value of ticker
  inline void IncIterTicker()
  {
    ++iterTicker;
  }

}

#endif