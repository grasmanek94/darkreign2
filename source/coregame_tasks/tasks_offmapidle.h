///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_OFFMAPIDLE_H
#define __TASKS_OFFMAPIDLE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"
#include "promote.h"
#include "offmapobj.h"


///////////////////////////////////////////////////////////////////////////////
// 
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class OffMapIdle
  //
  class OffMapIdle : public GameTask<OffMapObjType, OffMapObj>
  {
    TASK_CLASS(OffMapIdle)

  public:

    // Constructor
    OffMapIdle(GameObj *subject);

    // Task processing (returns TRUE if task completed)
    Bool Process();

    // Load and save state configuration
    void Load(FScope *fScope);
    void Save(FScope *fScope);

    // Called after all objects are loaded
    void PostLoad();

  private:

    // State machine procedures
    void StateIdle();
  };

}

#endif