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
#include "tasks_unitattack.h"
#include "sight.h"
#include "random.h"
#include "mapobjiter.h"
#include "ray.h"
#include "taskctrl.h"
#include "weapon.h"
#include "movement_pathfollow.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // State machine
  StateMachine<UnitAttack> UnitAttack::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitAttack
  //


  //
  // Constructor
  //
  UnitAttack::UnitAttack(GameObj *sub) :
    GameTask<UnitObjType, UnitObj>(staticConfig, sub),
    inst(&stateMachine)
  {
  }


  //
  // Constructor
  //
  UnitAttack::UnitAttack(GameObj *sub, const Target &target) :
    GameTask<UnitObjType, UnitObj>(staticConfig, sub),
    inst(&stateMachine, "Init"),
    target(target)
  {
    // Save the current location
    UnitAttack::subject->GetWeapon()->SaveLocation();
  }


  //
  // Destructor
  //
  UnitAttack::~UnitAttack()
  {
  }


  //
  // Save
  //
  void UnitAttack::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));

    if (target.Valid())
    {
      target.SaveState(fScope->AddFunction("Target"));
    }
  }


  //
  // Load
  //
  void UnitAttack::Load(FScope *fScope)
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
          target.LoadState(sScope);
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
  void UnitAttack::PostLoad()
  {
    target.PostLoad();
  }


  //
  // Notify this task of an event
  //
  Bool UnitAttack::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
        subject->GetWeapon()->HaltFire();
        NextState(0xABAA7B48); // "Init"
        return (TRUE);

      case Weapon::Notify::Failed:
        subject->PostSquadEvent(Task::Event(0xEFF953C4, GetTaskId())); // "Attack::Incapable"
        Quit();
        return (TRUE);

      case Weapon::Notify::Completed:
        subject->PostSquadEvent(Task::Event(0x784B6EEF, GetTaskId())); // "Attack::Completed"
        Quit();
        return (TRUE);
    }
    return (GameTask<UnitObjType, UnitObj>::ProcessEvent(event));
  }


  //
  // GetBlockingPriority
  //
  // Get the blocking priority of this task
  //
  U32 UnitAttack::GetBlockingPriority()
  {
    return 
    (
      subject->blindTarget.Test() ? GameTask<UnitObjType, UnitObj>::GetBlockingPriority() : TaskCtrl::BLOCK_UNAVAILABLE
    );
  }


  //
  // Set a new target
  //
  void UnitAttack::SetTarget(const Target &newTarget)
  {
    target = newTarget;
    NextState(0xABAA7B48); // "Init"
  }


  //
  // StateInit
  //
  void UnitAttack::StateInit()
  {
    subject->GetWeapon()->SetTarget(target);
    NextState(0x52709427); // "Attacking"
  }


  //
  // StateAttacking
  //
  void UnitAttack::StateAttacking()
  {
  }


  //
  // Perform task processing
  //
  Bool UnitAttack::Process()
  {
    inst.Process(this);  

    // Perform type processing
    GameProcess();

    return (quit);
  }


  //
  // Initialization
  //
  void UnitAttack::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Build state machine
    stateMachine.AddState("Init", &StateInit);
    stateMachine.AddState("Attacking", &StateAttacking);
  }


  //
  // Shutdown
  //
  void UnitAttack::Done()
  {
    stateMachine.CleanUp();
  }

}
