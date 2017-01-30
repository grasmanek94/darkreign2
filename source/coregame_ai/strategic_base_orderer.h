/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Orderer
// 25-MAR-1999
//


#ifndef __STRATEGIC_BASE_ORDERER_H
#define __STRATEGIC_BASE_ORDERER_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_base.h"
#include "strategic_weighting.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Base::Orderer
  //
  class Base::Orderer : public Weighting::Item
  {
  public:

    /////////////////////////////////////////////////////////////////////////////
    //
    // Forward Declarations
    //
    class Manifest;
    struct Type;

  private:

    // Name of the orderer
    GameIdent name;

    // Base state which this orderer belongs to
    Base::State &state;

    // The manifest this orderer is using
    Manifest *manifest;

    // Base state node
    NBinTree<Orderer>::Node nodeBaseState;

    // List node
    NList<Orderer>::Node nodeList;

    // Cut node
    NBinTree<Orderer, F32>::Node nodeCut;

    // Types which are in the system
    NBinTree<Type> types;

    // Tokens which are on order
    NList<Token> onOrder;

    // Tokens which are in the construction system
    NList<Token> inConstruction;

    // Units on the field constructed by this token
    UnitObjList onField;

    // Priority of this orderer
    U32 priority;

    // Maximum and minimum reserves
    U32 reserveMaximum;
    U32 reserveMinimum;

    // Is this orderer online ?
    Bool online;

    // When was the last time we processed the manifest
    U32 lastTime;

  public:

    // Constructor
    Orderer(Base::State &state, const char *name, U32 priority, FScope *fScope);

    // Destructor
    ~Orderer();

    // Save and load state
    void Save(FScope *scope);
    void Load(FScope *scope);

    // Enter the orderer
    void Enter();

    // Process the orderer
    Bool Process();

    // Leave the orderer
    void Leave();

    // Get the strategic object who owns this orderer
    Object & GetObject();


    // Submit and order for a particular type
    void SubmitToken(Token *token);

    // Return an order for a particular type
    void ReturnToken(Token *token);

    // Token was used
    void UsedToken(Token *token);

    // Token was completed
    void CompletedToken(Token *token, UnitObj *unit);

    // What does this orderer want (if anything)
    Token * WhatToOrder();

    // Take the next item which is order
    void TakeToken();

    // GetPlacement
    const GameIdent & GetPlacement();

    // Reshuffle
    void Reshuffle();

    // Are we meant to block if we don't get our prereqs
    Bool NoPrereqStall();

    // Are we not meant to build beyond our weighting
    Bool NoBeyondWeighting();

  public:

    // Get the name of the orderer
    const char * GetName()
    {
      return (name.str);
    }

    // Get the crc of the orderer name
    U32 GetNameCrc()
    {
      return (name.crc);
    }

    // Get the game ident of the orderer
    const GameIdent & GetIdent()
    {
      return (name);
    }

    // Get the base state which is in control of this orderer
    Base::State & GetState()
    {
      return (state);
    }

    // Get the priority of this orderer
    U32 GetPriority() const
    {
      return (priority);
    }

    // Get the types tree
    const NBinTree<Type> & GetTypesOnOrder() const
    {
      return (types);
    }

    // Get the tokens on order
    const NList<Token> & GetTokensOnOrder() const
    {
      return (onOrder);
    }

    // Get the tokens in construction
    const NList<Token> & GetTokensInConstruction() const
    {
      return (inConstruction);
    }

    // Get the units on the field
    const UnitObjList & GetUnitsOnField()
    {
      onField.PurgeDead();
      return (onField);
    }

    // Get the minimum amount this orderer wants to reserve
    // (don't spend money till this amount of money exists)
    U32 GetReserveMinimum() const
    {
      return (reserveMinimum);
    }

    // Get the maximum amount this orderer wants to reserve
    // (don't spend money if this amount of money exists)
    U32 GetReserveMaximum() const
    {
      return (reserveMaximum);
    }

  public:

    // Friends
    friend class Base::State;

  };

}



#endif
