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
#include "tasks_offmapidle.h"
#include "taskctrl.h"
#include "team.h"


//
// ~GameTask
//
// Destructor
//
GameTask<OffMapObjType, OffMapObj>::~GameTask()
{
}


//
// GameProcess
//
// Type specific processing
//
void GameTask<OffMapObjType, OffMapObj>::GameProcess()
{
}


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
  StateMachine<OffMapIdle> OffMapIdle::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class OffMapIdle
  //


  //
  // Constructor
  //
  OffMapIdle::OffMapIdle(GameObj *subject) :
    GameTask<OffMapObjType, OffMapObj>(staticConfig, subject),
    inst(&stateMachine, "Idle")
  {
  }


  //
  // Save
  //
  void OffMapIdle::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));
  }


  //
  // Load
  //
  void OffMapIdle::Load(FScope *fScope)
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
  void OffMapIdle::PostLoad()
  {
  }


  //
  // Perform task processing
  //
  Bool OffMapIdle::Process()
  {
    inst.Process(this);  

    return (FALSE);
  }


  //
  // Single state
  //
  void OffMapIdle::StateIdle()
  {
    // Has the facility been destroyed
    if (!subject->GetFacility())
    {
      subject->MarkForDeletion();
    }
  }


  //
  // Initialization
  //
  void OffMapIdle::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Add states to the state machine
    stateMachine.AddState("Idle", &OffMapIdle::StateIdle);
  }


  //
  // Shutdown
  //
  void OffMapIdle::Done()
  {
    stateMachine.CleanUp();
  }

}
