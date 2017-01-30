///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_RESTOREMOBILE_H
#define __TASKS_RESTOREMOBILE_H


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
  // Class RestoreMobile
  //
  class RestoreMobile : public GameTask<RestoreObjType, RestoreObj>
  {
    TASK_CLASS(RestoreMobile)

  private:

    // The unit that is to be restored
    UnitObjPtr target;

    // The location of the target when we started moving
    Vector destination;

    // Movement handle
    Movement::Handle moveHandle;

    // Do we have a valid target that needs restoration
    Bool ValidTarget();

    // Returns the restoration destination position for the current target
    Vector GetRestorationDestination();

    // Has the target moved significantly from it's original location
    Bool TargetMoved();

    // Is the target withing restoration range
    Bool TargetInRange(Bool thinkFast = FALSE);

    // Should we move to reach target
    Bool MoveIntoRange();

  public:

    // Constructors
    RestoreMobile(GameObj *subject);
    RestoreMobile(GameObj *subject, UnitObj *target);

    // Task processing (returns TRUE if task completed)
    Bool Process();

    // Load and save state configuration
    void Load(FScope *fScope);
    void Save(FScope *fScope);

    // Called after all objects are loaded
    void PostLoad();

    // Process an event
    Bool ProcessEvent(const Event &event);

  public:

    // Get the target
    UnitObj * GetTarget()
    {
      return (target.GetPointer());
    }

  private:

    // State machine procedures
    void StateInit();
    void StateMoving();
    void StateWaiting();
    void StatePreProcess(StateMachineNotify notify);
    void StateRestoring(StateMachineNotify notify);
    void StatePostProcess(StateMachineNotify notify);
  };
}

#endif