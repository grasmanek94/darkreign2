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
#include "resolver.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Asset::Request::Squad
  //


  //
  // Constructor
  //
  Asset::Request::Squad::Squad(Script *script, U32 handle, SquadObj *from)
  : Asset::Request(script, handle),
    from(from)
  {
  }


  //
  // Constructor
  //
  Asset::Request::Squad::Squad()
  {
  }


  //
  // Destructor
  //
  Asset::Request::Squad::~Squad()
  {
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Asset::Request::Squad::SaveState(FScope *scope)
  {
    // Add the name of this type as an argument
    scope->AddArgString(GetName());

    // Save the squad
    StdSave::TypeReaper(scope, "From", from);

    // Save parent data
    Request::SaveState(scope->AddFunction("Parent"));
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Asset::Request::Squad::LoadState(FScope *scope, void *context)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xD1E197A4: // "From"
          StdLoad::TypeReaper(sScope, from);
          Resolver::Object<SquadObj, SquadObjType>(from);
          break;

        case 0x411AC76D: // "Parent"
          Request::LoadState(sScope, context);
          break;
      }
    }
  }


  //
  // Reset
  //
  void Asset::Request::Squad::Reset()
  {
    Request::Reset();
  }


  //
  // Submit asset requests directly to the manager
  //
  Bool Asset::Request::Squad::DirectEvaluation(Manager &manager)
  {
    if (from.Alive())
    {
      for (UnitObjList::Iterator i((UnitObjList *) &from->GetList()); *i; i++)
      {
        if ((*i)->Alive())
        {
          manager.Evaluation(*this, ***i, 1.0f);
        }
      }
    }
    return (TRUE);
  }


  //
  // Evaluate
  //
  // Evaluate the given asset
  //
  F32 Asset::Request::Squad::Evaluate(Asset &)
  {
    return (0.0f);
  }


  //
  // Accept
  //
  // The given asset has been accepted
  //
  void Asset::Request::Squad::Accept(Asset &asset, Bool primary)
  {
    if (primary)
    {
      // Add the object to the list which are assigned to us
      assigned.Append(&asset);
    }
  }


  //
  // Offer
  //
  // The given asset is being offered, do we want it ?
  //
  Bool Asset::Request::Squad::Offer(Asset &)
  {
    // We want 'em all
    return (TRUE);
  }


  //
  // OutOfAssets
  //
  // The asset manager is informing us there's no more assets
  //
  Bool Asset::Request::Squad::OutOfAssets(Manager &manager)
  {
    Completed(manager);
    return (TRUE);
  }

}
