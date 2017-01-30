///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 25-SEP-1998
//


#ifndef __TASKCTRL_H
#define __TASKCTRL_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace TaskCtrl
//
namespace TaskCtrl
{
  // Blocking priorities
  enum
  {
    BLOCK_AUTOMATION = 7,
    BLOCK_UNAVAILABLE = 8,
    BLOCK_INTERRUPT = 9,
    BLOCK_DEATH = 10,
    BLOCK_NODEATH = 11,
  };

  // Initialize and shutdown system
  void Init();
  void Done();

  // Create a new task
  Task * NewTask(GameObj *subject, const char *type, U32 id = 0);

  // Save a task
  void SaveTask(FScope *fScope, const char *name, Task &task);

  // Save a list of tasks
  void SaveTasks(FScope *fScope, const char *name, const NList<Task> &tasks);

  // Load a task
  Task * LoadTask(GameObj *subject, FScope *fScope);

  // Load a list of tasks
  void LoadTasks(GameObj *subject, FScope *fScope, NList<Task> &tasks);

  // PostLoad a task
  void PostLoad(Task &task);

  // PostLoad a list of tasks
  void PostLoad(NList<Task> &tasks);


  // Save system data
  void Save(FScope *scope);

  // Load system data
  void Load(FScope *scope);


  // Promote a task
  template <class TASK> TASK * Promote(Task *task)
  {
    return (task ? (task->GetNameCrc() == TASK::GetConfigNameCrc()) ? static_cast<TASK *>(task) : NULL : NULL);
  }

  // Promote an objects task
  template <class TASK> TASK * Promote(GameObj *obj)
  {
    ASSERT(obj);
    return (Promote<TASK>(obj->GetCurrentTask()));
  }

  // Promote an objects idle task
  template <class TASK> TASK * PromoteIdle(GameObj *obj)
  {
    ASSERT(obj);
    return (Promote<TASK>(obj->GetIdleTask()));
  }

}


#endif