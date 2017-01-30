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
#include "tasks_squadexplore.h"
#include "taskutil.h"
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
  StateMachine<SquadExplore> SquadExplore::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class SquadExplore
  //


  //
  // Constructor
  //
  SquadExplore::SquadExplore(GameObj *subject) :
    GameTask<SquadObjType, SquadObj>(staticConfig, subject),
    inst(&stateMachine, "Init"),
    taskMove(NULL)
  {
  }


  //
  // Destructor
  //
  SquadExplore::~SquadExplore()
  {
    if (taskMove)
    {
      delete taskMove;
    }
  }


  //
  // Save
  //
  void SquadExplore::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));

    if (taskMove)
    {
      TaskCtrl::SaveTask(fScope, "TaskMove", *taskMove);
    }
  }

  //
  // Load
  //
  void SquadExplore::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
          break;

        case 0x14DB2B40: // "TaskMove"
        {
          if ((taskMove = TaskCtrl::Promote<SquadMove>(TaskCtrl::LoadTask(subject, sScope))) != NULL)
          {
            taskMove->SetTask(this);
          }
          break;
        }

        default:
          LoadTaskData(sScope);
          break;
      }
    }
  }


  //
  // Called after all objects are loaded
  //
  void SquadExplore::PostLoad()
  {
    if (taskMove)
    {
      TaskCtrl::PostLoad(*taskMove);
    }
  }


  //
  // Perform task processing
  //
  Bool SquadExplore::Process()
  {
    inst.Process(this);  
    return (quit);
  }


  //
  // Notify this task of an event
  //
  Bool SquadExplore::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
        inst.Set(0xABAA7B48); // "Init"
    }

    // Pass down to the move task if there is one
    if (taskMove)
    {
      return (taskMove->ProcessEvent(event));
    }

    // Return game task's take on the event
    return (GameTask<SquadObjType, SquadObj>::ProcessEvent(event));
  }


  //
  // Initial state
  //
  void SquadExplore::StateInit()
  {
    // Determine the central cell of the squad and the most common traction type
    Vector location(0.0f, 0.0f, 0.0f);
    Point<S32> cell;
    U32 count = 0;

    U32 *tractions = new U32[MoveTable::TractionCount()];
    Utils::Memset(tractions, 0x00, MoveTable::TractionCount() * sizeof (U32));

    for (SquadObj::UnitList::Iterator u(&subject->GetList()); *u; u++)
    {
      ASSERT((*u)->Alive())
      UnitObj *unitObj = **u;

      location += unitObj->RootOrigin();
      tractions[unitObj->UnitType()->GetTractionIndex(unitObj->GetCurrentLayer())] += 1;

      count++;
    }

    // What's the winning traction index
    U8 tractionIndex = 0;
    U32 maxTractionCount = U32_MIN;

    for (U8 t = 0; t < MoveTable::TractionCount(); t++)
    {
      if (tractions[t] > maxTractionCount)
      {
        tractionIndex = t;
        maxTractionCount = tractions[t];
      }
    }

    delete tractions;

    if (count)
    {
      location /= F32(count);
      cell.Set(WorldCtrl::MetresToCellX(location.x), WorldCtrl::MetresToCellZ(location.z));
    }
    else
    {
      Quit();
      return;
    }

    // Find an unexplored spot
    TaskUtil::FindUnexplored(location, cell, tractionIndex, subject->GetTeam());

    if (taskMove)
    {
      delete taskMove;
    }

    taskMove = new Tasks::SquadMove(subject, this, location);
    NextState(0x9E947215); // "Moving"
  }


  //
  // Scouting state
  //
  void SquadExplore::StateMoving()
  {
    if (taskMove)
    {
      // Process the move task
      if (taskMove->Process())
      {
        delete taskMove;
        taskMove = NULL;

        NextState(0xABAA7B48); // "Init"
      }
    }
    else
    {
      NextState(0xABAA7B48); // "Init"
    }
  }


  //
  // Initialization
  //
  void SquadExplore::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Build state machine
    stateMachine.AddState("Init", &StateInit);
    stateMachine.AddState("Moving", &StateMoving);
  }


  //
  // Shutdown
  //
  void SquadExplore::Done()
  {
    stateMachine.CleanUp();
  }

}
