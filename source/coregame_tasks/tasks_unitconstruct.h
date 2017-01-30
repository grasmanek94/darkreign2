///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
//
// 24-MAR-1999
//


#ifndef __TASKS_UNITCONSTRUCT_H
#define __TASKS_UNITCONSTRUCT_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"
#include "promote.h"
#include "gametime.h"


///////////////////////////////////////////////////////////////////////////////
// 
// NameSpace Tasks
//
namespace Tasks
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitConstruct
  //
  class UnitConstruct : public GameTask<UnitObjType, UnitObj>
  {
    TASK_CLASS(UnitConstruct)

  protected:

    // The current progress total
    F32 progressTotal;

    // The progress per cycle at max efficiency
    F32 progressMax;

    // Amount of resource removed
    S32 resourceRemoved;

    // Amount of resource left to remove
    S32 resourceRemaining;

    // The number of hitpoints left to add
    S32 hitPointsLeft;

    // The rig that is constructing
    UnitObjPtr constructor;

    // Team that this object was registered on for construction
    Team *constructListTeam;

    // The construction effect
    FX::Object * effect;

  public:

    // Constructor
    UnitConstruct(GameObj *subject);

    // Task processing (returns TRUE if task completed)
    Bool Process();

    // Load and save state configuration
    void Load(FScope *fScope);
    void Save(FScope *fScope);

    // Called after all objects are loaded
    void PostLoad();

    // Process an event
    Bool ProcessEvent(const Event &event);

    // Retrieve data from the task, return TRUE if filled
    Bool Retrieve(U32 id, RetrievedData &data);

    // Get the progress percentage
    F32 GetProgress();

    // Returns the exact amount of resource removed
    S32 GetResourceRemoved();

    // Setup an object prior to the assignment of a construction task
    static void Setup(UnitObj *unit);

  private:

    // Is the effect required
    Bool EffectRequired();

    // Create the construction effect
    void CreateEffect();

    // State machine procedures
    void StateInit();
    void StateConstruct();
    void StatePaused();
    void StateAnimating();
    void StateCompleted();

    // FX Callback
    static Bool FXCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *context);
  };
}

#endif