/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Base
//


#ifndef __STRATEGIC_BASE_H
#define __STRATEGIC_BASE_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic.h"
#include "strategic_weighting.h"


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
  class Base : public Weighting::Item
  {
  public:

    /////////////////////////////////////////////////////////////////////////////
    //
    // Forward Declarations
    //
    class Manager;
    class Orderer;
    class State;
    class Token;
    class Recycler;

    enum Offer
    {
      OFFER_SPENT,
      OFFER_TURNDOWN,
      OFFER_SHORT
    };

  private:

    // Name of the base
    GameIdent name;

    // Name of the config
    GameIdent configName;

    // Base manager which is managing this base
    Manager &manager;

    // Manager Node
    NBinTree<Base>::Node nodeManager;

    // Cut node
    NBinTree<Base, F32>::Node nodeCut;

    // List node
    NList<Base>::Node nodeList;

    // Location of the base
    Point<F32> location;

    // Orientation of the base
    F32 orientation;

    // States
    NBinTree<State> states;

    // Current state
    State *currentState;

    // Flag indicating a new state
    Bool newState;

    // Is the list of units/idle constructors dirty ?
    Bool processTokens;

    // All of the units in the base
    UnitObjList units;

    // Armour class to automatically disrupt (-1 if not used)
    S32 autoDisrupt;

    // All of the distruptors in the base
    UnitObjList disruptors;

    // Index of disruptor list
    U32 disruptorIndex;

    // Last time we assigned a disruptor
    U32 disruptorCycle;

    // Idle constructors
    UnitObjList constructorsIdle;

    // Constructors which are manufacturing
    UnitObjList constructorsWorking;

    // List of all tokens
    NList<Token> allTokens;

    // Tokens which are searching for a spot
    NBinTree<Token> tokensSearching;

    // Tokens which are awaiting construction
    NBinTree<Token> tokensWaiting;

    // Tokens which are being constructed
    NBinTree<Token> tokensConstructing;

    // Tokens which are orphans tokens
    NList<Token> tokensOrphaned;

  public:

    // Death track info
    DTrack::Info dTrack;

  public:

    // Constructor
    Base(Manager &manager, const Point<F32> &location, F32 orientation, const char *name, const char *configName, FScope *fScope);

    // Destructor
    ~Base();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope, void *context);

    // Process the base
    void Process();

    // Get the strategic object who owns this base
    Object & GetObject();

    const Object & GetObject() const;

    // Offer cash to the base
    Offer OfferCash(U32 &cash, U32 &spent);

    // GotoState
    void GotoState(const GameIdent &state);

    // Set the weighting of one of the orderers
    void SetOrdererWeighting(const GameIdent &orderer, U32 weighting);

    // Set the weighting of one of the orderers
    void SetOrdererPriority(const GameIdent &orderer, U32 priority);

    // Assign constructors to this base
    void AssignConstructors(const char *tag);

    // Assign units to this base
    void AssignUnits(const char *tag);

    // Set auto disruption
    void AutoDisrupt(const char *armourClass);

    // Add a constructor
    void AddConstructor(UnitObj *constructor);

    // Add a unit
    void AddUnit(UnitObj *unit);

    // Remove a unit
    void RemoveUnit(UnitObj *unit);

    // Activate a constructor
    void ActivateConstructor(UnitObj *constructor);

    // Activate a constructor
    void ActivateConstructor(UnitObjListNode *constructor);

    // Deactivate a constructor
    void DeactivateConstructor(UnitObj *constructor);

    // Deactivate a constructor
    void DeactivateConstructor(UnitObjListNode *constructor);

    // Is this an active constructor
    UnitObjListNode * FindActiveConstructor(UnitObj *constructor);

    // Is this an inactivate constructor
    UnitObjListNode * FindInactiveConstructor(UnitObj *constructor);


    // Add orphaned token
    void AddOrphanedToken(Token *token);

    // Remove orphaned token
    void RemoveOrphanedToken(Token *token);


    // Register construction of a token
    void TokenConstruction(Token &token);

    // Register destruction of a token
    void TokenDestruction(Token &token);

    // Find the token with the given id
    Token * FindToken(U32 id);

  public:

    // Get the name of the base
    const char * GetName() const
    {
      return (name.str);
    }

    // Get the crc of the orderer base
    U32 GetNameCrc() const
    {
      return (name.crc);
    }

    // Get the game ident of the base
    const GameIdent & GetIdent() const
    {
      return (name);
    }

    // Get the orderer manager in control of this orderer
    Manager & GetManager()
    {
      return (manager);
    }

    // Get the location of the base
    const Point<F32> & GetLocation() const
    {
      return (location);
    }

    // Get the orientation of the base
    F32 GetOrientation() const
    {
      return (orientation);
    }

    // Set the process tokens flag
    void SetProcessTokens()
    {
      processTokens = TRUE;
    }

    // Get the current state
    State * GetCurrentState()
    {
      return (currentState);
    }

    // Get the number of units in the base
    U32 GetNumUnits() const
    {
      return (units.GetCount());
    }

    // Get the number of idle constructors
    U32 GetNumConstructorsIdle() const
    {
      return (constructorsIdle.GetCount());
    }

    // Get the number of working constructors
    U32 GetNumConstructorsWorking() const
    {
      return (constructorsWorking.GetCount());
    }

    // Get all of the units in the base
    const UnitObjList & GetUnits() const
    {
      return (units);
    }

    // Get all of the disruptors in the base
    const UnitObjList & GetDisruptors() const
    {
      return (disruptors);
    }

    // Get all of the idle constructors
    const UnitObjList & GetConstructorsIdle() const
    {
      return (constructorsIdle);
    }

    // Get all of the working constructors
    const UnitObjList & GetConstructorsWorking() const
    {
      return (constructorsWorking);
    }

    // Tokens which are searching for a spot
    const NBinTree<Token> & GetTokensSearching() const
    {
      return (tokensSearching);
    }

    // Tokens which are awaiting construction
    const NBinTree<Token> & GetTokensWaiting() const
    {
      return (tokensWaiting);
    }

    // Tokens which are being constructed
    const NBinTree<Token> & GetTokensConstructing() const
    {
      return (tokensConstructing);
    }

  public:

    // Friends
    friend class Manager;
    friend class Token;
    friend class Orderer;
    friend class State;

  };

}

#endif
