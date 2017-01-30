///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_UNITCLOAK_H
#define __TASKS_UNITCLOAK_H


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
  // Class UnitCloak
  //
  class UnitCloak : public GameTask<UnitObjType, UnitObj>
  {
    TASK_CLASS(UnitCloak)

    // The cloaking progress
    F32 progressTotal;

    // The progress rate per cycle
    F32 progressRate;


    // Check if we should turn cloaking on/off
    Bool CheckCloak(Bool on);

    // Remove cloak if active
    void RemoveCloak();

    // Check to see if we need to change cloaked status
    Bool CheckStatus();

  public:

    // Constructor
    UnitCloak(GameObj *subject);

    // Task processing (returns TRUE if task completed)
    Bool Process();

    // Load and save state configuration
    void Load(FScope *fScope);
    void Save(FScope *fScope);

    // Called after all objects are loaded
    void PostLoad();

    // Process an event
    Bool ProcessEvent(const Event &event);

    // Is the unit currently cloaked
    Bool IsCloaked();

    // Get the current effect progress
    F32 GetProgress();

  private:

    // State machine procedures
    void StateInit();
    void StateVisible();
    void StateCloaked(StateMachineNotify notify);

    // FX Callback
    static Bool FXCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *context);
  };

}

#endif