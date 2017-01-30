///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


#ifndef __TASKS_TRANSPORTPAD_H
#define __TASKS_TRANSPORTPAD_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"
#include "promote.h"
#include "transportobj.h"


///////////////////////////////////////////////////////////////////////////////
// 
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class TransportPad
  //
  class TransportPad : public GameTask<TransportObjType, TransportObj>
  {
    TASK_CLASS(TransportPad)

    // The current charge
    F32 charge;

    // The progress per cycle at max efficiency
    F32 chargeRate;

    // The portal
    TransportObjPtr portal;

    // Time remaining for portal
    GameTime::Timer portalTimer;

    // Location to unload
    Vector location;

    // Delete any current portal
    Bool RemovePortal();

  public:

    // Constructor
    TransportPad(GameObj *subject);

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

    // Unload at the given location
    Bool Unload(const Vector &destination, Bool single = FALSE);

    // Is this transporter able to unload right now
    Bool UnloadAvailable();

    // Check if this telepad can currently unload at the given location
    Bool CheckUnload(U32 x, U32 z);
    
    // Get the current charge percentage
    F32 GetCharge();

    // Can this given unit come back thru the portal
    Bool CheckPortalEntry(UnitObj *unit);

    // Suck a unit through a portal
    Bool TransferPortalUnit(UnitObj *unit);

    // Attempt to unload
    void SetUnloadDestination(const Vector &destination);

  private:

    // State machine procedures
    void StateProcess();
    void StateUnload();

  };

}

#endif