/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Object
//
// 31-AUG-1998
//


#ifndef __STRATEGIC_OBJECT_H
#define __STRATEGIC_OBJECT_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "ai.h"
#include "player.h"
#include "strategic.h"
#include "random.h"
#include "squadobjdec.h"
#include "strategic_notification.h"
#include "strategic_asset_manager.h"
#include "strategic_base_manager.h"
#include "strategic_bombardier_manager.h"
#include "strategic_intel_manager.h"
#include "strategic_placement_manager.h"
#include "strategic_resource_manager.h"
#include "strategic_rule_manager.h"
#include "strategic_script_manager.h"
#include "strategic_transport_manager.h"
#include "strategic_water_manager.h"
#include "mods.h"


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
  class Object : public Player
  {
  private:

    // GameIdent
    Mods::Mod *mod;

    // Asset manager
    Asset::Manager assetManager;

    // Base manager
    Base::Manager baseManager;

    // Bombardier manager
    Bombardier::Manager bombardierManager;

    // Intel manager
    Intel::Manager intelManager;

    // Placement manager
    Placement::Manager placementManager;

    // Resource manager
    Resource::Manager resourceManager;
    
    // Rule manager
    Rule::Manager ruleManager;

    // Script manager
    Script::Manager scriptManager;

    // Transport manager
    Transport::Manager transportManager;

    // Water manager
    Water::Manager waterManager;

    // Random number generator
    Random::Generator *randomGenerator;

    // Shutdown flag
    Bool shutdown;

    // Notifications
    NList<Notification> notifications;

  public:

    // Constructor and destructor
    Object(Mods::Mod *mod);
    ~Object();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope);

    // Perform processing for this strategic object
    void Process();

    // Reset the object
    void Reset();

    // SetTeam: Set the team with which this player is associated
    void SetTeam(Team *t);

    // ClearTeam: Clears the team with which this player is associated
    void ClearTeam();

    // EnableRouting: Enable routing for this player (so it can participate in a network game)
    void EnableRouting();

    // DisableRouting: Disable routing for this player
    void DisableRouting();

    // SubmitRequest: Submit an asset request
    void SubmitRequest(Asset::Request *request);

    // GiveUnit: Give a single unit to the AI object
    void GiveUnit(UnitObj *unit);

    // GiveUnits: Give units to the AI object
    void GiveUnits(const UnitObjList &units);

    // TakeUnit: Take a single unit from the AI
    void TakeUnit(UnitObj *unit);

    // Notification
    Bool Notify(GameObj *from, U32 message, U32 param1 = 0, U32 param2 = 0);

  public:

    // Get the random number generator being used by this AI
    Random::Generator * GetRandomGenerator()
    {
      ASSERT(randomGenerator)
      return (randomGenerator);
    }

    // Asset manager
    Asset::Manager & GetAssetManager()
    {
      return (assetManager);
    }

    // Base manager
    Base::Manager & GetBaseManager()
    {
      return (baseManager);
    }

    // Bombardier manager
    Bombardier::Manager & GetBombardierManager()
    {
      return (bombardierManager);
    }

    // Intel manager
    Intel::Manager & GetIntelManager()
    {
      return (intelManager);
    }

    // Placement manager
    Placement::Manager & GetPlacementManager()
    {
      return (placementManager);
    }

    // Resource manager
    Resource::Manager & GetResourceManager()
    {
      return (resourceManager);
    }

    // Rule manager
    Rule::Manager & GetRuleManager()
    {
      return (ruleManager);
    }

    // Script manager
    Script::Manager & GetScriptManager()
    {
      return (scriptManager);
    }

    // Transport manager
    Transport::Manager & GetTransportManager()
    {
      return (transportManager);
    }

    // Water manager
    Water::Manager & GetWaterManager()
    {
      return (waterManager);
    }

    // GetTeam : Gets the team with which this player is associated
    Team * GetTeam() const
    { 
      ASSERT(team)
      return (team); 
    }

    // GetMod : Get the personality mod
    Mods::Mod * GetMod() const
    {
      return (mod);
    }

    // IsShutdown : Are we shutting down ?
    Bool IsShutdown() const
    {
      return (shutdown);
    }

  };

}

#endif