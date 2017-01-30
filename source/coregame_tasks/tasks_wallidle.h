///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
//
// 14-NOV-1999
//


#ifndef __TASKS_WALLIDLE_H
#define __TASKS_WALLIDLE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"
#include "promote.h"
#include "wallobj.h"


///////////////////////////////////////////////////////////////////////////////
// 
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class WallIdle
  //
  class WallIdle : public GameTask<WallObjType, WallObj>
  {
    TASK_CLASS(WallIdle)

  protected:

    // The list of walls to link to
    WallObjList linkTargets;

  public:

    // Constructor and destructor
    WallIdle(GameObj *subject);
    ~WallIdle();

    // Task processing (returns TRUE if task completed)
    Bool Process();

    // Load and save state configuration
    void Load(FScope *fScope);
    void Save(FScope *fScope);

    // Called after all objects are loaded
    void PostLoad();

    // Process an event
    Bool ProcessEvent(const Event &event);

  protected:

    // State machine procedures
    void StateInit();
    void StateLink();
    void StateIdle();

  public:

    // Set targets and enter init state
    void SetLinkTargets(const WallObjList &list);
  };
}

#endif