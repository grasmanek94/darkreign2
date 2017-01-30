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
#include "tasks_restorestatic.h"
#include "tasks_unitmove.h"
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
  StateMachine<RestoreStatic> RestoreStatic::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class RestoreStatic
  //


  // Static data
  NList<RestoreStatic> RestoreStatic::list(&RestoreStatic::node);


  //
  // Find
  //
  // Find the closest static restore object
  //
  RestoreObj * RestoreStatic::Find(UnitObj *target, Bool acceptFirst)
  {
    RestoreObj *winner = NULL;
    F32 min = F32_MAX;

    // Check each object
    for (NList<RestoreStatic>::Iterator i(&list); *i; i++)
    {
      // Get the restore object
      RestoreObj *candidate = (*i)->subject;

      // Test team relation
      if (candidate->CanRestore(target))
      {
        // Should we accept the first one we find
        if (acceptFirst)
        {
          return (candidate);
        }

        // Calculate the distance to the candidate
        F32 dist = Vector(candidate->Origin() - target->Origin()).Magnitude2();

        // Is it the closest so far
        if (dist < min)
        {
          min = dist;
          winner = candidate;
        }       
      }
    }

    return (winner);
  }


  //
  // Constructor
  //
  RestoreStatic::RestoreStatic(GameObj *sub) 
  : GameTask<RestoreObjType, RestoreObj>(staticConfig, sub),
    inst(&stateMachine, "Idle"),
    rallyPointActive(FALSE),
    rallyPoint(0, 0)
  {
    // Add to the list
    list.Append(this);

    // Check type
    if (!subject->UnitType()->CanBoard())
    {
      ERR_CONFIG(("RestoreStatic assigned to a non-boardable type [%s]", subject->TypeName()));
    }
  }


  //
  // Destructor
  //
  RestoreStatic::~RestoreStatic()
  {
    list.Unlink(this);
  }


  //
  // Save
  //
  void RestoreStatic::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));

    if (rallyPointActive)
    {
      StdSave::TypePoint(fScope, "RallyPoint", rallyPoint);
    }
  }


  //
  // Load
  //
  void RestoreStatic::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
          break;

        case 0x67DEDB99: // "RallyPoint"
          StdLoad::TypePoint(sScope, rallyPoint);
          rallyPointActive = TRUE;
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
  void RestoreStatic::PostLoad()
  {
  }


  //
  // Perform task processing
  //
  Bool RestoreStatic::Process()
  {
    inst.Process(this);  

    return (quit);
  }

  
  //
  // ValidTarget
  //
  // Do we have a valid target that needs restoration
  //
  Bool RestoreStatic::ValidTarget()
  {
    return 
    (
      subject->GetBoardManager()->InUse() && 
      subject->RestoreRequired(subject->GetBoardManager()->GetUnitObj())
    );
  }


  //
  // Idle state
  //
  void RestoreStatic::StateIdle()
  {
  }


  //
  // Restoring state
  //
  void RestoreStatic::StateRestoring(StateMachineNotify notify)
  {
    switch (notify)
    {
      case SMN_ENTRY:
      {
        if (ValidTarget())
        {
          // Start the fx
          subject->StartRestoreFX();
        }
        break;
      }

      case SMN_PROCESS:
      {
        if (ValidTarget())
        {
          UnitObj *target = subject->GetBoardManager()->GetUnitObj();

          // Do a single restoration cycle
          subject->Restore(target);

          // Animate the lights
          subject->Mesh().PollActivateTexAnim();

          // Is more restoration required
          if (subject->RestoreRequired(target))
          {
            break;
          }
        }

        NextState(0x9C4D1D6F); // "BuggerOff"
        break;
      }
    }
  }


  //
  // StateBuggerOff
  //
  // Tell any target to leave the premises
  //
  void RestoreStatic::StateBuggerOff()
  {
    // Is there a target
    if (subject->GetBoardManager()->InUse())
    {
      UnitObj *target = subject->GetBoardManager()->GetUnitObj();

      // Is there a rally point on this facility
      if (rallyPointActive)
      {
        U32 index = 0;

        // Find a trail that sits on the rally point
        if (TrailObj *trail = TrailObj::Find(rallyPoint.x, rallyPoint.z, target->GetTeam(), &index))
        {
          target->PrependTask(new Tasks::UnitMove(target, trail, index));
        }
        else
        {
          // Setup a destination vector
          Vector v(WorldCtrl::CellToMetresX(rallyPoint.x), 0.0F, WorldCtrl::CellToMetresZ(rallyPoint.z));

          // Tell the unit to move there
          target->PrependTask(new Tasks::UnitMove(target, v));
        }
      }
      else
      {
        target->MoveAwayTask();
      }
    }

    // Go back to the idle state
    NextState(0x793360CC); // "Idle"
  }


  //
  // ProcessEvent
  //
  // Process incoming events
  //
  Bool RestoreStatic::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case Movement::Notify::UnitEntered:
      {
        if (ValidTarget())
        {
          subject->SetAnimation(0x197A7A41); // "PreProcess"
        }
        else
        {
          NextState(0x9C4D1D6F); // "BuggerOff"
        }
        return (TRUE);
      }

      case Movement::Notify::UnitExited:
      {
        NextState(0x793360CC); // "Idle"
        return (TRUE);
      }

      case 0x2F20A733: // "RestoreObjType::IsRestoring"
      {
        return (inst.Test(0x34AE4B12)); // "Restoring"
      }

      case MapObjNotify::AnimationDone:
      {
        switch (event.param1)
        {
          case 0x197A7A41: // "PreProcess"
          {
            // Start the processing animation
            subject->SetAnimation(0xBDF7C019); // "Process"

            // Begin restoration
            NextState(0x34AE4B12); // "Restoring"
            break;
          }

          case 0xBDF7C019: // "Process"
          {
            // Do we need to play it again
            if (inst.Test(0x34AE4B12)) // "Restoring"
            {
              subject->SetAnimation(0xBDF7C019); // "Process"
            }
            else
            {
              subject->SetAnimation(0x927FACBD); // "PostProcess"
            }
            break;
          }

          case 0x927FACBD: // "PostProcess"
            break;
        }

        return (TRUE);
      }

      case UnitObjNotify::CanUseRallyPoint:
      {
        return (TRUE);
      }

      case UnitObjNotify::SetRallyPoint:
      {
        // Is the given point on the map
        if (WorldCtrl::CellOnMap(event.param1, event.param2))
        {
          rallyPoint.Set(event.param1, event.param2);
          rallyPointActive = TRUE;
          return (TRUE);
        }

        return (FALSE);
      }
    }
    return (GameTask<RestoreObjType, RestoreObj>::ProcessEvent(event));
  }

  
  //
  // Retrieve
  //
  // Retrieve data from the task, return TRUE if filled
  //
  Bool RestoreStatic::Retrieve(U32 id, RetrievedData &data)
  {
    switch (id)
    {
      case UnitObjNotify::GetRallyPoint:
      {
        if (rallyPointActive)
        {
          data.u1 = rallyPoint.x;
          data.u2 = rallyPoint.z;
          return (TRUE);
        }

        return (FALSE);
      }
    }

    return (GameTask<RestoreObjType, RestoreObj>::Retrieve(id, data));
  }


  //
  // Initialization
  //
  void RestoreStatic::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Add states to the state machine
    stateMachine.AddState("Idle", &StateIdle);
    stateMachine.AddState("Restoring", &StateRestoring);
    stateMachine.AddState("BuggerOff", &StateBuggerOff);
  }


  //
  // Shutdown
  //
  void RestoreStatic::Done()
  {
    stateMachine.CleanUp();
  }
}
