/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Intel
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_intel_manager.h"
#include "strategic_config.h"
#include "resolver.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Intel::Manager
  //


  //
  // Constructor
  //
  Intel::Manager::Manager()
  : intels(&Intel::nodeManagerTree),
    idle(&Intel::nodeManagerList),
    process(&Intel::nodeManagerList)
  {
  }


  //
  // Destructor
  //
  Intel::Manager::~Manager()
  {
    // Delete all of the intels
    idle.UnlinkAll();
    process.UnlinkAll();
    intels.DisposeAll();
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Intel::Manager::SaveState(FScope *scope)
  {
    // Save the intels
    for (NBinTree<Intel>::Iterator i(&intels); *i; ++i)
    {
      FScope *sScope = scope->AddFunction("Intel");
      sScope->AddArgInteger(i.GetKey());
      sScope->AddArgString((*i)->configName.str);
    }
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Intel::Manager::LoadState(FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x8FFC20CB: // "Intel"
        {
          // Load the key
          U32 key = StdLoad::TypeU32(sScope);

          // Load the config name
          GameIdent configName = StdLoad::TypeString(sScope);

          // Ask config if there's a intel
          Config::Generic *config = Config::FindConfig(0x8FFC20CB, configName); // "Intel"

          FScope *fScope = config->GetFScope();

          if (config)
          {
            if (intels.Find(key))
            {
              ERR_CONFIG(("Duplicate intel key %08X", intels.Find(key)))
            }

            Intel *intel = Intel::Create(*this, configName, fScope);

            intels.Add(key, intel);
            idle.Append(intel);
          }
          else
          {
            ERR_FATAL(("Could not find Intel config '%s' during load", configName.str))
          }
          break;
        }
      }
    }
  }


  //
  // Process Intel
  //
  void Intel::Manager::Process()
  {
    // Check to see if there's any intels due for processing
    NList<Intel>::Iterator i(&idle);
    while (Intel *intel = i++)
    {
      if (intel->IsReady())
      {
        idle.Unlink(intel);
        process.Append(intel);
      }
    }

    // Process all of the intels
    NList<Intel>::Iterator p(&process);
    while (Intel *intel = p++)
    {
      if (!intel->Process())
      {
        process.Unlink(intel);
        idle.Append(intel);
        intel->SetTime();
      }
    }
  }


  //
  // Add an intel
  //
  Intel & Intel::Manager::AddIntel(const GameIdent &intelName, const GameIdent &configName)
  {
    // Ask config if there's a intel
    Config::Generic *config = Config::FindConfig(0x8FFC20CB, configName); // "Intel"

    if (config)
    {
      if (intels.Find(intelName.crc))
      {
        ERR_CONFIG(("Intel '%s' already exists", intelName.str))
      }

      FScope *fScope = config->GetFScope();

      // Create new intel
      Intel *intel = Intel::Create(*this, configName, fScope);

      FSCOPE_CHECK(fScope)

      // Add Intel
      intels.Add(intelName.crc, intel);
      idle.Append(intel);

      // Return the intel
      return (*intel);
    }
    else
    {
      ERR_CONFIG(("Could not find intel configuration '%s'", configName.str))
    }
  }


  //
  // Remove an intel
  //
  void Intel::Manager::RemoveIntel(const GameIdent &intelName)
  {
    Intel *intel = intels.Find(intelName.crc);

    if (intel)
    {
      if (intel->idle)
      {
        idle.Unlink(intel);
      }
      else
      {
        process.Unlink(intel);
      }
      intels.Dispose(intel);
    }

  }

}
