/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Base State
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_base_state.h"
#include "strategic_base_orderer.h"
#include "strategic_base_token.h"
#include "strategic_base_manager.h"
#include "strategic_base_recycler.h"
#include "strategic_config.h"
#include "strategic_object.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Base::State
  //


  //
  // Constructor
  //
  Base::State::State(Base &base, const char *name, FScope *fScope)
  : base(base),
    name(name),
    orderers(&Orderer::nodeBaseState),
    ordererList(&Orderer::nodeList),
    recyclerList(&Recycler::nodeList),
    ordererIter(&ordererList),
    satisfied(TRUE)
  {
    FScope *iScope;
    FScope *sScope;

    // Load the orderers
    iScope = fScope->GetFunction("Orderers");
    while ((sScope = iScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x9F1D54D0: // "Add"

          // Get the name of the orderer
          GameIdent ordererName = StdLoad::TypeString(sScope);

          // Find the configuration for this orderer
          Config::Generic *config = Config::FindConfig(0xF2230BDA, ordererName); // "Base::Orderer"

          if (!config)
          {
            fScope->ScopeError("Could not find Base::Orderer '%s'", ordererName.crc);
          }

          // Reset the manager
          //Reset();

          // Get the resource weighting of the orderer
          U32 resource = StdLoad::TypeU32(sScope, 100);

          // Get the priority of the orderer
          U32 priority = StdLoad::TypeU32(sScope, 100);

          // Get the initial usage of the orderer
          U32 usage = StdLoad::TypeU32(sScope, U32(0));

          FScope *fScope = config->GetFScope();

          Orderer *orderer = new Orderer(*this, ordererName.str, priority, fScope);
          orderers.AddItem(*orderer, resource, priority);
          orderers.AddUsage(*orderer, usage);
          ordererList.Append(orderer);

          FSCOPE_CHECK(fScope)
          break;
      }
    }

    iScope = fScope->GetFunction("Recyclers", FALSE);
    if (iScope)
    {
      while ((sScope = iScope->NextFunction()) != NULL)
      {
        switch (sScope->NameCrc())
        {
          case 0x9F1D54D0: // "Add"
            recyclerList.Append(new Recycler(*this, sScope));
            break;
        }
      }
    }

    // Is there are state to go to if the orderers are satisfied
    stateSatisfied = StdLoad::TypeString(fScope, "Satisfied", "");

    // Do we need to be idle before being satisfied
    idle = StdLoad::TypeU32(fScope, "Idle", 0, Range<U32>::flag);
  }


  //
  // Destructor
  //
  Base::State::~State()
  {
    ordererList.UnlinkAll();
    recyclerList.DisposeAll();
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Base::State::SaveState(FScope *scope)
  {
    FScope *sScope = scope->AddFunction("OrdererList");
    for (NList<Orderer>::Iterator i(&ordererList); *i; ++i)
    {
      // Get this orderer
      Orderer &o = **i;

      // Create the scope for this orderer
      FScope *fScope = sScope->AddFunction("Add");

      // Save weighting group data
      StdSave::TypeU32(fScope, "Weight", o.weight);
      StdSave::TypeU32(fScope, "Usage", o.usage);
      StdSave::TypeU32(fScope, "Idle", o.idle);
      StdSave::TypeU32(fScope, "LastUsage", o.lastUsage);

      // Save orderer data
      (*i)->Save(fScope->AddFunction("Orderer"));
    }

    StdSave::TypeU32(scope, "Satisfied", satisfied);
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Base::State::LoadState(FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xBC0D1171: // "OrdererList"
        {
          for (NList<Orderer>::Iterator i(&ordererList); *i; ++i)
          {
            // Get this orderer
            Orderer &o = **i;

            if (FScope *fScope = sScope->NextFunction())
            {
              // Load the weighting group data
              U32 weight = StdLoad::TypeU32(fScope, "Weight");
              U32 usage = StdLoad::TypeU32(fScope, "Usage");
              Bool idle = StdLoad::TypeU32(fScope, "Idle");
              U32 lastUsage = StdLoad::TypeU32(fScope, "LastUsage");

              // Load orderer data
              (*i)->Load(fScope->GetFunction("Orderer"));

              // Now adjust using the weighting group data
              orderers.ModifyItemWeighting(o, weight);

              // Clear out existing usage
              orderers.RemoveUsage(o, orderers.GetItemUsage(o));
              orderers.AddUsage(o, usage);

              orderers.ModifyItemPriority(o, o.GetPriority());
            
              if (o.idle && !idle)
              {
                orderers.ActivateItem(o);
              }

              o.lastUsage = lastUsage;
            }
            else
            {
              ERR_FATAL(("Missing orderer %d/%d", ordererList.GetCount(), sScope->GetBodyCount()));
            }
          }
          break;
        }

        case 0xC0CAC233: // "Satisfied"
          satisfied = StdLoad::TypeU32(sScope);
          break;
      }
    }
  }


  //
  // Process
  //
  // Process the state
  //
  void Base::State::Process()
  {
    /*
    // Are we at the end of the iterator
    if (!*ordererIter)
    {
      // If we're satisfied, and theres a state to go to, go to it
      if (satisfied && !stateSatisfied.Null())
      {
        base.GotoState(stateSatisfied);
      }

      // Reset satisfied flag
      satisfied = TRUE;

      // Reset iterator
      !ordererIter;
    }

    if (*ordererIter)
    {
      satisfied &= (*ordererIter)->Process();

      // Increment the iterator
      ++ordererIter;
    }

    */

    // Check to see if all the orderers are satisfied
    Bool satisfied = TRUE;

    // Process all of the recyclers
    for (NList<Recycler>::Iterator r(&recyclerList); *r; ++r)
    {
      (*r)->Process();
    }

    // Process all of the orderers
    for (NList<Orderer>::Iterator o(&ordererList); *o; ++o)
    {
      // Limit on order to 5 and limit in construction to 15
      if 
      (
        (*o)->GetTokensOnOrder().GetCount() < 5 &&
        (*o)->GetTokensInConstruction().GetCount() < 15
      )
      {
        satisfied &= (*o)->Process();
      }
    }

    if (satisfied && !stateSatisfied.Null())
    {
      if 
      (
        !(
          idle && 
          (
            base.GetTokensSearching().GetCount() || 
            base.GetTokensWaiting().GetCount() || 
            base.GetTokensConstructing().GetCount()
          )
        )
      )
      {
        base.GotoState(stateSatisfied);
      }
    }
  }


  //
  // OfferCash
  //
  // Offer cash to the base
  //
  Base::Offer Base::State::OfferCash(U32 &cash, U32 &spent)
  {
    // Cash has been offered, are there any orderers who want to spend it ???
    Weighting::Cuts<Orderer> cuts(&Orderer::nodeCut);

    // Computer the orderers cuts and then offer the cash to the winner ...
    orderers.Process(cuts);

    Base::Offer offer = OFFER_TURNDOWN;

    // Go through the items in priority
    for (NBinTree<Weighting::Cuts<Orderer>::Item>::Iterator i(&cuts.items); *i; ++i)
    {
      // First offer the cash to the orderer's which are below their cuts
      offer = OfferCash(cash, spent, (*i)->belowCut);

      // If they didn't want it then offer to orderer's which are above their cuts
      if (offer == OFFER_TURNDOWN)
      {
        offer = OfferCash(cash, spent, (*i)->aboveCut);
      }

      // If they didn't want it then offer to orderer's above their cut with lower priority
      if (offer != OFFER_TURNDOWN)
      {
        break;
      }
    }

    cuts.Clear();

    // Return the offer status
    return (offer);
  }


  //
  // Offer cash to a list of Orderers
  //
  Base::Offer Base::State::OfferCash(U32 &cash, U32 &spent, const NBinTree<Orderer, F32> &ordererTree)
  {
    for (NBinTree<Orderer, F32>::Iterator o(&ordererTree); *o; o++)
    {
      Orderer *orderer = *o;

      // Check to see if we've got too much cash for this orderer
      if (cash < orderer->GetReserveMaximum())
      {
        // Does this orderer want something ?
        Token *token = orderer->WhatToOrder();

        if (token)
        {
  //        LOG_AI(("Orderer '%s' wants item '%s' for %d [%d] [%d]", orderer->GetName(), token->GetType().GetName(), token->GetType().GetResourceCost(), cash, orderer->GetReserveMinimum()))

          // Do we have the prereqs for this type ?
          if (!token->GetType().GetPrereqs().Have(GetBase().GetObject().GetTeam()))
          {
            // Does this orderer want us to block if prereqs aren't met ?
            if (orderer->NoPrereqStall())
            {
              return (OFFER_SHORT);
            }

  //          LOG_AI(("We don't have the prereqs for that at the moment"))
            continue;
          }

          // Are already beyond our weighting ?
          if (orderer->NoBeyondWeighting())
          {
            F32 weighting = orderers.GetWeighting() ? orderers.GetItemWeighting(*orderer) / orderers.GetWeighting() : F32_MAX_MOD;
            F32 usage = orderers.GetUsage() ? orderers.GetItemUsage(*orderer) / orderers.GetUsage() : F32_MAX_MOD;

            if (usage > weighting)
            {
              // Turn down the offer
              return (OFFER_TURNDOWN);
            }
          }

          U32 cost = token->GetType().GetResourceCost();

          // Do we have enough cash ?
          if (cash >= (cost + orderer->GetReserveMinimum()))
          {
  //          LOG_AI(("We can afford that [%d:%d] [%d]", cost, cash, orderer->GetReserveMinimum()))

            // Tell the token that it has been accepted
            token->Accepted();

            // Spend the cash
            spent = cost;
            cash -= cost;

            // Modify this orderers usage
            orderers.AddUsage(*orderer, cost);

            return (OFFER_SPENT);
          }
          else
          {
  //          LOG_AI(("We can't afford that [%d:%d] [%d]", cost, cash, orderer->GetReserveMinimum()))

            // Check the items in the pipeline (which are waiting for constructors)
            // to see if we're more important than any of them.  Start throwing away
            // lower priority items until we get the cash we need.

            for (NBinTree<Token>::Iterator t(&base.tokensWaiting); *t; t++)
            {
              Token *token = *t;

              // If this token has an orderer (isn't orphaned) then check to see if we're 
              // more important than it is (in monetary terms) and if we took it out of the
              // loop we're have enough resources to buy what we need.
              Orderer *o = token->GetOrderer();
              if (o && (o != orderer))
              {
                // How much resource has this orderer used 
                if (orderer->GetPriority() < o->GetPriority())
                {
  //              LOG_AI(("'%s' [%d] is lower priority so we're going to axe it!", token->GetType().GetName(), token->GetType().GetResourceCost()))

                  // Modify this orderers usage
                  orderers.RemoveUsage(*o, token->GetType().GetResourceCost());

                  // Tell the token that its going back on the on order pile
                  token->Returned();

                  // Reclaim the cash
                  cash += token->GetType().GetResourceCost();

                  // Do we have enough cash now ?
                  if (cash >= (cost + orderer->GetReserveMinimum()))
                  {
                    break;
                  }
                }
              }
            }

            // After reclaiming do we have enough cash ?
            if (cash >= (cost + orderer->GetReserveMinimum()))
            {
              // Tell the token that it has been accepted
              token->Accepted();

              // Spend the cash
              spent = cost;

              ASSERT(cash >= cost)
              cash -= cost;

              // Modify this orderers usage
              orderers.AddUsage(*orderer, cost);

              return (OFFER_SPENT);
            }

            return (OFFER_SHORT);
          }
        }
      }
    }

    // Turn down the offer since none of the orderers wanted to spend the cash
    return (OFFER_TURNDOWN);
  }


  //
  // Enter
  //
  // Enter the state
  //
  void Base::State::Enter()
  {
    // Enter all of the orderers
    for (NList<Orderer>::Iterator o(&ordererList); *o; o++)
    {
      (*o)->Enter();
    }
  }


  //
  // Leave
  //
  // Leave the state
  //
  void Base::State::Leave()
  {
    // Leave all of the orderers
    for (NList<Orderer>::Iterator o(&ordererList); *o; o++)
    {
      (*o)->Leave();
    }
  }



  //
  // Set the weighting of one of the orderers
  //
  void Base::State::SetOrdererWeighting(const GameIdent &ordererName, U32 weighting)
  {
    // Find the orderer with this name
    Orderer *orderer = orderers.Find(ordererName.crc);

    if (orderer)
    {
//      LOG_AI(("Modifying Orderer '%s' to %d", ordererName.str, weighting))
      orderers.ModifyItemWeighting(*orderer, weighting);
    }
  }


  //
  // Set the priorit of one of the orderers
  //
  void Base::State::SetOrdererPriority(const GameIdent &ordererName, U32 priority)
  {
    // Find the orderer with this name
    Orderer *orderer = orderers.Find(ordererName.crc);

    if (orderer)
    {
//      LOG_AI(("Modifying Orderer '%s' to %d", ordererName.str, priority))
      orderers.ModifyItemPriority(*orderer, priority);
    }
  }


  
  //
  // An orderer has become active
  //
  void Base::State::ActivateOrderer(Orderer &orderer)
  {
//    LOG_AI(("Base::Orderer '%s' becoming active", orderer.GetName()))

    // How many orderers are active
    if (!orderers.GetActive().GetCount())
    {
      // Tell the base manager to activate this base
      base.GetManager().ActivateBase(base);
    }

    // Activate the orderer
    orderers.ActivateItem(orderer);
  }


  //
  // An orderer has become idle
  //
  void Base::State::DeactivateOrderer(Orderer &orderer)
  {
//    LOG_AI(("Base::Orderer '%s' becoming idle", orderer.GetName()))

    // Deactivate the orderer
    orderers.DeactivateItem(orderer);

    // How many orderers are active
    if (!orderers.GetActive().GetCount())
    {
      // Tell the base manager to activate this base
      base.GetManager().DeactivateBase(base);
    }
  }


}
