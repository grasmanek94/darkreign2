///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_UNITFIRE_H
#define __TASKS_UNITFIRE_H


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
  // Class UnitFire
  //
  class UnitFire : public GameTask<UnitObjType, UnitObj>
  {
    TASK_CLASS(UnitFire)

  private:

    Target target;
    F32 horizontal;
    F32 vertical;
    F32 speed;

  public:

    // Constructor
    UnitFire(GameObj *subject, const Target &target, F32 horizontal, F32 vertical, F32 speed);
    UnitFire(GameObj *subject);
    ~UnitFire();

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
    void StateFiring();

  };

}


#endif