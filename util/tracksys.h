///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// General Purpose Death Tracker
//
// 8-JUL-1998
//

#ifndef __TRACKSYS_H
#define __TRACKSYS_H


#include "dtrack.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class TrackSys - Manages a death tracker for general purpose items
//

class TrackSys
{
private:
 
  // Has the system been initialized
  static Bool sysInit;

  // The death tracker itself
  static DTrack *dTracker;


public:

  // Initialize system
  static void Init();

  // Shutdown system
  static void Done();

  // Register the construction of an item
  static void RegisterConstruction(DTrack::Info &info);

  // Register the destruction of the item 'info'
  static void RegisterDestruction(DTrack::Info &info);
};

#endif
