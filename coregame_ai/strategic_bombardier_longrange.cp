/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Bombardier
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_bombardier.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Bombardier::LongRange
  //


  //
  // Constructor
  //
  Bombardier::LongRange::LongRange(Manager &manager, UnitObj *unit)
  : Bombardier(manager, unit)
  {
  }


  //
  // Destructor
  //
  Bombardier::LongRange::~LongRange()
  {
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Bombardier::LongRange::SaveState(FScope *scope)
  {
    Bombardier::SaveState(scope);
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Bombardier::LongRange::LoadState(FScope *scope)
  {
    Bombardier::LoadState(scope);
  }


  //
  // Process the bombardier
  //
  Bool Bombardier::LongRange::Process()
  {
    // If the unit has died then bail
    if (!unit.Alive())
    {
      return (TRUE);
    }

    return (FALSE);
  }

}
