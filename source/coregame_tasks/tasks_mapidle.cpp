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
#include "tasks_mapidle.h"


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
  StateMachine<MapIdle> MapIdle::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class MapIdle
  //


  //
  // Constructor
  //
  MapIdle::MapIdle(GameObj *subject) :
    GameTask<MapObjType, MapObj>(staticConfig, subject),
    inst(&stateMachine, "Init")
  {
  }


  //
  // Save
  //
  void MapIdle::Save(FScope *fScope)
  {
    // Only save state if we're active
    if (subject->GetCurrentTask() == this)
    {
      SaveTaskData(fScope);
      inst.SaveState(fScope->AddFunction("StateMachine"));
    }
  }


  //
  // Load
  //
  void MapIdle::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
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
  void MapIdle::PostLoad()
  {
  }


  //
  // Perform task processing
  //
  Bool MapIdle::Process()
  {
    inst.Process(this);  
    return (FALSE);
  }


  //
  // Initial state
  //
  void MapIdle::StateInit()
  {
    // Blend the animation back to the idle animation
    subject->SetAnimation(0x8F651465); // "Default"

    // Proceed to the idle state
    inst.Set(0x793360CC); // "Idle"
  }


  //
  // Idle state
  //
  void MapIdle::StateIdle()
  {
  }


  //
  // Notify this task of an event
  //
  Bool MapIdle::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
        inst.Set(0xABAA7B48); // "Init"
        return (TRUE);
    }
    return (GameTask<MapObjType, MapObj>::ProcessEvent(event));
  }


  //
  // Initialization
  //
  void MapIdle::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Add states to the state machine
    stateMachine.AddState("Init", &MapIdle::StateInit);
    stateMachine.AddState("Idle", &MapIdle::StateIdle);
  }


  //
  // Shutdown
  //
  void MapIdle::Done()
  {
    stateMachine.CleanUp();
  }

}
