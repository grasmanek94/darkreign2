///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_UNITPOWERDOWN_H
#define __TASKS_UNITPOWERDOWN_H


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
  // Class UnitPowerDown
  //
  class UnitPowerDown : public GameTask<UnitObjType, UnitObj>
  {
    TASK_CLASS(UnitPowerDown)

  private:

    // The current progress total
    F32 progressTotal;

    // The progress per cycle at max efficiency
    F32 progressMax;

  public:

    // Constructor
    UnitPowerDown(GameObj *subject);

    // Task processing (returns TRUE if task completed)
    Bool Process();

    // Load and save state configuration
    void Load(FScope *fScope);
    void Save(FScope *fScope);

    // Called after all objects are loaded
    void PostLoad();

    // Process an event
    Bool ProcessEvent(const Event &event);

    // Retrieve data from the task, return TRUE if filled
    Bool Retrieve(U32 id, RetrievedData &data);

  private:

    // State machine procedures
    void StateInit();
    void StatePowerDown(StateMachineNotify notify);
    void StatePoweredDown(StateMachineNotify notify);
    void StatePowerUp(StateMachineNotify notify);

    // FX Callback
    static Bool FXCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *context);
  };

}

#endif