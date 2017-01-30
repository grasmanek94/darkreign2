/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Rule
// 25-MAR-1999
//


#ifndef __STRATEGIC_RULE_MANAGER_H
#define __STRATEGIC_RULE_MANAGER_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_rule.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Rule::Manager
  //
  class Rule::Manager
  {
  private:

    // Strategic Object the manager belongs to
    Object *strategic;

    // List of all rules belonging to this manager
    NList<Rule> allRules;

    // List of active rules
    NList<Rule> rules;

    // The id of the next rule
    U32 ruleId;

  public:

    // Constructor and destructor
    Manager();
    ~Manager();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope);

    // Process rules
    void Process();

    // Add a rule
    void Add(Rule &rule);

    // Remove a rule
    void Remove(Rule &rule);

    // Register construction/destruction of a rule
    U32 RuleConstruction(Rule &rule);
    void RuleDestruction(Rule &rule);

    // Find a rule using the given id
    Rule * FindRule(U32 id);

  public:

    // Setup
    void Setup(Object *object)
    {
      strategic = object;
    }

    // Get the strategic object from the manager
    Object & GetObject()
    {
      ASSERT(strategic)
      return (*strategic);
    }

  };

}

#endif
