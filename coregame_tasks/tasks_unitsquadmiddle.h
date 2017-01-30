///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Constructor Task
//
// 9-JUN-1999
//


#ifndef __TASKS_UNITSQUADMIDDLE_H
#define __TASKS_UNITSQUADMIDDLE_H


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
  // Class UnitSquadMiddle
  //
  class UnitSquadMiddle : public GameTask<UnitObjType, UnitObj>
  {
    TASK_CLASS(UnitSquadMiddle)

  private:

    // Movement request handle
    Movement::Handle moveHandle;

  public:

    // Constructor
    UnitSquadMiddle(GameObj *subject);

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
    void StateWaiting();

  };

}

#endif