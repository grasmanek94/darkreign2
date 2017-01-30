/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Bombardier
//


#ifndef __STRATEGIC_BOMBARDIER_H
#define __STRATEGIC_BOMBARDIER_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic.h"
#include "offmapobj.h"


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
  // Class Bombardier
  //
  class Bombardier
  {
  public:

    /////////////////////////////////////////////////////////////////////////////
    //
    // Forward Declarations
    //
    class Manager;

    class OffMap;
    class Unit;
    class RuleSet;

  protected:

    // Bombardier manager which is managing this bombardier
    Manager &manager;

    // Manager node
    NList<Bombardier>::Node nodeManager;

    // Unit which this bombardier is operating for
    UnitObjPtr unit;

  public:

    // Constructor
    Bombardier(Manager &manager, UnitObj *unit);

    // Destructor
    ~Bombardier();

    // Save and load state
    virtual void SaveState(FScope *scope);
    virtual void LoadState(FScope *scope);

    // Process the bombardier
    virtual Bool Process() = 0;

    // Get the type name of the bombardier
    virtual const char * GetName() = 0;

    // Get info about the bombardier
    virtual const char * GetInfo() { return (""); }

  public:

    // Get the script manager in control of this script
    Manager & GetManager()
    {
      return (manager);
    }

    // Get the bombardier unit
    UnitObj * GetUnit()
    {
      return (unit.GetPointer());
    }

  public:

    // Friends
    friend class Manager;

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Bombardier::OffMap
  //
  // OffMap bombardiers
  //
  class Bombardier::OffMap : public Bombardier
  {
  private:

    // Type of bomb
    OffMapObjTypePtr offMapType;

    // Current bombardier state
    enum State
    {
      // Idle
      IDLE,

      // Waiting for the bomb to finish construction
      WAITING,
      
      // Releasing a device
      RELEASE,

      // Released 
      RELEASED,

      MAX_STATES

    } state;

    // Are we searching for somewhere to drop the bomb ?
    Bool searching;

    // What is the cluster we're currently evaluating
    Point<U32> evaluating;

    // Current winning cluster
    Point<U32> winningCluster;

    // Score at the winning locating
    F32 winningScore;

    // Time of the next launch
    F32 nextTime;

    // Names of the states
    static const char *stateNames[];

  public:

    // Constructor
    OffMap(Manager &manager, UnitObj *unit);

    // Destructor
    ~OffMap();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope);

    // Process the bombardier
    Bool Process();

    // Get info about the bombardier
    const char * GetInfo();

  public:

    // Get the current state
    const char * GetState()
    {
      ASSERT(state < MAX_STATES)
      return (stateNames[state]);
    }

    // Get the off map object
    OffMapObjType * GetOffMapType()
    {
      return (offMapType.GetPointer());
    }

    // Get the type name of the bombardier
    const char * GetName()
    {
      return ("Bombardier::OffMap");
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Bombardier::Unit
  //
  class Bombardier::Unit : public Bombardier
  {
  private:

    // What is the cluster we're currently evaluating
    Point<U32> evaluating;

    // Current winning cluster
    Point<U32> winningCluster;

    // Score at the winning locating
    F32 winningScore;

  public:

    // Constructor
    Unit(Manager &manager, UnitObj *unit);

    // Destructor
    ~Unit();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope);

    // Process the bombardier
    Bool Process();

  public:

    // Get the type name of the bombardier
    const char * GetName()
    {
      return ("Bombardier::Unit");
    }

  };

}

#endif
