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
#include "tasks_unitsquadmiddle.h"
#include "restoreobj.h"
#include "promote.h"
#include "sight.h"
#include "weapon.h"
#include "squadobj.h"


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
  StateMachine<UnitSquadMiddle> UnitSquadMiddle::stateMachine;

  // Constants
  static const F32 strayDistance = 40.0f;
  static const F32 strayDistance2 = strayDistance * strayDistance;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Constructor
  //


  //
  // Constructor
  //
  UnitSquadMiddle::UnitSquadMiddle(GameObj *subject)
  : GameTask<UnitObjType, UnitObj>(staticConfig, subject),
    inst(&stateMachine, "Init")
  {
  }


  //
  // Save
  //
  void UnitSquadMiddle::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));   
    
    moveHandle.SaveState(fScope->AddFunction("MoveHandle"));
  }


  //
  // Load
  //
  void UnitSquadMiddle::Load(FScope *fScope)
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

        default:
          LoadTaskData(sScope);
          break;
      }
    }
  }


  //
  // Called after all objects are loaded
  //
  void UnitSquadMiddle::PostLoad()
  {
  }


  //
  // Perform task processing
  //
  Bool UnitSquadMiddle::Process()
  {
    // If the squad we're in is dead, then give up ?
    if (subject->GetSquad())
    {
      // State machine processing
      inst.Process(this);

      // Perform game processing
      GameProcess();
    }
    else
    {
      Quit();
    }

    return (quit);
  }


  //
  // StateInit
  //
  void UnitSquadMiddle::StateInit()
  {
    // Are we too far from the middle of the squad ?
    Vector squad;
    if (subject->GetSquad()->GetLocation(squad))
    {
      subject->Move(moveHandle, &squad, NULL, FALSE, GetFlags() & TF_FROM_ORDER ? &Movement::RequestData::orders : NULL);
      NextState(0x9E947215); // "Moving"
    }
    else
    {
      Quit();
    }
  }


  //
  // StateMoving
  //
  void UnitSquadMiddle::StateMoving()
  {
  }


  //
  // StateWaiting
  //
  void UnitSquadMiddle::StateWaiting()
  {
    // Are we too far from the middle of the squad ?
    Vector squad;
    if (subject->GetSquad()->GetLocation(squad))
    {
      if ((squad - subject->Origin()).Magnitude2() > strayDistance2)
      {
        NextState(0xABAA7B48); // "Init"
      }
    }
    else
    {
      Quit();
    }
  }


  //
  // Notify this task of an event
  //
  Bool UnitSquadMiddle::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
        NextState(0xABAA7B48); // "Init"
        return (TRUE);

      case Movement::Notify::Incapable:
      case Movement::Notify::Completed:
      {
        if (moveHandle == event.param1)
        {
          NextState(0xCC45C48B); // "Waiting"
          return (TRUE);
        }
        break;
      }
    }

    return (GameTask<UnitObjType, UnitObj>::ProcessEvent(event));
  }


  //
  // Initialization
  //
  void UnitSquadMiddle::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Build state machine
    stateMachine.AddState("Init", &StateInit);
    stateMachine.AddState("Moving", &StateMoving);
    stateMachine.AddState("Waiting", &StateWaiting);
  }


  //
  // Shutdown
  //
  void UnitSquadMiddle::Done()
  {
    stateMachine.CleanUp();
  }

}
