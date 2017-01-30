/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic AI
//
// 31-AUG-1998
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_private.h"
#include "strategic_object.h"
#include "strategic_config.h"
#include "strategic_asset_manager.h"
#include "strategic_resource_decomposition.h"
#include "strategic_water_decomposition.h"
#include "strategic_script.h"

#include "varsys.h"
#include "console.h"
#include "unitobj.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{
  LOGDEFLOCAL("Strategic");


  /////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // Initialization Flag
  static Bool initialized = FALSE;

  // Death Tracker
  static DTrack *dTracker;

  // Strategic Objects
  static List<Object> objects;


  //
  // Init
  //
  // Initialize Strategic AI
  //
  void Init()
  {
    ASSERT(!initialized)

    // Allocate a death tracker
    dTracker = new DTrack("AI", 1024);

    // Initialize Script
    Script::Init();

    // Initialize Config
    Config::Init();

    // Initialize Asset Manager
    Asset::Manager::Init();

    // Initialize Resource Decomposition
    Resource::Decomposition::Init();

    // Initialize Water Decomposition
    Water::Decomposition::Init();

    // Set the initialized flag
    initialized = TRUE;
  }


  //
  // Done
  //
  // Shutdown Strategic AI
  //
  void Done()
  {
    ASSERT(initialized)

    // Initialize Water Decomposition
    Water::Decomposition::Done();

    // Shutdown Resource Decomposition
    Resource::Decomposition::Done();

    // Shutdown Asset Manager
    Asset::Manager::Done();

    // Shutdown Config
    Config::Done();

    // Shutdown Script
    Script::Done();

    // Delete Strategic Objects
    objects.DisposeAll();

    // Delete the death tracker
    delete dTracker;

    // Delete command scope
    VarSys::DeleteItem("ai.strategic");

    // Clear the initialized flag
    initialized = FALSE;
  }


  //
  // Create
  //
  Object * Create(U32 personality, Bool route)
  {
    ASSERT(initialized)

    // Find the named personality mod
    Mods::Mod *mod = NULL;
    if (personality)
    {
      mod = Mods::GetMod(Mods::Types::Personality, personality);
    }

    // Create the new object
    Object *object = new Object(mod);

    // Is it routed ?
    if (route)
    {
      object->EnableRouting();
    }

    return (object);
  }


  //
  // Save
  //
  void Save(FScope *scope)
  {
    // Save the strategic objects for each team
    FScope *sScope = scope->AddFunction("Objects");

    for (U32 i = 0; i < Game::MAX_TEAMS; i++)
    {
      if (Team *team = Team::Id2Team(i))
      {
        if (Object *object = team->GetStrategicObject())
        {
          object->SaveState(StdSave::TypeString(sScope, "Object", team->GetName()));
        }
      }
    }

    // Save the script system data
    Script::Save(scope->AddFunction("Script"));
  }


  //
  // Load
  //
  void Load(FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xEC92967E: // "Objects"
        {
          FScope *ssScope;

          while ((ssScope = sScope->NextFunction()) != NULL)
          {
            switch (ssScope->NameCrc())
            {
              case 0xA75FFAEB: // "Object"
              {
                if (Team *team = Team::Name2Team(StdLoad::TypeString(ssScope)))
                {
                  if (Object *object = team->GetStrategicObject())
                  {
                    object->LoadState(ssScope);
                  }                 
                }
                break;
              }
            }
          }
          break;
        }

        case 0x8810AE3C: // "Script"
          Script::Load(sScope);
          break;
      }
    }
  }


  //
  // Reset
  //
  // Reset all of the strategic objects
  //
  void Reset()
  {
    ASSERT(initialized)

    // Perform operations which aren't specific to an object
    Resource::Decomposition::Reset();
    Water::Decomposition::Reset();

    // Reset each of the strategic objects
    for (List<Object>::Iterator i(&objects); *i; i++)
    {
      (*i)->Reset();
    }
  }


  //
  // Process Strategic AI
  //
  void Process()
  {
    ASSERT(initialized)
    ASSERT(objects.GetCount() <= Game::MAX_TEAMS)
    ASSERT(GameTime::SimCycle() > 0)

    // Schedule each AI to process on the 8th sim cycle
    U32 ai = (GameTime::SimCycle() - 1) & (Game::MAX_TEAMS - 1);

    if (ai < objects.GetCount())
    {
      objects[ai]->Process();
    }
  }


  //
  // RegisterObject
  //
  void RegisterObject(Object *object)
  {
    ASSERT(initialized)
    objects.Append(object);
  }


  //
  // RegisterConstruction
  //
  // Register the creation of an AI item
  //
  void RegisterConstruction(DTrack::Info &info)
  {
    ASSERT(initialized)

    // Call the death tracker object
    dTracker->RegisterConstruction(info);
  }


  //
  // RegisterDestruction
  //
  // Register the destruction of the AI item 'info'
  //
  void RegisterDestruction(DTrack::Info &info)
  {
    ASSERT(initialized)

    // Call the death tracker object
    dTracker->RegisterDestruction(info);
  }

}