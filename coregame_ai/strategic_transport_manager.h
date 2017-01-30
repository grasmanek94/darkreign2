/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Transport Manager
//


#ifndef __STRATEGIC_TRANSPORT_MANAGER_H
#define __STRATEGIC_TRANSPORT_MANAGER_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_transport.h"
#include "strategic_notification.h"


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
  class Transport::Manager
  {
  private:

    // Strategic Object the manager belongs to
    Object *strategic;


    // Idle transports
    NBinTree<Transport> idle;

    // Active Transports
    NBinTree<Transport> active;


  public:

    // Constructor
    Manager();

    // Destructor
    ~Manager();

    // Save a transport tree
    void SaveTransportTree(FScope *scope, const NBinTree<Transport> &tree);

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope);

    // Handle notification
    void Notify(Notification &notification);


    // Add a transport
    void AddTransport(TransportObj &transport);

    // Remove a transport
    void RemoveTransport(TransportObj &transport);

    // Find a transport
    Transport * FindIdleTransport(TransportObj &transport);


    // Activate a transport
    void ActivateTransport(Transport &transport);

    // Deactivate a transport
    void DeactivateTransport(Transport &transport);


    // Find a suitable list of transports for this squad
    void FindTransports(Script &script);


    // Remove the dead transports
    void RemoveDeadTransports();


    // Cleanup all transports
    void CleanUp();

  public:

    // Setup
    void Setup(Object *object)
    {
      strategic = object;
    }

    // Get the strategic object from the manager
    Object & GetObject()
    {
      ASSERT(strategic)
      return (*strategic);
    }

  };

}

#endif
