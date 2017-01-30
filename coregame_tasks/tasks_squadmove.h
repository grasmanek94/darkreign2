///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_SQUADMOVE_H
#define __TASKS_SQUADMOVE_H


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
  // Class SquadMove
  //
  class SquadMove : public GameTask<SquadObjType, SquadObj>
  {
    TASK_CLASS(SquadMove)

  private:

    struct NPoint : public Point<U32>
    {
      // List node
      NList<NPoint>::Node node;

      // Direction to the next slot
      F32 direction;
      
      NPoint(U32 x, U32 z)
      : Point<U32>(x, z), direction(0) 
      { 
      }

      NPoint(const Point<U32> &p)
      : Point<U32>(p), direction(0)
      { 
      }

    };

    // The destination to reach
    Vector destination;

    // Direction to face
    F32 direction;

    // List of meeting points
    NList<NPoint> points;

    // Current point of consideration
    S32 point;

    // The trail to use
    TrailObjPtr trail;

    // Index to start on the trail
    U32 index;

    // This task can actually be "spawned" by another task
    Task *task;

  public:

    // Constructor
    SquadMove(GameObj *subject);
    SquadMove(GameObj *subject, const Vector &destination);
    SquadMove(GameObj *subject, Task *task, const Vector &destination);
    SquadMove(GameObj *subject, TrailObj *trail, U32 index);
    ~SquadMove();

    // Task processing (returns TRUE if task completed)
    Bool Process();

    // Load and save state configuration
    void Load(FScope *fScope);
    void Save(FScope *fScope);

    // Called after all objects are loaded
    void PostLoad();

    // Process an event
    Bool ProcessEvent(const Event &event);

    // Set the spawning task
    void SetTask(Task *t)
    {
      task = t;
    }

  private:

    // State machine procedures
    void StateInit();
//  void StatePathing();
//  void StatePathAnalysis();
    void StateMoving();

//  void GoToNextPoint(SquadObj::ListNode *node);

  };
}

#endif