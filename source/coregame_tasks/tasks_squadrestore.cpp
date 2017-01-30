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
#include "tasks_squadrestore.h"
#include "taskctrl.h"
#include "tasks_unitgorestore.h"
#include "resolver.h"


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
  StateMachine<SquadRestore> SquadRestore::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class SquadRestore
  //


  //
  // Constructor
  //
  SquadRestore::SquadRestore(GameObj *subject) 
  : GameTask<SquadObjType, SquadObj>(staticConfig, subject),
    inst(&stateMachine, "Init")
  {
  }


  //
  // Destructor
  //
  SquadRestore::~SquadRestore()
  {
  }


  //
  // Save
  //
  void SquadRestore::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));
  }


  //
  // Load
  //
  void SquadRestore::Load(FScope *fScope)
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
  void SquadRestore::PostLoad()
  {
  }


  //
  // Perform task processing
  //
  Bool SquadRestore::Process()
  {
    inst.Process(this);  

    return (quit);
  }


  //
  // Notify this task of an event
  //
  Bool SquadRestore::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
        inst.Set(0xABAA7B48); // "Init"
        return (TRUE);

      case 0x35AAB38A: // "Restore::Incapable"
      case 0xA2188EA1: // "Restore::Completed"
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
          }
        }
        return (TRUE);
      }

      default:
        return (GameTask<SquadObjType, SquadObj>::ProcessEvent(event));
    }
  }


  //
  // Initial state
  //
  void SquadRestore::StateInit()
  {
    // Iterate all units in the squad
    for (SquadObj::UnitList::Iterator i(&subject->GetList()); *i; ++i)
    {
      // Is this unit alive
      if (UnitObj *unit = (*i)->GetPointer())
      {
        // Can this unit restore ?
        if (unit->CanEverMove() && unit->FlushTasks(Tasks::UnitGoRestore::GetConfigBlockingPriority()))
        {
          Tasks::UnitGoRestore *task;

          // Give it the move task
          unit->PrependTask(task = new Tasks::UnitGoRestore(unit), GetFlags());
          (*i)->completed = FALSE;
          (*i)->task = task->GetTaskId();
        }
        else
        {
          (*i)->completed = TRUE;
          (*i)->task = 0;
        }
      }
    }

    NextState(0x34AE4B12); // "Restoring"
  }


  //
  // Restoring state
  //
  void SquadRestore::StateRestoring()
  {
    Bool completed = TRUE;

    for (SquadObj::UnitList::Iterator i(&subject->GetList()); *i; ++i)
    {
      if ((*i)->Alive() && !(*i)->completed)
      {
        completed = FALSE;
        break;
      }
    }

    if (completed)
    {
      // Notify the player that we're done
      subject->NotifyPlayer(0x763C5781); // "Squad::TaskCompleted"
      Quit();
    }
  }


  //
  // Initialization
  //
  void SquadRestore::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Build state machine
    stateMachine.AddState("Init", &StateInit);
    stateMachine.AddState("Restoring", &StateRestoring);
  }


  //
  // Shutdown
  //
  void SquadRestore::Done()
  {
    stateMachine.CleanUp();
  }

}
