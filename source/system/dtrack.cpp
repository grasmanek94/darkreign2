///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Object Death Tracking System
//
// 4-MAY-1998
//

#include "dtrack.h"



///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// When less than this many items free, a new block will be allocated
#define DTRACK_FREELIMIT    32

// Initial id value (all below this are reserved)
#define DTRACK_INITIALID    1


// Check free limit value
#if (DTRACK_FREELIMIT <= 0)
  #error Invalid free limit value
#endif


///////////////////////////////////////////////////////////////////////////////
//
// Class DTrack - Tracks the creation and destruction of items
//

//
// DTrack::CacheAdd
//
// Add new position to cache (returns TRUE when full)
//
Bool DTrack::CacheAdd(U32 *trackPos)
{
  ASSERT(trackPos);
  ASSERT(*trackPos == DTRACK_EMPTY);

  // Do we have room to add it to the cache
  if (cacheCount < cacheSize)
  {
    cache[cacheCount] = trackPos;
    cacheCount++;
  }

  ASSERT(cacheCount <= cacheSize);

  // Return true if cache now full
  return (cacheCount == cacheSize);
}


//
// DTrack::AllocateNewBlock
//
// Allocate a new block and fill cache
//
void DTrack::AllocateNewBlock()
{
  // Allocate new block
  Block *newBlock = new Block(blockSize);

  // Append to block list
  blocks.Append(newBlock);

  // Do not try and add if cache is full
  Bool cacheFull = FALSE;

  // Step through each new item
  for (U32 i = 0; i < blockSize; i++)
  {
    // Flag it as empty
    newBlock->data[i] = DTRACK_EMPTY;

    // Add new position to cache
    if (!cacheFull)
    {
      cacheFull = CacheAdd(&newBlock->data[i]);
    }
  }
}


//
// DTrack::HitCache
//
// Get an available position from the cache (never fails)
//
U32* DTrack::HitCache()
{
  // Is there an item in the cache
  if (cacheCount)
  {
    // This is a cache hit
    stateInfo.cacheHits++;
  }
  else
  {
    ASSERT(blocks.GetCount() * blockSize >= stateInfo.items);

    // Is the remaining number of items below the free limit
    if ((blocks.GetCount() * blockSize) - stateInfo.items < DTRACK_FREELIMIT)
    {
      // So allocate a new block
      AllocateNewBlock();
    }
    else
    {
      // This is a cache miss
      stateInfo.cacheMisses++;

      // Stop scanning when cache is full
      Bool cacheFull = FALSE;

      // Scan each block
      for (NList<Block>::Iterator b(&blocks); *b && !cacheFull; b++)
      {
        // Check each item in block
        for (U32 i = 0; i < blockSize && !cacheFull; i++)
        {
          // Is this position empty
          if ((*b)->data[i] == DTRACK_EMPTY)
          {
            // Add to cache
            cacheFull = CacheAdd(&(*b)->data[i]);
          }
        }
      }
    }
  }

  // Must be positions in cache now
  ASSERT(cacheCount);

  // Return top item
  return (cache[--cacheCount]);
}


//
// DTrack::RegisterConstructionPrivate
//
// Private method for registering item creation
//
void DTrack::RegisterConstructionPrivate(Info &info, U32 id)
{
  // Get an available position
  U32 *newPos = HitCache();

  ASSERT(newPos);
  ASSERT(blocks.GetCount() > 0);
  ASSERT(*newPos == DTRACK_EMPTY);

  // Set the id
  info.id = id;

  // Store the position
  info.trackPos = newPos;

  // Store the id at current block position
  *newPos = info.id;

  // Increase number of items
  stateInfo.items++;

  // Increase id allocation counter
  if (info.id >= idCount)
  {
    // Check for wrap-around
    if (info.id == U32_MAX)
    {
      LOG_WARN(("Id allocator for tracker '%s' has wrapped!", name.str));

      // Set id count past reserved space
      idCount = DTRACK_INITIALID;
    }
    else
    {
      // Increase the current count
      idCount = info.id + 1;
    }
  }

  // Update max number of items
  if (stateInfo.items > stateInfo.maxItems)
  {
    stateInfo.maxItems = stateInfo.items;
  }
}

  
//
// DTrack::DebugDestruction
//
// Debug routine called on destruction
//
void DTrack::DebugDestruction()
{
  // Do some debugging checks
  U32 count = 0;

  // Scan each block
  for (NList<Block>::Iterator b(&blocks); *b; b++)
  {
    // Check each item
    for (U32 i = 0; i < blockSize; i++)
    {
      if ((*b)->data[i] != DTRACK_EMPTY)
      {
        count++;
      }
    }
  }

  // Show diag if any items still registered
  if (count)
  {
    LOG_DIAG(("DTrack '%s' still had %d/%d registered items at shutdown", name.str, count, stateInfo.items));
  }

  // Log some performance diagnostics
  /*
  LOG_DIAG
  ((
    "DTrack %s c%d/%d b%d max%d i%d mem%d", name.str, stateInfo.cacheHits, 
    stateInfo.cacheMisses, blocks.GetCount(), stateInfo.maxItems, idCount,
    (blocks.GetCount() * blockSize * sizeof(U32)) + (cacheSize * sizeof(U32*))
  ));
  */
}
  

//
// DTrack::DTrack
//
// Constructor
//
DTrack::DTrack(const char *nameIn, U32 blockSizeIn, U32 cacheSizeIn)
{
  // Setup tracker parameters
  name = nameIn;
  blockSize = blockSizeIn;

  // Should we automatically calculate the cachesize
  if (!cacheSizeIn)
  {
    // Having cache at same size as block seems to give good performance
    cacheSize = blockSize;
  }
  else
  {
    // Accept supplied value
    cacheSize = cacheSizeIn; 
  } 

  // Initialize the block list
  blocks.SetNodeMember(&Block::node);

  // Allocate cache
  cache = new U32*[cacheSize];

  // Set initial id count past reserved space
  idCount = DTRACK_INITIALID;

  // No items in the cache
  cacheCount = 0;

  // Kick-start the system with one block
  AllocateNewBlock();
}


//
// DTrack::~DTrack
//
// Destructor
//
DTrack::~DTrack()
{
  // Do debug checks and log diagnostics
  DebugDestruction();
  
  // Delete each block
  blocks.DisposeAll();

  // Delete the cache
  delete [] cache;
}


//
// DTrack::RegisterConstruction
//
// Register the creation of an item
//
void DTrack::RegisterConstruction(Info &info)
{
  // Call private method using current id value
  RegisterConstructionPrivate(info, idCount);
}


//
// DTrack::RegisterConstruction
//
// Register the construction of an item using the supplied id
//
void DTrack::RegisterConstruction(Info &info, U32 claimId)
{
  // Do error checking in a debug build
  #ifdef DEVELOPMENT

  // Is a valid id
  if (claimId < DTRACK_INITIALID)
  {
    LOG_ERR(("Tracker '%s' caught an invalid id claim (%d)", name.str, claimId));
    RegisterConstruction(info);
    return;
  }

  // Scan each block
  for (NList<Block>::Iterator b(&blocks); *b; b++)
  {
    // Check each item
    for (U32 i = 0; i < blockSize; i++)
    {
      if ((*b)->data[i] == claimId)
      {
        LOG_ERR(("Tracker '%s' Duplicate id (%d) in slot %d", name.str, claimId, i));
        RegisterConstruction(info);
        return;
      }
    }
  }

  #endif

  // Call private method using supplied id
  RegisterConstructionPrivate(info, claimId);
}


//
// DTrack::RegisterDestruction
//
// Register the destruction of the item 'info'
//
void DTrack::RegisterDestruction(Info &info)
{
  // Never registered, or has died - perhaps deleting twice?
  if (!info.IsSetup())
  {
    ERR_FATAL(("Invalid destruction : Item is NOT setup! (Tracker %s)", name.str));
  }
  
  // Different id, something bad has happened - perhaps bad memory?
  if (*info.trackPos != info.id)
  {
    ERR_FATAL(("Invalid destruction : Different Id (Tracker %s, %d, %d)", name.str, *info.trackPos, info.id));
  }

  // Set item as empty
  *info.trackPos = DTRACK_EMPTY;

  // Add to cache
  CacheAdd(info.trackPos);

  // Info is no longer setup
  info.Clear();

  ASSERT(stateInfo.items);

  // Decrease number of items
  stateInfo.items--;
}