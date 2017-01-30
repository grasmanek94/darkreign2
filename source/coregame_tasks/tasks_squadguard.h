///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_SQUADGUARD_H
#define __TASKS_SQUADGUARD_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"
#include "promote.h"
#include "squadobj.h"
#include "target.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class SquadGuard
  //
  class SquadGuard : public GameTask<SquadObjType, SquadObj>
  {
    TASK_CLASS(SquadGuard)

  private:

    // Target which is being guarded
    Target target;

  public:

    // Constructor
    SquadGuard(GameObj *subject);
    SquadGuard(GameObj *subject, const Target &target);
    ~SquadGuard();

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
    void StateGuarding();

  };
}

#endif