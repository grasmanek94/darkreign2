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
    /////////////////////////////////////////////////////////////////////////////
    //
    // NameSpace Stream - Streaming services (mp3s)
    //
    namespace Stream
    {
      /////////////////////////////////////////////////////////////////////////////
      //
      // Struct StreamItem - Data for a single stream
      //
      struct StreamItem
      {
        // Stream identifier
        StrCrc<32> ident;

        // Handle to the voice being used
        HSAMPLE handle;

        // Data file for the stream
        FileSys::DataFile *file;

        // Tree node
        NBinTree<StreamItem>::Node node;

        // Constructor
        StreamItem(const char *ident, HSAMPLE handle, FileSys::DataFile *file) : 
          ident(ident), 
          handle(handle),
          file(file)
        {
          ASSERT(handle)

          // Initialise the voice
          AIL_init_sample(handle);

          // Set the data file
          if (AIL_set_named_sample_file(handle, const_cast<char*>(file->Name()), file->GetMemoryPtr(), file->Size(), 0))
          {
            // Start the stream playing
            AIL_start_sample(handle);
          }
        }

        // Destructor
        ~StreamItem()
        {
          // Release the voice
          AIL_release_sample_handle(handle);

          // Close the file
          FileSys::Close(file);
        }

        // Is the stream currently playing
        Bool IsPlaying()
        {
          return ((AIL_sample_status(handle) != SMP_DONE));
        }
      };


      /////////////////////////////////////////////////////////////////////////////
      //
      // System Functions
      //

      // Active streams
      static NBinTree<StreamItem> streams(&StreamItem::node);


      //
      // Purge
      //
      // Delete all stopped streams
      //
      static void Purge()
      {
        NBinTree<StreamItem>::Iterator i(&streams); 
        StreamItem *item;

        while ((item = i++) != NULL)
        {
          // Has this stream finished playing
          if (!item->IsPlaying())
          {
            streams.Dispose(item);
          }
        }
      }


      //
      // Start streaming a file
      //
      Bool Start(const char *ident, const char *file)
      {
        if (Initialized() && !Disabled())
        {
          if (HDIGDRIVER driver = GetDriver())
          {
            // Purge finished streams
            Purge();

            // Stop stream if already playing
            Stop(ident);

            // Open the file
            if (FileSys::DataFile *dataFile = FileSys::Open(file))
            {
              // Allocate a new voice handle
              if (HSAMPLE handle = AIL_allocate_sample_handle(driver))
              {
                // Allocate a stream item
                StreamItem *item = new StreamItem(ident, handle, dataFile);

                // Add to the tree
                streams.Add(item->ident.crc, item);
              }
              else
              {
                FileSys::Close(dataFile);
              }

              // File was found
              return (TRUE);
            }

            // File not found
            return (FALSE);
          }
        }

        // Success by default
        return (TRUE);      
      }


      //
      // Stop any current stream
      //
      void Stop(const char *ident)
      {
        if (Initialized())
        {
          // Purge finished streams
          Purge();

          // Do we need to stop all streams
          if (ident == NULL)
          {
            streams.DisposeAll();
          }
          else
          {
            // Find and stop the stream
            if (StreamItem *item = streams.Find(Crc::CalcStr(ident)))
            {
              streams.Dispose(item);
            }
          }
        }
      }


      //
      // Returns TRUE if the stream is currently playing
      //
      Bool IsPlaying(const char *ident)
      {
        if (Initialized())
        {
          if (StreamItem *item = streams.Find(Crc::CalcStr(ident)))
          {
            return (item->IsPlaying());
          }
        }

        return (FALSE);
      }
    }
  }
}

