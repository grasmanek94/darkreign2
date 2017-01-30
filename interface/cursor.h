/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Cursor system
//
// 01-JUN-1998
//


#ifndef __CURSOR_H
#define __CURSOR_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "bitmapdec.h"
#include "fscope.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace CursorSys 
//
namespace CursorSys
{
  // Standard cursor types
  enum StdCursor
  {
    DEFAULT   = 0,
    IBEAM     = 1,
    WAIT      = 2,
    NO        = 3,

    MAX_CURSORS,
  };

  // Initialise the cursor system
  void Init();

  // Shutdown the cursor system
  void Done();

  // Process a StandardCursors scope
  void ProcessStandardCursors(FScope *fScope);

  // Process a CreateCursor scope
  void ProcessCreateCursor(FScope *fScope);

  // Delete all cursors
  void DeleteAll();

  // Set the cursor to be the active cursor
  Bool Set(const char *name);

  // Set the cursor to be the active cursor
  Bool Set(U32 id);

  // Draw the cursor
  void Display(S32 x, S32 y);

  // Get default cursor
  U32 DefaultCursor();

  // Find a standard cursor
  U32 GetStandardCursor(StdCursor crs);

};

#endif
