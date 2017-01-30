///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_SQUADATTACK_H
#define __TASKS_SQUADATTACK_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"
#include "promote.h"
#include "squadobj.h"
#include "target.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class SquadAttack
  //
  class SquadAttack : public GameTask<SquadObjType, SquadObj>
  {
    TASK_CLASS(SquadAttack)

  private:

    // The target to attack
    Target target;

  public:

    // Constructor
    SquadAttack(GameObj *subject);
    SquadAttack(GameObj *subject, const Target &target);
    ~SquadAttack();

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
    void StateAttack();

  };
}

#endif