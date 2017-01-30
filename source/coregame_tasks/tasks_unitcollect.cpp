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
#include "tasks_unitcollect.h"
#include "taskctrl.h"
#include "resolver.h"
#include "resourceobjiter.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Defines
  //
  #define LOG_COLLECT(x)
  //#define LOG_COLLECT(x) LOG_DIAG(x)

  #define REJECT_TIME     60.0f
  #define BETTER_DISTANCE 1600.0f


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // State machine
  StateMachine<UnitCollect> UnitCollect::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitCollect
  //


  //
  // Constructor
  //
  UnitCollect::UnitCollect(GameObj *subject) :
    GameTask<UnitObjType, UnitObj>(staticConfig, subject),
    inst(&stateMachine, "SearchForResource"),
    haveResourceLocation(FALSE),
    haveStorageLocation(FALSE)
  {
  }


  //
  // Destructor
  //
  UnitCollect::~UnitCollect()
  {
    // Cleanup Reject List
    rejects.Clear();
  }


  //
  // Save
  //
  void UnitCollect::Save(FScope *fScope)
  {
    // Save base data
    SaveTaskData(fScope);

    // Save the state machine
    inst.SaveState(fScope->AddFunction("StateMachine"));

    // Save the resource reaper
    StdSave::TypeReaper(fScope, "Resource", resourceObj);

    if (haveResourceLocation)
    {
      // Save the location
      StdSave::TypeVector(fScope, "LastResourceLocation", lastResourceLocation);
    }

    // Save the storage reaper
    StdSave::TypeReaper(fScope, "Storage", storageObj);

    if (haveStorageLocation)
    {
      // Save the location
      StdSave::TypeVector(fScope, "LastStorageLocation", lastStorageLocation);
    }

    // Save the movement handle
    moveHandle.SaveState(fScope->AddFunction("MoveHandle"));
  }


  //
  // Load
  //
  void UnitCollect::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
          break;

        case 0x4CD1BE27: // "Resource"
          StdLoad::TypeReaper(sScope, resourceObj);
          break;

        case 0x5F40B4A2: // "LastResourceLocation"
          StdLoad::TypeVector(sScope, lastResourceLocation);          
          haveResourceLocation = TRUE;
          break; 

        case 0x19077E4A: // "Storage"
          StdLoad::TypeReaper(sScope, storageObj);
          break;

        case 0x40128E41: // "LastStorageLocation"
          StdLoad::TypeVector(sScope, lastStorageLocation);          
          haveStorageLocation = TRUE;
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
  void UnitCollect::PostLoad()
  {
    // Resolve the storage reaper
    Resolver::Object<UnitObj, UnitObjType>(storageObj);

    // Resolve the collect reaper
    Resolver::Object<ResourceObj, ResourceObjType>(resourceObj);
  }


  //
  // Perform task processing
  //
  Bool UnitCollect::Process()
  {
    inst.Process(this);  

    // Remove dead rejects
    rejects.PurgeDead();

    // Remove any timed out rejects
    F32 time = GameTime::SimTotalTime();
    ResourceList::Iterator r(&rejects);
    ListNode *node;

    while ((node = r++) != NULL)
    {
      // Has the reject timed out ?
      if (node->timeout < time)
      {
        LOG_COLLECT(("Removing [%d] from reject list", node->Id()))

        // Remove from the reject list
        rejects.Dispose(node);
      }
    }

    // Perform type processing
    GameProcess();

    return (quit);
  }


  //
  // Notify this task of an event
  //
  Bool UnitCollect::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
      {
        LOG_COLLECT(("Interrupted"))

        if (event.param1 & Task::TF_FROM_ORDER)
        {
          // When interrupted by a task from an order, stay in the wait state
          NextState(0x6E758990); // "Wait"
        }
        else
        {
          // Otherwise go back to what we were doing before
          RecycleState();
        }
        return (TRUE);
      }

      case Movement::Notify::BoardComplete:
      {
        // Just boarded a building
        if (moveHandle == event.param1)
        {
          switch (inst.GetNameCrc())
          {
            case 0x33FA0EA0: // "GoToStorage"
              // Boarded the storage
              LOG_COLLECT(("Movement::BoardComplete -> GoToStorage"))
              NextState(0x8D59FB3F); // "PreUnloadResource"
              break;
          }
        }
        return (TRUE);
      }

      case Movement::Notify::Completed:
      {
        if (moveHandle == event.param1)
        {
          switch (inst.GetNameCrc())
          {
            case 0x21F96549: // "GoToResourceArea"
              LOG_COLLECT(("Movement::Completed -> GotoResourceArea"))
              NextState(0x2E8B68BF); // "SearchForResource"
              break;

            case 0x06083411: // "GoToResource"
              LOG_COLLECT(("Movement::Completed -> GoToResource"))

              // Is the resource alive ?
              if (resourceObj.Alive())
              {
                FootPrint::Instance *footInst = resourceObj->GetFootInstance();
                Vector dst;

                if (footInst)
                {
                  // Are we on the fringe ?
                  if (!footInst->CellOnFringe(Point<S32>(subject->GetCellX(), subject->GetCellZ())))
                  {
                    // Place the object on the reject list and reject it for REJECT_TIME seconds
                    rejects.Append(resourceObj)->timeout = GameTime::SimTotalTime() + REJECT_TIME;

                    NextState(0x21F96549); // "GoToResourceArea"
                    break;
                  }
                  else
                  {
                    NextState(0x91E1EABF); // "AlignResource"
                  }
                }
                else
                {
                  ERR_FATAL(("Resource [%d] at %d, %d is not footprinted!", resourceObj->Id(), resourceObj->GetCellX(), resourceObj->GetCellZ()))
                }
              }

              break;

            case 0x91E1EABF: // "AlignResource"
              LOG_COLLECT(("Movement::Completed -> AlignResource"))
              NextState(0xC4077D40); // "PreLoadResource"
              break;

            case 0x33FA0EA0: // "GoToStorage"
              // This should only handle the case where the storage object died,
              // as a successful board will be handled by Movement::Notify::BoardComplete
              if (!storageObj.Alive())
              {
                LOG_COLLECT(("Movement::Completed -> GoToStorage"))
                NextState(0x70EB48AA); // "SearchForStorage"
              }
              break;
          }
        }
        return (TRUE);
      }

      case Movement::Notify::Incapable:
      {
        if (moveHandle == event.param1)
        {
          switch (inst.GetNameCrc())
          {
            case 0x06083411: // "GoToResource"
            {
              LOG_COLLECT(("Movement::Incapable -> GoToResource"))

              // Is the resource alive ?
              if (resourceObj.Alive())
              {
                FootPrint::Instance *footInst = resourceObj->GetFootInstance();
                Vector dst;

                if (footInst)
                {
                  // Are we on the fringe ?
                  if (!footInst->CellOnFringe(Point<S32>(subject->GetCellX(), subject->GetCellZ())))
                  {
                    // Place the object on the reject list and reject it for REJECT_TIME seconds
                    rejects.Append(resourceObj)->timeout = GameTime::SimTotalTime() + REJECT_TIME;

                    LOG_COLLECT(("Adding [%d] to reject list", resourceObj->Id()))

                    // Place the object on the reject list and reject it for REJECT_TIME seconds
                    rejects.Append(resourceObj)->timeout = GameTime::SimTotalTime() + REJECT_TIME;

                    NextState(0x21F96549); // "GoToResourceArea"
                  }
                  else
                  {
                    // By some fluke we are on the fringe even though movement failed
                    NextState(0x91E1EABF); // "AlignResource"
                  }
                }
                else
                {
                  ERR_FATAL(("Resource [%d] at %d, %d is not footprinted!", resourceObj->Id(), resourceObj->GetCellX(), resourceObj->GetCellZ()))
                }
              }
              else
              {
                // Look for more resource
                NextState(0x21F96549); // "GoToResourceArea"
              }
              break;
            }

            case 0x21F96549: // "GoToResourceArea"
            {
              LOG_COLLECT(("Movement::Incapable -> GoToResourceArea"))
              NextState(0x2E8B68BF); // "SearchForResource"
              break;
            }

            case 0x33FA0EA0: // "GoToStorage"
              LOG_COLLECT(("Movement::Incapable -> SearchForStorage"))
              NextState(0x70EB48AA); // "SearchForStorage"
              break;
          }
        }
        return (TRUE);
      }

      case MapObjNotify::AnimationDone:
      {
        switch (event.param1)
        {
          case 0x64EDDD3B: // "PreLoad"
            LOG_COLLECT(("Primitive::Anim::Completed -> Pre-Load"))
            NextState(0x0974185E); // "LoadResource"
            break;

          case 0x6C7C139F: // "PostLoad"
            LOG_COLLECT(("Primitive::Anim::Completed -> Post-Load"))
            NextState(0x55801D9E); // "LeaveResource"
            break;

          case 0x96717F1D: // "PreUnload"
            LOG_COLLECT(("Primitive::Anim::Completed -> Pre-Unload"))
            NextState(0xDB0FD7F2); // "UnloadResource"
            break;

          case 0xF3411A66: // "PostUnload"
            LOG_COLLECT(("Primitive::Anim::Completed -> Post-Unload"))
            NextState(0x06083411); // "GoToResource"
            break;
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
  Bool UnitCollect::Retrieve(U32 id, RetrievedData &data)
  {
    switch (id)
    {
      case TaskRetrieve::Progress:
      {
        F32 t = F32(subject->UnitType()->GetResourceTransport());
        F32 i = F32(subject->GetResourceTransport());

        if (t && i)
        {
          data.u1 = 0x6EA6146E; // "Unit::ResourceStore"
          data.f1 = F32(i) / F32(t);
          return (TRUE);
        }

        return (FALSE);
      }
    }

    return (GameTask<UnitObjType, UnitObj>::Retrieve(id, data));
  }


  //
  // Store what we've got
  //
  void UnitCollect::Store()
  {
    if (storageObj.Alive())
    {
      SetStorageObject(storageObj, FALSE);
    }
    else
    {
      NextState(0x70EB48AA); // "SearchForStorage"
    }
  }


  //
  // UnitCollect::SetStorageObject
  //
  // Set the store object
  //
  void UnitCollect::SetStorageObject(UnitObj *obj, Bool update)
  {
    ASSERT(obj)

    // Ignore requests to board non-boardable object
    if (obj->UnitType()->CanBoard())
    {
      storageObj = obj;
      lastStorageLocation = obj->Origin();
      haveStorageLocation = TRUE;

      // If we are currently on this object, ignore request
      if (obj->GetBoardManager()->InUse() && obj->GetBoardManager()->GetUnitObj() == subject)
      {
        return;
      }

      // If just updating storage object, but already set, ignore request
      if (update && storageObj.Alive() && obj == storageObj)
      {
        return;
      }

      // Do we have any resource
      if (subject->GetResourceTransport())
      {
        // If we're not meant to just update OR we're currently storing then go to storage
        if (
          !update || 
          inst.Test(0x33FA0EA0) || // "GoToStorage"
          inst.Test(0x8D59FB3F) || // "PreUnloadResource"
          inst.Test(0xDB0FD7F2)) // "UnloadResource"
        {
          NextState(0x33FA0EA0); // "GoToStorage"
        }
      }
    }
  }


  //
  // ClearStorageObject
  //
  // Clear all knowledge of a storage object
  //
  void UnitCollect::ClearStorageObject()
  {
    storageObj = NULL;
    haveStorageLocation = FALSE;
  }


  //
  // Get more resource
  //
  void UnitCollect::Resource()
  {
    if (resourceObj.Alive())
    {
      SetResourceObject(resourceObj);
    }
    else
    {
      NextState(0x2E8B68BF); // "SearchForResource"
    }
  }


  //
  // UnitCollect::SetResourceObject
  //
  // Set the collect object
  //
  void UnitCollect::SetResourceObject(ResourceObj *obj)
  {
    ASSERT(obj)

    if (obj->GetFootInstance())
    {
      resourceObj = obj;
      lastResourceLocation = obj->Origin();
      haveResourceLocation = TRUE;

      NextState(0x06083411); // "GoToResource"
    }
    else
    {
      LOG_WARN(("Collector ignoring non-zipped resource object [%d] at [%d,%d]", obj->Id(), obj->GetCellX(), obj->GetCellZ()));
    }
  }

 
  //
  // UnitCollect::StateWait
  //
  void UnitCollect::StateWait(StateMachineNotify)
  {
  }


  //
  // UnitCollect::StateGoToResource
  //
  void UnitCollect::StateGoToResource(StateMachineNotify notify)
  {
    LOG_COLLECT(("StateGoToResource %d", notify))

    switch (notify)
    {
      case SMN_ENTRY:

        // Is the resource alive ?
        if (resourceObj.Alive())
        {
          LOG_COLLECT(("Going to Resource [%d] at %d, %d", resourceObj->Id(), resourceObj->GetCellX(), resourceObj->GetCellZ()))

          FootPrint::Instance *footInst = resourceObj->GetFootInstance();
          Vector dst;

          if (footInst)
          {
            // Move to the closest point on the footprint
            Point<S32> pos(subject->GetCellX(), subject->GetCellZ());
            footInst->ClampToFringe(pos);
            dst.Set(WorldCtrl::CellToMetresX(pos.x), 0.0F, WorldCtrl::CellToMetresZ(pos.z));
          }
          else
          {
            ERR_FATAL(("Resource [%d] at %d, %d is not footprinted!", resourceObj->Id(), resourceObj->GetCellX(), resourceObj->GetCellZ()))
          }

          // Move to it
          Movement::RequestData rd;
          rd.giveUpGrains = 0;

          subject->Move(moveHandle, &dst, NULL, GetFlags() & TF_AI ? TRUE : FALSE, &rd);
        }
        else
        {
          // Return to the resource area
          NextState(0x21F96549); // "GoToResourceArea"
        }
        break;

      case SMN_PROCESS:

        // Is the resource alive ?
        if (resourceObj.Alive())
        {
          // In route to the resource
        }
        else
        {
          // Return to the resource area
          NextState(0x21F96549); // "GoToResourceArea"
        }
        break;
    }
  }


  //
  // UnitCollect::StateAlignResource
  //
  void UnitCollect::StateAlignResource(StateMachineNotify notify)
  {
    LOG_COLLECT(("StateAlignResource %d", notify))

    switch (notify)
    {
      case SMN_ENTRY:

        // Is the resource alive ?
        if (resourceObj.Alive())
        {
          // Are we one tile away ?
          if (
            (abs(subject->GetCellX() - resourceObj->GetCellX()) <= 1) &&
            (abs(subject->GetCellZ() - resourceObj->GetCellZ()) <= 1))
          {
            // What's the direction to the resource
            Vector dir = resourceObj->Origin() - subject->Origin();
            dir.Normalize();

            // Rotate by 90 degrees
            //F32 temp = dir.x;
            //dir.x = -dir.z;
            //dir.z = temp;

            // Turn to that orientation
            subject->Move(moveHandle, NULL, &dir, GetFlags() & TF_AI ? TRUE : FALSE);
          }
          else
          {
            LOG_COLLECT(("Adding [%d] to reject list", resourceObj->Id()))

            // Place the object on the reject list and reject it for REJECT_TIME seconds
            rejects.Append(resourceObj)->timeout = GameTime::SimTotalTime() + REJECT_TIME;

            // Search for another resource
            NextState(0x2E8B68BF); // "SearchForResource"
          }
        }
        else
        {
          // Search for another resource
          NextState(0x2E8B68BF); // "SearchForResource"
        }
        break;

      case SMN_PROCESS:

        // Is the resource alive ?
        if (resourceObj.Alive())
        {
          // Still turning
        }
        else
        {
          // Search for another resource
          NextState(0x2E8B68BF); // "SearchForResource"
        }
        break;
    }
  }


  //
  // UnitCollect::StatePreLoadResource
  //
  void UnitCollect::StatePreLoadResource(StateMachineNotify notify)
  {
    LOG_COLLECT(("StatePreLoadResource %d", notify))

    switch (notify)
    {
      case SMN_ENTRY:
        // Start playing the pre load animation
        subject->SetAnimation(0x64EDDD3B); // "PreLoad"
        break;
    }
  }


  //
  // FXCallback
  //
  // Collect effect
  //
  Bool UnitCollect::FXCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *)
  {
    UnitCollect * task = TaskCtrl::Promote<Tasks::UnitCollect>(mapObj);

    if (mapObj && task && task->IsCollecting() && task->resourceObj.Alive())
    {
      cbd.meshEffect.percent = 0; //task->GetProgress();

      NodeIdent ni( "HP-FIRE");
      if (mapObj->Mesh().FindIdent( ni))
      {
        cbd.particle.matrix = mapObj->WorldMatrix( ni);
      }
      else
      {
        cbd.particle.matrix = mapObj->WorldMatrix();
      }
      cbd.particle.length = task->resourceObj->Position();
      cbd.particle.length -= cbd.particle.matrix.posit;

      cbd.particle.valid = TRUE;

      return (FALSE);
    }
    return (TRUE);
  }


  //
  // UnitCollect::StateLoadResource
  //
  void UnitCollect::StateLoadResource(StateMachineNotify notify)
  {
    LOG_COLLECT(("StateLoadResource %d", notify))

    switch (notify)
    {
      case SMN_ENTRY:
        // Start playing the load animation
        subject->SetAnimation(0xCCB3995F); // "Load"

        subject->StartGenericFX( 0x9E990373, FXCallBack); // "Collector::Collect"
        break;

      case SMN_PROCESS:
      {
        // Is the resource we're after still alive ?
        if (resourceObj.Alive())
        {
          // Attempt to get resource from the resource
          U32 rate = subject->UnitType()->GetResourceLoad();
          U32 amount = resourceObj->TakeResource(rate);

          // Store the amount taken in the transport
          U32 added = subject->AddResourceTransport(amount);

          // Are we full ?
          if (added < amount)
          {
            // Return the overflow to the resource
            resourceObj->GiveResource(amount - added);

            // Loading resource completed
            NextState(0x4C8538E7); // "PostLoadResource"
          }
          else
          {
            // Did we get what we asked for ?
            if (amount < rate)
            {
              // There's no more resource, does this resource replenish
              if (resourceObj->ResourceType()->GetResourceRate() > 0)
              {
                // Are there any better resource objects nearby
                ResourceObj *better = FindResource();

                // If there's a better one, 
                // which isn't the one we're at,
                // and it's resource is over the change limit,
                // then go to it.
                if (
                  better && 
                  better != resourceObj && 
                  better->GetResource() > better->ResourceType()->GetResourceChange())
                {
                  // Stop loading and go to the better one
                  SetResourceObject(better);
                  
                  // Stop loading from this resource
                  NextState(0x4C8538E7); // "PostLoadResource"
                }
                else
                {
                  // Wait around, there'll be more resource
                }
              }
              else
              {
                // Since this is a non-regenerating resource then forget about it
                resourceObj = NULL;

                // Stop loading from this resource
                NextState(0x4C8538E7); // "PostLoadResource"
              }
            }
          }
        }
        else
        {
          // The resource died
          NextState(0x4C8538E7); // "PostLoadResource"
        }
        break;
      }
    }
  }


  //
  // UnitCollect::StatePostLoadResource
  //
  void UnitCollect::StatePostLoadResource(StateMachineNotify notify)
  {
    LOG_COLLECT(("StatePostLoadResource %d", notify))

    switch (notify)
    {
      case SMN_ENTRY:
        // Start playing the post load animation
        subject->SetAnimation(0x6C7C139F); // "PostLoad"
        break;
    }
  }


  //
  // UnitCollect::StateLeaveResource
  //
  void UnitCollect::StateLeaveResource(StateMachineNotify notify)
  {
    LOG_COLLECT(("StateLeaveResource %d", notify))

    switch (notify)
    {
      case SMN_ENTRY:

        // Are we full ?
        if (subject->GetResourceTransport() < subject->UnitType()->GetResourceTransport())
        {
          // Go to resource
          NextState(0x06083411); // "GoToResource"
        }
        else
        {
          // Go to storage
          NextState(0x33FA0EA0); // "GoToStorage"
        }
        break;
    }
  }

  //
  // UnitCollect::StateGoToStorage
  //
  void UnitCollect::StateGoToStorage(StateMachineNotify notify)
  {
    LOG_COLLECT(("StateGoToStorage %d", notify))

    switch (notify)
    {
      case SMN_ENTRY:
        // If the storage is alive move to it
        if (storageObj.Alive())
        {
          subject->MoveOnto(moveHandle, storageObj, GetFlags() & TF_AI ? TRUE : FALSE);
        }
        else
        {
          // Find another storage facility
          NextState(0x70EB48AA); // "SearchForStorage"
        }
        break;

      case SMN_PROCESS:

        // Is the storage we're after still alive ?
        if (!storageObj.Alive())
        {
          // Find another storage facility
          NextState(0x70EB48AA); // "SearchForStorage"
        }
        break;
    }
  }


  //
  // UnitCollect::StatePreUnloadResource
  //
  void UnitCollect::StatePreUnloadResource(StateMachineNotify notify)
  {
    LOG_COLLECT(("StatePreUnloadResource %d", notify))

    switch (notify)
    {
      case SMN_ENTRY:
        // Play the pre-unload animation
        subject->SetAnimation(0x96717F1D); // "PreUnload"
        break;
    }
  }


  //
  // UnitCollect::StateUnloadResource
  //
  void UnitCollect::StateUnloadResource(StateMachineNotify notify)
  {
    LOG_COLLECT(("StateUnloadResource %d", notify))

    switch (notify)
    {
      case SMN_ENTRY:
        // Start playing the pre load animation
        subject->SetAnimation(0xBE6BD9FC); // "Unload"
        break;

      case SMN_PROCESS:
      {
        // Do we have any resources to unload ?
        if (subject->GetResourceTransport())
        {
          // If the storage facility is alive, transfer resources to it
          if (storageObj.Alive())
          {
            // Transfer our resources to the storage facility
            if (storageObj->GetTeam())
            {
              U32 resource = Min(subject->GetResourceTransport(), subject->UnitType()->GetResourceUnload());
              storageObj->GetTeam()->AddResourceStore(resource);
              storageObj->GetTeam()->ReportResource(resource, "resource.collected");
              subject->RemoveResourceTransport(resource);
            }
            else
            {
              // Dump the resource
              subject->ClearResourceTransport();

              // Head out
              NextState(0xA0B4BB75); // "PostUnloadResource"
            }
          }
        }
        else
        {
          // We're done ... head out
          NextState(0xA0B4BB75); // "PostUnloadResource"
        }
        break;
      }
    }
  }


  //
  // UnitCollect::StatePostUnloadResource
  //
  void UnitCollect::StatePostUnloadResource(StateMachineNotify notify)
  {
    LOG_COLLECT(("StatePostUnloadResource %d", notify))

    switch (notify)
    {
      case SMN_ENTRY:
        // Start playing the post unload animation
        subject->SetAnimation(0xF3411A66); // "PostUnload"
        break;
    }
  }


  //
  // UnitCollect::StateGoToResourceArea
  //
  void UnitCollect::StateGoToResourceArea(StateMachineNotify notify)
  {
    LOG_COLLECT(("StateGoToResourceArea %d", notify))

    switch (notify)
    {
      case SMN_ENTRY:
        // Lets go to the spot where we last got resource
        if (haveResourceLocation)
        {
          // Move to it
          subject->Move(moveHandle, &lastResourceLocation, NULL, GetFlags() & TF_AI ? TRUE : FALSE);
        }
        else
        {
          // Move to a nearby location and wait
          subject->MoveAwayDirect(moveHandle);

          // Search for another resource
          NextState(0x2E8B68BF); // "SearchForResource"
        }
        break;

    }
  }


  //
  // UnitCollect::StateSearchForResource
  //
  void UnitCollect::StateSearchForResource(StateMachineNotify notify)
  {
    LOG_COLLECT(("StateSearchForResource %d", notify))

    switch (notify)
    {
      case SMN_ENTRY:
      case SMN_PROCESS:
      {
        // Are there any more resources 
        ResourceObj *resource = FindResource();

        if (resource)
        {
          LOG_COLLECT(("[%d] found resource [%d] {%d}", subject->Id(), resource->Id(), resource->GetResource()))

          // Found a resource, go get it
          SetResourceObject(resource);
        }
        else
        {
          // If we're carrying resources, take them back to storage
          if (subject->GetResourceTransport())
          {
            NextState(0x33FA0EA0); // "GoToStorage"
          }
        }
        break;
      }
    }
  }


  //
  // UnitCollect::StateSearchForStorage
  //
  void UnitCollect::StateSearchForStorage(StateMachineNotify notify)
  {
    LOG_COLLECT(("StateSearchForStorage %d", notify))

    switch (notify)
    {
      case SMN_ENTRY:
      case SMN_PROCESS:
      {
        Vector origin;

        // Do we have a storage location
        if (haveStorageLocation)
        {
          // Search from the storage location
          origin = lastStorageLocation;
        }
        else
        {
          // Use our current location
          origin = subject->Origin();
        }

        // Find the closest storage facility
       
        // Get the list of objects on this team
        Team *team = subject->GetTeam();

        UnitObj *winner = NULL;
        F32 bestProximity = F32_MAX;

        if (team)
        {
          for (NList<UnitObj>::Iterator u(&team->GetUnitObjects()); *u; u++)
          {
            if ((*u)->HasProperty(0xAE95DF36)) // "Ability::StoreResource"
            {
              F32 proximity = ((*u)->Origin() - origin).Magnitude2();

              if (proximity < bestProximity)
              {
                winner = *u;
                bestProximity = proximity;
              }
            }
          }
        }

        if (winner)
        {
          SetStorageObject(winner, FALSE);
        }
        break;
      }
    }
  }


  //
  // UnitCollect::FindResource
  //
  // Find the a resource
  //
  ResourceObj * UnitCollect::FindResource()
  {
    // Build filter data
    ResourceObjIter::FilterDataUnit filterData(subject);
    ResourceObjIter::Resource i(ResourceObjIter::HasBeenSeenByTeam, filterData);
    ResourceObj *obj;
    ResourceObj *winner = NULL;
    F32 bestProximity = F32_MAX;
    U32 bestResource = 0;

    while ((obj = i.Next()) != NULL)
    {
      // Is this resource on the reject list
      if (rejects.Exists(obj))
      {
        // Ignore it
        continue;
      }

      // Is this resource unzipped
      if (!obj->GetFootInstance())
      {
        LOG_WARN(("Collector ignoring non-zipped resource object [%d] at [%d,%d]", obj->Id(), obj->GetCellX(), obj->GetCellZ()));

        // Ignore it
        continue;
      }

      // Is this the first object ?
      if (!winner)
      {
        bestProximity = i.GetProximity2();
        bestResource = obj->GetResource();
        winner = obj;
      }
      else

      // Does it have less resource and is much closer
      // OR
      // Does it have more resource and isn't too far away
      // OR
      // Has the same resource and is closer
      if (
        ((obj->GetResource() > bestResource) &&
        (i.GetProximity2() < (bestProximity + BETTER_DISTANCE)))
        ||
        ((obj->GetResource() > bestResource) &&
        (i.GetProximity2() < (bestProximity + BETTER_DISTANCE)))
        ||
        ((obj->GetResource() == bestResource) &&
        (i.GetProximity2() < bestProximity)))
      {
        bestProximity = i.GetProximity2();
        bestResource = obj->GetResource();
        winner = obj;
      }
    }

    return (winner);
  }


  //
  // Initialization
  //
  void UnitCollect::Init()
  {
    // Setup config
    staticConfig.Setup();

    // State Machine procedures
    stateMachine.AddState("Wait", &StateWait);

    stateMachine.AddState("GoToResource", &StateGoToResource);
    stateMachine.AddState("AlignResource", &StateAlignResource);
    stateMachine.AddState("PreLoadResource", &StatePreLoadResource);
    stateMachine.AddState("LoadResource", &StateLoadResource);
    stateMachine.AddState("PostLoadResource", &StatePostLoadResource);

    stateMachine.AddState("LeaveResource", &StateLeaveResource);
    stateMachine.AddState("GoToStorage", &StateGoToStorage);

    stateMachine.AddState("PreUnloadResource", &StatePreUnloadResource);
    stateMachine.AddState("UnloadResource", &StateUnloadResource);
    stateMachine.AddState("PostUnloadResource", &StatePostUnloadResource);

    stateMachine.AddState("GoToResourceArea", &StateGoToResourceArea);

    stateMachine.AddState("SearchForResource", &StateSearchForResource);
    stateMachine.AddState("SearchForStorage", &StateSearchForStorage);

  }


  //
  // Shutdown
  //
  void UnitCollect::Done()
  {
    stateMachine.CleanUp();
  }
}
