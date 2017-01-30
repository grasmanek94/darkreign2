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
#include "tasks_trapidle.h"
#include "taskctrl.h"
#include "team.h"


//
// ~GameTask
//
// Destructor
//
template <> GameTask<TrapObjType, TrapObj>::~GameTask()
{
}


//
// GameProcess
//
// Type specific processing
//
template <> void GameTask<TrapObjType, TrapObj>::GameProcess()
{
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
  StateMachine<TrapIdle> TrapIdle::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class TrapIdle
  //


  //
  // FXCallback
  //
  // Cloaking effect
  //
  Bool TrapIdle::FXCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *)
  {
    TrapIdle *task = TaskCtrl::Promote<Tasks::TrapIdle>(mapObj);

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
  TrapIdle::TrapIdle(GameObj *subject) :
    GameTask<TrapObjType, TrapObj>(staticConfig, subject),
    inst(&stateMachine, "Init"),
    everCloaked(FALSE),
    progressTotal(0.0F)
  {
    // Get the number of seconds to cloak
    F32 time = 5.0F;

    // Work out the maximum cloak rate
    progressRate = (time > 0.0F) ? (1.0F / (time * GameTime::SimTimeInv())) : 1.0F;
  }


  //
  // Save
  //
  void TrapIdle::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));
    StdSave::TypeU32(fScope, "EverCloaked", everCloaked);   
    StdSave::TypeF32(fScope, "ProgressTotal", progressTotal);
  }


  //
  // Load
  //
  void TrapIdle::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
          break;

        case 0x69D02CE3: // "EverCloaked"
          everCloaked = StdLoad::TypeU32(sScope);
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
  void TrapIdle::PostLoad()
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
  Bool TrapIdle::CheckCloak(Bool on)
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
  void TrapIdle::RemoveCloak()
  {
    // Clear flags
    subject->SetFlag(UnitObj::FLAG_CLANDESTINE, FALSE);
    subject->SetFlag(UnitObj::FLAG_INVISIBLE, FALSE);

    // Trigger a puff of smoke
    subject->StartGenericFX(0x3314B14F); // "Cloaking::Finish"
  }


  //
  // Perform task processing
  //
  Bool TrapIdle::Process()
  {
    inst.Process(this);  

    return (FALSE);
  }


  //
  // Task initialization
  //
  void TrapIdle::StateInit()
  {
    // Blend the animation back to the idle animation
    subject->SetAnimation(0x8F651465); // "Default"

    // Already cloaked, or never cloaked and allowed
    if (IsCloaked() || (!everCloaked && CheckCloak(TRUE)))
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
  void TrapIdle::StateVisible()
  {
    // Check for enemies
    subject->Poll();
  }


  //
  // Unit is cloaked
  //
  void TrapIdle::StateCloaked(StateMachineNotify notify)
  {
    switch (notify)
    {
      case SMN_ENTRY:
      {
        // Ignore if already cloaked
        if (!IsCloaked())
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

          // Remember that we have now cloaked
          everCloaked = TRUE;
        }

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
        else
        {
          // Check for enemies
          subject->Poll();
        }

        // See if we need to decloak
        if (CheckCloak(FALSE))
        {
          RemoveCloak();
          NextState(0x01C95681); // "Visible"
        }
        break;
      }
    }
  }


  //
  // Notify this task of an event
  //
  Bool TrapIdle::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
        NextState(0xABAA7B48); // "Init"
        return (TRUE);
    }
    return (GameTask<TrapObjType, TrapObj>::ProcessEvent(event));
  }


  //
  // IsCloaked
  //
  // Is the trap currently cloaked
  //
  Bool TrapIdle::IsCloaked()
  {
    return (subject->GetFlag(UnitObj::FLAG_CLANDESTINE));
  }


  //
  // GetProgress
  //
  // Get the current effect progress
  //
  F32 TrapIdle::GetProgress()
  {
    return (Min<F32>(progressTotal, 1.0F));
  }


  //
  // Initialization
  //
  void TrapIdle::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Add states to the state machine
    stateMachine.AddState("Init", &TrapIdle::StateInit);
    stateMachine.AddState("Visible", &TrapIdle::StateVisible);
    stateMachine.AddState("Cloaked", &TrapIdle::StateCloaked);
  }


  //
  // Shutdown
  //
  void TrapIdle::Done()
  {
    stateMachine.CleanUp();
  }

}
