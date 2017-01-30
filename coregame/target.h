///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Target
// 30-SEP-1998
//


#ifndef __TARGET_H
#define __TARGET_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mapobj.h"
#include "mathtypes.h"
#include "fscope.h"
#include "unitobjdec.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
const U32 TargetThreatBoost = 10;


///////////////////////////////////////////////////////////////////////////////
//
// Class Target
//
class Target
{
public:

  enum Type
  {
    INVALID,
    OBJECT,
    LOCATION
  };
  
private:

  Type      type;
  MapObjPtr object;
  Vector    location;

public:

  // Constructors

  // NULL constructor
  Target()
  : type(INVALID) 
  { 
  }

  // OBJECT constructor
  Target(MapObj *object) 
  : type(OBJECT), object(object) 
  { 
  }

  // LOCATION constructor
  Target(const Vector &location) 
  : type(LOCATION), location(location) 
  { 
  }

  // Copy constructor
  Target(const Target &target);

  // Return the target type
  Type GetType() const
  {
    return (type);
  }

  // Return the target object
  MapObjPtr & GetObj() const
  {
    ASSERT(type == OBJECT)
    return (const_cast<MapObjPtr &>(object));
  }

  // Return the target object
  MapObj * GetObject() const
  {
    return ((type == OBJECT) ? object.GetPointer() : NULL);
  }

  // Return the target location
  const Vector & GetLocation() const;

  // Is the target alive ?
  Bool Alive() const
  {
    switch (type)
    {
      case OBJECT:
        return (const_cast<MapObjPtr &>(object).Alive());
        break;

      case LOCATION:
        return (TRUE);
        break;

      case INVALID:
        return (FALSE);

      default:
        ERR_FATAL(("Unknown Target Type"))
        break;
    }
  }

  // Check the target against this object
  Bool CheckTarget(MapObj &mapObj) const
  {
    switch (type)
    {
      case OBJECT:
        return (object.DirectId() == mapObj.Id());

      default:
        return (FALSE);
    }
  }

  // Is the target valid
  Bool Valid()
  {
    // If we are targetting an object which has gone off the map, invalidate it
    if (type == OBJECT && object.Alive() && !object->OnMap())
    {
      type = INVALID;
    }

    // Is the target invalid ?
    return (type != INVALID);
  }

  Bool Valid() const
  {
    // If we are targetting an object which has gone off the map, invalidate it
    if (type == OBJECT && object.Alive() && !object->OnMap())
    {
      return (FALSE);
    }
    return (type != INVALID);
  }

  // Invalidate a target
  void InValidate()
  {
    type = INVALID;
  }

  // Assignment
  const Target & operator=(const Target &target)
  {
    type = target.type;
    object = target.object;
    location = target.location;
    return (*this);
  }

  // Load and save state configuration
  void LoadState(FScope *fScope);
  void SaveState(FScope *fScope);
  void PostLoad();
  
  // Is this target visible to the given team (must be Valid)
  Bool IsVisible(Team *team) const;

  // Is this target visible to the given unit (must be Valid)
  Bool IsVisible(UnitObj *unit) const;

  // Get info
  const char * GetInfo() const;

};


#endif