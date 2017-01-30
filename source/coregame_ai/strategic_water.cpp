/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Water
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_private.h"
#include "strategic_water.h"
#include "strategic_water_manager.h"
#include "strategic_water_decomposition.h"
#include "strategic_object.h"
#include "orders_game.h"
#include "resolver.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Water
  //

  // Number of cycles between updates
  const U32 updateInterval = 450;


  //
  // Constructor
  //
  Water::Water(Manager &manager, const Body &body, U32 id, const Point<F32> &closest)
  : manager(manager),
    id(id),
    closest(closest),
    mapExpanse(body.mapExpanse),
    mapCoverage(body.mapCoverage)
  {
    // Register our construction
    RegisterConstruction(dTrack);
  }


  //
  // Constructor
  //
  Water::Water(Manager &manager, U32 id)
  : manager(manager),
    id(id)
  {
    // Register our construction
    RegisterConstruction(dTrack);
  }


  //
  // Destructor
  //
  Water::~Water()
  {
    // Clear on order types
    onOrder.Clear();

    // Clear units assigned
    units.Clear();

    // Register our demise
    RegisterDestruction(dTrack);
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Water::SaveState(FScope *scope)
  {
    StdSave::TypeF32(scope, "MapExpanse", mapExpanse);
    StdSave::TypeF32(scope, "MapCoverage", mapCoverage);
    StdSave::TypeReaperList(scope, "Units", units);
    StdSave::TypeReaperListObjType(scope, "OnOrder", onOrder);
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Water::LoadState(FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x9C434D0C: // "MapExpanse"
          mapExpanse = StdLoad::TypeF32(sScope);
          break;

        case 0x3C1F3E04: // "MapCoverage"
          mapCoverage = StdLoad::TypeF32(sScope);
          break;

        case 0xCED02493: // "Units"
          StdLoad::TypeReaperList(sScope, units);
          Resolver::ObjList<UnitObj, UnitObjType, UnitObjListNode>(units);
          break;

        case 0x189FFF75: // "OnOrder"
          StdLoad::TypeReaperListObjType(sScope, onOrder);
          Resolver::TypeList<UnitObjType, UnitObjTypeListNode>(onOrder);
          break;
      }
    }
  }


  //
  // Add on Order
  //
  void Water::AddOnOrder(UnitObjType &type)
  {
    // On order types
    onOrder.Append(&type);
  }


  //
  // Remove on order
  //
  void Water::RemoveOnOrder(UnitObjType &type)
  {
    // Find this type in the on order types
    if (!onOrder.Remove(&type, TRUE))
    {
      ERR_FATAL(("Could not find type '%s' in the on order types", type.GetName()))
    }
  }


  //
  // Add unit
  //
  void Water::AddUnit(UnitObj *unit)
  {
    ASSERT(unit)

    // Add to other units
    units.Append(unit);
  }


  //
  // Process
  //
  Bool Water::Process()
  {
    return (FALSE);
  }
}

