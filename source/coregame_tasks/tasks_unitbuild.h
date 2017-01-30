///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_UNITBUILD_H
#define __TASKS_UNITBUILD_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"
#include "promote.h"
#include "footprint.h"
#include "claim.h"


///////////////////////////////////////////////////////////////////////////////
// 
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitBuild
  //
  class UnitBuild : public GameTask<UnitObjType, UnitObj>
  {
    TASK_CLASS(UnitBuild)

  private:

    // Object type to build
    UnitObjTypePtr build;

    // Metre position to construct object
    Matrix location;

    // Have we been through initialization
    Bool initialized;

    // Destination for rig
    Vector destination;

    // Construction time out
    GameTime::Timer timer;

    // Footprint placer
    FootPrint::Placement place;

    // Footprint type
    FootPrint::Type *foot;

    // Claiming data
    Claim::Manager claimInfo;

    // Have we claimed the location
    Bool claimed;

    // Movement request handle
    Movement::Handle moveHandle;

    // Reaper to the object under construction
    UnitObjPtr target;

    // Should the task currently block other tasks
    Bool blocking;

    // Initialize the footprint data
    void InitializeFootData();

  public:

    // Constructors and destructor
    UnitBuild(GameObj *subject);
    UnitBuild(GameObj *subject, UnitObjType *build, const Vector &dest, WorldCtrl::CompassDir dir);
    ~UnitBuild();

    // Task processing (returns TRUE if task completed)
    Bool Process();

    // Load and save state configuration
    void Load(FScope *fScope);
    void Save(FScope *fScope);

    // Called after all objects are loaded
    void PostLoad();

    // Process an event
    Bool ProcessEvent(const Event &event);

    // Get the dynamic blocking priority of this task
    U32 GetBlockingPriority();

    // Abort construction cleanly
    void Abort();

  private:

    // State machine procedures
    void StateInit();
    void StateMoving();
    void StateLimited();
    void StateArrived(StateMachineNotify notify);
    void StateThumping(StateMachineNotify notify);
    void StatePreProcess(StateMachineNotify notify);
    void StateBuild(StateMachineNotify notify);
    void StateConsume(StateMachineNotify notify);
    void StateFinished();
    void StatePostProcess();
    void StateBadLocation();

    // Evacuate the placement area
    UnitEvacuate::Result Evacuate();

    // Can we still construct
    Bool CheckConstruction(Bool thumpTest, FootPrint::Placement::Result &placeResult);

    // Are we in the correct location to construct
    Bool CheckLocation();

    // FX Callback
    static Bool FXCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *context);
  };
}

#endif