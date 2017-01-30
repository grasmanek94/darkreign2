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
#include "tasks_squadboard.h"
#include "tasks_unitboard.h"
#include "tasks_unitmove.h"
#include "tasks_transportpad.h"
#include "taskctrl.h"
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
  StateMachine<SquadBoard> SquadBoard::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class SquadBoard
  //


  //
  // Constructor
  //
  SquadBoard::SquadBoard(GameObj *subject) 
  : GameTask<SquadObjType, SquadObj>(staticConfig, subject),
    inst(&stateMachine, "Init")
  {
  }


  //
  // Constructor
  //
  SquadBoard::SquadBoard(GameObj *subject, const TransportObjList &transportsIn)
  : GameTask<SquadObjType, SquadObj>(staticConfig, subject),
    inst(&stateMachine, "Init")
  {
    transports.Dup(transportsIn);
  }


  //
  // Destructor
  //
  SquadBoard::~SquadBoard()
  {
    transports.Clear();
  }


  //
  // Save
  //
  void SquadBoard::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));
    StdSave::TypeReaperList(fScope, "Transports", transports);   
  }


  //
  // Load
  //
  void SquadBoard::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
          break;

        case 0xC2244521: // "Transports"
          StdLoad::TypeReaperList(sScope, transports);
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
  void SquadBoard::PostLoad()
  {
    Resolver::ObjList<TransportObj, TransportObjType, TransportObjListNode>(transports);
  }


  //
  // Perform task processing
  //
  Bool SquadBoard::Process()
  {
    inst.Process(this);  
    return (quit);
  }


  //
  // StateInit
  //
  void SquadBoard::StateInit()
  {
    // Get each of the units in the squad to board transports
    SquadObj::UnitList::Iterator u(&subject->GetList());

    // We could enhance this to use units which are closest to transports etc.

    // Clear the squad's completed flags
    for (!u; *u; u++)
    {
      (*u)->completed = FALSE;
      (*u)->task = 0;
    }

    // Reset squad iterator
    !u;

    for (TransportObjList::Iterator t(&transports); *t; t++)
    {
      if ((*t)->Alive())
      {
        TransportObj *transport = **t;
        U32 slots = transport->TransportType()->GetSpaces();
        U32 added = 0;

        while (slots--)
        {
          UnitObjPtr *unitPtr = *u;

          if (unitPtr)
          {
            ASSERT(unitPtr->Alive())
            UnitObj *unit = *unitPtr;

            if (unit->CanEverMove())
            {
              // Tell the unit to board
              unit->FlushTasks(Tasks::UnitBoard::GetConfigBlockingPriority());

              Task *task;

              // Is this a telepad
              if (TaskCtrl::PromoteIdle<Tasks::TransportPad>(transport))
              {
                unit->PrependTask(task = new Tasks::UnitMove(unit, transport), GetFlags());
              }
              else
              {
                unit->PrependTask(task = new Tasks::UnitBoard(unit, transport), GetFlags());
              }

              (*u)->task = task->GetTaskId();

              added++;
            }
            else
            {
              // This unit can't move so complete it
              (*u)->completed = TRUE;
            }
          }
          else
          {
            break;
          }

          // Increment iterator
          u++;
        }

        if (!added)
        {
          // No units were added
          break;
        }
      }
    }

    // Complete those units which missed out
    for (; *u; u++)
    {
      (*u)->completed = TRUE;
    }

    NextState(0x09E5F977); // "Boarding"
  }


  //
  // StateBoarding
  //
  void SquadBoard::StateBoarding()
  {
    Bool completed = TRUE;

    SquadObj::UnitList::Iterator i(&subject->GetList());
    for (!i; *i; i++)
    {
      if ((*i)->Alive() && !(*i)->completed)
      {
        completed = FALSE;
        break;
      }
    }

    if (completed)
    {
      // Notify the player that we're done
      subject->NotifyPlayer(0x763C5781); // "Squad::TaskCompleted"

      Quit();
    }
  }


  //
  // Notify
  //
  // Notify this task of an event
  //
  Bool SquadBoard::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
        NextState(0xABAA7B48); // "Init"
        return (TRUE);

      case 0x6B0DB5AA: // "Move::Incapable"
      {
        // One of the units in the squad is telling us
        // it can't make it to the destination
//        LOG_DIAG(("Unit %d in the squad could not make it to its destination", data))

        for (SquadObj::UnitList::Iterator i(&subject->GetList()); *i; i++)
        {
          if (
            (*i)->Alive() && 
            (*i)->task == event.param1 &&
            (*i)->Id() == event.param2)
          {
            (*i)->completed = TRUE;
//            GoToNextPoint(*i);
          }
        }
        return (TRUE);
      }

      case 0x7D63DD2C: // "Board::Completed"
      case 0xFCBF8881: // "Move::Completed"
      {
        // One of the units in the squad is telling us 
        // it made it to the destination
        //LOG_DIAG(("Unit %d in the squad boarded", event.param2))

        // Mark the completed flag for this squad member
        for (SquadObj::UnitList::Iterator i(&subject->GetList()); *i; i++)
        {
          if (
            (*i)->Alive() && 
            (*i)->task == event.param1 &&
            (*i)->Id() == event.param2)
          {
            (*i)->completed = TRUE;
            subject->ThinkFast();
            break;
          }
        }
        return (TRUE);
      }
    }
    return (GameTask<SquadObjType, SquadObj>::ProcessEvent(event));
  }


  //
  // Initialization
  //
  void SquadBoard::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Build state machine
    stateMachine.AddState("Init", &StateInit);
    stateMachine.AddState("Boarding", &StateBoarding);
  }


  //
  // Shutdown
  //
  void SquadBoard::Done()
  {
    stateMachine.CleanUp();
  }

}
