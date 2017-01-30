///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
//
// 14-NOV-1999
//


#ifndef __TASKS_SPYIDLE_H
#define __TASKS_SPYIDLE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"
#include "promote.h"
#include "spyobj.h"


///////////////////////////////////////////////////////////////////////////////
// 
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class SpyIdle
  //
  class SpyIdle : public GameTask<SpyObjType, SpyObj>
  {
    TASK_CLASS(SpyIdle)

  private:
    friend class SpyObj;

    // Target of our current operation
    UnitObjPtr target;

    // Team that LOS is being stolen from/given to
    Team *sightSrc;
    Team *sightDst;

    // Team that power is being stolen from
    Team *powerSrc;

    // Movement handle
    Movement::Handle moveHandle;

    // Things we are stealing
    U32 stealLOS : 1,
        stealPower : 1,
        stealResource : 1,

    // Has the spy set its spying bit in its target
        hasSetSpying : 1,

    // Are we currently morphed
        isMorphed : 1;

    // Last processing cycle
    U32 lastCycle;

    // Position that spy will be expelled to
    Vector expelPos;

    // Mesh before and after morhping
    MeshEnt *oldMesh, *newMesh;


  public:

    // Constructor
    SpyIdle(GameObj *subject);

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

    // Check if our morph has been detected
    void CheckMorphDetected();

    // Restore old mesh
    void RestoreMesh();

    // Set spying bit in target object
    void SetSpyingBit(Bool state);

    // Ejected
    void Ejected();

    // State machine procedures
    void StateInit();
    void StateInfiltrate(StateMachineNotify);
    void StateMorph();
    void StateSteal(StateMachineNotify);
    void StateBusted(StateMachineNotify);
    void StateInfiltrateBusy();

  public:

    // Infiltrate a unit
    void Infiltrate(UnitObj *target);

    // Eject the spy from its building
    void Eject();

    // Morph into a unit
    void Morph(UnitObj *target);

    // Morph has been detected
    void MorphDetected();

    // Has the spy set its spying bit in the target unit
    Bool TestSetSpying();

    // Is spy infiltrating the specified unit?
    Bool IsInfiltrating(UnitObj *unit);

    // Is the spy morphed
    Bool IsMorphed();

    // Get the dynamic blocking priority of this task
    U32 GetBlockingPriority();
  };
}

#endif