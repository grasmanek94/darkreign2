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
#include "tasks_squadguard.h"
#include "tasks_unitguard.h"


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
  StateMachine<SquadGuard> SquadGuard::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class SquadGuard
  //


  //
  // Constructor
  //
  SquadGuard::SquadGuard(GameObj *subject) 
  : GameTask<SquadObjType, SquadObj>(staticConfig, subject),
    inst(&stateMachine, "Init")
  {
  }


  //
  // Constructor
  //
  SquadGuard::SquadGuard(GameObj *subject, const Target &target) 
  : GameTask<SquadObjType, SquadObj>(staticConfig, subject),
    inst(&stateMachine, "Init"),
    target(target)
  {
  }



  //
  // Destructor
  //
  SquadGuard::~SquadGuard()
  {
  }


  //
  // Save
  //
  void SquadGuard::Save(FScope *fScope)
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
  void SquadGuard::Load(FScope *fScope)
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
  void SquadGuard::PostLoad()
  {
    target.PostLoad();
  }


  //
  // Perform task processing
  //
  Bool SquadGuard::Process()
  {
    inst.Process(this);  

    return (quit);
  }


  //
  // Notify this task of an event
  //
  Bool SquadGuard::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
        inst.Set(0xABAA7B48); // "Init"
        return (TRUE);

      case 0xD6ACAB57: // "Guard::Incapable"
      case 0x411E967C: // "Guard::Completed"
      {
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
  void SquadGuard::StateInit()
  {
    // Guard that target already
    for (SquadObj::UnitList::Iterator i(&subject->GetList()); *i; i++)
    {
      if ((*i)->Alive())
      {
        (*i)->completed = FALSE;
        (*i)->task = 0;
        UnitObj *unit = **i;
        Task *task;

        // If the unit can attack then make it attack
        if ((**i)->CanEverFire())
        {
          unit->FlushTasks(Tasks::UnitGuard::GetConfigBlockingPriority());
          unit->PrependTask(task = new UnitGuard(unit, target), GetFlags());
          (*i)->task = task->GetTaskId();
        }
        else 
        {
          (*i)->completed = TRUE;
        }
      }
    }
    NextState(0x33E7172A); // "Guarding"
  }
  

  //
  // Scouting state
  //
  void SquadGuard::StateGuarding()
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
    subject->NotifyPlayer(0x763C5781); // "Squad::TaskCompleted"

    Quit();
  }


  //
  // Initialization
  //
  void SquadGuard::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Build state machine
    stateMachine.AddState("Init", &StateInit);
    stateMachine.AddState("Guarding", &StateGuarding);
  }


  //
  // Shutdown
  //
  void SquadGuard::Done()
  {
    stateMachine.CleanUp();
  }

}
