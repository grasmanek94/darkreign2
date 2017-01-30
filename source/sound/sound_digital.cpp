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
#include "registry.h"
#include "bink.h"

namespace Sound 
{ 
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Digital - System dealing with digital sound
  //
  namespace Digital
  {
    // Is the system initialized
    static Bool initialized = FALSE;

    // Are services disabled
    static Bool disabled;

    // Default voice counts
    static U32 defaultVoice2D = 4;
    static U32 defaultVoice3D = 6;

    // Handle to the digital audio driver
    static HDIGDRIVER driver;

    // Provider preference
    static U32 providerPreference = 0xBD4161A9; // "Miles Fast 2D Positional Audio"

    // Currently open M3D provider record, or NULL
    static class Provider *provider;

    // Text description of driver
    static char digDriverStr[256];


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Provider - Handles M3D digital audio providers
    //
    class Provider
    {
    private:

      // List of enumerated M3D providers
      static List<Provider> list;

      // Iterator for providers
      static List<Provider>::Iterator listIterator;

    public:

      // Initialize provider records
      static void Init()
      {
        ASSERT(!list.GetCount());

        // Create new list
        HPROVIDER newProvider;
        char *name;
        HPROENUM next = HPROENUM_FIRST;

        U32 index = 0;

        // Request each available provider
        while (AIL_enumerate_3D_providers(&next, &newProvider, &name))
        {
          LOG_DIAG((" - [%s] (%d)", name, index));
    
          // Create a new provider record
          new Provider(index++, name, newProvider);
        }

        // Initialize iterator
        listIterator.SetList(&list);
      }

      // Release all current provider records
      static void Done()
      {
        list.DisposeAll();
      }

      // Get the list of available providers
      static const List<Provider> & GetList()
      {
        return (list);
      }

      // Next
      static Bool Next(const char * &name, Bool &first)
      {
        // Should we reset the iterator
        if (first)
        {
          listIterator.GoToHead();
          first = FALSE;
        }

        // Get the next provider
        Provider *p = listIterator++;

        // Set the name
        if (p)
        {
          name = p->Name();
          return (TRUE);
        }

        return (FALSE);
      }

    private:

      // The index of this provider
      U32 index;

      // Is this provider open
      Bool open;

      // Text description
      const char *name;

      // Miles provider handle
      HPROVIDER handle;

      // The listener allocated for this provider
      H3DPOBJECT listener;

      // Constructor
      Provider(U32 index, const char *n, HPROVIDER h) : index(index), open(FALSE), name(n), handle(h)
      {
        list.Append(this);
      }

    public:

      // Open this provider
      Bool Open()
      {   
        // Ensure we're closed
        Close();

        // Attempt to open the provider
        if (AIL_open_3D_provider(handle) == M3D_NOERR)
        {
          listener = AIL_3D_open_listener(handle);
          return (open = TRUE);
        }

        return (FALSE);
      }

      // Close this provider
      void Close()
      {
        if (open)
        {
          AIL_3D_close_listener(listener);
          AIL_close_3D_provider(handle);
          open = FALSE;
        }
      }

      // Returns the index of this provider
      U32 Index() 
      { 
        return (index); 
      }

      // Returns the text description of this provider
      const char * Name() 
      { 
        return (name); 
      }

      // Returns the crc of the text description of this provider
      U32 NameCrc() 
      { 
        return (Crc::CalcStr(name)); 
      }

      // Returns the M3D handle for this provider
      HPROVIDER & Handle()
      { 
        return (handle); 
      }

      // Returns the listener allocated for this provider (must be open)
      H3DPOBJECT Listener()
      {
        ASSERT(open);
        return (listener);
      }
    };

    // List of providers    
    List<Provider> Provider::list;
    List<Provider>::Iterator Provider::listIterator;


    //
    // Open2DDevice
    //
    // Attempts to get a handle to a digital device, TRUE on success
    // 
    static Bool Open2DDevice(HDIGDRIVER *dig, S32 rate, S16 bits, S16 chans)
    {
      ASSERT(initialized);

      PCMWAVEFORMAT sPCMWF;

      // Setup with requested and default paramaters
      sPCMWF.wf.wFormatTag      = WAVE_FORMAT_PCM;
      sPCMWF.wf.nChannels       = chans;
      sPCMWF.wf.nSamplesPerSec  = rate;
      sPCMWF.wf.nAvgBytesPerSec = rate * (bits / 8) * chans;
      sPCMWF.wf.nBlockAlign     = (U16) ((bits / 8) * chans);
      sPCMWF.wBitsPerSample     = bits;

      // Get the digital driver, returning error code
      return
      (
        AIL_waveOutOpen(dig, NULL, WAVE_MAPPER, (LPWAVEFORMAT) &sPCMWF) ? FALSE : TRUE
      );
    }


    //
    // HideAureal
    //
    // Remove/Restore Aureal logo settings
    //
    void HideAureal(Bool init)
    {
      static Bool found = FALSE;
      static S32 screen, audio;

      // Open the A3D key
      Registry::Key key(HKEY_LOCAL_MACHINE, "Software\\Aureal\\A3D", Registry::ALL, FALSE);

      if (init)
      {
        // Was it found
        if (!key.NotFound())
        {
          // Store the values for later
          screen = key.GetInt("SplashScreen");
          audio = key.GetInt("SplashAudio");

          // But turn them off now
          key.SetInt("SplashScreen", FALSE);
          key.SetInt("SplashAudio", FALSE);

          // Remember to restore it
          found = TRUE;
        }
      }
      else

      if (found && !key.NotFound())
      {
        // Restore the poor users splash screen values
        key.SetInt("SplashScreen", screen);
        key.SetInt("SplashAudio", audio);
      }
    }


    //
    // Init
    //
    // Initialize systems
    //
    void Init()
    {
      ASSERT(!initialized);

      // Turn off Aureal logo
      //HideAureal(TRUE);

      // Setup local data
      disabled = FALSE;
      driver = NULL;
      provider = NULL;

      // Find available 3D service providers
      Provider::Init();

      // Initialize the cache
      Cache::Init();

      // Initialize the reserved voice system
      Reserved::Init();

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

      // Release all digital services
      Release();

      // Shutdown the reserved voice system
      Reserved::Done();

      // Shutdown the cache
      Cache::Done();

      // Release 3D provider records
      Provider::Done();

      // Restore Aureal settings
      //HideAureal(FALSE);

      // Clear init flag
      initialized = FALSE;
    }

    
    //
    // CriticalShutdown
    //
    // Critical shutdown function
    //
    void CriticalShutdown()
    {
      // Restore Aureal settings
      //HideAureal(FALSE);
    }


    //
    // Disabled
    //
    // Is sound currently disabled
    //
    Bool Disabled()
    {
      return (disabled);
    }


    //
    // GetDriver
    //
    // Get a pointer to the 2D driver
    //
    HDIGDRIVER GetDriver()
    {
      return (initialized ? driver : NULL);
    }


    //
    // GetVolume
    //
    // Convert a user volume to a system volume
    //
    S32 ConvertVolume(F32 volume)
    {
      return (Clamp<S32>(MIN_VOLUME, Utils::FtoL((volume * MAX_VOLUME) + 0.5F), MAX_VOLUME));
    }


    //
    // Release2D
    //
    // Release 2D digital audio services
    //
    static void Release2D()
    {
      ASSERT(!Claimed3D());

      if (driver)
      {     
        // Release sound driver
        AIL_waveOutClose(driver);
        driver = NULL;     
      }
    }


    //
    // Claim2D
    //
    // Claim 2D digital audio services
    //
    static Bool Claim2D()
    {
      ASSERT(initialized);
      ASSERT(!Claimed2D());
      ASSERT(!Claimed3D());

      // Attempt to open digital device
      if (Open2DDevice(&driver, 22050, 16, 2))
      {
        // Clear driver string because miles concats to it
        *digDriverStr = '\0';

        // Get text description of driver
        AIL_digital_configuration(driver, 0, 0, digDriverStr);

        LOG_DIAG(("Claim2D: Opened [%s]", GetServiceName2D()));

        // Provide bink with the available driver
        BinkSoundUseMiles(driver);

        return (TRUE);
      }

      // Unable to get a digital audio driver
      LOG_DIAG(("Claim2D: Failed"));
      LOG_DIAG((" - %s", AIL_last_error()));

      // Must clear this because Miles screws with it
      driver = NULL;
      return (FALSE);
    }


    //
    // Release3D
    //
    // Release 3D digital audio services
    //
    static void Release3D()
    {
      // Is there a currently open provider
      if (provider)
      {
        // Shut it down
        provider->Close();

        // Clear our pointer
        provider = NULL;
      }
    }


    //
    // Claim3D
    //
    // Claim 3D digital audio services
    //
    static Bool Claim3D()
    {
      ASSERT(initialized);
      ASSERT(Claimed2D());
      ASSERT(!Claimed3D());

      if (providerPreference)
      {
        // Search all providers for the preference
        for (List<Provider>::Iterator i(&Provider::GetList()); *i; i++)
        {
          // Get a pointer to the provider
          Provider *p = *i;

          // Is this the one we're after
          if (p->NameCrc() == providerPreference)
          {
            // Attempt to open the provider
            if (p->Open())
            {
              LOG_DIAG(("Claim3D: Opened [%s]", p->Name()));

              // Set as current provider
              provider = p;
              return (TRUE);
            }
            else
            {
              LOG_DIAG(("Claim3D: Failed [%s]", p->Name()));
              LOG_DIAG((" - %s", AIL_last_error()));
              return (FALSE);
            }
          }
        }

        LOG_DIAG(("Claim3D: Unable to find provider preference (%08x)", providerPreference));
      }
      else
      {
        LOG_DIAG(("Claim3D: No provider preference set"));
      }

      return (FALSE);
    }


    //
    // ClaimOutput
    //
    // Claim output voices using current services
    //
    void ClaimOutput()
    {
      ASSERT(initialized);

      // Claim output channels
      Output::Claim
      (
        driver ? &driver : NULL, defaultVoice2D, 
        provider ? &provider->Handle() : NULL, defaultVoice3D
      );     
    }


    //
    // Next3DProvider
    //
    // Enumerate all available providers
    //
    Bool Next3DProvider(const char * &name, Bool &first)
    {
      return (initialized ? Provider::Next(name, first) : FALSE);
    }


    //
    // Claim
    //
    // Claim digital services
    //
    void Claim(U32 max2D, U32 max3D)
    {
      if (initialized)
      {
        // Release any current services
        Release();
 
        // Setup our preferences
        AIL_set_preference(DIG_USE_WAVEOUT, NO);

        // Save default voice count requests
        defaultVoice2D = Max<U32>(1, max2D);
        defaultVoice3D = max3D;

        // Claim 2D digital audio services
        if (Claim2D())
        {
          // Claim 3D digital audio services
          if (defaultVoice3D)
          {
            Claim3D();
          }
    
          // Claim output voices
          ClaimOutput();
        }
      }
    }

    
    //
    // Release
    //
    // Release both 2D and 3D services
    //
    void Release()
    {
      if (initialized)
      {
        // Stop everything
        Stop();

        // Clear all records
        ClearRecords();

        // Release voices
        Output::Release();

        // Release services
        Release3D();
        Release2D();
      }
    }


    //
    // SetProviderPreference
    //
    // 
    // Set the 3D provider preference (FALSE if not found)
    //
    Bool SetProviderPreference(U32 index)
    {
      if (initialized)
      {
        // Go to the requested index
        if (index < Provider::GetList().GetCount())
        {
          // Create an iterator for the provider list
          List<Provider>::Iterator i(&Provider::GetList());

          // Go to the requested index
          i.GoTo(index);

          // Get a pointer to the provider
          Provider *p = *i;

          ASSERT(p);

          LOG_DIAG(("SetProviderPreference: %s", p->Name()));

          // Set this as current preference
          providerPreference = p->NameCrc();
          return (TRUE);
        }

        LOG_DIAG
        ((
          "SetProviderPreference: Failed (%d/%d)", index, Provider::GetList().GetCount()
        ));

        // Clear the preference
        providerPreference = 0;
      }

      return (FALSE);
    }


    //
    // SetProviderPreference
    //    
    // Set the 3D provider preference (FALSE if not found)
    //
    Bool SetProviderPreference(const char *name)
    {
      if (initialized)
      {
        // Search all providers for the preference
        for (List<Provider>::Iterator i(&Provider::GetList()); *i; i++)
        {
          // Get a pointer to the provider
          Provider *p = *i;

          // Calculate the crc of the requested provider
          U32 crc = Crc::CalcStr(name);

          // Is this the one we're after
          if (p->NameCrc() == crc)
          {
            LOG_DIAG(("SetProviderPreference: %s", name));

            // Set this as current preference
            providerPreference = crc;
            return (TRUE);
          }
        }

        LOG_DIAG(("SetProviderPreference: Failed (%s)", name));

        // Clear the preference
        providerPreference = 0;
      }

      return (FALSE);
    }


    //
    // Change3DProvider
    //
    // Change the 3D provider using the current preference
    //
    Bool Change3DProvider(Bool claim)
    {
      if (Claimed2D())
      {
        // Stop everything
        Stop();

        // Release voices
        Output::Release();

        // Release any current 3D service
        Release3D();

        // Save the result
        Bool result = TRUE;

        // Claim 3D services using current preference
        if (claim)
        {
          result = Claim3D();
        }

        // Claim output voices
        ClaimOutput();

        return (result);
      }

      return (claim ? FALSE : TRUE);
    }


    //
    // Claimed2D
    //
    // Are 2D digital audio services available
    //
    Bool Claimed2D()
    {
      return (initialized && driver ? TRUE : FALSE);
    }


    //
    // Claimed3D
    //
    // Are 3D digital audio services available
    //
    Bool Claimed3D()
    {
      return (initialized && provider ? TRUE : FALSE);
    }


    //
    // SetDisabled
    //
    // Disable all digital sound services
    //
    void SetDisabled(Bool toggle)
    {
      if (toggle)
      {
        Stop();

        disabled = TRUE;
      }
      else
      {
        disabled = FALSE;
      }
    }


    //
    // GetServiceIndex3D
    //
    // Index of the current 3D provider (only valid if Claimed3D is true)
    //
    U32 GetServiceIndex3D()
    {
      return (provider ? provider->Index() : 0);
    }


    //
    // GetServiceName2D
    //
    // Text description of current service
    //
    const char * GetServiceName2D()
    {
      return (initialized && driver ? digDriverStr : "No current service");
    }


    //
    // GetServiceName3D
    //
    // Text description of current service
    //
    const char * GetServiceName3D()
    {
      return (initialized && provider ? provider->Name() : "No current service");
    }


    //
    // UpdateListener
    //
    // Update the listener orientation for all open providers
    //
    void UpdateListener(F32 fx, F32 fy, F32 fz, F32 ux, F32 uy, F32 uz)
    {
      if (initialized && provider)
      {
        AIL_set_3D_orientation(provider->Listener(), fx, fy, fz, ux, uy, uz);
      }
    }


    //
    // ClearRecords
    //
    // Clears all records for known sound effects
    //
    void ClearRecords()
    {
      if (initialized)
      {
        Cache::Clear();
        Record::ReleaseAll();
      }
    }


    //
    // SetVolume
    //
    // Set the digital audio volume (0..1)
    //
    void SetVolume(F32 volume)
    {
      if (initialized && driver)
      {
        AIL_set_digital_master_volume(driver, ConvertVolume(volume));
      }
    }


    //
    // Volume
    //
    // Get the current digital audio volume
    //
    F32 Volume()
    {
      if (initialized && driver)
      {
        return (F32(AIL_digital_master_volume(driver)) / F32(MAX_VOLUME));
      }

      return (0.0F);
    }


    //
    // CPUPercent
    //
    // Returns the percentage (0-100) of the cpu used by the digital driver
    //
    U32 CPUPercent()
    {
      return (initialized && driver ? AIL_digital_CPU_percent(driver) : 0);
    }

    
    //
    // SampleCount3D
    //
    // Returns the number of active 3D samples
    //
    S32 SampleCount3D()
    {
      return (initialized && provider ? AIL_active_3D_sample_count(provider->Handle()) : 0);
    }

    
    //
    // Stop
    //
    // Stop all effects, streams, and reserved voices
    //
    void Stop()
    {
      // Release reserved voices
      Reserved::Release();

      // Stop all streams
      Stream::Stop();

      // Stop output
      Output::Stop();
    }
  }
}

