/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic AI
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_util.h"
#include "resolver.h"
#include "sides.h"
#include "team.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Util
  //
  namespace Util
  {
    
    //
    // Load type
    //
    UnitObjType * LoadType(FScope *fScope, Team *team)
    {
      UnitObjTypePtr reaper;
      GameIdent typeName = StdLoad::TypeString(fScope);

      // Is there a side mapping for this identifier ?
      if (team)
      {
        Sides::Side &side = Sides::GetSide(team->GetSide());
        const char *mapped = side.GetMapping(typeName.crc);

        if (mapped)
        {
          // Resolve the type
          Resolver::Type<UnitObjType>(reaper, mapped, TRUE);
        }
        else
        {
          // Resolve the type
          Resolver::Type<UnitObjType>(reaper, typeName, TRUE);
        }
      }
      else
      {
        // Resolve the type
        Resolver::Type<UnitObjType>(reaper, typeName, TRUE);
      }

      ASSERT(reaper.Alive())
      return (reaper);
    }

  }

}

