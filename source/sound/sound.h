///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Sound System
//
// 8-OCT-1997
//


#ifndef __SOUND_H
#define __SOUND_H


//
// Includes
//
#include "dtrack.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Sound - Sound system
//

namespace Sound
{
  // Initialize and shutdown systems
  void Init();
  void Done();

  // Poll reserved voices and cd audio
  void Poll();

  // Critical shutdown function
  void CriticalShutdown();

  // Returns TRUE if the system is initialised
  Bool Initialized();

  // Returns the last Miles error message
  const char * LastError();


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Digital - System dealing with digital sound
  //
  namespace Digital
  {
    // The default volume  
    const F32 DEFAULT_VOLUME = 0.5F;

    // Used to indicate an effect has no owner
    enum { NO_OWNER = 0 };

    // Panning extents
    enum { PAN_LEFT = 0, PAN_MIDDLE = 64, PAN_RIGHT = 127 };

    // Enumerate all available 3D providers
    Bool Next3DProvider(const char * &name, Bool &first);

    // Claim and release both 2D and 3D services
    void Claim(U32 max2D = 8, U32 max3D = 0);
    void Release();

    // Set the 3D provider preference (FALSE if not found)
    Bool SetProviderPreference(U32 index);
    Bool SetProviderPreference(const char *name);

    // Change the 3D provider using the current preference
    Bool Change3DProvider(Bool claim = TRUE);

    // Are digital audio services available
    Bool Claimed2D();
    Bool Claimed3D();

    // Disable all digital sound services
    void SetDisabled(Bool toggle);

    // Index of the current 3D provider (only valid if Claimed3D is true)
    U32 GetServiceIndex3D();

    // Text description of current service, or "No current service"
    const char * GetServiceName2D();
    const char * GetServiceName3D();

    // Update the listener orientation for all open providers
    void UpdateListener(F32 fx, F32 fy, F32 fz, F32 ux, F32 uy, F32 uz);

    // Clears all records for known sound effects
    void ClearRecords();
 
    // Set the digital audio volume (0..1)
    void SetVolume(F32 volume);

    // Get the current digital audio volume (0..1)
    F32 Volume();

    // Returns the percentage (0-100) of the cpu used by the digital driver
    U32 CPUPercent();

    // Returns the number of active 3D samples
    S32 SampleCount3D();

    // Stop all effects, streams, and reserved voices
    void Stop();


    /////////////////////////////////////////////////////////////////////////////
    //
    // NameSpace Stream - Streaming services (mp3s)
    //
    namespace Stream
    {
      // Start streaming a file
      Bool Start(const char *ident, const char *file);

      // Stop a stream, or ALL if ident is NULL
      void Stop(const char *ident = NULL);

      // Returns TRUE if the stream is currently playing
      Bool IsPlaying(const char *ident);
    }


    //
    // Reaper types
    //
    typedef Reaper<class Effect> EffectPtr;
    typedef Reaper<struct Record> RecordPtr;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Namespace Output - Manages audio channel use
    //
    namespace Output
    {
      // Stop all voices with the given owner id
      void StopByOwner(U32 id);
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Effect - Sound effect wrapper
    //
    class Effect
    {
    protected:

      // Is this effect valid
      U8 valid;

      // Record data for this wrapper
      RecordPtr record;

      // Last voice played on
      U8 voiceIndex;

    public:

      // Death tracking info
      DTrack::Info dTrack;

      // Constructor and destructor
      Effect();
      Effect(const char *name);
      ~Effect();

      // Setup this effect
      Bool Setup(const char *name);

      // Clear this effect
      void Clear();

      // Play this effect using a 2D channel (TRUE if actually started)
      Bool Play2D
      (
        F32 vol = DEFAULT_VOLUME, U32 owner = NO_OWNER, F32 priority = 0.0F, S32 loopCount = 1, 
        S32 pan = PAN_MIDDLE
      );

      // Update a 2D effect (TRUE if effect was playing)
      Bool Update2D(S32 pan, F32 priority = 0.0F, F32 vol = DEFAULT_VOLUME);

      // Play this effect using a 3D channel (TRUE if actually started)
      Bool Play3D
      (
        F32 x, F32 y, F32 z, F32 min = 5.0F, F32 max = 200.0F,
        F32 vol = DEFAULT_VOLUME, U32 owner = NO_OWNER, F32 priority = 0.0F, S32 loopCount = 1
      );

      // Update the position of a 3D effect (TRUE if effect was playing)
      Bool Update3D
      (
        F32 x, F32 y, F32 z, F32 dx, F32 dy, F32 dz, F32 priority = 0.0F, F32 vol = DEFAULT_VOLUME
      );

      // Stop any voices that were started by this effect instance
      void StopByEffect();

      // Stop any voices that are playing the same data file
      void StopByRecord();

      // Is an effect being played by this wrapper
      Bool Playing();

      // Is this effect setup and pointing to a valid record
      Bool Valid();

      // Get the voice index
      U8 & GetVoiceIndex()
      {
        return (voiceIndex);
      }
    };
    

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Namespace Reserved - Provides named reserved voices
    //
    namespace Reserved
    {
      enum Mode
      {
        MODE0,   // Submissions will be ignored
        MODE1,   // Clear all MODE2 and MODE3 items from queue, then queue this item
        MODE2,   // Ignore if MODE1 item active, otherwise queue
        MODE3,   // Ignore if anything is active
        MODE4,   // Stop anything that is playing and play this item
      };

      // Set the volume modifier applied to all reserved voices
      void SetVolume(F32 volume);

      // Get the current volume modifier
      F32 Volume();
      
      // Submit a sample to a reserved voice
      void Submit(const char *voice, const char *file, Mode mode = MODE2);

      // Reset the given reserved voice, or all if NULL
      void Reset(const char *voice = NULL);
    }   
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Redbook - System dealing with redbook audio
  //
  namespace Redbook
  {
    // Set the volume label to look for first
    void SetVolumeLabel(const char *volume);

    // Control whether redbook audio enabled
    Bool GetEnabled();
    void SetEnabled(Bool flag);

    // Sync with enabled state by claiming or releasing driver
    void SyncEnabled();

    // Claim and release a redbook driver
    Bool Claim();
    Bool Release();

    // Do we currently have a redbook driver
    Bool Claimed();

    // Stop any current track
    void Stop();

    // Play a redbook track (request is wrapped, so returns track number started)
    U32 Play(U32 newTrack);

    // Set the minimum cd audio track allowed (for skipping data tracks)
    void SetMinTrack(U32 newMinTrack);

    // The number of tracks on the current audio device
    U32 TrackCount();

    // The current audio track
    U32 CurrentTrack();

    // Sets the current redbook audio volume
    void SetVolume(F32 volume);

    // Returns the current redbook volume
    F32 Volume();
  }
}

#endif
