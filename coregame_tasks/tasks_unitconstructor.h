///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Constructor Task
//
// 9-JUN-1999
//


#ifndef __TASKS_UNITCONSTRUCTOR_H
#define __TASKS_UNITCONSTRUCTOR_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"
#include "promote.h"
#include "player.h"
#include "offmapobj.h"


///////////////////////////////////////////////////////////////////////////////
// 
// NameSpace Tasks
//
namespace Tasks
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitConstructor
  //
  class UnitConstructor : public GameTask<UnitObjType, UnitObj>
  {
    TASK_CLASS(UnitConstructor)
    
  protected:

    // Type currently being constructed
    UnitObjTypePtr construct;

    // The queue of unit types waiting to exit the facility
    UnitObjTypeList outputQueue;

    // OffMap object waiting to be used
    OffMapObjPtr offMap;

    // Are we currently paused
    Bool paused;

    // The current progress total
    F32 progressTotal;

    // The progress per cycle at max efficiency
    F32 progressMax;

    // Amount of resource removed
    S32 resourceRemoved;

    // Amount of resource left to remove
    S32 resourceRemaining;

    // Is there a construction order in transit
    Bool orderInTransit;

    // The client-side construction queue
    UnitObjTypeList clientQueue;

    // The optional rally point
    Bool rallyPointActive;
    Point<U32> rallyPoint;

    // The count of objects created
    U32 creationCount;

    // State machine procedures
    void StateProcess();

    // Has the unit limit been reached for the subjects team
    Bool UnitLimitReached(UnitObjType *type);

  public:

    // Constructor and destructor
    UnitConstructor(GameObj *subject);
    ~UnitConstructor();

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

    // Get the type currently being constructed, or NULL
    UnitObjType * GetConstructType();

    // Get the offmap object waiting to be used, or NULL
    OffMapObj * GetOffMapObject();

    // Returns the percent complete
    F32 GetProgress();
   
    // Is construction currently paused
    Bool Paused();


    // Add one queue item
    void QueueAdd(UnitObjType *type);

    // Remove one queue item
    void QueueRemove(UnitObjType *type);

    // Clear entire queue, or just given type
    void QueueClear(UnitObjType *type = NULL);

    // Total number of items queued, or just for given type
    S32 QueueCount(UnitObjType *type = NULL);

    // Process the next queue item
    Bool QueueProcess(Player &player);

    
    // Set the order in transit flag
    void SetOrderInTransit(Bool flag)
    {
      orderInTransit = flag;
    }

    // Is there an order currently in transit
    Bool GetOrderInTransit()
    {
      return (orderInTransit);
    }

    // FX Callback
    static Bool FXCallBack(MapObj *mapObj, FX::CallBackData &, void *context);
  };
}

#endif