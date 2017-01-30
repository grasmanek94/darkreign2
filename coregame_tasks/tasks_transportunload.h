///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_TransportUnload_H
#define __TASKS_TransportUnload_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"
#include "promote.h"
#include "transportobj.h"


///////////////////////////////////////////////////////////////////////////////
// 
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class TransportUnload
  //
  class TransportUnload : public GameTask<TransportObjType, TransportObj>
  {
    TASK_CLASS(TransportUnload)

  private:

    // The location to unload cargo
    Vector destination;

    // Movement request handle
    Movement::Handle moveHandle;

  public:

    // Constructors
    TransportUnload(GameObj *subject);
    TransportUnload(GameObj *subject, const Vector &destination);

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