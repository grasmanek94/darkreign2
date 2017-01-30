///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Mission Vars
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "varfile.h"
#include "missionvar.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MissionVar
//
namespace MissionVar
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Definitions
  //

  // Scope where mission vars are saved
  const char *varScope = "coregame.mission.dyndata";


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //
  static Bool initialized = FALSE;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Substitution
  //
  struct Substitution : public VarSys::Substitution
  {

    // Constructor
    Substitution() 
    : VarSys::Substitution(MISSION_VARCHAR) 
    { 
    }

    // Expansion
    const char *Expand(const char *, void *)
    {
      ASSERT(initialized)
      return (varScope);
    }

  };

  static Substitution substitution;



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Prototypes
  //

  // Reset
  void Reset();

  //
  // Init
  //
  void Init()
  {
    ASSERT(!initialized)

    // Register the substitution
    VarSys::RegisterSubstitution(substitution);

    Reset();

    initialized = TRUE;
  }


  //
  // Done
  //
  void Done()
  {
    ASSERT(initialized)

    Reset();

    // Unregister the var substitution
    VarSys::UnregisterSubstitution(substitution);

    initialized = FALSE;
  }


  //
  // Save the mission vars
  //
  void Save(FScope *fScope)
  {
    ASSERT(initialized)

    if (VarSys::VarScope *scope = VarSys::FindVarScope(varScope))
    {  
      VarFile::Save(fScope, scope);
    }
  }


  //
  // Load the mission vars
  //
  void Load(FScope *fScope)
  {
    ASSERT(initialized)

    VarFile::Load(fScope, varScope);
  }


  //
  // Reset
  //
  void Reset()
  {
    VarSys::DeleteItem(varScope);
  }



}
