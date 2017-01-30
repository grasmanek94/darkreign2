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
#include "tasks_squadfollowtag.h"
#include "random.h"
#include "sight.h"
#include "taskctrl.h"
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
  StateMachine<SquadFollowTag> SquadFollowTag::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class SquadFollowTag
  //


  //
  // Constructor
  //
  SquadFollowTag::SquadFollowTag(GameObj *subject)
  : GameTask<SquadObjType, SquadObj>(staticConfig, subject),
    tag(NULL),
    taskMove(NULL),
    inst(&stateMachine, "Init")
  {
  }


  //
  // Constructor
  //
  SquadFollowTag::SquadFollowTag(GameObj *subject, TagObj *tag) :
    GameTask<SquadObjType, SquadObj>(staticConfig, subject),
    tag(tag),
    inst(&stateMachine, "Init")
  {
    tag->GetLocation(destination);
    taskMove = new SquadMove(subject, this, destination);
  }


  //
  // Destructor
  //
  SquadFollowTag::~SquadFollowTag()
  {
    if (taskMove)
    {
      delete taskMove;
    }
  }


  //
  // Save
  //
  void SquadFollowTag::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));

    if (taskMove)
    {
      TaskCtrl::SaveTask(fScope, "TaskMove", *taskMove);
    }

    StdSave::TypeReaper(fScope, "Tag", tag);
    StdSave::TypeVector(fScope, "Destination", destination);
  }

  //
  // Load
  //
  void SquadFollowTag::Load(FScope *fScope)
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

        case 0x1E534497: // "Tag"
          StdLoad::TypeReaper(sScope, tag);
          break;

        case 0xA302E408: // "Destination"
          StdLoad::TypeVector(sScope, destination);
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
  void SquadFollowTag::PostLoad()
  {
    if (taskMove)
    {
      TaskCtrl::PostLoad(*taskMove);
    }

    Resolver::Object<TagObj, TagObjType>(tag);
  }


  //
  // Perform task processing
  //
  Bool SquadFollowTag::Process()
  {
    inst.Process(this);  

    return (quit);
  }


  //
  // Notify this task of an event
  //
  Bool SquadFollowTag::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
        inst.Set(0xABAA7B48); // "Init"
        return (TRUE);

      default:
        // Pass down to the move task if there is one
        if (taskMove)
        {
          return (taskMove->ProcessEvent(event));
        }
        break;

    }
    return (GameTask<SquadObjType, SquadObj>::ProcessEvent(event));
  }


  //
  // Initial state
  //
  void SquadFollowTag::StateInit()
  {
    // Has the tag moved far from the current destination ?
    Vector location;
    tag->GetLocation(location);
    if (Vector(destination - location).Magnitude2() > 100)
    {
      if (taskMove)
      {
        delete taskMove;
      }
      destination = location;
      taskMove = new SquadMove(subject, this, destination);
      taskMove->SetFlags(GetFlags());
    }

    if (taskMove)
    {
      // Process the move task
      if (taskMove->Process())
      {
        delete taskMove;
        taskMove = NULL;
      }
    }
  }


  //
  // Initialization
  //
  void SquadFollowTag::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Build state machine
    stateMachine.AddState("Init", &StateInit);
  }


  //
  // Shutdown
  //
  void SquadFollowTag::Done()
  {
    stateMachine.CleanUp();
  }

}
