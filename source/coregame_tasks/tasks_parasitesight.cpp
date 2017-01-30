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
#include "tasks_parasitesight.h"
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
  StateMachine<ParasiteSight> ParasiteSight::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class ParasiteSight
  //


  //
  // Constructor
  //
  ParasiteSight::ParasiteSight(GameObj *subject) :
    GameTask<ParasiteObjType, ParasiteObj>(staticConfig, subject),
    inst(&stateMachine, "Init")
  {
  }


  //
  // Save
  //
  void ParasiteSight::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));
    timer.SaveState(fScope->AddFunction("Timer"));
  }


  //
  // Load
  //
  void ParasiteSight::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
          break;

        case 0x5AB44811: // "Timer"
          timer.LoadState(sScope);
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
  void ParasiteSight::PostLoad()
  {
  }


  //
  // Perform task processing
  //
  Bool ParasiteSight::Process()
  {
    inst.Process(this);  
    
    // Idle task never terminates
    return (FALSE);
  }


  //
  // StateInit
  //
  // Just attached to target
  //
  void ParasiteSight::StateInit()
  {
    // Is the target still alive
    if (subject->GetTarget())
    {
      // Start the countdown
      timer.Start(subject->ParasiteType()->GetTime1());

      // Proceed
      NextState(0xC97BFBB9); // "Stage1"
    }
    else
    {
      NextState(0x8C1F21CA); // "Done"
    }
  }


  //
  // StateStage1
  //
  // Waiting to alert target's player
  //
  void ParasiteSight::StateStage1()
  {
    // Is the target still alive
    if (subject->GetTarget())
    {
      // Is it time to warn player
      if (timer.Test())
      {
        // Signal team radio that target was infected
        if (subject->GetTarget()->GetTeam() && subject->ParasiteType()->GetUseRadio())
        {
          // "ParasiteSight::Detected"
          subject->GetTarget()->GetTeam()->GetRadio().Trigger
          (
            0xBCEA031C, Radio::Event(subject->GetTarget())
          );
        }

        // Trigger the effect
        subject->MapType()->StartGenericFX(subject->GetTarget(), 0x85FF6A8F); // "Parasite::Detected"

        // Start the countdown
        timer.Start(subject->ParasiteType()->GetTime2());

        // Proceed
        NextState(0xC438DD60); // "Stage2"
      }
    }
    else
    {
      NextState(0x8C1F21CA); // "Done"
    }
  }


  //
  // StateStage2
  //
  // Waiting to die
  //
  void ParasiteSight::StateStage2()
  {
    // Is the target still alive
    if (subject->GetTarget())
    {
      // Is the parasite done
      if (timer.Test())
      {
        NextState(0x8C1F21CA); // "Done"
      }
    }
    else
    {
      NextState(0x8C1F21CA); // "Done"
    }
  }


  //
  // StateDone
  //
  // Parasite is terminated
  //
  void ParasiteSight::StateDone()
  {
    // Kill the parasite
    subject->MarkForDeletion();
  }


  //
  // Notify
  //
  // Notify this task of an event
  //
  Bool ParasiteSight::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
        return (TRUE);
    }

    return (GameTask<ParasiteObjType, ParasiteObj>::ProcessEvent(event));
  }


  //
  // Initialization
  //
  void ParasiteSight::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Build state machine
    stateMachine.AddState("Init", &ParasiteSight::StateInit);
    stateMachine.AddState("Stage1", &ParasiteSight::StateStage1);
    stateMachine.AddState("Stage2", &ParasiteSight::StateStage2);
    stateMachine.AddState("Done", &ParasiteSight::StateDone);
  }


  //
  // Shutdown
  //
  void ParasiteSight::Done()
  {
    stateMachine.CleanUp();
  }
}
