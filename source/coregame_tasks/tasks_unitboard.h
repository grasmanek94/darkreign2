///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_UNITBOARD_H
#define __TASKS_UNITBOARD_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"
#include "promote.h"
#include "transportobj.h"


///////////////////////////////////////////////////////////////////////////////
// 
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitBoard
  //
  class UnitBoard : public GameTask<UnitObjType, UnitObj>
  {
    TASK_CLASS(UnitBoard)

  private:

    // The transport being boarded
    TransportObjPtr target;

    // The location of the target when we started moving
    Vector destination;

    // Movement handle
    Movement::Handle moveHandle;


    // Do we have a valid transport target
    Bool ValidTarget();

    // Returns either the destination or current position of the target
    Vector GetTargetVector();

    // Has the target moved significantly from it's original location
    Bool TargetMoved();

    // Is the target within boarding range
    Bool TargetInRange();

  public:

    // Constructors
    UnitBoard(GameObj *subject);
    UnitBoard(GameObj *subject, TransportObj *target);

    // Task processing (returns TRUE if task completed)
    Bool Process();

    // Load and save state configuration
    void Load(FScope *fScope);
    void Save(FScope *fScope);

    // Called after all objects are loaded
    void PostLoad();

    // Process an event
    Bool ProcessEvent(const Event &event);

  private:

    // State machine procedures
    void StateInit();
    void StateSeekEntry();
    void StateDone();
  };
}

#endif