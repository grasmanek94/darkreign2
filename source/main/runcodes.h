///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Runcode System
//
// 14-SEP-1998
//


#ifndef __RUNCODE_H
#define __RUNCODE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "utiltypes.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class RunCodes
// 
class RunCodes
{
private:

  LOGDEC

public:

  // Run Codes callback functions
  typedef void (* RCPROC)(void);

  // Notify callback
  typedef U32 (* NOTIFYPROC)(U32);

private:

  struct RunCode
  {
    // Identifier for the runcode
    GameIdent ident;

    // Initializing function for the runcode
    RCPROC    fnInit;

    // Function called after the config is loaded
    RCPROC    fnPost;

    // Shutdown function for the runcode
    RCPROC    fnDone;

    // Process function for the runcode
    RCPROC    fnGame;

    // Notify callback 
    NOTIFYPROC fnNotify;
  };

  // Identifier for the runcodes
  GameIdent ident;

  // Current runcode
  RunCode *currentRC;

  // Next runcode
  RunCode *nextRC;

  // Change runcode
  Bool changeRC;

  // Tree of runcode states
  BinTree<RunCode> runCodes;

  // Run this function once when process is called
  RCPROC runOnceProc;

public:

  // Constructor
  RunCodes(const char *);

  // Run code intialization
  void Register(const char *name, RCPROC fnGame, RCPROC fnInit = NULL, RCPROC fnDone = NULL, RCPROC fnPost = NULL, NOTIFYPROC fnNotify = NULL);

  // Change run code to "s" or rc
  Bool Set(const char *s);
  Bool Set(U32 rc);

  // Clear the runcode
  void Clear();

  // Process
  void Process();

  // Cleanup
  void Cleanup();

  // Get information on the current runcode
  const char *GetCurrent()
  {
    return (currentRC ? currentRC->ident.str : "<NONE>");
  }

  // Get current runcode's CRC
  U32 GetCurrentCrc()
  {
    return (currentRC ? currentRC->ident.crc : 0xDF10EE67); // "<NONE>"
  }

  // Get information on the next runcode
  U32 GetNextCrc()
  {
    return (nextRC ? nextRC->ident.crc : 0xDF10EE67); // "<NONE>"
  }

  // Reset the runcode
  void Reset()
  {
    currentRC = NULL;
    nextRC = NULL;
  }

  // IsClear
  Bool IsClear()
  {
    return ((currentRC || nextRC) ? FALSE : TRUE);
  }

  // SetRunOnceProc
  void SetRunOnceProc(RCPROC proc)
  {
    runOnceProc = proc;
  }
};

#endif