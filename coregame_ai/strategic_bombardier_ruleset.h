/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Bombardier
//


#ifndef __STRATEGIC_BOMBARDIER_RULESET_H
#define __STRATEGIC_BOMBARDIER_RULESET_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_bombardier.h"
#include "damage.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Bombardier::RuleSet
  //
  // RuleSet for bombardiers
  //
  class Bombardier::RuleSet
  {
  private:

    class Rule;

    // Manager node
    NBinTree<RuleSet>::Node nodeManager;

    // Rules
    NList<Rule> rules;

    // Damage table
    const Damage::Type *damage;

  public:

    // Constructor
    RuleSet(FScope *fScope);

    // Destructor
    ~RuleSet();

    // For the given unit type/team, evaluate the cluster using the ruleset
    Bool Evaluate
    (
      F32 &score,
      MapCluster &cluster, 
      Team &team, 
      const Point<F32> *pos = NULL
    );
    
  public:

    friend Manager;

  };

}

#endif