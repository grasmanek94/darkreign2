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
#include "tasks_restoremobile.h"
#include "tasks_unitgorestore.h"
#include "movement_pathfollow.h"
#include "taskctrl.h"
#include "resolver.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Tasks
//
namespace Tasks
{
  //#define LOG_LOCAL LOG_DIAG
  #define LOG_LOCAL(x)


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // State machine
  StateMachine<RestoreMobile> RestoreMobile::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class RestoreMobile
  //


  //
  // Constructor
  //
  RestoreMobile::RestoreMobile(GameObj *subject) :
    GameTask<RestoreObjType, RestoreObj>(staticConfig, subject),
    inst(&stateMachine, "Init")
  {
    LOG_LOCAL(("Constructor1"));
  }


  //
  // Constructor
  //
  RestoreMobile::RestoreMobile(GameObj *subject, UnitObj *target) :
    GameTask<RestoreObjType, RestoreObj>(staticConfig, subject),
    inst(&stateMachine, "Init"), target(target)
  {
    LOG_LOCAL(("Constructor2"));
  }


  //
  // Save
  //
  void RestoreMobile::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));
    StdSave::TypeReaper(fScope, "Target", target);
    StdSave::TypeVector(fScope, "Destination", destination);
    moveHandle.SaveState(fScope->AddFunction("MoveHandle"));
  }


  //
  // Load
  //
  void RestoreMobile::Load(FScope *fScope)
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

        case 0xA302E408: // "Destination"
          StdLoad::TypeVector(sScope, destination);
          break;

        case 0xD3D101D2: // "MoveHandle"
          moveHandle.LoadState(sScope);
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
  void RestoreMobile::PostLoad()
  {
    Resolver::Object<UnitObj, UnitObjType>(target);
  }


  //
  // Perform task processing
  //
  Bool RestoreMobile::Process()
  {
    inst.Process(this);  
    return (quit);
  }


  //
  // ValidTarget
  //
  // Do we have a valid target that needs restoration
  //
  Bool RestoreMobile::ValidTarget()
  {
    return (target.Alive() && target->OnMap() && subject->RestoreRequired(target));
  }


  //
  // GetRestorationDestination
  //
  // Returns the restoration destination position for the current target
  //
  Vector RestoreMobile::GetRestorationDestination()
  {
    ASSERT(ValidTarget())

    if (target->GetFootInstance())
    {
      // Move to the closest point on the footprint
      Point<S32> pos(subject->GetCellX(), subject->GetCellZ());
      target->GetFootInstance()->ClampToFringe(pos);
      return (Vector(WorldCtrl::CellToMetresX(pos.x), 0.0F, WorldCtrl::CellToMetresZ(pos.z)));
    }

    // Move to the centre of the object
    return (target->Position());
  }


  //
  // TargetMoved
  //
  // Has the target moved significantly from it's original location
  //
  Bool RestoreMobile::TargetMoved()
  {
    // Has the target moved ourside our restoration range
    return
    (
      Vector(GetRestorationDestination() - destination).MagnitudeXZ() > subject->RestoreType()->Distance()
    );
  }


  //
  // TargetInRange
  //
  // Is the target withing restoration range
  //
  Bool RestoreMobile::TargetInRange(Bool thinkFast)
  {
    // Get the distance to the target
    F32 range = subject->RestoreType()->Distance();
    F32 distance = Vector(GetRestorationDestination() - subject->Position()).MagnitudeXZ();

    // Think faster when target is close
    if (thinkFast && distance < range * 2)
    {
      ThinkFast();
    }

    return (distance < range);
  }


  //
  // MoveIntoRange
  //
  // Should we move to reach target
  //
  Bool RestoreMobile::MoveIntoRange()
  {
    // If task came from an order, we can always move into range
    if (GetFlags() & TF_FROM_ORDER)
    {
      return (TRUE);
    }

    // Are we mean't to be holding our position ?
    if (Tactical::QueryProperty(GetTaskTable(), subject, Tactical::TP_HOLDPOSITION))
    {
      return (FALSE);
    }

    // Do not follow if the target is moving to a static restorer
    if (TaskCtrl::Promote<Tasks::UnitGoRestore>(target))
    {
      return (FALSE);
    }

    // Move to this target
    return (TRUE);
  }


  //
  // StateInit
  //
  // Start moving towards current target
  //
  void RestoreMobile::StateInit()
  {
    LOG_LOCAL(("StateInit"));

    // Must be able to move
    if (subject->CanEverMove() && ValidTarget())
    {
      LOG_LOCAL(("StateInit::Valid"));

      // Tell the subject to stop
      subject->GetDriver()->Stop();

      // Setup our destination
      destination = GetRestorationDestination();

      // Is the target in range
      if (TargetInRange())
      {
        LOG_LOCAL(("StateInit::Waiting"));

        // Jump to wait state
        NextState(0xCC45C48B); // "Waiting"
      }
      else
      {
        // Should we follow this unit
        if (MoveIntoRange())
        {
          LOG_LOCAL(("StateInit::Moving"));

          // Start moving
          subject->Move(moveHandle, &destination, NULL, FALSE);
          NextState(0x9E947215); // "Moving"
        }
        else
        {
          LOG_LOCAL(("StateInit::Quit1"));
          Quit();
        }
      }
    }
    else
    {
      LOG_LOCAL(("StateInit::Quit2"));

      Quit();
    }
  }


  //
  // StateMoving
  //
  // In transit to target
  //
  void RestoreMobile::StateMoving()
  {
    LOG_LOCAL(("StateMoving"));

    if (ValidTarget())
    {
      LOG_LOCAL(("StateMoving::Valid"));

      if (TargetMoved())
      {
        LOG_LOCAL(("StateMoving::TargetMoved"));

        // Plot a new course
        NextState(0xABAA7B48); // "Init"       
      }
      else

      if (TargetInRange(TRUE))
      {
        LOG_LOCAL(("StateMoving::InRange"));

        // Stop moving
        subject->GetDriver()->Stop();
      }
      else
      {
        LOG_LOCAL(("StateMoving::Moving"));
      }
    }
    else
    {
      LOG_LOCAL(("StateMoving::Quit"));

      Quit();
    }
  }


  //
  // StateWaiting
  //
  // Waiting for direct mode to become available
  //
  void RestoreMobile::StateWaiting()
  {
    LOG_LOCAL(("StateWaiting"));

    if (ValidTarget())
    {
      LOG_LOCAL(("StateWaiting::Valid"));

      if (TargetMoved())
      {
        LOG_LOCAL(("StateWaiting::TargetMoved"));

        // Plot a new course
        NextState(0xABAA7B48); // "Init"       
      }
      else

      if (subject->GetDriver()->AllowDirectControl())
      {
        LOG_LOCAL(("StateWaiting::AllowDirect"));

        // Enter direct control mode
        subject->GetDriver()->SetupDirectControl(TRUE);

        // Start the restore process
        NextState(0x197A7A41); // "PreProcess"
      }
      else
      {
        LOG_LOCAL(("StateWaiting::Waiting"));
      }
    }
    else
    {
      LOG_LOCAL(("StateWaiting::Quit"));

      Quit();
    }
  }


  //
  // StatePreProcess
  //
  // Handle the PreProcess animation
  //
  void RestoreMobile::StatePreProcess(StateMachineNotify notify)
  {
    LOG_LOCAL(("StatePreProcess"));

    switch (notify)
    {
      case SMN_ENTRY:
        LOG_LOCAL(("StatePreProcess::Entry"));
        subject->SetAnimation(0x197A7A41); // "PreProcess"
        break;
    }
  }


  //
  // StateRestoring
  //
  // Restoring the current target
  //
  void RestoreMobile::StateRestoring(StateMachineNotify notify)
  {
    LOG_LOCAL(("StateRestoring"));

    switch (notify)
    {
      case SMN_ENTRY:
      {
        LOG_LOCAL(("StateRestoring::Entry"));

        if (ValidTarget())
        {
          LOG_LOCAL(("StateRestoring::Entry::Valid"));

          subject->SetAnimation(0xBDF7C019); // "Process"
          subject->StartRestoreFX();
        }
        break;
      }

      case SMN_PROCESS:
      {
        LOG_LOCAL(("StateRestoring::Process"));

        if (ValidTarget())
        {
          LOG_LOCAL(("StateRestoring::Process::Valid"));

          if (TargetInRange())
          {
            LOG_LOCAL(("StateRestoring::Process::InRange"));

            // Calculate new direction
            Vector v(target->Position() - subject->Position());
            v.NormalizeXZ();
            VectorDir d;
            v.Convert(d);

            // Tell the unit to turn
            subject->GetDriver()->DirectTurn(d.u);

            // Do a single restoration cycle
            subject->Restore(target);
          }
          else
          {
            LOG_LOCAL(("StateRestoring::Process::Init"));

            // Need to move closer to target
            NextState(0xABAA7B48); // "Init"       
          }
        }
        else
        {
          LOG_LOCAL(("StateRestoring::Invalid"));

          NextState(0x927FACBD); // "PostProcess"
        }
        break;
      }

      case SMN_EXIT:
      {
        LOG_LOCAL(("StateRestoring::Exit"));

        // Drop out of direct control mode
        subject->GetDriver()->SetupDirectControl(FALSE);
        break;
      }
    }
  }


  //
  // StatePostProcess
  //
  // Handle the finishing animation
  //
  void RestoreMobile::StatePostProcess(StateMachineNotify notify)
  {
    LOG_LOCAL(("StatePostProcess"));

    switch (notify)
    {
      case SMN_ENTRY:
      {
        LOG_LOCAL(("StatePostProcess::Entry"));
        subject->SetAnimation(0x927FACBD); // "PostProcess"
      }
    }
  }


  //
  // Notify
  //
  // Notify this task of an event
  //
  Bool RestoreMobile::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
      {
        LOG_LOCAL(("Interrupted"));

        // Drop out of direct control mode
        subject->GetDriver()->SetupDirectControl(FALSE);

        NextState(0xABAA7B48); // "Init"
        return (TRUE);
      }

      case Movement::Notify::Completed:
      {
        LOG_LOCAL(("Movement::Completed"));

        if (moveHandle == event.param1)
        {
          LOG_LOCAL(("Movement::Completed::Match %d", moveHandle));

          if (ValidTarget() && TargetInRange())
          {
            LOG_LOCAL(("Movement::Completed::Waiting"));

            NextState(0xCC45C48B); // "Waiting"
          }
          else
          {
            LOG_LOCAL(("Movement::Completed::Init"));

            NextState(0xABAA7B48); // "Init"
          }
        }
        return (TRUE);
      }

      case Movement::Notify::Incapable:
      {
        LOG_LOCAL(("Movement::Incapable"));

        if (moveHandle == event.param2)
        {
          LOG_LOCAL(("Movement::Incapable::Match"));
          Quit();
        }
        return (TRUE);
      }

      case MapObjNotify::AnimationDone:
      {
        LOG_LOCAL(("AnimationDone 0x%08x", event.param1));

        switch (event.param1)
        {
          case 0x197A7A41: // "PreProcess"
          {
            LOG_LOCAL((" - PreProcess"));

            // Begin restoration
            NextState(0x34AE4B12); // "Restoring"
            return (TRUE);
          }

          case 0xBDF7C019: // "Process"
          {
            LOG_LOCAL((" - Process"));

            // Do we need to play it again
            if (inst.Test(0x34AE4B12)) // "Restoring"
            {
              LOG_LOCAL(("   - Restoring"));
              subject->SetAnimation(0xBDF7C019); // "Process"
            }

            return (TRUE);
          }

          case 0x927FACBD: // "PostProcess"
          {
            LOG_LOCAL((" - PostProcess"));

            // Back to default animation
            subject->SetAnimation(0x8F651465); // "Default"

            // Task is finished
            Quit();
            return (TRUE);
          }
        }
        return (FALSE);
      }

      case 0x2F20A733: // "RestoreObjType::IsRestoring"
      {
        return (inst.Test(0x34AE4B12)); // "Restoring"
      }
    }
    return (GameTask<RestoreObjType, RestoreObj>::ProcessEvent(event));
  }


  //
  // Initialization
  //
  void RestoreMobile::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Build state machine
    stateMachine.AddState("Init", &RestoreMobile::StateInit);
    stateMachine.AddState("Moving", &RestoreMobile::StateMoving);
    stateMachine.AddState("Waiting", &RestoreMobile::StateWaiting);
    stateMachine.AddState("PreProcess", &RestoreMobile::StatePreProcess);
    stateMachine.AddState("Restoring", &RestoreMobile::StateRestoring);
    stateMachine.AddState("PostProcess", &RestoreMobile::StatePostProcess);
  }


  //
  // Shutdown
  //
  void RestoreMobile::Done()
  {
    stateMachine.CleanUp();
  }
}
