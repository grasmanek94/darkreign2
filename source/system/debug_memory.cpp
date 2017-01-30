///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Debug Memory Utilities
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "debug_memory_win32.h"
#include "debug_win32.h"
#include "dlgtemplate.h"
#include "debug_symbol.h"

#include "version.h"
#include "file.h"
#include "random.h"

#include <commctrl.h>
#include <tlhelp32.h>


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Use this define to turn the whole memory system on or off
#define DEBUG_MEMORY
#pragma message("DEBUG MEMORY")


#if defined(DEVELOPMENT) || defined(SYNC_BRUTAL_ACTIVE)

  // Use this define to enable memory checking 
  #define DEBUG_MEMORY_CHECKING
  #pragma message("DEBUG MEMORY CHECKING")

  // Use this define to enable general memory statistics
  #define DEBUG_MEMORY_STATS
  #pragma message("DEBUG MEMORY STATS")

  // Use this to enable snapshot report dialog
  //#define DEBUG_MEMORY_SNAPSHOT
  //#pragma message("DEBUG MEMORY SNAPSHOT")
#endif

// Use this define to enable memory caching (faster)
#define DEBUG_MEMORY_CACHING
#pragma message("DEBUG MEMORY CACHING")

// Make the memory system thread safe
#define DEBUG_MEMORY_THREAD_SAFE
#pragma message("DEBUG MEMORY THREAD SAFE")

// Use this define to ensure multiple threads aren't using the heap
//#define DEBUG_MEMORY_TEST_HEAP_SAFETY
//#pragma message("DEBUG MEMORY TEST HEAP SAFETY")

// Use this define to enable use of the global heap (slower)
//#define DEBUG_MEMORY_USE_GLOBAL
//#pragma message("DEBUG MEMORY GLOBAL HEAP")

#ifdef DEVELOPMENT
  // Use this define to enable memory cache statistics
  #define DEBUG_MEMORY_CACHE_STATS
  #pragma message("DEBUG MEMORY CACHE STATS")

#endif

// Use this define to test for NULL when freeing memory
#define DEBUG_MEMORY_TEST_FREE
#pragma message("DEBUG MEMORY TEST FREE")

// Use this define to LOG when freeing NULL memory
//#define DEBUG_MEMORY_LOG_FREE
//#pragma message("DEBUG MEMORY LOG FREE")

// Use this define to ERR_MEM when freeing NULL memory
//#define DEBUG_MEMORY_ERR_FREE
//#pragma message("DEBUG MEMORY ERR FREE")

//#define DEBUG_MEMORY_TEST_INIT
//#pragma message("DEBUG MEMORY TEST INIT")

// Use this to use the protected memory scheme 
// This disables the caching system
//#define DEBUG_MEMORY_PROTECTED
//#pragma message("DEBUG MEMORY PROTECTED")

// What to fill various memory blocks with
#ifdef SYNC_BRUTAL_ACTIVE

  static U8 DEBUG_MEMORY_GUARD = 0xFD;
  #pragma message("DEBUG MEMORY GUARD : Random")
  static U8 DEBUG_MEMORY_FREE  = 0xDD;
  #pragma message("DEBUG MEMORY FREE : Random")
  static U8 DEBUG_MEMORY_INIT  = 0xCD;
  #pragma message("DEBUG MEMORY INIT : Random")

#else

  #define DEBUG_MEMORY_GUARD 0xFD
  #pragma message("DEBUG MEMORY GUARD : 0xFD")
  #define DEBUG_MEMORY_FREE 0xDD
  #pragma message("DEBUG MEMORY FREE : 0xDD")
  #define DEBUG_MEMORY_INIT 0xCD
  #pragma message("DEBUG MEMORY INIT : 0xCD")

#endif

// Number of slots in the cache and the maximum size of cachable data
#define DEBUG_MEMORY_CACHE_SLOTS        16
#pragma message("DEBUG MEMORY CACHE SLOTS : 16")
#define DEBUG_MEMORY_CACHE_GRANULARITY  5
#pragma message("DEBUG MEMORY CACHE GRANULARITY : 5")
#define DEBUG_MEMORY_CACHE_MAX_SIZE (DEBUG_MEMORY_CACHE_SLOTS << DEBUG_MEMORY_CACHE_GRANULARITY)

// Memory Macros
#define DEBUG_MEMORY_CALLSTACK_DEPTH 6
#pragma message("DEBUG MEMORY CALLSTACK DEPTH : 6")
#define DEBUG_MEMORY_HEAD_GUARD_SIZE 8
#pragma message("DEBUG MEMORY HEAD GUARD SIZE : 8")
#define DEBUG_MEMORY_TAIL_GUARD_SIZE 4
#pragma message("DEBUG MEMORY TAIL GUARD SIZE : 4")

// Maximum memory dump size
#define DEBUG_MEMORY_DUMP_MAX 32


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

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Struct Module
    //
    struct Module
    {
      // Name of the module
      StrBuf<32> name;

      // Size of the module
      U32 size;

    };


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Struct ProcessStats
    //
    struct ProcessStats
    {
      U32 totalPrivate;

      U32 totalFixed;
      U32 totalMoveable;
      U32 totalFree;

      U32 numHeaps;

      Module modulesPrivate[22];
      U32 moduleNumPrivate;
      U32 moduleMemoryPrivate;

      Module modulesShared[22];
      U32 moduleNumShared;
      U32 moduleMemoryShared;
    };


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Struct BlockHead
    //
    struct BlockHead
    {
      // Pointer to next block
      BlockHead *next;

      // Pointer to previous block
      BlockHead *prev;

      // Size of the memory allocated
      U32 size;

      // Callstack of Allocator
      U32 callStack[DEBUG_MEMORY_CALLSTACK_DEPTH];

      // Head Guard Block
      U8 headGuard[DEBUG_MEMORY_HEAD_GUARD_SIZE];
    };


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Struct BlockTail
    //
    struct BlockTail
    {
      // Tail Guard Block
      U8 tailGuard[DEBUG_MEMORY_TAIL_GUARD_SIZE];
    };


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Struct CacheStates
    //
    struct CacheStats
    {
      // number allocated ever
      U32 allocated;

      // number allocated now
      U32 allocatedCurrent;

      // maximum allocated at one time
      U32 allocatedMax;

      // cache hits
      U32 allocatedHits;
    };

    
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Struct InfoNode
    //
    struct InfoNode : UnCached
    {
      // call stack
      U32 callStack[DEBUG_MEMORY_CALLSTACK_DEPTH];

      // amount of memory
      U32 size;

      // list node
      NList<InfoNode>::Node node;

      InfoNode(BlockHead *block)
      {
        memcpy(callStack, block->callStack, sizeof (callStack));
        size = block->size;
      }
    };


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Struct UsageNode
    //
    struct UsageNode : public UnCached
    {
      // Address of this node
      U32 address;

      // Amount of memory at this node
      U32 size;

      // Number of blocks at this node
      U32 blocks;

      // Tree Node  
      NBinTree<UsageNode>::Node node;

      // Tree of children (by address)
      NBinTree<UsageNode> children;

      // Percentage of parent
      F32 cutParent;

      // Percentage of the whole lot
      F32 cutTotal;

      // Tree View Item
      HTREEITEM treeItem;

      // UsageNode
      UsageNode(U32 address) :
        address(address),
        size(0),
        blocks(0),
        children(&UsageNode::node)
      {
      }

      // ~UsageNode
      ~UsageNode()
      {
        children.DisposeAll();
      }

    };

    
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Struct NameNode
    //
    struct NameNode : public UnCached
    {
      String name;
      NBinTree<NameNode>::Node node;
    };


    //
    // Prototypes
    //
    void * CacheAlloc(U32 size);
    void CacheFree(void *data);

    void * ProtectedAlloc(U32 size);
    void ProtectedFree(void *data);

    void * MemoryAlloc(U32 size);
    void MemoryFree(void *data);
    U32  MemorySize(void *data);
    void MemoryValidate(void *data);

    Bool ValidateAll();
    Bool ValidateBlock(BlockHead *head);
    void DumpBlockInternals(BlockHead *head);
    void DumpBlock(BlockHead *head);

    #ifdef DEBUG_MEMORY_SNAPSHOT

    void SnapShotReport();
    void SnapShotUpdatePercentages(UsageNode *root, UsageNode *node);
    void SnapShotAddItemToTree(HWND tree, HTREEITEM item, UsageNode *node);
    const char *SnapShotGetName(U32 address);
    Bool CALLBACK SnapShotDlgProc(HWND hdlg, UINT msg, WPARAM, LPARAM lparam);

    #endif

    void CacheReport(int slot, U32 stat);

    Bool GetProcessMemory(ProcessStats &processStats);


    //
    // Internal Data
    //
    static Bool initialized = FALSE;
    static U32 allocatedBlocks = 0;
    static U32 allocatedBytes = 0; 
    static U32 overheadBytes = 0;
    static U32 cacheBytes = 0;
    static U32 cacheBlocks = 0;
    static U32 maxAllocatedBlocks = 0; 
    static U32 maxAllocatedBytes = 0;  
    static U32 maxOverheadBytes = 0;   
    static U32 maxCacheBlocks = 0;
    static U32 maxCacheBytes = 0;
    static U32 totalNewCalls = 0;
    static BlockHead *blocks;
    static HANDLE heap = 0;
    static void *memCache[DEBUG_MEMORY_CACHE_SLOTS];
    static CacheStats memCacheStats[DEBUG_MEMORY_CACHE_SLOTS];
    static U32 debugHeapThreadId;

    #ifdef DEBUG_MEMORY_SNAPSHOT

    static Bool snapShot = FALSE;
    static NList<InfoNode> snapShotInfo(&InfoNode::node);
    static NBinTree<NameNode> snapShotNames(&NameNode::node);

    #endif
  
    // Modules for displaying module useage
    static ProcessStats processStats;
    static Clock::Watch newSampler;
    static U32 prevNew = 0;

    // Mutex for making allocating/freeing memory thread safe
    #ifdef DEBUG_MEMORY_THREAD_SAFE

    static CRITICAL_SECTION memoryCriticalSection;

    #endif

    // Mono buffer for displaying memory information
    MonoBufDefStatic(monoBuffer);


    static enum
    {
      SNAPSHOT_TREE,
      SNAPSHOT_LIST
    };
    

    //
    // Init
    //
    // Initialize the Memory System
    // - Create Heap
    // - Reset Statistics
    //
    void Init()
    {
      ASSERT(!initialized)

      SYSTEM_INFO sysInfo;
      GetSystemInfo(&sysInfo);

      #ifdef SYNC_BRUTAL_ACTIVE

        // Initialise fill values with random data
        Random::Generator generator(GetTickCount());

        DEBUG_MEMORY_GUARD = (U8)generator.Integer(256);
        //DEBUG_MEMORY_FREE  = (U8)generator.Integer(256);
        DEBUG_MEMORY_INIT  = (U8)generator.Integer(256);

      #endif

      #ifdef DEBUG_MEMORY_THREAD_SAFE

        // Initialise the critical section
        // Can't rely on static instantiation of System::CriticalSection object if using init_seg
        InitializeCriticalSection(&memoryCriticalSection);

      #endif

      #ifdef DEBUG_MEMORY_USE_GLOBAL
        // Get the process heap
        heap = GetProcessHeap();
      #else
        // Allocate our very own heap
        heap = HeapCreate(HEAP_NO_SERIALIZE, 10000000, 0);
      #endif

      #ifdef DEBUG_MEMORY_TEST_HEAP_SAFETY
        // Save the thread current thread id
        debugHeapThreadId = System::Thread::GetCurrentId();
      #endif

      // Clear the memory cache
      memset(memCache, 0x00, sizeof (memCache));

      #ifdef DEBUG_MEMORY_CACHE_STATS

        U32 i;
        for (i = 0; i < DEBUG_MEMORY_CACHE_SLOTS; i++)
        {
          memCacheStats[i].allocated = 0;
          memCacheStats[i].allocatedCurrent = 0;
          memCacheStats[i].allocatedMax = 0;
          memCacheStats[i].allocatedHits = 0;
        }

      #endif

      Examine();

      initialized = TRUE;
    }


    //
    // Done
    //
    // Shutdown the Memory System 
    //
    void Done()
    {
      ASSERT(initialized)

      #ifndef DEBUG_MEMORY_USE_GLOBAL

        // Free memory resources
        HeapDestroy(heap);

      #endif

      #ifdef DEBUG_MEMORY_THREAD_SAFE

        DeleteCriticalSection(&memoryCriticalSection);

      #endif

      initialized = FALSE;
    }


    //
    // InitMono
    //
    void InitMono()
    {
      ASSERT(initialized)

      #ifdef SYNC_BRUTAL_ACTIVE

        // Log random memory fill values for diagnostic purposes
        LOG_DIAG(("DEBUG_MEMORY_GUARD = 0x%.2X", DEBUG_MEMORY_GUARD))
        LOG_DIAG(("DEBUG_MEMORY_FREE  = 0x%.2X", DEBUG_MEMORY_FREE))
        LOG_DIAG(("DEBUG_MEMORY_INIT  = 0x%.2X", DEBUG_MEMORY_INIT))

      #endif

      // Create mono buffers
      MonoBufCreate("Memory", &monoBuffer);

      // Write headings
      MonoBufWrite(monoBuffer,  0,  0, "Process Memory", Mono::BRIGHT);
      MonoBufWrite(monoBuffer,  1,  0, "Total Private", Mono::NORMAL);
      MonoBufWrite(monoBuffer,  2,  0, "Fixed", Mono::NORMAL);
      MonoBufWrite(monoBuffer,  3,  0, "Moveable", Mono::NORMAL);
      MonoBufWrite(monoBuffer,  4,  0, "Free", Mono::NORMAL);
      MonoBufWrite(monoBuffer,  5,  0, "Num Heaps", Mono::NORMAL);

      MonoBufWrite(monoBuffer,  6,  0, "Windows Memory", Mono::BRIGHT);
      MonoBufWrite(monoBuffer,  7,  0, "Memory Load", Mono::NORMAL);
      MonoBufWrite(monoBuffer,  8,  0, "Physical Used", Mono::NORMAL);
      MonoBufWrite(monoBuffer,  9,  0, "Physical Avail", Mono::NORMAL);
      MonoBufWrite(monoBuffer, 10,  0, "Page File Used", Mono::NORMAL);
      MonoBufWrite(monoBuffer, 11,  0, "Page Avail", Mono::NORMAL);
      MonoBufWrite(monoBuffer, 12,  0, "Virtual Used", Mono::NORMAL);
      MonoBufWrite(monoBuffer, 13,  0, "Virtual Free", Mono::NORMAL);

      MonoBufWrite(monoBuffer, 14,  0, "Internal Memory Statistics", Mono::BRIGHT);
      MonoBufWrite(monoBuffer, 15,  0, "Memory Blocks", Mono::NORMAL);
      MonoBufWrite(monoBuffer, 16,  0, "Memory Bytes", Mono::NORMAL);
      MonoBufWrite(monoBuffer, 17,  0, "Overhead Bytes", Mono::NORMAL);
      MonoBufWrite(monoBuffer, 18,  0, "Cache Blocks", Mono::NORMAL);
      MonoBufWrite(monoBuffer, 19,  0, "Cache Bytes", Mono::NORMAL);
      MonoBufWrite(monoBuffer, 20,  0, "In Use Bytes", Mono::NORMAL);
      MonoBufWrite(monoBuffer, 21,  0, "New calls", Mono::NORMAL);
      
      MonoBufWrite(monoBuffer, 22,  0, "Total Unknown", Mono::NORMAL);

      MonoBufWrite(monoBuffer, 23,  0, "Use CTRL+WIN+Z to update", Mono::BRIGHT);

      MonoBufWrite(monoBuffer,  0, 40, "Priv Modules", Mono::BRIGHT);
      MonoBufWrite(monoBuffer, 23, 40, "Shrd Modules", Mono::BRIGHT);
    }


    //
    // DoneMono
    //
    void DoneMono()
    {
      ASSERT(initialized)

      // Delete mono buffer
      MonoBufDestroy(&monoBuffer);
    }


    //
    // DisplayMono
    //
    void DisplayMono()
    {
      #define MEG(x) x, F32(x) / (1024.0f * 1024.0f)

      // Process Memory
      GetProcessMemory(processStats);
      MonoBufWriteV(monoBuffer, ( 1, 19, Mono::BRIGHT, "%10d [%7.2f]", MEG(processStats.totalPrivate)));
      MonoBufWriteV(monoBuffer, ( 2, 19, Mono::BRIGHT, "%10d [%7.2f]", MEG(processStats.totalFixed)));
      MonoBufWriteV(monoBuffer, ( 3, 19, Mono::BRIGHT, "%10d [%7.2f]", MEG(processStats.totalMoveable)));
      MonoBufWriteV(monoBuffer, ( 4, 19, Mono::BRIGHT, "%10d [%7.2f]", MEG(processStats.totalFree)));
      MonoBufWriteV(monoBuffer, ( 5, 19, Mono::BRIGHT, "%10d", processStats.numHeaps));
      
      // Windows Memory
      MEMORYSTATUS memory;
      GlobalMemoryStatus(&memory);
      MonoBufWriteV(monoBuffer, ( 7, 19, Mono::BRIGHT, "%10d", memory.dwMemoryLoad));
      MonoBufWriteV(monoBuffer, ( 8, 19, Mono::BRIGHT, "%10d [%7.2f]", MEG(memory.dwTotalPhys)));
      MonoBufWriteV(monoBuffer, ( 9, 19, Mono::BRIGHT, "%10d [%7.2f]", MEG(memory.dwAvailPhys)));
      MonoBufWriteV(monoBuffer, (10, 19, Mono::BRIGHT, "%10d [%7.2f]", MEG((memory.dwTotalPageFile - memory.dwAvailPageFile))));
      MonoBufWriteV(monoBuffer, (11, 19, Mono::BRIGHT, "%10d [%7.2f]", MEG(memory.dwAvailPageFile)));
      MonoBufWriteV(monoBuffer, (12, 19, Mono::BRIGHT, "%10d [%7.2f]", MEG((memory.dwTotalVirtual - memory.dwAvailVirtual))));
      MonoBufWriteV(monoBuffer, (13, 19, Mono::BRIGHT, "%10d [%7.2f]", MEG(memory.dwAvailVirtual)));

      // Internal Memory System
      newSampler.Sample(Debug::Memory::GetTotalNewCalls() - prevNew);
      MonoBufWriteV(monoBuffer, (15, 20, Mono::NORMAL, "%9d", Debug::Memory::GetAllocatedBlocks()));
      MonoBufWriteV(monoBuffer, (15, 30, Mono::BRIGHT, "%9d", Debug::Memory::GetMaxAllocatedBlocks()));
      MonoBufWriteV(monoBuffer, (16, 20, Mono::NORMAL, "%9d", Debug::Memory::GetAllocatedBytes()));
      MonoBufWriteV(monoBuffer, (16, 30, Mono::BRIGHT, "%9d", Debug::Memory::GetMaxAllocatedBytes()));
      MonoBufWriteV(monoBuffer, (17, 20, Mono::NORMAL, "%9d", Debug::Memory::GetOverheadBytes()));
      MonoBufWriteV(monoBuffer, (17, 30, Mono::BRIGHT, "%9d", Debug::Memory::GetMaxOverheadBytes()));
      MonoBufWriteV(monoBuffer, (18, 20, Mono::NORMAL, "%9d", Debug::Memory::GetCacheBlocks()));
      MonoBufWriteV(monoBuffer, (18, 30, Mono::BRIGHT, "%9d", Debug::Memory::GetMaxCacheBlocks()));
      MonoBufWriteV(monoBuffer, (19, 20, Mono::NORMAL, "%9d", Debug::Memory::GetCacheBytes()));
      MonoBufWriteV(monoBuffer, (19, 30, Mono::BRIGHT, "%9d", Debug::Memory::GetMaxCacheBytes()));
      MonoBufWriteV(monoBuffer, (20, 20, Mono::NORMAL, "%9d", Debug::Memory::GetAllocatedBytes() - Debug::Memory::GetOverheadBytes() - Debug::Memory::GetCacheBytes()));
      MonoBufWriteV(monoBuffer, (20, 30, Mono::BRIGHT, "%9d", Debug::Memory::GetMaxAllocatedBytes() - Debug::Memory::GetMaxOverheadBytes() - Debug::Memory::GetMaxCacheBytes()));
      MonoBufWriteV(monoBuffer, (21, 20, Mono::NORMAL, "%9d", newSampler.GetSmooth()));
      MonoBufWriteV(monoBuffer, (21, 30, Mono::BRIGHT, "%9d", Debug::Memory::GetTotalNewCalls()));

      MonoBufWriteV(monoBuffer, (22, 19, Mono::BRIGHT, "%10d [%7.2f]", MEG(processStats.totalFixed - (Debug::Memory::GetAllocatedBytes() - Debug::Memory::GetOverheadBytes() + Debug::Memory::GetCacheBytes()))));

      prevNew = Debug::Memory::GetTotalNewCalls();

      // Display modules
      MonoBufWriteV(monoBuffer, ( 0, 53, Mono::BRIGHT, "{%4d}", processStats.moduleNumPrivate));
      MonoBufWriteV(monoBuffer, ( 0, 61, Mono::BRIGHT, "%9d [%7.2f]", MEG(processStats.moduleMemoryPrivate)));

      for (U32 i = 0; i < processStats.moduleNumPrivate; i++)
      {
        MonoBufWriteV(monoBuffer, ( i + 1, 40, Mono::NORMAL, "%-20s %9d [%7.2f]", 
          processStats.modulesPrivate[i].name.str,
          MEG(processStats.modulesPrivate[i].size)));
      }
      for (; i < 22; i++)
      {
        MonoBufWrite(monoBuffer, i + 1, 40, "                                        ", Mono::NORMAL);
      }

      MonoBufWriteV(monoBuffer, (23, 53, Mono::BRIGHT, "{%4d}", processStats.moduleNumShared));
      MonoBufWriteV(monoBuffer, (23, 61, Mono::BRIGHT, "%9d [%7.2f]", MEG(processStats.moduleMemoryShared)));
    }


    //
    // Check
    //
    void Check()
    {
      // Don't check memory leaks if we are fuxored
      if (IsFuxored())
      {
        return;
      }

      ASSERT(initialized)

      #ifdef DEBUG_MEMORY_CACHING

        // Don't flush log while dumping memory
        Log::SetFlush(FALSE);

        // Print out memory cache stats
        U32 i;

        #ifdef DEBUG_MEMORY_CACHE_STATS

          LOG_DIAG(("MemSize          Alloc  Current      Max     Hits    Ratio    Dregs"))

          U32 totalAllocated = 0;
          U32 totalAllocatedCurrent = 0;
          U32 totalAllocatedMax = 0;
          U32 totalAllocatedHits = 0;
          U32 totalDregs = 0;

        #endif

        for (i = 0; i < DEBUG_MEMORY_CACHE_SLOTS; i++)
        {
          U32 stat = 0;

          void * ptr = memCache[i];

          while (ptr)
          {
            void * next;
            next = (void *) (*(U32 *) ptr);
            ptr = next;
            stat++;
          }
      
          #ifdef DEBUG_MEMORY_CACHE_STATS

            CacheReport(i, stat);

            totalAllocated += memCacheStats[i].allocated;
            totalAllocatedCurrent += memCacheStats[i].allocatedCurrent;
            totalAllocatedMax += memCacheStats[i].allocatedMax;
            totalAllocatedHits += memCacheStats[i].allocatedHits;
            totalDregs += stat;

          #endif

        }

        #ifdef DEBUG_MEMORY_CACHE_STATS

          LOG_DIAG(("[%05d-00001] %8d %8d %8d %8d %8f %8d", 
            DEBUG_MEMORY_CACHE_MAX_SIZE, 
            totalAllocated, 
            totalAllocatedCurrent, 
            totalAllocatedMax, 
            totalAllocatedHits, 
            (totalAllocatedHits + totalAllocatedMax) ? 
              ((F32) totalAllocatedHits) / ((F32) (totalAllocatedHits + totalAllocatedMax)) : 
              0,
            totalDregs))

        #endif

      #endif

      #ifdef DEBUG_MEMORY_CHECKING

        if (blocks)
        {
          // Write directly to video memory
          //MonoBufDef(hndlMono);
          //MonoGetClientPanel(hndlMono);

          // Activate the panel
          //MonoPanelChange(hndlMono);

          //MonoBufClear(hndlMono);

          BlockHead *head = blocks;
          F32 count = 0;
          U32 size = 0;

          #ifdef DEBUG_MEMORY_CACHE_STATS
            F32 blocks = (F32) (allocatedBlocks - totalDregs);
          #else
            F32 blocks = (F32) allocatedBlocks;
          #endif

          LOG_DIAG(("%.0f memory leaks", blocks))

          //MonoBufWriteV(hndlMono, (1, 0, Mono::BRIGHT, "There were %.0f unfreed blocks", blocks));
          //MonoBufWrite(hndlMono, 2, 0, "Writing allocators of unfreed blocks to the Log File", Mono::NORMAL);
          //MonoBufWrite(hndlMono, 4, 0, "Progress:", Mono::NORMAL);
          //MonoBufWrite(hndlMono, 5, 0, "Memory:", Mono::NORMAL);

          if (ValidateAll())
          {
            while (head)
            {
              count++;
              size += head->size;

              //MonoBufWriteV(hndlMono, (4, 10, Mono::BRIGHT, "%7.4f %%", count * 100 / blocks));
              //MonoBufWriteV(hndlMono, (5, 10, Mono::BRIGHT, "%d bytes", size));

              LOG_ERR(("%d bytes allocated by", head->size))
              CallStack::Display(head->callStack);

              // Dump memory contents
              LOG_ERR(("Memory contents"));
              Utils::MemoryDump((U8 *)head + sizeof(BlockHead), Min<U32>(head->size, DEBUG_MEMORY_DUMP_MAX));

              head = head->next;
            }
          }
        }

      #endif

      #ifdef DEBUG_MEMORY_STATS

        LOG_DIAG(("Max Memory Blocks  : %d", maxAllocatedBlocks))
        LOG_DIAG(("Max Memory Bytes   : %d", maxAllocatedBytes))
        LOG_DIAG(("Max Overhead Bytes : %d", maxOverheadBytes))

      #endif

      #ifdef DEBUG_MEMORY_SNAPSHOT

      // Was a snap shot taken ?
      if (snapShot)
      {
        SnapShotReport();
      }

      #endif
    }


    //
    // Flush the cache
    //
    void FlushCache()
    {
      #ifdef DEBUG_MEMORY_CACHING

      #ifdef DEBUG_MEMORY_THREAD_SAFE
        EnterCriticalSection(&memoryCriticalSection);
      #endif

        LOG_DIAG(("Flushing Memory Cache"))

        if (ValidateAll())
        {
          LOG_DIAG(("There were %d blocks %d bytes on the cache", cacheBlocks, cacheBytes))
          cacheBlocks = 0;
          maxCacheBlocks = 0;
          cacheBytes = 0;
          maxCacheBytes = 0;

          for (int i = 0; i < DEBUG_MEMORY_CACHE_SLOTS; i++)
          {
            U32 stat = 0;

            void * ptr = memCache[i];

            while (ptr)
            {
              void * next;
              next = (void *) (*(U32 *) ptr);

              // Free the memory
              MemoryFree(ptr);

              ptr = next;
              stat++;
            }

            #ifdef DEBUG_MEMORY_CACHE_STATS

              // Write Stats
              CacheReport(i, stat);

              // Reset some stats
              memCacheStats[i].allocated = 0;

            #endif

            // Clear the cache
            memCache[i] = NULL;
          }
        }

      #ifdef DEBUG_MEMORY_THREAD_SAFE
        LeaveCriticalSection(&memoryCriticalSection);
      #endif

      #endif
    }


    //
    // SnapShot
    //
    // Takes a snap shot of memory, when the program exits it displays memory usage statistics
    //
    void SnapShot()
    {
      #ifdef DEBUG_MEMORY_SNAPSHOT

      #ifdef DEBUG_MEMORY_CHECKING
        if (snapShot)
        {
          ERR_FATAL(("Snap Shot has already been taken"))
        }
        snapShot = TRUE;

        if (ValidateAll())
        {
          BlockHead *block;

          block = blocks;

          while (block)
          {
            snapShotInfo.Append(new InfoNode(block));                    
            block = block->next;
          }
        }
      #else
        ERR_FATAL(("Snap Shot is only supported if DEBUG_MEMORY_CHECKING is enabled"))
      #endif

      #endif
    }


    // Statistical Information

    //
    // GetAllocatedBlocks
    //
    // Return the number of allocated blocks
    //
    U32 GetAllocatedBlocks()
    {
      return (allocatedBlocks);
    }


    //
    // GetAllocatedBytes
    //
    // Return the number of allocated bytes
    //
    U32 GetAllocatedBytes()
    {
      return (allocatedBytes);
    }


    //
    // GetOverheadBytes
    //
    // Return the number of overhead bytes
    //
    U32 GetOverheadBytes()
    {
      return (overheadBytes);
    }


    //
    // GetCacheBlocks
    //
    // Return the number of bytes sitting in the cache
    //
    U32 GetCacheBlocks()
    {
      return (cacheBlocks);
    }


    //
    // GetCacheBytes
    //
    // Return the number of bytes sitting in the cache
    //
    U32 GetCacheBytes()
    {
      return (cacheBytes);
    }


    //
    // GetMaxAllocatedBlocks
    //
    // Return the maximum number of allocated blocks
    //
    U32 GetMaxAllocatedBlocks()
    {
      return (maxAllocatedBlocks);
    }


    //
    // GetMaxAllocatedBytes
    //
    // Return the maximum number of allocated bytes
    //
    U32 GetMaxAllocatedBytes()
    {
      return (maxAllocatedBytes);
    }


    //
    // GetMaxOverheadBytes
    //
    // Return the maximum number of overhead bytes
    //
    U32 GetMaxOverheadBytes()
    {
      return (maxOverheadBytes);
    }


    //
    // GetMaxCacheBlocks
    //
    // Return the maximum number of cache blocks
    //
    U32 GetMaxCacheBlocks()
    {
      return (maxCacheBlocks);
    }


    //
    // GetMaxCacheBytes
    //
    // Return the maximum number of cache bytes
    //
    U32 GetMaxCacheBytes()
    {
      return (maxCacheBytes);
    }


    //
    // GetTotalNewCalls
    //
    // Return total number of calls to new
    //
    U32 GetTotalNewCalls()
    {
      return (totalNewCalls);
    }


    //
    // Alloc
    //
    // Allocates the specified amount of memory
    // If it fails then it generates an error
    //
    // If this is a DEBUG build then add head and tail components to the memory block
    //
    void * Alloc(U32 size)
    {
      ASSERT(initialized)

      if (!size)
      {
        return (NULL);
      }

      #ifdef DEBUG_MEMORY_THREAD_SAFE
      
        EnterCriticalSection(&memoryCriticalSection);

      #endif

      #ifdef DEBUG_MEMORY_TEST_HEAP_SAFETY

        // Check to make sure that we're in the correct thread
        ASSERT(debugHeapThreadId == System::Thread::GetCurrentId())

      #endif

      #ifdef DEBUG_MEMORY_CHECKING

        U32       regs_ebp;
        U8        *ptr;
        BlockHead *head;
        BlockTail *tail;
        void      *data;

        // Grap Registers for callstack tracing
        __asm mov regs_ebp, ebp

        #ifdef DEBUG_MEMORY_STATS

          // Adjust memory overhead statistics
          overheadBytes += sizeof (BlockHead) + sizeof (BlockTail);
          maxOverheadBytes = Max(maxOverheadBytes, overheadBytes);

        #endif

        #ifdef DEBUG_MEMORY_PROTECTED
          // Grab a chunk of memory
          ptr = (U8 *) ProtectedAlloc(size + sizeof (BlockHead) + sizeof (BlockTail));
        #else
          // Grab a chunk of memory
          ptr = (U8 *) CacheAlloc(size + sizeof (BlockHead) + sizeof (BlockTail));
        #endif

        // Set pointers into this memory
        head = (BlockHead *) ptr;
        data = ptr + sizeof (BlockHead);
        tail = (BlockTail *) (ptr + sizeof (BlockHead) + size);

        // Initialize the head block
        head->prev = NULL;
        head->next = NULL;
        head->size = size;

        CallStack::Collect(regs_ebp, head->callStack, DEBUG_MEMORY_CALLSTACK_DEPTH, 1);
        memset(head->headGuard, DEBUG_MEMORY_GUARD, DEBUG_MEMORY_HEAD_GUARD_SIZE);

        // Initialize the data block
        memset(data, DEBUG_MEMORY_INIT, size);

        // Initialize the tail block
        memset(tail->tailGuard, DEBUG_MEMORY_GUARD, DEBUG_MEMORY_TAIL_GUARD_SIZE);

        // Add the block to the chain of blocks
        if (blocks)
        {
          blocks->prev = head;
          head->next = blocks;
        }
        blocks = head;

        #ifdef DEBUG_MEMORY_THREAD_SAFE
      
          LeaveCriticalSection(&memoryCriticalSection);

        #endif

        // Return the pointer to the memory
        return (data);

      #else

        #ifdef DEBUG_MEMORY_PROTECTED
          U8 *ptr = (U8 *) ProtectedAlloc(size);
        #else
          U8 *ptr = (U8 *) CacheAlloc(size);
        #endif

        #ifdef DEBUG_MEMORY_THREAD_SAFE
      
          LeaveCriticalSection(&memoryCriticalSection);

        #endif

        return (ptr);

      #endif
    }


    //
    // Free
    //
    // Frees the memory specified
    //
    void Free(void *data)
    {
      ASSERT(initialized)

      #ifdef DEBUG_MEMORY_TEST_HEAP_SAFETY

        // Check to make sure that we're in the correct thread
        ASSERT(debugHeapThreadId == System::Thread::GetCurrentId());

      #endif

      #ifdef DEBUG_MEMORY_TEST_FREE

        // Check to make sure that we're not freeing NULL
        if (!data)
        {
          return;
        }

      #endif

      #ifdef DEBUG_MEMORY_LOG_FREE

        if (!data)
        {
          LOG_ERR(("Attempt to FREE NULL memory - "))
          CallStack::Dump();
        }

      #endif

      #ifdef DEBUG_MEMORY_ERR_FREE

        if (!data)
        {
          ERR_MEM(("Free NULL"))
        }

      #endif

      #ifdef DEBUG_MEMORY_THREAD_SAFE
    
        EnterCriticalSection(&memoryCriticalSection);

      #endif

      #ifdef DEBUG_MEMORY_CHECKING

        BlockHead *head;
        BlockTail *tail;
        U8        *ptr;

        // Make sure that the pointer is safe
        if (ValidatePtr(data))
        {
          ptr = (U8 *) data;
          head = (BlockHead *) (ptr - sizeof (BlockHead));
          tail = (BlockTail *) (ptr + head->size);

          // Adjust the previous block pointer
          if (head->prev)
          {
            head->prev->next = head->next;
          }
          else
          {
            blocks = head->next;
          }

          // Adjust the next block pointer
          if (head->next)
          {
            head->next->prev = head->prev;
          }

          // Check to see if any of the memory was unitialized
          #ifdef DEBUG_MEMORY_TEST_INIT

            // Firstly test to see if this is freed memory
            U32 val = (U32) data;
            U32 s = head->size;

            __asm
            {
              cld
              mov edi, val
              mov val, 0
              mov ecx, s
            _loop:
              mov al, DEBUG_MEMORY_INIT
              scasb
              jz _fail
              dec ecx
              jnz _loop
              jmp _end
            _fail:
              mov val, 1
            _end:
            }

            if (val)
            {
              LOG_DIAG(("Freeing memory that was unititialized:"))
              DumpBlockInternals(head);
            }

          #endif

          // Adjust the memory overhead statistics
          #ifdef DEBUG_MEMORY_STATS

            // Adjust memory overhead statistics
            overheadBytes -= sizeof (BlockHead) + sizeof (BlockTail);

          #endif
  
          // Fill the memory
          memset(head, DEBUG_MEMORY_FREE, head->size + sizeof (BlockHead) + sizeof (BlockTail));

          #ifdef DEBUG_MEMORY_PROTECTED
            // Free the memory
            ProtectedFree(head);
          #else
            // Free the memory
            CacheFree(head);
          #endif
        }

      #else

        #ifdef DEBUG_MEMORY_PROTECTED
          // Free the memory
          ProtectedFree(data);
        #else
          // Free the memory
          CacheFree(data);
        #endif

      #endif

      #ifdef DEBUG_MEMORY_THREAD_SAFE
    
        LeaveCriticalSection(&memoryCriticalSection);

      #endif

    }


    //
    // CacheAlloc
    //
    // Allocates the specified amount of memory
    // If it fails then it generates an error
    //
    // If this is a DEBUG build then add head and tail components to the memory block
    //
    void * CacheAlloc(U32 size)
    {
      U8 *ptr;

      #ifdef DEBUG_MEMORY_CACHING

        size += 0x1 << DEBUG_MEMORY_CACHE_GRANULARITY;
        size &= 0xFFFFFFFF << DEBUG_MEMORY_CACHE_GRANULARITY;

        U32 index = (size - 1) >> DEBUG_MEMORY_CACHE_GRANULARITY;

        if (index < DEBUG_MEMORY_CACHE_SLOTS && memCache[index])
        {

          #ifdef DEBUG_MEMORY_CACHE_STATS

            memCacheStats[index].allocatedHits++;

            ASSERT(cacheBlocks)
            cacheBlocks--;

            ASSERT(cacheBytes >= size)
            cacheBytes -= size;

          #endif

          // Check to see if there are any blocks we can use
          ptr = (U8 *) memCache[index];
          memCache[index] = (void *) (*(U32 *) ptr);

          ASSERT(MemorySize(ptr) >= size)
        }
        else
        {
          ptr = (U8 *) MemoryAlloc(size);

          ASSERT(size == MemorySize(ptr))
        }

        #ifdef DEBUG_MEMORY_CACHE_STATS

          if (size <= DEBUG_MEMORY_CACHE_MAX_SIZE)
          {
            memCacheStats[index].allocated++;
            memCacheStats[index].allocatedCurrent++;
            memCacheStats[index].allocatedMax = Max(memCacheStats[index].allocatedMax, memCacheStats[index].allocatedCurrent);
          }

        #endif

      #else

        ptr = (U8 *) MemoryAlloc(size);

      #endif

      return (ptr);
    }


    //
    // CacheFree
    //
    void CacheFree(void *data)
    {
      #ifdef DEBUG_MEMORY_CACHING

        U32 size = MemorySize(data);

        if (size <= DEBUG_MEMORY_CACHE_MAX_SIZE)
        {
          U32 index = (size - 1) >> DEBUG_MEMORY_CACHE_GRANULARITY;

          #ifdef DEBUG_MEMORY_CACHE_STATS

            ASSERT(memCacheStats[index].allocatedCurrent)
            memCacheStats[index].allocatedCurrent--;

            cacheBlocks++;
            cacheBytes += size;
            maxCacheBlocks = Max(cacheBlocks, maxCacheBlocks);
            maxCacheBytes = Max(cacheBytes, maxCacheBytes);

          #endif

          // We can cache this memory
          *((U32 *) data) = (U32) memCache[index];
          memCache[index] = data;
        }
        else
        {
          MemoryFree(data);
        }

      #else

        MemoryFree(data);

      #endif

    };

    U32 total = 0;

    //
    // ProtectedAlloc
    //
    void * ProtectedAlloc(U32 size)
    {
      size += 4;

      // Allocate an additional page on each side of the memory we wish to allocate
      U32 totalSize = (((size + 0xFFFF) >> 16) + 2) << 16;

      total += totalSize;
      //LOG_DIAG(("Allocating %d (%d)", totalSize, total))

      void *ptr = VirtualAlloc(NULL, totalSize, MEM_COMMIT, PAGE_READWRITE);

      // Change the protection on the the no access pages
      U32 old;
      VirtualProtect(ptr, 0x10000, PAGE_NOACCESS, &old);
      VirtualProtect(((U8 *)ptr) + totalSize - 0x10000, 0x10000, PAGE_NOACCESS, &old);

      // Write the size into the the start
      U32 * start = (U32 *) (((U8 *)ptr) + 0x10000);
      *start = totalSize;

      return (start + 1);
    }


    //
    // ProtectedFree
    //
    void ProtectedFree(void *data)
    {
      U32 *start = (U32 *) data;
      start -= 1;

      U32 size = *start;

      total -= size;
      //LOG_DIAG(("Freeing %d (%d)", size, total))

      // Extract the size
      VirtualFree(((U8 *) data) - 0x10000, size, MEM_RELEASE);
    }


    //
    // MemoryAlloc
    //
    void * MemoryAlloc(U32 size)
    {
      U8 *ptr;

      // Allocate the memory the old fashioned way
      ptr = (U8 *) HeapAlloc(heap, 0, size + 4);

      if (!ptr)
      {
        ERR_MEM(("Out of Memory"))
      }

      // Save the size of the block
      *(U32 *) ptr = size;


      #ifdef DEBUG_MEMORY_STATS

        allocatedBlocks++;
        maxAllocatedBlocks = Max(maxAllocatedBlocks, allocatedBlocks);

        allocatedBytes += size;
        maxAllocatedBytes = Max(maxAllocatedBytes, allocatedBytes);

      #endif

      // Return the pointer
      return (ptr + 4);
    }


    //
    // MemoryFree
    //
    void MemoryFree(void *data)
    {
      U8 * ptr = (U8 *) data;
      ptr -= 4;

      #ifdef DEBUG_MEMORY_STATS

        // Adjust the memory statistics
        ASSERT(allocatedBlocks)
        allocatedBlocks--;

        ASSERT(allocatedBytes >= *(U32 *) ptr)
        allocatedBytes -= *(U32 *) ptr;

      #endif

      // Really free the memory
      HeapFree(heap, 0, ptr);
    }


    //
    // MemorySize
    //
    U32 MemorySize(void *ptr)
    {
      return (*(((U32 *) ptr) - 1));
    }

    
    //
    // MemoryValidate
    //
    void MemoryValidate(void *ptr)
    {
      if (!HeapValidate(heap, 0, ((U8 *) ptr) - 4))
      {
        ERR_FATAL(("Heap indicates that the memory is invalid"))
      }
    }


    //
    // ValidatePtr
    //
    // Checks to see if the pointer is valid
    //
#ifndef DEBUG_MEMORY
    Bool ValidatePtr(void *) { return (TRUE); }
#else
    Bool ValidatePtr(void *data)
    {
      ASSERT(initialized)

      #ifdef DEBUG_MEMORY_CHECKING

        if (!data)
        {
          ERR_FATAL(("Pointer is NULL"))
        }

        U8        *ptr;

        __try
        {
          ptr = (U8 *) data;

          // Firstly test to see if this is freed memory
          U32 val = (U32) data;

          __asm
          {
            cld
            mov edi, val
            mov ecx, 4
            mov al, DEBUG_MEMORY_FREE
            repe scasb    
            mov val, ecx
          }

          if (!val)
          {
            ERR_FATAL(("Previously DELETED memory"))
          }

          BlockHead *head = (BlockHead *) (ptr - sizeof (BlockHead));

          #ifndef DEBUG_MEMORY_PROTECTED
            // Validate Memory
            MemoryValidate(head);
          #endif

          // Validate the Block
          ValidateBlock(head);

        }
        __except(Exception::Filter(GetExceptionInformation()))
        {
          return (FALSE);
        }

        return (TRUE);

      #else

        data;
        return (TRUE);

      #endif
    }
#endif


    //
    // ValidateBlock
    //
    // Validate a memory block by
    // - testing the head guard
    // - testing the tail guard
    //
    Bool ValidateBlock(BlockHead *head)
    {
      ASSERT(initialized)

      BlockTail *tail;
      U32       result;
      U8        *guard;

      // Get pointer to tail block from head block information
      tail = (BlockTail *) (((U8 *) head) + sizeof (BlockHead) + head->size);

      // Test guarded header
      guard = head->headGuard;
      __asm
      {
        cld
        mov edi, guard
        mov ecx, DEBUG_MEMORY_HEAD_GUARD_SIZE
        mov al, DEBUG_MEMORY_GUARD
        repe scasb    
        mov result, ecx
      }
 
      if (result)
      {
        LOG_ERR(("Dump caused by Invalid Head Guard Block"))
        DumpBlock(head);
        ERR_MEM(("Head Guard Block is invalid!"))
      }

      // Test guarded tail
      guard = tail->tailGuard;
      __asm
      {
        cld
        mov edi, guard
        mov ecx, DEBUG_MEMORY_TAIL_GUARD_SIZE
        mov al, DEBUG_MEMORY_GUARD
        repe scasb    
        mov result, ecx
      }

      if (result)
      {
        LOG_ERR(("Dump caused by Invalid Tail Guard Block"))
        DumpBlock(head);
        ERR_MEM(("Tail Guard Block is invalid!"))
      }

      return (TRUE);
    }

    
    //
    // DumpBlockInternals
    //
    void DumpBlockInternals(BlockHead *head)
    {
      LOG_ERR(("Allocator:"))
      CallStack::Display(head->callStack);

      LOG_ERR(("Contents: %d bytes", head->size))
      Utils::MemoryDump((U8 *)head + sizeof (BlockHead), head->size);

      LOG_ERR(("Head: %d bytes", sizeof (BlockHead)))
      Utils::MemoryDump((U8 *)head, sizeof (BlockHead));

      LOG_ERR(("Tail: %d bytes", sizeof (BlockTail)))
      Utils::MemoryDump((U8 *)head + sizeof (BlockHead) + head->size, sizeof (BlockTail));
    }

    
    //
    // DumpBlock
    //
    // Dumps out lots of info about a block
    //
    void DumpBlock(BlockHead *head)
    {
      __try
      {
        LOG_ERR(("Block: %08Xh [%d (%d) bytes] Next: %08Xh Prev: %08Xh", head, head->size, head->size + sizeof (BlockHead) + sizeof (BlockTail), head->prev, head->next))
        DumpBlockInternals(head);

        // Search all the blocks for the block
        // which is before and after this block
        BlockHead *before = (BlockHead *) 0x00000000;
        BlockHead *after = (BlockHead *) 0xFFFFFFFF;
        BlockHead *ptr = blocks;

        while (ptr)
        {
          if (ptr < head && ptr > before)
          {
            before = ptr;
          }

          if (ptr > head && ptr < after)
          {
            after = ptr;
          }

          ptr = ptr->next;
        }

        // Was there a block before this block ?
        if (before != (BlockHead *) 0x00000000)
        {
          LOG_ERR(("Before Block: %08Xh [%d (%d) bytes] %d away", before, before->size, before->size + sizeof (BlockHead) + sizeof (BlockTail), (U32) head - (U32) before))
          DumpBlockInternals(before);
        }

        // Was there a block after this block ?
        if (after != (BlockHead *) 0xFFFFFFFF)
        {
          LOG_ERR(("After Block: %08Xh [%d (%d) bytes] %d away", after, after->size, after->size + sizeof (BlockHead) + sizeof (BlockTail), (U32) after - (U32) head))
          DumpBlockInternals(after);
        }
      }
      __except(Exception::Filter(GetExceptionInformation()))
      {
        LOG_ERR(("Exception Occured whilst dumping block"))
      }
    }


    //
    // ValidateAll
    //
    // Validates all the memory in the entire heap
    //
    Bool ValidateAll()
    {
      ASSERT(initialized)

    #ifdef DEBUG_MEMORY_CHECKING

      BlockHead *head;

      __try
      {
        head = blocks;

        while (head)
        {
          if (!ValidateBlock(head))
          {
            return (FALSE);
          }
          head = head->next;
        }
      }
      __except(Exception::Filter(GetExceptionInformation()))
      {
        return (FALSE);
      }

      return (TRUE);

    #else

      return (TRUE);

    #endif
    }


    //
    // Examine all of the memory
    //
    void Examine()
    {
      U8 *start = (U8 *) 0x00400000;
      U8 *end   = (U8 *) 0x80000000;

      U8 *addr = start;
      MEMORY_BASIC_INFORMATION memInfo;

      U32 total = 0;
      U32 readOnlyTotal = 0;
      U32 readWriteTotal = 0;
      U32 writeCopyTotal = 0;
      U32 executeTotal = 0;
      U32 executeReadTotal = 0;
      U32 executeReadWriteTotal = 0;
      U32 executeWriteCopyTotal = 0;

      U32 guardTotal = 0;
      U32 noAccessTotal = 0;
      U32 noCacheTotal = 0;

      U32 imageTotal = 0;
      U32 mappedTotal = 0;
      U32 privateTotal = 0;

      while (addr < end) 
      {
        if (!VirtualQuery(addr, &memInfo, sizeof (MEMORY_BASIC_INFORMATION)))
        {
          break;
        }

        // Is this block of memory being used ?
        if (memInfo.State == MEM_COMMIT) 
        {
          // Update statistics
          total += memInfo.RegionSize;

          if (memInfo.Protect & PAGE_READONLY)
          {
            readOnlyTotal += memInfo.RegionSize;
          }
          if (memInfo.Protect & PAGE_READWRITE)
          {
            readWriteTotal += memInfo.RegionSize;
          }
          if (memInfo.Protect & PAGE_WRITECOPY)
          {
            writeCopyTotal += memInfo.RegionSize;
          }
          if (memInfo.Protect & PAGE_EXECUTE)
          {
            executeTotal += memInfo.RegionSize;
          }
          if (memInfo.Protect & PAGE_EXECUTE_READ)
          {
            executeReadTotal += memInfo.RegionSize;
          }
          if (memInfo.Protect & PAGE_EXECUTE_READWRITE)
          {
            executeReadWriteTotal += memInfo.RegionSize;
          }
          if (memInfo.Protect & PAGE_EXECUTE_WRITECOPY)
          {
            executeWriteCopyTotal += memInfo.RegionSize;
          }

          if (memInfo.Protect & PAGE_GUARD)
          {
            guardTotal += memInfo.RegionSize;
          }
          if (memInfo.Protect & PAGE_NOACCESS)
          {
            noAccessTotal += memInfo.RegionSize;
          }
          if (memInfo.Protect & PAGE_NOCACHE)
          {
            noCacheTotal += memInfo.RegionSize;
          }

          if (memInfo.Type & MEM_IMAGE)
          {
            imageTotal += memInfo.RegionSize;
          }
          if (memInfo.Type & MEM_MAPPED)
          {
            mappedTotal += memInfo.RegionSize;
          }
          if (memInfo.Type & MEM_PRIVATE)
          {
            privateTotal += memInfo.RegionSize;
          }

        }

        // Addjust address to the next block
        addr = ((U8 *) memInfo.BaseAddress) + memInfo.RegionSize;
      }

      LOG_DIAG(("Memory Info:"))
      LOG_DIAG(("Total Commited     : %9d", total))
      LOG_DIAG(("Read Only          : %9d", readOnlyTotal))
      LOG_DIAG(("Read Write         : %9d", readWriteTotal))
      LOG_DIAG(("Write Copy         : %9d", writeCopyTotal))
      LOG_DIAG(("Execute            : %9d", executeTotal))
      LOG_DIAG(("Execute Read       : %9d", executeReadTotal))
      LOG_DIAG(("Execute Read Write : %9d", executeReadWriteTotal))
      LOG_DIAG(("Execute Write Copy : %9d", executeWriteCopyTotal))
      LOG_DIAG(("Guard              : %9d", guardTotal))
      LOG_DIAG(("No Access          : %9d", noAccessTotal))
      LOG_DIAG(("No Cache           : %9d", noCacheTotal))

      LOG_DIAG(("Memory Image       : %9d", imageTotal))
      LOG_DIAG(("Memory Mapped      : %9d", mappedTotal))
      LOG_DIAG(("Memory Private     : %9d", privateTotal))

    }


    //
    // GetCodeIntegrity
    //
    U32 GetCodeIntegrity(U32 crc)
    {
      return (Crc::CalcStr(Version::GetBuildString(), crc));

      //FilePath filePath;
      //GetModuleFileName(NULL, filePath.str, filePath.GetSize());
      //return (File::Crc(filePath.str, crc));
    }


    #ifdef DEBUG_MEMORY_SNAPSHOT

    //
    // SnapShotReport
    //
    void SnapShotReport()
    {
      // Load Common Controls
      INITCOMMONCONTROLSEX comctl;
      comctl.dwSize = sizeof (INITCOMMONCONTROLSEX);
      comctl.dwICC = ICC_TREEVIEW_CLASSES;
      if (!InitCommonControlsEx(&comctl))
      {
        MessageBox(NULL, "Could not initialize common controls", "Error", MB_OK | MB_ICONSTOP);
        return;
      }

      // Create the almighty root node
      UsageNode *root = new UsageNode(0x0000000);

      for (NList<InfoNode>::Iterator info(&snapShotInfo); *info; info++)
      {
        // If we walk up the stack we get the bottom up view

        // If we walk down the stack we get the top down view

        // Proceed through the call stack members and add the neccesary 
        // tree node elements, if the node exist's then just add to it

        U32 *walk = (*info)->callStack;
        UsageNode *node = root;

        for (;;)
        {
          // Update the current node
          node->size += (*info)->size;
          node->blocks++;

          // Is there any more data on the call stack ?
          if (*walk)
          {
            // Is this address in the current node ?
            UsageNode *newNode = node->children.Find(*walk);

            if (newNode)
            {
              // Yep, proceed to it
              node = newNode;
            }
            else
            {
              // Nope, add a new node and proceed to it
              newNode = new UsageNode(*walk);
              node->children.Add(*walk, newNode);
              node = newNode;
            }

            // Go to the next element of the call stack
            walk++;
          }
          else
          {
            break;
          }
        }
      }

      // We have the tree ... update the percentages
      root->cutParent = 1;
      root->cutTotal = 1;
      SnapShotUpdatePercentages(root, root);

      // The data has been formated, now display it
      DlgTemplate dlg("Memory Consumption", 50, 14, 700, 300, DS_SETFONT | DS_CENTER | WS_CAPTION | WS_VISIBLE | WS_SYSMENU);

      // Add Tree View
      dlg.AddItem(
        5, 5,
        150, 292,
        SNAPSHOT_TREE,
        WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_DISABLEDRAGDROP | TVS_SHOWSELALWAYS,
        "SysTreeView32",
        "");

      // Add List Box
      dlg.AddItem(
        160, 5,
        535, 292,
        SNAPSHOT_LIST,
        WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | WS_HSCROLL | LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_SORT | LBS_DISABLENOSCROLL | LBS_NOTIFY,
        "LISTBOX",
        "");

      if (DialogBoxIndirectParam((HINSTANCE) Debug::Inst(), (DLGTEMPLATE *) dlg.Get(), NULL, (DLGPROC) SnapShotDlgProc, (LPARAM) root) == -1)
      {
        MessageBox(NULL, Debug::LastError(), "Error", MB_OK | MB_ICONSTOP);
      }

      // Clean up time
      delete root;
      snapShotInfo.DisposeAll();
      snapShotNames.DisposeAll();
    }


    //
    // SnapShotUpdatePercentages
    //
    void SnapShotUpdatePercentages(UsageNode *root, UsageNode *node)
    {
      ASSERT(node)

      for (NBinTree<UsageNode>::Iterator i(&node->children); *i; i++)
      {
        // Update percentage of total
        (*i)->cutTotal = (F32) (*i)->size / (F32) root->size;

        // Update percentage of parent
        (*i)->cutParent = (F32) (*i)->size / (F32) node->size;

        // Recurse
        SnapShotUpdatePercentages(root, *i);
      }
    }


    //
    // SnapShotAddToTree 
    //
    void SnapShotAddItemToTree(HWND tree, HTREEITEM item, UsageNode *node) 
    {
      HTREEITEM      newItem;
      TVINSERTSTRUCT tvi;

      // Add this node to the tree
      tvi.hParent = item;
      tvi.hInsertAfter = TVI_LAST;
      tvi.item.mask = TVIF_TEXT | TVIF_PARAM;
      tvi.item.pszText = (char *) SnapShotGetName(node->address); 
      tvi.item.cchTextMax = strlen(tvi.item.pszText); 
      tvi.item.lParam = (LPARAM) node;
      newItem = TreeView_InsertItem(tree, &tvi);

      // Save the tree handle in the node
      node->treeItem = newItem;

      // Add all of this nodes children to the tree
      for (NBinTree<UsageNode>::Iterator i(&node->children); *i; i++)
      {
        SnapShotAddItemToTree(tree, newItem, *i);
      }
    }


    //
    // SnapShotGetName
    //
    const char *SnapShotGetName(U32 address)
    {
      // Is this address in the name tree ?
      NameNode *node = snapShotNames.Find(address);

      if (!node)
      {
        // Create a new node
        node = new NameNode;
        node->name = Debug::Symbol::Expand(address);
      }
      
      return (node->name);
    }


    //
    // SnapShotDlgProc
    //
    Bool CALLBACK SnapShotDlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lparam)
    {
      switch (msg)
      {
        case WM_NOTIFY:
        {
          switch (((LPNMHDR) lparam)->code) 
          { 
            case TVN_SELCHANGED:
            {
              NMTREEVIEW *tv = (NMTREEVIEW *) lparam;
              UsageNode *node = (UsageNode *) tv->itemNew.lParam;
              HWND list = GetDlgItem(hdlg, SNAPSHOT_LIST);

              SendMessage(list, WM_SETREDRAW, 0, 0);

              // Delete all the items in the listbox
              while (SendMessage(list, LB_DELETESTRING, 0, 0) != LB_ERR)
                ;

              // Add information
              SendMessage(list, LB_ADDSTRING, 0, (LPARAM) node); 

              // Add all of the children of this node
              for (NBinTree<UsageNode>::Iterator i(&node->children); *i; i++)
              {
                SendMessage(list, LB_ADDSTRING, 0, (LPARAM) *i); 
              }

              SendMessage(list, WM_SETREDRAW, 1, 0);

              break;
            }

            default:
              break;
          }

          break;
        }

        case WM_INITDIALOG:
        {
          // The root of the tree is passed in as lparam
          UsageNode *root = (UsageNode *) lparam;

          // Add all of the nodes to the tree view control
          SnapShotAddItemToTree(GetDlgItem(hdlg, SNAPSHOT_TREE), TVI_ROOT, root);

          return (TRUE);
          break;
        }

        // Close dialog box
        case WM_CLOSE:
        {
          EndDialog(hdlg, 0);
          break;
        }

        // Double Click on the list box
        case WM_COMMAND:
        {
          switch (LOWORD(wParam))             
          { 
            case SNAPSHOT_LIST:
              switch (HIWORD(wParam)) 
              { 
                case LBN_DBLCLK:
                {
                  HWND list = GetDlgItem(hdlg, SNAPSHOT_LIST);
                  int index = SendMessage(list, LB_GETCURSEL, 0, 0);

                  if (index > 0)
                  {
                    UsageNode *node = (UsageNode *) SendMessage(list, LB_GETITEMDATA, index, 0);

                    // Select this node in the tree
                    HWND tree = GetDlgItem(hdlg, SNAPSHOT_TREE);
                    TreeView_SelectItem(tree, node->treeItem);
                  }

                  return (TRUE);
                  break;
                }
              }
              break;
          }
          break;
        }

        // Get the measurement of the listbox items
        case WM_MEASUREITEM:
        {
          MEASUREITEMSTRUCT *mi;
          mi = (MEASUREITEMSTRUCT *) lparam; 
          mi->itemHeight = 16; 
          return TRUE; 
        }

        // Compare items by looking at their sample values
        case WM_COMPAREITEM:
        {
          COMPAREITEMSTRUCT *ci;
          ci = (COMPAREITEMSTRUCT *) lparam;

          UsageNode *item1 = (UsageNode *) ci->itemData1;
          UsageNode *item2 = (UsageNode *) ci->itemData2;

          return (item1->size >= item2->size ? -1 : 1);
          break;
        }

        // Draw an item in the listbox
        case WM_DRAWITEM:
        {
          DRAWITEMSTRUCT *di;
          di = (DRAWITEMSTRUCT *) lparam;

          // If there are no list box items, skip this message
          if (di->itemID == -1) 
          { 
            break; 
          }

          // Draw the item
          switch (di->itemAction) 
          { 
            case ODA_SELECT: 
            case ODA_DRAWENTIRE: 
            {
              UsageNode *node;

              node = (UsageNode *) di->itemData;

              // If the item is a profile node, draw it
              if (node)
              {
                TEXTMETRIC tm;
                int        y;
                String str;
  
                GetTextMetrics(di->hDC, &tm); 
                y = (di->rcItem.bottom + di->rcItem.top - tm.tmHeight) / 2;

                // Write Total Size
                SetTextColor(di->hDC, RGB(0, 0, 0));
                str = String::Make("%d", node->size);
                TextOut(di->hDC, 60, y, str, str.GetLength());

                // Write Total Number of Blocks
                SetTextColor(di->hDC, RGB(0, 0, 0));
                str = String::Make("%d", node->blocks);
                TextOut(di->hDC, 120, y, str, str.GetLength());

                // Write the Percentage of Total
                SetTextColor(di->hDC, RGB(128, 0, 0));
                str = String::Make("%01.4f", node->cutTotal);
                TextOut(di->hDC, 160, y, str, str.GetLength());

                // Write the Percentage of Parent
                SetTextColor(di->hDC, RGB(0, 0, 128));
                str = String::Make("%01.4f", node->cutParent);
                TextOut(di->hDC, 200, y, str, str.GetLength());

                // Write the Name
                SetTextColor(di->hDC, RGB(0, 0, 0));
                const char *name = SnapShotGetName(node->address);
                TextOut(di->hDC, 240, y, name, Utils::Strlen(name));

                // Create Total Brush
                LOGBRUSH logbrush;
                logbrush.lbStyle = BS_SOLID;
                logbrush.lbColor = RGB(128, 0, 0);
                HPEN totalPen = CreatePen(PS_SOLID, 0, RGB(128, 0, 0));
                HBRUSH totalBrush = CreateBrushIndirect(&logbrush);

                // Create Parent Brush
                logbrush.lbColor = RGB(0, 0, 128);
                logbrush.lbStyle = BS_SOLID;
                HPEN parentPen = CreatePen(PS_SOLID, 0, RGB(0, 0, 128));
                HBRUSH parentBrush = CreateBrushIndirect(&logbrush);

                // Draw Parent Bar
                // HPEN oldPen = (HPEN) 
                SelectObject(di->hDC, parentPen); 
                // HBRUSH oldBrush = (HBRUSH) 
                SelectObject(di->hDC, parentBrush);
                Rectangle(di->hDC, 55 - (U32) (node->cutParent * 50), y + 1, 55, y + 6);
                Rectangle(di->hDC, 5, y + 5, 55, y + 6);

                // Draw Total Bar
                SelectObject(di->hDC, totalPen); 
                SelectObject(di->hDC, totalBrush);
                Rectangle(di->hDC, 55 - (U32) (node->cutTotal * 50), y + 6, 55, y + 11);
                Rectangle(di->hDC, 5, y + 10, 55, y + 11);

                // Delete Objects
                DeleteObject(totalPen);
                DeleteObject(totalBrush);
                DeleteObject(parentPen);
                DeleteObject(parentBrush);

              }
              else
              // Its just plain text draw it
              {
                TEXTMETRIC tm;
                char buff[250];

                SetTextColor(di->hDC, RGB(0, 0, 0));
                SendMessage(di->hwndItem, LB_GETTEXT, di->itemID, (LPARAM) buff);
                GetTextMetrics(di->hDC, &tm); 
                TextOut(di->hDC, 0, (di->rcItem.bottom + di->rcItem.top - tm.tmHeight) / 2, buff, strlen(buff));
              }

              break; 
            }
 
            case ODA_FOCUS: 
              break; 
          } 
          return (TRUE); 
          break;

        }
      }

      return (FALSE);
    }

    #endif


    //
    // CacheReport
    //
    void CacheReport(int slot, U32 stat)
    {
      if (memCacheStats[slot].allocated || memCacheStats[slot].allocated)
      {
        U32 attempts = memCacheStats[slot].allocatedHits + memCacheStats[slot].allocatedMax;
        F32 ratio = attempts ? ((F32) memCacheStats[slot].allocatedHits) / ((F32) attempts) : 0;

        LOG_DIAG(("[%05d-%05d] %8d %8d %8d %8d %8f %8d", 
          ((slot + 1) << DEBUG_MEMORY_CACHE_GRANULARITY), (slot << DEBUG_MEMORY_CACHE_GRANULARITY) + 1, 
          memCacheStats[slot].allocated, 
          memCacheStats[slot].allocatedCurrent, 
          memCacheStats[slot].allocatedMax, 
          memCacheStats[slot].allocatedHits, 
          ratio,
          stat))
      }
    }



    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Aligning
    //


    //
    // Allocate alligned memory
    //
    void * Aligning::AligningAlloc(U32 size, U32 alignment)
    {
      U32 mask = 0xFFFFFFFF << alignment;

      // Allocate the size requested + alignment bytes + 4 bytes
      U8 * ptr = new U8[size + (1 << alignment) + 4];

      // Find the aligned data
      U8 * ptr2 = (U8 *) (((U32) ptr) + 4 + ~mask & mask);
 
      // Save pointer to the original pointer
      *(((U32 *) ptr2) - 1) = (U32) ptr;

      // Return the alligned pointer
      return (ptr2);
    }


    //
    // Free some alligned memory
    //
    void Aligning::AligningFree(void *ptr)
    {
      // Calculate the pointer
      void *ptr2 = (void *) *(((U32 *) ptr) - 1);

      // Free it
      delete ptr2;
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class UnCached
    //


    //
    // Allocate some uncached memory
    //
    void * UnCached::Alloc(U32 size)
    {
      void *ptr = HeapAlloc(GetProcessHeap(), 0, size);

      if (!ptr)
      {
        ERR_MEM(("Out of Memory"))
      }

      return (ptr);
    }


    //
    // Free some uncached memory
    //
    void UnCached::Free(void *data)
    {
      HeapFree(GetProcessHeap(), 0, data);
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Cache
    //


    //
    // Cache::Alloc
    //
    void * Cache::Alloc(void *cache, U32 size)
    {
      U8 *ptr;

      #ifdef DEBUG_MEMORY_CACHING

        if (cache)
        {
          // Check to see if there are any blocks we can use
          ptr = (U8 *) cache;
          cache = (void *) (*(U32 *) ptr);
        }
        else
        {
          // Allocate the memory the old fashioned way
          ptr = (U8 *) HeapAlloc(GetProcessHeap(), 0, size);
        }

      #else

        cache;
        ptr = (U8 *) HeapAlloc(GetProcessHeap(), 0, size);

      #endif

      if (!ptr)
      {
        ERR_MEM(("Out of Memory"))
      }

      return (ptr);
    }


    //
    // Cache::Free
    //
    void Cache::Free(void *cache, void *data)
    {
      #ifdef DEBUG_MEMORY_CACHING

        // We can cache this memory
        *((U32 *) data) = (U32) cache;
        cache = data;

      #else

        cache;
        HeapFree(GetProcessHeap(), 0, data);

      #endif
    }


    //
    // GetProcessMemory
    //
    Bool GetProcessMemory(ProcessStats &processStats)
    { 
      // Clear out the stats
      processStats.totalPrivate = 0;
      processStats.totalFixed = 0;
      processStats.totalMoveable = 0;
      processStats.totalFree = 0;
      processStats.numHeaps = 0;
      processStats.moduleNumPrivate = 0;
      processStats.moduleMemoryPrivate = 0;
      processStats.moduleNumShared = 0;
      processStats.moduleMemoryShared = 0;

      HANDLE snapShot = NULL; 

      // Take a snapshot of all modules in the specified process. 
      snapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPHEAPLIST, 0); 
      if (snapShot == HANDLE(-1))
      {
        return (FALSE); 
      }

      // Fill the size of the structure before using it. 
      MODULEENTRY32 me32 = {0}; 
      me32.dwSize = sizeof (MODULEENTRY32); 

      // Walk the module list of the process, and find the module of 
      // interest. Then copy the information to the buffer pointed 
      // to by lpMe32 so that it can be returned to the caller. 
      if (Module32First(snapShot, &me32)) 
      { 
        do 
        {
          // Is this is private module or a shared module
          if (me32.GlblcntUsage > 1)
          {
            // Shared
            if (processStats.moduleNumShared < 22)
            {
              processStats.modulesShared[processStats.moduleNumShared].name = me32.szModule;
              processStats.modulesShared[processStats.moduleNumShared].size = me32.modBaseSize;
            }
            processStats.moduleNumShared++;
            processStats.moduleMemoryShared += me32.modBaseSize;
          }
          else
          {
            // Private
            if (processStats.moduleNumPrivate < 22)
            {
              processStats.modulesPrivate[processStats.moduleNumPrivate].name = me32.szModule;
              processStats.modulesPrivate[processStats.moduleNumPrivate].size = me32.modBaseSize;
            }
            processStats.moduleNumPrivate++;
            processStats.moduleMemoryPrivate += me32.modBaseSize;
          }
        } 
        while (Module32Next(snapShot, &me32)); 
      } 

      // Iterate the HeapLists
      HEAPLIST32 hl32;
      hl32.dwSize = sizeof (HEAPLIST32);
    
      if (Heap32ListFirst(snapShot, &hl32))
      {
        do
        {
          HEAPENTRY32 he32 = {0};
          he32.dwSize = sizeof (HEAPENTRY32);

          U32 fixed = 0;
          U32 free = 0;
          U32 moveable = 0;

          // Increment heap count
          processStats.numHeaps++;

          if (Heap32First(&he32, hl32.th32ProcessID, hl32.th32HeapID))
          {
            do
            {
              if (he32.dwFlags & LF32_FIXED)
              {
                fixed += he32.dwBlockSize;
              }
              if (he32.dwFlags & LF32_FREE)
              {
                free += he32.dwBlockSize;
              }
              if (he32.dwFlags & LF32_MOVEABLE)
              {
                moveable += he32.dwBlockSize;
              }
            }
            while (Heap32Next(&he32));
          }

          processStats.totalFixed += fixed;
          processStats.totalFree += free;
          processStats.totalMoveable += moveable;
        }
        while (Heap32ListNext(snapShot, &hl32));

      }

      // Calculate the total private
      processStats.totalPrivate = processStats.moduleMemoryPrivate + processStats.totalFixed + processStats.totalMoveable;

      // Close snapshot
      CloseHandle(snapShot); 
      return (TRUE); 
    } 

  }

}

/*

#ifdef _USEMEMMGR

static struct {
	void *startAddr;
	void *endAddr;
	long roTotal;
	long rwTotal;
	long total;
	char contents[128];
} partitions[4] = {
	{(void *)0x00000000,	(void *)0x003fffff,	0,	0, 0,	"DOS/16-bit Windows :"},
	{(void *)0x00400000,	(void *)0x7fffffff,	0,	0, 0,	"Process private    :"},
	{(void *)0x80000000,	(void *)0xbfffffff,	0,	0, 0,	"Sys data shared&DLL:"},
	{(void *)0xc0000000,	(void *)0xffffffff,	0,	0, 0,	"Sys code shared    :"}
};

void Curr_MemDebug(int &totalMemNow, int &highWaterMem)
{
  // which windows partition we are looking at
  int	i;

  // Address of region being queried
  LPCVOID	addr;

  // Total memory committed
  DWORD total = 0;

  MEMORY_BASIC_INFORMATION memInfo;
  DWORD roTotal = 0;
  DWORD rwTotal = 0;

  for (i = 0; i < 4; i++) 
  {
    addr = partitions[i].startAddr;
    partitions[i].roTotal = partitions[i].rwTotal = partitions[i].total = 0;
    while (addr < partitions[i].endAddr) 
    {
      if (!VirtualQuery(addr, &memInfo, sizeof(memInfo)))
      {
        break;
      }

      if (memInfo.State == MEM_COMMIT) 
      {
        total += memInfo.RegionSize;
        partitions[i].total += memInfo.RegionSize;
        if (memInfo.Protect & PAGE_READONLY) 
        {
          roTotal += memInfo.RegionSize;
          partitions[i].roTotal += memInfo.RegionSize;
        }
        if (memInfo.Protect & PAGE_READWRITE) 
        {
          rwTotal += memInfo.RegionSize;
          partitions[i].rwTotal += memInfo.RegionSize;
        }
      }
      addr = (char *)memInfo.BaseAddress + memInfo.RegionSize;
    }
  }
  totalMemNow = total;
  if (totalMemNow > highWaterMem)
  {
    highWaterMem = totalMemNow;
  }
}

#define MegaBytes(x) (((unsigned int)x)/(1024.*1024.))
void Dump_MemDebug(char *msg)
{
  // which windows partition we are looking at
  int i;

  // Address of region being queried
  LPCVOID	addr;

  // Total memory committed
  DWORD total = 0;
  MEMORY_BASIC_INFORMATION memInfo;
  DWORD roTotal = 0;
  DWORD rwTotal = 0;
  char tmpStr[256];

  for (i = 0; i < 4; i++)
  {
    addr = partitions[i].startAddr;
    partitions[i].roTotal = partitions[i].rwTotal = partitions[i].total = 0;
    while (addr < partitions[i].endAddr) 
    {
      if (!VirtualQuery(addr, &memInfo, sizeof(memInfo)))
      {
				break;
      }
      if (memInfo.State == MEM_COMMIT) 
      {
        total += memInfo.RegionSize;
        partitions[i].total += memInfo.RegionSize;
        if (memInfo.Protect & PAGE_READONLY) 
        {
          roTotal += memInfo.RegionSize;
          partitions[i].roTotal += memInfo.RegionSize;
        }
        if (memInfo.Protect & PAGE_READWRITE) 
        {
          rwTotal += memInfo.RegionSize;
          partitions[i].rwTotal += memInfo.RegionSize;
        }
      }
      addr = (char *)memInfo.BaseAddress + memInfo.RegionSize;
    }
  }

  char *headStr1 = "Partition               read    read   total\n";
  char *headStr1a= "                        only   write\n";
  char *headStr2 = "--------------------------------------------\n";
  LOG_INFO1((headStr1));		Trace(headStr1);
  LOG_INFO1((headStr1a));		Trace(headStr1a);
  for (i = 0; i < 4; i++)
  {
    sprintf(tmpStr, "%s%8.3f%8.3f%8.3f MBytes\n",	partitions[i].contents, MegaBytes(partitions[i].roTotal), MegaBytes(partitions[i].rwTotal), MegaBytes(partitions[i].total));
    LOG_INFO1((tmpStr));	Trace(tmpStr);
  }
  LOG_INFO1((headStr2)); Trace(headStr2);
  sprintf(tmpStr,"Total Committed    :%8.3f%8.3f%8.3f MBytes\n", MegaBytes(roTotal), MegaBytes(rwTotal), MegaBytes(total));
  LOG_INFO1((tmpStr)); Trace(tmpStr);
}

#endif // _USEMEMMGR

*/


#ifdef DEBUG_MEMORY


//
// Overloaded new and delete operators
//

//
// new
//
void * CDECL operator new(size_t size)
{
#ifdef DEBUG_MEMORY_STATS
  ++Debug::Memory::totalNewCalls;
#endif

  if (Debug::Memory::initialized)
  {
    return (Debug::Memory::Alloc(size));
  }
  else
  {
    return (Debug::Memory::UnCached::Alloc(size));
  }
}


//
// new for MSVC runtime library
//
void * CDECL operator new(unsigned int size, int, const char *, int)
{
#ifdef DEBUG_MEMORY_STATS
  ++Debug::Memory::totalNewCalls;
#endif

  if (Debug::Memory::initialized)
  {
    return (Debug::Memory::Alloc(size));
  }
  else
  {
    return (Debug::Memory::UnCached::Alloc(size));
  }
}


//
// delete
//
void CDECL operator delete(void *mem)
{
  if (Debug::Memory::initialized)
  {
    Debug::Memory::Free(mem);
  }
  else
  {
    if (mem)
    {
      Debug::Memory::UnCached::Free(mem);
    }
  }
}

#endif


//
// Required libraries
//
#pragma comment(lib, "comctl32")
