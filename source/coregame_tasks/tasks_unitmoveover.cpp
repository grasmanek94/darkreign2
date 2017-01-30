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
#include "tasks_unitmoveover.h"
#include "movement_pathfollow.h"


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
  StateMachine<UnitMoveOver> UnitMoveOver::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitMoveOver
  //


  //
  // Constructor
  //
  UnitMoveOver::UnitMoveOver(GameObj *subject) :
    GameTask<UnitObjType, UnitObj>(staticConfig, subject),
    inst(&stateMachine, "Init"),
    direction(0xFF),
    len(0),
    timeOut(0),
    waitTime(0)
  {
  }


  //
  // Constructor
  //
  UnitMoveOver::UnitMoveOver(GameObj *subject, U32 mediateHandle, U8 direction, U8 len, U32 timeOut, U8 waitTime) :
    GameTask<UnitObjType, UnitObj>(staticConfig, subject),
    inst(&stateMachine, "Init"),
    mediateHandle(mediateHandle),
    direction(direction),
    len(len),
    timeOut(timeOut),
    waitTime(waitTime)
  {
  }


  //
  // Save
  //
  void UnitMoveOver::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));   
    moveHandle.SaveState(fScope->AddFunction("MoveHandle"));
    StdSave::TypeU32(fScope, "Dir", direction);
    StdSave::TypeU32(fScope, "Len", len);
    StdSave::TypeU32(fScope, "TimeOut", timeOut);
    StdSave::TypeU32(fScope, "Wait", waitTime);
    StdSave::TypeU32(fScope, "MediateHandle", mediateHandle);
  }


  //
  // Load
  //
  void UnitMoveOver::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
          break;

        case 0xD2457FAE: // "Dir"
          direction = U8(StdLoad::TypeU32(sScope));
          break;

        case 0x6AF13979: // "Len"
          len = U8(StdLoad::TypeU32(sScope));
          break;

        case 0x6202B2C6: // "TimeOut"
          timeOut = StdLoad::TypeU32(sScope);
          break;

        case 0x6E758990: // "Wait"
          waitTime = U8(StdLoad::TypeU32(sScope));
          break;

        case 0x7A2790F3: // "MediateHandle"
          mediateHandle = StdLoad::TypeU32(sScope);
          break;

        case 0xD3D101D2: // "MoveHandle"
          moveHandle.LoadState(sScope);
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
  void UnitMoveOver::PostLoad()
  {
  }


  //
  // Redirect the unit to another avoidance item
  //
  void UnitMoveOver::Redirect(U32 mediateHandleIn, U8 directionIn, U8 lenIn, U32 timeOutIn, U8 waitTimeIn)
  {
    UnQuit();

    // Abort the previous item
    Movement::Mediator::Complete(mediateHandle, Movement::Mediator::SS_DONE);

    // Setup new info
    mediateHandle = mediateHandleIn;
    direction = directionIn;
    len = lenIn;
    timeOut = timeOutIn;
    waitTime = waitTimeIn;

    // Back to init state
    NextState(0xABAA7B48); // "Init"
  }


  //
  // Perform task processing
  //
  Bool UnitMoveOver::Process()
  {
    inst.Process(this);  

    // Perform type processing
    GameProcess();

    // Check for time out
    if (GameTime::SimCycle() > timeOut)
    {
      // Notify movement system that we are done
      Movement::Mediator::Complete(mediateHandle, Movement::Mediator::SS_DONE);
      Quit();
    }

    return (quit);
  }



  //
  // Initial state
  //
  void UnitMoveOver::StateInit()
  {
    Movement::RequestData rd;
    rd.giveUpGrains = 0;

    // Force a direct movement in the desired direction
    if (subject->GetDriver()->Setup(moveHandle, direction, len, &rd))
    {
      // Proceed
      NextState(0x9E947215); // "Moving"
    }
    else
    {
      // Failed
      Movement::Mediator::Complete(mediateHandle, Movement::Mediator::SS_ABORTED);
      Quit();
    }
  }


  //
  // Moving state
  //
  void UnitMoveOver::StateMoving()
  {
    // Thumb twiddling in progress
  }


  //
  // Notify this task of an event
  //
  Bool UnitMoveOver::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case 0x59707289: // "Interrupted"
      {
        //LOG_DIAG(("M%.8X %d UnitMoveOver interrupted@%d", mediateHandle, subject->Id(), GameTime::SimCycle()))

        Movement::Mediator::Complete(mediateHandle, Movement::Mediator::SS_ABORTED);
        Quit();
        return (TRUE);
      }

      case Movement::Notify::Incapable:
      {
        // Should never happen??
        return (TRUE);
      }

      case Movement::Notify::Completed:
      {
        if (moveHandle == event.param1)
        {
          // Wait about a second before ending
          timeOut = GameTime::SimCycle() + waitTime;
        }
        return (TRUE);
      }
    }
    return (GameTask<UnitObjType, UnitObj>::ProcessEvent(event));
  }


  //
  // Initialization
  //
  void UnitMoveOver::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Build state machine
    stateMachine.AddState("Init", &UnitMoveOver::StateInit);
    stateMachine.AddState("Moving", &UnitMoveOver::StateMoving);
  }


  //
  // Shutdown
  //
  void UnitMoveOver::Done()
  {
    stateMachine.CleanUp();
  }
}
