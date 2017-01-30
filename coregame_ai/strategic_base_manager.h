/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Base Manager
//


#ifndef __STRATEGIC_BASE_MANAGER_H
#define __STRATEGIC_BASE_MANAGER_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_base.h"
#include "strategic_notification.h"
#include "strategic_base_orderer_type.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Base::Manager
  //
  class Base::Manager
  {
  public:

    /////////////////////////////////////////////////////////////////////////////
    //
    // Struct Stats
    //
    struct Stats
    {
      // Constructor
      Stats();

      // Destructor
      ~Stats();

      // Amount of resource
      U32 resource;

      // Amount of power supply
      U32 powerDaySupply;
      U32 powerNightSupply;

      // Amount of power consumption
      U32 powerConsumption;

      // Save and load state
      void SaveState(FScope *scope);
      void LoadState(FScope *scope);
    };

  private:

    // Strategic Object the manager belongs to
    Object *strategic;

    // The id of the next token
    U32 tokenId;

    // Bases under the contorl of this manager
    Weighting::Group<Base> bases;

    // Primary base
    Base *primary;

    // Tokens which are handling notifications
    NBinTree<Token> handlers;

    // Should we take orders next time we process
    Bool takeOrders;

    // Types which are in the system
    NBinTree<Orderer::Type> types;

    // OnOrder stats
    Stats onOrder;

    // InConstruction stats
    Stats inConstruction;

    // Refund stats
    UnitObjList refunds;

    // List of all tokens
    NList<Token> allTokens;

    // Used to load the bases
    static Base * Loader(FScope *scope, NBinTree<Base> &tree, void *context);

  public:

    // Constructor and destructor
    Manager();
    ~Manager();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope);

    // Setup
    void Setup(Object *object);

    // Process orderers
    void Process();

    // Handle notification
    void Notify(Notification &notification);

  private:

    // Take orders
    void TakeOrders();

    // Process cuts
    Bool ProcessCuts(const Weighting::Cuts<Base> &cuts);

  public:

    // Add a base
    Base & AddBase(const GameIdent &base, const GameIdent &configName, U32 weighting, U32 priority, const Point<F32> &location, F32 orientation);

    // Remove a base
    void RemoveBase(const GameIdent &base);

    // Find a base
    Base * FindBase(const GameIdent &base);

    // Set base state
    void SetBaseState(const GameIdent &base, const GameIdent &state);

    // Set base weighting
    void SetBaseWeighting(const GameIdent &base, U32 weighting);

    // Set base priority
    void SetBasePriority(const GameIdent &base, U32 priority);

    // Set base orderer weighting
    void SetBaseOrdererWeighting(const GameIdent &base, const GameIdent &orderer, U32 weighting);

    // Set base orderer priority
    void SetBaseOrdererPriority(const GameIdent &base, const GameIdent &orderer, U32 priority);

    // Set the primary base
    void SetPrimaryBase(const GameIdent &base);

    // Assign constructors to a base
    void AssignBaseConstructors(const GameIdent &base, const char *tag);

    // Assign units to a base
    void AssignBaseUnits(const GameIdent &base, const char *tag);

    // Set auto disruption for a base
    void AutoDisrupt(const GameIdent &base, const char *armourClass);


    // A base has become active
    void ActivateBase(Base &base);

    // A base has become active
    void DeactivateBase(Base &base);


    // Register a token which is waiting for a notification
    void RegisterNotificationHandler(U32 id, Token &token);

    // Unregister a token which is waiting for a notification
    void UnregisterNotificationHandler(Token &token);

  private:

    // Add a type
    void AddType(UnitObjType &type);

    // Remove a type
    void RemoveType(UnitObjType &type);

    // Add a type to stats
    void AddStats(UnitObjType &type, Stats &stats);

    // Remoe a type from stats
    void RemoveStats(UnitObjType &type, Stats &stats);

  public:

    // Add a type to OnOrder
    void AddTypeOnOrder(UnitObjType &type);

    // Remove a type from OnOrder
    void RemoveTypeOnOrder(UnitObjType &type);

    // Remove a type from InConstruction
    void RemoveTypeInConstruction(UnitObjType &type);

    // Move a type from OnOrder to InConstruction
    void MoveTypeOnOrderToInConstruction(UnitObjType &type);

    // Move a type from InConstruction to OnOrder
    void MoveTypeInConstructionToOnOrder(UnitObjType &type);

    // Get the number of the given type in the system
    U32 GetTypeCount(UnitObjType &type);

    // Register construction of a token
    U32 TokenConstruction(Token &token);
    void TokenConstruction(Token &token, U32 id);

    // Register destruction of a token
    void TokenDestruction(Token &token);

    // Find the token with the given id
    Token * FindToken(U32 id);

    // Save a token bin tree
    void SaveTokenBinTree(const NBinTree<Token> &tree, FScope *scope);

    // Load a token bin tree
    void LoadTokenBinTree(NBinTree<Token> &tree, FScope *scope);

    // Save a token list
    void SaveTokenList(const NList<Token> &tree, FScope *scope);

    // Load a token list
    void LoadTokenList(NList<Token> &tree, FScope *scope, Orderer *orderer = NULL);
    
  public:

    // Get the strategic object from the manager
    Object & GetObject()
    {
      ASSERT(strategic)
      return (*strategic);
    }
    const Object & GetObject() const
    {
      ASSERT(strategic)
      return (*strategic);
    }

    // Set the take orders flag
    // (likely to be set when money is received or base/orderer priorities change)
    void SetTakeOrders()
    {
      takeOrders = TRUE;
    }

    // Get the OnOrder stats
    const Stats & GetOnOrderStats()
    {
      return (onOrder);
    }

    // Get the InConstruction stats
    const Stats & GetInConstructionStats()
    {
      return (inConstruction);
    }

    // Get the bases
    const Weighting::Group<Base> & GetBases()
    {
      return (bases);
    }

    // Get the primary base
    Base * GetPrimaryBase()
    {
      return (primary);
    }

    // Add a unit to the refund list
    void AddRefund(UnitObj *unitObj)
    {
      ASSERT(unitObj)
      refunds.Append(unitObj);
    }

    // Get the refund list
    const UnitObjList & GetRefunds()
    {
      refunds.PurgeDead();
      return (refunds);
    }

  };

}

#endif