///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_UNITANIMATE_H
#define __TASKS_UNITANIMATE_H


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
  // Class UnitAnimate
  //
  class UnitAnimate : public GameTask<UnitObjType, UnitObj>
  {
    TASK_CLASS(UnitAnimate)

  protected:

    // Animation cycle
    U32 cycleId;

  public:

    // Constructor
    UnitAnimate(GameObj *subject);
    UnitAnimate(GameObj *subject, U32 cycle);

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
    void StateAnimate(StateMachineNotify);

  };

}

#endif