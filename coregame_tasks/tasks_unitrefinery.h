///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_UNITREFINERY_H
#define __TASKS_UNITREFINERY_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"
#include "promote.h"
#include "unitobj.h"


///////////////////////////////////////////////////////////////////////////////
// 
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitRefinery
  //
  class UnitRefinery : public GameTask<UnitObjType, UnitObj>
  {
    TASK_CLASS(UnitRefinery)

    // Pointer to the unit currently being processed
    UnitObjPtr target;

  public:

    // Constructor
    UnitRefinery(GameObj *subject);

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
    void StateIdle();

    // FX Callback
    static Bool FXCallBack(MapObj *mapObj, FX::CallBackData &, void *);
  };

}

#endif