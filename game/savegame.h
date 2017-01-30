///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Save Game
//
// 14-JAN-2000
//


#ifndef __SAVEGAME_H
#define __SAVEGAME_H


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace SaveGame
//
namespace SaveGame
{
  // Information about a save game
  struct Info
  {
    // The length of an info string
    enum { STR_LENGTH = 256 };

    // A single info string
    typedef char String[STR_LENGTH];

    // Is the slot information valid
    Bool valid;

    // The description of the save game
    String description;

    // The name of the mission
    String missionName;

    // The path of the mission
    String missionPath;
  };

  
  // Initialize and shutdown system
  void Init();
  void Done();


  // Get the information for the given slot (FALSE if not used)
  Bool GetInfo(const char *slot, Info &info);

  // Is the given slot used
  Bool SlotUsed(const char *slot);

  // Save a game 
  Bool Save(const char *slot, const char *description);

  // Is a save in progress
  Bool SaveActive();

  // Returns the path to use while a save is active
  const char * GetDestination();


  // Load a game
  Bool Load(const char *slot);

  // Is a load in progress
  Bool LoadActive();


  // Notify the system of a save or load event
  void Notify(U32 event);
};

#endif