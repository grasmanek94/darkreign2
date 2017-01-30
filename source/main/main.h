///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Main startup code
//
// 07-APR-1998
//


#ifndef __MAIN_H
#define __MAIN_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "event.h"
#include "varsys.h"
#include "ptree.h"
#include "filesys.h"
#include "runcodes.h"
#include "win32.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Main - Application framework and run-code management
//
namespace Main
{

  typedef StrCrc<32> CmdLineArg;
  typedef StrCrc<512> CmdLineVal;

  // Command scope handler type
  typedef void (ScopeHandlerType)(FScope *);

  // Root level scope handler function
  typedef void (RootScopeHandler)(FScope *);

  // GUI event hook
  typedef void (GUIHookProc)(U32 id, UINT msg, WPARAM wParam, LPARAM lParam);

  // Commald line handler type
  typedef void (CmdLineHookProc)(const CmdLineArg &arg, const CmdLineVal &val);
  
  // Application state
  extern Bool active;

  // Frame rate counter
  extern int frameCount;

  // increments every frame
  extern U32 frameNumber;   

  extern U32 thisTime;
  extern U32 lastTime;
  extern U32 elapTime;
  extern U32 elapLast;
  extern F32 elapSecs;

  extern VarFloat   elapFrame;
  extern VarInteger elapCap;
  extern VarInteger frameRate;
  extern VarInteger triCount;
  extern VarInteger triPerSec;

  // Initial video mode
  extern Bool vidModeSet;
  extern U32 vidModeX;
  extern U32 vidModeY;

  // Don't enable floating exceptions
  extern Bool fpuExceptions;

  // Profiler running?
  extern Bool profileOn;

  // Runcode object
  extern RunCodes runCodes;


  // Init/Shutdown of game
  void Init(HINSTANCE hInst, const char *cmd);
  void Done();

  // Init/Shutdown of low level systems (debug, logging, mono)
  void LowLevelSystemInit();
  void LowLevelSystemDone();

  // Init/Shutdown of core systems (sound, video, file system)
  void CoreSystemInit( Bool doFullScreen = TRUE);
  void CoreSystemDone();

  // Init/Shutdown of Command handler
  void CmdInit();
  void CmdDone();

  // timer value get functions
  F32 ElapTimeFrame();
  F32 ElapTimeGame();

  // Start Execution of game
  void CDECL Start();

  // Shut down the game
  void Quit();

  // Set the name of the executable to execute when we are shutting down
  void RegisterNextProcess(const char *file);

  // Handler for the standard command scope
  void ScopeHandler(FScope *fScope);

  // Feeds all sub-scopes into the supplied handler
  void ProcessCmdScope(FScope *fScope, ScopeHandlerType *func = ScopeHandler);

  // Execute a configuration file and send each scope to the supplied handler
  Bool Exec(const char *name, ScopeHandlerType *func = ScopeHandler, Bool required = TRUE);

  // Register a root level scope handler
  void RegisterRootScope(const char *name, RootScopeHandler *func);
  void UnregisterRootScope(const char *name, RootScopeHandler *func);

  // Register a GUI event hook function
  void RegisterGUIHook(const char *name, GUIHookProc *proc);
  void UnregisterGUIHook(const char *name, GUIHookProc *proc);

  // Register a Command line handler
  void RegisterCmdLineHandler(const char *hook, CmdLineHookProc *proc);
  void UnregisterCmdLineHandler(const char *hook);

  // Turn background processing on or off (while window is inactive)
  void SetBackgroundProcessing(Bool f);

  // Process windows messages
  void MessagePump();

  LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

  // Utility function to create a game window
  HWND CreateGameWindow(const char *title);

  // Get the window handle of the game window
  HWND GetGameWindow();

  // Add a command to the list of commands run at the next runcode chage
  void AddRunOnceCmd(const char *str);

  // EXTERNALLY DEFINED : window creation function.
  // If no specific behavior is required simply call "Main::CreateGameWindow"
  HWND CreateMainWindow();

  // EXTERNALLY DEFINED : called to execute the initial config file
  void ExecInitialConfig();

};


#endif
