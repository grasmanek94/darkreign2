///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_UNITGORESTORE_H
#define __TASKS_UNITGORESTORE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"
#include "promote.h"
#include "movement.h"


///////////////////////////////////////////////////////////////////////////////
// 
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitGoRestore
  //
  class UnitGoRestore : public GameTask<UnitObjType, UnitObj>
  {
    TASK_CLASS(UnitGoRestore)

  private:

    // The destination facility
    RestoreObjPtr facility;

    // Movement request handle
    Movement::Handle moveHandle;

  public:

    // Constructor
    UnitGoRestore(GameObj *subject);

    // Task processing (returns TRUE if task completed)
    Bool Process();

    // Load and save state configuration
    void Load(FScope *fScope);
    void Save(FScope *fScope);

    // Called after all objects are loaded
    void PostLoad();

    // Process an event
    Bool ProcessEvent(const Event &event);

    // Change the restore facility
    void ChangeFacility(RestoreObj *restore);

  private:

    // State machine procedures
    void StateInit();
    void StateMoving();

  };

}

#endif