///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_MAPIDLE_H
#define __TASKS_MAPIDLE_H


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
  // Class MapIdle
  //
  class MapIdle : public GameTask<MapObjType, MapObj>
  {
    TASK_CLASS(MapIdle)

  public:

    // Constructor
    MapIdle(GameObj *subject);

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
    void StateIdle();

  };

}

#endif