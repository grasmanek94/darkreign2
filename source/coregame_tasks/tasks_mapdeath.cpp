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
#include "tasks_mapdeath.h"
#include "meshent.h"
#include "gameobjctrl.h"
#include "mapobj.h"
#include "movement_pathfollow.h"
#include "weapon.h"


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
  StateMachine<MapDeath> MapDeath::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class MapDeath
  //


  //
  // Constructor
  //
  MapDeath::MapDeath(GameObj *subject) :
    GameTask<MapObjType, MapObj>(staticConfig, subject),
    inst(&stateMachine, "Init")
  {
  }


  //
  // Save
  //
  void MapDeath::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));
  }


  //
  // Load
  //
  void MapDeath::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
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
  void MapDeath::PostLoad()
  {
  }


  //
  // Perform task processing
  //
  Bool MapDeath::Process()
  {
    inst.Process(this);  
    return (quit);
  }


  //
  // Initial state
  //
  void MapDeath::StateInit()
  {
    // Stop the objects driver
    UnitObj *unitObj = Promote::Object<UnitObjType, UnitObj>(subject);

    if (unitObj)
    {
      if (unitObj->GetWeapon())
      {
        unitObj->GetWeapon()->HaltFire();
      }

      if (unitObj->CanEverMove())
      {
        unitObj->GetDriver()->Stop(Movement::Driver::STOP_DYING);
      }
    }

    // Play the death animation
    subject->SetAnimation(0xF40D135F); // "Death"

    NextState(0x382D3C63); // "Dead"
  }


  //
  // Dead state
  //
  void MapDeath::StateDead()
  {
  }


  //
  // Kill state
  //
  void MapDeath::StateKill()
  {
    // Mark the object for deletion
    subject->MarkForDeletion();

    // We're done
    Quit();
  }


  //
  // Notify this task of an event
  //
  Bool MapDeath::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
        inst.Set(0xABAA7B48); // "Init"
        return (TRUE);

      case MapObjNotify::AnimationDone:
      {
        switch (event.param1)
        {
          case 0xF40D135F: // "Death"
            NextState(0xEF9FC2B9); // "Kill"
            break;
        }

        return (TRUE);
      }
    }
    return (GameTask<MapObjType, MapObj>::ProcessEvent(event));
  }


  //
  // RequiresDeathTask
  //
  // Returns TRUE if 'obj' requires a death task
  //
  Bool MapDeath::RequiresDeathTask(MapObj *obj)
  {
    ASSERT(obj);

    // Does this object have a death animation
    return (obj->MapType()->GetMeshRoot()->FindAnimCycle(0xF40D135F) ? TRUE : FALSE);  // Death
  }


  //
  // Initialization
  //
  void MapDeath::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Add states to the state machine
    stateMachine.AddState("Init", &MapDeath::StateInit);
    stateMachine.AddState("Dead", &MapDeath::StateDead);
    stateMachine.AddState("Kill", &MapDeath::StateKill);
  }


  //
  // Shutdown
  //
  void MapDeath::Done()
  {
    stateMachine.CleanUp();
  }

}
