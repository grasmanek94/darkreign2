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
#include "tasks_unitrefinery.h"
#include "taskctrl.h"
#include "resolver.h"
#include "babel.h"


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
  StateMachine<UnitRefinery> UnitRefinery::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitRefinery
  //

  
  //
  // FXCallback
  //
  // Returns TRUE when processing is completed
  //
  Bool UnitRefinery::FXCallBack(MapObj *mapObj, FX::CallBackData &, void *)
  {
    if (UnitRefinery *task = TaskCtrl::Promote<Tasks::UnitRefinery>(mapObj))
    {
      return (task->target.Dead());
    }

    return (TRUE);
  }


  //
  // Constructor
  //
  UnitRefinery::UnitRefinery(GameObj *sub) : GameTask<UnitObjType, UnitObj>(staticConfig, sub),
    inst(&stateMachine, "Idle")
  {
  }


  //
  // Save
  //
  void UnitRefinery::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));  
    StdSave::TypeReaper(fScope, "Target", target);
  }


  //
  // Load
  //
  void UnitRefinery::Load(FScope *fScope)
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
          StdLoad::TypeReaper(sScope, target);
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
  void UnitRefinery::PostLoad()
  {
    Resolver::Object<UnitObj, UnitObjType>(target);
  }


  //
  // Perform task processing
  //
  Bool UnitRefinery::Process()
  {
    inst.Process(this);  

    return (quit);
  }


  //
  // Idle
  //
  void UnitRefinery::StateIdle()
  {
    if (target.Alive())
    {
      // Animate the lights
      subject->Mesh().PollActivateTexAnim();
    }
  }


  //
  // ProcessEvent
  //
  // Process notifications
  //
  Bool UnitRefinery::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case Movement::Notify::UnitEntered:
      {
        if (event.object.Alive())
        {
          // Save the target
          target = Promote::Object<UnitObjType, UnitObj>(event.object);

          // Start the processing animation
          subject->SetAnimation(0xBDF7C019); // "Process"

          // Create the effect
          subject->StartGenericFX(0x8563F8AF, FXCallBack); // "Refinery::Process"
          
          return (TRUE);
        }

        return (FALSE);
      }

      case Movement::Notify::UnitLeaving:
      {
        // Clear the target reaper
        target = NULL;

        return (TRUE);
      }

      case MapObjNotify::AnimationDone:
      {
        switch (event.param1)
        {
          case 0xBDF7C019: // "Process"
          {
            if (target.Alive())
            {
              // Start the processing animation
              subject->SetAnimation(0xBDF7C019); // "Process"
            }
            break;
          }
        }
          
        return (TRUE);
      }
    }
    return (GameTask<UnitObjType, UnitObj>::ProcessEvent(event));
  }


  //
  // Retrieve
  //
  // Retrieve data from the task, return TRUE if filled
  //
  Bool UnitRefinery::Retrieve(U32 id, RetrievedData &data)
  {
    switch (id)
    {
      case TaskRetrieve::Info:
      {
        if (target.Alive())
        {
          data.s2 = TRANSLATE(("#game.client.hud.tasks.refinery.process"));
          return (TRUE);
        }
        return (FALSE);
      }
    }

    return (GameTask<UnitObjType, UnitObj>::Retrieve(id, data));
  }


  //
  // Initialization
  //
  void UnitRefinery::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Add states to the state machine
    stateMachine.AddState("Idle", &StateIdle);
  }


  //
  // Shutdown
  //
  void UnitRefinery::Done()
  {
    stateMachine.CleanUp();
  }
}
