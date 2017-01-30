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
#include "tasks_transportunload.h"
#include "tasks_transportpad.h"
#include "taskctrl.h"



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
  StateMachine<TransportUnload> TransportUnload::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class TransportUnload
  //


  //
  // Constructor
  //
  TransportUnload::TransportUnload(GameObj *subject) :
    GameTask<TransportObjType, TransportObj>(staticConfig, subject),
    inst(&stateMachine, "Init")
  {
  }


  //
  // Constructor
  //
  TransportUnload::TransportUnload(GameObj *subject, const Vector &destination) :
    GameTask<TransportObjType, TransportObj>(staticConfig, subject),
    inst(&stateMachine, "Init"), destination(destination)
  {
  }


  //
  // Save
  //
  void TransportUnload::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));
    StdSave::TypeVector(fScope, "Destination", destination);   
    moveHandle.SaveState(fScope->AddFunction("MoveHandle"));
  }


  //
  // Load
  //
  void TransportUnload::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
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
  void TransportUnload::PostLoad()
  {
  }


  //
  // Perform task processing
  //
  Bool TransportUnload::Process()
  {
    inst.Process(this);  
    return (quit);
  }


  //
  // StateInit
  //
  // Start moving towards destination
  //
  void TransportUnload::StateInit()
  {
    // Setup height at destination
    destination.y = TerrainData::FindFloor(destination.x, destination.z);

    // Start moving
    subject->Move(moveHandle, &destination, NULL, FALSE);
    NextState(0x9E947215); // "Moving"
  }


  //
  // StateMoving
  //
  // Moving towards destination
  //
  void TransportUnload::StateMoving()
  {
    // Are we within range of the unloading location
    if (Vector(subject->Position() - destination).MagnitudeXZ() < subject->TransportType()->GetDistance())
    {   
      // Unload units
      if (subject->Unload(destination))
      {
        // Notify the player that we're done
        subject->NotifyPlayer(0x5CA4B1C1); // "Transport::Unloaded"

        // Finished
        Quit();
      }
    }
  }

  
  //
  // Notify
  //
  // Notify this task of an event
  //
  Bool TransportUnload::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
        NextState(0xABAA7B48); // "Init"
        return (TRUE);

      case Movement::Notify::Incapable:
        if (moveHandle == event.param1)
        {
          Quit();
        }
        return (TRUE);

      case Movement::Notify::Completed:
        if (moveHandle == event.param1)
        {
          subject->Move(moveHandle, &destination, NULL, FALSE);
        }
        return (TRUE);

    }
    return (GameTask<TransportObjType, TransportObj>::ProcessEvent(event));
  }


  //
  // Initialization
  //
  void TransportUnload::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Build state machine
    stateMachine.AddState("Init", &TransportUnload::StateInit);
    stateMachine.AddState("Moving", &TransportUnload::StateMoving);
  }


  //
  // Shutdown
  //
  void TransportUnload::Done()
  {
    stateMachine.CleanUp();
  }
}
