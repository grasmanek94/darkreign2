///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Object Death Tracking System
//
// 4-MAY-1998
//

#ifndef __DTRACK_H
#define __DTRACK_H


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Value of an empty item
#define DTRACK_EMPTY        0
#define DTRACK_INVALID      DTRACK_EMPTY


///////////////////////////////////////////////////////////////////////////////
//
// Class DTrack - Tracks the creation and destruction of unique items
//

class DTrack
{
public:

  // Holds information pointing to one DTrack item
  struct Info
  {
    // Id of the unique item
    U32 id;

    // Pointer to the id in the tracker array
    U32 *trackPos;

    // Clear this info
    void Clear()
    {
      id = DTRACK_EMPTY;
      trackPos = 0;
    }

    // Is this item setup
    Bool IsSetup() const
    {
      return (trackPos ? TRUE : FALSE);
    }

    // Constructor
    Info()
    {
      Clear();
    }
  };

private:

  // Holds system state information
  struct StateInfo
  {
    // Current number of items being tracked
    U32 items;

    // Maximum number of items ever tracked
    U32 maxItems;

    // Number of cache hits
    U32 cacheHits;

    // Number of cache misses
    U32 cacheMisses;

    // Constructor
    StateInfo()
    {
      cacheHits = cacheMisses = 0;
      items = maxItems = 0;
    }
  };

  // A single death track block
  struct Block
  {
    // Node for block list
    NList<Block>::Node node;

    // Block data
    U32 *data;

    // Constructor
    Block(U32 size)
    {
      data = new U32[size];
    }

    // Destructor
    ~Block()
    {
      delete [] data;
    }
  };

private:
 
  // Name of this object
  StrBuf<64> name;

  // Requested block size
  U32 blockSize;

  // Size of cache
  U32 cacheSize;

  // Current unique id count
  U32 idCount;

  // Pointers to allocated blocks
  NList<Block> blocks;

  // Cache of available positions
  U32** cache;

  // Number of items in the cache
  U32 cacheCount;

  // Information about the system state 
  StateInfo stateInfo;

  // Add new position to cache (returns TRUE if full)
  Bool CacheAdd(U32 *pos);

  // Allocate a new block and fill cache
  void AllocateNewBlock();

  // Get an available position from the cache
  U32* HitCache();

  // Private method for registering item creation
  void RegisterConstructionPrivate(Info &info, U32 id);

  // Debug routine called on destruction
  void DebugDestruction();

public:

  // Constructor - if cacheSizeIn is zero, automatically calculate size
  DTrack(const char *nameIn, U32 blockSizeIn, U32 cacheSizeIn = 0);
  
  // Destructor
  ~DTrack();

  // Register the construction of an item
  void RegisterConstruction(Info &info);

  // Register the construction of an item using the supplied id
  void RegisterConstruction(Info &info, U32 claimId);

  // Register the destruction of the item 'info'
  void RegisterDestruction(Info &info);
};



///////////////////////////////////////////////////////////////////////////////
//
// Class Reaper - Used to point to an object registered with a Death Tracker
//
// NOTE: Assumes DATA contains an Info member called 'dTrack'
//

template <class DATA> class Reaper
{
private:

  // Item information
  DTrack::Info dTrack;

  // Pointer to target object
  DATA *ptr;

public:

  // Clear this pointer
  void Clear()
  {
    dTrack.Clear();
    ptr = NULL;
  }

  // Constructor
  Reaper()
  {
    Clear();
  }

  // Copy constructor
  Reaper(DATA *target)
  {
    Setup(target);
  }

  // Point this reaper at object 'target', which can be NULL
  void Setup(DATA *target)
  {
    if (target)
    {
      // Has this object been setup
      if (!target->dTrack.IsSetup())
      {
        // Should NEVER point at a dead object
        ERR_FATAL(("Attempt to point a reaper at a dead object"));
      }

      // Copy death track information
      dTrack = target->dTrack;

      // Save pointer to target
      ptr = target;
    }
    else
    {
      Clear();
    }
  }

  // True if the target object is still alive
  Bool Alive()
  {
    // Is this info valid
    if (dTrack.IsSetup())
    {
      // Are the id's the same
      if (*dTrack.trackPos == dTrack.id)
      {
        // Then the item is still alive
        return (TRUE);
      }
      
      // Item is dead, so optimize subsequent calls
      dTrack.Clear();
    }

    // Info was never setup or has died
    return (FALSE);
  }

  // Const version of the alive test
  Bool Alive() const
  {
    // Is this info valid
    if (dTrack.IsSetup())
    {
      // Are the id's the same
      if (*dTrack.trackPos == dTrack.id)
      {
        // Then the item is still alive
        return (TRUE);
      }
    }

    // Info was never setup or has died
    return (FALSE);
  }

  // Returns TRUE if the reaper is not alive
  Bool Dead()
  {
    return (!Alive());
  }

  // Const version of the above method
  Bool Dead() const
  {
    return (!Alive());
  }

  // Assignment (can assign NULL)
  Reaper<DATA>& operator=(DATA *target)
  {
    Setup(target);
    return (*this);
  }

  // Return target pointer, regardless of alive/dead/cleared
  DATA* GetData() const
  {
    return (ptr);
  }

  // If reaper alive, return pointer, otherwise NULL
  DATA* GetPointer()
  {
    return (Alive() ? ptr : NULL);
  }

  // Const version of above method
  DATA* GetPointer() const
  {
    return (Alive() ? ptr : NULL);
  }

  // Return target pointer
  DATA* operator->()
  {
    ASSERT(Alive());
    return (ptr);
  }

  // Return target pointer
  DATA* operator->() const
  {
    ASSERT(Alive());
    return (ptr);
  }

  // Return target pointer
  operator DATA*()
  {
    ASSERT(Alive());
    return (ptr);
  }

  // Return target pointer
  operator DATA*() const
  {
    ASSERT(Alive());
    return (ptr);
  }

  // Returns the id of the object
  U32 Id()
  {
    ASSERT(Alive());
    return (dTrack.id);
  }

  // Returns the id, regardless of alive/dead/cleared
  U32 DirectId() const
  {
    return (dTrack.id);
  }

  // Save an id so this reaper can be resolved later
  void SetResolveId(U32 id)
  {
    ASSERT(!Alive());
    ASSERT(id != DTRACK_EMPTY);

    dTrack.id = id;
  }

  // Returns TRUE if this reaper has a resolve id set
  Bool HasResolveId()
  {
    // If we're not alive, but we have a non-empty id
    return (!Alive() && dTrack.id != DTRACK_EMPTY);
  }

  // Returns the id set by the above method
  U32 GetResolveId()
  {
    ASSERT(!Alive());
    ASSERT(dTrack.id != DTRACK_EMPTY);

    return (dTrack.id);
  }

};


#endif
