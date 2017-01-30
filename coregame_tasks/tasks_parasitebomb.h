///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_PARASITEBOMB_H
#define __TASKS_PARASITEBOMB_H


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
  // Class ParasiteBomb
  //
  class ParasiteBomb : public GameTask<ParasiteObjType, ParasiteObj>
  {
    TASK_CLASS(ParasiteBomb)

  private:

    // Time until destructorama
    GameTime::Timer timer;

  public:

    // Constructors
    ParasiteBomb(GameObj *subject);

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
    void StateFizzle();
    void StateDone();
  };
}

#endif