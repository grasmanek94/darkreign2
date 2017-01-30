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
#include "tasks_unitcloak.h"
#include "taskctrl.h"
#include "team.h"


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
  StateMachine<UnitCloak> UnitCloak::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitCloak
  //

  //
  // FXCallback
  //
  // Cloaking effect
  //
  Bool UnitCloak::FXCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *)
  {
    UnitCloak *task = TaskCtrl::Promote<Tasks::UnitCloak>(mapObj);

    // Do we have a cloaked object
    if (task && task->IsCloaked())
    {
      // Should we display it on this machine
      if (!Team::TestRelation(task->GetSubject()->GetTeam(), Team::GetDisplayTeam(), Relation::ENEMY))
      {
        cbd.meshEffect.percent = task->GetProgress();
        return (FALSE);
      }
    }

    return (TRUE);
  }


  //
  // Constructor
  //
  UnitCloak::UnitCloak(GameObj *subject) :
    GameTask<UnitObjType, UnitObj>(staticConfig, subject),
    inst(&stateMachine, "Init"),
    progressTotal(0.0F)
  {
    // Get the number of seconds to cloak
    F32 time = 10.0F;

    // Work out the maximum cloak rate
    progressRate = (time > 0.0F) ? (1.0F / (time * GameTime::SimTimeInv())) : 1.0F;
  }


  //
  // Save
  //
  void UnitCloak::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));
    StdSave::TypeF32(fScope, "ProgressTotal", progressTotal);
  }


  //
  // Load
  //
  void UnitCloak::Load(FScope *fScope)
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

        default:
          LoadTaskData(sScope);
          break;
      }
    }    
  }


  //
  // Called after all objects are loaded
  //
  void UnitCloak::PostLoad()
  {
    // We can not check the display team here, since it is not setup yet
    // Instead we check it in the callback

    // Do we need to create the cloaked effect
    if (IsCloaked())
    {
      subject->StartGenericFX(0xB52BA0ED, FXCallBack); // "Cloaking::Active"
    }
  }


  //
  // CheckCloak
  //
  // Check if we should turn cloaking on/off
  //
  Bool UnitCloak::CheckCloak(Bool on)
  {
    // Are we trying to cloak
    if (on)
    {
      // Is cloaking available on our team
      if (!subject->GetTeam() || subject->GetTeam()->CloakingAvailable())
      {
        // If no enemies can see us
        if (!subject->SeenByRelation(Relation::ENEMY))
        {
          // Can cloak now
          return (TRUE);
        }
      }

      // Unable to cloak at this time
      return (FALSE);
    }
    else
    {
      // Have we been manually de-cloaked
      if (!subject->GetFlag(UnitObj::FLAG_CLANDESTINE))
      {
        return (TRUE);
      }

      // Is cloaking available on our team
      if (subject->GetTeam() && !subject->GetTeam()->CloakingAvailable())
      {
        return (TRUE);
      }

      // Can we be seen by any enemy
      if (subject->SeenByRelation(Relation::ENEMY))
      {
        // Is one of those an anti-clandestine unit
        if (UnitObjType::FindAntiClandestine(subject->GetTeam(), subject->Position()))
        {
          return (TRUE);
        }
      }

      // No need to de-cloak
      return (FALSE);
    }
  }


  //
  // RemoveCloak
  //
  // Remove cloak if active
  //
  void UnitCloak::RemoveCloak()
  {
    // Clear flags
    subject->SetFlag(UnitObj::FLAG_CLANDESTINE, FALSE);
    subject->SetFlag(UnitObj::FLAG_INVISIBLE, FALSE);

    // Trigger a puff of smoke
    subject->StartGenericFX(0x3314B14F); // "Cloaking::Finish"
  }


  //
  // CheckStatus
  //
  // Check to see if we need to change cloaked status
  //
  Bool UnitCloak::CheckStatus()
  {
    // Are we currently cloaked
    if (inst.Test(0xAE500932)) // "Cloaked"
    {
      // Do we need to uncloak
      if (CheckCloak(FALSE))
      {
        RemoveCloak();
        NextState(0x01C95681); // "Visible"
        return (TRUE);
      }
    }
    else
    {
      // Do we need to cloak
      if (CheckCloak(TRUE))
      {
        NextState(0xAE500932); // "Cloaked"
        return (TRUE);
      }
    }

    // No state change
    return (FALSE);
  }


  //
  // Perform task processing
  //
  Bool UnitCloak::Process()
  {
    inst.Process(this);  

    // Perform type processing
    GameProcess();

    return (FALSE);
  }


  //
  // Task initialization
  //
  void UnitCloak::StateInit()
  {
    // Blend the animation back to the idle animation
    subject->SetAnimation(0x8F651465); // "Default"

    // Now try and cloak
    if (CheckCloak(TRUE))
    {
      NextState(0xAE500932); // "Cloaked"
    }
    else
    {
      NextState(0x01C95681); // "Visible"
    }
  }


  //
  // Unit is visible
  //
  void UnitCloak::StateVisible()
  {
    CheckStatus();
  }


  //
  // Unit is cloaked
  //
  void UnitCloak::StateCloaked(StateMachineNotify notify)
  {
    switch (notify)
    {
      case SMN_ENTRY:
      {
        // Clear progress
        progressTotal = 0.0F;

        // Always trigger the starting fx
        subject->StartGenericFX(0x7F793F24); // "Cloaking::Start"

        // Trigger the cloaking effect for non-enemy teams
        if (!Team::TestDisplayRelation(subject, Relation::ENEMY))
        {
          subject->StartGenericFX(0xB52BA0ED, FXCallBack); // "Cloaking::Active"
        }

        // Set flags
        subject->SetFlag(UnitObj::FLAG_CLANDESTINE, TRUE);
        subject->SetFlag(UnitObj::FLAG_INVISIBLE, TRUE);

        break;
      }

      case SMN_PROCESS:
      {
        // Are we still processing the effect
        if (progressTotal < 1.0F)
        {
          // Process each cycle
          ThinkFast();

          // Increment the counter
          progressTotal += progressRate;
        }

        // See if we need to decloak
        CheckStatus();
        break;
      }
    }
  }


  //
  // Notify this task of an event
  //
  Bool UnitCloak::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
      {
        // Are we cloaked
        if (IsCloaked())
        {
          RemoveCloak();
        }

        NextState(0xABAA7B48); // "Init"
        return (TRUE);
      }
    }
    return (GameTask<UnitObjType, UnitObj>::ProcessEvent(event));
  }


  //
  // IsCloaked
  //
  // Is the unit currently cloaked
  //
  Bool UnitCloak::IsCloaked()
  {
    return (inst.Test(0xAE500932)); // "Cloaked"
  }


  //
  // GetProgress
  //
  // Get the current effect progress
  //
  F32 UnitCloak::GetProgress()
  {
    return (Min<F32>(progressTotal, 1.0F));
  }


  //
  // Initialization
  //
  void UnitCloak::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Add states to the state machine
    stateMachine.AddState("Init", &UnitCloak::StateInit);
    stateMachine.AddState("Visible", &UnitCloak::StateVisible);
    stateMachine.AddState("Cloaked", &UnitCloak::StateCloaked);
  }


  //
  // Shutdown
  //
  void UnitCloak::Done()
  {
    stateMachine.CleanUp();
  }

}
