/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Script
// 25-MAR-1999
//


#ifndef __STRATEGIC_SCRIPT_STATE_H
#define __STRATEGIC_SCRIPT_STATE_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_script.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Status
  //
  namespace Status
  {
    const U32 Ignored   = 0xD560452A; // "Ignored"
    const U32 Completed = 0x3369B9CC; // "Completed"
    const U32 Partial   = 0xABC0969F; // "Partial"
    const U32 Failed    = 0x788D2AC3; // "Failed"

  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Script::State
  //
  class Script::State
  {
  public:

    /////////////////////////////////////////////////////////////////////////////
    //
    // Forward Declarations
    //
    class Setting;
    class Action;
    class Condition;
    class Transition;

  private:

    // Script this state belongs to
    Script &script;

    // Name of this state
    GameIdent name;

    // Settings to use in this state
    NList<Setting> settings;

    // Action to be performed in this state
    Action *action;

    // Conditions to be tested in this state
    NList<Condition> conditions;

    // Transitions table
    NBinTree<Transition> transitions;

    // Completed transtion (is referenced too often to afford lookup)
    Transition *completed;

  public:

    // Script Node
    NBinTree<State>::Node nodeScript;

    // Constructor
    State(Script &script, const char *name);
    State(Script &script, const char *name, Action *action);
    State(Script &script, const char *name, FScope *fScope);

    // Destructor
    ~State();

    // Entering the state
    void Enter();

    // Process the state, flag indicates that an action completed
    void Process();

    // Notify
    void Notify(U32 message, U32 data);

    /*
    // Install a condition
    void InstallCondition(const GameIdent &ident, State::Condition *condition);

    // UnInstall a condition
    void UnInstallCondition(const GameIdent &ident);

    // CheckCondition
    Bool CheckCondition(const GameIdent &ident);

    // Purge conditions
    void PurgeConditions();
    */

  public:

    // Get the name of the state
    const char * GetName()
    {
      return (name.str);
    }

    // Get the Script that owns this state
    Script & GetScript()
    {
      return (script);
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Script::State::Setting
  //
  class Script::State::Setting
  {
  protected:

    // Script which owns this setting
    Script &script;

  public:

    // State node
    NList<Setting>::Node nodeState;

    // Apply the settings
    virtual void Apply() = 0;

  public:

    // Constructor
    Setting(Script &script)
    : script(script)
    {
    }

    // Destructor
    virtual ~Setting() 
    { 
    };

  public:

    // Get the state this setting belongs to
    Script & GetScript()
    {
      return (script);
    }

  public:

    // Create a settings from the given fscope
    static Setting * Create(Script &script, FScope *fScope);

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Script::State::Action
  //
  class Script::State::Action
  {
  protected:

    // Script which owns this action
    Script &script;

  public:

    // Constructor and Destructor
    Action(Script &script) 
    : script(script)
    { 
    }

    virtual ~Action() 
    { 
    }

    // Execute the actions
    virtual void Execute() = 0;

    // Translate the notification
    virtual U32 Notify(U32 message, U32 data) = 0;

  public:

    // Get the script this action belongs to
    Script & GetScript()
    {
      return (script);
    }

  public:

    // Create an action from the given fscope
    static Action * Create(Script &script, FScope *fScope);

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Script::State::Condition
  //
  class Script::State::Condition
  {
  private:

    // Transitions to perform when the condition is met
    Transition *transition;

    // List of sub conditions which are tested if this condition was met
    NList<Condition> conditions;

  protected:

    // Script this condition belongs to
    Script &script;

  public:

    // State node
    NList<Condition>::Node nodeState;

    // Constructor and Destructor
    Condition(Script &script, FScope *fScope);
    virtual ~Condition();

    // Reset the condition
    virtual void Reset();

    // Test the condition
    virtual Bool Test() = 0;

    // The test has succeeded
    Bool Success();

  public:

    // Get the script this condition belongs to
    Script & GetScript()
    {
      return (script);
    }

  public:

    // Create a condition from the given fscope
    static Condition * Create(Script &script, FScope *fScope);

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Script::State::Transition
  //
  class Script::State::Transition
  {
  public:

    // Transition node
    NBinTree<Transition>::Node nodeState;

  public:

    // Destructor
    virtual ~Transition() 
    { 
    }

    // Perform the transition
    virtual void Perform(Script &script) = 0;

  public:

    // Create a transition from the given scope
    static Transition * Create(FScope *fScope);

  };

}

#endif
