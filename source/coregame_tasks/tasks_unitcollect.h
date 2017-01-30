///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_UNITCOLLECT_H
#define __TASKS_UNITCOLLECT_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"
#include "promote.h"


///////////////////////////////////////////////////////////////////////////////
// 
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitCollect
  //
  class UnitCollect : public GameTask<UnitObjType, UnitObj>
  {
    TASK_CLASS(UnitCollect)

  private:

    // Current resource object
    ResourceObjPtr resourceObj;

    // Flag indicating that we have a resource location
    Bool haveResourceLocation;
    
    // Last location resource was collected from
    Vector lastResourceLocation;

    // Current storage facility
    UnitObjPtr storageObj;

    // Flag indicating that we have a storage location
    Bool haveStorageLocation;

    // Last location resource was stored at
    Vector lastStorageLocation;

    // ListNode
    struct ListNode : public ResourceObjPtr
    {
      // List node
      NList<ListNode>::Node node;

      // Time when reject expires
      F32 timeout;

    };
    typedef ReaperList<ResourceObj, ListNode> ResourceList;

    // Rejected resource
    ResourceList rejects;

    // Movement request handle
    Movement::Handle moveHandle;

    // FX Callback
    static Bool FXCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *context);

    Bool IsCollecting()
    {
      return inst.Test(0x0974185E); // "LoadResource"
    }

  public:

    // Constructor and Destructor
    UnitCollect(GameObj *subject);
    ~UnitCollect();

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

  private:

    // State Machine procedures
    void StateWait(StateMachineNotify notify);

    void StateGoToResource(StateMachineNotify notify);
    void StateAlignResource(StateMachineNotify notify);
    void StatePreLoadResource(StateMachineNotify notify);
    void StateLoadResource(StateMachineNotify notify);
    void StatePostLoadResource(StateMachineNotify notify);

    void StateLeaveResource(StateMachineNotify notify);
    void StateGoToStorage(StateMachineNotify notify);

    void StatePreUnloadResource(StateMachineNotify notify);
    void StateUnloadResource(StateMachineNotify notify);
    void StatePostUnloadResource(StateMachineNotify notify);

    void StateGoToResourceArea(StateMachineNotify notify);

    void StateSearchForResource(StateMachineNotify notify);
    void StateSearchForStorage(StateMachineNotify notify);

    // Find the a resource
    ResourceObj * FindResource();

  public:

    // Store what we've got
    void Store();

    // Set the storage object
    void SetStorageObject(UnitObj *obj, Bool update);

    // Clear all knowledge of a storage object
    void ClearStorageObject();

    // Get more resource
    void Resource();

    // Set the resource object
    void SetResourceObject(ResourceObj *obj);
    
  };

}

#endif