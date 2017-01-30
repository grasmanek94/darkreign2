/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Water
//


#ifndef __STRATEGIC_WATER_MANAGER_H
#define __STRATEGIC_WATER_MANAGER_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_water.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Water::Manager
  //
  class Water::Manager
  {
  private:

    // Strategic Object the manager belongs to
    Object *strategic;

    // The water id counter
    U32 waterId;

    // Water
    NBinTree<Water, F32> waters;

  public:

    // Constructor and destructor
    Manager();
    ~Manager();

    // Reset
    void Reset();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope);

    // Process water
    void Process();

    // Find a water
    Water * FindWater(U32 id);

  public:

    // Setup
    void Setup(Object *object)
    {
      strategic = object;
    }

    // Get the strategic object from the manager
    Object & GetObject()
    {
      ASSERT(strategic)
      return (*strategic);
    }

    // Get the waters
    const NBinTree<Water, F32> & GetWaters()
    {
      return (waters);
    }

  };

}

#endif
