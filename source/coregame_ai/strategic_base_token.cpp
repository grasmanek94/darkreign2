/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Base
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_base_token.h"
#include "strategic_base_orderer.h"
#include "strategic_base_manager.h"
#include "strategic_base_state.h"
#include "strategic_object.h"
#include "strategic_placement.h"
#include "orders_game.h"
#include "resolver.h"
#include "sync.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Base::Token
  //


  //
  // Names of the states
  //
  const char * Base::Token::stateNames[] =
  {
    "ONORDER",
    "WAITING",
    "SEARCHING",
    "FACILITYCONSTRUCTING",
    "LOCATING",
    "CONSTRUCTING",
    "UPGRADING",
    "NONE"
  };


  //
  // Constructor
  //
  Base::Token::Token(Base &baseIn, UnitObjType &typeIn, Orderer &orderer, Resource *resource, Water *water, Script *script)
  : base(&baseIn),
    type(&typeIn),
    orderer(&orderer),
    placement(NULL),
    priority(orderer.GetPriority()),
    state(ONORDER),
    resource(resource),
    water(water),
    script(script),
    dir(WorldCtrl::NORTH)
  {
    // Register with the manager, getting an id
    id = base->GetManager().TokenConstruction(*this);

    // Register with the base
    base->TokenConstruction(*this);

    ASSERT(type->GetConstructorType() || type->GetUpgradeFor())
    if (type->GetConstructorType() && !type->GetConstructorType()->GetIsFacility())
    {
      // Tell the orderer to russle up a placement
      placement = &base->GetObject().GetPlacementManager().GetPlacement(orderer.GetPlacement());
    }

    // Tell the base manager about this type
    base->GetManager().AddTypeOnOrder(*type);
  }
    
  
  //
  // Loading constructor
  //
  Base::Token::Token(FScope *scope, Manager &manager)
  : base(NULL),
    type(NULL),
    orderer(NULL),
    placement(NULL),
    priority(0),
    state(ONORDER),
    resource(NULL),
    script(NULL),
    id(0),
    dir(WorldCtrl::NORTH)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x795DF32D: // "Id"
          id = StdLoad::TypeU32(sScope);
          break;

        case 0x1D9D48EC: // "Type"
          type = GameObjCtrl::FindType<UnitObjType>(StdLoad::TypeString(sScope));
          break;

        case 0x4BC2F208: // "Base"
          base = manager.FindBase(StdLoad::TypeString(sScope));
          break;

        case 0x693D5359: // "Location"
          StdLoad::TypeVector(sScope, location);
          break;

        case 0x04BC5B80: // "Direction"
          dir = WorldCtrl::CompassDir(StdLoad::TypeU32(sScope));
          break;

        case 0xB73D3720: // "Constructor"
          StdLoad::TypeReaper(sScope, constructor);
          Resolver::Object<UnitObj, UnitObjType>(constructor);
          break;

        case 0x2E3313B9: // "ConstructorId"
          constructorId = StdLoad::TypeU32(sScope);
          break;

        case 0x5D7C647F: // "Placement"
          placement = &base->GetObject().GetPlacementManager().GetPlacement(StdLoad::TypeString(sScope));
          break;
          
        case 0x4CD1BE27: // "Resource"
          if ((resource = manager.GetObject().GetResourceManager().FindResource(StdLoad::TypeU32(sScope))) == NULL)
          {
            ERR_FATAL(("Resource did not resolve"))
          }
          break;

        case 0x6ADE7843: // "Water"
          if ((water = manager.GetObject().GetWaterManager().FindWater(StdLoad::TypeU32(sScope))) == NULL)
          {
            ERR_FATAL(("Water did not resolve"))
          }
          break;

        case 0x8810AE3C: // "Script"
          if ((script = manager.GetObject().GetScriptManager().FindScript(StdLoad::TypeU32(sScope))) == NULL)
          {
            ERR_FATAL(("Script did not resolve"))
          }
          break;

        case 0x96880780: // "State"
          state = State(StdLoad::TypeU32(sScope));
          break;

        case 0xFFF34C6F: // "Priority"
          priority = StdLoad::TypeU32(sScope);
          break;
      }
    }

    ASSERT(type)
    ASSERT(base)
    ASSERT(id)

    // Register with the manager
    manager.TokenConstruction(*this, id);

    // Register with the base
    base->TokenConstruction(*this);
  }


  //
  // Destructor
  //
  Base::Token::~Token()
  {
//    LOG_AI(("Token '%s' terminated", type->GetName()))

    // Do we have an orderer ?
    if (orderer && nodeList.InUse())
    {
      orderer->UsedToken(this);
    }
    // Are we an orphan ?
    else if (nodeList.InUse())
    {
      base->RemoveOrphanedToken(this);
    }

    // If there's a placement ... (and we're not shutting down)
    if (placement && !base->GetObject().IsShutdown())
    {
      // Make sure that this token is removed from the placement
      placement->RemoveToken(*this);
    }

    // If we're waiting for a notification, remove us from the bse manager
    if (nodeManager.InUse())
    {
      base->GetManager().UnregisterNotificationHandler(*this);
    }

    // Go to the none state
    SetState(NONE);

    // Register destruction with manager and base
    base->GetManager().TokenDestruction(*this);
    base->TokenDestruction(*this);
  }


  //
  // Save state data
  //
  void Base::Token::SaveState(FScope *scope)
  {
    StdSave::TypeU32(scope, "Id", id);
    StdSave::TypeString(scope, "Type", type->GetName());
    StdSave::TypeString(scope, "Base", base->GetName());

    StdSave::TypeVector(scope, "Location", location);
    StdSave::TypeU32(scope, "Direction", U32(dir));
    StdSave::TypeReaper(scope, "Constructor", constructor);
    StdSave::TypeU32(scope, "ConstructorId", constructorId);

    if (resource.Alive())
    {
      StdSave::TypeU32(scope, "Resource", resource->GetId());
    }

    if (water.Alive())
    {
      StdSave::TypeU32(scope, "Water", water->GetId());
    }

    if (placement)
    {
      StdSave::TypeString(scope, "Placement", placement->GetName().str);
    }

    if (script.Alive())
    {
      StdSave::TypeU32(scope, "Script", script->GetNameCrc());
    }

    StdSave::TypeU32(scope, "State", U32(state));
    StdSave::TypeU32(scope, "Priority", priority);

  }

 
  //
  // Process the token
  //
  void Base::Token::CheckConstructor(const UnitObjList &buildings, const UnitObjList &constructors)
  {
    ASSERT(state == WAITING)

    // Do we already have a constructor, if so, use it
    if (constructor.Alive())
    {
      SetConstructor(constructor);
      return;
    }
    else if (type->GetUpgradeFor())
    {
      // Iterate the buildings and see if any of them want to be upgraded to this type
      for (UnitObjList::Iterator i(&buildings); *i; i++)
      {
        if ((*i)->Alive())
        {
          UnitObjType *upgrader = (**i)->GetNextUpgrade();
          if (upgrader && upgrader->Id() == type->Id())
          {
            UnitObj *upgradee = (**i);

            // Is this upgrader an active constructor ?
            if (base->FindActiveConstructor(upgradee))
            {
              // We can't use it at the moment
              continue;
            }

            // Can this unit upgrade now ?
            if (!upgradee->CanUpgradeNow())
            {
              // We can't upgrade at the moment
              continue;
            }

            // Save the upgradee as the constructor
            constructor = upgradee;
            constructorId = constructor.DirectId();

            // We're now upgrading
            SetState(UPGRADING);

            // Is this upgrader a constructor ?
            UnitObjListNode *node = base->FindInactiveConstructor(upgradee);
            if (node)
            {
              base->ActivateConstructor(node);
            }

            // Tell the manager that we're waiting for notification from the constructor
            base->GetManager().RegisterNotificationHandler(upgradee->Id(), *this);

            // Tell the upgrader to upgrade
            Orders::Game::ClearSelected::Generate(base->GetObject());
            Orders::Game::AddSelected::Generate(base->GetObject(), upgradee);
            Orders::Game::Upgrade::Generate(base->GetObject());
            return;
          }
        }
      }

      // Hmm, no constructor could be found, do we still have 
      // the prereqs for this, if not, put it back on order 
      // and get it out of our face
      if (!GetType().GetPrereqs().Have(GetBase().GetObject().GetTeam()))
      {
        Returned();
      }

      // LOG_AI(("Token '%s' no upgrader", type->GetName()))
    }
    else
    {
      if (type->GetConstructorType()->GetIsFacility())
      {
        // Iterate the idle constructors and see if there's a suitable one available
        for (UnitObjList::Iterator i(&constructors); *i; i++)
        {
          if ((*i)->Alive())
          {
            // Can this token be constructed by this constructor ???
            if ((**i)->CanConstruct(type))
            {
              UnitObjListNode *node = *i;
              UnitObj *unit = *node;

              // Activate the constructor
              base->ActivateConstructor(node);

              // Assign this constructor
              SetConstructor(unit);
              return;
            }
          }
        }
      }
      else
      {
        // Find the closest constructor which is on the 
        // same connected region as the build location
        UnitObjListNode *winner = NULL;
        F32 winningDistance = F32_MAX;
   
        // Iterate the idle constructors and see if there's a suitable one available
        for (UnitObjList::Iterator i(&constructors); *i; i++)
        {
          if ((*i)->Alive())
          {
            // Can this token be constructed by this constructor ???
            if ((**i)->CanConstruct(type))
            {
              UnitObjListNode *node = *i;
              UnitObj *unit = *node;

              F32 distance = (unit->Origin() - location).Magnitude2();
              if (distance < winningDistance)
              {
                winner = node;
                winningDistance = distance;
              }
            }
          }
        }

        if (winner)
        {
          UnitObj *unit = *winner;

          // Activate the constructor
          base->ActivateConstructor(winner);

          // Assign this constructor
          SetConstructor(unit);
          return;
        }
      }

      /*
      // Hmm, no constructor could be found, do we still have 
      // the prereqs for this, if not, put it back on order 
      // and get it out of our face
      if (!GetType().GetPrereqs().Have(GetBase().GetObject().GetTeam()))
      {
        Returned();
      }
      */
    }
  }


  //
  // Orphan this token
  //
  void Base::Token::Orphan()
  {
//    LOG_AI(("Token '%s' is now orphaned", type->GetName()))
    orderer = NULL;
    base->AddOrphanedToken(this);
  }


  //
  // The token has been accepted
  //
  void Base::Token::Accepted()
  {
    ASSERT(state == ONORDER)

    // Tell the base manager about the transition
    base->GetManager().MoveTypeOnOrderToInConstruction(*type);

    // Take the token from the orderer
    ASSERT(orderer)
    orderer->TakeToken();

    // Is this type constructed by a facility
    if (type->GetUpgradeFor() || type->GetConstructorType()->GetIsFacility())
    {
      // We're now waiting for a constructor
      SetState(WAITING);
    }
    else
    {
      ASSERT(placement)

      // We're now searching for somewhere to build
      SetState(SEARCHING);

      // Using the placement, find a location for the token
      placement->FindLocation(*this);
    }
  }


  //
  // The token has been returned
  //
  void Base::Token::Returned()
  {
    // If we're searching then abort it
    if (state == SEARCHING)
    {
      ASSERT(placement)

      placement->AbortFind(*this);
    }

    if (orderer)
    {
      // Returh the token to on order
      SetState(ONORDER);

      // Return the token from the orderer
      orderer->ReturnToken(this);

      // Tell the base manager about the transition
      base->GetManager().MoveTypeInConstructionToOnOrder(*type);
    }
    else
    {
      // Tell the base manager this 
      base->GetManager().RemoveTypeInConstruction(*type);

      // If there's a resource, remove it from that
      if (resource.Alive())
      {
        resource->RemoveOnOrder(*type);
      }

      // If there's a water, remove it from that
      if (water.Alive())
      {
        water->RemoveOnOrder(*type);
      }

      delete this;
    }
  }


  //
  // Set the location
  //
  void Base::Token::SetLocation(const Vector &locationIn, WorldCtrl::CompassDir dirIn)
  {
    ASSERT(state == SEARCHING)

    // Save the location
    location = locationIn;

    // Save the direction
    dir = dirIn;

    // We're now waiting for a constructor
    SetState(WAITING);
  }


  //
  // No location
  //
  void Base::Token::NoLocation()
  {
    ASSERT(state == SEARCHING)

    LOG_AI(("Token '%s' Couldn't find anywhere to build", type->GetName()))

    // Check to see if the plan has a fallback
    placement = placement->GetFallback();

    if (placement)
    {
      LOG_AI(("Token '%s' Using fallback placement", type->GetName()))

      // Using the plan, find a location for the token
      placement->FindLocation(*this);
    }
    else
    {
      LOG_AI(("Token '%s' No Fallback, we have failed!", type->GetName()))

      // Well, lets release our constructor if we have one
      ReturnConstructor();

      if (orderer)
      {
        ASSERT(base)

        // Get our original plan for the next time we try
        placement = &base->GetObject().GetPlacementManager().GetPlacement(orderer->GetPlacement());
      }

      // Return to on order and we'll try again later
      Returned();
    }
  }


  //
  // Set the constructor
  //
  void Base::Token::SetConstructor(UnitObj *constructorIn)
  {
    constructor = constructorIn;
    constructorId = constructor.DirectId();

    // Now that a constructor has been found, is this token 
    // constructed by a facility or by a constructor ?
    if (constructor->UnitType()->GetIsFacility())
    {
      // We are now facility constructing
      SetState(FACILITYCONSTRUCTING);

      // Tell the manager that we're waiting for notification from the constructor
      base->GetManager().RegisterNotificationHandler(constructor->Id(), *this);

      // Give the facility the order to construct the unit
      Orders::Game::Constructor::Generate(base->GetObject(), constructor->Id(), 0x258B47CE, type->GetNameCrc()); // "Order::UnitConstructor::Build"
    }
    else
    {
      // The constructor is on its way to the construction site
      SetState(LOCATING);

      // Tell the manager that we're waiting for notification from the constructor
      base->GetManager().RegisterNotificationHandler(constructor->Id(), *this);

      // Tell the constructor to build it
      Orders::Game::ClearSelected::Generate(base->GetObject());
      Orders::Game::AddSelected::Generate(base->GetObject(), constructor);
      Orders::Game::Build::Generate(base->GetObject(), type, location, dir, Orders::APPEND);
    }
  }


  //
  // Return constructor
  //
  void Base::Token::ReturnConstructor()
  {
    // If the constructor that was used is still 
    // alive assign it back to the idle constructors
    if (constructor.Alive())
    {
      // Remove from the working constructors and add to the idle constructors
      base->DeactivateConstructor(constructor);

      // Clear the constructor
      constructor = NULL;
      constructorId = 0;
    }
  }


  //
  // Handle notifications
  //
  void Base::Token::Notify(Notification &notification)
  {
    switch(notification.message)
    {
      case 0x4034E822: // "UnitBuild::Constructing"
      {
        // Remove from the handlers
        base->GetManager().UnregisterNotificationHandler(*this);

        ASSERT(state == LOCATING)

        // Return the constructor
        ReturnConstructor();

        // The first parameter is the id of the unit constructed ... 
        // check to see if its a constructor and if it is add to the 
        // bases idle constructors
        UnitObj *unit = Resolver::Object<UnitObj, UnitObjType>(notification.param1);

        if (unit)
        {
          // The constructor is now constructing
          SetState(CONSTRUCTING);

          // The unit being constructed is now the constructor
          constructor = unit;
          constructorId = constructor.DirectId();

          // Tell the manager that we're waiting for notification from the constructor
          base->GetManager().RegisterNotificationHandler(constructor->Id(), *this);
        }
        else
        {
          // The unit is dead, return the token since we didn't get to build it
          Returned();
        }
        break;
      }

      case 0x15B758A7: // "UnitBuild::BadLocation"
      {
        // Remove from the handlers
        base->GetManager().UnregisterNotificationHandler(*this);

        ASSERT(state == LOCATING)

        // We need to find another location
        SetState(SEARCHING);

        ASSERT(placement)

        // The location was bad, find another one
        placement->FindNextLocation(*this);

        // Note that the constructor was kept!
        break;
      }

      case 0x3A4B49C0: // "UnitBuild::Failed"
      {
        // Remove from the handlers
        base->GetManager().UnregisterNotificationHandler(*this);

        ASSERT(state == LOCATING)
        ASSERT(constructorId == notification.from.DirectId())

        LOG_AI(("Token '%s' Building Failed", type->GetName()))

        // Return the constructor
        ReturnConstructor();

        // Return this token to the orderer from which it came
        Returned();
        break;
      }

      case 0xFAC5A90A: // "UnitConstruct::Completed"
      {
        // Remove from the handlers
        base->GetManager().UnregisterNotificationHandler(*this);

        ASSERT(state == CONSTRUCTING)

        // Tell the base manager about it
        base->GetManager().RemoveTypeInConstruction(*type);

        // If there's a resource, remove it from that
        if (resource.Alive())
        {
          resource->RemoveOnOrder(*type);
        }

        // If there's a water, remove it from that
        if (water.Alive())
        {
          water->RemoveOnOrder(*type);
        }

        // Is the unit a alive ?
        if (constructor.Alive())
        {
          NewUnit(constructor);
        }

        // Was a companion created ?
        if (notification.param1)
        {
          UnitObj *unit = Resolver::Object<UnitObj, UnitObjType>(notification.param1);
          if (unit)
          {
            NewUnit(unit);
          }
        }


//        LOG_AI(("Token '%s' Construction Completed", type->GetName()))

        // This token is done
        delete this;
        break;
      }

      case 0x7FF81455: // "UnitConstructor::Completed"
      {
        // Remove from the handlers
        base->GetManager().UnregisterNotificationHandler(*this);

        ASSERT(state == FACILITYCONSTRUCTING)

        // The first parameter is the id of the unit constructed ... 
        // check to see if its a constructor and if it is add to the 
        // bases idle constructors
        UnitObj *unit = Resolver::Object<UnitObj, UnitObjType>(notification.param1);
        
        // The unit could have already been killed at this point ...
        if (unit)
        {
          NewUnit(unit);
        }

//        LOG_AI(("Token '%s' Construction Completed", type->GetName()))

        // Return the constructor
        ReturnConstructor();

        // Tell the base manager about it
        base->GetManager().RemoveTypeInConstruction(*type);

        // If there's a resource, remove it from that
        if (resource.Alive())
        {
          resource->RemoveOnOrder(*type);
        }

        // If there's a water, remove it from that
        if (water.Alive())
        {
          water->RemoveOnOrder(*type);
        }

        // This token is done like a dinner
        delete this;
        break;
      }

      case 0x3608BAC3: // "UnitConstructor::Failed"
      case 0x3B2EB6EA: // "UnitConstructor::Canceled"
      {
        ASSERT(state == FACILITYCONSTRUCTING || state == UPGRADING)

        if (state == FACILITYCONSTRUCTING)
        {
          // Remove from the handlers
          base->GetManager().UnregisterNotificationHandler(*this);

          ASSERT(constructorId == notification.from.DirectId())

  //        LOG_AI(("Token '%s' Facility Construction Canceled", type->GetName()))

          // Return the constructor
          ReturnConstructor();

          // Return this token to the orderer from which it came
          Returned();
        }
        break;
      }

      case 0xC3C43161: // "UnitUpgrade::Completed"
      {
        // Remove from the handlers
        base->GetManager().UnregisterNotificationHandler(*this);

        ASSERT(constructorId == notification.from.DirectId())
        ASSERT(state == UPGRADING)

        // Tell the base manager about it
        base->GetManager().RemoveTypeInConstruction(*type);

        // If there's a resource, remove it from that
        if (resource.Alive())
        {
          resource->RemoveOnOrder(*type);
        }

        // If there's a water, remove it from that
        if (water.Alive())
        {
          water->RemoveOnOrder(*type);
        }

        // Was this upgrader a constructor ?
        UnitObjListNode *node = base->FindActiveConstructor(constructor);
        if (node)
        {
          base->DeactivateConstructor(node);
        }

//        LOG_AI(("Token '%s' Upgrade Completed", type->GetName()))

        // This token is done like a dinner
        delete this;
        break;
      }

      case 0x11EAEF8E: // "Unit::Died"
      {
        // Remove from the handlers
        base->GetManager().UnregisterNotificationHandler(*this);

        ASSERT(constructorId == notification.from.DirectId())

        switch (state)
        {
          case LOCATING:
          case FACILITYCONSTRUCTING:
          case CONSTRUCTING:
          case UPGRADING:
            // We're waiting for a new constructor
            SetState(WAITING);
            break;

          default:
            ERR_FATAL(("Not expectecting a notification in this state!"))
        }
      }
    }
  }


  //
  // GetState
  //
  const char * Base::Token::GetState()
  {
    ASSERT(state < MAX_STATE)
    return (stateNames[state]);
  }


  //
  // SetState
  //
  void Base::Token::SetState(State newState)
  {
    SYNC_BRUTAL("Token '" << type->GetName() << "' State " << stateNames[state] << " -> " << stateNames[newState])

    // Based on the current state, remove ourselves from any particular lists etc
    switch (state)
    {
      case SEARCHING:
        // Remove from the tokens searching for positions
        base->tokensSearching.Unlink(this);
        break;

      case WAITING:
        // Remove from the tokens waiting for constructors
        base->tokensWaiting.Unlink(this);
        break;

      case LOCATING:
      case UPGRADING:
      case CONSTRUCTING:
      case FACILITYCONSTRUCTING:
        // Remove from the tokens which are constructing
        base->tokensConstructing.Unlink(this);
        break;
    }

    LOG_AI(("Token [%08X] '%s' changing from %s to %s", this, type->GetName(), stateNames[state], stateNames[newState]))

    state = newState;

    // Depending upon the new state, add ourselves to particular lists etc
    switch (state)
    {
      case SEARCHING:
        // Add to the tokens searching for suitable locations
        base->tokensSearching.Add(priority, this);
        break;

      case WAITING:
        // Add to the base's tree of tokens waiting for constructors
        base->tokensWaiting.Add(priority, this);
        break;

      case LOCATING:
      case UPGRADING:
      case CONSTRUCTING:
      case FACILITYCONSTRUCTING:
        // Add to the tokens constructing
        base->tokensConstructing.Add(priority, this);
        break;

    }

    // We want to process after state changes
    base->SetProcessTokens();
  }


  //
  // Deal with a freshly constructed unit
  //
  void Base::Token::NewUnit(UnitObj *unit)
  {
    ASSERT(unit)

    // Tell the orderer we completed the token
    if (orderer)
    {
      orderer->CompletedToken(this, unit);
    }

    if (resource.Alive())
    {
      // Give the unit to the resource
      resource->AddUnit(unit);
    }
    else if (script.Alive())
    {
      // Give the unit to the script
      script->AddUnit(unit);
    }
    else
    {
      if (water.Alive())
      {
        // Give the unit to the water
        water->AddUnit(unit);
      }

      // Is this a constructor ?
      if (unit->HasProperty(0xDCDE71CD)) // "Ability::Construction"
      {
        base->AddConstructor(unit);
      }

      // Add this unit to the base
      base->AddUnit(unit);
    }
  }

}
