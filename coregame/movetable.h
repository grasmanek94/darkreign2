///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Movement Table
//
// 1-SEP-1998
//

#ifndef __MOVETABLE_H
#define __MOVETABLE_H


//
// Includes
//
#include "balancetable.h"
#include "fscope.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace MoveTable - Maintains surface/traction balance information
//

namespace MoveTable
{

  // The lowest speed that should be considered for pathsearching
  const F32 MIN_PASSABLE_SPEED = 0.05F;

  // Maximum number of surface/traction types
  const U32 MAX_SURFACE_TYPES = 64;
  const U32 MAX_TRACTION_TYPES = 64;

  // Balance data for each surface/traction combination
  struct BalanceData
  {
    // Percentage of speed over this surface
    F32 speed;

    // Percentage of maximum hit points
    F32 health;

    // Slope between 0 and 90, where 0 is horizontal
    U8 slope;
  };

  // Custom balance table using the movement data
  typedef BalanceTable<BalanceData> MoveBalanceTable;

  // Key info
  typedef BalanceTable<BalanceData>::KeyInfo KeyInfo;

  // Pointer to dynamically allocated balance table
  extern MoveBalanceTable *table;


  // Initialize and shutdown system
  void Init();
  void Done();

  // Get the index for a surface/traction type, creating if not found
  U8 SurfaceIndex(const char *key);
  U8 TractionIndex(const char *key);

  // Process a traction creation scope
  void ProcessCreateTractionType(FScope *fScope);

  // Get the balance info for a particular surface/traction combination
  inline BalanceData & GetBalance(U8 surface, U8 traction)
  {
    return (table->GetBalance(surface, traction));
  }

  // Returns a reference to the movement table
  inline MoveBalanceTable & GetTable()
  {
    return (*table);
  }

  // Returns the key info for a surface type, or NULL if not found
  inline KeyInfo * FindSurfaceInfo(const char *key)
  {
    return (table->FindKeyX(key));
  }

  // Returns the key info for a surface type, or NULL if not found
  inline KeyInfo * FindSurfaceInfo(U8 index)
  {
    return (table->FindKeyXIndex(index));
  }

  // Returns the key info for a traction type, or NULL if not found
  inline KeyInfo * FindTractionInfo(const char *key)
  {
    return (table->FindKeyY(key));
  }

  // Returns the key info for a traction type, or NULL if not found
  inline KeyInfo * FindTractionInfo(U8 index)
  {
    return (table->FindKeyYIndex(index));
  }

  // Returns the number of surface entries
  inline U32 SurfaceCount()
  {
    return (table->KeyCountX());
  }

  // Returns the number of traction entries
  inline U32 TractionCount()
  {
    return (table->KeyCountY());
  }
}

#endif
