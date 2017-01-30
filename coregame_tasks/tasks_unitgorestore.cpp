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
#include "tasks_unitgorestore.h"
#include "resolver.h"
#include "restoreobj.h"


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
  StateMachine<UnitGoRestore> UnitGoRestore::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitGoRestore
  //


  //
  // Constructor
  //
  UnitGoRestore::UnitGoRestore(GameObj *subject) :
    GameTask<UnitObjType, UnitObj>(staticConfig, subject),
    inst(&stateMachine, "Init")
  {
  }


  //
  // Save
  //
  void UnitGoRestore::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));   

    StdSave::TypeReaper(fScope, "Facility", facility);

    moveHandle.SaveState(fScope->AddFunction("MoveHandle"));
  }


  //
  // Load
  //
  void UnitGoRestore::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
          break;

        case 0x2B4A9F8C: // "Facility"
          StdLoad::TypeReaper(sScope, facility);
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
  void UnitGoRestore::PostLoad()
  {
    Resolver::Object<RestoreObj, RestoreObjType>(facility);
  }


  //
  // Perform task processing
  //
  Bool UnitGoRestore::Process()
  {
    inst.Process(this);  

    return (quit);
  }


  //
  // Initial state
  //
  void UnitGoRestore::StateInit()
  {
    // If none set, find a restore facility
    if (facility.Dead())
    {
      facility = subject->GetRestoreFacility();
    }

    // Do we have a valid facility
    if (facility.Alive())
    {
      // Order the unit to move to the restore facility
      subject->MoveOnto(moveHandle, facility, GetFlags() & TF_AI ? TRUE : FALSE);

      // Proceed
      NextState(0x9E947215); // "Moving"
    }
    else
    {
      subject->PostSquadEvent(Event(0x35AAB38A, GetTaskId())); // "Restore::Incapable"
      Quit();
    }
  }


  //
  // Moving state
  //
  void UnitGoRestore::StateMoving()
  {
    // Is the facility dead
    if (facility.Dead())
    {
      // Find a new facility
      NextState(0xABAA7B48); // "Init"
    }
  }


  //
  // Notify this task of an event
  //
  Bool UnitGoRestore::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
        NextState(0xABAA7B48); // "Init"
        return (TRUE);

      case Movement::Notify::Incapable:
        if (moveHandle == event.param1)
        {
          subject->PostSquadEvent(Event(0x35AAB38A, GetTaskId())); // "Restore::Incapable"
          Quit();
        }
        return (TRUE);

      case Movement::Notify::Completed:
        if (moveHandle == event.param1)
        {
          subject->PostSquadEvent(Event(0xA2188EA1, GetTaskId())); // "Restore::Completed"
          Quit();
        }
        return (TRUE);
    }
    return (GameTask<UnitObjType, UnitObj>::ProcessEvent(event));
  }


  //
  // ChangeFacility
  //
  // Change the restore facility
  //
  void UnitGoRestore::ChangeFacility(RestoreObj *restore)
  {
    if (restore != facility.GetPointer())
    {
      facility = restore;
      NextState(0xABAA7B48); // "Init"
    }
  }


  //
  // Initialization
  //
  void UnitGoRestore::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Build state machine
    stateMachine.AddState("Init", &UnitGoRestore::StateInit);
    stateMachine.AddState("Moving", &UnitGoRestore::StateMoving);
  }


  //
  // Shutdown
  //
  void UnitGoRestore::Done()
  {
    stateMachine.CleanUp();
  }
}
