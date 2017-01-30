///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Constructor Task
//
// 9-JUN-1999
//


#ifndef __TASKS_UNITGUARD_H
#define __TASKS_UNITGUARD_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"
#include "promote.h"
#include "target.h"


///////////////////////////////////////////////////////////////////////////////
// 
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitGuard
  //
  class UnitGuard : public GameTask<UnitObjType, UnitObj>
  {
    TASK_CLASS(UnitGuard)

  private:

    // Target which is being guarded
    Target guardTarget;

    // Target which we are attacking
    Target attackTarget;

    // Movement request handle
    Movement::Handle moveHandle;

  public:

    // Constructor
    UnitGuard(GameObj *subject);
    UnitGuard(GameObj *subject, const Target &guardTarget);

    // Task processing (returns TRUE if task completed)
    Bool Process();

    // Load and save state configuration
    void Load(FScope *fScope);
    void Save(FScope *fScope);

    // Called after all objects are loaded
    void PostLoad();

    // Process an event
    Bool ProcessEvent(const Event &event);

    // Set the current attack target
    void SetAttackTarget(const Target &target);

  public:

    // Can the given unit use the guard task
    static Bool CanGuard(UnitObj *unit);

  public:

    // Get the target
    const Target & GetGuardTarget()
    {
      return (guardTarget);
    }

    // Get the attack target
    const Target & GetAttackTarget()
    {
      return (attackTarget);
    }

  private:

    // State machine procedures
    void StateInit();
    void StateGuarding();
    void StateMoving();

  };

}

#endif