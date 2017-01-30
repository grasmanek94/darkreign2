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
#include "tasks_unitrecycle.h"
#include "taskctrl.h"
#include "common.h"
#include "team.h"
#include "fx.h"
#include "fx_object.h"
#include "wallobj.h"
#include "babel.h"
#include "console.h"

#include "strategic_base.h"
#include "strategic_object.h"


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
  StateMachine<UnitRecycle> UnitRecycle::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitRecycle
  //

  //
  // FXCallback
  //
  // Sets up the progress for an FX object
  //
  Bool UnitRecycle::FXCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *)
  {
    if (UnitRecycle *task = TaskCtrl::Promote<Tasks::UnitRecycle>(mapObj))
    {
      cbd.meshEffect.percent = 1.0F - task->GetProgress();
      
      return (FALSE);
    }

    return (TRUE);
  }


  //
  // Constructor
  //
  UnitRecycle::UnitRecycle(GameObj *sub) : 
    GameTask<UnitObjType, UnitObj>(staticConfig, sub),
    inst(&stateMachine, "Init"),
    refund(0), timeTotal(1.0F), timeCurrent(1.0F), cancel(FALSE)
  {
  }


  //
  // Constructor
  //
  UnitRecycle::UnitRecycle(GameObj *sub, S32 refund, F32 timeTotal, F32 timeCurrent, Bool cancel = FALSE) : 
    GameTask<UnitObjType, UnitObj>(staticConfig, sub),
    inst(&stateMachine, "Init"),
    refund(refund), 
    timeTotal(timeTotal), 
    timeCurrent(timeCurrent),
    cancel(cancel)
  {
  }


  //
  // Save
  //
  void UnitRecycle::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));
    
    StdSave::TypeU32(fScope, "Refund", U32(refund));
    StdSave::TypeF32(fScope, "TimeTotal", timeTotal);
    StdSave::TypeF32(fScope, "TimeCurrent", timeCurrent);
    StdSave::TypeF32(fScope, "ProgressTotal", progressTotal);
    StdSave::TypeF32(fScope, "ProgressMax", progressMax);
    StdSave::TypeU32(fScope, "Cancel", cancel);
  }


  //
  // Load
  //
  void UnitRecycle::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
          break;

        case 0x8DC42CB8: // "Refund"
          refund = S32(StdLoad::TypeU32(sScope));
          break;

        case 0x04869E21: // "TimeTotal"
          timeTotal = StdLoad::TypeF32(sScope);
          break;

        case 0xD7DA8832: // "TimeCurrent"
          timeCurrent = StdLoad::TypeF32(sScope);
          break;

        case 0x97522DEF: // "ProgressTotal"
          progressTotal = StdLoad::TypeF32(sScope);
          break;

        case 0xDA11482A: // "ProgressMax"
          progressMax = StdLoad::TypeF32(sScope);
          break;

        case 0x3E412225: // "Cancel"
          cancel = StdLoad::TypeU32(sScope);
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
  void UnitRecycle::PostLoad()
  {
    if (!inst.Test(0xABAA7B48)) // "Init"
    {
      // Create the effect
      subject->StartGenericFX(0x0894BA62, FXCallBack); // "Recycle::Start"
    }
  }


  //
  // Perform task processing
  //
  Bool UnitRecycle::Process()
  {
    inst.Process(this);
    ThinkFast();
    return (quit);
  }


  //
  // Init
  //
  // Setup recycle timer
  //
  void UnitRecycle::StateInit()
  {
    // Setup progress counts
    progressTotal = (timeTotal > 0.0F) ? timeCurrent / timeTotal : 0.0F;
    progressMax = (timeCurrent > 0.0F) ? (progressTotal / (timeCurrent * GameTime::SimTimeInv())) : 1.0F;

    // Tell constructors to cancel construction
    subject->SendEvent(Task::Event(0x70C4B225), TRUE); // "UnitConstructor::Cancel"

    // Remove from the team
    if (subject->GetTeam())
    {
      subject->GetTeam()->RemoveFromTeam(subject);
    }

    // Create the effect
    subject->StartGenericFX(0x0894BA62, FXCallBack); // "Recycle::Start"
    
    // Turn off walls
    WallObjType::Toggle(subject, FALSE);

    // Proceed
    NextState(0xF8AB3A7A); // "Recycle"
  }


  //
  // Recycle
  //
  // Check timer and refund resources once recycled
  //
  void UnitRecycle::StateRecycle()
  {
    // Apply progress
    progressTotal -= progressMax;

    // Has recycling finished
    if (progressTotal <= 0.0F)
    {
      // Refund the resources
      if (subject->GetTeam() && refund > 0)
      {
        // Calculate the total refund
        U32 totalRefund = U32(subject->UnitType()->GetRecyclePercentage() * refund);

        // Add to the team
        subject->GetTeam()->AddResourceStore(totalRefund);

        // Report the resource type
        subject->GetTeam()->ReportResource(totalRefund, "resource.recycled");

        // Generate a message
        if (Team::GetDisplayTeam() == subject->GetTeam())
        {
          CON_MSG((TRANSLATE(("#game.messages.recyclerefund", 2, subject->GetUpgradedUnit().GetDesc(), totalRefund))));
        }
      }

      // Trigger finish FX
      subject->StartGenericFX(0x2062BAAD, NULL, TRUE); // "Recycle::Finish"

      // Did this building consume its constructor
      if 
      (
        subject->UnitType()->GetConstructorType() && 
        !subject->UnitType()->GetConstructorType()->GetIsFacility() &&
        subject->UnitType()->GetConstructorConsume()
      )
      {
        // Ensure the constructors resources are initialized
        subject->UnitType()->GetConstructorType()->InitializeResources();

        // Create a fresh new constructor
        UnitObj *unit = subject->UnitType()->GetConstructorType()->SpawnClosest
        (
          subject->Position(), subject->GetTeam()
        );

        // If this team is controlled by AI then assign the 
        // constructor to the primary base (if there is one)
        if (unit && unit->GetTeam() && unit->GetTeam()->IsAI())
        {
          Strategic::Object *object = unit->GetTeam()->GetStrategicObject();
          if (object)
          {
            Strategic::Base *base = object->GetBaseManager().GetPrimaryBase();

            if (base)
            {
              base->AddUnit(unit);
              base->AddConstructor(unit);
            }
          }
        }
      }

      // Remove the object
      subject->MarkForDeletion();

      // Remove boarded object
      if (subject->UnitType()->CanBoard() && subject->GetBoardManager()->InUse())
      {
        subject->GetBoardManager()->GetUnitObj()->SelfDestruct();
      }

      // Recycle completed
      Quit();
    }
  }


  //
  // StateDecycle
  //
  // Recycling has been cancelled
  // 
  void UnitRecycle::StateDecycle()
  {
    // Apply progress
    progressTotal += progressMax;

    // Has decycling finished
    if (progressTotal >= 1.0F)
    {
      // Add back to the team
      if (subject->GetTeam())
      {
        subject->GetTeam()->AddToTeam(subject);
      }

      Quit();
    }  
  }


  //
  // Notify this task of an event
  //
  Bool UnitRecycle::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
        return (TRUE);

      case TaskNotify::CanCancel:
        return (FALSE);
        //return (cancel && inst.Test(0xF8AB3A7A)); // "Recycle"

      case TaskNotify::Cancel:
      {
        if (ProcessEvent(TaskNotify::CanCancel))
        {
          NextState(0xB9EBA067); // "Decycle"
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
  Bool UnitRecycle::Retrieve(U32 id, RetrievedData &data)
  {
    switch (id)
    {
      case TaskRetrieve::Progress:
        data.u1 = GetNameCrc();
        data.f1 = GetProgress();
        return (TRUE);

      case TaskRetrieve::Info:
      {
        if (inst.Test(0xB9EBA067)) // "Decycle"
        {
          data.s2 = TRANSLATE(("#game.client.hud.tasks.recycle.cancel"));
        }
        else
        {
          data.s2 = TRANSLATE(("#game.client.hud.tasks.recycle.process"));
        }
        return (TRUE);
      }
    }

    return (GameTask<UnitObjType, UnitObj>::Retrieve(id, data));
  }


  //
  // GetProgress
  //
  // Get the progress percentage
  //
  F32 UnitRecycle::GetProgress()
  {
    return (Clamp<F32>(0.0F, progressTotal, 1.0F));
  }


  //
  // Initialization
  //
  void UnitRecycle::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Add states to the state machine
    stateMachine.AddState("Init", &UnitRecycle::StateInit);
    stateMachine.AddState("Recycle", &UnitRecycle::StateRecycle);
    stateMachine.AddState("Decycle", &UnitRecycle::StateDecycle);
  }


  //
  // Shutdown
  //
  void UnitRecycle::Done()
  {
    stateMachine.CleanUp();
  }
}
