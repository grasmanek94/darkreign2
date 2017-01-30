/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Transport Manager
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_transport_manager.h"
#include "strategic_script.h"
#include "resolver.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Transport::Manager
  //


  //
  // Constructor
  //
  Transport::Manager::Manager()
  : idle(&Transport::nodeManager),
    active(&Transport::nodeManager)
  {

  }


  //
  // Destructor
  //
  Transport::Manager::~Manager()
  {
    // Clean up
    idle.DisposeAll();
    active.DisposeAll();
  }


  //
  // SaveTransportTree
  //
  // Save a transport tree
  //
  void Transport::Manager::SaveTransportTree(FScope *scope, const NBinTree<Transport> &tree)
  {
    for (NBinTree<Transport>::Iterator i(&tree); *i; i++)
    {
      if (FScope *sScope = StdSave::TypeReaper(scope, "Transport", **i))
      {
        (*i)->SaveState(sScope);
      }
    }
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Transport::Manager::SaveState(FScope *scope)
  {
    SaveTransportTree(scope, idle);
    SaveTransportTree(scope, active);
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Transport::Manager::LoadState(FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xDFB7F0C8: // "Transport"
        {
          // Load and resolve the transport object
          TransportObjPtr reaper;
          StdLoad::TypeReaper(sScope, reaper);
          Resolver::Object<TransportObj, TransportObjType>(reaper);

          if (reaper.Alive())
          {
            if (Transport *transport = FindIdleTransport(*reaper))
            {
              transport->LoadState(sScope);
            }
          }       
          break;
        }
      }
    }
  }


  //
  // Handle notification
  //
  void Transport::Manager::Notify(Notification &notification)
  {
    // Is this one of our transports ?
    Transport *transport = active.Find(notification.from.DirectId());

    switch (notification.message)
    {
      case 0x61FC2088: // "Unit::MoveCompleted"
        if (transport)
        {
          // If this transport is assigned to a script then
          // tell that script the transport has completed its move
          if (transport->script)
          {
            transport->SetFlag();
            transport->script->Notify(0x998A995A); // "Transport::Moved"
          }
        }

      case 0x5CA4B1C1: // "Transport::Unloaded"
        if (transport)
        {
          // If this transport is assigned to a script then 
          // tell that script the transport is unloaded
          if (transport->script)
          {
            transport->SetFlag();
            transport->script->Notify(0x5CA4B1C1); // "Transport::Unloaded"
          }
        }
        break;

      case 0x11EAEF8E: // "Unit::Died"

        // A unit has died, check to see if its one of our transports
        if (transport)
        {
          // If this transport is assigned to a base then remove it

          // If this transport is assigned to a squad then
          // notify the squad that the transport has died

          if (transport->script)
          {
            transport->RemoveFromSquad(transport->script);
            transport->script->Notify(0x38601711); // "Transport::Died"
          }

        }
        break;
    }
  }


  //
  // Add a transport
  //
  void Transport::Manager::AddTransport(TransportObj &transport)
  {
    idle.Add(transport.Id(), new Transport(&transport, *this));
  }


  //
  // Remove a transport
  //
  void Transport::Manager::RemoveTransport(TransportObj &transport)
  {
    transport;

    ERR_FATAL(("Why are we removing a transport ??"))
  }


  //
  // Find a transport
  //
  Transport * Transport::Manager::FindIdleTransport(TransportObj &transport)
  {
    return (idle.Find(transport.Id()));
  }


  //
  // Activate transport
  //
  void Transport::Manager::ActivateTransport(Transport &transport)
  {
    ASSERT(transport.nodeManager.InUse())
    ASSERT(idle.InTree(&transport))

    U32 key = transport.nodeManager.GetKey();

    idle.Unlink(&transport);
    active.Add(key, &transport);
  }


  //
  // Deactivate transport
  //
  void Transport::Manager::DeactivateTransport(Transport &transport)
  {
    ASSERT(transport.nodeManager.InUse())
    ASSERT(active.InTree(&transport))

    U32 key = transport.nodeManager.GetKey();

    active.Unlink(&transport);
    idle.Add(key, &transport);
  }


  //
  // FindTransports
  //
  // Find a suitable list of transports for this squad
  //
  void Transport::Manager::FindTransports(Script &script)
  {
    // We have a number of slots we need (provided by the number of units in the squad)
    // Use the transports closest to the squad first

    U32 slots = script.GetSquad()->GetList().GetCount();

    BinTree<Transport, F32> transports;

    // Get the current location of the suqad
    Vector location;

    if (script.GetSquad()->GetLocation(location))
    {
      // Make sure that there's no dead transports lingering around
      RemoveDeadTransports();

      // Sort the transports by distance from the squad
      for (NBinTree<Transport>::Iterator i(&idle); *i; i++)
      {
        transports.Add((location - (**i)->Origin()).Magnitude2(), *i);
      }

      // Now itereate the sorted transports and assign them to the script
      for (BinTree<Transport, F32>::Iterator t(&transports); *t; t++)
      {
        Transport &transport = **t;

        // Assign this transport to the script
        transport.AssignToSquad(&script);

        // How many slots does this transport provide ?
        U32 available = transport->TransportType()->GetSpaces();

        if (available >= slots)
        {
          slots = 0;
          break;
        }

        // We still have slots to go .. keep looking
        slots -= available;
      }

      // We're out of transports, did we get enough ?
      if (slots)
      {
        // We didn't get enough, notify the script
        script.Notify(0x3BBBD1F7); // "Transport::NotEnough"
      }
      else
      {
        // We got enough, notify the script
        script.Notify(0x9BA84E05); // "Transport::Enough"
      }

      transports.UnlinkAll();
    }

  }


  //
  // Remove the dead transports
  //
  void Transport::Manager::RemoveDeadTransports()
  {
    NBinTree<Transport>::Iterator i(&idle);
    Transport *transport;
    while ((transport = i++) != NULL)
    {
      if (!transport->Alive())
      {
        idle.Dispose(transport);
      }
    }

    NBinTree<Transport>::Iterator a(&active);
    while ((transport = a++) != NULL)
    {
      if (!transport->Alive())
      {
        active.Dispose(transport);
      }
    }

  }


  //
  // Cleanup all transports
  //
  void Transport::Manager::CleanUp()
  {
    // Remove the transports from any scripts they are associated with
    NBinTree<Transport>::Iterator t(&active);
    Transport *transport;
    while ((transport = t++) != NULL)
    {
      if (transport->script)
      {
        transport->RemoveFromSquad(transport->script);
      }
    }

  }

}

