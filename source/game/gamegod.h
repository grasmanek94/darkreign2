///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// GameGod, the creator and the destroyer
//
// 24-JUL-1998
//

#ifndef __GAMEGOD_H
#define __GAMEGOD_H


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

#define APPLICATION_CONFIGFILE  "library\\engine\\startup.cfg"



///////////////////////////////////////////////////////////////////////////////
//
// Namespace GameGod
//

namespace GameGod
{
  // Startup and shutdown
  void Init();
  void Done();

  // Main game loop
  void CDECL Start();

  // Set mode callback
  void SetModeChangeCallback();

  // Should we check types (for unused fScopes) ?
  Bool CheckTypes();

  // Should we check objects (for unused fScope) ?
  Bool CheckObjects();

  // Set flow action
  void SetFlowAction(const char *action);


  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Loader - Mission loading progress
  //
  namespace Loader
  {
    // Specify a new subsystem
    void SubSystem(const char *name, U32 items);

    // Update progress of previous system
    void Update(U32 count);

    // Advance counter
    void Advance(U32 count = 1);

    // Finished loading
    void Done();
  }
};    

#endif