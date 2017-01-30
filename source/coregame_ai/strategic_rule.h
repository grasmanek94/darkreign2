/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Rule
//
// All rules store the result of the evaluation into a var specified
// in the rule configuration
//

#ifndef __STRATEGIC_RULE_H
#define __STRATEGIC_RULE_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic.h"
#include "strategic_script.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace RuleNotify
  //
  namespace RuleNotify
  {
    const U32 Completed = 0xFB6EF202; // "Rule::Notify::Competed"
    const U32 Failed    = 0xC3BA18C3; // "Rule::Notify::Failed"
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Rule
  //
  class Rule
  {
  public:

    /////////////////////////////////////////////////////////////////////////////
    //
    // Forward Declarations
    //
    class Manager;

  protected:

    // Rule manager which is managing this rule
    Manager &manager;

    // Manager nodes
    NList<Rule>::Node nodeManagerAll;
    NList<Rule>::Node nodeManager;

    // Pointer to the config scope
    FScope *config;

    // Script which is processing this rule
    Reaper<Script> script;

    // Variable which will have the result assigned to it
    VarInteger var;

    // The id of this rule
    U32 id;

  protected:

    // Constructor
    Rule(FScope *fScope, Manager &manager, Script *script);

  public:

    // Destructor
    virtual ~Rule();

    // Apply this rule
    void Apply();

    // Reset the rule
    virtual Bool Reset() = 0;

    // Continue evaluation of the rule, returns TRUE when completed
    virtual Bool Evaluate() = 0;

  public:

    // Get the script manager in control of this script
    Manager & GetManager()
    {
      return (manager);
    }

    // Get the script
    Script * GetScript()
    {
      return (script.GetPointer());
    }

    // Get the config scope
    FScope * GetConfig()
    {
      ASSERT(config)
      return (config);
    }

    // Get the id of this rule
    U32 GetId()
    {
      return (id);
    }

  public:

    // Create a rule from the given fscope
    static Rule * Create(FScope *fScope, Manager &manager, Script &script);

  public:

    // Friends
    friend class Manager;

  };

}

#endif
