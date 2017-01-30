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
#include "tasks_unitconstruct.h"
#include "tasks_unitcollect.h"
#include "restoreobj.h"
#include "fx.h"
#include "fx_object.h"
#include "team.h"
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
  StateMachine<UnitConstruct> UnitConstruct::stateMachine;

  // The crc of the animation to play after construction
  static const U32 constructionAnimation = 0x46B53958; // "Constructing"


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitConstruct
  //

  //
  // FXCallback
  //
  // Sets up the construction progress for an FX object
  //
  Bool UnitConstruct::FXCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *)
  {
    UnitConstruct *task = TaskCtrl::Promote<Tasks::UnitConstruct>(mapObj);

    if (task)
    {
      if (task->EffectRequired())
      {
        cbd.meshEffect.percent = task->GetProgress();
        return (FALSE);
      }

      // Clear the effect pointer in the task
      task->effect = NULL;
    }

    return (TRUE);
  }


  //
  // Constructor
  //
  UnitConstruct::UnitConstruct(GameObj *sub) : GameTask<UnitObjType, UnitObj>(staticConfig, sub),
    inst(&stateMachine, "Init"),
    progressTotal(0.0F),
    progressMax(0.0F),
    resourceRemoved(0),
    resourceRemaining(0),
    hitPointsLeft(0),
    constructListTeam(NULL),
    effect(NULL)
  {
  }


  //
  // Save
  //
  void UnitConstruct::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));

    StdSave::TypeF32(fScope, "ProgressTotal", progressTotal);
    StdSave::TypeF32(fScope, "ProgressMax", progressMax);
    StdSave::TypeU32(fScope, "ResourceRemoved", U32(resourceRemoved));
    StdSave::TypeU32(fScope, "ResourceRemaining", U32(resourceRemaining));
    StdSave::TypeU32(fScope, "HitPointsLeft", U32(hitPointsLeft));
    StdSave::TypeReaper(fScope, "Constructor", constructor);

    if (constructListTeam)
    {
      StdSave::TypeString(fScope, "ConstructListTeam", constructListTeam->GetName());
    }
  }
  

  //
  // Load
  //
  void UnitConstruct::Load(FScope *fScope)
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

        case 0xF5B25A80: // "ResourceRemoved"
          resourceRemoved = S32(StdLoad::TypeU32(sScope));
          break;

        case 0xE262D203: // "ResourceRemaining"
          resourceRemaining = S32(StdLoad::TypeU32(sScope));
          break;

        case 0x8BD8D5BA: // "HitPointsLeft"
          hitPointsLeft = S32(StdLoad::TypeU32(sScope));
          break;

        case 0xB73D3720: // "Constructor"
          StdLoad::TypeReaper(sScope, constructor);
          break;

        case 0x9574AB0C: // "ConstructListTeam"
          constructListTeam = Team::Name2Team(StdLoad::TypeString(sScope));
          break;

        default:
          LoadTaskData(sScope);
          break;
      }
    }

    // Create the construction effect
    CreateEffect();
  }


  //
  // Called after all objects are loaded
  //
  void UnitConstruct::PostLoad()
  {
    Resolver::Object<UnitObj, UnitObjType>(constructor);
  }


  //
  // Perform task processing
  //
  Bool UnitConstruct::Process()
  {
    inst.Process(this);  
    ThinkFast();
    return (quit);
  }


  //
  // EffectRequired
  //
  // Is the effect required
  //
  Bool UnitConstruct::EffectRequired()
  {
    return 
    (
      inst.Test(0xABAA7B48) // "Init"
      ||
      inst.Test(0x4E5A04E4) // "Construct"
      ||
      inst.Test(0xF073DD95) // "Paused"
    );
  }


  //
  // CreateEffect
  //
  // Create the construction effect
  //
  void UnitConstruct::CreateEffect()
  {
    // Is the effect required
    if (EffectRequired())
    {
      // Keep a pointer to the effect so it can be terminated if necessary
      effect = subject->StartGenericFX(0x0702F396, FXCallBack); // "Construct::Start"
    }
  }


  //
  // StateInit
  //
  // Start the construction process
  //
  void UnitConstruct::StateInit()
  {
    // Unit should not have been added to the team list yet
    ASSERT(!subject->IsActivelyOnTeam())

    // Get construction time
    F32 time = subject->UnitType()->GetConstructionTime();

    // Setup progress counts
    progressTotal = 0.0F;
    progressMax = (time > 0.0F) ? (1.0F / (time * GameTime::SimTimeInv())) : 1.0F;

    // Grab total resource to be removed
    resourceRemoved = 0;
    resourceRemaining = subject->UnitType()->GetResourceCost();

    // Setup number of hitpoints to add
    hitPointsLeft = subject->MapType()->GetHitPoints() - subject->UnitType()->GetConstructionHitPoints();

    // Register construction of this object
    if ((constructListTeam = subject->GetTeam()) != NULL)
    {
      constructListTeam->RegisterConstructionBegin(subject);
    }

    // Create the construction effect
    CreateEffect();

    NextState(0x4E5A04E4); // "Construct"
  }


  //
  // StateConstruct
  //
  // Process construction
  //
  void UnitConstruct::StateConstruct()
  {  
    // Work out where we should be
    F32 newProgressTotal = progressTotal + progressMax;

    // By default we make progress
    Bool progressed = TRUE;

    // Is there any resource to remove
    if (resourceRemaining)
    {
      // How much resource should we have removed
      S32 resourceTarget = S32(newProgressTotal * F32(subject->UnitType()->GetResourceCost()));

      // How much should we remove this cycle
      S32 resourceToRemove = Min<S32>(resourceTarget - resourceRemoved, resourceRemaining);

      // Do nothing if no resource to remove
      if (resourceToRemove > 0)
      {
        // Attempt to use the resource
        if (!subject->GetTeam() || subject->GetTeam()->RemoveResourceStore(resourceToRemove))
        {
          subject->GetTeam()->ReportResource(resourceToRemove, "resource.spent");

          // Update resource counts
          resourceRemaining -= resourceToRemove;
          resourceRemoved += resourceToRemove;
        }
        else
        {
          // Progress is blocked
          progressed = FALSE;
        }
      }
    }

    // Did we make any progress
    if (progressed)
    {
      // Record the new progress
      progressTotal = newProgressTotal;

      // Are there any hitpoints to add
      if (hitPointsLeft > 0)
      {
        // Get the total number of hitpoints we are adding
        S32 total = subject->MapType()->GetHitPoints() - subject->UnitType()->GetConstructionHitPoints();

        // How many hitpoints should we have left at this point
        S32 target = total - S32(total * progressTotal);

        // Get the number of hitpoints to add this cycle
        if (S32 add = Clamp<S32>(0, hitPointsLeft - target, hitPointsLeft))
        {
          // Add to the object
          subject->ModifyHitPoints(add);

          // Remove from what we have left
          hitPointsLeft -= add;
        }
      }
    }

    ASSERT(resourceRemaining >= 0)

    // Is construction completed
    if (!resourceRemaining && progressTotal >= 1.0F)
    {
      ASSERT(resourceRemoved == S32(subject->UnitType()->GetResourceCost()))

      // Trigger finish FX
      subject->StartGenericFX(0x8E64F310); // "Construct::Finish"

      // If there's a constructor, tell it we're done
      if (constructor.Alive())
      {
        constructor->PostEvent(Task::Event(0xFAC5A90A, subject)); // "UnitConstruct::Completed"
      }

      // Does the subject have a construction animation
      if (subject->HasAnimation(constructionAnimation))
      {
        // Activate animation processing
        subject->Mesh().ActivateAnim(TRUE);

        // Proceed to the next state
        NextState(0x4A61D476); // "Animating"
      }
      else
      {
        // Construction completed
        NextState(0x3369B9CC); // "Completed"
      }

      // Release the line of sight of the subject
      subject->ModifySight(FALSE, FALSE);
    }
  }


  //
  // StatePaused
  //
  // Currently paused
  //
  void UnitConstruct::StatePaused()
  {
  }


  //
  // StateAnimating
  //
  // Playing a post construction animation
  //
  void UnitConstruct::StateAnimating()
  {
  }


  //
  // StateCompleted
  //
  // Construction is complete
  //
  void UnitConstruct::StateCompleted()
  {
    // If this object is a unit on a team, add it to that team
    if (subject->GetTeam())
    {
      subject->GetTeam()->AddToTeam(subject);
    }

    // Does this unit get given armour
    if (subject->MapType()->GetArmourInitial())
    {
      // Strictly only give armour if full hitpoints
      if (subject->GetHitPoints() == subject->MapType()->GetHitPoints())
      {
        subject->ModifyArmour(subject->MapType()->GetArmour());
      }
    }

    U32 companionId = 0;

    // Create a companion unit
    if (subject->UnitType()->GetCompanionType())
    {
      // Just to be safe
      if (!subject->GetBoardManager()->InUse())
      {
        // Create the companion
        UnitObj &companion = subject->UnitType()->GetCompanionType()->Spawn(subject);

        // Does the subject store resource
        if (subject->HasProperty(0xAE95DF36)) // "Ability::StoreResource"
        {
          // Is this a collector
          if (Tasks::UnitCollect *task = TaskCtrl::PromoteIdle<Tasks::UnitCollect>(&companion))
          {
            task->SetStorageObject(subject, TRUE);
          }
        }

        // Set the id for the AI notification
        companionId = companion.Id();
      }
    }

    // Notify the player that the unit is available
    subject->NotifyPlayer(0xFAC5A90A, companionId); // "UnitConstruct::Completed"

    // "Tasks::UnitConstruct::Completed"
    if (subject->GetTeam())
    {
      subject->GetTeam()->GetRadio().Trigger(0xF5992178, Radio::Event(subject)); 
    }

    // Register construction of this object
    if (constructListTeam)
    {
      constructListTeam->RegisterConstructionEnd(subject);
    }

    // Completed
    Quit();
  }


  //
  // Notify this task of an event
  //
  Bool UnitConstruct::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
      {
        // Notify the constructor
        if (constructor.Alive())
        {
          constructor->PostEvent(Task::Event(0x160E89B9, subject)); // "UnitConstruct::Interrupted"
        }

        if (effect)
        {
          delete effect;
          effect = NULL;
        }
        return (TRUE);
      }

      case 0x68604C50: // "UnitBuild::SetConstructor"
      {
        if (event.object.Alive())
        {
          constructor = Promote::Object<UnitObjType, UnitObj>(event.object);
        }
        return (TRUE);
      }

      case MapObjNotify::AnimationDone:
      {
        switch (event.param1)
        {
          case constructionAnimation:
          {
            NextState(0x3369B9CC); // "Completed"
            break;
          }
        }
        return (TRUE);
      }

      case RestoreObjNotify::BlockHitPoints:
      {
        // Get the total number of hitpoints we are adding
        S32 total = subject->MapType()->GetHitPoints() - subject->UnitType()->GetConstructionHitPoints();

        // How many hitpoints should we have at this point
        S32 expected = subject->UnitType()->GetConstructionHitPoints() + S32(total * GetProgress());

        // Block if reached the expected hitpoint value
        return (subject->GetHitPoints() >= expected);
      }

      case TaskNotify::CanPause:
      {
        return (!inst.NextState() && inst.Test(0x4E5A04E4)); // "Construct"
      }

      case TaskNotify::CanUnpause:
      {
        return (!inst.NextState() && inst.Test(0xF073DD95)); // "Paused"
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
          NextState(0x4E5A04E4); // "Construct"
          return (TRUE);
        }
        return (FALSE);
      }

      case 0x834E6A12: // "Debug::Immediate"
      {
        progressTotal = 1.0F;
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
  Bool UnitConstruct::Retrieve(U32 id, RetrievedData &data)
  {
    switch (id)
    {
      case TaskRetrieve::Progress:
      {
        data.f1 = GetProgress();
        data.u1 = GetNameCrc();
        return (TRUE);
      }

      case TaskRetrieve::Info:
      {
        // Are we constructing
        if (inst.Test(0x4E5A04E4)) // "Construct"
        {
          data.s2 = TRANSLATE(("#game.client.hud.tasks.construct.process"));
          return (TRUE);
        }

        if (inst.Test(0xF073DD95)) // "Paused"
        {
          data.s2 = TRANSLATE(("#game.client.hud.tasks.construct.paused"));
          return (TRUE);
        }

        return (FALSE);
      }
    }

    return (GameTask<UnitObjType, UnitObj>::Retrieve(id, data));
  }


  //
  // GetProgress
  //
  // Get the progress percentage
  //
  F32 UnitConstruct::GetProgress()
  {
    return (Clamp<F32>(0.0F, progressTotal, 1.0F));
  }


  //
  // GetResourceRemoved
  //
  // Returns the exact amount of resource removed
  //
  S32 UnitConstruct::GetResourceRemoved()
  {
    return (resourceRemoved);
  }


  //
  // Setup
  //
  // Setup an object prior to the assignment of a construction task
  //
  void UnitConstruct::Setup(UnitObj *unit)
  {
    ASSERT(unit)

    // Setup the initial hitpoints
    if (!unit->GetHitPoints())
    {
      unit->ModifyHitPoints(unit->UnitType()->GetConstructionHitPoints());
    }

    // Does this unit have a construction animation
    if (unit->HasAnimation(constructionAnimation))
    {
      // Set the animation
      unit->SetAnimation(constructionAnimation, FALSE, FALSE);
    }

    // Restrict the line of sight before the first update
    unit->ModifySight(TRUE, TRUE);
  }


  //
  // Initialization
  //
  void UnitConstruct::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Add states to the state machine
    stateMachine.AddState("Init", &UnitConstruct::StateInit);
    stateMachine.AddState("Construct", &UnitConstruct::StateConstruct);
    stateMachine.AddState("Paused", &UnitConstruct::StatePaused);
    stateMachine.AddState("Animating", &UnitConstruct::StateAnimating);
    stateMachine.AddState("Completed", &UnitConstruct::StateCompleted);
  }


  //
  // Shutdown
  //
  void UnitConstruct::Done()
  {
    stateMachine.CleanUp();
  }
}
