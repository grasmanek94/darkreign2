///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Debugging Tools
//
// 1-DEC-1997
//


#ifndef __DEBUG_H
#define __DEBUG_H


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//


//
// Assertion Macro (ONLY PRESENT IN DEVELOPMENT BUILDS!)
//
// test: what is being tested
// msg : description of why this should be asserted
//
#ifdef ASSERTIONS_ACTIVE
  #define ASSERT(test)                                                                  \
  if (!(int) (test))                                                                    \
  {                                                                                     \
    ::Debug::Error::Set(__FILE__, __LINE__, __TIMESTAMP__, ::Debug::Error::ASSERTION);  \
    ::Debug::Error::Err(#test);                                                         \
  }
#else
  #define ASSERT(test)
#endif


//
// Validate Macro (ONLY PRESENT IN DEVELOPMENT BUILDS!)
//
#ifdef DEVELOPMENT
  #define VALIDATE(ptr) { ::Debug::Memory::ValidatePtr(ptr); }
#else
  #define VALIDATE(ptr)
#endif


//
// Error Macro
//
#define ERR(t, y)                                             \
{                                                             \
  ::Debug::Error::Set(__FILE__, __LINE__, __TIMESTAMP__, t);  \
  ::Debug::Error::Err y;                                      \
}


//
// Error Macro
// Allows specification of line and file number and timestamp
//
#define ERR_INDIRECT(fn, ln, t, y)                            \
{                                                             \
  ::Debug::Error::Set(fn, ln, __TIMESTAMP__, t);              \
  ::Debug::Error::Err y;                                      \
}


//
// Specific Error Macros
//
#define ERR_MEM(y)      ERR(::Debug::Error::MEM, y)
#define ERR_CONFIG(y)   ERR(::Debug::Error::CONFIG, y)
#define ERR_FATAL(y)    ERR(::Debug::Error::FATAL, y)
#define ERR_WAIT(y)     ERR(::Debug::Error::WAIT, y)
#define ERR_MESSAGE(y)  ERR(::Debug::Error::MESSAGE, y)


//
// Guard block
//
#ifdef DEVELOPMENT
  #define STATIC_GUARD_BLOCK_ENABLED
#endif

#ifdef STATIC_GUARD_BLOCK_ENABLED

  #define GUARD_BLOCK_MACRO3(x) static ::Debug::StaticGuard::Block _guard##x
  #define GUARD_BLOCK_MACRO2(x) GUARD_BLOCK_MACRO3(x);
  #define GUARD_BLOCK_MACRO1(x) GUARD_BLOCK_MACRO2 ##x

  #define DEBUG_STATIC_GUARD_BLOCK        GUARD_BLOCK_MACRO1((__LINE__))
  #define DEBUG_STATIC_GUARD_BLOCK_CHECK  ::Debug::StaticGuard::Block::CheckAll();

#else

  #define DEBUG_STATIC_GUARD_BLOCK
  #define DEBUG_STATIC_GUARD_BLOCK_CHECK

#endif


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Debug
//
namespace Debug
{

  // PreIgnition: The initialization before initialization
  // Needs to be done before mono and logging are up
  void PreIgnition();

  // Initialization
  // Needs to be done after mono and logging are up
  void Init();

  // Shutdown
  void Done();

  // LastError
  const char *LastError();

  // Setup instance
  void SetupInst(HINSTANCE inst);

  // Inst
  HINSTANCE Inst();

  // UnderDebugger
  Bool UnderDebugger();

  // Are we fuxored
  Bool IsFuxored();


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace AtExit
  //
  namespace AtExit
  {

    const U32 MAXPROC = 10;
    typedef void (CDECL EXITPROC)(void);

    Bool Register(EXITPROC *fn);
    void Execute();

  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace CallStack
  //
  namespace CallStack
  {

    // Dump: Dump the current callstack
    void Dump();

    // Caller: Dump the current caller
    void Caller();

    // Collect: Collect the callstack
    void Collect(U32 ebp, U32 *buf, U32 num, U32 skip);

    // Display
    void Display(U32 *buf);

  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Exception
  //
  namespace Exception
  {

    // SetHandler: Set the default exception handler
    void SetHandler();

    // Handler: Enclosing exception function
    void Handler(void (CDECL *)());

  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Error
  //
  namespace Error
  {

    enum Type
    {
      MEM,        // unable to allocate memory
      CONFIG,     // configuration error
      ASSERTION,  // assertion
      FATAL,      // all other cases
      WAIT,       // forces the window to wait
      MESSAGE     // user friendly messages
    };

    // Used by the macro to save file and line number and time stamp
    void Set(const char *fn, U32 ln, const char *ts, Type type);   

    // The Actual Error function
    void NORETURN CDECL Err(const char *format, ...);       

  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Watchdog
  //
  namespace Watchdog
  {

    // Enable the watchdog
    void Enable();

    // Create watchdog
    void Create();

    // Delete watchdog
    void Delete();

    // Poll watchdog
    void Poll();
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace StaticGuard
  //
  namespace StaticGuard
  {

    // Size of guard, in bytes
    const U32 STATIC_GUARD_BLOCK_SIZE = 16;


    /////////////////////////////////////////////////////////////////////////////
    //
    // NameSpace StaticGuard
    //
    struct Block
    {
      // Head of the list
      static Block *head;

      // Per instance guard information
      U8 buf[STATIC_GUARD_BLOCK_SIZE];
      Block *next;

      // Constructor
      Block();

      // CheckAll
      static void CheckAll();
    };
  }
};

#endif
