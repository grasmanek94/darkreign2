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
#include "tasks_squadformation.h"


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
  StateMachine<SquadFormation> SquadFormation::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class SquadFormation
  //


  //
  // Constructor
  //
  SquadFormation::SquadFormation(GameObj *subject) :
    GameTask<SquadObjType, SquadObj>(staticConfig, subject),
    inst(&stateMachine)
  {
  }


  //
  // Constructor
  //
  SquadFormation::SquadFormation(GameObj *subject, U32 formation, const Vector &location, F32 direction)
  : GameTask<SquadObjType, SquadObj>(staticConfig, subject),
    inst(&stateMachine, "Init"),
    formation(formation),
    location(location),
    direction(direction)
  {
  }


  //
  // Destructor
  //
  SquadFormation::~SquadFormation()
  {
  }


  //
  // Save
  //
  void SquadFormation::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));
    StdSave::TypeU32(fScope, "Formation", formation);
    StdSave::TypeVector(fScope, "Location", location);
    StdSave::TypeF32(fScope, "Direction", direction);
  }


  //
  // Load
  //
  void SquadFormation::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
          break;

        case 0x7223612A: // "Formation"
          formation = StdLoad::TypeU32(sScope);
          break;

        case 0x693D5359: // "Location"
          StdLoad::TypeVector(sScope, location);
          break;

        case 0x04BC5B80: // "Direction"
          direction = StdLoad::TypeF32(sScope);
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
  void SquadFormation::PostLoad()
  {
  }


  //
  // Perform task processing
  //
  Bool SquadFormation::Process()
  {
    inst.Process(this);  

    return (quit);
  }


  //
  // Notify this task of an event
  //
  Bool SquadFormation::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
        inst.Set(0xABAA7B48); // "Init"
        return (TRUE);

      case 0x6B0DB5AA: // "Move::Incapable"
        // One of the units in the squad is telling us
        // it can't make it to the destination
//        LOG_DIAG(("Unit %d in the squad could not make it to its destination", data))
        return (TRUE);

      case 0xFCBF8881: // "Move::Completed"
      {
        // One of the units in the squad is telling us 
        // it made it to the destination
//        LOG_DIAG(("Unit %d in the squad made it to its destination", data))

        // Have all the remaining squad memebers made it ?
        Bool completed = TRUE;
        for (SquadObj::UnitList::Iterator i(&subject->GetList()); *i; i++)
        {
          if ((*i)->Alive())
          {
            if (
              (*i)->task == event.param1 &&
              (*i)->Id() == event.param2)
            {
              (*i)->completed = TRUE;
            }
            else
            if (!(*i)->completed)
            {
              completed = FALSE;
            }
          }
        }

        if (completed)
        {
//          LOG_DIAG(("Entire squad has made it to the destination!"))
          subject->NotifyPlayer(0x763C5781); // "Squad::TaskCompleted"
          Quit();
        }
        return (TRUE);
      }

      default:
        return (GameTask<SquadObjType, SquadObj>::ProcessEvent(event));
    }
  }


  //
  // Initial state
  //
  void SquadFormation::StateInit()
  {
    // Reset all of the squad members
    SquadObj::UnitList::Iterator i(&subject->GetList());
    for (!i; *i; i++)
    {
      (*i)->completed = FALSE;
    }

    Formation::Apply(formation, location, direction, (const UnitObjList &) subject->GetList(), GetFlags());
    NextState(0x9E947215); // "Moving"
  }


  //
  // Moving State
  //
  void SquadFormation::StateMoving()
  {
  }


  //
  // Initialization
  //
  void SquadFormation::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Build state machine
    stateMachine.AddState("Init", &StateInit);
    stateMachine.AddState("Moving", &StateMoving);
  }


  //
  // Shutdown
  //
  void SquadFormation::Done()
  {
    stateMachine.CleanUp();
  }

}
