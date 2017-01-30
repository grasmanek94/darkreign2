///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Sound System
//
// 8-OCT-1997
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "sound_private.h"


namespace Sound 
{ 
  namespace Digital
  {
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Namespace Output - Manages audio channel use
    //
    namespace Output
    {
      ///////////////////////////////////////////////////////////////////////////////
      //
      // Struct Voice - A single audio voice, used for 2D and 3D channels
      //
      struct Voice
      {
        // Is this a 3D voice
        Bool flag3D;

        // Sample handles for each type
        HSAMPLE handle2D;
        H3DSAMPLE handle3D;

        // Reaper to effect that played the current sample
        EffectPtr effect;

        // Reaper to record being played
        RecordPtr record;
 
        // Who owns what's playing
        U32 owner;
 
        // Priority of this effect
        F32 priority;


        //
        // Constructor for a 2D channel
        //
        Voice(HSAMPLE v)
        {
          // Type setup
          flag3D = FALSE;
          handle2D = v;

          // Setup default values
          owner = NO_OWNER;
          priority = 0.0F;

          // Initialize the voice
          AIL_init_sample(handle2D);
        }


        //
        // Constructor for a 3D channel
        //
        Voice(H3DSAMPLE v)
        {
          // Type setup
          flag3D = TRUE;
          handle3D = v;

          // Setup default values
          owner = NO_OWNER;
          priority = 0.0F;
        }


        //
        // Destructor
        //
        ~Voice()
        {
          // Release the voice
          if (flag3D)
          {
            AIL_release_3D_sample_handle(handle3D);
          }
          else
          {
            AIL_release_sample_handle(handle2D);
          }
        }


        //
        // Return the current sample status
        //
        U32 Status()
        {
          return
          (
            flag3D ? AIL_3D_sample_status(handle3D) : AIL_sample_status(handle2D)
          );
        }


        //
        // Stop voice playing
        //
        void Stop()
        {
          if (flag3D)
          {
            AIL_end_3D_sample(handle3D);
          }
          else
          {
            AIL_end_sample(handle2D);
          }
        }
      };

      // Maximum number of voices allowed
      enum { MAX_VOICES = 128 };
  
      // Current number of voices allocated
      static U32 totalVoices = 0;

      // Array of pointers to allocated voices
      static Voice *voices[MAX_VOICES];



      ///////////////////////////////////////////////////////////////////////////////
      //
      // System Functions
      //

      //
      // Request
      //
      // Request the use of a voice
      //
      static Voice * Request(F32 priority, Bool flag3D, U8 &index)
      {
        ASSERT(Initialized());

        // Try and find a voice that is currently free
        for (U32 i = 0; i < totalVoices; i++)
        {
          // Get the voice at this index
          Voice *voice = voices[i];

          // Correct type and available for use
          if ((voice->flag3D == flag3D) && (voice->Status() == SMP_DONE))
          {
            index = (U8)i;
            return (voice);
          }
        }
 
        // Try and free a voice using the priority system
        for (i = 0; i < totalVoices; i++)
        {
          // Get the voice at this index
          Voice *voice = voices[i];

          // Correct type and lower priority
          if ((voice->flag3D == flag3D) && (voice->priority < priority))
          {
            index = (U8)i;
            voice->Stop();
            return (voice);
          }
        }

        return (NULL);     
      }


      //
      // FindVoicePlayingEffect
      //
      // Returns the first voice playing 'e', or NULL
      //
      static Voice * FindVoicePlayingEffect(Effect *e)
      {
        // Ensure index is valid
        if (e->GetVoiceIndex() < totalVoices)
        {
          // Get the voice at this index
          Voice *v = voices[e->GetVoiceIndex()];

          // Still playing and pointing to the given effect
          if ((v->Status() != SMP_DONE) && v->effect.Alive() && (v->effect == e))
          {
            return (v);
          }
        }

        return (NULL);
      }

      
      //
      // Claim
      //
      // Claim and 2D and 3D voices using the given providers
      //
      void Claim(HDIGDRIVER *h2D, U32 max2D, HPROVIDER *h3D, U32 max3D)
      {
        LOG_DIAG(("Output::Claim:"));

        // Do we have a 2D digital audio service
        if (h2D)
        {
          // Allocate the 2D voices
          for (U32 v = 0; v < max2D && totalVoices < MAX_VOICES; v++)
          {
            // Allocate a new voice handle
            if (HSAMPLE vHandle = AIL_allocate_sample_handle(*h2D))
            {
              // Allocate a new 2D voice
              voices[totalVoices++] = new Voice(vHandle);
            }
            else
            {
              // No more voices to allocate
              break;      
            }
          }
        
          LOG_DIAG((" - Allocated %d 2D voices", v));
        }
        else
        {
          LOG_DIAG((" - No 2D digital audio services"));
        }

        // Do we have a 3D digital audio service
        if (h3D)
        {
          // Allocate the 3D voices
          for (U32 v = 0; v < max3D && totalVoices < MAX_VOICES; v++)
          {
            // Allocate a new voice handle
            H3DSAMPLE vHandle = AIL_allocate_3D_sample_handle(*h3D);

            if (vHandle)
            {
              // Allocate a new 3D voice
              voices[totalVoices++] = new Voice(vHandle);
            }
            else
            {
              // No more voices to allocate
              break;      
            }
          }
        
          LOG_DIAG((" - Allocated %d 3D voices", v));
        }
        else
        {
          LOG_DIAG((" - No 3D digital audio services"));
        }      
      }


      //
      // Release
      //
      // Release all voices
      //
      void Release()
      {
        // Delete all voices
        for (U32 i = 0; i < totalVoices; i++)
        {
          delete voices[i];
        }

        // Reset the total voice count
        totalVoices = 0;
      }


      //
      // Play2D
      //
      // Play a record as a 2D sample (TRUE if actually started)
      //
      Bool Play2D
      (
        Effect *e, Record *r, S32 vol, U32 owner, F32 priority, S32 loop, S32 pan
      )
      {
        ASSERT(e);
        ASSERT(r);

        // Request a 2D voice
        Voice *voice = Request(priority, FALSE, e->GetVoiceIndex());

        if (!voice)
        {
          return (FALSE);
        }

        // Request the cache data
        Cache::Item *item = Cache::Request(r);

        if (!item)
        {
          // Unable to load data
          return (FALSE);
        }

        // Stop any sounds from this owner
        if (owner != NO_OWNER) 
        {
          Output::StopByOwner(owner);
        }

        // The current time
        U32 time = AIL_ms_count();

        // Are we under the minimum repeat time
        if ((time - r->lastUse) < MIN_REPEATTIME)
        {
          return (FALSE);
        }

        // Save info about this use
        r->lastUse = time;

        // Initialise the voice
        AIL_init_sample(voice->handle2D);

        // Initialise the sound effect data
        if (!AIL_set_sample_file(voice->handle2D, item->data, 0))
        {
          LOG_WARN(("Ignoring possibly corrupted file '%s'", r->Name()));
          r->valid = FALSE;
          return (FALSE);
        }

        // Setup the voice
        voice->effect = e;
        voice->record = r;
        voice->owner = owner;
        voice->priority = priority;

        // Set the volume
        AIL_set_sample_volume(voice->handle2D, vol);

        // Set the stereo panning
        AIL_set_sample_pan(voice->handle2D, pan);

        // Set the loop count for this voice
        AIL_set_sample_loop_count(voice->handle2D, loop);

        // Start the sample playing
        AIL_start_sample(voice->handle2D);

        return (TRUE);
      }


      //
      // UpdateEffect2D
      //
      // Update effect data (TRUE if effect was playing)
      //
      Bool UpdateEffect2D(Effect *e, S32 pan, F32 priority, S32 vol)
      {
        // See if this effect is still being played
        if (Voice *voice = FindVoicePlayingEffect(e))
        {
          AIL_set_sample_pan(voice->handle2D, pan);
          voice->priority = priority;
          AIL_set_sample_volume(voice->handle2D, vol);
          return (TRUE);
        }

        return (FALSE);
      }


      //
      // Play3D
      //
      // Play a record as a 3D sample (TRUE if actually started)
      //
      Bool Play3D
      (
        Effect *e, Record *r, S32 vol, U32 owner, F32 priority, S32 loop, 
        F32 x, F32 y, F32 z, F32 min, F32 max
      )
      {
        ASSERT(e);
        ASSERT(r);

        // Request a 3D voice
        Voice *voice = Request(priority, TRUE, e->GetVoiceIndex());

        if (!voice)
        {
          return (FALSE);
        }

        ASSERT(voice->flag3D);

        // Request the cache data
        Cache::Item *item = Cache::Request(r);

        if (!item)
        {
          // Unable to load data
          return (FALSE);
        }

        // Stop any sounds from this owner
        if (owner != NO_OWNER) 
        {
          Output::StopByOwner(owner);
        }

        // The current time
        U32 time = AIL_ms_count();

        // Are we under the minimum repeat time
        if ((time - r->lastUse) < MIN_REPEATTIME)
        {
          return (FALSE);
        }

        // Save info about this use
        r->lastUse = time;

        // Initialise the sound effect data
        if (!AIL_set_3D_sample_file(voice->handle3D, item->data))
        {
          LOG_WARN(("Invalid 3D data '%s' (Require MS Mono Uncompressed PCM WAV)", r->Name()));
          r->valid = FALSE;
          return (FALSE);
        }

        // Setup the voice
        voice->effect = e;
        voice->record = r;
        voice->owner = owner;
        voice->priority = priority;

        // Set the volume
        AIL_set_3D_sample_volume(voice->handle3D, vol);

        // Set the loop count for this voice
        AIL_set_3D_sample_loop_count(voice->handle3D, loop);

        // Set the 3D world position
        AIL_set_3D_position(voice->handle3D, x, y, z);

        // Set the distances for this sample
        AIL_set_3D_sample_distances(voice->handle3D, max, min);

        // Always face the listener 
        AIL_set_3D_orientation(voice->handle3D, -x, -y, -z, 0.0F, 1.0F, 0.0F);

        // Start the sample playing
        AIL_start_3D_sample(voice->handle3D);

        return (TRUE);
      }

      
      //
      // UpdateEffect3D
      //
      // Update effect data (TRUE if effect was playing)
      //
      Bool UpdateEffect3D
      (
        Effect *e, F32 x, F32 y, F32 z, F32 dx, F32 dy, F32 dz, F32 priority, S32 vol
      )
      {
        // See if this effect is still being played
        Voice *voice = FindVoicePlayingEffect(e);

        // If so, update the 3D world position and velocity
        if (voice)
        {
          AIL_set_3D_position(voice->handle3D, x, y, z);
          AIL_set_3D_velocity_vector(voice->handle3D, dx, dy, dz);
          AIL_set_3D_sample_volume(voice->handle3D, vol);
          voice->priority = priority;
          return (TRUE);
        }

        return (FALSE);
      }


      //
      // EffectPlaying
      //
      // Is this given effect still playing
      //
      Bool EffectPlaying(Effect *e)
      {
        return (FindVoicePlayingEffect(e) ? TRUE : FALSE);
      }


      //
      // RecordInUse
      //
      // TRUE if 'record' is currently being used by a voice
      //
      Bool RecordInUse(Record *record)
      {
        if (Initialized())
        {
          // Try and find a voice that is using the record
          for (U32 i = 0; i < totalVoices; i++)
          {
            // Get the voice at this index
            Voice *voice = voices[i];

            if (voice->Status() != SMP_DONE && voice->record == record)
            {
              return (TRUE);
            }
          }
        }

        return (FALSE);
      }

    
      //
      // Stop
      //
      // Stops all voices that are currently playing
      //
      void Stop()
      {
        if (Initialized())
        {
          for (U32 i = 0; i < totalVoices; i++)
          {
            voices[i]->Stop();
          }
        }
      }


      //
      // StopByEffect
      //
      // Stop all voices played using the given effect
      //
      void StopByEffect(Effect *e)
      {
        if (Initialized())
        {
          for (U32 i = 0; i < totalVoices; i++)
          {
            // Get the voice at this index
            Voice *voice = voices[i];

            // Effect is still alive and matches the one supplied
            if (voice->effect.Alive() && voice->effect == e)
            {
              voice->Stop();
            }
          }        
        }
      }


      //
      // StopByRecord
      //
      // Stop all voices playing the given record
      //
      void StopByRecord(Record *r)
      {
        if (Initialized())
        {
          for (U32 i = 0; i < totalVoices; i++)
          {
            // Get the voice at this index
            Voice *voice = voices[i];

            if ((voice->Status() != SMP_DONE) && (voice->record == r))
            {
              voice->Stop();
            }
          }
        }
      }


      //
      // StopByOwner
      //
      // Stop all voices with the given owner id
      //
      void StopByOwner(U32 id)
      {
        if (Initialized())
        {
          for (U32 i = 0; i < totalVoices; i++)
          {
            // Get the voice at this index
            Voice *voice = voices[i];

            if (voice->owner == id)
            {
              voice->Stop();
            }
          }
        }
      }
    }
  }
}

