/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Transport
//


#ifndef __STRATEGIC_TRANSPORT_H
#define __STRATEGIC_TRANSPORT_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic.h"
#include "transportobj.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Forward Declarations
  //
  class Base;
  class Script;


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Transport
  //
  class Transport : public TransportObjPtr
  {
  public:

    /////////////////////////////////////////////////////////////////////////////
    //
    // Forward Declarations
    //
    class Manager;

  private:

    // The location on the map to return to
    Vector location;

    // Pointer to squad that this transport is associated with
    Script *script;

    // Script node
    NList<Transport>::Node nodeScript;

    // Transport manager
    Manager &manager;

    // Manager node
    NBinTree<Transport>::Node nodeManager;

    // Flag
    Bool flag;

  public:

    // Constructor
    Transport(TransportObj *transport, Manager &manager);

    // Destructor
    ~Transport();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope);


    // Return this transport to its initial location
    void Return();

    
    // Assign this base to a squad
    void AssignToSquad(Script *squad);

    // Remove this base from a squad
    void RemoveFromSquad(Script *squad);


  public:

    // Set location
    void SetLocation(const Vector &locationIn)
    {
      location = locationIn;
    }

    // Clear the flag
    void ClearFlag()
    {
      flag = FALSE;
    }

    // Set the flag
    void SetFlag()
    {
      flag = TRUE;
    }

    // Test the flag
    Bool TestFlag()
    {
      return (flag);
    }

  public:

    friend class Manager;
    friend class Script;

  };

}

#endif
