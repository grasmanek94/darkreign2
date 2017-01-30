/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// AI
//
// 19-AUG-1998
//

#ifndef __AI_H
#define __AI_H


//////////////////////////////////////////////////////////////////////////////
//
// Includes
//


//////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
#ifdef DEVELOPMENT
  #define LOG_AI(x)       \
  if (::AI::IsLogging())  \
  {                       \
    LOG_DIAG(x)           \
  }
#else
  #define LOG_AI(x)
#endif


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace AI
//
namespace AI
{
  // Init: Initialize AI
  void Init();

  // Done: Shutdown AI
  void Done();

  // ConfigLoad: Load AI Configuration files
  void ConfigLoad();

  // Post Load the AI
  void ConfigPostLoad();

  // Save and load AI state data
  void Save(FScope *scope);
  void Load(FScope *scope);

  // Init Simulation
  void InitSimulation();

  // Done Simulation
  void DoneSimulation();

  // Process AI
  void Process();

  // Is logging enabled
  Bool IsLogging();

  // Process a type definition file, FALSE if unable to load file
  Bool ProcessConfigFile(const char *name);

}

#endif