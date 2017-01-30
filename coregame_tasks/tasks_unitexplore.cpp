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
#include "tasks_unitexplore.h"
#include "taskutil.h"
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
  StateMachine<UnitExplore> UnitExplore::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Constructor
  //


  //
  // Constructor
  //
  UnitExplore::UnitExplore(GameObj *sub) 
  : GameTask<UnitObjType, UnitObj>(staticConfig, sub),
    inst(&stateMachine, "Init")
  {
  }


  //
  // Save
  //
  void UnitExplore::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));
    moveHandle.SaveState(fScope->AddFunction("MoveHandle"));
    StdSave::TypeU32(fScope, "HangUntil", hangUntil);
  }


  //
  // Load
  //
  void UnitExplore::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
          break;

        case 0xD3D101D2: // "MoveHandle"
          moveHandle.LoadState(sScope);
          break;

        case 0x7BCDC81D: // "HangUntil"
          hangUntil = StdLoad::TypeU32(sScope);
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
  void UnitExplore::PostLoad()
  {
  }


  //
  // Perform task processing
  //
  Bool UnitExplore::Process()
  {
    inst.Process(this);
    GameProcess();
    return (quit);
  }


  //
  // StateInit
  //
  void UnitExplore::StateInit()
  {
    // Find an unexplored point and go to it
    Vector location = subject->RootOrigin();
    Point<S32> cell(subject->GetCellX(), subject->GetCellZ());
    TaskUtil::FindUnexplored(location, cell, subject->UnitType()->GetTractionIndex(subject->GetCurrentLayer()), subject->GetTeam());
    subject->Move(moveHandle, &location, NULL, GetFlags() & TF_AI ? TRUE : FALSE);

    NextState(0x9E947215); // "Moving"   
  }


  //
  // StateMoving
  //
  void UnitExplore::StateMoving()
  {
  }


  //
  // StateLoitering
  //
  void UnitExplore::StateLoitering(StateMachineNotify notify)
  {
    switch (notify)
    {
      case SMN_ENTRY:
        // Wait 5-10 seconds before moving again
        hangUntil = GameTime::SimCycle() + (50 + Random::sync.Integer(50));
        break;

      case SMN_PROCESS:
        if (GameTime::SimCycle() >= hangUntil)
        {
          NextState(0xABAA7B48); // "Init"
        }
        break;
    }
  }


  //
  // Notify this task of an event
  //
  Bool UnitExplore::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
        NextState(0xABAA7B48); // "Init"
        return (TRUE);

      case 0xE609174A: // "Movement::Completed"
      case 0x71BB2A61: // "Movement::Incapable"
        if (moveHandle == event.param1)
        {
          NextState(0x3CF1657E); // "Loitering"
        }
        return (TRUE);

      default:
        return (GameTask<UnitObjType, UnitObj>::ProcessEvent(event));
    }
  }


  //
  // Initialization
  //
  void UnitExplore::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Build state machine
    stateMachine.AddState("Init", &StateInit);
    stateMachine.AddState("Moving", &StateMoving);
    stateMachine.AddState("Loitering", &StateLoitering);
  }


  //
  // Shutdown
  //
  void UnitExplore::Done()
  {
    stateMachine.CleanUp();
  }
}
