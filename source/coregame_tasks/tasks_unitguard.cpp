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
#include "tasks_unitguard.h"
#include "restoreobj.h"
#include "promote.h"
#include "sight.h"
#include "weapon.h"


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
  StateMachine<UnitGuard> UnitGuard::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Constructor
  //


  //
  // Constructor
  //
  UnitGuard::UnitGuard(GameObj *subject)
  : GameTask<UnitObjType, UnitObj>(staticConfig, subject),
    inst(&stateMachine, "Init")
  {
  }


  //
  // Initializing Constructor
  //
  UnitGuard::UnitGuard(GameObj *subject, const Target &guardTarget)
  : GameTask<UnitObjType, UnitObj>(staticConfig, subject),
    inst(&stateMachine, "Init"),
    guardTarget(guardTarget)
  {
  }


  //
  // Save
  //
  void UnitGuard::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));   
    
    if (guardTarget.Valid())
    {
      guardTarget.SaveState(fScope->AddFunction("GuardTarget"));
    }

    if (attackTarget.Valid())
    {
      attackTarget.SaveState(fScope->AddFunction("AttackTarget"));
    }

    moveHandle.SaveState(fScope->AddFunction("MoveHandle"));
  }


  //
  // Load
  //
  void UnitGuard::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
          break;

        case 0x78CF31E6: // "GuardTarget"
          guardTarget.LoadState(sScope);
          break;

        case 0xF874D787: // "AttackTarget"
          attackTarget.LoadState(sScope);
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
  void UnitGuard::PostLoad()
  {
    guardTarget.PostLoad();
    attackTarget.PostLoad();
  }


  //
  // Perform task processing
  //
  Bool UnitGuard::Process()
  {
    // If the object we're guard isn't alive then give up
    if (!guardTarget.Alive())
    {
      subject->PostSquadEvent(Task::Event(0x411E967C, GetTaskId())); // "Guard::Completed"
      Quit();
    }
    else
    {
      // State machine processing
      inst.Process(this);

      // Perform game processing
      GameProcess();
    }

    return (quit);
  }


  //
  // Set the current attack target
  //
  void UnitGuard::SetAttackTarget(const Target &target)
  {
    attackTarget = target;
    NextState(0xABAA7B48); // "Init"
  }


  //
  // CanGuard
  //
  // Can the given unit use the guard task
  //
  Bool UnitGuard::CanGuard(UnitObj *unit)
  {
    ASSERT(unit)

    // Has to be able to move to guard
    if (!unit->UnitType()->CanEverMove())
    {
      return (FALSE);
    }

    // Can the unit fire
    if (unit->CanEverFire())
    {
      return (TRUE);
    }

    // Is the unit a mobile restorer
    if (RestoreObj *restore = Promote::Object<RestoreObjType, RestoreObj>(unit))
    {
      if (!restore->IsStatic())
      {
        return (TRUE);
      }
    }

    // Unable to guard
    return (FALSE);
  }


  //
  // StateInit
  //
  void UnitGuard::StateInit()
  {
    // Do we have a live target ?
    if (attackTarget.Alive())
    {
      ASSERT(subject->GetWeapon())

      // Give it to the weapon
      subject->GetWeapon()->SetTarget(attackTarget);
    }

    NextState(0x33E7172A); // "Guarding"
  }


  //
  // StateGuarding
  //
  void UnitGuard::StateGuarding()
  {
    // Are we engaging an assailent ?
    if (attackTarget.Valid())
    {
      ASSERT(subject->GetWeapon())

      if (!guardTarget.IsVisible(subject))
      {
        // Make sure that we don't look for targets
        flags &= ~TF_FLAG1;

        // Make sure we stop shooting
        subject->GetWeapon()->HaltFire();

        // Move to the location of the guard point
        subject->Move(moveHandle, &guardTarget.GetLocation(), NULL, FALSE, GetFlags() & TF_FROM_ORDER ? &Movement::RequestData::orders : NULL);
        NextState(0x9E947215); // "Moving"
      }
    }
    else
    {
      // We wan't to be close to the guard target, make sure we are no more than 25m away
      if ((guardTarget.GetLocation() - subject->RootOrigin()).Magnitude2() > (25.0f * 25.0f))
      {
        // Move to the location of the guard point
        subject->Move(moveHandle, &guardTarget.GetLocation(), NULL, FALSE, GetFlags() & TF_FROM_ORDER ? &Movement::RequestData::orders : NULL);
        NextState(0x9E947215); // "Moving"
      }
    }

    // Make sure that we look for targets
    flags |= TF_FLAG1;
  }


  //
  // StateMoving
  //
  void UnitGuard::StateMoving()
  {
  }


  //
  // Notify this task of an event
  //
  Bool UnitGuard::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
        NextState(0xABAA7B48); // "Init"
        return (TRUE);

      case Weapon::Notify::Failed:
      case Weapon::Notify::Completed:
        // Invalidate our attack target
        attackTarget.InValidate();
        break;

      case Movement::Notify::Incapable:
        if (moveHandle == event.param1)
        {
          // Return to the guarding state
          NextState(0x33E7172A); // "Guarding"
          return (TRUE);
        }

      case Movement::Notify::Completed:
      {
        if (moveHandle == event.param1)
        {
          // Return to the guarding state
          NextState(0x33E7172A); // "Guarding"
          return (TRUE);
        }
      }
    }

    return (GameTask<UnitObjType, UnitObj>::ProcessEvent(event));
  }


  //
  // Initialization
  //
  void UnitGuard::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Build state machine
    stateMachine.AddState("Init", &StateInit);
    stateMachine.AddState("Guarding", &StateGuarding);
    stateMachine.AddState("Moving", &StateMoving);
  }


  //
  // Shutdown
  //
  void UnitGuard::Done()
  {
    stateMachine.CleanUp();
  }

}
