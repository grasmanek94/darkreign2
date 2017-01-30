///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "tasks_transportpad.h"
#include "resolver.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // State machine
  StateMachine<TransportPad> TransportPad::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class TransportPad
  //


  //
  // Constructor
  //
  TransportPad::TransportPad(GameObj *sub) 
  : GameTask<TransportObjType, TransportObj>(staticConfig, sub),
    inst(&stateMachine, "Process"),
    charge(1.0F)
  {
    // Get the number of seconds to charge
    F32 time = subject->TransportType()->GetChargeTime();

    // Work out the maximum charge rate
    chargeRate = (time > 0.0F) ? (1.0F / (time * GameTime::SimTimeInv())) : 1.0F;

    location.ClearData();
  }


  //
  // Save
  //
  void TransportPad::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));
    StdSave::TypeF32(fScope, "Charge", charge);

    if (portal.Alive())
    {
      StdSave::TypeReaper(fScope, "Portal", portal);
      portalTimer.SaveState(fScope->AddFunction("PortalTimer"));
    }

    StdSave::TypeVector(fScope, "Location", location);
  }

  //
  // Load
  //
  void TransportPad::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
          break;

        case 0x4DA42E38: // "Charge"
          charge = StdLoad::TypeF32(sScope);
          break;

        case 0x79D1E3E5: // "Portal"
          StdLoad::TypeReaper(sScope, portal);
          break;

        case 0x5FF45B95: // "PortalTimer"
          portalTimer.LoadState(sScope);
          break;

        case 0x693D5359: // "Location"
          StdLoad::TypeVector(sScope, location);
          break;

        default:
          LoadTaskData(sScope);
          break;
      }
    }
  }



  //
  // Called after all objects are loaded
  //
  void TransportPad::PostLoad()
  {
    Resolver::Object<TransportObj, TransportObjType>(portal);
  }


  //
  // Perform task processing
  //
  Bool TransportPad::Process()
  {
    inst.Process(this);  
    return (quit);
  }

  
  //
  // Processing state
  //
  void TransportPad::StateProcess()
  {
    // Do we need to charge
    if (charge < 1.0F)
    {
      // Increment the progress counter
      charge += chargeRate * subject->GetEfficiency();
      
      // Are we done
      if (charge >= 1.0F)
      {
        // Close any open portal
        portalTimer.Reset();
      }
      else
      {
        ThinkFast();
      }
    }

    // Is the portal open
    if (portal.Alive())
    {
      // Has it expired
      if (portalTimer.Test())
      {
        RemovePortal();
      }

      // Process each cycle
      ThinkFast();
    }
  }


  //
  // Processing state
  //
  void TransportPad::StateUnload()
  {
    // Process the process state (so that recharging occurs)
    StateProcess();

    // Get the cell position
    U32 x = WorldCtrl::MetresToCellX(location.x);
    U32 z = WorldCtrl::MetresToCellZ(location.z);

    // Can we unload
    if (subject->CheckUnload(x, z))
    {
      // Unload immediately
      Unload(location);

      // Notify the player that we're done
      subject->NotifyPlayer(0x5CA4B1C1); // "Transport::Unloaded"

      // Change to the process state
      NextState(0xBDF7C019); // "Process"
    }
  }


  //
  // RemovePortal
  //
  // Delete any current portal
  //
  Bool TransportPad::RemovePortal()
  {
    if (portal.Alive())
    {
      portal->MarkForDeletion();
      return (TRUE);
    }

    return (FALSE);
  }


  //
  // Process
  //
  // Process Events
  //
  Bool TransportPad::ProcessEvent(const Event &event)
  {
    UnitObj *unit = event.object.Alive() ? Promote::Object<UnitObjType, UnitObj>(event.object) : NULL;

    switch (event.message)
    {
      case GameObjNotify::Interrupted:
      case MapObjNotify::Dying:
        RemovePortal();
        return (TRUE);

      case Movement::Notify::BlockEntry:
      {
        ASSERT(unit)

        // Block entry if unit is not allowed on
        return (!subject->CheckCargo(unit));
      }

      case Movement::Notify::UnitEntered:
      {
        // Get into the transport
        subject->PickupCargo(unit);

        // Return value is ignored
        return (FALSE);
      }
    }
    return (GameTask<TransportObjType, TransportObj>::ProcessEvent(event));
  }


  //
  // Retrieve
  //
  // Retrieve data from the task, return TRUE if filled
  //
  Bool TransportPad::Retrieve(U32 id, RetrievedData &data)
  {
    switch (id)
    {
      case TaskRetrieve::Progress:
      {
        if (subject->GetTelepadLink())
        {
          return (FALSE);
        }

        data.f1 = GetCharge();

        // Display different charging vs charged bars
        if (data.f1 < 1.0F)
        {
          data.u1 = 0x4F2144F1; // "Tasks::TransportPad::Charge"
        }
        else
        {
          data.u1 = 0x23DBCCCA; // "Tasks::TransportPad::Charged"
        }

        return (TRUE);
      }

      case TaskRetrieve::Count:
      {
        data.u1 = subject->GetUsedSpaces();
        return (TRUE);
      }
    }

    return (GameTask<TransportObjType, TransportObj>::Retrieve(id, data));
  }


  //
  // Unload
  //
  // Unload at the given location
  //
  Bool TransportPad::Unload(const Vector &destination, Bool single)
  {
    // Ensure previous portal is deleted
    RemovePortal();

    // Get the portal type
    if (TransportObjType *p = subject->TransportType()->GetPortalType())
    {
      Vector closest;
   
      // Get the closest linked location
      if (p->FindLinkedPos(destination, closest))
      {
        // Create the portal
        portal = (TransportObj*)(&p->Spawn(closest));

        // Point it back
        portal->SetTelepadLink(subject);

        // Start the timer
        portalTimer.Start(subject->TransportType()->GetPortalTime());
      }
      else
      {
        return (FALSE);
      }
    }

    // Start processing each cycle
    ThinkFast();

    // Attempt to unload
    if (subject->Unload(destination, single))
    {
      // Clear the charge
      charge = 0.0F;

      // Success
      return (TRUE);
    }

    // No objects unloaded
    return (FALSE);
  }


  // 
  // UnloadAvailable
  //
  // Is this transporter able to unload right now
  //
  Bool TransportPad::UnloadAvailable()
  {
    return ((charge >= 1.0F) ? TRUE : FALSE);
  }


  //
  // CheckUnload
  //
  // Check if this telepad can currently unload at the given location
  //
  Bool TransportPad::CheckUnload(U32 x, U32 z)
  {
    // Get the portal type
    TransportObjType *p = subject->TransportType()->GetPortalType();

    // No portal, or portal can move to this cell
    return(!p || PathSearch::CanMoveToCell(p->GetTractionIndex(subject->GetCurrentLayer()), x, z));
  }


  //
  // GetCharge
  //
  // Get the current charge percentage
  //
  F32 TransportPad::GetCharge()
  {
    return ((charge >= 1.0F) ? 1.0F : charge);
  }


  //
  // CheckPortalEntry
  //
  // Can this given unit come back thru the portal
  //
  Bool TransportPad::CheckPortalEntry(UnitObj *)
  {
    // Insert code here to check for an available spot
    return (TRUE);
  }


  //
  // TransferPortalUnit
  //
  // Suck a unit through a portal
  //
  Bool TransportPad::TransferPortalUnit(UnitObj *unit)
  {
    Vector src, dst;

    if (subject->GetFootInstance())
    {
      Point<S32> pos(unit->GetCellX(), unit->GetCellZ());
      subject->GetFootInstance()->ClampToFringe(pos);
      src.Set(WorldCtrl::CellToMetresX(pos.x), 0.0F, WorldCtrl::CellToMetresZ(pos.z));
    }
    else
    {
      src = subject->Position();
    }

    // Find the closest cell near the target
    if (unit->UnitType()->FindClosestPos(src, dst))
    {
      subject->Unload(dst, unit);
      return (TRUE);
    }   

    return (FALSE);
  }


  //
  // Attempt to unload
  //
  void TransportPad::SetUnloadDestination(const Vector &destination)
  {
    NextState(0xBE6BD9FC); // "Unload"
    location = destination;
  }


  //
  // Initialization
  //
  void TransportPad::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Add states to the state machine
    stateMachine.AddState("Process", &StateProcess);
    stateMachine.AddState("Unload", &StateUnload);
  }


  //
  // Shutdown
  //
  void TransportPad::Done()
  {
    stateMachine.CleanUp();
  }
}
