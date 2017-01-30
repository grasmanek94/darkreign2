///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Configurable Unit Construction Limitations
//
// 6-MAR-2000
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "unitlimits.h"
#include "stdload.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace UnitLimits
//
namespace UnitLimits
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // System Functions
  //

  // Is the system initialized
  static Bool initialized = FALSE;

  // The name of the config file
  static const char *CONFIG_FILE_NAME = "unitlimits.cfg";

  // The per team command point limit
  static U32 limit = 40;


  //
  // Init
  //
  // Initialize system
  //
  void Init()
  {
    ASSERT(!initialized)

    PTree pTree;

    // Process the configuration
    if (pTree.AddFile(CONFIG_FILE_NAME))
    {
      // Get the global scope
      FScope *fScope = pTree.GetGlobalScope();

      // Load limit
      limit = StdLoad::TypeU32(fScope, "Limit");
    }

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
    ASSERT(initialized)

    // System now shutdown
    initialized = FALSE;
  }


  //
  // GetLimit
  //
  // Returns the per team command point limit
  //
  U32 GetLimit()
  {
    return (limit);
  }
}


