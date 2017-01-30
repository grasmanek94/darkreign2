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
#include "tasks_unitupgrade.h"
#include "taskctrl.h"
#include "tasks_unitconstruct.h"
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
  StateMachine<UnitUpgrade> UnitUpgrade::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitUpgrade
  //


  //
  // Constructor
  //
  UnitUpgrade::UnitUpgrade(GameObj *sub) 
  : GameTask<UnitObjType, UnitObj>(staticConfig, sub),
    inst(&stateMachine, "Upgrade")
  {
  }


  //
  // Constructor
  //
  UnitUpgrade::UnitUpgrade(GameObj *sub, UnitObj *upgrade) : 
    GameTask<UnitObjType, UnitObj>(staticConfig, sub),
    inst(&stateMachine, "Upgrade"),
    upgrade(upgrade)
  {
  }


  //
  // Save
  //
  void UnitUpgrade::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));  
    StdSave::TypeReaper(fScope, "Upgrade", upgrade);
  }


  //
  // Load
  //
  void UnitUpgrade::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
          break;

        case 0xE1977EAE: // "Upgrade"
          StdLoad::TypeReaper(sScope, upgrade);
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
  void UnitUpgrade::PostLoad()
  {
    Resolver::Object<UnitObj, UnitObjType>(upgrade);
  }


  //
  // GetProgress
  //
  // Returns the percent complete
  //
  F32 UnitUpgrade::GetProgress()
  {
    if (upgrade.Alive())
    {
      return (upgrade->GetConstructionPercentage());
    }

    return (0.0F);
  }


  //
  // Perform task processing
  //
  Bool UnitUpgrade::Process()
  {
    // Is the upgrade still being constructed
    if (GetConstructTask())
    {
      // Process the current state
      inst.Process(this);  
    }
    else
    {
      if (upgrade.Alive())
      {
        // Tell the team that we built something
        subject->GetTeam()->ReportConstruction(upgrade);
      }

      // Tell the player that the upgrade is completed
      subject->NotifyPlayer(0xC3C43161); // "UnitUpgrade::Completed"

      // Finished
      Quit();
    }

    return (quit);
  }


  //
  // GetConstructTask
  //
  // Get the construct task of the upgrade, or NULL if finished
  //
  Tasks::UnitConstruct * UnitUpgrade::GetConstructTask()
  {
    if (upgrade.Alive())
    {
      return (TaskCtrl::Promote<Tasks::UnitConstruct>(upgrade));
    }
    
    return (NULL);
  }


  //
  // Upgrade
  //
  void UnitUpgrade::StateUpgrade()
  {
    // Animate the lights
    subject->Mesh().PollActivateTexAnim();
  }


  //
  // Paused
  //
  void UnitUpgrade::StatePaused(StateMachineNotify notify)
  {
    switch (notify)
    {
      case SMN_ENTRY:
        upgrade->SendEvent(TaskNotify::Pause);
        break;

      case SMN_EXIT:
        upgrade->SendEvent(TaskNotify::Unpause);
        break;
    }
  }


  //
  // Process an event
  //
  Bool UnitUpgrade::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case TaskNotify::CanPause:
      {         
        return (inst.Test(0xE1977EAE) && upgrade.Alive() && upgrade->SendEvent(event)); // "Upgrade"
      }

      case TaskNotify::CanUnpause:
      {
        return (inst.Test(0xF073DD95) && upgrade.Alive() && upgrade->SendEvent(event)); // "Paused"
      }

      case TaskNotify::Pause:
      {      
        if (ProcessEvent(TaskNotify::CanPause))
        {
          NextState(0xF073DD95); // "Paused"
          return (TRUE);
        }
        return (FALSE);
      }

      case TaskNotify::Unpause:
      {
        if (ProcessEvent(TaskNotify::CanUnpause))
        {
          NextState(0xE1977EAE); // "Upgrade"
          return (TRUE);
        }
        return (FALSE);
      }
    }

    return (GameTask<UnitObjType, UnitObj>::ProcessEvent(event));
  }


  //
  // Retrieve
  //
  // Retrieve data from the task, return TRUE if filled
  //
  Bool UnitUpgrade::Retrieve(U32 id, RetrievedData &data)
  {
    switch (id)
    {
      case TaskRetrieve::Progress:
        data.u1 = GetNameCrc();
        data.f1 = GetProgress();
        return (TRUE);

      case TaskRetrieve::Info:
      {
        // Are we upgrading
        if (inst.Test(0xE1977EAE)) // "Upgrade"
        {
          data.s2 = TRANSLATE(("#game.client.hud.tasks.upgrade.process"));
          return (TRUE);
        }

        if (inst.Test(0xF073DD95)) // "Paused"
        {
          data.s2 = TRANSLATE(("#game.client.hud.tasks.upgrade.paused"));
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
  void UnitUpgrade::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Add states to the state machine
    stateMachine.AddState("Upgrade", &UnitUpgrade::StateUpgrade);
    stateMachine.AddState("Paused", &UnitUpgrade::StatePaused);
  }


  //
  // Shutdown
  //
  void UnitUpgrade::Done()
  {
    stateMachine.CleanUp();
  }

}
