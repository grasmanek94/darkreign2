///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_SQUADBOARD_H
#define __TASKS_SQUADBOARD_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"
#include "promote.h"
#include "squadobj.h"
#include "transportobj.h"


///////////////////////////////////////////////////////////////////////////////
// 
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class SquadBoard
  //
  class SquadBoard : public GameTask<SquadObjType, SquadObj>
  {
    TASK_CLASS(SquadBoard)

  private:

    // The transports being boarded
    TransportObjList transports;

  public:

    // Constructors
    SquadBoard(GameObj *subject);
    SquadBoard(GameObj *subject, const TransportObjList &transport);

    // Destructor
    ~SquadBoard();

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
    void StateBoarding();
  };
}

#endif