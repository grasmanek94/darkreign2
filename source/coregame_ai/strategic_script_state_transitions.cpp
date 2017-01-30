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
#include "random.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace ScriptTransitions
  //
  namespace ScriptTransitions
  {

    /////////////////////////////////////////////////////////////////////////////
    //
    // Class End
    //
    class End : public Script::State::Transition
    {
    public:

      // Constructor
      End(FScope *fScope);

      // Perform the transition
      void Perform(Script &script);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class GotoState
    //
    class GotoState : public Script::State::Transition
    {
    private:

      // State to goto
      GameIdent state;

    public:

      // Constructor and Destructor
      GotoState(FScope *fScope);

      // Perform the transition
      void Perform(Script &script);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class GotoRandomState
    //
    class GotoRandomState : public Script::State::Transition
    {
    private:

      // States to goto
      GameIdentListWeighted states;

    public:

      // Constructor and Destructor
      GotoRandomState(FScope *fScope);
      ~GotoRandomState();

      // Perform the transition
      void Perform(Script &script);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class CallState
    //
    class CallState : public Script::State::Transition
    {
    private:

      // State to call
      GameIdent state;

    public:

      // Constructor and Destructor
      CallState(FScope *fScope);

      // Perform the transition
      void Perform(Script &script);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class CallRandomState
    //
    class CallRandomState : public Script::State::Transition
    {
    private:

      // States to call
      GameIdentListWeighted states;

    public:

      // Constructor and Destructor
      CallRandomState(FScope *fScope);
      ~CallRandomState();

      // Perform the transition
      void Perform(Script &script);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class ReturnState
    //
    class ReturnState : public Script::State::Transition
    {
    public:

      // Constructor and Destructor
      ReturnState(FScope *fScope);

      // Perform the transition
      void Perform(Script &script);

    };

  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Script::State::Transition
  //


  //
  // Create
  //
  // Create a transition from the given scope
  //
  Script::State::Transition * Script::State::Transition::Create(FScope *fScope)
  {
    FScope *sScope;
    sScope = fScope->NextFunction();

    if (sScope)
    {
      switch (sScope->NameCrc())
      {
        case 0xB2C53B91: // "End"
          return new ScriptTransitions::End(sScope);

        case 0xCDF555B0: // "GotoState"
          return new ScriptTransitions::GotoState(sScope);

        case 0xCC5F38AA: // "GotoRandomState"
          return new ScriptTransitions::GotoRandomState(sScope);

        case 0xB90E4B66: // "CallState"
          return new ScriptTransitions::CallState(sScope);

        case 0xF25B6891: // "CallRandomState"
          return new ScriptTransitions::CallRandomState(sScope);

        case 0x09CA610D: // "ReturnState"
          return new ScriptTransitions::ReturnState(sScope);

        default:
          sScope->ScopeError("Unknown transition type '%s'", sScope->NameStr());
      }
    }
    else
    {
      fScope->ScopeError("Expected a transition");
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace ScriptTransitions
  //
  namespace ScriptTransitions
  {

    /////////////////////////////////////////////////////////////////////////////
    //
    // Class End
    //

    //
    // Constructor
    //
    End::End(FScope *fScope)
    {
      FSCOPE_DIRTY(fScope)
    }


    //
    // Perform the transition
    //
    void End::Perform(Script &script)
    {
      script.End();
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class GotoState
    //


    //
    // Constructor
    //
    GotoState::GotoState(FScope *fScope)
    {
      state = StdLoad::TypeString(fScope);
    }


    //
    // Perform the transition
    //
    void GotoState::Perform(Script &script)
    {
      script.GotoState(state);
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class GotoRandomState
    //


    //
    // Constructor
    //
    GotoRandomState::GotoRandomState(FScope *fScope)
    {
      // Load the states
      states.Load(fScope);

      // There's got to be at least 1 state in there
      if (!states.GetCount())
      {
        fScope->ScopeError("At least one state is required in GotoRandomState");
      }
    }


    //
    // Destructor
    //
    GotoRandomState::~GotoRandomState()
    {
      states.DisposeAll();
    }


    //
    // Perform the transition
    //
    void GotoRandomState::Perform(Script &script)
    {
      // Pick a random state
      script.GotoState(states.RandomSync());
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class CallState
    //


    //
    // Constructor
    //
    CallState::CallState(FScope *fScope)
    {
      state = StdLoad::TypeString(fScope);
    }


    //
    // Perform the transition
    //
    void CallState::Perform(Script &script)
    {
      script.CallState(state);
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class CallRandomState
    //


    //
    // Constructor
    //
    CallRandomState::CallRandomState(FScope *fScope)
    {
      // Load the states
      states.Load(fScope);

      // There's got to be at least 1 state in there
      if (!states.GetCount())
      {
        fScope->ScopeError("At least one state is required in CallRandomState");
      }
    }


    //
    // Destructor
    //
    CallRandomState::~CallRandomState()
    {
      states.DisposeAll();
    }


    //
    // Perform the transition
    //
    void CallRandomState::Perform(Script &script)
    {
      // Pick a random state
      script.CallState(states.RandomSync());
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class ReturnState
    //

    //
    // Constructor
    //
    ReturnState::ReturnState(FScope *)
    {
    }


    //
    // Perform the transition
    //
    void ReturnState::Perform(Script &script)
    {
      script.ReturnState();
    }
  }
}
