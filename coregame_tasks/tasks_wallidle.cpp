///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
//
// 14-NOV-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "tasks_wallidle.h"
#include "team.h"
#include "resolver.h"
#include "taskctrl.h"


///////////////////////////////////////////////////////////////////////////////
//
// Template GameTask<WallObjType, WallObj>
//


//
// ~GameTask
//
// Destructor
//
template <> GameTask<WallObjType, WallObj>::~GameTask()
{
}


//
// GameProcess
//
// Type specific processing
//
template <> void GameTask<WallObjType, WallObj>::GameProcess()
{
  // Walls can be gun turrets etc
  Tactical::Process(this, GetTaskTable(), subject);
}


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
  StateMachine<WallIdle> WallIdle::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class WallIdle
  //


  //
  // Constructor
  //
  WallIdle::WallIdle(GameObj *sub) 
  : GameTask<WallObjType, WallObj>(staticConfig, sub),
    inst(&stateMachine, "Init")
  {
  }


  //
  // Destructor
  //
  WallIdle::~WallIdle() 
  {
    linkTargets.Clear();
  }


  //
  // Save
  //
  void WallIdle::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));
    StdSave::TypeReaperList(fScope, "LinkTargets", linkTargets);
  }


  //
  // Load
  //
  void WallIdle::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
          break;

        case 0x1AE12DB3: // "LinkTargets"
          StdLoad::TypeReaperList(sScope, linkTargets);
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
  void WallIdle::PostLoad()
  {
    Resolver::ObjList<WallObj, WallObjType, WallObjListNode>(linkTargets);
  }


  //
  // Perform task processing
  //
  Bool WallIdle::Process()
  {
    inst.Process(this);

    GameProcess();

    return (quit);
  }


  //
  // Initial state
  //
  void WallIdle::StateInit()
  {
    NextState(0x4DA8F2D8); // "Link"
  }


  //
  // Link to each wall
  //
  void WallIdle::StateLink()
  {
    // Step through each of the remaining targets
    for (WallObjList::Iterator i(&linkTargets); *i; ++i)
    {
      // Is this one alive
      if (WallObj *wall = (*i)->GetPointer())
      {
        // Attempt to form the link
        switch (subject->FormLink(wall))
        {
          // A cell in the link is claimed by a mobile unit
          case WallObj::TR_CLAIMED_M:
          {
            UnitEvacuate evacuator;

            // Setup a unit evacuateor
            subject->SetupEvacuator(wall, evacuator);

            // Attempt to clear the area
            if (evacuator.Process(subject->GetTeam()) != UnitEvacuate::MOBILE)
            {
              // Give up on this link
              (*i)->Clear();
            }

            break;
          }

          // Either successful or a result we can not change
          default:
          {
            (*i)->Clear();
            break;
          }
        }
      }
    }

    // Purge the list of dead targets
    linkTargets.PurgeDead();

    // If no more targets, be idle
    if (!linkTargets.GetCount())
    {
      NextState(0x793360CC); // "Idle"
    }
  }

  
  //
  // Finished linking, now idle
  //
  void WallIdle::StateIdle()
  {
  }


  //
  // Notify this task of an event
  //
  Bool WallIdle::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case 0x137985A7: // "GameObj::NonIdleProcess"
      {
        // Continue processing if linking
        if (inst.Test(0x4DA8F2D8)) // "Link"
        {
          StateLink();
        }
       
        return (TRUE);       
      }
    }
    return (GameTask<WallObjType, WallObj>::ProcessEvent(event));
  }


  //
  // SetLinkTargets
  //
  // Set targets and enter init state
  //
  void WallIdle::SetLinkTargets(const WallObjList &list)
  {
    // Set the list
    linkTargets.Dup(list);

    // Change state
    NextState(0xABAA7B48); // "Init"
  }


  //
  // Initialization
  //
  void WallIdle::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Add states to the state machine
    stateMachine.AddState("Init", &StateInit);
    stateMachine.AddState("Link", &StateLink);
    stateMachine.AddState("Idle", &StateIdle);
  }


  //
  // Shutdown
  //
  void WallIdle::Done()
  {
    stateMachine.CleanUp();
  }
}
