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
#include "tasks_unitboard.h"
#include "resolver.h"


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
  StateMachine<UnitBoard> UnitBoard::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitBoard
  //


  //
  // Constructor
  //
  UnitBoard::UnitBoard(GameObj *subject) :
    GameTask<UnitObjType, UnitObj>(staticConfig, subject),
    inst(&stateMachine, "Init")
  {
  }


  //
  // Constructor
  //
  UnitBoard::UnitBoard(GameObj *subject, TransportObj *target) :
    GameTask<UnitObjType, UnitObj>(staticConfig, subject),
    inst(&stateMachine, "Init"), target(target)
  {
  }


  //
  // Save
  //
  void UnitBoard::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));
    StdSave::TypeReaper(fScope, "Target", target);
    StdSave::TypeVector(fScope, "Destination", destination);
    moveHandle.SaveState(fScope->AddFunction("MoveHandle"));
  }


  //
  // Load
  //
  void UnitBoard::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
          break;

        case 0xD6A8B702: // "Target"
          StdLoad::TypeReaper(sScope, target);
          break;

        case 0xA302E408: // "Destination"
          StdLoad::TypeVector(sScope, destination);
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
  void UnitBoard::PostLoad()
  {
    Resolver::Object<TransportObj, TransportObjType>(target);
  }


  //
  // Perform task processing
  //
  Bool UnitBoard::Process()
  {
    inst.Process(this);  
    return (quit);
  }


  //
  // ValidTarget
  //
  // Do we have a valid transport target
  //
  Bool UnitBoard::ValidTarget()
  {
    // Do we have a valid target reaper
    if (target.Alive())
    {
      // Ensure we are still welcome
      return (target->CheckCargo(subject));
    }

    return (FALSE);
  }


  //
  // GetTargetVector
  //
  // Returns either the destination or current position of the target
  //
  Vector UnitBoard::GetTargetVector()
  {
    Vector v;

    // If target is moving, take its destination
    if (!target->GetCurrentDestination(v))
    {
      // Otherwise take its current position
      v = target->Position();
    }

    return (v);
  }


  //
  // TargetMoved
  //
  // Has the target moved significantly from it's original location
  //
  Bool UnitBoard::TargetMoved()
  {
    Vector v = GetTargetVector();

    return
    (
      Vector(v - destination).MagnitudeXZ() > target->TransportType()->GetDistance()
    );
  }


  //
  // TargetInRange
  //
  // Is the target within boarding range
  //
  Bool UnitBoard::TargetInRange()
  {
    F32 distance = target->TransportType()->GetDistance();
    F32 current = Vector(target->Position() - subject->Position()).MagnitudeXZ();

    // Think faster when target is close
    if (current < distance * 1.5F)
    {
      ThinkFast();
    }

    return (current < distance);
  }


  //
  // StateInit
  //
  // Start moving towards current target
  //
  void UnitBoard::StateInit()
  {
    // Do we have a valid target
    if (ValidTarget())
    {
      // Set initial destination to be current location
      destination = subject->Position();

      // Do we need to move to reach the target vector
      if (TargetMoved())
      {
        // Set new destination
        destination = GetTargetVector();

        // Start moving
        subject->Move(moveHandle, &destination, NULL, FALSE);
      }

      NextState(0xB298F907); // "SeekEntry"
    }
    else
    {
      // Tell the squad that we've failed
      subject->PostSquadEvent(Event(0x88A050C3, GetTaskId())); // "Board::Failed"

      NextState(0x8C1F21CA); // "Done"
    }
  }


  //
  // StateSeekEntry
  //
  // Move to target (or the targets destination) and wait
  //
  void UnitBoard::StateSeekEntry()
  {
    if (ValidTarget())
    {
      if (TargetMoved())
      {
        // Plot a new course
        NextState(0xABAA7B48); // "Init"
      }
      else

      if (TargetInRange())
      {
        // Get into the transport
        target->PickupCargo(subject);

        // Tell the squad that we're in
        subject->PostSquadEvent(Event(0x7D63DD2C, GetTaskId())); // "Board::Completed"

        // We're done
        NextState(0x8C1F21CA); // "Done"
      }
    }
    else
    {
      // Tell the squad that we've failed
      subject->PostSquadEvent(Event(0x88A050C3, GetTaskId())); // "Board::Failed"

      NextState(0x8C1F21CA); // "Done"
    }
  }


  //
  // StateDone
  //
  // All finished
  //
  void UnitBoard::StateDone()
  {
    Quit();
  }


  //
  // Notify
  //
  // Notify this task of an event
  //
  Bool UnitBoard::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
        NextState(0xABAA7B48); // "Init"
        return (TRUE);

      case Movement::Notify::Incapable:
        if (moveHandle == event.param1)
        {
          subject->Move(moveHandle, &destination, NULL, FALSE);
        }
        return (TRUE);

      case Movement::Notify::Completed:
        if (moveHandle == event.param1)
        {
          subject->Move(moveHandle, &destination, NULL, FALSE);
        }
        return (TRUE);

    }
    return (GameTask<UnitObjType, UnitObj>::ProcessEvent(event));
  }


  //
  // Initialization
  //
  void UnitBoard::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Build state machine
    stateMachine.AddState("Init", &UnitBoard::StateInit);
    stateMachine.AddState("SeekEntry", &UnitBoard::StateSeekEntry);
    stateMachine.AddState("Done", &UnitBoard::StateDone);
  }


  //
  // Shutdown
  //
  void UnitBoard::Done()
  {
    stateMachine.CleanUp();
  }
}
