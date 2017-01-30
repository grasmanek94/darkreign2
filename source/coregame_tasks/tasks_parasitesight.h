///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_PARASITESIGHT_H
#define __TASKS_PARASITESIGHT_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"
#include "promote.h"
#include "parasiteobj.h"


///////////////////////////////////////////////////////////////////////////////
// 
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class ParasiteSight
  //
  class ParasiteSight : public GameTask<ParasiteObjType, ParasiteObj>
  {
    TASK_CLASS(ParasiteSight)

  private:

    // State timer
    GameTime::Timer timer;

  public:

    // Constructors
    ParasiteSight(GameObj *subject);

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
    void StateStage1();
    void StateStage2();
    void StateDone();
  };
}

#endif