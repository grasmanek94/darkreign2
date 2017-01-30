///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_RESOURCEREGEN_H
#define __TASKS_RESOURCEREGEN_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"
#include "promote.h"
#include "restoreobj.h"


///////////////////////////////////////////////////////////////////////////////
// 
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class ResourceRegen
  //
  class ResourceRegen : public GameTask<ResourceObjType, ResourceObj>
  {
    TASK_CLASS(ResourceRegen)

  private:

    // The construction effect
    FX::Object * effect;

  public:

    // Constructor
    ResourceRegen(GameObj *subject);

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
    void StateRegen();

    // FX Callback
    static Bool FXCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *context);

  };

}

#endif