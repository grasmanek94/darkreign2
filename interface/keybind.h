///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Input bindings
//
// 12-JUN-1998
//


#ifndef __KEYBIND_H
#define __KEYBIND_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "event.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace KeyBind - Key bindings
//
namespace KeyBind
{

  //
  // Binding flags
  //
  enum
  {
    READONLY  = 0x00000001,
  };


  //
  // Dump flags
  //
  enum
  {
    DUMP_PRESS = 0x0001,
    DUMP_HOLD  = 0x0002,
    DUMP_KEYS  =  0x0004,
    DUMP_ALL   = DUMP_PRESS | DUMP_HOLD,
  };


  // Initialise keybinding system
  void Init();

  // Shutdown key binding system
  void Done();

  // Read scancodes from an FScope
  void ProcessScanCodes(FScope *fScope);

  // Read modifiers from an FScope
  void ProcessModifiers(FScope *fScope);

  // Create a key binding from a key name string and command string
  Bool Create(const char *key, const char *command, U32 flags = 0);

  // Remove a key binding with matching key name
  Bool Remove(const char *key);

  // Return the key name corresponding to the given scan code
  Bool FindKeyByScan(U32 scanCode, const char * &name);

  // Remove all key bindings (except read only ones)
  void RemoveAll();

  // Handle input events
  U32 HandleEvent(Event &e);

  // Update hold bindings
  void Poll();

  // Clear the state of all polled binding, used when the game is deactivated
  void ClearPolled();

  // Display key bindings on console
  void Dump(U32 flags = DUMP_ALL, const char *keyName = NULL);

};


#endif
