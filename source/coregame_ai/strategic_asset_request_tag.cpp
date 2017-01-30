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
#include "tagobj.h"
#include "promote.h"
#include "resolver.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Asset::Request::Tag
  //


  //
  // Constructor
  //
  Asset::Request::Tag::Tag(Script *script, U32 handle, TagObj *tag)
  : Asset::Request(script, handle),
    tag(tag)
  {
  }

  
  //
  // Constructor
  //
  Asset::Request::Tag::Tag()
  {
  }


  //
  // Destructor
  //
  Asset::Request::Tag::~Tag()
  {
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Asset::Request::Tag::SaveState(FScope *scope)
  {
    // Add the name of this type as an argument
    scope->AddArgString(GetName());

    // Save the tag
    StdSave::TypeReaper(scope, "Tag", tag);

    // Save parent data
    Request::SaveState(scope->AddFunction("Parent"));
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Asset::Request::Tag::LoadState(FScope *scope, void *context)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x1E534497: // "Tag"
          StdLoad::TypeReaper(sScope, tag);
          Resolver::Object<TagObj, TagObjType>(tag);
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
  void Asset::Request::Tag::Reset()
  {
    Request::Reset();
  }


  //
  // Submit asset requests directly to the manager
  //
  Bool Asset::Request::Tag::DirectEvaluation(Manager &manager)
  {
    if (tag.Alive())
    {
      tag->list.PurgeDead();
  
      for (MapObjList::Iterator i(&tag->list); *i; i++)
      {
        if ((*i)->Alive())
        {
          UnitObj *unitObj = Promote::Object<UnitObjType, UnitObj>(**i);

          if (unitObj)
          {
            manager.Evaluation(*this, *unitObj, 1.0f);
          }
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
  F32 Asset::Request::Tag::Evaluate(Asset &)
  {
    return (0.0f);
  }


  //
  // Accept
  //
  // The given asset has been accepted
  //
  void Asset::Request::Tag::Accept(Asset &asset, Bool primary)
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
  Bool Asset::Request::Tag::Offer(Asset &)
  {
    // We want 'em all
    return (TRUE);
  }


  //
  // OutOfAssets
  //
  // The asset manager is informing us there's no more assets
  //
  Bool Asset::Request::Tag::OutOfAssets(Manager &manager)
  {
    Completed(manager);
    return (TRUE);
  }

}
