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
#include "tasks_unitanimate.h"
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
  StateMachine<UnitAnimate> UnitAnimate::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitIdle
  //


  //
  // Constructor
  //
  UnitAnimate::UnitAnimate(GameObj *subject) :
    GameTask<UnitObjType, UnitObj>(staticConfig, subject),
    inst(&stateMachine, "Init"),
    cycleId(0)
  {
  }


  //
  // Constructor
  //
  UnitAnimate::UnitAnimate(GameObj *subject, U32 cycle) :
    GameTask<UnitObjType, UnitObj>(staticConfig, subject),
    inst(&stateMachine, "Init"),
    cycleId(cycle)
  {
  }


  //
  // Save
  //
  void UnitAnimate::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));  
    StdSave::TypeU32(fScope, "CycleId", cycleId);
  }


  //
  // Load
  //
  void UnitAnimate::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
          break;

        case 0x928CDC71: // "CycleId"
          cycleId = StdLoad::TypeU32(sScope);
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
  void UnitAnimate::PostLoad()
  {
  }


  //
  // Perform task processing
  //
  Bool UnitAnimate::Process()
  {
    inst.Process(this);  

    // Perform type processing
    GameProcess();

    return (FALSE);
  }


  //
  // Initial state
  //
  void UnitAnimate::StateInit()
  {
    NextState(0xFAA22138); // "Animate"
  }


  //
  // Play animation state
  //
  void UnitAnimate::StateAnimate(StateMachineNotify notify)
  {
    switch (notify)
    {
      case SMN_ENTRY:
      {
        if (cycleId)
        {
          subject->SetAnimation(cycleId);
        }
        break;
      }
    }
  }


  //
  // Notify this task of an event
  //
  Bool UnitAnimate::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
        NextState(0xABAA7B48); // "Init"
        return (TRUE);

      case MapObjNotify::AnimationDone:
      {
        // Do we need to return to the default animation
        if (event.param1 == cycleId)
        {
          Quit();
        }

        return (TRUE);
      }
    }
    return (GameTask<UnitObjType, UnitObj>::ProcessEvent(event));
  }


  //
  // Initialization
  //
  void UnitAnimate::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Add states to the state machine
    stateMachine.AddState("Init", &UnitAnimate::StateInit);
    stateMachine.AddState("Animate", &UnitAnimate::StateAnimate);
  }


  //
  // Shutdown
  //
  void UnitAnimate::Done()
  {
    stateMachine.CleanUp();
  }
}
