///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_SQUADFOLLOWTAG_H
#define __TASKS_SQUADFOLLOWTAG_H


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
  // Class SquadFollowTag
  //
  class SquadFollowTag : public GameTask<SquadObjType, SquadObj>
  {
    TASK_CLASS(SquadFollowTag)

  private:

    // Move task used to move the troops around
    SquadMove *taskMove;

    // Tag being followed
    TagObjPtr tag;

    // The current destination of the squad
    Vector destination;

  public:

    // Constructor
    SquadFollowTag(GameObj *subject);
    SquadFollowTag(GameObj *subject, TagObj *tag);
    ~SquadFollowTag();

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

  };
}

#endif