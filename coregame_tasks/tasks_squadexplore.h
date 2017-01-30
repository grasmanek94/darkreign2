///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_SQUADEXPLORE_H
#define __TASKS_SQUADEXPLORE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"
#include "promote.h"
#include "squadobj.h"
#include "tasks_squadmove.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class SquadExplore
  //
  class SquadExplore : public GameTask<SquadObjType, SquadObj>
  {
    TASK_CLASS(SquadExplore)

  private:

    // Move task used to move the troops around
    SquadMove *taskMove;

  public:

    // Constructor
    SquadExplore(GameObj *subject);
    ~SquadExplore();

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
    void StateMoving();

  };
}

#endif