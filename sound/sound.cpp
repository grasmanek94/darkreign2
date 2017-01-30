///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Sound System
//
// 8-OCT-1997
//


//
// Includes
//
#include "sound_private.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Sound - Sound system
//

namespace Sound
{
  // Is the system initialized
  static Bool initialized = FALSE;

  // The sound system death tracker 
  static DTrack *dTracker;


  //
  // Init
  //
  // Initialize systems
  //
  void Init()
  {
    LOG_DIAG(("Sound::Init()"));

    if (!initialized)
    {
      // Allocate a death tracker
      dTracker = new DTrack("Sound", 128);

      // Tell MSS where to find its files
      AIL_set_redist_directory("library\\mss");

      // Initialize MSS
      AIL_startup();

      // Initialize the digital sound system
      Digital::Init();

      // Initialize the redbook audio system
      Redbook::Init();

      // Set init flag
      initialized = TRUE;
    }

    LOG_DIAG((" - Completed"));
  }


  //
  // Done
  //
  // Shutdown systems
  //
  void Done()
  {
    LOG_DIAG(("Sound::Done()"));

    if (initialized)
    {
      // Shutdown the digital sound system
      Digital::Done();

      // Shutdown the redbook audio system
      Redbook::Done();

      // Shutdown MSS
      AIL_shutdown();

      // Delete the death tracker
      delete dTracker;

      // Clear init flag
      initialized = FALSE;
    }

    LOG_DIAG((" - Completed"));
  }


  //
  // Poll
  //
  // Poll reserved voices and cd audio
  //
  void Poll()
  {
    if (initialized)
    {
      // Process reserved voice queues
      Digital::Reserved::Poll();

      // Process CD audio track changing
      Redbook::Poll();
    }
  }

  
  //
  // CriticalShutdown
  //
  // Critical shutdown function
  //
  void CriticalShutdown()
  {
    LOG_DIAG(("Entering Sound::CriticalShutdown"));

    if (initialized)
    {
      Digital::CriticalShutdown();
      Redbook::CriticalShutdown();
      AIL_shutdown();
    }
    LOG_DIAG(("Leaving Sound::CriticalShutdown"));
  }


  //
  // Initialized
  //
  // Returns TRUE if the system is initialised
  //
  Bool Initialized()
  {
    return (initialized);
  }


  //
  // LastError
  //
  // Returns the last Miles error message
  //
  const char * LastError()
  {
    return (AIL_last_error());
  }


  //
  // DumpInfo
  //
  // Log information about the current state
  //
  void DumpInfo()
  {

  }


  //
  // RegisterConstruction
  //
  // Register the creation of an item
  //
  void RegisterConstruction(DTrack::Info &info)
  { 
    ASSERT(initialized)
    dTracker->RegisterConstruction(info);
  }


  //
  // RegisterDestruction
  //
  // Register the destruction of an item
  //
  void RegisterDestruction(DTrack::Info &info)
  {
    ASSERT(initialized)
    dTracker->RegisterDestruction(info);
  }
}

