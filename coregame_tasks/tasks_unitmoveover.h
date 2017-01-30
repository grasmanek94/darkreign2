///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_UNITMOVEOVER_H
#define __TASKS_UNITMOVEOVER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"
#include "promote.h"


///////////////////////////////////////////////////////////////////////////////
// 
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitMoveOver - Told by a unit to get out of its way
  //
  class UnitMoveOver : public GameTask<UnitObjType, UnitObj>
  {
    TASK_CLASS(UnitMoveOver)

  private:

    // Direction to move in
    U8 direction;

    // Number of grains to move by
    U8 len;

    // Amount to time to wait after a successful completion
    U8 waitTime;

    // Timeout on this cycle
    U32 timeOut;

    // Mediator handle
    U32 mediateHandle;

    // Movement request handle
    Movement::Handle moveHandle;

  public:

    // Constructor
    UnitMoveOver(GameObj *subject);
    UnitMoveOver(GameObj *subject, U32 mediateHandle, U8 direction, U8 len, U32 timeOut, U8 waitTime);

    // Redirect the unit to another avoidance item
    void Redirect(U32 mediateHandleIn, U8 directionIn, U8 lenIn, U32 timeOut, U8 waitTime);

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