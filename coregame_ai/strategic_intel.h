/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Intel
//


#ifndef __STRATEGIC_INTEL_H
#define __STRATEGIC_INTEL_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic.h"
#include "offmapobj.h"


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


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Intel
  //
  class Intel
  {
  public:

    /////////////////////////////////////////////////////////////////////////////
    //
    // Forward Declarations
    //
    class Manager;

  protected:

    // Intel manager
    Manager &manager;

  private:

    // Config name
    GameIdent configName;

    // Manager list node
    NList<Intel>::Node nodeManagerList;
    NBinTree<Intel>::Node nodeManagerTree;

    // Is the intel idle ?
    Bool idle;

    // Interval
    U32 intervalMin;
    U32 intervalMax;
    U32 nextTime;

  public:

    // Constructor
    Intel(Manager &manager, const GameIdent &configName, FScope *fScope);

    // Destructor
    virtual ~Intel();

    // Set the next time to process
    void SetTime();

    // Is it ready to go
    Bool IsReady();

    // Process the intel
    virtual Bool Process() = 0;

  public:

    // Create an intel from the given fscope
    static Intel * Create(Manager &manager, const GameIdent &configName, FScope *fScope);

  public:

    // Get the script manager in control of this script
    Manager & GetManager()
    {
      return (manager);
    }

  public:

    // Friends
    friend class Manager;

  };

}

#endif
