/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Asset Request
// 25-MAR-1999
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_asset_request.h"
#include "strategic_asset_manager.h"
#include "strategic_object.h"
#include "orders_game.h"
#include "orders_squad.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Asset::Request
  //


  //
  // Constructor
  //
  Asset::Request::Request(Script *script, U32 handle)
  : script(script),
    handle(handle),
    assigned(&Asset::nodeAssigned),
    evaluationIter(&evaluation)
  {
  }

  
  //
  // Constructor
  //
  Asset::Request::Request()
  : assigned(&Asset::nodeAssigned),
    evaluationIter(&evaluation)
  {
  }

  
  //
  // Destructor
  //
  Asset::Request::~Request()
  {
    // Make sure there's no assets lying around
    assigned.UnlinkAll();

    // Make sure there's no assets being evaluated
    evaluation.UnlinkAll();

    // Make sure there's no residual siblings
    siblings.UnlinkAll();
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Asset::Request::SaveState(FScope *scope)
  {
    Item::SaveState(scope->AddFunction("Parent"));

    if (script.Alive())
    {
      StdSave::TypeU32(scope, "Script", script->GetNameCrc());
    }

    StdSave::TypeU32(scope, "Handle", handle);
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Asset::Request::LoadState(FScope *scope, void *context)
  {
    // The context holds a pointer to the asset manager
    Manager *manager = reinterpret_cast<Manager *>(context);

    ASSERT(manager)

    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x411AC76D: // "Parent"
          Item::LoadState(sScope, context);
          break;

        case 0x8810AE3C: // "Script"
          script = manager->GetObject().GetScriptManager().FindScript(StdLoad::TypeU32(sScope));
          break;

        case 0xB5EA6B79: // "Handle"
          handle = StdLoad::TypeU32(sScope);
          break;
      }
    }
  }


  //
  // Completed
  //
  // Give the assigned units to the squad and notify the 
  // script manager that the request was completed
  //
  void Asset::Request::Completed(Manager &manager)
  {
    // Get the strategic object
    Object & object = manager.GetObject();

    // Clear the currently selected units
    Orders::Game::ClearSelected::Generate(object);

    // Build a unit list from the asset list
    UnitObjList units;
    for (NList<Asset>::Iterator a(&assigned); *a; a++)
    {
      UnitObj *unit = (*a)->GetUnit();

      if (unit)
      {
        units.Append(unit);
      }
    }

    if (units.GetCount())
    {
      // Get the strategic player to select the assigned objects
      Orders::Game::AddSelected::Generate(object, units);

      // Clear out the temporary unit list
      units.Clear();

      // Assign these objects to the squad
      if (script.Alive() && script->IsSquadAlive())
      {
        Orders::Squad::AddSelected::Generate(object, script->GetSquad()->Id());
      }
    }

    if (script.Alive())
    {
      // Notify that the recruit is completed
      Orders::Squad::Notify::Generate(object, script->GetSquad()->Id(), 0xE6DC8EF5, handle); // "Squad::RecruitComplete"
    }

    // Delete the assigned assets
    assigned.UnlinkAll();
  }


  //
  // Reset
  //
  void Asset::Request::Reset()
  {
    // Make sure there's no assets being evaluated
    evaluation.UnlinkAll();

    // Make sure there's no siblings lying around
    siblings.UnlinkAll();
  }


  //
  // Abandoned
  //
  // Abandon the assigned units and give them back to the
  // strategic asset manager
  //
  void Asset::Request::Abandoned(Manager &manager)
  {
    manager;

/*
    LOG_AI(("%s: Abandoning units:", script.Alive() ? script->GetName() : "DEAD"))
    for (NList<Asset>::Iterator u(&assigned); *u; u++)
    {
      if ((*u)->Alive())
      {
        LOG_AI(("[%d] %s", (*u)->GetId(), (*u)->GetName()))
      }
    }
*/

    assigned.UnlinkAll();
  }

}
