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
#include "tasks_parasitebomb.h"
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
  StateMachine<ParasiteBomb> ParasiteBomb::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class ParasiteBomb
  //


  //
  // Constructor
  //
  ParasiteBomb::ParasiteBomb(GameObj *subject) :
    GameTask<ParasiteObjType, ParasiteObj>(staticConfig, subject),
    inst(&stateMachine, "Init")
  {
  }


  //
  // Save
  //
  void ParasiteBomb::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));
    timer.SaveState(fScope->AddFunction("Timer"));
  }


  //
  // Load
  //
  void ParasiteBomb::Load(FScope *fScope)
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
  void ParasiteBomb::PostLoad()
  {
  }


  //
  // Perform task processing
  //
  Bool ParasiteBomb::Process()
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
  void ParasiteBomb::StateInit()
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
      NextState(0xEBCE0E53); // "Fizzle"
    }
  }


  //
  // StateStage1
  //
  void ParasiteBomb::StateStage1()
  {
    // Is the target still alive
    if (subject->GetTarget())
    {
      // Is it time to be detected
      if (timer.Test())
      {
        // Signal team radio that target was infected
        if (subject->GetTarget()->GetTeam() && subject->ParasiteType()->GetUseRadio())
        {
          // "ParasiteBomb::Detected"
          subject->GetTarget()->GetTeam()->GetRadio().Trigger
          (
            0x89B999DA, Radio::Event(subject->GetTarget())
          );
        }

        // Trigger the effect
        subject->MapType()->StartGenericFX(subject->GetTarget(), 0x85FF6A8F); // "Parasite::Detected"

        // Start the countdown
        timer.Start(subject->ParasiteType()->GetTime2());

        // Finished
        NextState(0xC438DD60); // "Stage2"
      }     
    }
    else
    {
      NextState(0xEBCE0E53); // "Fizzle"
    }
  }

  
  //
  // StateStage2
  //
  void ParasiteBomb::StateStage2()
  {
    // Is the target still alive
    if (subject->GetTarget())
    {
      // Is it time to kill
      if (timer.Test())
      {
        // Blow up the parasite
        subject->SelfDestruct(TRUE, subject->GetTeam());

        // Kill the target
        subject->GetTarget()->SelfDestruct();

        // Finished
        NextState(0x8C1F21CA); // "Done"
      }     
    }
    else
    {
      NextState(0xEBCE0E53); // "Fizzle"
    }
  }


  //
  // StateFizzle
  //
  // Bomb didn't go off
  //
  void ParasiteBomb::StateFizzle()
  {
    // Kill the parasite
    subject->SelfDestruct();

    // Finished
    NextState(0x8C1F21CA); // "Done"
  }


  //
  // StateDone
  //
  // Sit and do nothing
  //
  void ParasiteBomb::StateDone()
  {
  }


  //
  // Notify
  //
  // Notify this task of an event
  //
  Bool ParasiteBomb::ProcessEvent(const Event &event)
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
  void ParasiteBomb::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Build state machine
    stateMachine.AddState("Init", &ParasiteBomb::StateInit);
    stateMachine.AddState("Stage1", &ParasiteBomb::StateStage1);
    stateMachine.AddState("Stage2", &ParasiteBomb::StateStage2);
    stateMachine.AddState("Fizzle", &ParasiteBomb::StateFizzle);
    stateMachine.AddState("Done", &ParasiteBomb::StateDone);
  }


  //
  // Shutdown
  //
  void ParasiteBomb::Done()
  {
    stateMachine.CleanUp();
  }
}
