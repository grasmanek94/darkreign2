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
#include "tasks_squadattack.h"
#include "tasks_unitattack.h"
#include "taskctrl.h"


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
  StateMachine<SquadAttack> SquadAttack::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class SquadAttack
  //


  //
  // Constructor
  //
  SquadAttack::SquadAttack(GameObj *subject) :
    GameTask<SquadObjType, SquadObj>(staticConfig, subject),
    inst(&stateMachine),
    target(NULL)
  {
  }


  //
  // Constructor
  //
  SquadAttack::SquadAttack(GameObj *subject, const Target &target) :
    GameTask<SquadObjType, SquadObj>(staticConfig, subject),
    inst(&stateMachine, "Init"),
    target(target)
  {
  }


  //
  // Destructor
  //
  SquadAttack::~SquadAttack()
  {
  }


  //
  // Save
  //
  void SquadAttack::Save(FScope *fScope)
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
  void SquadAttack::Load(FScope *fScope)
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
  void SquadAttack::PostLoad()
  {
    target.PostLoad();
  }


  //
  // Perform task processing
  //
  Bool SquadAttack::Process()
  {
    inst.Process(this);  

    return (quit);
  }


  //
  // Notify this task of an event
  //
  Bool SquadAttack::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
        inst.Set(0xABAA7B48); // "Init"
        return (TRUE);

      case 0xEFF953C4: // "Attack::Incapable"
      {
        // One of the units in the squad is telling us it can't attack the target
//        LOG_DIAG(("Unit %d in the squad could not attack", data))

        for (SquadObj::UnitList::Iterator i(&subject->GetList()); *i; i++)
        {
          if (
            (*i)->Alive() && 
            (*i)->task == event.param1 &&
            (*i)->Id() == event.param2)
          {
            (*i)->completed = TRUE;
            subject->ThinkFast();
          }
        }
        return (TRUE);
      }

      case 0x784B6EEF: // "Attack::Completed"
      {
        // One of the units in the squad is telling us that it has completed attacking
//        LOG_DIAG(("Unit %d in the squad has completed its attack", data))

        // Mark the completed flag for this squad member
        for (SquadObj::UnitList::Iterator i(&subject->GetList()); *i; i++)
        {
          if (
            (*i)->Alive() && 
            (*i)->task == event.param1 &&
            (*i)->Id() == event.param2)
          {
            (*i)->completed = TRUE;
            subject->ThinkFast();
            break;
          }
        }
        return (TRUE);
      }

    }
    return (GameTask<SquadObjType, SquadObj>::ProcessEvent(event));
  }


  //
  // Initial state
  //
  void SquadAttack::StateInit()
  {
    // Attack that target already
    for (SquadObj::UnitList::Iterator i(&subject->GetList()); *i; ++i)
    {
      if (UnitObj *u = (*i)->GetPointer())
      {
        // If the unit can damage the target then make it attack
        if 
        (
          !u->ForceAttacking(target) && u->CanDamageNow(target) && 
          u->FlushTasks(Tasks::UnitAttack::GetConfigBlockingPriority())
        )
        {
          Task *task = new UnitAttack(u, target);
          u->PrependTask(task, GetFlags());
          (*i)->task = task->GetTaskId();
          (*i)->completed = FALSE;
        }
        else 
        {
          (*i)->task = 0;
          (*i)->completed = TRUE;
        }
      }
    }
    NextState(0xA8FEF90A); // "Attack"
  }


  //
  // Attack state
  //
  void SquadAttack::StateAttack()
  {
    // Have all of the squad memebers completed their attack ?
    for (SquadObj::UnitList::Iterator i(&subject->GetList()); *i; i++)
    {
      if ((*i)->Alive() && !(*i)->completed)
      {
        return;
      }
    }

    // Entire squad has completed its attack
//    LOG_DIAG(("Entire squad has completed its attack"))
    subject->NotifyPlayer(0x763C5781); // "Squad::TaskCompleted"

    Quit();
  }


  //
  // Initialization
  //
  void SquadAttack::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Build state machine
    stateMachine.AddState("Init", &StateInit);
    stateMachine.AddState("Attack", &StateAttack);
  }


  //
  // Shutdown
  //
  void SquadAttack::Done()
  {
    stateMachine.CleanUp();
  }

}
