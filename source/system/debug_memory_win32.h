///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Debugging Tools
//
// 1-DEC-1997
//


#ifndef __DEBUG_MEMORY_WIN32
#define __DEBUG_MEMORY_WIN32


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "system.h"
#include "debug_memory.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Debug
//
namespace Debug
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Memory
  //
  namespace Memory
  {
    // Private cache template, pass in the pointer for the cache to use
    // Cache must be statically initialized to NULL!
    // CritSect must be initalized
    template <void **CACHE, System::CritSect *CRIT> class PrivSafeCache : public Cache
    {
    public:

      // new operator 
      void * operator new(size_t size)
      {
        void * data;
        CRIT->Wait();
        data = Alloc(*CACHE, size);
        CRIT->Signal();
        return (data);
      }

      // delete operator
      void operator delete(void *data)
      {
        CRIT->Wait();
        Free(*CACHE, data);
        CRIT->Signal();
      }

    };
  }
}

#endif