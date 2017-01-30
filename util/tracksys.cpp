///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// General Purpose Death Tracker
//
// 8-JUL-1998
//

#include "tracksys.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class TrackSys - Manages a death tracker for general purpose items
//

//
// Static Data
//
Bool TrackSys::sysInit;
DTrack* TrackSys::dTracker;


//
// TrackSys::Init
//
// Initialize system
//
void TrackSys::Init()
{
  ASSERT(!sysInit);

  // Allocate a death tracker
  dTracker = new DTrack("Misc", 4096);

  // System now initialized
  sysInit = TRUE;
}

 
//
// TrackSys::Done
//
// Shutdown system
//
void TrackSys::Done()
{
  ASSERT(sysInit);

  // Delete the death tracker
  delete dTracker;

  // System now shutdown
  sysInit = FALSE;
}
 

//
// TrackSys::RegisterConstruction
//
// Register the creation of an item
//
void TrackSys::RegisterConstruction(DTrack::Info &info)
{
  ASSERT(sysInit);

  // Call the death tracker object
  dTracker->RegisterConstruction(info);
}


//
// TrackSys::RegisterDestruction
//
// Register the destruction of the item 'info'
//
void TrackSys::RegisterDestruction(DTrack::Info &info)
{
  ASSERT(sysInit);

  // Call the death tracker object
  dTracker->RegisterDestruction(info);
}
