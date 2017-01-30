///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Debugging Tools
//
// 1-DEC-1997
//


#ifndef __DEBUG_MEMORY_H
#define __DEBUG_MEMORY_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Debug
//
namespace Debug
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Memory
  //
  namespace Memory
  {

    // Initialize Memory System
    void Init();

    // Shutdown Memory System
    void Done();

    // InitMono
    void InitMono();

    // DoneMono
    void DoneMono();

    // DisplayMono
    void DisplayMono();

    // Check for leaks, print out stats etc
    void Check();

    // Flush cache
    void FlushCache();

    // Perform a snap shot
    void SnapShot();

    // Validate a pointer
    Bool ValidatePtr(void *data);

    // Validate the entire heap
    Bool ValidateAll();

    // Examine
    void Examine();

    // GetCodeIntegrity
    U32 GetCodeIntegrity(U32 crc);

    // Statistical Information
    U32 GetAllocatedBlocks();
    U32 GetAllocatedBytes();
    U32 GetOverheadBytes();
    U32 GetCacheBlocks();
    U32 GetCacheBytes();
    U32 GetMaxAllocatedBlocks();
    U32 GetMaxAllocatedBytes();
    U32 GetMaxOverheadBytes();
    U32 GetMaxCacheBlocks();
    U32 GetMaxCacheBytes();
    U32 GetTotalNewCalls();


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Aligning
    //
    class Aligning
    {
    public:

      // Allocate alligned memory
      static void * AligningAlloc(U32 size, U32 alignment);

      // Free some alligned memory
      static void AligningFree(void *);
      
    };


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Aligning
    //
    template <U32 ALIGNMENT> class Aligned : public Aligning
    {
    public:

      // new operator
      void * operator new(size_t size)
      {
        return (AligningAlloc(size, ALIGNMENT));
      }

      // delete operator
      void operator delete(void *mem)
      {
        AligningFree(mem);
      }

    };


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class UnCached
    //
    class UnCached
    {
    public:

      // Allocate some memory for a cache
      static void * Alloc(U32 size);

      // Free some memory for a cache
      static void Free(void *data);

      // new operator
      void * operator new(size_t size)
      {
        return (Alloc(size));
      }

      // delete operator
      void operator delete(void *data)
      {
        Free(data);
      }
    };

    
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Cache
    //
    class Cache
    {
    protected:

      // Allocate some memory for a cache
      static void * Alloc(void *cache, U32 size);

      // Free some memory for a cache
      static void Free(void * cache, void *data);
    };


    // Private cache template, pass in the pointer for the cache to use
    // Cache must be statically initialized to NULL!
    template <void **CACHE> class PrivCache : public Cache
    {
    public:

      // new operator
      void * operator new(size_t size)
      {
        return (Alloc(*CACHE, size));
      }

      // delete operator
      void operator delete(void *data)
      {
        Free(*CACHE, data);
      }

    };
  }
}

#endif