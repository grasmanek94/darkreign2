/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Rule
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_rule_manager.h"
#include "strategic_object.h"


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


  //
  // Constructor
  //
  Rule::Manager::Manager()
  : allRules(&Rule::nodeManagerAll),
    rules(&Rule::nodeManager),
    ruleId(0)
  {

  }


  //
  // Destructor
  //
  Rule::Manager::~Manager()
  {
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Rule::Manager::SaveState(FScope *scope)
  {
    for (NList<Rule>::Iterator i(&rules); *i; ++i)
    {
      StdSave::TypeU32(scope, "AddRule", (*i)->GetId());
    }
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Rule::Manager::LoadState(FScope *scope)
  {
    FScope *sScope;

    // No rules should be added before loading
    ASSERT(!rules.GetCount())

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xAF2F5AEF: // "AddRule"
        {
          U32 id = StdLoad::TypeU32(sScope);

          if (Rule *rule = FindRule(id))
          {
            rule->Apply();
          }
          else
          {
            LOG_WARN(("Unable to resolve strategic rule! [%d]", id));
          }
          break;
        }
      }
    }
  }


  //
  // Process rule
  //
  void Rule::Manager::Process()
  {
    NList<Rule>::Iterator r(&rules);

    while (Rule *rule = r++)
    {
      if (!rule->Evaluate())
      {
        rules.Unlink(rule);
      }
    }
  }


  //
  // Add a rule
  //
  void Rule::Manager::Add(Rule &rule)
  {
    // The rule may already be in the list
    if (!rule.nodeManager.InUse())
    {
      rules.Append(&rule);
    }
  }


  //
  // Remove a rule
  //
  void Rule::Manager::Remove(Rule &rule)
  {
    // Remove the rule from the list if its in the list
    if (rule.nodeManager.InUse())
    {
      rules.Unlink(&rule);
    }
  }


  //
  // RuleConstruction
  //
  // Register construction/destruction of a rule
  //
  U32 Rule::Manager::RuleConstruction(Rule &rule)
  {
    allRules.Append(&rule);
    return (ruleId++);
  }


  //
  // RuleDestruction
  //
  // Register construction/destruction of a rule
  //
  void Rule::Manager::RuleDestruction(Rule &rule)
  {
    allRules.Unlink(&rule);
  }

  
  //
  // FindRule
  //
  // Find a rule using the given id
  //
  Rule * Rule::Manager::FindRule(U32 id)
  {
    for (NList<Rule>::Iterator i(&allRules); *i; ++i)
    {
      if ((*i)->GetId() == id)
      {
        return (*i);
      }
    }

    return (NULL);
  }
}
