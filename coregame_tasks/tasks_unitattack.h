///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_UNITATTACK_H
#define __TASKS_UNITATTACK_H


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
  // Class UnitAttack
  //
  class UnitAttack : public GameTask<UnitObjType, UnitObj>
  {
    TASK_CLASS(UnitAttack)

  private:

    // The target
    Target target;

  public:

    // Constructor
    UnitAttack(GameObj *subject);
    UnitAttack(GameObj *subject, const Target &target);

    ~UnitAttack();

    // Task processing (returns TRUE if task completed)
    Bool Process();

    // Load and save state configuration
    void Load(FScope *fScope);
    void Save(FScope *fScope);

    // Called after all objects are loaded
    void PostLoad();

    // Process an event
    Bool ProcessEvent(const Event &event);

    // Get the blocking priority of this task
    U32 GetBlockingPriority();

    // Set a new target
    void SetTarget(const Target &target);

  private:

    // State machine procedures
    void StateInit();
    void StateAttacking();

  };

}


#endif