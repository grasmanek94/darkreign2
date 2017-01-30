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
#include "strategic_object.h"
#include "unitobjctrl.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Asset::Request::Type
  //

  //
  // Constructor
  //
  Asset::Request::Type::Type(Script *script, U32 handle, const Config::RecruitType *config, const Point<F32> &location, F32 range, Bool acceptInsufficient)
  : Asset::Request(script, handle),
    config(config),
    location(location),
    range2(range * range),
    acceptInsufficient(acceptInsufficient)
  {
    if (range < 8.0f)
    {
      ERR_CONFIG(("Range %f in a Recruit Type is too small (must be > 8m)", range))
    }
    range2inv = 1.0f / range2;
  }


  //
  // Constructor
  //
  Asset::Request::Type::Type()
  {
  }


  //
  // Destructor
  //
  Asset::Request::Type::~Type()
  {
    // Clear the amounts tree
    amounts.DisposeAll();
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Asset::Request::Type::SaveState(FScope *scope)
  {
    // Add the name of this type as an argument
    scope->AddArgString(GetName());

    StdSave::TypeString(scope, "Config", config->GetName().str);
    StdSave::TypePoint(scope, "Location", location);
    StdSave::TypeF32(scope, "Range2", range2);
    StdSave::TypeU32(scope, "AcceptInsufficient", acceptInsufficient);

    // Save parent data
    Request::SaveState(scope->AddFunction("Parent"));
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Asset::Request::Type::LoadState(FScope *scope, void *context)
  {
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

        case 0x693D5359: // "Location"
          StdLoad::TypePoint(sScope, location);
          break;

        case 0xD0535C7B: // "Range2"
          range2 = StdLoad::TypeF32(sScope);
          range2inv = 1.0f / range2;
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
  // Reset the request
  //
  void Asset::Request::Type::Reset()
  {
    Request::Reset();

    // Clear the amounts tree
    amounts.DisposeAll();
  }


  //
  // Evaluate
  //
  // Evaluate the given asset
  //
  F32 Asset::Request::Type::Evaluate(Asset &asset)
  {
    // Grab the unit from the asset
    UnitObj *unit = asset.GetUnit();
    ASSERT(unit)


    //
    // Type
    //
    // Is this unit in the list of types we want ?
    //
    F32 type = config->GetAmounts().Find(unit->GameType()->Id()) ? 1.0f : 0.0f;


    //
    // Distance
    //
    // 1 if its at the location
    // 0 if its at the maximum range or beyond
    //
    F32 distance;

    F32 offsetX = unit->Origin().x - location.x;
    F32 offsetZ = unit->Origin().z - location.z;
    distance = offsetX * offsetX + offsetZ * offsetZ;

    distance = range2 - distance;

    if (distance > 0.0f)
    {
      distance *= range2inv;
    }
    else
    {
      distance = 0.0f;
    }

    //
    // Combine all values
    //
    F32 val = type * distance;

    return (val);
  }


  //
  // Accept
  //
  // The given asset has been accepted
  //
  void Asset::Request::Type::Accept(Asset &asset, Bool primary)
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
  Bool Asset::Request::Type::Offer(Asset &asset)
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
  Bool Asset::Request::Type::OutOfAssets(Manager &manager)
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
