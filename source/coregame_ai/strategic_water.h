/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Resource
//


#ifndef __STRATEGIC_WATER_H
#define __STRATEGIC_WATER_H


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
  // Class Water
  //
  class Water
  {
  public:

    /////////////////////////////////////////////////////////////////////////////
    //
    // Forward Declarations
    //
    class Manager;
    class Decomposition;
    struct Body;

  private:

    // Water manager which is managing this water
    Manager &manager;

    // Manager node
    NBinTree<Water, F32>::Node nodeManager;

    // Id
    U32 id;

    // Expanse of the map
    F32 mapExpanse;

    // Coverage of the map
    F32 mapCoverage;

    // Closest point to base
    Point<F32> closest;

    // Units assigned to this water
    UnitObjList units;

    // Units which are on order for this water
    UnitObjTypeList onOrder;

    // Units which are in construction for this water
    UnitObjTypeList inConstruction;
   
  public:

    // Death track info
    DTrack::Info dTrack;

  public:

    // Constructors
    Water(Manager &manager, const Body &body, U32 id, const Point<F32> &closest);
    Water(Manager &manager, U32 id);

    // Destructor
    ~Water();

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

    // Get the id of this water
    U32 GetId()
    {
      return (id);
    }

    // Get the list of units assigned to this water
    const UnitObjList & GetUnits() const
    {
      return (units);
    }
    
    // Get the list of types which are on order for this water
    const UnitObjTypeList & GetOnOrder() const
    {
      return (onOrder);
    }

    // Closest point to base
    const Point<F32> GetClosest() const
    {
      return (closest);
    }

  public:

    // Friends
    friend class Manager;

  };

}

#endif
