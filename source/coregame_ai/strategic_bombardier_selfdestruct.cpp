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
  // Class Bombardier::SelfDestruct
  //


  //
  // Constructor
  //
  Bombardier::SelfDestruct::SelfDestruct(Manager &manager, UnitObj *unit)
  : Bombardier(manager, unit)
  {
  }


  //
  // Destructor
  //
  Bombardier::SelfDestruct::~SelfDestruct()
  {
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Bombardier::SelfDestruct::SaveState(FScope *scope)
  {
    Bombardier::SaveState(scope);
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Bombardier::SelfDestruct::LoadState(FScope *scope)
  {
    Bombardier::LoadState(scope);
  }


  //
  // Process the bombardier
  //
  Bool Bombardier::SelfDestruct::Process()
  {
    // If the unit has died then bail
    if (!unit.Alive())
    {
      return (TRUE);
    }

    return (FALSE);
  }

}

