///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1998
// Matthew Versluys
//
// Debug Symbols
//


#ifndef __DEBUG_SYMBOL_H
#define __DEBUG_SYMBOL_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "debug_symbol.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Debug
//
namespace Debug
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Symbol
  //
  namespace Symbol
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Definitions
    //
    #define SYMOPT_CASE_INSENSITIVE  0x00000001
    #define SYMOPT_UNDNAME           0x00000002
    #define SYMOPT_DEFERRED_LOADS    0x00000004
    #define SYMOPT_NO_CPP            0x00000008
    #define SYMOPT_LOAD_LINES        0x00000010
    #define SYMOPT_OMAP_FIND_NEAREST 0x00000020
    #define SYM_PATH                 "library\\dbg\\"

    struct Symbol
    {
      U32 structSize;
      U32 address;
      U32 size;
      U32 flags;
      U32 namelen;
      char name[512];
    };

    struct Line
    {
      U32 structSize;
      void *key;
      U32 line; 
      char *file; 
      U32 address; 
    };
   
    struct Module
    {
      U32 structSize;
      U32 imageBase;
      U32 imageSize;
      U32 timeDateStamp;
      U32 checkSum;
      U32 numSyms;
      U32 symType;
      char moduleName[32];
      char imageName[256];
      char loadedImageName[256];
    };

 
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Type Definitions
    //
    typedef Bool (STDCALL *SymInitializeFunc)(HANDLE process, char *path, Bool invade);
    typedef Bool (STDCALL *SymCleanupFunc)(HANDLE process);
    typedef U32 (STDCALL *SymSetOptionsFunc)(U32 options);
    typedef Bool (CALLBACK *SymEnumModulesCallbackFunc)(char *module, U32 address, void *context);
    typedef Bool (STDCALL *SymEnumerateModulesFunc)(HANDLE process, SymEnumModulesCallbackFunc callback, void *context);
    typedef Bool (CALLBACK *SymEnumSymbolsCallbackFunc)(char *symbol, U32 address, U32 size, void *context);
    typedef Bool (STDCALL *SymEnumerateSymbolsFunc)(HANDLE process, U32 base, SymEnumSymbolsCallbackFunc callback, void *context);
    typedef Bool (STDCALL *SymGetSymFromAddrFunc)(HANDLE process, U32 address, U32 *displacement, Symbol *symbol);
    typedef Bool (STDCALL *SymGetLineFromAddrFunc)(HANDLE process, U32 address, U32 *displacement, Line *line);
    typedef Bool (STDCALL *SymGetModuleBaseFunc)(HANDLE process, U32 address);
    typedef Bool (STDCALL *SymGetModuleInfoFunc)(HANDLE process, U32 address, Module *module);


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Internal Data
    //
    static SymInitializeFunc SymInitialize;
    static SymCleanupFunc SymCleanup;
    static SymSetOptionsFunc SymSetOptions;
    static SymEnumerateModulesFunc SymEnumerateModules;
    static SymEnumerateSymbolsFunc SymEnumerateSymbols;
    static SymGetSymFromAddrFunc SymGetSymFromAddr;
    static SymGetLineFromAddrFunc SymGetLineFromAddr;
    static SymGetModuleBaseFunc SymGetModuleBase;
    static SymGetModuleInfoFunc SymGetModuleInfo;

    static HANDLE process;

    static Bool initialized = FALSE;
    static Bool available = FALSE;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Prototypes
    //
    Bool CALLBACK SymEnumModulesCallback(char *module, U32 address, void *context);
    Bool CALLBACK SymEnumSymbolsCallback(char *symbol, U32 address, U32 size, void *context);


    //
    // Initialization
    //
    void Init()
    {
      ASSERT(!initialized)

      // Try to find the DebugHlp DLL
      HINSTANCE library = LoadLibrary(SYM_PATH "dbghelp.dll");

      // Save the process handle (note, this is Win9x specific!)
      process = (HANDLE) GetCurrentProcessId();

      if (library)
      {
        SymInitialize = (SymInitializeFunc) GetProcAddress(library, "SymInitialize");
        SymCleanup = (SymCleanupFunc) GetProcAddress(library, "SymCleanup");
        SymSetOptions = (SymSetOptionsFunc) GetProcAddress(library, "SymSetOptions");
        SymEnumerateModules = (SymEnumerateModulesFunc) GetProcAddress(library, "SymEnumerateModules");
        SymEnumerateSymbols = (SymEnumerateSymbolsFunc) GetProcAddress(library, "SymEnumerateSymbols");
        SymGetSymFromAddr = (SymGetSymFromAddrFunc) GetProcAddress(library, "SymGetSymFromAddr");
        SymGetLineFromAddr = (SymGetLineFromAddrFunc) GetProcAddress(library, "SymGetLineFromAddr");
        SymGetModuleBase = (SymGetModuleBaseFunc) GetProcAddress(library, "SymGetModuleBase");
        SymGetModuleInfo = (SymGetModuleInfoFunc) GetProcAddress(library, "SymGetModuleInfo");

        if (
          SymInitialize && 
          SymCleanup && 
          SymSetOptions && 
          SymEnumerateModules && 
          SymEnumerateSymbols &&
          SymGetSymFromAddr &&
          SymGetLineFromAddr &&
          SymGetModuleBase &&
          SymGetModuleInfo)
        {
          SymSetOptions(SYMOPT_UNDNAME | SYMOPT_LOAD_LINES | SYMOPT_DEFERRED_LOADS);
          if (SymInitialize(process, ";"SYM_PATH, TRUE))
          {
            available = TRUE;
          }
          else
          {
            LOG_DIAG(("SymInitialized failed: %s", Debug::LastError()))
          }
        }
        else
        {
          LOG_DIAG(("Could not find neccesary functions in dbghelp.dll"))
        }

      }
      else
      {
        LOG_DIAG(("Could not find dbghelp.dll"))
      }

      initialized = TRUE;
    }


    //
    // Shutdown
    //
    void Done()
    {
      ASSERT(initialized)
      if (available)
      {
        SymCleanup(process);
        available = FALSE;
      }
      initialized = FALSE;
    }


    //
    // Dump
    //
    void Dump()
    {
      ASSERT(initialized)
      if (available)
      {
        U32 size = 0;
        SymEnumerateModules(process, SymEnumModulesCallback, &size);
        LOG_DIAG(("Grand Total: %s", size))
      }
    }


    //
    // Expand
    //
    void Expand(U32 address, 
      char *&functionName, U32 &functionDisplacement,
      char *&fileName, U32 &lineNumber, U32 &lineNumberDisplacement,
      char *&moduleName)
    {
      ASSERT(initialized)

      functionName = "unknown";
      functionDisplacement = 0;
      fileName = "unknown";
      lineNumber = 0;
      lineNumberDisplacement = 0;
      moduleName = "unknown";

      if (available)
      {
        static Symbol symbol;
        symbol.structSize = 20;
        symbol.namelen = 512;
        if (SymGetSymFromAddr(process, address, &functionDisplacement, &symbol))
        {
          functionName = symbol.name;
        }

        Line line;
        line.structSize = sizeof (Line);
        if (SymGetLineFromAddr(process, address, &lineNumberDisplacement, &line))
        {
          fileName = line.file;
          lineNumber = line.line;
        }

        U32 moduleBase = SymGetModuleBase(process, address);

        if (moduleBase)
        {
          static Module module;
          module.structSize = sizeof (Module);
          if (SymGetModuleInfo(process, moduleBase, &module))
          {
            moduleName = module.moduleName;
          }
        }
      }
    }


    //
    // Expand
    //
    const char *Expand(U32 address)
    {
      ASSERT(initialized)

      char *functionName;
      U32 functionDisplacement;

      char *fileName;
      U32 lineNumber;
      U32 lineNumberDisplacement;

      char *moduleName;

      Expand(address, functionName, functionDisplacement, fileName, lineNumber, lineNumberDisplacement, moduleName);

      static char buffer[2048];

      Utils::Sprintf(buffer, 2048, "%08X %s %s [%04X] %s(%d) [%04X]",
        address, 
        moduleName, 
        functionName, functionDisplacement, 
        fileName, lineNumber, lineNumberDisplacement);

      return (buffer);
    }


    //
    // SymEnumModulesCallback
    //
    Bool CALLBACK SymEnumModulesCallback(char *module, U32 address, void *context)
    {
      LOG_DIAG(("[%08X] Module: %s", address, module))
      U32 *total = (U32 *) context;
      U32 size = 0;
      SymEnumerateSymbols(process, address, SymEnumSymbolsCallback, &size);
      LOG_DIAG(("Total Size: %d", size))
      *total += size;
      return (TRUE);
    }


    //
    // SymEnumModulesCallback
    //
    Bool CALLBACK SymEnumSymbolsCallback(char *symbol, U32 address, U32 size, void *context)
    {
      U32 *total = (U32 *) context;
      *total += size;
      LOG_DIAG(("[%08X] (%d) %s", address, size, symbol))
      return (TRUE);
    }
    
  }

}

#endif
