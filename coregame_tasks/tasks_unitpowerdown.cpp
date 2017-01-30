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
#include "tasks_unitpowerdown.h"
#include "team.h"
#include "wallobj.h"
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
  StateMachine<UnitPowerDown> UnitPowerDown::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitPowerDown
  //

  //
  // FXCallback
  //
  // Powered down effect
  //
  Bool UnitPowerDown::FXCallBack(MapObj *mapObj, FX::CallBackData &, void *)
  {
    return (!mapObj->SendEvent(Task::Event(0x948A8F93))); // "UnitPowerDown::EffectProcess"
  }


  //
  // Constructor
  //
  UnitPowerDown::UnitPowerDown(GameObj *sub) 
  : GameTask<UnitObjType, UnitObj>(staticConfig, sub),
    inst(&stateMachine, "Init"),
    progressTotal(0.0F),
    progressMax(0.0F)
  {
  }


  //
  // Save
  //
  void UnitPowerDown::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));
    StdSave::TypeF32(fScope, "ProgressTotal", progressTotal);
    StdSave::TypeF32(fScope, "ProgressMax", progressMax);
  }


  //
  // Load
  //
  void UnitPowerDown::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
          break;

        case 0x97522DEF: // "ProgressTotal"
          progressTotal = StdLoad::TypeF32(sScope);
          break;

        case 0xDA11482A: // "ProgressMax"
          progressMax = StdLoad::TypeF32(sScope);
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
  void UnitPowerDown::PostLoad()
  {
  }


  //
  // Notify this task of an event
  //
  Bool UnitPowerDown::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
      {
        inst.Set(0xABAA7B48); // "Init"
        return (TRUE);
      }

      case 0x57BE223A: // "PowerUp"
      {
        if (TestState(0x7B807046)) // "PoweredDown"
        {
          NextState(0x57BE223A); // "PowerUp"
        }
        return (TRUE);
      }

      case 0x948A8F93: // "UnitPowerDown::EffectProcess"
      {
        return 
        (
          inst.Test(0x7B807046) // "PoweredDown"
          ||
          inst.Test(0x57BE223A) // "PowerUp"
        );
      }

      case 0xDC8CB3E0: // "UnitPowerDown::CanPowerUp"
      {
        return (inst.Test(0x7B807046)); // "PoweredDown"
      }
    }
    return (GameTask<UnitObjType, UnitObj>::ProcessEvent(event));
  }


  //
  // Retrieve
  //
  // Retrieve data from the task, return TRUE if filled
  //
  Bool UnitPowerDown::Retrieve(U32 id, RetrievedData &data)
  {
    switch (id)
    {
      case TaskRetrieve::Progress:
      {
        // "PowerDown" || "PowerUp" 
        if (inst.Test(0xD177BA84) || inst.Test(0x57BE223A))
        {
          data.f1 = progressTotal;
          data.u1 = GetNameCrc();
          return (TRUE);
        }

        return (FALSE);
      }

      case TaskRetrieve::Info:
      {
        if (inst.Test(0x7B807046)) // "PoweredDown"
        {
          data.s2 = TRANSLATE(("#game.client.hud.tasks.powerdown.offline"));
          return (TRUE);
        }
        return (FALSE);
      }
    }

    return (GameTask<UnitObjType, UnitObj>::Retrieve(id, data));
  }


  //
  // Perform task processing
  //
  Bool UnitPowerDown::Process()
  {
    inst.Process(this);
    return (quit);
  }


  //
  // Init
  //
  // Setup recycle timer
  //
  void UnitPowerDown::StateInit()
  {
    NextState(0xD177BA84); // "PowerDown"
  }


  //
  // PowerDown
  //
  void UnitPowerDown::StatePowerDown(StateMachineNotify notify)
  {
    // Process once per cycle in this state
    ThinkFast();

    switch (notify)
    {
      case SMN_ENTRY:
      {
        // Get power down time
        F32 time = subject->UnitType()->GetPowerDownTime();

        // Setup progress counts
        progressTotal = 1.0F;
        progressMax = (time > 0.0F) ? (1.0F / (time * GameTime::SimTimeInv())) : 1.0F;

        break;
      }

      case SMN_PROCESS:
      {
        // Apply progress
        progressTotal -= progressMax;

        // Is operation complete
        if (progressTotal <= 0.0F)
        {
          // Switch to the powered down state
          NextState(0x7B807046); // "PoweredDown"
        }

        break;
      }
    }
  }


  //
  // PoweredDown
  //
  void UnitPowerDown::StatePoweredDown(StateMachineNotify notify)
  {
    switch (notify)
    {
      case SMN_ENTRY:
      {
        // "Engine::PoweredDown::Process"
        subject->StartFX(0x8D1559F5, FXCallBack);

        // Remove the team list
        if (subject->GetTeam())
        {
          subject->GetTeam()->RemoveFromTeam(subject);
        }

        // Turn off walls
        WallObjType::Toggle(subject, FALSE);

        // Restrict sight
        subject->ModifySight(TRUE, FALSE);

        break;
      }
    }
  }


  //
  // PowerUp
  //
  void UnitPowerDown::StatePowerUp(StateMachineNotify notify)
  {
    // Process once per cycle in this state
    ThinkFast();

    switch (notify)
    {
      case SMN_ENTRY:
      {
        // Get power up time
        F32 time = subject->UnitType()->GetPowerUpTime();

        // Setup progress counts
        progressTotal = 0.0F;
        progressMax = (time > 0.0F) ? (1.0F / (time * GameTime::SimTimeInv())) : 1.0F;

        break;
      }

      case SMN_PROCESS:
      {
        // Apply progress
        progressTotal += progressMax;

        // Is operation complete
        if (progressTotal >= 1.0F)
        {
          // Add back to the team lists
          if (subject->GetTeam())
          {
            subject->GetTeam()->AddToTeam(subject);
          }
            
          // Reactivate walls
          WallObjType::Toggle(subject, TRUE);

          // Restore sight
          subject->ModifySight(FALSE, FALSE);
          
          Quit();
        }
        break;
      }
    }
  }
 

  //
  // Initialization
  //
  void UnitPowerDown::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Add states to the state machine
    stateMachine.AddState("Init", &StateInit);
    stateMachine.AddState("PowerDown", &StatePowerDown);
    stateMachine.AddState("PoweredDown", &StatePoweredDown);
    stateMachine.AddState("PowerUp", &StatePowerUp);
  }


  //
  // Shutdown
  //
  void UnitPowerDown::Done()
  {
    stateMachine.CleanUp();
  }
}
