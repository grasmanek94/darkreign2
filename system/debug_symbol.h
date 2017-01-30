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
#include "debug.h"


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

    // Initialize
    void Init();

    // Shutdown
    void Done();

    // Dump all symbols
    void Dump();

    // Expand an address
    void Expand(U32 address, 
      char *&functionName, U32 &functionDispacement,
      char *&fileName, U32 &lineNumber, U32 &lineNumberDisplacement,
      char *&moduleName);

    // Expand into a string
    const char * Expand(U32 address);

  }

}

#endif
