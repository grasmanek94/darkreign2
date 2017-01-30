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
    // Namespace Reserved - Provides named reserved voices
    //
    namespace Reserved
    {
      ///////////////////////////////////////////////////////////////////////////////
      //
      // Class Voice - A single reserved voice 
      //
      class Voice
      {
      public:

        // The tree node
        NBinTree<Voice>::Node node;

      protected:

        // A single queue item
        class Item
        {
        public:

          // List node
          NList<Item>::Node node;

        protected:

          // The datafile for streams
          FileSys::DataFile *dataFile;

          // The mode of this item
          Mode mode;

        public:

          // Constructor
          Item(const char *file, Mode mode) : mode(mode)
          {
            // Open the file
            dataFile = FileSys::Open(file);
          }

          // Destructor
          ~Item()
          {
            if (dataFile)
            {
              FileSys::Close(dataFile);
            }
          }

          // Get the datafile, or NULL if not found
          FileSys::DataFile * GetDataFile()
          {
            return (dataFile);
          }

          // Get the mode
          Mode GetMode()
          {
            return (mode);
          }

          // Does this items file match the given one
          Bool SameFile(const char *file)
          {
            return (dataFile && !Utils::Stricmp(dataFile->Name(), file));
          }
        };

      protected:

        // Sample handle
        HSAMPLE handle;

        // The currently active item
        Item *active;

        // The list of queued items
        NList<Item> queue;

      protected:

        //
        // Playing
        //
        // Is the voice currently playing the active item
        //
        Bool Playing()
        {
          return (active && handle && (AIL_sample_status(handle) != SMP_DONE));
        }


        //
        // DisposeActiveItem
        //
        // Dispose of the active item
        //
        void DisposeActiveItem()
        {
          if (active)
          {
            ASSERT(!Playing())

            delete active;
            active = NULL;
          }
        }


        //
        // RemoveFromQueue
        //
        // Remove any items that do not have the given mode
        //
        void RemoveFromQueue(Mode keep)
        {
          NList<Item>::Iterator i(&queue);
          Item *item;

          while ((item = i++) != NULL)
          {
            if (item->GetMode() != keep)
            {
              queue.Dispose(item);
            }
          }
        }
        
        
        //
        // Find
        //
        // Is the given file playing or in the queue
        //
        Item * Find(const char *file)
        {
          // Is it the active file
          if (active && active->SameFile(file))
          {
            return (active);
          }

          // Is it in the queue
          for (NList<Item>::Iterator i(&queue); *i; ++i)
          {
            if ((*i)->SameFile(file))
            {
              return (*i);
            }
          }

          return (NULL);
        }


        //
        // Play
        //
        // Play the given item
        //
        void Play(Item *item)
        {
          ASSERT(item)
          ASSERT(!Playing())

          // Has the voice been setup with a sample
          Bool setup = FALSE;

          // Get the datafile from the item
          if (FileSys::DataFile *d = item->GetDataFile())
          {
            // Initialize the voice
            AIL_init_sample(handle);

            // Is this an mp3 stream
            if (Utils::Strstr(d->Name(), ".mp3"))
            {
              // Set the data file
              if (AIL_set_named_sample_file(handle, ".mp3", d->GetMemoryPtr(), d->Size(), 0))
              {
                // Voice is now setup
                setup = TRUE;
              }
            }
            else
            {
              // Start it playing
              if (AIL_set_sample_file(handle, d->GetMemoryPtr(), 0))
              {
                // Voice is now setup
                setup = TRUE;
              }
            }
          }

          // Was the voice setup
          if (setup)
          {
            // Set the volume
            AIL_set_sample_volume(handle, ConvertVolume(Volume()));

            // Start the sample playing
            AIL_start_sample(handle);

            // Set this as the active item
            active = item;          
          }
          else
          {
            // Should we have been able to play
            if (item->GetDataFile())
            {
              LOG_WARN(("Failed to play sound file [%s]", item->GetDataFile()->Name()));
            }

            // Dispose of the item
            delete item;
          }
        }


        //
        // StopActive
        //
        // Stop and release any active sample
        //
        void StopActive()
        {
          if (Playing())
          {
            AIL_end_sample(handle);
          }

          DisposeActiveItem();  
        }


        //
        // Queue
        //
        // Queue the given sample
        //
        void Queue(const char *file, Mode mode)
        {
          // Create a new item
          queue.Append(new Item(file, mode));
        }


      public:

        //
        // Constructor
        //
        Voice() : active(NULL), queue(&Item::node), handle(NULL)
        {
          // Do we have access to 2D digital audio services
          if (HDIGDRIVER driver = GetDriver())
          {
            // Allocate a new voice handle
            if ((handle = AIL_allocate_sample_handle(driver)) != NULL)
            {
              AIL_init_sample(handle);
            }
          }
        }


        //
        // Destructor
        //
        ~Voice()
        {
          // Free up the voice before deleting items
          if (handle)
          {
            AIL_release_sample_handle(handle);
            handle = NULL;
          }

          // Dispose of any active item
          DisposeActiveItem();

          // Delete all items in the queue
          queue.DisposeAll();
        }

        
        // 
        // Submit
        //
        // Submit a sample to this voice
        //
        void Submit(const char *file, Mode mode)
        {
          // If we have a voice and not already playing this file
          if (handle && !Find(file))
          {
            switch (mode)
            {
              case MODE1:
              {
                // Remove all but MODE1 items
                RemoveFromQueue(MODE1);

                // Queue the sample
                Queue(file, mode);

                break;
              }

              case MODE2:
              {
                // Ignore if there is a MODE1 item playing
                if (!Playing() || (active->GetMode() != MODE1))
                {
                  // Queue the sample
                  Queue(file, mode);
                }
                break;
              }

              case MODE3:
              {
                // Ignore if there is anything playing
                if (!Playing())
                {
                  // Queue the sample
                  Queue(file, mode);
                }
                break;
              }

              case MODE4:
              {
                // Stop anything else playing
                StopActive();

                // Queue the sample
                Queue(file, mode);
                break;
              }
            }
          }
        }


        //
        // Poll
        // 
        // Process the queue
        //
        void Poll()
        {
          // Is the voice available
          if (!Playing())
          {
            // Dispose of any active item
            DisposeActiveItem();

            // Remove the first item in the queue
            if (Item *item = queue.UnlinkHead())
            {
              // Play it
              Play(item);
            }
          }
        }
      };

      
      ///////////////////////////////////////////////////////////////////////////////
      //
      // System Functions
      //

      // Is the system initialized
      static Bool initialized = FALSE;

      // Tree of all existing reserved voices
      static NBinTree<Voice> voices(&Voice::node);

      // The reserved voice volume modifier
      static F32 volumeModifier;


      //
      // GetVoice
      //
      // Get the given reserved voice item
      //
      static Voice & GetVoice(const char *ident)
      {
        // Get the crc of the identifier
        U32 crc = Crc::CalcStr(ident);

        // Does the voice already exist
        Voice *v = voices.Find(crc);

        if (!v)
        {
          // Create a new item
          v = new Voice();

          // Add to the tree
          voices.Add(crc, v);
        }

        ASSERT(v)

        return (*v);
      }


      //
      // Init
      //
      // Initialize system
      //
      void Init()
      {
        ASSERT(!initialized)
        ASSERT(!voices.GetCount())

        // Set initial volume modifier
        volumeModifier = 1.0F;

        // System now initialized
        initialized = TRUE;
      }


      //
      // Done
      //
      // Shutdown system 
      //
      void Done()
      {
        ASSERT(initialized)

        // Release any voices
        Release();

        // System now shutdown
        initialized = FALSE;
      }


      //
      // Release
      //
      // Release all voices
      //
      void Release()
      {
        voices.DisposeAll();
      }


      //
      // Poll
      //
      // Poll all reserved voices
      //
      void Poll()
      {
        ASSERT(initialized)

        // Poll each voice
        for (NBinTree<Voice>::Iterator i(&voices); *i; ++i)
        {
          (*i)->Poll();
        }
      }


      //
      // SetVolume
      //
      // Set the volume modifier applied to all reserved voices
      //
      void SetVolume(F32 volume)
      {
        volumeModifier = volume;
      }


      //
      // GetVolume
      //
      // Get the current volume modifier
      //
      F32 Volume()
      {
        return (volumeModifier);
      }


      //
      // Submit
      //
      // Submit a sample to a reserved voice
      //
      void Submit(const char *voice, const char *file, Mode mode)
      {
        // Ignore if not initialized or no driver
        if (initialized && GetDriver() && !Disabled())
        {
          GetVoice(voice).Submit(file, mode);
        }
      }


      //
      // Reset
      //
      // Reset the given reserved voice
      //
      void Reset(const char *voice)
      {
        if (voice)
        {
          // Find the voice
          if (Voice *v = voices.Find(Crc::CalcStr(voice)))
          {
            voices.Dispose(v);
          }
        }
        else
        {
          voices.DisposeAll();
        }
      }
    }      
  }
}

