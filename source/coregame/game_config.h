///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game Configuration Loading and Saving
//
// 14-AUG-1998
//

#ifndef __GAME_CONFIG_H
#define __GAME_CONFIG_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Game
//
namespace Game
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Config
  //
  namespace Config
  {
    // Initialize and shutdown the system
    void Init();
    void Done();
   
    // Get the default rule
    GameIdent & GetDefaultRule();

    // Get the fixed rule flag
    Bool & GetFixedRule();

    // Get the list of required addons
    GameIdentList & GetRequiredAddons();

    // Save the Game Configuration for a mission, false if unable to create file
    Bool Save(const char *name);

    // Load the Game Configuration for a mission, false if not found
    Bool Load(const char *name); 
  }
}

#endif
