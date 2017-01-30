///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_MAPRECYCLE_H
#define __TASKS_MAPRECYCLE_H


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
  // Class UnitRecycle
  //
  class UnitRecycle : public GameTask<UnitObjType, UnitObj>
  {
    TASK_CLASS(UnitRecycle)

    // Amount of resource to refund
    S32 refund;

    // How many seconds would it take to refund at 100% completion
    F32 timeTotal;

    // How many seconds will it take to recycle at current completion
    F32 timeCurrent;

    // The current progress total
    F32 progressTotal;

    // The progress per cycle at max efficiency
    F32 progressMax;

    // Can this recycle be cancelled
    Bool cancel;

  public:

    // Constructor
    UnitRecycle(GameObj *subject);
    UnitRecycle(GameObj *subject, S32 refund, F32 timeTotal, F32 timeCurrent, Bool cancel);

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

    // Get the progress percentage
    F32 GetProgress();

  private:

    // State machine procedures
    void StateInit();
    void StateRecycle();
    void StateDecycle();

    // FX Callback
    static Bool FXCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *context);
  };
}

#endif