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
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Redbook - System dealing with redbook audio
  //
  namespace Redbook
  {
    // Is the system initialized
    static Bool initialized = FALSE;

    // Handle to the cd audio driver
    static HREDBOOK driver;

    // Current redbook track
    static U32 track;

    // Minimum redbook track to play
    static U32 minTrack;

    // Are we searching for redbook tracks
    static Bool search;

    // Next redbook poll time
    static U32 pollTime;

    // Is rebook enabled
    static Bool enabled;

    // Volume label to look for first
    static char volumeLabel[64];

    // Maximum redbook volume
    enum { MIN_VOLUME = 0, MAX_VOLUME = 127 };


    //
    // Init
    //
    // Initialize systems
    //
    void Init()
    {
      ASSERT(!initialized);

      driver = NULL;
      track = 0;
      minTrack = 2;
      search = FALSE;
      enabled = TRUE;
      *volumeLabel = '\0';

      // Set init flag
      initialized = TRUE;
    }


    //
    // Done
    //
    // Shutdown systems
    //
    void Done()
    {
      ASSERT(initialized);

      Release();

      // Clear init flag
      initialized = FALSE;
    }


    //
    // SetVolumeLabel
    //
    // Set the volume label to look for first
    //
    void SetVolumeLabel(const char *volume)
    {
      Utils::Strmcpy(volumeLabel, volume, sizeof(volumeLabel));
    }

    
    //
    // CriticalShutdown
    //
    // Critical shutdown function
    //
    void CriticalShutdown()
    {
      Release();
    }

    
    //
    // GetEnabled
    //
    // Is redbook audio enabled
    //
    Bool GetEnabled()
    {
      return (enabled);
    }


    //
    // SetEnabled
    //
    // Set whether redbook is enabled
    //
    void SetEnabled(Bool flag)
    {
      enabled = flag;
    }


    //
    // SyncEnabled
    //
    // Sync with enabled state by claiming or releasing driver
    //
    void SyncEnabled()
    {
      if (initialized)
      {
        if (enabled)
        {
          if (!Claimed())
          {
            Claim();
          }
        }
        else
        {
          if (Claimed())
          {
            Release();
          }
        }
      }
    }


    //
    // Claim
    //
    // Claim a redbook driver
    //
    Bool Claim()
    {   
      if (initialized)
      {
        int drive;

        // Release any current driver
        Release();
  
        // First, try and find the specified volume
        if (*volumeLabel && Drive::FindDriveByVolume(Drive::CDROM, volumeLabel, drive))
        {
          if ((driver = AIL_redbook_open_drive(char(drive + 'A'))) != NULL)
          {
            LOG_DIAG(("Redbook::Claim - Success by volume [%s]", volumeLabel));
          }
        }

        // Second, try and find an audio cd (for custom cd in second drive)
        if (!driver && Drive::FindDriveByVolume(Drive::CDROM, "Audio CD", drive))
        {
          if ((driver = AIL_redbook_open_drive(char(drive + 'A'))) != NULL)
          {
            LOG_DIAG(("Redbook::Claim - Success on Audio CD"));
          }
        }

        // Third, just grab the first device
        if (!driver)
        {
          for (S32 i = 0; i < MAX_DEVICES; i++)
          {
            if ((driver = AIL_redbook_open(i)) != NULL)
            {
              LOG_DIAG(("Redbook::Claim - Success on first available [%d]", i));
              break;
            }
          }
        }

        if (driver)
        {
          // Stop the cd playing anything
          Stop();
          return (TRUE);
        }

        LOG_DIAG(("Redbook::Claim - Failed"));
      }

      return (FALSE);
    }


    //
    // Release
    //
    // Release a redbook driver
    //
    Bool Release()
    {
      if (initialized)
      {
        if (driver)
        {
          // Stop the cd playing anything
          Stop();

          // Release the driver
          AIL_redbook_close(driver);

          // Clear driver pointer
          driver = NULL;

          return (TRUE);
        }
      }

      return (FALSE);
    }


    //
    // Claimed
    //
    // Do we currently have a redbook driver
    //
    Bool Claimed()
    {
      return (initialized && driver ? TRUE : FALSE);
    }


    //
    // Stop
    //
    // Stop redbook audio
    //
    void Stop()
    {
      if (initialized && driver)
      {
        // Reset our track counter
        track = 0;

        // Stop the redbook device
        AIL_redbook_stop(driver);

        // Stop searching
        search = FALSE;
      }
    }


    //
    // Play
    //
    // Play a redbook track (request is wrapped, so returns track number started)
    //
    U32 Play(U32 newTrack)
    {
      if (initialized && driver)
      {
        // Get number of tracks on disc
        U32 count = AIL_redbook_tracks(driver);

        // Clip track into acceptable range
        if ((newTrack > count) || (newTrack < minTrack)) 
        {
          newTrack = minTrack;
        }

        // If we ended up with a valid track number, start it playing
        if (newTrack >= 1 && newTrack <= count)
        {
          U32 start, end;
    
          // Get track info
          AIL_redbook_track_info(driver, newTrack, &start, &end);

          // Start it playing
          AIL_redbook_play(driver, start, end);

          // Update our track count
          track = newTrack;

          // Set next scan to be just past when this track ends
          pollTime = AIL_ms_count() + (end - start) + POLL_ERROR;

          // We're rockin, so turn on track searching
          search = TRUE;
        }
        else
        {
          // Probably a data cd
          search = FALSE;

          // Stop cd playing
          Stop();
        }
      }

      return (track);
    }


    //
    // SetMinTrack
    //
    // Set the minimum cd audio track allowed (for skipping data tracks)
    //
    void SetMinTrack(U32 newMinTrack)
    {
      minTrack = Max<U32>(1, newMinTrack);
    }


    //
    // Poll
    //
    // Poll the cd audio (used for proceeding to next track automatically)
    //
    void Poll()
    {
      if (initialized && driver)
      {
        U32 status;

        // Time to search
        if (search && (pollTime <= AIL_ms_count()))
        {
          // Read the status (this can be slow on some drives)
          switch (status = AIL_redbook_status(driver))
          {
            // Still playing a track
            case REDBOOK_PLAYING:
            {
              U32 curTrack, curPos, end;

              // Get current info
              curTrack = AIL_redbook_track(driver);
              curPos = AIL_redbook_position(driver);

              // Some systems will briefly report playing the NEXT track!
              if ((curTrack == track) && track && curPos)
              {
                // Get track info
                AIL_redbook_track_info(driver, track, NULL, &end);

                // Recalc the scan time
                pollTime = AIL_ms_count() + (end - curPos) + POLL_ERROR;
              }

              break;
            }

            // Finished playing track
            case REDBOOK_STOPPED:
              Play(track + 1);
              break;

            default :
              Stop();
              break;
          }
        }
      }    
    }


    //
    // TrackCount
    //
    // The number of tracks on the current audio device
    //
    U32 TrackCount()
    {
      return((initialized && driver) ? AIL_redbook_tracks(driver) : 0);
    }


    //
    // CurrentTrack
    //
    // The current audio track
    //
    U32 CurrentTrack()
    {
      return (initialized ? track : 0);
    }


    //
    // SetVolume
    //
    // Sets the current redbook audio volume
    //
    void SetVolume(F32 volume)
    {
      if (initialized && driver)
      {
        AIL_redbook_set_volume
        (
          driver, Clamp<S32>(MIN_VOLUME, S32((volume * F32(MAX_VOLUME)) + 0.5F), MAX_VOLUME)
        );
      }
    }


    //
    // Volume
    //
    // Returns the current redbook volume
    //
    F32 Volume()
    {
      if (initialized && driver)
      {
        return (F32(AIL_redbook_volume(driver)) / F32(MAX_VOLUME));
      }

      return (0.0F);
    }
  }
}

