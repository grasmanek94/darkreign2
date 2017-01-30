///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_UNITMOVE_H
#define __TASKS_UNITMOVE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"
#include "promote.h"
#include "trailobj.h"
#include "movement.h"


///////////////////////////////////////////////////////////////////////////////
// 
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitMove
  //
  class UnitMove : public GameTask<UnitObjType, UnitObj>
  {
    TASK_CLASS(UnitMove)

  private:

    // Do we have a destination
    U32 hasDest : 1,

    // Do we have a direction
        hasDir : 1,

    // Do we have a custom request?
        hasCustomReq : 1;

    // The destination to reach
    Vector destination;  

    // The destination to reach
    Vector direction;

    // The trail to use
    TrailObjPtr trail;

    // The trail index to start at
    U32 trailIndex;

    // The trail follower
    TrailObj::Follower follower;

    // An object to move onto
    UnitObjPtr moveOn;

    // Request data
    Movement::RequestData requestData;

    // Movement request handle
    Movement::Handle moveHandle;

  public:

    // Constructor
    UnitMove(GameObj *subject);
    UnitMove(GameObj *subject, const Vector &destination);
    UnitMove(GameObj *subject, const Vector &destination, const Vector &direction);
    UnitMove(GameObj *subject, TrailObj *trail, U32 index);
    UnitMove(GameObj *subject, UnitObj *moveOn);

    // Setup request data
    void SetupRequest(const Movement::RequestData &rd);

    // Get request data
    Movement::RequestData *GetRequest();

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

    // Move to the next point in the trail
    Bool NextTrailPoint();

    // State machine procedures
    void StateInit();
    void StateMoving();

  public:

    // Set the direction
    void SetDir(Vector &dir)
    {
      hasDir = TRUE;
      direction = dir;
      inst.Set(0xABAA7B48); // "Init"
    }

  };

}

#endif