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
#include "strategic_private.h"
#include "strategic_base.h"
#include "strategic_base_manager.h"
#include "strategic_base_orderer.h"
#include "strategic_base_state.h"
#include "strategic_base_token.h"
#include "strategic_object.h"
#include "promote.h"
#include "resolver.h"
#include "orders_game.h"


/////////////////////////////////////////////////////////////////////////////
//
// Definitions
//


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Base
  //

  
  //
  // Base::Base
  //
  Base::Base(Manager &manager, const Point<F32> &location, F32 orientation, const char *name, const char *configName, FScope *fScope)
  : name(name),
    configName(configName),
    manager(manager),
    location(location),
    orientation(orientation),
    states(&State::nodeBase),
    currentState(NULL),
    newState(FALSE),
    processTokens(FALSE),
    autoDisrupt(-1),
    disruptorIndex(0),
    disruptorCycle(0),
    allTokens(&Token::nodeBaseAll),
    tokensSearching(&Token::nodeBase),
    tokensWaiting(&Token::nodeBase),
    tokensConstructing(&Token::nodeBase),
    tokensOrphaned(&Token::nodeList)
  {
    RegisterConstruction(dTrack);

    GameIdent initialState;

//    LOG_AI(("%5.1f Base '%s' Created", GameTime::SimTotalTime(), GetName()))

    FScope *sScope;
    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x631D77FE: // "InitialState"
          initialState = StdLoad::TypeString(sScope);
          break;

        case 0x96880780: // "State"
        {
          GameIdent id = StdLoad::TypeString(sScope);
          states.Add(id.crc, new State(*this, id.str, sScope));
          break;
        }

        default:
          sScope->ScopeError("Unknown function '%s' in base '%s'", sScope->NameStr(), name);
      }
    }

    // GoTo the initial state
    GotoState(initialState);
  }


  //
  // Orderer::~Orderer
  //
  // Destructor
  //
  Base::~Base()
  {
//    LOG_AI(("%5.1f Base '%s' Destroyed", GameTime::SimTotalTime(), GetName()))

    // If we're in a state, leave it (shutdown procedure too complex to allow this)
//    if (currentState)
//    {
//      currentState->Leave();
//    }

    // Delete orphaned tokens
    tokensOrphaned.DisposeAll();

    // Delete all of the states
    states.DisposeAll();

    // Clean out constructor lists
    units.Clear();
    disruptors.Clear();
    constructorsIdle.Clear();
    constructorsWorking.Clear();

    RegisterDestruction(dTrack);
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Base::SaveState(FScope *scope)
  {
    // Save construction information
    StdSave::TypePoint(scope, "Location", location);
    StdSave::TypeF32(scope, "Orientation", orientation);
    StdSave::TypeString(scope, "Name", name.str);
    StdSave::TypeString(scope, "ConfigName", configName.str);

    // Save instance information
    scope = scope->AddFunction("Instance");

    Item::SaveState(scope->AddFunction("Parent"));
    
    FScope *fScope = scope->AddFunction("AllTokens");
    for (NList<Token>::Iterator t(&allTokens); *t; ++t)
    {
      (*t)->SaveState(fScope->AddFunction("Add"));
    }

    // Save the states tree
    fScope = scope->AddFunction("States");
    for (NBinTree<State>::Iterator s(&states); *s; ++s)
    {
      (*s)->SaveState(fScope->AddFunction("Add"));
    }

    StdSave::TypeU32(scope, "CurrentState", currentState->GetNameCrc());
    StdSave::TypeU32(scope, "NewState", newState);
    StdSave::TypeU32(scope, "ProcessTokens", processTokens);
    StdSave::TypeReaperList(scope, "Units", units);
    StdSave::TypeReaperList(scope, "Disruptors", disruptors);
    StdSave::TypeU32(scope, "AutoDisrupt", autoDisrupt);
    StdSave::TypeU32(scope, "DisruptorIndex", disruptorIndex);
    StdSave::TypeU32(scope, "DisruptorCycle", disruptorCycle);
    StdSave::TypeReaperList(scope, "ConstructorsIdle", constructorsIdle);
    StdSave::TypeReaperList(scope, "ConstructorsWorking", constructorsWorking);
    
    manager.SaveTokenBinTree(tokensSearching, scope->AddFunction("TokensSearching"));
    manager.SaveTokenBinTree(tokensWaiting, scope->AddFunction("TokensWaiting"));
    manager.SaveTokenBinTree(tokensConstructing, scope->AddFunction("TokensConstructing"));
    manager.SaveTokenList(tokensOrphaned, scope->AddFunction("TokensOrphaned"));
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Base::LoadState(FScope *scope, void *context)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x411AC76D: // "Parent"
          Item::LoadState(sScope, context);
          break;

        case 0x48096F5A: // "AllTokens"
        {
          FScope *fScope;

          while ((fScope = sScope->NextFunction()) != NULL)
          {
            switch (fScope->NameCrc())
            {
              case 0x9F1D54D0: // "Add"
              {
                // Constructor adds token to the all lists
                new Token(fScope, manager);
                break;
              }
            }
          }
          break;
        }

        case 0xEB9C8D59: // "States"
        {
          for (NBinTree<State>::Iterator i(&states); *i; ++i)
          {
            if (FScope *fScope = sScope->NextFunction())
            {
              (*i)->LoadState(fScope);
            }
          }
          break;
        }

        case 0xFE325917: // "CurrentState"
          currentState = states.Find(StdLoad::TypeU32(sScope));
          break;

        case 0x35BC5703: // "NewState"
          newState = StdLoad::TypeU32(sScope);
          break;

        case 0xD20160BE: // "ProcessTokens"
          processTokens = StdLoad::TypeU32(sScope);
          break;

        case 0xCED02493: // "Units"
          StdLoad::TypeReaperList(sScope, units);
          Resolver::ObjList<UnitObj, UnitObjType, UnitObjListNode>(units);
          break;

        case 0x1BF75F38: // "Disruptors"
          StdLoad::TypeReaperList(sScope, disruptors);
          Resolver::ObjList<UnitObj, UnitObjType, UnitObjListNode>(disruptors);
          break;

        case 0x93075ED7: // "AutoDisrupt"
          autoDisrupt = StdLoad::TypeU32(sScope);
          break;

        case 0xD9D7B747: // "DisruptorIndex"
          disruptorIndex = StdLoad::TypeU32(sScope);
          break;

        case 0xC134342B: // "DisruptorCycle"
          disruptorCycle = StdLoad::TypeU32(sScope);
          break;

        case 0x95358C83: // "ConstructorsIdle"
          StdLoad::TypeReaperList(sScope, constructorsIdle);
          Resolver::ObjList<UnitObj, UnitObjType, UnitObjListNode>(constructorsIdle);
          break;
        
        case 0xB6194C3B: // "ConstructorsWorking"
          StdLoad::TypeReaperList(sScope, constructorsWorking);
          Resolver::ObjList<UnitObj, UnitObjType, UnitObjListNode>(constructorsWorking);
          break;

        case 0x7D4149AB: // "TokensSearching"
          manager.LoadTokenBinTree(tokensSearching, sScope);
          break;

        case 0x52015470: // "TokensWaiting"
          manager.LoadTokenBinTree(tokensWaiting, sScope);
          break;

        case 0x6C5528B7: // "TokensConstructing"
          manager.LoadTokenBinTree(tokensConstructing, sScope);
          break;

        case 0xFF360560: // "TokensOrphaned"
          manager.LoadTokenList(tokensOrphaned, sScope);
          break;
      }
    }
  }


  //
  // Process
  //
  // Process the orderer
  //
  void Base::Process()
  {
    if (newState)
    {
      // Clear new state flag
      newState = FALSE;

      // Enter the new state
      currentState->Enter();
    }
    else
    {
      currentState->Process();
    }

    // Has the list of constructors/units changed ?
    if (processTokens)
    {
      processTokens = FALSE;

      units.PurgeDead();
      constructorsIdle.PurgeDead();
      constructorsWorking.PurgeDead();

      // Process all of the tokens waiting for constructors in the system
      NBinTree<Token>::Iterator w(&tokensWaiting);
      while (Token *token = w++)
      {
        token->CheckConstructor(units, constructorsIdle);
      }
    }

    // Are we auto disrupting ?
    if (autoDisrupt != -1)
    {
      // Has sufficient cycles elapsed since the last time we sorted the disruptors ?
      if (GameTime::SimCycle() - disruptorCycle > 300)
      {
        // Save the cycle we processed
        disruptorCycle = GameTime::SimCycle();

        // Check to see if the index has gone out of range
        if (disruptorIndex > disruptors.GetCount())
        {
          disruptors.PurgeDead();
          disruptorIndex = 0;
        }

        // Get the list node
        UnitObjListNode *node = disruptors[disruptorIndex];

        if (node)
        {
          // Get the unit list
          UnitObj *unit = node->GetPointer();

          // If it was alive then find the most undisrupted spot and send that unit there
          if (unit)
          {
            // Iterate the map, find the cluster with the most defense and the least disruption
            MapCluster *mapCluster = WorldCtrl::GetCluster(0, 0);
            MapCluster *winningCluster = NULL;

            U32 team = GetObject().GetTeam()->GetId();
            Point<U32> cluster;
            U32 minDisruption = U32_MAX;
            U32 maxDefense = 0;

            for (cluster.z = 0; cluster.z < WorldCtrl::ClusterMapZ(); cluster.z++)
            {
              for (cluster.x = 0; cluster.x < WorldCtrl::ClusterMapX(); cluster.x++)
              {
                U32 defense = mapCluster->ai.GetDefense(team, autoDisrupt);

                if (defense)
                {
                  U32 disruption = mapCluster->ai.GetDisruption();

                  if (disruption < minDisruption)
                  {
                    minDisruption = disruption;
                    maxDefense = defense;
                    winningCluster = mapCluster;
                  }
                  else if (disruption == minDisruption && defense > maxDefense)
                  {
                    maxDefense = defense;
                    winningCluster = mapCluster;
                  }
                }

                mapCluster++;
              }
            }

            if (winningCluster)
            {
              Point<F32> point = winningCluster->GetMidPoint();
              Vector location(point.x, Terrain::FindFloor(point.x, point.z), point.z);

              // Move this disruptor to the middle of this cluster
              Orders::Game::ClearSelected::Generate(GetObject());
              Orders::Game::AddSelected::Generate(GetObject(), unit);
              Orders::Game::Move::Generate(GetObject(), location, FALSE, Orders::FLUSH);
            }
          }

          disruptorIndex++;
        }
      }
    }
  }


  //
  // GetObject
  //
  // Get the strategic object who owns this orderer
  //
  Object & Base::GetObject()
  {
    return (manager.GetObject());
  }


  //
  // GetObject
  //
  // Get the strategic object who owns this orderer
  //
  const Object & Base::GetObject() const
  {
    return (manager.GetObject());
  }


  //
  // Offer cash to the base
  //
  Base::Offer Base::OfferCash(U32 &cash, U32 &spent)
  {
    // Propogate to the current state
    return (currentState->OfferCash(cash, spent));
  }


  //
  // GotoState
  //
  // Goto the given state
  //
  void Base::GotoState(const GameIdent &state)
  {
//    LOG_AI(("%5.1f Base '%s' Entering State '%s'", GameTime::SimTotalTime(), GetName(), state.str))

    // If there was a previous state, leave it
    if (currentState)
    {
      currentState->Leave();
    }

    // Change to the new state
    currentState = states.Find(state.crc);
    newState = TRUE;

    if (!currentState)
    {
      ERR_CONFIG(("Could not find State '%s' in Base '%s'", state.str, GetName()))
    }
  }


  //
  // Set the weighting of one of the orderers
  //
  void Base::SetOrdererWeighting(const GameIdent &ordererName, U32 weighting)
  {
    ASSERT(currentState)
    currentState->SetOrdererWeighting(ordererName, weighting);
  }


  //
  // Set the priority of one of the orderers
  //
  void Base::SetOrdererPriority(const GameIdent &ordererName, U32 priority)
  {
    ASSERT(currentState)
    currentState->SetOrdererPriority(ordererName, priority);
  }


  //
  // Assign constructors to this base
  //
  void Base::AssignConstructors(const char *tagName)
  {
    processTokens = TRUE;

    // Was a tag specified ?
    if (tagName)
    {
      // Resolve the tag
      TagObj *tag = TagObj::FindTag(tagName);

      if (tag)
      {
        // Iterate the units in the tag
        for (MapObjList::Iterator i(&tag->list); *i; i++)
        {
          // Is it alive
          if (!(*i)->Alive())
          {
            continue;
          }

          // Is this a unit ?
          UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(**i);
          if (!unit)
          {
            continue;
          }

          // Is it on our team ?
          if (unit->GetTeam() != GetObject().GetTeam())
          {
            continue;
          }

          // Is it a constructor ?
          if (!unit->HasProperty(0xDCDE71CD)) // "Ability::Construction"
          {
            continue;
          }

//          LOG_AI(("Assigning constructor [%d] '%s' to base '%s'", unit->Id(), unit->UnitType()->GetName(), GetName()))

          // Add this constructor to the list of idle constructors
          constructorsIdle.Append(unit);
        }
      }
      else
      {
        ERR_CONFIG(("Could not find tag '%s' when assign constructors to base '%s'", tagName, GetName()))
      }
    }
    else
    {
      // Iterate all of the units on this team
      for (NList<UnitObj>::Iterator u(&GetObject().GetTeam()->GetUnitObjects()); *u; u++)
      {
        UnitObj *unit = *u;

        // Is it a constructor ?
        if (!unit->HasProperty(0xDCDE71CD)) // "Ability::Construction"
        {
          continue;
        }

//        LOG_AI(("Assigning constructor [%d] '%s' to base '%s'", unit->Id(), unit->UnitType()->GetName(), GetName()))

        // Add this constructor to the list of idle constructors
        constructorsIdle.Append(unit);
      }
    }
  }


  //
  // Assign units to this base
  //
  void Base::AssignUnits(const char *tagName)
  {
    processTokens = TRUE;

    // Was a tag specified ?
    if (tagName)
    {
      // Resolve the tag
      TagObj *tag = TagObj::FindTag(tagName);

      if (tag)
      {
        // Iterate the units in the tag
        for (MapObjList::Iterator i(&tag->list); *i; i++)
        {
          // Is it alive
          if (!(*i)->Alive())
          {
            continue;
          }

          // Is this a unit ?
          UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(**i);
          if (!unit)
          {
            continue;
          }

          // Is it on our team ?
          if (unit->GetTeam() != GetObject().GetTeam())
          {
            continue;
          }

//          LOG_AI(("Assigning unit [%d] '%s' to base '%s'", unit->Id(), unit->UnitType()->GetName(), GetName()))

          // Add this constructor to the list of idle constructors
          AddUnit(unit);
        }
      }
      else
      {
        ERR_CONFIG(("Could not find tag '%s' when assigning units to base '%s'", tagName, GetName()))
      }
    }
    else
    {
      // Iterate all of the units on this team
      for (NList<UnitObj>::Iterator u(&GetObject().GetTeam()->GetUnitObjects()); *u; u++)
      {
        UnitObj *unit = *u;

//        LOG_AI(("Assigning unit [%d] '%s' to base '%s'", unit->Id(), unit->UnitType()->GetName(), GetName()))

        // Add this constructor to the list of idle constructors
        AddUnit(unit);
      }
    }
  }


  //
  // Set auto disruption
  //
  void Base::AutoDisrupt(const char *armourClass)
  {
    if (ArmourClass::ArmourClassExists(armourClass))
    {
      autoDisrupt = ArmourClass::Name2ArmourClassId(armourClass);
    }
    else
    {
      autoDisrupt = -1;
    }
  }


  //
  // Add a constructor
  //
  void Base::AddConstructor(UnitObj *constructor)
  {
    ASSERT(!constructorsIdle.Exists(constructor))
    constructorsIdle.Append(constructor);
    processTokens = TRUE;
  }


  //
  // Add a unit
  //
  void Base::AddUnit(UnitObj *unit)
  {
    ASSERT(!units.Exists(unit))

    units.Append(unit);

    if (unit->UnitType()->GetDisruptor())
    {
      disruptors.Append(unit);
    }

    processTokens = TRUE;
    manager.SetTakeOrders();
  }


  //
  // Remove a unit
  //
  void Base::RemoveUnit(UnitObj *unit)
  {
    units.Remove(unit);

    if (unit->UnitType()->GetDisruptor())
    {
      disruptors.Remove(unit);
    }
  }


  //
  // Activate a constructor
  //
  void Base::ActivateConstructor(UnitObj *constructor)
  {
    ActivateConstructor(constructorsIdle.Find(constructor));
  }


  //
  // Activate a constructor
  //
  void Base::ActivateConstructor(UnitObjListNode *node)
  {
    ASSERT(node)
    constructorsIdle.NList<UnitObjListNode>::Unlink(node);
    constructorsWorking.NList<UnitObjListNode>::Append(node);
  }


  //
  // Deactivate a constructor
  //
  void Base::DeactivateConstructor(UnitObj *constructor)
  {
    DeactivateConstructor(constructorsWorking.Find(constructor));
  }


  //
  // Deactivate a constructor
  //
  void Base::DeactivateConstructor(UnitObjListNode *node)
  {
    ASSERT(node)
    constructorsWorking.NList<UnitObjListNode>::Unlink(node);
    constructorsIdle.NList<UnitObjListNode>::Append(node);
    processTokens = TRUE;
  }


  //
  // Is this an active constructor
  //
  UnitObjListNode * Base::FindActiveConstructor(UnitObj *constructor)
  {
    return (constructorsWorking.Find(constructor));
  }


  //
  // Is this an inactive constructor
  //
  UnitObjListNode * Base::FindInactiveConstructor(UnitObj *constructor)
  {
    return (constructorsIdle.Find(constructor));
  }


  //
  // Add orphaned token
  //
  void Base::AddOrphanedToken(Token *token)
  {
    ASSERT(token)
    tokensOrphaned.Append(token);
  }


  //
  // Remove orphaned token
  //
  void Base::RemoveOrphanedToken(Token *token)
  {
    ASSERT(token)
    tokensOrphaned.Unlink(token);
  }


  //
  // TokenConstruction
  //
  // Register construction of a token
  //
  void Base::TokenConstruction(Token &token)
  {
    allTokens.Append(&token);
  }


  //
  // TokenDestruction
  //
  // Register destruction of a token
  //
  void Base::TokenDestruction(Token &token)
  {
    allTokens.Unlink(&token);
  }


  //
  // Find the token with the given id
  //
  Base::Token * Base::FindToken(U32 id)
  {
    for (NList<Token>::Iterator i(&allTokens); *i; ++i)
    {
      if ((*i)->GetId() == id)
      {
        return (*i);
      }
    }

    return (NULL);
  }
}
