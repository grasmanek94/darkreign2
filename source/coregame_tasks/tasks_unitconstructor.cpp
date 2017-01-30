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
#include "tasks_unitconstructor.h"
#include "gameobjctrl.h"
#include "gametime.h"
#include "mapobjctrl.h"
#include "offmapobj.h"
#include "team.h"
#include "orders_game.h"
#include "client.h"
#include "tasks_unitmove.h"
#include "taskctrl.h"
#include "resolver.h"
#include "babel.h"
#include "message.h"


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
  StateMachine<UnitConstructor> UnitConstructor::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Constructor
  //

  //
  // FXCallback
  //
  // Returns TRUE when processing is completed
  //
  Bool UnitConstructor::FXCallBack(MapObj *mapObj, FX::CallBackData &, void *context)
  {
    // Is the constructor task still active
    UnitConstructor *task = TaskCtrl::Promote<Tasks::UnitConstructor>(mapObj);

    // Effect is done if construction finished, or new one has started
    return (!task || task->construct.Dead() || (task->creationCount != U32(context)));
  }


  //
  // Constructor
  //
  UnitConstructor::UnitConstructor(GameObj *subject) :
    GameTask<UnitObjType, UnitObj>(staticConfig, subject),
    inst(&stateMachine, "Process"),
    paused(FALSE),
    progressTotal(0),
    progressMax(0),
    resourceRemoved(0),
    resourceRemaining(0),
    orderInTransit(FALSE),
    rallyPointActive(FALSE),
    rallyPoint(0, 0),
    creationCount(0)
  {
  }


  //
  // Destructor
  //
  UnitConstructor::~UnitConstructor()
  {
    outputQueue.Clear();
    clientQueue.Clear();
  }


  //
  // Save
  //
  void UnitConstructor::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));

    StdSave::TypeReaperObjType(fScope, "Construct", construct);
    StdSave::TypeReaperListObjType(fScope, "OutputQueue", outputQueue);   
    StdSave::TypeReaper(fScope, "OffMap", offMap);
    StdSave::TypeU32(fScope, "Paused", paused);
    StdSave::TypeF32(fScope, "ProgressTotal", progressTotal);
    StdSave::TypeF32(fScope, "ProgressMax", progressMax);
    StdSave::TypeU32(fScope, "ResourceRemoved", U32(resourceRemoved));
    StdSave::TypeU32(fScope, "ResourceRemaining", U32(resourceRemaining));
    StdSave::TypeReaperListObjType(fScope, "ClientQueue", clientQueue);   

    if (rallyPointActive)
    {
      StdSave::TypePoint(fScope, "RallyPoint", rallyPoint);
    }

    StdSave::TypeU32(fScope, "CreationCount", creationCount);
  }
  

  //
  // Load
  //
  void UnitConstructor::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
          break;

        case 0x4E5A04E4: // "Construct"
          StdLoad::TypeReaperObjType(sScope, construct);
          break;

        case 0x5EEEB29D: // "OutputQueue"
          StdLoad::TypeReaperListObjType(sScope, outputQueue);
          break;

        case 0xBF87218C: // "OffMap"
          StdLoad::TypeReaper(sScope, offMap);
          break;

        case 0xF073DD95: // "Paused"
          paused = StdLoad::TypeU32(sScope);
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

        case 0xA7E4C68A: // "ClientQueue"
          StdLoad::TypeReaperListObjType(sScope, clientQueue);
          break;

        case 0x67DEDB99: // "RallyPoint"
          StdLoad::TypePoint(sScope, rallyPoint);
          rallyPointActive = TRUE;
          break;

        case 0xB74DCFCA: // "CreationCount"
          creationCount = StdLoad::TypeU32(sScope);
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
  void UnitConstructor::PostLoad()
  {
    Resolver::Type(construct);
    Resolver::TypeList(outputQueue);
    Resolver::TypeList(clientQueue);
  }


  //
  // Perform task processing
  //
  Bool UnitConstructor::Process()
  {
    // Process the client side queue
    QueueProcess(Client::GetPlayer());

    // Process the lock-step construction
    inst.Process(this);

    // Always process per cycle
    ThinkFast();

    // Never quit and idle task
    return (FALSE);
  }


  //
  // StateProcess
  //
  // Process object construction
  //
  void UnitConstructor::StateProcess()
  {
    // Do we need to process construction
    if (subject->GetTeam() && construct.Alive() && !paused && !offMap.Alive())
    {
      // Save previous progress incase resource not available
      F32 progressPrevious = progressTotal;

      // Increment the progress counter
      progressTotal += (progressMax * subject->GetEfficiency());

      // Is there any resource to remove
      if (resourceRemaining)
      {
        // How much resource should we have removed
        S32 resourceTarget = S32(progressTotal * F32(construct->GetResourceCost()));

        // How much should we remove this cycle
        S32 resourceToRemove = Min<S32>(resourceTarget - resourceRemoved, resourceRemaining);

        // Do nothing if no resource to remove
        if (resourceToRemove > 0)
        {
          // Attempt to use the resource
          if (subject->GetTeam()->RemoveResourceStore(resourceToRemove))
          {
            subject->GetTeam()->ReportResource(resourceToRemove, "resource.spent");

            // Update resource counts
            resourceRemaining -= resourceToRemove;
            resourceRemoved += resourceToRemove;

            // Animate the lights
            subject->Mesh().PollActivateTexAnim();
          }
          else
          {
            // Resource not available, construction must delay
            progressTotal = progressPrevious;
          }
        }
      }

      ASSERT(resourceRemaining >= 0)

      // Is construction completed
      if (!resourceRemaining && progressTotal >= 1.0F)
      {
        ASSERT(resourceRemoved == S32(construct->GetResourceCost()))

        // Should this object be created off the map
        if (Promote::Type<OffMapObjType>(construct))
        {
          ASSERT(!offMap.Alive())

          // Create the object
          offMap = (OffMapObj*)(MapObjCtrl::ObjectNew(construct));

          // Set the team and facility
          offMap->SetTeam(subject->GetTeam());
          offMap->SetFacility(subject);

          // Tell the team that we built something
          subject->GetTeam()->ReportConstruction(offMap);

          if (subject->GetTeam() == Team::GetDisplayTeam())
          {
            // "Tasks::UnitConstructor::OffMapCompleted"
            Message::TriggerGameMessage(0xA3B85C9A);
          }
        }
        else
        {
          // Add to the output queue
          outputQueue.Append(construct);
        }

        // Successfully created an object
        creationCount++;

        // Available for new requests
        construct = NULL;
      }
    }

    // If we need to put this on the map the board manager must exist
    if (outputQueue.GetCount())
    {      
      if (!subject->UnitType()->CanBoard())
      {
        ERR_FATAL(("Can not construct from [%s], Exit points are missing", subject->TypeName()));
      }

      // Is the rail ready
      if (!subject->GetBoardManager()->InUse())
      {
        // Get the next type in the queue
        UnitObjType *type = outputQueue.GetFirst();

        ASSERT(type)

        // Ensure we're not limited for this type
        if (!UnitLimitReached(type))
        {
          // Remove it from the queue
          outputQueue.Pop();

          // Create the object on the building
          UnitObj *unit = &type->Spawn(subject, !rallyPointActive);

          // Is there a rally point on this facility
          if (rallyPointActive)
          {
            U32 index = 0;

            // Find a trail that sits on the rally point
            if (TrailObj *trail = TrailObj::Find(rallyPoint.x, rallyPoint.z, unit->GetTeam(), &index))
            {
              unit->PrependTask(new Tasks::UnitMove(unit, trail, index));
            }
            else
            {
              // Setup a destination vector
              Vector v(WorldCtrl::CellToMetresX(rallyPoint.x), 0.0F, WorldCtrl::CellToMetresZ(rallyPoint.z));

              // Tell the unit to move there
              unit->PrependTask(new Tasks::UnitMove(unit, v));
            }
          }

          // Copy the tactical settings from the facility
          unit->settings = subject->settings;

          // Tell the team that we built something
          subject->GetTeam()->ReportConstruction(unit);

          // Notify the player that construction has completed
          subject->NotifyPlayer(0x7FF81455, unit->Id()); // "UnitConstructor::Completed"
        }
      }
    }
  }


  //
  // UnitLimitReached
  //
  // Has the unit limit been reached for the subjects team
  //
  Bool UnitConstructor::UnitLimitReached(UnitObjType *type)
  {
    return 
    (    
      // Subject is on a team
      subject->GetTeam() 
      
      && 
      
      // And the unit limit has been reached
      subject->GetTeam()->UnitLimitReached(type)
    );
  }


  //
  // ProcessEvent
  //
  // Notify this task of an event
  //
  Bool UnitConstructor::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case 0x70C4B225: // "UnitConstructor::Cancel"
      {
        if (construct.Alive())
        {
          // Return the used resources
          if (subject->GetTeam())
          {
            subject->GetTeam()->AddResourceStore(resourceRemoved);
            subject->GetTeam()->ReportResource(resourceRemoved, "resource.refunds");
          }

          // Notify the player that construction has been canceled
          subject->NotifyPlayer(0x3B2EB6EA); // "UnitConstructor::Canceled"

          construct = NULL;
        }
        return (TRUE);
      }

      case 0x258B47CE: // "Order::UnitConstructor::Build"
      {
        // Order is no longer in transit
        SetOrderInTransit(FALSE);

        // Value is the crc of a map object type
        UnitObjType *type = GameObjCtrl::FindType<UnitObjType>(event.param1);

        // Can we start construction
        if (type && !construct.Alive() && !offMap.Alive() && !UnitLimitReached(type))
        {
          // Check that we can construct this type (order lag & client hacks)
          if (subject->CanConstruct(type))
          {
            // If no animation playing, start one
            if (!subject->Mesh().AnimIsActive())
            {
              // Start the processing animation
              subject->SetAnimationUpgrades(0xBDF7C019); // "Process"
            }

            // "Constructor::Process"
            subject->StartGenericFX
            (
              0xE5DA1C06, FXCallBack, FALSE, NULL, (void *)(creationCount)
            ); 

            // Save the construction type
            construct = type;

            // Clear paused state
            paused = FALSE;
         
            // Setup progress counts
            progressTotal = 0.0F;
            progressMax = (construct->GetConstructionTime() > 0.0F) ? 
              (1.0F / (construct->GetConstructionTime() * GameTime::SimTimeInv())) : 1.0F;

            // Setup resource counts
            resourceRemoved = 0;
            resourceRemaining = construct->GetResourceCost();
          }
        }
        else
        {
          // Notify the player that construction has failed
          subject->NotifyPlayer(0x3608BAC3); // "UnitConstructor::Failed"
        }

        return (TRUE);
      }

      case 0x51A492DE: // "Order::UnitConstructor::Cancel"
      {
        // Value is the crc of a map object type
        UnitObjType *type = GameObjCtrl::FindType<UnitObjType>(event.param1);

        if (construct.Alive() && type == construct)
        {
          ProcessEvent(Event(0x70C4B225)); // "UnitConstructor::Cancel"
        }
        return (TRUE);
      }

      case 0xC44A7A13: // "Order::UnitConstructor::ClearOffMap"
      {
        if (offMap.Alive() && offMap->Id() == event.param1)
        {
          offMap = NULL;
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

      case MapObjNotify::AnimationDone:
      case MapObjNotify::AnimationDoneChild:
      {
        switch (event.param1)
        {
          case 0xBDF7C019: // "Process"
          {
            if (construct.Alive() && event.object.Alive())
            {
              if (UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(event.object))
              {
                // Start the processing animation again
                unit->SetAnimationUpgrades(0xBDF7C019); // "Process"
              }
            }
            break;
          }
        }
          
        return (TRUE);
      }

      case TaskNotify::CanPause:
      { 
        return (construct.Alive() && !paused);
      }

      case TaskNotify::CanUnpause:
      {
        return (construct.Alive() && paused);
      }

      case TaskNotify::Pause:
      {      
        if (ProcessEvent(TaskNotify::CanPause))
        {
          paused = TRUE;
          return (TRUE);
        }
        return (FALSE);
      }

      case TaskNotify::Unpause:
      {
        if (ProcessEvent(TaskNotify::CanUnpause))
        {
          paused = FALSE;
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
  Bool UnitConstructor::Retrieve(U32 id, RetrievedData &data)
  {
    switch (id)
    {
      case TaskRetrieve::Progress:
      {
        if (GetOffMapObject())
        {
          data.u1 = 0xB6272C24; // "Tasks::UnitConstructor::OffMap"
          data.f1 = 1.0F;
          return (TRUE);
        }
        else

        if (GetConstructType())
        {
          data.u1 = GetNameCrc();
          data.f1 = GetProgress();
          return (TRUE);
        }

        return (FALSE);
      }

      case TaskRetrieve::Count:
      {
        data.u1 = QueueCount();
        return (TRUE);
      }

      case TaskRetrieve::Info:
      {
        // Is there an offmap object available
        if (offMap.Alive())
        {
          data.s2 = TRANSLATE(("#game.client.hud.tasks.constructor.offmap", 1, offMap->GetDesc()));
          return (TRUE);
        }
        
        // Is something under construction
        if (construct.Alive())
        {
          if (paused)
          {
            data.s2 = TRANSLATE(("#game.client.hud.tasks.constructor.paused"));
          }
          else
          {
            data.s2 = TRANSLATE(("#game.client.hud.tasks.constructor.process", 2, construct->GetDesc(), GetProgress() * 100.0F));
          }

          return (TRUE);
        }

        return (FALSE);
      }

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

    return (GameTask<UnitObjType, UnitObj>::Retrieve(id, data));
  }


  //
  // GetConstructType
  //
  // Get the type currently being constructed, or NULL
  //
  UnitObjType * UnitConstructor::GetConstructType()
  {
    return (construct.Alive() ? construct.GetData() : NULL);
  }


  //
  // GetOffMapObject
  //
  // Get the offmap object waiting to be used, or NULL
  //
  OffMapObj * UnitConstructor::GetOffMapObject()
  {
    return (offMap.GetPointer());
  }

  
  //
  // GetProgress
  //
  // Get the progress percentage
  //
  F32 UnitConstructor::GetProgress()
  {
    return (construct.Alive() ? progressTotal : 0.0F);
  }


  //
  // Paused
  //
  // Is construction currently paused
  //
  Bool UnitConstructor::Paused()
  {
    return (paused);
  }


  //
  // QueueAdd
  //
  // Add one queue item
  //
  void UnitConstructor::QueueAdd(UnitObjType *type)
  {
    ASSERT(type)
    clientQueue.Append(type);
  }


  //
  // QueueRemove
  //
  // Remove one queue item
  //
  void UnitConstructor::QueueRemove(UnitObjType *type)
  {
    ASSERT(type)
    
    UnitObjTypeList::Iterator i(&clientQueue);

    for (i.GoToTail(); *i; i--)
    {
      if (**i == type)
      {
        clientQueue.Dispose(*i);
        return;
      }
    }
  }


  // 
  // QueueClear
  //
  // Clear entire queue, or just given type
  //
  void UnitConstructor::QueueClear(UnitObjType *type)
  {
    if (type)
    {
      clientQueue.Remove(type);
    }
    else
    {
      clientQueue.Clear();
    }
  }


  //
  // QueueCount
  //
  // Total number of items queued, or just for given type
  //
  S32 UnitConstructor::QueueCount(UnitObjType *type)
  {
    if (type)
    {
      S32 count = 0;

      for (UnitObjTypeList::Iterator i(&clientQueue); *i; i++)
      {
        if (**i == type)
        {
          count++;
        }
      }

      return (count);
    }
    else
    {
      return (clientQueue.GetCount());
    }
  }


  //
  // QueueProcess
  //
  // Process the next queue item
  //
  Bool UnitConstructor::QueueProcess(Player &player)
  {  
    // Should we check the queue at this stage
    if (!orderInTransit && !construct.Alive() && !offMap.Alive())
    {
      // Check the client side queue
      for (UnitObjTypeList::Iterator i(&clientQueue); *i; i++)
      {
        // Get the type
        UnitObjType *t = **i;

        // Are we allowed to construct and do we have the prereqs to build this item
        if (subject->CanConstruct(t) && !UnitLimitReached(t))
        {
          // "Order::UnitConstructor::Build"
          Orders::Game::Constructor::Generate
          (
            player, subject->Id(), 0x258B47CE, t->GetNameCrc()
          );

          // Remove this item from the queue
          clientQueue.Dispose(*i);

          // Do not check the queue anymore
          SetOrderInTransit(TRUE);

          // Order has been sent
          return (TRUE);
        }
      }
    }

    // No order sent
    return (FALSE);
  }


  //
  // Initialization
  //
  void UnitConstructor::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Build state machine
    stateMachine.AddState("Process", &StateProcess);
  }


  //
  // Shutdown
  //
  void UnitConstructor::Done()
  {
    stateMachine.CleanUp();
  }
}
