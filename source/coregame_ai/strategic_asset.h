/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Asset
// 25-MAR-1999
//


#ifndef __STRATEGIC_ASSET_H
#define __STRATEGIC_ASSET_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic.h"
#include "unitobj.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Asset
  //
  class Asset
  {
  public:

    /////////////////////////////////////////////////////////////////////////////
    //
    // Forward Declarations
    //
    class Manager;
    class Request;

  private:

    // Manager Node
    NBinTree<Asset>::Node nodeManager;

    // Assigned Node
    NList<Asset>::Node nodeAssigned;

    // List of requests which are interested in this asset
    List<Request> requests;

    // Reaper to a unit
    UnitObjPtr unit;

  public:

    // Unit constructor
    Asset(UnitObj *unit);

    // Destructor
    ~Asset();

    // Id of the asset
    U32 GetId();

    // A name for the asset
    const char *GetName();

    // Is the asset alive ?
    Bool Alive();

    // Get the unit from the asset
    UnitObj * GetUnit();

  public:

    // Has the asset been assigned ?
    Bool IsAssigned()
    {
      return (nodeAssigned.InUse());
    }

  public:

    // Friends
    friend class Manager;
    friend class Request;

  };
}

#endif
