///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_UNITUPGRADE_H
#define __TASKS_UNITUPGRADE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"
#include "promote.h"
#include "tasks_unitconstruct.h"


///////////////////////////////////////////////////////////////////////////////
// 
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitUpgrade
  //
  class UnitUpgrade : public GameTask<UnitObjType, UnitObj>
  {
    TASK_CLASS(UnitUpgrade)

    // The upgrade object
    UnitObjPtr upgrade;

  public:

    // Constructor
    UnitUpgrade(GameObj *subject);
    UnitUpgrade(GameObj *subject, UnitObj *upgrade);

    // Task processing (returns TRUE if task completed)
    Bool Process();

    // Load and save state configuration
    void Load(FScope *fScope);
    void Save(FScope *fScope);

    // Called after all objects are loaded
    void PostLoad();

    // Returns the percent complete
    F32 GetProgress();

    // Process an event
    Bool ProcessEvent(const Event &event);

    // Retrieve data from the task, return TRUE if filled
    Bool Retrieve(U32 id, RetrievedData &data);
  
  private:

    // Get the construct task of the upgrade, or NULL if finished
    Tasks::UnitConstruct * GetConstructTask();

    // State machine procedures
    void StateUpgrade();
    void StatePaused(StateMachineNotify notify);
  };

}

#endif