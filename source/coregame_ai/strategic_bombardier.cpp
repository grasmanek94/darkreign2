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
#include "strategic_bombardier_manager.h"
#include "stdload.h"
#include "resolver.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Bombardier
  //


  //
  // Constructor
  //
  Bombardier::Bombardier(Manager &manager, UnitObj *unit)
  : manager(manager),
    unit(unit)
  {
  }


  //
  // Destructor
  //
  Bombardier::~Bombardier()
  {
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Bombardier::SaveState(FScope *scope)
  {
    StdSave::TypeReaper(scope, "Unit", unit);
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Bombardier::LoadState(FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xD9A182E4: // "Unit"
        {
          StdLoad::TypeReaper(sScope, unit);
          Resolver::Object<UnitObj, UnitObjType>(unit);
          break;
        }
      }
    }
  }

}

