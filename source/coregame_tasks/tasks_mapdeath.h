///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_MAPDEATH_H
#define __TASKS_MAPDEATH_H


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
  // Class MapDeath
  //
  class MapDeath : public GameTask<MapObjType, MapObj>
  {
    TASK_CLASS(MapDeath)

  public:

    // Constructor
    MapDeath(GameObj *subject);

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
    void StateDead();
    void StateKill();

  public:

    // Returns TRUE if 'obj' requires a death task
    static Bool RequiresDeathTask(MapObj *obj);

  };

}

#endif