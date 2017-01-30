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
#include "tasks_unitfire.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // State machine
  StateMachine<UnitFire> UnitFire::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitFire
  //


  //
  // Constructor
  //
  UnitFire::UnitFire(GameObj *subject, const Target &target, F32 horizontal, F32 vertical, F32 speed) 
  : GameTask<UnitObjType, UnitObj>(staticConfig, subject),
    inst(&stateMachine, "Init"),
    target(target),
    horizontal(horizontal),
    vertical(vertical),
    speed(speed)
  {
  }

  UnitFire::UnitFire(GameObj *subject)
  : GameTask<UnitObjType, UnitObj>(staticConfig, subject),
    inst(&stateMachine),
    target(NULL),
    horizontal(0.0f),
    vertical(0.0f),
    speed(speed)
  {
  }


  //
  // Destructor
  //
  UnitFire::~UnitFire()
  {
  }


  //
  // Save
  //
  void UnitFire::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));   
    
    if (target.Valid())
    {
      target.SaveState(fScope->AddFunction("Target"));
    }

    StdSave::TypeF32(fScope, "Horizontal", horizontal);
    StdSave::TypeF32(fScope, "Vertical", vertical);
    StdSave::TypeF32(fScope, "Speed", speed);
  }


  //
  // Load
  //
  void UnitFire::Load(FScope *fScope)
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
          target.LoadState(sScope);
          break;

        case 0x2942B3CD: // "Horizontal"
          horizontal = StdLoad::TypeF32(sScope);
          break;

        case 0xC8F18F06: // "Vertical"
          vertical = StdLoad::TypeF32(sScope);
          break;

        case 0x2B96BEE1: // "Speed"
          speed = StdLoad::TypeF32(sScope);
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
  void UnitFire::PostLoad()
  {
    target.PostLoad();
  }


  //
  // Perform task processing
  //
  Bool UnitFire::Process()
  {
    inst.Process(this);  

    // Perform type processing
    GameProcess();

    return (quit);
  }


  //
  // Notify this task of an event
  //
  Bool UnitFire::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
        inst.Set(0xABAA7B48); // "Init"
        return (TRUE);

      case 0xA4DB84E7: // "Incapable"
        quit = TRUE;
        return (TRUE);

      case 0x3369B9CC: // "Completed"
        quit = TRUE;
        return (TRUE);
    }
    return (GameTask<UnitObjType, UnitObj>::ProcessEvent(event));
  }


  //
  // Initial state
  //
  void UnitFire::StateInit()
  {
    NextState(0xA4B70EC1); // "Firing"
  }


  //
  // Firing state
  //
  void UnitFire::StateFiring()
  {
  }


  //
  // Initialization
  //
  void UnitFire::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Build state machine
    stateMachine.AddState("Init", &StateInit);
    stateMachine.AddState("Firing", &StateFiring);
  }


  //
  // Shutdown
  //
  void UnitFire::Done()
  {
    stateMachine.CleanUp();
  }

}
