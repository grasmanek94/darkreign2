/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Asset
// 25-MAR-1999
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_asset.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Asset
  //

  
  //
  // Asset::Asset
  //
  // Unit Constructor
  //
  Asset::Asset(UnitObj *unit)
  : unit(unit)
  {
  }


  //
  // Asset::~Asset
  //
  // Destructor
  //
  Asset::~Asset()
  {
    // Unlink requests list
    requests.UnlinkAll();

    // Make sure we're not in any lists/trees
    ASSERT(!nodeManager.InUse())
    ASSERT(!nodeAssigned.InUse())
  }


  //
  // Asset::GetId
  //
  U32 Asset::GetId()
  {
    ASSERT(unit.Alive())
    return (unit.Id());
  }


  //
  // Asset::GetName
  //
  // A name for the object
  //
  const char * Asset::GetName()
  {
    return (unit->GameType()->GetName());
  }


  //
  // Asset::Alive
  //
  Bool Asset::Alive()
  {
    return (unit.Alive());
  }

  
  //
  // Asset::GetUnit
  //
  // Get the unit from the asset
  //
  UnitObj * Asset::GetUnit()
  {
    if (unit.Alive())
    {
      return (unit);
    }
    else
    {
      return (NULL);
    }
  }

}