///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_RESTORESTATIC_H
#define __TASKS_RESTORESTATIC_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"
#include "promote.h"
#include "restoreobj.h"


///////////////////////////////////////////////////////////////////////////////
// 
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class RestoreStatic
  //
  class RestoreStatic : public GameTask<RestoreObjType, RestoreObj>
  {
    TASK_CLASS(RestoreStatic)

  protected:

    // List of all static restorers
    static NList<RestoreStatic> list;

    // The list node
    NList<RestoreStatic>::Node node;

    // The optional rally point
    Bool rallyPointActive;
    Point<U32> rallyPoint;

  public:

    // Find the closest static restore object
    static RestoreObj * Find(UnitObj *target, Bool acceptFirst = FALSE);

    // Constructor and destructor
    RestoreStatic(GameObj *subject);
    ~RestoreStatic();

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

    // Do we have a valid target that needs restoration
    Bool ValidTarget();

  private:

    // State machine procedures
    void StateIdle();
    void StateRestoring(StateMachineNotify notify);
    void StateBuggerOff();
  };

}

#endif