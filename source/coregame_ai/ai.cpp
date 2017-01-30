//////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// AI
//
// 19-AUG-1998
//


//////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "ai.h"
#include "ai_map.h"
#include "ai_debug.h"
#include "strategic.h"
#include "strategic_config.h"
#include "tactical.h"
#include "formation.h"
#include "varsys.h"
#include "multiplayer.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
#define AI_CONFIGFILE "ai.cfg"


//////////////////////////////////////////////////////////////////////////////
//
// NameSpace AI
// 
namespace AI
{

  //////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //
  static Bool initialized = FALSE;
  static VarInteger logging;


  //////////////////////////////////////////////////////////////////////////////
  //
  // Prototypes
  //


  //
  // Init
  //
  // Initialize AI
  //
  void Init()
  {
    ASSERT(!initialized)

    // Var
    VarSys::CreateInteger("ai.logging", 0, VarSys::DEFAULT, &logging);

    // Initialize Debug System
    Debug::Init();

    // Initialize Map System
    Map::Init();

    // Initialize Tactical AI System
    Tactical::Init();

    // Initialize Strategic AI System
    Strategic::Init();

    // Initiailze Formation System
    Formation::Init();

    // Set the initialized flag
    initialized = TRUE;
  }


  //
  // Done
  //
  // Shutdown AI
  //
  void Done()
  {
    ASSERT(initialized)

    // Shutdown Formation System
    Formation::Done();

    // Shutdown Strategic AI System
    Strategic::Done();

    // Shutdown Tactical AI System
    Tactical::Done();

    // Shutdown Map System
    Map::Done();

    // Shutdown Debug System
    Debug::Done();

    // Delete command scope
    VarSys::DeleteItem("ai");

    // Clear the initialized flag
    initialized = FALSE;
  }


  //
  // ConfigLoad
  //
  void ConfigLoad()
  {
    ASSERT(initialized)

    // Load information from the AI type file
    ProcessConfigFile(AI_CONFIGFILE);

  }


  //
  // ConfigPostLoad
  //
  void ConfigPostLoad()
  {
    ASSERT(initialized)

    // Reset Strategic AI
    Strategic::Reset();

  }


  //
  // Save
  //
  // Save AI state data
  //
  void Save(FScope *scope)
  {
    Strategic::Save(scope->AddFunction("Strategic"));
  }


  //
  // Load
  //
  // Load AI state data
  //
  void Load(FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x622EF512: // "Strategic"
          Strategic::Load(sScope);
          break;
      }
    }
  }


  //
  // InitSimulation
  //
  void InitSimulation()
  {
    // Notify Debug that the sim is starting
    Debug::InitSimulation();
  }

  
  //
  // DoneSimulation
  //
  void DoneSimulation()
  {
    // Notify Debug that the sim is ending
    Debug::DoneSimulation();
  }


  //
  // Process
  //
  void Process()
  {
    ASSERT(initialized)

    // Process Strategic AI
    Strategic::Process();

  }


  //
  // Is the AI Logging
  //
  Bool IsLogging()
  {
    ASSERT(initialized)

    return (logging);  
  }



  //
  // ProcessConfigFile
  //
  // Process a configuration file, FALSE if unable to load file
  //
  Bool ProcessConfigFile(const char *name)
  {
    ASSERT(initialized)
    ASSERT(name)

    PTree tFile(TRUE, MultiPlayer::GetDataCrc());
    FScope *sScope, *fScope;

    // Parse the file
    if (!tFile.AddFile(name))
    {
      return (FALSE);
    }
  
    // Get the global scope
    fScope = tFile.GetGlobalScope();

    // Step through each function in this scope
    while ((sScope = fScope->NextFunction()) != 0)
    {
      switch (sScope->NameCrc())
      {
        case 0x622EF512: // "Strategic"
          Strategic::Config::ProcessConfig(sScope);
          break;

        case 0xEB474C2E: // "Tactical"
          Tactical::ProcessConfig(sScope);
          break;

        case 0x7223612A: // "Formation"
          Formation::ProcessConfig(sScope);
          break;

        default:
          sScope->ScopeError("Unkown function '%s' in '%s'", sScope->NameStr(), name);
          break;
      }
    } 

    LOG_DIAG(("Type Stream Crc: %08X", tFile.GetCrc()))
    MultiPlayer::SetDataCrc(tFile.GetCrc());

    // success
    return (TRUE);
  }

}