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
    // Namespace Cache - Manages actual effect data
    //
    namespace Cache
    {
      //
      // Static data
      // 
      static NList<Item> items(&Item::node);
      static U32 maxCacheSize;
      static U32 curCacheSize;
      static S32 actuallyUsed;
      static S32 cacheHits;
      static S32 cacheMisses;


      ///////////////////////////////////////////////////////////////////////////////
      //
      // Struct Item - A single cache item
      //

      //
      // Constructor
      //
      Item::Item(Record *record, void *data) : record(record), data(data)
      {
        items.Append(this);

        // Update cache size
        curCacheSize += record->fastFind->Size();
      }

      
      //
      // Destructor
      //
      Item::~Item()
      {
        ASSERT(curCacheSize - record->fastFind->Size() >= 0);

        curCacheSize -= record->fastFind->Size();
        items.Unlink(this);
        AIL_mem_free_lock(data);
      }



      ///////////////////////////////////////////////////////////////////////////////
      //
      // System Functions
      //

      //
      // FlushItem
      //
      // Removes the least used sound effect from the cache.
      //
      static Bool FlushItem()
      {
        Item *least = NULL;

        // Step through the cache
        for (NList<Item>::Iterator i(&items); *i; i++)
        {
          // Is this less used than the last one
          if (!least || ((*i)->record->freq < least->record->freq))
          {
            // Must not be currently in use
            if (!Output::RecordInUse((*i)->record))
            {
              least = *i;
            }
          }
        }

        // If we found one
        if (least) 
        {
          delete least;
          return (TRUE);
        }

        return (FALSE);
      }


      //
      // Init
      //
      // Initialize the cache
      //
      void Init()
      {
        maxCacheSize = DEF_MAXCACHESIZE;
        curCacheSize = 0;
        actuallyUsed = 0; 
        cacheHits = 0;
        cacheMisses = 0;
      }


      //
      // Done
      //
      // Shutdown the cache
      //
      void Done()
      {
        Clear();
      }


      //
      // Clear
      //
      // Clear all items
      //
      void Clear()
      {
        // Stop all voices before clearing cache
        Output::Stop();

        // Dispose of all cache items
        NList<Item>::Iterator i(&items);
        Item *item;

        while ((item = i++) != NULL)
        {
          delete item;
        }
      }


      //
      // Request
      //
      // Request sound effect data from the cache
      //
      Item * Request(Record *record)
      {
        ASSERT(Initialized());
        ASSERT(record);
        ASSERT(record->valid);
        ASSERT(record->fastFind);

        // Is this effect too large for the cache
        if (record->fastFind->Size() > maxCacheSize)
        {
          LOG_WARN(("File '%s' is too large for the cache", record->Name()));
          record->valid = FALSE;
          return (NULL);      
        }

        // Record if first time being played
        if (!record->freq)
        {
          actuallyUsed++;
        }

        // Increment the frequency of this effect
        record->freq++;

        // Step through each cache item
        for (NList<Item>::Iterator i(&items); *i; i++)
        {
          // Is this the one we're after
          if ((*i)->record == record)
          {
            cacheHits++;
            return (*i);
          }
        }

        // Record a cache miss
        cacheMisses++;
 
        // Make space for this new effect (will also shrink cache if max size decreased)
        while (items.GetCount() && (maxCacheSize - curCacheSize < record->fastFind->Size()))
        {
          // Try and remove a sound effect out of the cache
          if (!FlushItem())
          {
            // Unable to remove any effects
            return (NULL);
          }
        }

        ASSERT(maxCacheSize - curCacheSize >= record->fastFind->Size());

        // Open the file
        FileSys::DataFile *dFile = FileSys::Open(record->fastFind);

        // Should be found since already checked
        if (!dFile)
        {
          LOG_WARN(("File '%s' has vanished!", record->Name()));
          record->valid = FALSE;
          return (NULL);
        }

        // Let's just be extra careful, someone might swap data files on a server
        if (dFile->Size() != record->fastFind->Size())
        {
          LOG_WARN(("File '%s' has changed size!", record->Name()));
          record->valid = FALSE;
          return (NULL);
        }
  
        // Allocate memory for file data
        void *data = AIL_mem_alloc_lock(record->fastFind->Size());

        if (!data)
        {
          return (NULL);
        }

        // Read the data from disk
        if (dFile->Read(data, record->fastFind->Size()) != record->fastFind->Size())
        {
          // Free the memory we just allocated
          AIL_mem_free_lock(data);

          // Close the file
          FileSys::Close(dFile);

          // We won't try this one again
          record->valid = FALSE;

          LOG_WARN(("Error reading effect '%s' into cache", record->Name()));
          return (NULL);
        }

        // Close the file
        FileSys::Close(dFile);

        // Return a new cache item
        return (new Item(record, data));
      }
    }
  }
}

