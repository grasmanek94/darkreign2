/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Orderer
// 25-MAR-1999
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "perfstats.h"
#include "strategic_private.h"
#include "strategic_object.h"
#include "strategic_base_state.h"
#include "strategic_base_token.h"
#include "strategic_base_orderer.h"
#include "strategic_base_orderer_type.h"
#include "strategic_base_orderer_manifest.h"
#include "console.h"


/////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
#ifdef DEVELOPMENT
  #define CON_TEAM(x) CONSOLE(GetObject().GetTeam()->GetConsoleId(0x29853B05), x ) // "Strategic::Orderer"
#else
  #define CON_TEAM(x)
#endif


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Orderer
  //

  const U32 MaxManifestProcessDelay = 20;

  
  //
  // Base::Orderer::Orderer
  //
  Base::Orderer::Orderer(Base::State &state, const char *name, U32 priority, FScope *fScope) 
  : name(name),
    state(state),
    manifest(NULL),
    priority(priority),
    reserveMinimum(StdLoad::TypeU32(fScope, "ReserveMinimum", 0)),
    reserveMaximum(StdLoad::TypeU32(fScope, "ReserveMaximum", U32_MAX)),
    types(&Type::nodeTree),
    onOrder(&Token::nodeList),
    inConstruction(&Token::nodeList),
    online(FALSE),
    lastTime(0)
  {
    CON_TEAM(("%5.1f Base::Orderer '%s' Created", GameTime::SimTotalTime(), GetName()))
//    LOG_AI(("%5.1f Base::Orderer '%s' Created", GameTime::SimTotalTime(), GetName()))

    // Load the manifest
    manifest = Manifest::Create(*this, fScope->GetFunction("Manifest"));
  }


  //
  // Base::Orderer::~Orderer
  //
  // Destructor
  //
  Base::Orderer::~Orderer()
  {
    CON_TEAM(("%5.1f Base::Orderer '%s' Destroyed", GameTime::SimTotalTime(), GetName()))
//    LOG_AI(("%5.1f Base::Orderer '%s' Destroyed", GameTime::SimTotalTime(), GetName()))

    // Delete the manifest
    delete manifest;

    // Clear out the on order list and the in construction list
    types.DisposeAll();
    onOrder.DisposeAll();
    inConstruction.DisposeAll();
    onField.Clear();
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Base::Orderer::Save(FScope *scope)
  {
    FScope *sScope = scope->AddFunction("Types");
    for (NBinTree<Type>::Iterator i(&types); *i; ++i)
    {
      (*i)->SaveState(StdSave::TypeU32(sScope, "Add", i.GetKey()));
    }

    state.GetBase().GetManager().SaveTokenList(onOrder, scope->AddFunction("OnOrder"));
    state.GetBase().GetManager().SaveTokenList(inConstruction, scope->AddFunction("InConstruction"));

    StdSave::TypeU32(scope, "Priority", priority);
    StdSave::TypeU32(scope, "Online", online);
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Base::Orderer::Load(FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xBC94CAD6: // "Types"
        {
          FScope *fScope;

          while ((fScope = sScope->NextFunction()) != NULL)
          {
            switch (fScope->NameCrc())
            {
              case 0x9F1D54D0: // "Add"
              {
                U32 key = StdLoad::TypeU32(fScope);
                types.Add(key, new Type(fScope));
                break;
              }
            }
          }
          break;
        }

        case 0x189FFF75: // "OnOrder"
          state.GetBase().GetManager().LoadTokenList(onOrder, sScope, this);
          break;

        case 0x2544363E: // "InConstruction"
          state.GetBase().GetManager().LoadTokenList(inConstruction, sScope, this);
          break;

        case 0xFFF34C6F: // "Priority"
          priority = StdLoad::TypeU32(sScope);
          break;

        case 0x5AAFA976: // "Online"
          online = StdLoad::TypeU32(sScope);
          break;
      }
    }
  }


  //
  // Enter
  //
  // Enter the orderer
  //
  void Base::Orderer::Enter()
  {
    online = TRUE;

    CON_TEAM(("%5.1f Base::Orderer '%s' Enter", GameTime::SimTotalTime(), GetName()))
//    LOG_AI(("%5.1f Base::Orderer '%s' Enter", GameTime::SimTotalTime(), GetName()))

    // Enter the manifest
    manifest->Enter();
  }


  //
  // Process
  //
  // Process the orderer
  //
  Bool Base::Orderer::Process()
  {
    ASSERT(online)

    // Only process the manifest if there's nothing on order or we haven't done it in a while
    if (!onOrder.GetCount() || (GameTime::SimCycle() - lastTime) > MaxManifestProcessDelay)
    {
      lastTime = GameTime::SimCycle();

      PERF_S("Manifest")
      // Process the manifest
      manifest->Process();
      PERF_E("Manifest")
    }

    // If there's any tokens on order then we're not satified
    return (onOrder.GetCount() ? FALSE : TRUE);
  }


  //
  // Leave
  //
  void Base::Orderer::Leave()
  {
    ASSERT(online)

    CON_TEAM(("%5.1f Base::Orderer '%s' Leave", GameTime::SimTotalTime(), GetName()))
//    LOG_AI(("%5.1f Base::Orderer '%s' Leave", GameTime::SimTotalTime(), GetName()))
    
    // If there were types on order then deactivate this orderer
    if (onOrder.GetCount())
    {
      state.DeactivateOrderer(*this);
    }

    // Orphan all of the tokens belonging to this orderer 
    // (since its not going to get processed anymore)
    NList<Token>::Iterator t(&inConstruction);
    Token *token;
    while ((token = t++) != NULL)
    {
      inConstruction.Unlink(token);
      token->Orphan();
    }

    // Delete all the tokens which are on order
    onOrder.DisposeAll();

    // The types are no longer used either
    types.DisposeAll();

    // We're offline
    online = FALSE;
  }


  //
  // GetObject
  //
  // Get the strategic object who owns this orderer
  //
  Object & Base::Orderer::GetObject()
  {
    return (state.GetBase().GetObject());
  }


  //
  // SubmitToken
  //
  // Submit and order for a particular type
  //
  void Base::Orderer::SubmitToken(Token *token)
  {
    ASSERT(online)
    ASSERT(token)

    // Find this type in the type tree
    Type *type = types.Find(token->GetType().GetNameCrc());

    if (!type)
    {
      type = new Type(token->GetType());
      types.Add(token->GetType().GetNameCrc(), type);
    }

    // Increment the amount of this type
    type->amount++;

    // Tell the state which we are in that 
    // this orderer has become active
    if (!onOrder.GetCount())
    {
      state.ActivateOrderer(*this);
    }

    // Add to the on order list
    onOrder.Append(token);

    // Tell the base manager that its time to take orders
    state.GetBase().GetManager().SetTakeOrders();
  }


  //
  // ReturnToken
  //
  // Return an order for a particular type
  //
  void Base::Orderer::ReturnToken(Token *token)
  {
    ASSERT(online)
    ASSERT(token)
    ASSERT(token->nodeList.InUse())

    // Remove this token from the in construction list
    inConstruction.Unlink(token);

    // Tell the state which we are in that 
    // this orderer has become active
    if (!onOrder.GetCount())
    {
      state.ActivateOrderer(*this);
    }

    // Add to the on order list
    onOrder.Prepend(token);

    // Tell the base manager that its time to take orders
    state.GetBase().GetManager().SetTakeOrders();
  }


  //
  // UsedToken
  //
  // Token was used (an object was constructed)
  //
  void Base::Orderer::UsedToken(Token *token)
  {
    ASSERT(online)
    ASSERT(token)
    ASSERT(token->nodeList.InUse())

    // Remove this token from the inConstruction list
    inConstruction.Unlink(token);

    // Find this type in the type tree
    Type *type = types.Find(token->GetType().GetNameCrc());

    ASSERT(type)
    ASSERT(type->amount > 0)

    // Is this the last of this type ?
    if (--type->amount == 0)
    {
      types.Dispose(type);
    }
  }


  //
  // CompletedToken
  //
  // Token produced a unit
  //
  void Base::Orderer::CompletedToken(Token *, UnitObj *unit)
  {
    onField.AppendNoDup(unit);
  }


  //
  // WhatToOrder
  //
  // What does this orderer want
  //
  Base::Token * Base::Orderer::WhatToOrder()
  {
    ASSERT(online)

    if (!manifest->NoShuffleOrder())
    {
      Reshuffle();
    }

    return (onOrder.GetHead());
  }


  //
  // TakeToken
  //
  // Take the next item which is order
  //
  void Base::Orderer::TakeToken()
  {
    ASSERT(online)

    Token *token = onOrder.GetHead();
    ASSERT(token)

    // Remove from the on order list
    onOrder.Unlink(token);

    // Append to the in construction list
    inConstruction.Append(token);

    // If there's nothing left on order then tell 
    // the state that this order has become idle
    if (!onOrder.GetCount())
    {
      state.DeactivateOrderer(*this);
    }
  }


  //
  // GetPlacement
  //
  const GameIdent & Base::Orderer::GetPlacement()
  {
    return (manifest->GetPlacement());
  }


  //
  // Reshuffle
  //
  // Sort the onOrder list by prereq fullfillment
  //
  void Base::Orderer::Reshuffle()
  {
    NList<Token> temp(&Token::nodeList);
    onOrder.Transfer(temp);
    NList<Token>::Node *node = NULL;
    NList<Token>::Iterator t(&temp);
    while (Token *token = t++)
    {
      temp.Unlink(token);

      if (token->GetType().GetPrereqs().Have(state.GetBase().GetObject().GetTeam()))
      {
        if (node)
        {
          // If we have the prereqs, put it in the list before the first failure
          onOrder.InsertBefore(node, token);
        }
        else
        {
          // Append to the list
          onOrder.Append(token);
        }
      }
      else
      {
        // We don't have the prereqs, put it down the end
        onOrder.Append(token);
        if (!node)
        {
          node = &token->nodeList;
        }
      }
    }
  }


  //
  // Are we meant to block if we don't get our prereqs
  //
  Bool Base::Orderer::NoPrereqStall()
  {
    ASSERT(manifest)
    return (manifest->NoPrereqStall());
  }


  //
  // Are we not meant to build beyond our weighting
  //
  Bool Base::Orderer::NoBeyondWeighting()
  {
    ASSERT(manifest)
    return (manifest->NoBeyondWeighting());
  }


}
