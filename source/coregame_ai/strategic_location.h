/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic AI
//
// 31-AUG-1998
//


#ifndef __STRATEGIC_LOCATION_H
#define __STRATEGIC_LOCATION_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "ai.h"


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
  class Object;
  class Script;


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Location
  //
  class Location
  {
  public:

    // Forward Declarations
    class Base;
    class Offset;

  private:

    // Base part of the location
    Base *base;

    // Offset to the base part of the location
    Offset *offset;

  private:

    // Constructor
    Location(Base *base, Offset *offset);

  public:

    // Destructor
    ~Location();

    // Get the vector position of the location
    Vector GetVector(Object *object = NULL, Script *script = NULL);

    // Get the map location of the location
    Point<F32> GetPoint(Object *object = NULL, Script *script = NULL);

    // Create a location from the given fscope
    static Location * Create(FScope *fScope, Script *script); // = NULL);

  };

}


#endif
