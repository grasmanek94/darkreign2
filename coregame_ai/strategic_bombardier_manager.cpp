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
#include "strategic_bombardier_manager.h"
#include "strategic_bombardier_ruleset.h"
#include "strategic_config.h"
#include "strategic_object.h"
#include "resolver.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Bombardier::Manager
  //


  //
  // Constructor
  //
  Bombardier::Manager::Manager()
  : bombardiers(&Bombardier::nodeManager),
    ruleSets(&RuleSet::nodeManager),
    offMapPreferences(&OffMapPreference::nodeManager)
  {
  }


  //
  // Destructor
  //
  Bombardier::Manager::~Manager()
  {
    // Delete all of the bombardiers
    bombardiers.DisposeAll();

    // Delete all rulesets
    ruleSets.DisposeAll();

    // Delete all of the preferences
    offMapPreferences.DisposeAll();
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Bombardier::Manager::SaveState(FScope *scope)
  {
    // Save the bombardiers
    for (NList<Bombardier>::Iterator i(&bombardiers); *i; ++i)
    {
      (*i)->SaveState(scope->AddFunction((*i)->GetName()));
    }
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Bombardier::Manager::LoadState(FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x0824D249: // "Bombardier::OffMap"
        {
          Bombardier *bombardier = new Bombardier::OffMap(*this, NULL);
          bombardier->LoadState(sScope);
          bombardiers.Append(bombardier);
          break;
        }

        case 0x3826737C: // "Bombardier::Unit"
        {
          Bombardier *bombardier = new Bombardier::Unit(*this, NULL);
          bombardier->LoadState(sScope);
          bombardiers.Append(bombardier);
          break;
        }
      }
    }
  }


  //
  // Process Bombardier
  //
  void Bombardier::Manager::Process()
  {
    // Process all of the bombardiers
    Bombardier *bombardier;
    NList<Bombardier>::Iterator b(&bombardiers);

    while ((bombardier = b++) != NULL)
    {
      if (bombardier->Process())
      {
        bombardiers.Dispose(bombardier);
      }
    }
  }


  //
  // Add a new bombardier
  //
  Bool Bombardier::Manager::AddBombardier(UnitObj *unit)
  {
    ASSERT(unit)

    if (unit->UnitType()->HasProperty(0x9268FE92)) // "Provide::Offmap"
    {
      // Create a new offmap bombardier which will manage this unit
      bombardiers.Append(new Bombardier::OffMap(*this, unit));
      return (TRUE);
    }
    else if (unit->UnitType()->HasProperty(0xFC7A2C8C) && !strategic->GetTeam()->IsHuman()) // "Ability::Bombardier"
    {
      // Create a new offmap bombardier which will manage this unit
      bombardiers.Append(new Bombardier::Unit(*this, unit));
      return (TRUE);
    }
    else
    {
      return (FALSE);
    }
  }


  //
  // Set preferences
  //
  void Bombardier::Manager::SetPreferences(const GameIdent &ident)
  {
    // Ask config if there's a base
    Config::Generic *config = Config::FindConfig(0x5D5BF0E0, ident); // "Bombardier::Preferences"

    if (config)
    {
      LOG_AI(("Setting Bombardier Preferences to '%s'", ident.str))

      // Get rid of any existing preferences
      offMapPreferences.DisposeAll();

      // Clear the delay
      offMapDelayMin = 0.0f;
      offMapDelayMax = 0.0f;

      // Read in the new configuration
      FScope *fScope = config->GetFScope();

      while (FScope *sScope = fScope->NextFunction())
      {
        switch (sScope->NameCrc())
        {
          case 0xBF87218C: // "Offmap"
          {
            while (FScope *iScope = sScope->NextFunction())
            {
              switch (iScope->NameCrc())
              {
                case 0x1D9D48EC: // "Type"
                {
                  UnitObjTypePtr reaper;

                  // Load the type
                  StdLoad::TypeReaperObjType(iScope, reaper);

                  // Resolve the type
                  Resolver::Type<UnitObjType>(reaper, TRUE);

                  // Add the new preference
                  offMapPreferences.Add(reaper->GetNameCrc(), new OffMapPreference(*reaper, StdLoad::TypeF32(iScope)));
                  break;
                }

                case 0x33CED45D: // "Delay"
                {
                  offMapDelayMin = StdLoad::TypeF32(iScope, Range<F32>(0.0f, 1000.0f));
                  offMapDelayMax = StdLoad::TypeF32(iScope, offMapDelayMin, Range<F32>(0.0f, 1000.0f));
                  break;
                }
              }
            }
            break;
          }
        }
      }

      FSCOPE_CHECK(fScope)
    }
    else
    {
      ERR_CONFIG(("Could not find bombardier preferences configuration '%s'", ident.str))
    }
  }


  //
  // Get the delay
  //
  F32 Bombardier::Manager::GetOffMapDelay()
  {
    return (offMapDelayMin + (offMapDelayMin - offMapDelayMax) * GetObject().GetRandomGenerator()->Float());
  }


  //
  // Get a ruleset from a type crc
  //
  Bombardier::RuleSet & Bombardier::Manager::GetRuleSet(const GameIdent &typeName)
  {
    // Do we have this ruleset on hand ?
    RuleSet *ruleSet = ruleSets.Find(typeName.crc);

    if (!ruleSet)
    {
      // Try to build a new one from configs
      Config::Generic *config = Config::FindConfig(0x8B6AB1B1, typeName); // "Bombardier::RuleSet"

      if (config)
      {
        ruleSet = new RuleSet(config->GetFScope());
        ruleSets.Add(typeName.crc, ruleSet);
      }
      else
      {
        ERR_CONFIG(("Could not find Bombardier::Ruleset for '%s'", typeName.str))
      }
    }

    return (*ruleSet);
  }


}
