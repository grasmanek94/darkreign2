/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Orderer Type
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_base_orderer_type.h"
#include "strategic_util.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Struct Base::Orderer::Type
  //

  //
  // Loading Constructor
  //
  Base::Orderer::Type::Type(FScope *fScope, Team *team)
  {
    // Load the type
    type = Util::LoadType(fScope, team);

    if (!type->GetConstructorType() && !type->GetUpgradeFor())
    {
      fScope->ScopeError("Type '%s' has no constructor and isn't an upgrade for anything, so we can't build it", type->GetName());
    }

    // Load the amount
    amount = StdLoad::TypeU32(fScope, 1);
  }


  //
  // Initializing Constructor
  //
  Base::Orderer::Type::Type(UnitObjType &type)
  : type(&type),
    amount(0)
  {
  }


  //
  // SaveState
  //
  // The saved data must be consistent with the loading constructor and config files
  //
  void Base::Orderer::Type::SaveState(FScope *scope)
  {
    scope->AddArgString(type->GetName());
    scope->AddArgInteger(amount);
  }
}

