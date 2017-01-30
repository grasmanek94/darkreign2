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
#include "tasks_unitmove.h"
#include "trailobj.h"
#include "resolver.h"


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
  StateMachine<UnitMove> UnitMove::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitMove
  //


  //
  // Constructor
  //
  UnitMove::UnitMove(GameObj *subject) :
    GameTask<UnitObjType, UnitObj>(staticConfig, subject),
    inst(&stateMachine),
    hasDest(FALSE),
    hasDir(FALSE),
    hasCustomReq(FALSE)
  {
  }


  //
  // Constructor
  //
  UnitMove::UnitMove(GameObj *subject, const Vector &destination) :
    GameTask<UnitObjType, UnitObj>(staticConfig, subject),
    inst(&stateMachine, "Init"),
    hasDest(TRUE),
    destination(destination),
    hasDir(FALSE),
    hasCustomReq(FALSE)
  {
  }


  //
  // Constructor
  //
  UnitMove::UnitMove(GameObj *subject, const Vector &destination, const Vector &direction) : 
    GameTask<UnitObjType, UnitObj>(staticConfig, subject),
    inst(&stateMachine, "Init"),
    hasDest(TRUE),
    destination(destination),
    hasDir(TRUE),
    direction(direction),
    hasCustomReq(FALSE)
  {
  }

  
  //
  // Constructor
  //
  UnitMove::UnitMove(GameObj *subject, TrailObj *trail, U32 index) :
    GameTask<UnitObjType, UnitObj>(staticConfig, subject),
    inst(&stateMachine, "Init"),
    hasDest(FALSE),
    hasDir(FALSE),
    trail(trail),
    trailIndex(index),
    hasCustomReq(FALSE)
  {
  }

    
  //
  // Constructor
  //
  UnitMove::UnitMove(GameObj *subject, UnitObj *moveOn) :
    GameTask<UnitObjType, UnitObj>(staticConfig, subject),
    inst(&stateMachine, "Init"),
    hasDest(FALSE),
    hasDir(FALSE),
    moveOn(moveOn),
    hasCustomReq(FALSE)
  {
  }


  //
  // Save
  //
  void UnitMove::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));   

    if (hasDest)
    {
      StdSave::TypeVector(fScope, "Destination", destination);
    }

    if (hasDir)
    {
      StdSave::TypeVector(fScope, "Direction", direction);
    }

    if (hasCustomReq)
    {
      requestData.SaveState(fScope->AddFunction("RequestData"));
    }      
   
    if (trail.Alive())
    {
      StdSave::TypeReaper(fScope, "Trail", trail);
      StdSave::TypeU32(fScope, "TrailIndex", trailIndex);
      follower.SaveState(fScope->AddFunction("Follower"));
    }

    StdSave::TypeReaper(fScope, "MoveOn", moveOn);
    moveHandle.SaveState(fScope->AddFunction("MoveHandle"));
  }


  //
  // Load
  //
  void UnitMove::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
          break;

        case 0xA302E408: // "Destination"
          hasDest = TRUE;
          StdLoad::TypeVector(sScope, destination);
          break;

        case 0x04BC5B80: // "Direction"
          hasDir = TRUE;
          StdLoad::TypeVector(sScope, direction);
          break;

        case 0x82698073: // "Trail"
          StdLoad::TypeReaper(sScope, trail);
          break;

        case 0xA3998582: // "TrailIndex"
          trailIndex = StdLoad::TypeU32(sScope);
          break;

        case 0x90C84087: // "Follower"
          follower.LoadState(sScope);
          break;

        case 0x67E395E2: // "MoveOn"
          StdLoad::TypeReaper(sScope, moveOn);
          break;

        case 0x37E65F22: // "RequestData"
          hasCustomReq = TRUE;
          requestData.LoadState(sScope);
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
  void UnitMove::PostLoad()
  {
    Resolver::Object<TrailObj, TrailObjType>(trail);
    Resolver::Object<UnitObj, UnitObjType>(moveOn);
    follower.PostLoad();
  }


  //
  // Setup request data
  //
  void UnitMove::SetupRequest(const Movement::RequestData &rd)
  {
    requestData = rd;
    hasCustomReq = TRUE;
  }


  //
  // Get request data
  //
  Movement::RequestData *UnitMove::GetRequest()
  {
    if (hasCustomReq)
    {
#ifdef DEVELOPMENT
      if ((GetFlags() & TF_FROM_ORDER) != requestData.fromOrder)
      {
        LOG_ERR(("GetFlags() & TF_FROM_ORDER) != requestData.fromOrder"))
        LOG_ERR(("  Unit=%d (%s)", subject->Id(), subject->TypeName()))
      }
#endif
      return (&requestData);
    }
    else

    if (GetFlags() & TF_FROM_ORDER)
    {
      return (&Movement::RequestData::orders);
    }

    return (NULL);
  }


  //
  // Perform task processing
  //
  Bool UnitMove::Process()
  {
    inst.Process(this);  

    // Perform type processing
    GameProcess();

    return (quit);
  }


  //
  // NextTrailPoint
  //
  // Move to the next point in the trail
  //
  Bool UnitMove::NextTrailPoint()
  {
    Point<U32> p;

    // Get the current trail position
    if (follower.GetPos(p))
    {
      // Update the index so interruption will be handled
      trailIndex = follower.GetIndex();

      // Generate the destination
      Vector dest(WorldCtrl::CellToMetresX(p.x), 0.0F, WorldCtrl::CellToMetresZ(p.z));

      // Tell the unit to move
      subject->Move(moveHandle, &dest, NULL, GetFlags() & TF_AI ? TRUE : FALSE, GetRequest());

      // Proceed to the next point
      follower.Next();

      return (TRUE);
    }

    return (FALSE);
  }


  //
  // Initial state
  //
  void UnitMove::StateInit()
  {
    ASSERT(subject->CanEverMove())

    // Move onto another unit
    if (moveOn.Alive())
    {
      subject->MoveOnto(moveHandle, moveOn, GetFlags() & TF_AI ? TRUE : FALSE, GetRequest());
    }
    else

    // Follow a trail
    if (trail.Alive())
    {
      // Setup the follower
      follower.Set(trail, trailIndex);

      // Move to the first point
      if (!NextTrailPoint())
      {
        Quit();
      }
    }
    else

    // Move to a location and/or direction
    if (hasDest || hasDir)
    {
      subject->Move(
        moveHandle, 
        (hasDest) ? &destination : NULL, 
        (hasDir) ? &direction : NULL, 
        GetFlags() & TF_AI ? TRUE : FALSE, 
        GetRequest());
    }
    else
    {
      Quit();
      return;
    }

    // Proceed
    NextState(0x9E947215); // "Moving"
  }


  //
  // Moving state
  //
  void UnitMove::StateMoving()
  {
    // Thumb twiddling in progress
  }


  //
  // Notify this task of an event
  //
  Bool UnitMove::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
        NextState(0xABAA7B48); // "Init"
        return (TRUE);

      case Movement::Notify::Incapable:
        if (moveHandle == event.param1)
        {
          subject->PostSquadEvent(Event(0x6B0DB5AA, GetTaskId())); // "Move::Incapable"
          Quit();
        }
        return (TRUE);

      case Movement::Notify::Completed:
      {
        if (moveHandle == event.param1)
        {
          // Not following a trail, or finished the trail we're on
          if (!trail.Alive() || !NextTrailPoint())
          { 
            subject->NotifyPlayer(0x61FC2088); // "Unit::MoveCompleted"
            subject->PostSquadEvent(Event(0xFCBF8881, GetTaskId())); // "Move::Completed"

            Quit();
          }
        }
        return (TRUE);
      }
    }
    return (GameTask<UnitObjType, UnitObj>::ProcessEvent(event));
  }


  //
  // Initialization
  //
  void UnitMove::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Build state machine
    stateMachine.AddState("Init", &UnitMove::StateInit);
    stateMachine.AddState("Moving", &UnitMove::StateMoving);
  }


  //
  // Shutdown
  //
  void UnitMove::Done()
  {
    stateMachine.CleanUp();
  }
}
