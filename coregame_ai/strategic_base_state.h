/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Base State
//


#ifndef __STRATEGIC_BASE_STATE_H
#define __STRATEGIC_BASE_STATE_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_base.h"
#include "strategic_base_orderer.h"
#include "strategic_weighting.h"


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
  class Base::State
  {
  public:

    // Name of the state
    GameIdent name;

    // Name of the state to go to if the orderers are satisfied
    GameIdent stateSatisfied;

    // Only proceed to the satisfied state if we're idle
    Bool idle : 1,

    // Do we have satisfaction (don't want to order anything else)
         satisfied : 1;

    // Base which this state belongs to
    Base &base;

    // Base Node
    NBinTree<State>::Node nodeBase;

    // Weighting group which contains the orderers
    Weighting::Group<Orderer> orderers;

    // The recyclers
    NList<Recycler> recyclerList;

    // List of all orderers
    NList<Orderer> ordererList;

    // Iterator to orderer processing
    NList<Orderer>::Iterator ordererIter;

  public:

    // Constructor
    State(Base &base, const char *name, FScope *fScope);

    // Destructor
    ~State();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope);

    // Process the state
    void Process();

    // Offer cash to the state
    Offer OfferCash(U32 &cash, U32 &spent);

    // Offer cash to a list of Orderers
    Offer OfferCash(U32 &cash, U32 &spent, const NBinTree<Orderer, F32> &orderers);

    // Enter the state
    void Enter();

    // Leave the state
    void Leave();

    // Set the weighting of one of the orderers
    void SetOrdererWeighting(const GameIdent &orderer, U32 weighting);

    // Set the priority of one of the orderers
    void SetOrdererPriority(const GameIdent &orderer, U32 priority);

    // An orderer has become active
    void ActivateOrderer(Orderer &orderer);

    // An orderer has become idle
    void DeactivateOrderer(Orderer &orderer);

  public:

    // Get the name of the state
    const char * GetName()
    {
      return (name.str);
    }

    // Get the crc of the orderer state
    U32 GetNameCrc()
    {
      return (name.crc);
    }

    // Get the game ident of the state
    const GameIdent & GetIdent()
    {
      return (name);
    }

    // Get the base which owns this state
    Base & GetBase()
    {
      return (base);
    }

    // Get the orderers within this state
    Weighting::Group<Orderer> & GetOrderers()
    {
      return (orderers);
    }

  public:

    // Friends
    friend class Base;

  };

}

#endif
