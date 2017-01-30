///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "tasks_unitidle.h"
#include "random.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // State machine
  StateMachine<UnitIdle> UnitIdle::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitIdle
  //


  //
  // Constructor
  //
  UnitIdle::UnitIdle(GameObj *subject) :
    GameTask<UnitObjType, UnitObj>(staticConfig, subject),
    inst(&stateMachine, "Init"),
    animationCrc(0)
  {
  }


  //
  // Save
  //
  void UnitIdle::Save(FScope *fScope)
  {
    // Only save state if we're active
    if (subject->GetCurrentTask() == this)
    {
      SaveTaskData(fScope);
      inst.SaveState(fScope->AddFunction("StateMachine"));
      timer.SaveState(fScope->AddFunction("Timer"));
      StdSave::TypeU32(fScope, "AnimationCrc", animationCrc);
    }
  }


  //
  // Load
  //
  void UnitIdle::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
          break;

        case 0x5AB44811: // "Timer"
          timer.LoadState(sScope);
          break;

        case 0xA7DAA62B: // "AnimationCrc"
          animationCrc = StdLoad::TypeU32(sScope);
          break;

        default:
          LoadTaskData(sScope);
          break;
      }
    }
  }


  //
  // Called after all objects are loaded
  //
  void UnitIdle::PostLoad()
  {
  }


  //
  // Perform task processing
  //
  Bool UnitIdle::Process()
  {
    inst.Process(this);  

    // Perform type processing
    GameProcess();

    return (FALSE);
  }


  //
  // Initial state
  //
  void UnitIdle::StateInit()
  {
    NextState(0x0D33B8A8); // "Time"
  }


  //
  // Choose a time to play the idle animation
  //
  void UnitIdle::StateTime()
  {
    // How long until we play the idle animation
    timer.Start(Random::sync.Float() * 15.0F + 5.0F);

    // Proceed
    NextState(0x793360CC); // "Idle"
  }


  //
  // Idle state
  //
  void UnitIdle::StateIdle()
  {
    if (timer.Test())
    {
      GameIdent idleName;

      // Pick a random idle animation
      if (U32 index = Random::sync.Integer(subject->MapType()->GetIdleAnimationCount()))
      {
        Utils::Sprintf(idleName.str, idleName.GetSize(), "Idle%d", index + 1);
        idleName.Update();
      }
      else
      {
        idleName = "Idle";
      }

      // Start the animation
      subject->SetAnimation(animationCrc = idleName.crc);

      // Choose next time
      NextState(0x0D33B8A8); // "Time"
    }
  }


  //
  // Notify this task of an event
  //
  Bool UnitIdle::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
        NextState(0xABAA7B48); // "Init"
        return (TRUE);

      case MapObjNotify::AnimationDone:
      {
        // Do we need to return to the default animation
        if (event.param1 == animationCrc)
        {
          subject->SetAnimation(0x8F651465); // "Default"
        }

        return (TRUE);
      }
    }
    return (GameTask<UnitObjType, UnitObj>::ProcessEvent(event));
  }


  //
  // Initialization
  //
  void UnitIdle::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Add states to the state machine
    stateMachine.AddState("Init", &UnitIdle::StateInit);
    stateMachine.AddState("Time", &UnitIdle::StateTime);
    stateMachine.AddState("Idle", &UnitIdle::StateIdle);
  }


  //
  // Shutdown
  //
  void UnitIdle::Done()
  {
    stateMachine.CleanUp();
  }

}
