///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Sound System
//
// 8-OCT-1997
//


#ifndef __SOUND_PRIVATE_H
#define __SOUND_PRIVATE_H


//
// Includes
//
#include "mss.h"
#include "sound.h"
#include "filesys.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Sound - Sound system
//

namespace Sound
{ 
  // Register the construction and destruction of items
  void RegisterConstruction(DTrack::Info &info);
  void RegisterDestruction(DTrack::Info &info);


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Digital - System dealing with digital sound
  //
  namespace Digital
  {
    // Digital volume extents
    enum { MIN_VOLUME = 0, MAX_VOLUME = 127 };

    // Maximum number of 3D providers
    enum { MAX_PROVIDERS = 30 };

    // Maximum cache elements
    enum { MAX_CACHE_ITEMS = 100 };

    // Minimum sfx repeat time (ms)
    enum { MIN_REPEATTIME = 200 };

    // Default maximum cache size (bytes)
    enum { DEF_MAXCACHESIZE = 524288 };

    // Default loop mode
    enum { DEF_LOOPCOUNT = 1 };

    // Rebook poll error correction (ms)
    enum { RB_POLLERROR = 100 };

    // Initialize and shutdown digital system
    void Init();
    void Done();

    // Critical shutdown function
    void CriticalShutdown();

    // Is sound currently disabled
    Bool Disabled();

    // Get a pointer to the 2D driver
    HDIGDRIVER GetDriver();

    // Convert a user volume to a system volume
    S32 ConvertVolume(F32 volume);


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Struct Record - Data maintained for each known sound effect
    //
    struct Record
    {
      // Tree of known effects, keyed by file name crc
      static NBinTree<Record> tree;

      // Tree node
      NBinTree<Record>::Node node;

      // Death tracking info
      DTrack::Info dTrack;

      // Is this file valid
      Bool valid;

      // Crc of file name
      U32 nameCrc;

      // Fast file finder
      FileSys::FastFind *fastFind;

      // Play frequency
      S32 freq;

      // Last time played (ms)
      S32 lastUse;

      // Constructor and destructor
      Record(const char *name);
      ~Record();

      // File name of this record
      const char * Name();

      // Get the record for a file
      static Record * Request(const char *name);
      
      // Release all current records
      static void ReleaseAll();
    };


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Namespace Cache - Manages actual effect data
    //
    namespace Cache
    {
      // A single cache item
      struct Item
      {
        // List node
        NList<Item>::Node node;

        // Record for this data
        RecordPtr record;

        // Allocated memory for the data
        void *data;

        // Constructor and destructor
        Item(Record *record, void *data);
        ~Item();
      };

      // Initialize and shutdown the cache
      void Init();
      void Done();

      // Clear all items
      void Clear();

      // Request an item from the cache
      Item * Request(Record *record);
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Namespace Output - Manages audio channel use
    //
    namespace Output
    {
      // Claim and 2D and 3D voices using the given providers
      void Claim(HDIGDRIVER *h2D, U32 max2D, HPROVIDER *h3D, U32 max3D);

      // Release all claimed voices
      void Release();

      // Stops all voices that are currently playing
      void Stop();

      // Play a record as a 2D sample (TRUE if actually started)
      Bool Play2D
      (
        Effect *e, Record *r, S32 vol, U32 owner, F32 priority, S32 loop, S32 pan
      );

      // Update effect data (TRUE if effect was playing)
      Bool UpdateEffect2D(Effect *e, S32 pan, F32 priority, S32 vol);

      // Play a record as a 3D sample (TRUE if actually started)
      Bool Play3D
      (
        Effect *e, Record *r, S32 vol, U32 owner, F32 priority, S32 loop, 
        F32 x, F32 y, F32 z, F32 min, F32 max
      );

      // Update effect data (TRUE if effect was playing)
      Bool UpdateEffect3D
      (
        Effect *e, F32 x, F32 y, F32 z, F32 dx, F32 dy, F32 dz, F32 priority, S32 vol
      );

      // Is this given effect still playing
      Bool EffectPlaying(Effect *e);

      // TRUE if 'record' is currently being used by a voice
      Bool RecordInUse(Record *record);

      // Stops all voices that are currently playing
      void StopAllVoices();

      // Stop all voices played using the given effect
      void StopByEffect(Effect *e);

      // Stop all voices playing the given record
      void StopByRecord(Record *r);
    }   


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Namespace Reserved - Provides named reserved voices
    //
    namespace Reserved
    {
      // Initialize and shutdown system
      void Init();
      void Done();

      // Release all voices
      void Release();

      // Poll all reserved voices
      void Poll();
    }   
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Redbook - System dealing with redbook audio
  //
  namespace Redbook
  {
    // Max devices to scan for
    enum { MAX_DEVICES = 26 };

    // Time to wait before checking for next track (ms)
    enum { POLL_ERROR = 200 };

    // Initialize and shutdown redbook system
    void Init();
    void Done();

    // Critical shutdown function
    void CriticalShutdown();

    // Poll the cd audio (used for proceeding to next track automatically)
    void Poll();
  }
}

#endif
