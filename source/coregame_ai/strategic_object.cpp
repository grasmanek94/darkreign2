/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic AI
//
// 31-AUG-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "perfstats.h"
#include "strategic_object.h"
#include "strategic_private.h"
#include "team.h"
#include "unitobj.h"
#include "transportobj.h"
#include "console.h"
#include "gametime.h"
#include "promote.h"
#include "resolver.h"
#include "babel.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

#ifdef DEVELOPMENT
  #define CON_TEAM(x) CONSOLE(GetTeam()->GetConsoleId(0x622EF512), x ) // "Strategic"
#else
  #define CON_TEAM(x)
#endif


const U32 ITERATIONS_ASSET_MANAGER = 20;
const U32 ITERATIONS_BASE_MANAGER = 50;
const U32 ITERATIONS_BOMBARDIER_MANAGER = 100;
const U32 ITERATIONS_INTEL_MANAGER = 10;
const U32 ITERATIONS_PLACEMENT_MANAGER = 1;
const U32 ITERATIONS_RESOURCE_MANAGER = 5;
const U32 ITERATIONS_RULE_MANAGER = 100;
const U32 ITERATIONS_SCRIPT_MANAGER = 10;
const U32 ITERATIONS_WATER_MANAGER = 1;


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Object - Instance class for above type
  //


  //
  // Object::Object
  //
  // Constructor
  //
  Object::Object(Mods::Mod *mod) 
  : Player
    (
      mod ? mod->GetName() : "AI", 
      AI, 
      mod ? TRANSLATE((mod->GetDescription().str)) : NULL, 
      FALSE
    ),
    mod(mod),
    notifications(&Notification::node),
    shutdown(FALSE)
  {
    // Disable routing by default
    DisableRouting();

    // Inform Strategic of the creation of this object
    RegisterObject(this);    

    // Initialize the asset manager
    assetManager.Setup(this);

    // Initialize the base manager
    baseManager.Setup(this);

    // Initialize the bombardier manager
    bombardierManager.Setup(this);

    // Initialize the intel manager
    intelManager.Setup(this);

    // Initialize the placement manager
    placementManager.Setup(this);

    // Initialize the resource manager
    resourceManager.Setup(this);

    // Initialize the water manager
    waterManager.Setup(this);

    // Initialize the rule manager
    ruleManager.Setup(this);

    // Initialize the script manager
    scriptManager.Setup(this);

    // Initialize the transport manager
    transportManager.Setup(this);
  }


  //
  // Object::Object
  //
  // Destructor
  //
  Object::~Object()
  {
    // Clear any remaining notifications
    notifications.DisposeAll();

    // Get the transport manager to clean up
    transportManager.CleanUp();

    // Set the shutdown flag
    shutdown = TRUE;
  }


  //
  // SaveState
  //
  // Save object state information
  //
  void Object::SaveState(FScope *scope)
  {
    // Save the managers (order is significant)
    scriptManager.SaveState(scope->AddFunction("ScriptManager"));
    transportManager.SaveState(scope->AddFunction("TransportManager"));
    assetManager.SaveState(scope->AddFunction("AssetManager"));
    bombardierManager.SaveState(scope->AddFunction("BombardierManager"));
    intelManager.SaveState(scope->AddFunction("IntelManager"));
    placementManager.SaveState(scope->AddFunction("PlacementManager"));
    ruleManager.SaveState(scope->AddFunction("RuleManager"));
    resourceManager.SaveState(scope->AddFunction("ResourceManager"));
    waterManager.SaveState(scope->AddFunction("WaterManager"));
    baseManager.SaveState(scope->AddFunction("BaseManager"));

    // Create scope for notifications
    FScope *sScope = scope->AddFunction("Notifications");

    // Save each current notification
    for (NList<Notification>::Iterator i(&notifications); *i; ++i)
    {
      // Get this notification
      Notification &n = **i;

      // Save it out
      FScope *ssScope = sScope->AddFunction("Add");
      StdSave::TypeReaper(ssScope, "From", n.from);
      StdSave::TypeU32(ssScope, "Message", n.message);
      StdSave::TypeU32(ssScope, "Param1", n.param1);
      StdSave::TypeU32(ssScope, "Param2", n.param2);
    }
  }


  //
  // LoadState
  //
  // Load object state information
  //
  void Object::LoadState(FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x4F12C753: // "ScriptManager"
          scriptManager.LoadState(sScope);
          break;

        case 0x393E9106: // "TransportManager"
          transportManager.LoadState(sScope);
          break;

        case 0x2C287ED7: // "AssetManager"
          assetManager.LoadState(sScope);
          break;

        case 0xB0F126DB: // "BombardierManager"
          bombardierManager.LoadState(sScope);
          break;

        case 0x87824959: // "IntelManager"
          intelManager.LoadState(sScope);
          break;

        case 0x021DD97F: // "PlacementManager"
          placementManager.LoadState(sScope);
          break;

        case 0x9022D919: // "RuleManager"
          ruleManager.LoadState(sScope);
          break;

        case 0x4CEB40FE: // "ResourceManager"
          resourceManager.LoadState(sScope);
          break;

        case 0x45DA4572: // "WaterManager"
          waterManager.LoadState(sScope);
          break;

        case 0x57E13FCA: // "BaseManager"
          baseManager.LoadState(sScope);
          break;

        case 0x23DF48AA: // "Notifications"
        {
          FScope *ssScope;

          while ((ssScope = sScope->NextFunction()) != NULL)
          {
            switch (ssScope->NameCrc())
            {
              case 0x9F1D54D0: // "Add"
              {
                // Load the game object reaper
                GameObjPtr from;
                StdLoad::TypeReaper(ssScope, "From", from);
                Resolver::Object<GameObj, GameObjType>(from);

                // Add the notification
                notifications.Append
                (
                  new Notification
                  (
                    from.GetPointer(), 
                    StdLoad::TypeU32(ssScope, "Message"), 
                    StdLoad::TypeU32(ssScope, "Param1"), 
                    StdLoad::TypeU32(ssScope, "Param2")
                  )
                );
                break;
              }
            }
          }
          break;
        }
      }
    }
  }


  //
  // Object::Process
  //
  // Perform processing for this strategic object
  //
  void Object::Process()
  {
    // Transfer the notifications to a processing list to prevent recursion
    NList<Notification> process(&Notification::node);
    notifications.Transfer(process);

    // Process Notifications
    Notification *notification;
    NList<Notification>::Iterator n(&process);

    while ((notification = n++) != NULL)
    {
      // Send it to the base manager
      baseManager.Notify(*notification);

      // Send it to the script manager
      scriptManager.Notify(*notification);

      // Send it to the transport manager
      transportManager.Notify(*notification);

      // Dispose of the notification
      process.Dispose(notification);
    }

    U32 i;

    PERF_S("Asset");

    // Process Asset Manager
    for (i = 0; i < ITERATIONS_ASSET_MANAGER; i++)
    {
      assetManager.Process();
    }

    PERF_E("Asset");

    PERF_S("Base");

    // Process Base Manager
    for (i = 0; i < ITERATIONS_BASE_MANAGER; i++)
    {
      baseManager.Process();
    }

    PERF_E("Base");

    PERF_S("Bombardier");

    // Process the Bombardier manager
    for (i = 0; i < ITERATIONS_BOMBARDIER_MANAGER; i++)
    {
      bombardierManager.Process();
    }

    PERF_E("Bombardier");

    PERF_S("Intel");

    // Process the Intel manager
    for (i = 0; i < ITERATIONS_INTEL_MANAGER; i++)
    {
      intelManager.Process();
    }

    PERF_E("Intel");

    PERF_S("Placement");

    // Process the Placement manager
    for (i = 0; i < ITERATIONS_PLACEMENT_MANAGER; i++)
    {
      placementManager.Process();
    }

    PERF_E("Placement");

    PERF_S("Resource");

    // Process the Resource manager
    for (i = 0; i < ITERATIONS_RESOURCE_MANAGER; i++)
    {
      resourceManager.Process();
    }

    PERF_E("Resource");

    PERF_S("Water");

    // Process the Resource manager
    for (i = 0; i < ITERATIONS_WATER_MANAGER; i++)
    {
      waterManager.Process();
    }

    PERF_E("Water");

    PERF_S("Rule");

    // Process the Rule manager
    for (i = 0; i < ITERATIONS_RULE_MANAGER; i++)
    {
      ruleManager.Process();
    }

    PERF_E("Rule");

    PERF_S("Script");

    // Process Script Manager
    for (i = 0; i < ITERATIONS_SCRIPT_MANAGER; i++)
    {
      scriptManager.Process();
    }

    PERF_E("Script");
  }


  //
  // Object::Reset
  //
  // Reset the object
  //
  void Object::Reset()
  {
    ASSERT(team)

    // Ensure that the personality is initalized
    if (mod)
    {
      mod->Setup(team);
    }

    // Reset any managers that require reset
    resourceManager.Reset();
    waterManager.Reset();

    // Initialize the resource system
    for (NList<UnitObj>::Iterator i(&team->GetUnitObjects()); *i; i++)
    {
      GiveUnit(*i);
    }
  }


  //
  // Object::SetTeam
  //
  // Set the team with which this player is associated
  //
  void Object::SetTeam(Team *t)
  { 
    Player::SetTeam(t);
    t->SetStrategicObject(this);
  }


  //
  // Object::ClearTeam
  //
  // Clears the team with which this player is associated
  //
  void Object::ClearTeam() 
  { 
    team->SetStrategicObject(NULL);
    Player::ClearTeam();
  }


  //
  // Object::EnableRouting
  // 
  // Enable routing for this player (so it can participate in a network game)
  //
  void Object::EnableRouting()
  {
    // Set routing flag
    route = TRUE;

    // Make sure we're using NON SYNCHRONIZED random numbers
    randomGenerator = &Random::nonSync;
  }


  //
  // Object::DisableRouting 
  // 
  // Disable routing for this player
  //
  void Object::DisableRouting()
  {
    // Clear routing flag
    route = FALSE;

    // Make sure we're using SYNCRONIZED random numbers
    randomGenerator = &Random::sync;
  }


  //
  // Object::GiveUnit
  //
  // Give a single unit to the AI
  //
  void Object::GiveUnit(UnitObj *unit)
  {
    // Is this unit a transport ?
    TransportObj *transport = Promote::Object<TransportObjType, TransportObj>(unit);
    if (transport)
    {
      // Give it to the transport manager
      transportManager.AddTransport(*transport);
    }

    else

    // Does the bombardier manager want it
    if (bombardierManager.AddBombardier(unit))
    {
    }

    else
    // Give it to the asset manager
    {
      // Give it to the asset manager
      assetManager.SubmitAsset(*new Asset(unit));
    }
  }


  //
  // Object::GiveUnits
  //
  // Give units to the AI object
  //
  void Object::GiveUnits(const UnitObjList &units)
  {
    for (UnitObjList::Iterator u(&units); *u; u++)
    {
      if ((*u)->Alive())
      {
        GiveUnit(**u);
      }
    }
  }


  //
  // Object::TakeUnit
  //
  // Take a single unit from the AI
  //
  void Object::TakeUnit(UnitObj *unit)
  {
    ASSERT(unit)

    // Is this unit a transport ?
    TransportObj *transport = Promote::Object<TransportObjType, TransportObj>(unit);

    if (transport)
    {
      transportManager.RemoveTransport(*transport);
    }
    else
    {
      assetManager.RemoveUnit(*unit);
    }
  }


  //
  // Object::Notify
  //
  Bool Object::Notify(GameObj *from, U32 message, U32 param1, U32 param2)
  {
    // Add to the notification list
    notifications.Append(new Notification(from, message, param1, param2));
    return (TRUE);
  }

}
