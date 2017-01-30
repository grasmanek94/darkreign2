/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Script
// 25-MAR-1999
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_script_state.h"
#include "strategic_object.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Script::State
  //


  //
  // Constructor
  //
  Script::State::State(Script &script, const char *name)
  : script(script),
    settings(&Setting::nodeState),
    conditions(&Condition::nodeState),
    transitions(&Transition::nodeState),
    completed(NULL),
    name(name),
    action(NULL)
  {
  }


  //
  // Constructor
  //
  Script::State::State(Script &script, const char *name, Action *action)
  : script(script),
    settings(&Setting::nodeState),
    conditions(&Condition::nodeState),
    transitions(&Transition::nodeState),
    completed(NULL),
    name(name),
    action(action)
  {
  }


  //
  // Constructor
  //
  Script::State::State(Script &script, const char *name, FScope *fScope)
  : script(script),
    settings(&Setting::nodeState),
    conditions(&Condition::nodeState),
    transitions(&Transition::nodeState),
    name(name)
  {
    FScope *sScope;
    FScope *iScope;

    // Load the settings
    iScope = fScope->GetFunction("Settings", FALSE);
    if (iScope)
    {
      while ((sScope = iScope->NextFunction()) != NULL)
      {
        switch (sScope->NameCrc())
        {
          case 0x32BBA19C: // "Setting"
            settings.Append(Setting::Create(script, sScope));
            break;

          default:
            sScope->ScopeError("Unknown function '%s' in settings", sScope->NameStr());
        }
      }
    }

    // Load the action
    iScope = fScope->GetFunction("Action", FALSE);
    if (iScope)
    {
      action = Action::Create(script, iScope);
    }
    else
    {
      action = NULL;
    }

    // Load the conditions
    iScope = fScope->GetFunction("Conditions", FALSE);

    if (iScope)
    {
      while ((sScope = iScope->NextFunction()) != NULL)
      {
        switch (sScope->NameCrc())
        {
          case 0x6A34146A: // "Condition"
            conditions.Append(Condition::Create(script, sScope));
            break;

          case 0xFBCD164A: // "Status"
          {
            GameIdent code = StdLoad::TypeString(sScope);

            if (transitions.Exists(code.crc))
            {
              sScope->ScopeError("Status '%s' already defined", code.str);
            }

            // Add the new transition
            transitions.Add(code.crc, Transition::Create(sScope));
            break;
          }

          default:
            sScope->ScopeError("Unknown function '%s' in conditions", sScope->NameStr());
        }
      }
    }

    completed = transitions.Find(Status::Completed);
  }


  //
  // Destructor
  //
  Script::State::~State()
  {
    // Delete the settings
    settings.DisposeAll();

    // Delete the action if there was one
    if (action)
    {
      delete action;
    }

    // Delete the conditions
    conditions.DisposeAll();

    // Delete the transitions
    transitions.DisposeAll();
  }


  //
  // Entering the state
  //
  void Script::State::Enter()
  {
    // Apply settings
    for (NList<Setting>::Iterator s(&settings); *s; s++)
    {
      (*s)->Apply();
    }

    // Reset conditions
    for (NList<Condition>::Iterator c(&conditions); *c; c++)
    {
      (*c)->Reset();
    }
    
    // If there's an action then execute it
    if (action)
    {
      action->Execute();
    }
  }


  //
  // Process the state
  //
  void Script::State::Process()
  {
    // If there is no action and there's a transition then we follow it
    if (!action && completed)
    {
      completed->Perform(GetScript());
    }
    else

    // Process the conditions
    for (NList<Condition>::Iterator c(&conditions); *c; c++)
    {
      if ((*c)->Test())
      {
        if ((*c)->Success())
        {
          break;
        }
      }
    }
  }


  //
  // Notification
  //
  void Script::State::Notify(U32 message, U32 data)
  {
    // Ask the action to translate this notification
    if (action)
    {
      U32 translation = action->Notify(message, data);

      // Was the notification ignored ?
      if (translation != Status::Ignored)
      {
        // Find the transition
        Transition *transition = transitions.Find(translation);

        if (transition)
        {
          // Perform the transition
          transition->Perform(GetScript());
        }
      }
    }
  }
}
