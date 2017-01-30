///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Constructor Task
//
// 9-JUN-1999
//


#ifndef __TASKS_UNITEXPLORE_H
#define __TASKS_UNITEXPLORE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"
#include "promote.h"


///////////////////////////////////////////////////////////////////////////////
// 
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitExplore
  //
  class UnitExplore : public GameTask<UnitObjType, UnitObj>
  {
    TASK_CLASS(UnitExplore)

    // Movement request handle
    Movement::Handle moveHandle;

    // wait time
    U32 hangUntil;

  public:

    // Constructor
    UnitExplore(GameObj *subject);

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
    void StateMoving();
    void StateLoitering(StateMachineNotify notify);
  };

}

#endif