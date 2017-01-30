/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Base
//


#ifndef __STRATEGIC_BASE_TOKEN_H
#define __STRATEGIC_BASE_TOKEN_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_base.h"
#include "strategic_notification.h"
#include "strategic_resource.h"
#include "strategic_script.h"
#include "strategic_water.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Forward declarations
  //
  class Placement;

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Base::Token
  //
  class Base::Token
  {
  private:

    // The id from the manager
    U32 id;

    // Type of this token
    UnitObjType *type;

    // Base this token belongs to
    Base *base;

    // Orderer which created this token
    Orderer *orderer;

    // Placement which is finding locations for this token
    Placement *placement;

    // The location to use
    Vector location;
    WorldCtrl::CompassDir dir;

    // Object which is constructing this token
    UnitObjPtr constructor;

    // Constructor id
    U32 constructorId;

    // If this token is for a resource then this is the resource
    Reaper<Resource> resource;

    // If this token is for a water body then this is the water
    Reaper<Water> water;

    // If this toek is for a script then this is the script
    Reaper<Script> script;

    // State of the token
    enum State
    {
      // Tokens which are on order 
      ONORDER,

      // Token is waiting for a constructor
      WAITING,

      // Token is searching for someone to build
      SEARCHING,

      // Token is constructing from a facility
      FACILITYCONSTRUCTING,

      // Token is locating the point of construction
      LOCATING,

      // Token is constructing
      CONSTRUCTING,

      // Token is upgrading
      UPGRADING,

      // None state should only be used by the destructor
      NONE,

      MAX_STATE
    } state;

    // Token priority
    U32 priority;

    // Node for using this token in lists
    NList<Token>::Node nodeList;

    // Node for using this token in trees
    NBinTree<Token>::Node nodeBase;

    // Node for all tokens in a base
    NList<Token>::Node nodeBaseAll;

    // Node for using this token in the base manager
    NBinTree<Token>::Node nodeManager;

    // Node for all tokens in a manager
    NList<Token>::Node nodeManagerAll;

    // Names of the states
    static const char * stateNames[];

  public:

    // Constructor
    Token(Base &base, UnitObjType &type, Orderer &orderer, Resource *resource, Water *water, Script *script);

    // Loading constructor
    Token(FScope *scope, Manager &manager);

    // Destructor
    ~Token();

    // Save state data
    void SaveState(FScope *scope);

    // Check to see if there's an availabe constructor
    void CheckConstructor(const UnitObjList &buildings, const UnitObjList &constructors);

    // Orphan this token
    void Orphan();

    // The token has been accepted
    void Accepted();

    // The token has been returned
    void Returned();

    // Set the location
    void SetLocation(const Vector &location, WorldCtrl::CompassDir dir);

    // No location found
    void NoLocation();

    // Set the constructor
    void SetConstructor(UnitObj *constructor);

    // Return constructor
    void ReturnConstructor();

    // Handle notifications
    void Notify(Notification &notification);

    // GetState
    const char * GetState();

  private:

    // Perform a state change
    void SetState(State newState);

    // Deal with a freshly constructed unit
    void NewUnit(UnitObj *unit);

  public:

    // Get the id of this token
    U32 GetId()
    {
      return (id);
    }

    // Get the type of this token
    UnitObjType & GetType()
    {
      ASSERT(type)
      return (*type);
    }

    // Get the base this token belongs to
    Base & GetBase()
    {
      ASSERT(base)
      return (*base);
    }

    // Get the orderer from this token
    Orderer * GetOrderer()
    {
      return (orderer);
    }

    // Set the orderer for this token
    void SetOrderer(Orderer *o)
    {
      ASSERT(!orderer)
      orderer = o;
    }

    // Get priority
    U32 GetPriority()
    {
      return (priority);
    }

    // Get the constructor associated with this token
    UnitObj * GetConstructor()
    {
      return (constructor.GetPointer());
    }

    // Get the resource associated with this token
    Resource * GetResource()
    {
      return (resource.GetPointer());
    }

    // Get the water associated with this token
    Water * GetWater()
    {
      return (water.GetPointer());
    }

  public:

    friend class Base;
    friend class Orderer;
    friend class Manager;

  };

}

#endif
