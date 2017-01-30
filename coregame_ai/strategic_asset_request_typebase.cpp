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
#include "strategic_base.h"
#include "strategic_object.h"
#include "promote.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Asset::Request::TypeBase
  //


  //
  // Constructor
  //
  Asset::Request::TypeBase::TypeBase(Script *script, U32 handle, const Config::RecruitType *config, Base *base, Bool acceptInsufficient)
  : Asset::Request(script, handle),
    config(config),
    base(base),
    acceptInsufficient(acceptInsufficient)
  {
  }

  
  //
  // Constructor
  //
  Asset::Request::TypeBase::TypeBase()
  {
  }


  //
  // Destructor
  //
  Asset::Request::TypeBase::~TypeBase()
  {
    // Clear the amounts tree
    amounts.DisposeAll();
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Asset::Request::TypeBase::SaveState(FScope *scope)
  {
    // Add the name of this type as an argument
    scope->AddArgString(GetName());

    StdSave::TypeString(scope, "Config", config->GetName().str);

    if (base.Alive())
    {
      StdSave::TypeString(scope, "Base", base->GetName());
    }

    StdSave::TypeU32(scope, "AcceptInsufficient", acceptInsufficient);

    // Save parent data
    Request::SaveState(scope->AddFunction("Parent"));
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Asset::Request::TypeBase::LoadState(FScope *scope, void *context)
  {
    // The context holds a pointer to the asset manager
    Manager *manager = reinterpret_cast<Manager *>(context);

    ASSERT(manager)

    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x2FDBDEBB: // "Config"
        {
          // Load the name
          GameIdent name = StdLoad::TypeString(sScope);

          // Find the config
          if ((config = Config::FindRecruitType(name.crc)) == NULL)
          {
            ERR_FATAL(("Unable to find RecruitType config [%s]", name.str));
          }
          break;
        }

        case 0x4BC2F208: // "Base"
          base = manager->GetObject().GetBaseManager().FindBase(StdLoad::TypeString(sScope));
          break;

        case 0x6A5ADB35: // "AcceptInsufficient"
          acceptInsufficient = StdLoad::TypeU32(sScope);
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
  void Asset::Request::TypeBase::Reset()
  {
    Request::Reset();

    // Clear the amounts tree
    amounts.DisposeAll();
  }


  //
  // Submit asset requests directly to the manager
  //
  Bool Asset::Request::TypeBase::DirectEvaluation(Manager &manager)
  {
    // Iterate the units in the base an evaluate them
    if (base.Alive())
    {
      for (UnitObjList::Iterator u(&base->GetUnits()); *u; u++)
      {
        if ((*u)->Alive())
        {
          UnitObj *unit = **u;
          manager.Evaluation(*this, *unit, config->GetAmounts().Find(unit->GameType()->Id()) ? 1.0f : 0.0f);
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
  F32 Asset::Request::TypeBase::Evaluate(Asset &)
  {
    return (0.0f);
  }


  //
  // Accept
  //
  // The given asset has been accepted
  //
  void Asset::Request::TypeBase::Accept(Asset &asset, Bool primary)
  {
    if (primary)
    {
      // Add the object to the list which are assigned to us
      assigned.Append(&asset);
    }

    // Add this type to the amount tree
    U32 type = asset.GetUnit()->GameType()->Id();
    U32 *amount = amounts.Find(type);
    if (amount)
    {
      (*amount)++;
    }
    else
    {
      amounts.Add(type, new U32(1));
    }
  }


  //
  // Offer
  //
  // The given asset is being offered, do we want it ?
  //
  Bool Asset::Request::TypeBase::Offer(Asset &asset)
  {
    // Get the unit out of the asset
    UnitObj *unit = asset.GetUnit();
    ASSERT(unit)

    // Do we have enough of this type ?
    U32 type = unit->GameType()->Id();

    // We should only be offered this type if we asked for it
    ASSERT(config->GetAmount(type))

    // How many of this type do we currently have ?
    U32 *amount = amounts.Find(type);

    // Is this less that what we need ?
    return ((!amount || *amount < config->GetAmount(type)) ? TRUE : FALSE);
  }


  //
  // OutOfAssets
  //
  // The asset manager is informing us there's no more assets
  //
  Bool Asset::Request::TypeBase::OutOfAssets(Manager &manager)
  {
    // Check to see if we have enough assets to continue
    Bool enough = TRUE;

    for (BinTree<U32>::Iterator i(&config->GetAmounts()); *i; i++)
    {
      U32 *amount = amounts.Find(i.GetKey());

      if (!amount || *amount < (**i))
      {
        enough = FALSE;
        break;
      }
    }

    if (enough || acceptInsufficient)
    {
      // Remove all of the units from the base
      if (base.Alive())
      {
        for (NList<Asset>::Iterator a(&assigned); *a; a++)
        {
          UnitObj *unit = (*a)->GetUnit();

          if (unit)
          {
            base->RemoveUnit(unit);
          }
        }
      }

      Completed(manager);
      return (TRUE);
    }
    else
    {
      Reset();
      Abandoned(manager);
      return (FALSE);
    }
  }

}
