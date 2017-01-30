///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Console Command System
//
// 15-MAY-1998
//

#ifndef __CONSOLE_H
#define __CONSOLE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "tbuf.h"
#include "varsys.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
#define CONSOLE(x, y)   \
{                       \
  Console::SetType(x);  \
  Console::Message y ;  \
}

#define CON_DIAG(x) CONSOLE(0x3CC9CC9E, x ) // "Diag"
#define CON_ERR(x)  CONSOLE(0xC21C10D7, x ) // "Error"
#define CON_MSG(x)  CONSOLE(0xB8B548C0, x ) // "Message"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Console - System for handling the processing of console strings
//
namespace Console
{

  enum StdErrorTypes
  {
    // Invalid arguments
    ARGS,

    // Command unavailable at this time
    UNAVAILABLE,
  };

  // Used for scope display method
  enum DisplayTypes
  {
    // Flags for showing items
    SHOWSCOPES = 0x0001,
    SHOWCMDS   = 0x0002,
    SHOWVARS   = 0x0004,

    // Do not descend into scopes
    NORECURSE  = 0x0008,

    // Show all items
    SHOWALL    = (SHOWSCOPES | SHOWCMDS | SHOWVARS)
  };


  // Initialise system
  void Init();

  // Shutdown system
  void Done();

  // Process a console command string
  void ProcessCmd(const char *cmd, void *context = NULL);

  // Add a command history recall list
  void AddCmdHist(const char *s, Bool echo = TRUE);

  // Set the type of the messages to be displayed
  void SetType(U32 type);

  // Display a console message
  void CDECL Message(const char *format, ...);

  // Display a console message
  void CDECL Message(const CH *format, ...);

  // Display a standard console error string for type 'err'
  void Message(StdErrorTypes err);

  // Clear the scrollback buffer
  void ClearScrollBack();

  // Displays the contents of a VarSys item
  void DisplayVarItem(VarSys::VarItem *item, U32 indent, U32 flags = SHOWALL);
  
  // Displays the contents of a VarSys scope
  void DisplayVarScope(VarSys::VarScope *scope, U32 indent, U32 flags = SHOWALL);

  // Construct an argument name 
  void MakeArgName(U32 stack, VarPathIdent &path, U32 argCount);

  // Construct an argument count name
  void MakeArgCount(U32 stack, VarPathIdent &path);

  // Constrcut an argument offset name
  void MakeArgOffset(U32 stack, VarPathIdent &path, U32 offset);

  // Returns the current argument count
  U32 ArgCount();

  // Get the var scope of an argument
  Bool GetArg(U32 index, VarSys::VarItem * &val);

  // Get an integer argument, FALSE if not found or not string
  Bool GetArgInteger(U32 index, S32 &val);

  // Get an String argument, FALSE if not found or not string
  Bool GetArgString(U32 index, const char * &val);

  // Get an Float argument, FALSE if not found or not Float
  Bool GetArgFloat(U32 index, F32 &val);

  // Get a Scope argument, FALSE if not found or not a scope
  Bool GetArgScope(U32 index, const VarSys::VarItem * &val);

  // Get the entire command line as a string (only works inside handlers)
  const char * GetCmdString();

  // Get the entire command line as a string from arg 'index' (only works insider handlers)
  const char * GetCmdString(U32 index);


  //
  // Scroll-back buffer
  //

  typedef Bool (ConsoleHookProc)(const CH *, U32 &, void *);

  // Add string to the console
  void AddString(const char *str, U32 id);

  // Add a unicode string to the console
  void AddString(const CH *str, U32 id);

  // Enumerate all matching items in the display
  void EnumStrings(BinTree<U32> *filter, ConsoleHookProc *proc, void *context);

  // Register a callback for recieving console messages
  void RegisterMsgHook(BinTree<U32> *filter, ConsoleHookProc *proc, void *context);

  // Unregister the callback
  void UnregisterMsgHook(ConsoleHookProc *proc, void *context);

  // Converts console messages from one type to another
  void ConvertMessages(U32 from, U32 to);


  //
  // History
  //

  // History recall list
  typedef List<char> HistList;

  // History recall list iterator
  typedef List<char>::Iterator HistIterator;

  // Build a list of matching commands
  Bool BuildHistoryRecallList(const char *mask, HistList &list, HistIterator &iterator, Bool head);


  //
  // Var completion
  //

  // Var completion list typedef
  typedef List<VarPathIdent> VCList;

  // Var completion list iterator typedef
  typedef List<VarPathIdent>::Iterator VCIterator;

  // Build a list of matching var items
  Bool BuildVarCompletionList(const char *mask, VCList &list, VCIterator &iterator, Bool head);

};

#endif