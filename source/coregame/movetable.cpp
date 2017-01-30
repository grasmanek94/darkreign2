///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Movement Table
//
// 1-SEP-1998
//

//
// Includes
//
#include "movetable.h"
#include "stdload.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace MoveTable - Maintains surface/traction balance information
//

namespace MoveTable
{

  //
  // System data
  //
  static Bool initialized = FALSE;
  MoveBalanceTable *table = NULL;


  //
  // Init
  //
  // Initialize system
  //
  void Init()
  {
    ASSERT(!initialized);

    // Default balance data values
    BalanceData defaultData = { MIN_PASSABLE_SPEED, 0.0F, 0 };

    // Allocate the balance table
    table = new MoveBalanceTable(MAX_SURFACE_TYPES, MAX_TRACTION_TYPES, defaultData);

    // System now initialized
    initialized = TRUE;
  }


  //
  // Done
  //
  // Shutdown system
  //
  void Done()
  {
    ASSERT(initialized);

    // Delete the balance table
    delete table;

    // System now shutdown
    initialized = FALSE;
  }


  //
  // SurfaceIndex
  //
  // Get the index for a surface type, creating if not found
  //
  U8 SurfaceIndex(const char *key)
  {
    ASSERT(initialized);

    U8 index = 0;

    if (!table->GetXIndex(key, index))
    {
      ERR_CONFIG(("Max SURFACE types has been reached on '%s' (%u)", key, table->KeyCountX()));
    }

    return (index);
  }


  //
  // TractionIndex
  //
  // Get the index for a traction type, creating if not found
  //
  U8 TractionIndex(const char *key)
  {
    ASSERT(initialized);

    U8 index = 0;

    if (!table->GetYIndex(key, index))
    {
      ERR_CONFIG(("Max TRACTION types has been reached on '%s' (%u)", key, table->KeyCountY()));
    }

    return (index);
  }


  //
  // ProcessCreateTractionType
  //
  // Process a traction creation scope
  //
  void ProcessCreateTractionType(FScope *fScope)
  {
    ASSERT(initialized);
    ASSERT(fScope);

    FScope *sScope;
    U32 defaultSlope = 30;

    // Get the traction type index
    U8 traction = TractionIndex(fScope->NextArgString());

    // Process each function
    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x1788F661: // "DefaultSlope"
          defaultSlope = StdLoad::TypeU32(sScope, Range<U32>(0, 90));
          break;

        case 0x841386F6: // "OverSurface"
        {
          // Get the type index
          U8 surface = SurfaceIndex(sScope->NextArgString());

          // Get the balance data for this combination
          BalanceData &data = table->GetBalance(surface, traction);

          // Speed is a percentage between 5% and 100%
          data.speed = StdLoad::TypeF32(sScope, Range<F32>(MIN_PASSABLE_SPEED, 1.0F));

          // The slope is between 0 and 90, where 0 is horizontal
          data.slope = (U8)defaultSlope;

          // Health modifier is a percentage of maximum hitpoints
          data.health = StdLoad::TypeF32(sScope, data.health, Range<F32>(-1.0F, 1.0F));
          break;
        }
      }
    }
  }
}


