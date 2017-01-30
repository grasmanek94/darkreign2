///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Generic State Machine
//
// 10-SEP-1998
//


#ifndef __STATE_H
#define __STATE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "utiltypes.h"
#include "stdload.h"


//
// State processing notifications (TYPE independent)
//
enum StateMachineNotify { SMN_ENTRY, SMN_PROCESS, SMN_EXIT };


///////////////////////////////////////////////////////////////////////////////
//
// Template StateMachine
//
template <class TYPE> class StateMachine
{
private:


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class State
  //
  class State
  {
  public:

    // Function call back typedefs
    typedef void (TYPE::*NormalCallBack)();
    typedef void (TYPE::*NotifyCallBack)(StateMachineNotify notify);

    // State type
    enum Types { ST_NORMAL, ST_NOTIFY } stateType;

  private:

    // State Identifier
    GameIdent ident;

    // Type specific data
    union 
    {
      struct
      {
        NormalCallBack process;
      } normal;

      struct
      {
        NotifyCallBack process;
      } notify;
    };

  public:

    // Normal constructor
    State(const char *name, NormalCallBack callBack) : ident(name)
    {
      stateType = ST_NORMAL;
      normal.process = callBack;
    }

    // Notify constructor
    State(const char *name, NotifyCallBack callBack) : ident(name) 
    {
      stateType = ST_NOTIFY;
      notify.process = callBack;
    }
  
    // Peform state processing
    void Process(TYPE *type, StateMachineNotify n)
    {
      switch (stateType)
      {
        case ST_NORMAL:
          if (n == SMN_PROCESS)
          {
            (type->*normal.process)();
          }
          break;

        case ST_NOTIFY:
          (type->*notify.process)(n);
          break;
      }
    }

    // Return the identifier name
    const char *GetName()
    {
      return (ident.str);
    }

    // Returh the identifier CRC
    U32 GetNameCrc()
    {
      return (ident.crc);
    }

  };

  // All possible states
  BinTree<State> states;

public:


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Instance
  //
  class Instance
  {
  private:

    // State machine used for this instance
    StateMachine *machine;

    // Runtime state management
    State *currentState;
    State *nextState;
    Bool initializing;

  public:

    // Constructor
    Instance(StateMachine *machine, const char *stateName = NULL) : machine(machine)
    { 
      currentState = NULL;
      nextState = NULL;
      initializing = FALSE;

      if (stateName)
      {
        Set(stateName);
        currentState = nextState;
        initializing = TRUE;
      }
    }

    // Set the current state
    void Set(State *newState)
    {
      // Will occur if an unknown state name is used
      if (!newState)
      {
        ERR_FATAL(("State machine instance passed a NULL state"));
      }

      // Set the next state
      nextState = newState;
    }

    // Set the current state
    void Set(U32 stateCrc)
    {
      Set(machine->FindState(stateCrc));
    }

    // Set the current state
    void Set(const char *stateName)
    {
      Set(machine->FindState(stateName));
    }

    // Test the current state
    Bool Test(U32 stateCrc)
    {
      return (currentState == machine->FindState(stateCrc) ? TRUE : FALSE);
    }

    // Test the current state
    Bool Test(const char *stateName)
    {
      return (currentState == machine->FindState(stateName) ? TRUE : FALSE);
    }

    // Test the next state
    Bool TestNext(U32 stateCrc)
    {
      return (nextState && nextState == machine->FindState(stateCrc) ? TRUE : FALSE);
    }

    // Test the next state
    Bool TestNext(const char *stateName)
    {
      return (nextState && nextState == machine->FindState(stateName) ? TRUE : FALSE);
    }

    // Return pointer to the next state
    const State *NextState()
    {
      return (nextState);
    }

    // Process the current state
    void Process(TYPE *type)
    {
      ASSERT(currentState || nextState)

      // Do we want to go into another state ?
      if (nextState)
      {
        if (currentState && !initializing)
        {
          currentState->Process(type, SMN_EXIT);
        }
        initializing = FALSE;
        ASSERT(nextState)
        currentState = nextState;
        nextState = NULL;

        currentState->Process(type, SMN_ENTRY);
      }

      if (!nextState)
      {
        // Do normal processing
        currentState->Process(type, SMN_PROCESS);
      }
    }

    // Get the name of the current state
    const char * GetName()
    {
      if (currentState)
      {
        return (currentState->GetName());
      }
      else
      {
        return ("NULL");
      }
    }

    // Get the Crc of the current state
    U32 GetNameCrc()
    {
      ASSERT(currentState)
      return (currentState->GetNameCrc());
    }

    // Save the state
    void SaveState(FScope *scope)
    {
      if (currentState)
      {
        StdSave::TypeU32(scope, "CurrentState", currentState->GetNameCrc());
      }

      if (nextState)
      {
        StdSave::TypeU32(scope, "NextState", nextState->GetNameCrc());
      }

      if (initializing)
      {
        StdSave::TypeU32(scope, "Initializing", initializing);
      }
    }

    // Load the state
    void LoadState(FScope *scope)
    {
      FScope *sScope;

      // The state save assumes these are null
      currentState = NULL;
      nextState = NULL;
      initializing = FALSE;

      while ((sScope = scope->NextFunction()) != NULL)
      {
        switch (sScope->NameCrc())
        {
          case 0xFE325917: // "CurrentState"
            currentState = machine->FindState(StdLoad::TypeU32(sScope));
            break;

          case 0xDAA4E823: // "NextState"
            nextState = machine->FindState(StdLoad::TypeU32(sScope));
            break;

          case 0x37955420: // "Initializing"
            initializing = StdLoad::TypeU32(sScope);
            break;
        }
      }
    }
  };

  // Add a state to the state machine
  void AddState(const char *name, State::NormalCallBack process)
  {
    states.Add(Crc::CalcStr(name), new State(name, process));
  }

  // Add a state to the state machine
  void AddState(const char *name, State::NotifyCallBack process)
  {
    states.Add(Crc::CalcStr(name), new State(name, process));
  }

  // Return pointer to the state
  State *FindState(U32 crc)
  {
    State *state = states.Find(crc);
    
    if (!state)
    {
      ERR_FATAL(("Could not find state %08x", crc))
    }
    return (state);
  }

  // Return pointer to the state
  State *FindState(const char *name)
  {
    return (FindState(Crc::CalcStr(name)));
  }

  // Clean up the list of states
  void CleanUp()
  {
    states.DisposeAll();
  }
};


#endif