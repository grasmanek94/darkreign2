/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Placement
//


#ifndef __STRATEGIC_PLACEMENT_H
#define __STRATEGIC_PLACEMENT_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic.h"
#include "offmapobj.h"
#include "strategic_base_token.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Placement
  //
  class Placement
  {
  public:

    /////////////////////////////////////////////////////////////////////////////
    //
    // Forward Declarations
    //
    class Manager;
    class Cluster;
    class ClusterSet;
    class Locator;
    class RuleSet;
    struct TypeClusterSet;

  protected:

    // Placement manager
    Manager &manager;

  private:

    // Locators by token
    NBinTree<Locator> locatorsIdle;
    NBinTree<Locator> locatorsActive;

    // Active locator list
    NList<Locator> locatorsActiveList;

    // Active locator iterator
    NList<Locator>::Iterator locatorsActiveIterator;

    // Manager node
    NBinTree<Placement>::Node nodeManager;

    // Ruleset
    RuleSet &ruleSet;

    // Name of the placement
    GameIdent name;

    // Name of the fallback placement
    GameIdent fallback;

  public:

    // Constructor
    Placement(Manager &manager, const GameIdent &name, FScope *fScope);

    // Destructor
    ~Placement();


    // Find a location for a token
    void FindLocation(Base::Token &token);

    // Find the next location for a token
    void FindNextLocation(Base::Token &token);

    // Abort finding a location for a token
    void AbortFind(Base::Token &token);

    // Remove the token from the system
    void RemoveToken(Base::Token &token);

    // Process this placement
    void Process();

    // Get the fallback placement
    Placement * GetFallback();

  public:

    // Get the script manager in control of this script
    Manager & GetManager()
    {
      return (manager);
    }

    // Get the name of this placement
    const GameIdent & GetName()
    {
      return (name);
    }

  public:

    // Friends
    friend class Manager;

  };

}

#endif
