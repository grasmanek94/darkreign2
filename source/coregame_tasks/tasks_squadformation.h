///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_SQUADFORMATION_H
#define __TASKS_SQUADFORMATION_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"
#include "promote.h"
#include "squadobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class SquadFormation
  //
  class SquadFormation : public GameTask<SquadObjType, SquadObj>
  {
    TASK_CLASS(SquadFormation)

  private:

    // Name of the formation to form
    U32 formation;

    // Location to form the formation
    Vector location;

    // Direction to face the formation
    F32 direction;

  public:

    // Constructor
    SquadFormation(GameObj *subject);
    SquadFormation(GameObj *subject, U32 formation, const Vector &location, F32 direction);
    ~SquadFormation();

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