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
#include "tasks_unitbuild.h"
#include "team.h"
#include "wallobj.h"
#include "taskctrl.h"
#include "resolver.h"
#include "unitevacuate.h"
#include "sync.h"


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
  StateMachine<UnitBuild> UnitBuild::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitBuild
  //

  //
  // FXCallback
  //
  // Construction fx callback
  //
  Bool UnitBuild::FXCallBack(MapObj *mapObj, FX::CallBackData &, void *)
  {
    return (!mapObj->SendEvent(Task::Event(0x2013F610))); // "UnitBuild::EffectProcess"
  }


  //
  // Constructor
  //
  UnitBuild::UnitBuild(GameObj *subject) 
  : GameTask<UnitObjType, UnitObj>(staticConfig, subject),
    inst(&stateMachine),
    initialized(FALSE),
    claimInfo(NULL),
    claimed(FALSE),
    blocking(FALSE)
  {
    LOG_LOCAL(("UnitBuild::Constructor1"));
  }


  //
  // Constructor
  //
  UnitBuild::UnitBuild(GameObj *subject, UnitObjType *build, const Vector &dest, WorldCtrl::CompassDir dir) 
  : GameTask<UnitObjType, UnitObj>(staticConfig, subject),
    inst(&stateMachine, "Init"),
    initialized(FALSE),
    build(build),
    claimInfo(NULL),
    claimed(FALSE),
    blocking(FALSE)
  {
    LOG_LOCAL(("UnitBuild::Constructor2"));

    // Setup the construction location
    WorldCtrl::SetupWorldMatrix(location, dest, dir);
  }


  //
  // Destructor
  //
  UnitBuild::~UnitBuild()
  {
    claimInfo.Release();
  }


  //
  // Save
  //
  void UnitBuild::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));
    StdSave::TypeU32(fScope, "Initialized", initialized);
    StdSave::TypeReaperObjType(fScope, "Build", build);
    StdSave::TypeMatrix(fScope, "Location", location);
    StdSave::TypeVector(fScope, "Destination", destination);
    moveHandle.SaveState(fScope->AddFunction("MoveHandle"));
    StdSave::TypeReaper(fScope, "Target", target);
    StdSave::TypeU32(fScope, "Claimed", claimed);
    StdSave::TypeU32(fScope, "Blocking", blocking);
  }

  //
  // Load
  //
  void UnitBuild::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
          break;

        case 0x82771352: // "Initialized"
          initialized = StdLoad::TypeU32(sScope);
          break;

        case 0x10275578: // "Build"
          StdLoad::TypeReaperObjType(sScope, build);
          break;

        case 0x693D5359: // "Location"
          StdLoad::TypeMatrix(sScope, location);
          break;

        case 0xA302E408: // "Destination"
          StdLoad::TypeVector(sScope, destination);
          break;

        case 0xD3D101D2: // "MoveHandle"
          moveHandle.LoadState(sScope);
          break;

        case 0xD6A8B702: // "Target"
          StdLoad::TypeReaper(sScope, target);
          break;

        case 0xA06E31B5: // "Claimed"
          claimed = StdLoad::TypeU32(sScope);
          break;

        case 0x741B0A74: // "Blocking"
          blocking = StdLoad::TypeU32(sScope);
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
  void UnitBuild::PostLoad()
  {
    Resolver::Type(build, TRUE);
    Resolver::Object<UnitObj, UnitObjType>(target);

    // Setup footprint data
    if (initialized)
    {
      // Setup the base footprint data
      InitializeFootData();
      
      // Do an initial check at our location
      FootPrint::Placement::Result placeResult;

      if (CheckConstruction(FALSE, placeResult))
      {
        // Do we need to claim the cells
        if (claimed)
        {
          place.Claim(claimInfo, TRUE);
        }
      }
    }
  }


  //
  // InitializeFootData
  //
  // Initialize the footprint data
  //
  void UnitBuild::InitializeFootData()
  {
    // Get footprint type
    if ((foot = build->GetFootPrintType()) == NULL)
    {
      ERR_CONFIG(("Attempt to construct non-footprinted type '%s'", build->GetName()));
    }

    // Initialize the footprint placer
    place.Init(foot);
  }


  //
  // Perform task processing
  //
  Bool UnitBuild::Process()
  {
    if (subject->GetTeam())
    {
      inst.Process(this);
    }
    else
    {
      Quit();
    }

    return (quit);
  }


  //
  // StateInit
  //
  // Setup data and start moving to construction site
  //
  void UnitBuild::StateInit()
  {
    LOG_LOCAL(("UnitBuild::State::Init"));

    // Ensure this rig can move
    if (subject->CanEverMove())
    {
      // Initialize the footprint data
      InitializeFootData();

      // Adjust the location to be cell aligned
      place.AdjustLocation(location);

      // Setup the placement object for our destination
      FootPrint::Placement::Result placeResult;

      if (!CheckConstruction(FALSE, placeResult))
      {
        // Attempt to clear the area before we arrive
        Evacuate();
      }

      // Setup movement request data
      Movement::RequestData rd(GetFlags() & TF_FROM_ORDER ? TRUE : FALSE);
      rd.giveUpGrains = 0;

      // Get the current cell location of the rig
      Point<S32> pos(subject->GetCellX(), subject->GetCellZ());

      // Get the closest point on the fringe
      place.ClampToFringe(pos);

      // Set the destination in metres
      destination.Set(WorldCtrl::CellToMetresX(pos.x), 0.0F, WorldCtrl::CellToMetresZ(pos.z));

      // Calculate the turn vector
      Vector turn(location.posit - destination);
      turn.Normalize();

      LOG_LOCAL(("Before Move [%d]", moveHandle));

      // Tell the subject to move
      subject->Move(moveHandle, &destination, &turn, GetFlags() & TF_AI ? TRUE : FALSE, &rd);

      LOG_LOCAL(("After Move [%d]", moveHandle));

      // We are now initialized
      initialized = TRUE;

      // Proceed
      NextState(0x9E947215); // "Moving"
    }
    else
    {
      Quit();
    }
  }


  //
  // StateMoving
  //
  // In transit to construction site
  //
  void UnitBuild::StateMoving()
  { 
    LOG_LOCAL(("UnitBuild::State::Moving"));
  }

  
  //
  // StateLimited
  //
  // Limited by the team based command cost
  //
  void UnitBuild::StateLimited()
  { 
    LOG_LOCAL(("UnitBuild::State::Limited"));

    // Can we proceed
    if (!subject->GetTeam()->UnitLimitReached(build, subject->UnitType()))
    {
      NextState(0x42D2902A); // "Arrived"
    }
  }


  //
  // StateArrived
  //
  // Reached construction site
  //
  void UnitBuild::StateArrived(StateMachineNotify notify)
  {
    switch (notify)
    {
      case SMN_ENTRY:
      {
        LOG_LOCAL(("UnitBuild::State::Arrived::Entry"));

        // Is the unit limit reached
        if (subject->GetTeam()->UnitLimitReached(build, subject->UnitType()))
        {
          NextState(0x3F30F341); // "Limited"
        }
        else
        {
          // Are we at an invalid location
          if (CheckLocation())
          {
            // Set the timeout timer
            timer.Start(60.0F);
          }
          else
          {
            // Unable to build here
            NextState(0x0BC1460D); // "BadLocation"
          }
        }

        break;
      }

      case SMN_PROCESS:
      {
        LOG_LOCAL(("UnitBuild::State::Arrived::Process"));

        // Can we build here yet
        FootPrint::Placement::Result placeResult;
        
        if (CheckConstruction(TRUE, placeResult))
        {
          // Claim the area
          place.Claim(claimInfo, TRUE);

          // Flag that we have claimed
          claimed = TRUE;

          // Block other tasks from here on in
          blocking = TRUE;

          // Success, so start thumping
          NextState(0xF9853018); // "Thumping"
        }
        else
        {
          // Is it time to give up
          if (timer.Test())
          {
            LOG_LOCAL(("Giving up because timeout elapsed"));

            // Unable to build here
            NextState(0x0BC1460D); // "BadLocation"
          }
          else
          {
            // If not waiting for another thumping operation to complete...
            if (placeResult != FootPrint::Placement::PR_THUMPING)
            {
              // Try and clear the area
              switch (Evacuate())
              {
                case UnitEvacuate::AVAILABLE:
                case UnitEvacuate::UNKNOWN:
                case UnitEvacuate::STATIC:
                  NextState(0x0BC1460D); // "BadLocation"
                  break;

                case UnitEvacuate::MOBILE:
                  break;
              }
            }
          }
        }
        break;
      }
    }
  }


  //
  // StateThumping
  //
  // Thumping terrain to fit zip
  //
  void UnitBuild::StateThumping(StateMachineNotify notify)
  {
    switch (notify)
    {
      case SMN_ENTRY:
      {
        LOG_LOCAL(("UnitBuild::State::Thumping::Entry"));

        // Start effects
        if (build->GetConstructorConsume())
        {
          subject->StartGenericFX(0xF9614067, FXCallBack); // "Build::Process::Consume"
        }
        else
        {
          subject->StartGenericFX(0x0E352BE5, FXCallBack); // "Build::Process::NoConsume"
        }
        break;
      }

      case SMN_PROCESS:
      {
        LOG_LOCAL(("UnitBuild::State::Thumping::Process"));

        // Start a modification session
        TerrainData::SessionStart();

        // Zip the terrain
        if (place.ThumpTerrain(location.posit.y, 0.1F))
        {
          // Proceed
          NextState(0x197A7A41); // "PreProcess"
        }
        else
        {
          // Process each cycle while thumping
          ThinkFast();
        }

        // End the session
        TerrainData::SessionEnd();

        break;
      }
    }
  }


  //
  // StatePreProcess
  //
  // Play an animation
  //
  void UnitBuild::StatePreProcess(StateMachineNotify notify)
  {
    switch (notify)
    {
      case SMN_ENTRY:
      {
        LOG_LOCAL(("UnitBuild::State::PreProcess::Entry"));

        // Does the target consume rigs
        if (build->GetConstructorConsume())
        {
          NextState(0x10275578); // "Build"
        }
        else
        {
          subject->SetAnimation(0x197A7A41); // "PreProcess"
        }
        break;
      }
    }
  }


  //
  // StateBuild
  //
  // Construct the object
  //
  void UnitBuild::StateBuild(StateMachineNotify notify)
  {
    switch (notify)
    {
      case SMN_ENTRY:
      {
        LOG_LOCAL(("UnitBuild::State::Build::Entry"));

        ASSERT(target.Dead())

        // Release our claims
        place.Claim(claimInfo, FALSE);

        // Clear claim flag
        claimed = FALSE;

        // Verify that we can still build here
        FootPrint::Placement::Result placeResult;

        if (CheckConstruction(FALSE, placeResult))
        {
          // Create the object
          target = &build->Spawn(location, subject->GetTeam(), TRUE);

          // Tell the player about it
          subject->NotifyPlayer(0x4034E822, target->Id()); // "UnitBuild::Constructing"

          // Tell the target that our subject is constructing it    
          target->PostEvent(Task::Event(0x68604C50, subject)); // "UnitBuild::SetConstructor"

          // Does the target consume rigs
          if (build->GetConstructorConsume())
          {
            NextState(0x183E65F2); // "Consume"
          }
          else
          {
            // Start the process animation
            subject->SetAnimation(0xBDF7C019); // "Process"
          }
        }
        else
        {
          // Unable to build here
          NextState(0x0BC1460D); // "BadLocation"
        }

        break;
      }

      case SMN_PROCESS:
      {
        LOG_LOCAL(("UnitBuild::State::Build::Process"));

        // Has the target been destroyed
        if (target.Dead())
        {
          NextState(0x5A8B571B); // "Finished"
        }
        break;
      }
    }
  }


  //
  // StateConsume
  //
  // Consume the constructor
  //
  void UnitBuild::StateConsume(StateMachineNotify notify)
  {
    switch (notify)
    {
      case SMN_ENTRY:
        LOG_LOCAL(("UnitBuild::State::Consume::Entry"));
        subject->SetAnimation(0x183E65F2); // "Consume"
        break;
    }
  }


  //
  // StateFinished
  //
  // Construction is complete
  //
  void UnitBuild::StateFinished()
  {
    LOG_LOCAL(("UnitBuild::State::Finished"));

    // May have arrived here because target was destroyed
    if (target.Alive())
    {
      // Tell the team that we built something
      subject->GetTeam()->ReportConstruction(target);

      // Are we building a wall and we're not AI
      if (!subject->GetTeam()->IsAI())
      {
        if (WallObj *wall = Promote::Object<WallObjType, WallObj>(target))
        {
          wall->SetupAutoLink();
        }
      }
    }

    // Does the target consume rigs
    if (build->GetConstructorConsume())
    {
      Quit();
      subject->MarkForDeletion();
    }
    else
    {
      // Play finish animation
      NextState(0x927FACBD); // "PostProcess"
    }
  }


  //
  // StatePostProcess
  //
  // Play the final animation
  //
  void UnitBuild::StatePostProcess()
  {
    LOG_LOCAL(("UnitBuild::State::PostProcess"));
  }


  //
  // StateBadLocation
  //
  // Unable to construct at the given location
  //
  void UnitBuild::StateBadLocation()
  {
    LOG_LOCAL(("UnitBuild::State::PreProcess::BadLocation"));

    // Inform the player
    subject->NotifyPlayer(0x15B758A7); // "UnitBuild::BadLocation"

    if (subject->GetTeam())
    {
      // "Tasks::UnitBuild::BadLocation"
      subject->GetTeam()->GetRadio().Trigger(0xAF62C961, Radio::Event(subject)); 
    }
   
    // Finish
    Quit();
  }


  //
  // Notify this task of an event
  //
  Bool UnitBuild::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
      {
        LOG_LOCAL(("UnitBuild::Event::Interrupted"));

        // Release claims if interrupted
        if (claimed)
        {
          place.Claim(claimInfo, FALSE);
          claimed = FALSE;
        }

        // Destroy the object being constructed
        if (target.Alive())
        {
          // Tell the target to bugger off
          target->SelfDestruct(FALSE, subject->GetTeam());

          // If being consumed, destroy
          if (inst.Test(0x183E65F2)) // "Consume"
          {
            subject->MarkForDeletion();
          }
          else
          {
            Quit();
          }
        }
        else
        {
          NextState(0xABAA7B48); // "Init"
        }
        return (TRUE);
      }

      case Movement::Notify::Incapable:
      {
        LOG_LOCAL(("1 - UnitBuild::Event::Movement::Incapable [%d / %d]", moveHandle, event.param1));

        if (moveHandle == event.param1)
        {
          LOG_LOCAL(("UnitBuild::Event::Movement::Incapable"));

          // Did not get to requested position, but might be close enough
          if (CheckLocation())
          {
            NextState(0x42D2902A); // "Arrived"
          }
          else
          {
            LOG_LOCAL(("Rig could not reach location: [%s][%s/%d][%s]", subject->GetTeam() ? subject->GetTeam()->GetName() : "None", subject->TypeName(), subject->Id(), build->GetName()));
            Quit();
          }
        }
        return (TRUE);
      }

      case Movement::Notify::Completed:
      {
        LOG_LOCAL(("1 - UnitBuild::Event::Movement::Completed [%d / %d]", moveHandle, event.param1));

        if (moveHandle == event.param1)
        {
          LOG_LOCAL(("UnitBuild::Event::Movement::Completed"));

          NextState(0x42D2902A); // "Arrived"
        }
        return (TRUE);
      }

      case MapObjNotify::AnimationDone:
      {
        switch (event.param1)
        {
          case 0x197A7A41: // "PreProcess"
            LOG_LOCAL(("UnitBuild::Event::AnimationDone::PreProcess"));
            NextState(0x10275578); // "Build"
            break;

          case 0xBDF7C019: // "Process"
          {
            LOG_LOCAL(("UnitBuild::Event::AnimationDone::Process"));

            if (inst.Test(0x927FACBD)) // "PostProcess"
            {
              subject->SetAnimation(0x927FACBD); // "PostProcess"
            }
            else
            {
              subject->SetAnimation(0xBDF7C019); // "Process"
            }
            break;
          }

          case 0x183E65F2: // "Consume"
            LOG_LOCAL(("UnitBuild::Event::AnimationDone::Consume"));
            NextState(0x5A8B571B); // "Finished"
            break;

          case 0x927FACBD: // "PostProcess"
            LOG_LOCAL(("UnitBuild::Event::AnimationDone::PostProcess"));
            Quit();
            break;
        }
        return (TRUE);
      }

      case 0x2013F610: // "UnitBuild::EffectProcess"
      {
        // Should effects still be processed
        return 
        (
          inst.Test(0xF9853018) // "Thumping"
          ||
          inst.Test(0x197A7A41) // "PreProcess"
          ||
          inst.Test(0x10275578) // "Build"
          ||
          inst.Test(0x183E65F2) // "Consume"
        );
      }

      case 0xFAC5A90A: // "UnitConstruct::Completed"
      {
        LOG_LOCAL(("UnitBuild::Event::UnitConstruct::Completed"));

        // Ignore completion if playing consume animation
        if (!inst.Test(0x183E65F2)) // "Consume"
        {
          NextState(0x5A8B571B); // "Finished"
        }
        return (TRUE);
      }

      case 0x160E89B9: // "UnitConstruct::Interrupted"
      {
        LOG_LOCAL(("UnitBuild::Event::UnitConstruct::Interrupted"));
        NextState(0x5A8B571B); // "Finished"
        return (TRUE);
      }
    }

    return (GameTask<UnitObjType, UnitObj>::ProcessEvent(event));
  }


  //
  // Evacuate
  //
  // Evacuate the placement area (TRUE if we should give up)
  //
  UnitEvacuate::Result UnitBuild::Evacuate()
  {
    UnitEvacuate evac;

    // Setup the evacuator
    place.SetupEvacuator(evac);

    // Try and clear the area
    return (evac.Process(subject->GetTeam(), subject));
  }


  //
  // CheckConstruction
  //
  // Can we still construct
  //
  Bool UnitBuild::CheckConstruction(Bool thumpTest, FootPrint::Placement::Result &placeResult)
  {
    LOG_LOCAL(("UnitBuild::CheckConstruction"));

    // Set placement flags
    U32 flags = thumpTest ? 0 : FootPrint::Placement::CHECK_NOTHUMPTEST;

    // Check placement
    placeResult = place.Check(location, flags);

    if (placeResult == FootPrint::Placement::PR_OK)
    {
      // Use the height supplied by the thump test
      if (thumpTest)
      {
        location.posit.y = place.GetThumpHeight();
      }

      // On a team and can currently build this type
      if (subject->GetTeam() && subject->CanConstruct(build))
      {
        // Success
        return (TRUE);
      }
    }

    // Can't construct
    return (FALSE);
  }


  //
  // CheckLocation
  //
  // Are we in the correct location to construct
  //
  Bool UnitBuild::CheckLocation()
  {
    LOG_LOCAL(("UnitBuild::CheckLocation"));

    return (place.CellOnFringe(Point<S32>(subject->GetCellX(), subject->GetCellZ())));
  }


  //
  // GetBlockingPriority
  //
  // Get the blocking priority of this task
  //
  U32 UnitBuild::GetBlockingPriority()
  {   
    return (blocking ? TaskCtrl::BLOCK_UNAVAILABLE : GameTask<UnitObjType, UnitObj>::GetBlockingPriority());
  }


  //
  // Abort
  //
  // Abort construction cleanly
  //
  void UnitBuild::Abort()
  {
    switch (inst.GetNameCrc())
    {
      case 0x197A7A41: // "PreProcess"
      case 0x10275578: // "Build"
        NextState(0x5A8B571B); // "Finished"
        break;

      case 0x183E65F2: // "Consume"
      case 0x5A8B571B: // "Finished"
      case 0x927FACBD: // "PostProcess"
      case 0x0BC1460D: // "BadLocation"
        break;

      default:
        Quit();
    }
  }


  //
  // Initialization
  //
  void UnitBuild::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Build state machine
    stateMachine.AddState("Init", &UnitBuild::StateInit);
    stateMachine.AddState("Moving", &UnitBuild::StateMoving);
    stateMachine.AddState("Limited", &UnitBuild::StateLimited);
    stateMachine.AddState("Arrived", &UnitBuild::StateArrived);
    stateMachine.AddState("Thumping", &UnitBuild::StateThumping);
    stateMachine.AddState("PreProcess", &UnitBuild::StatePreProcess);
    stateMachine.AddState("Build", &UnitBuild::StateBuild);
    stateMachine.AddState("Consume", &UnitBuild::StateConsume);
    stateMachine.AddState("Finished", &UnitBuild::StateFinished);
    stateMachine.AddState("PostProcess", &UnitBuild::StatePostProcess);
    stateMachine.AddState("BadLocation", &UnitBuild::StateBadLocation);
  }


  //
  // Shutdown
  //
  void UnitBuild::Done()
  {
    stateMachine.CleanUp();
  }
}
