/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Intel
//


#ifndef __STRATEGIC_INTEL_MANAGER_H
#define __STRATEGIC_INTEL_MANAGER_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_intel.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Intel::Manager
  //
  class Intel::Manager
  {
  private:

    // Strategic Object the manager belongs to
    Object *strategic;

    // All of the intels by name
    NBinTree<Intel> intels;

    // Intels which are idle
    NList<Intel> idle;

    // Intels which are being processed
    NList<Intel> process;

  public:

    // Constructor and destructor
    Manager();
    ~Manager();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope);

    // Process Intels
    void Process();

    // Add an intel
    Intel & AddIntel(const GameIdent &intelName, const GameIdent &configName);

    // Remove an intel
    void RemoveIntel(const GameIdent &intelName);

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

  };

}

#endif
