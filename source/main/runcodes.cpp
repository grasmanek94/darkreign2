///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Runcode System
//
// 14-SEP-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "runcodes.h"
#include "main.h"


///////////////////////////////////////////////////////////////////////////////
//
// Logging
//
LOGDEF(RunCodes, "RunCodes")


///////////////////////////////////////////////////////////////////////////////
//
// Class RunCodes
// 


//
// RunCodes::RunCodes
//
RunCodes::RunCodes(const char *name) 
: ident(name), 
  currentRC(NULL), 
  nextRC(NULL), 
  changeRC(FALSE),
  runOnceProc(NULL)
{
}


//
// RunCodes::Register
//
// Run code intialization
//
void RunCodes::Register(const char *name, RCPROC fnGame, RCPROC fnInit, RCPROC fnDone, RCPROC fnPost, NOTIFYPROC fnNotify)
{
  RunCode *r = new RunCode;

  r->ident  = name;
  r->fnInit = fnInit;
  r->fnPost = fnPost;
  r->fnDone = fnDone;
  r->fnGame = fnGame;
  r->fnNotify = fnNotify;

  // Make sure a runcode with the same id has not already been registered
  RunCode *prev;
  if ((prev = runCodes.Find(r->ident.crc)) != NULL)
  {
    ERR_FATAL(("[%s] Runcode [%s] already registered (as [%s])", ident.str, r->ident.str, prev->ident.str));
  }

  runCodes.Add(r->ident.crc, r);
}


//
// RunCodes::Set
//
Bool RunCodes::Set(const char *s)
{
  Bool rVal;

  ASSERT(s);

  if ((rVal = Set(Crc::CalcStr(s))) != FALSE)
  {
//    LOG_DIAG(("[%s] Setting runcode to [%s]", ident.str, s))
  }
  else
  {
    LOG_ERR(("[%s] Run code [%s] does not exist", ident.str, s))
  }
  return (rVal);
}


//
// RunCodes::Set
//
Bool RunCodes::Set(U32 rc)
{
  RunCode *r = runCodes.Find(rc);

  if (r)
  {
    nextRC = r;
    changeRC = TRUE;
    return (TRUE);
  }
  return (FALSE);
}


//
// RunCodes::Clear
//
void RunCodes::Clear()
{
  // Initialize the new run code
  LOG_DIAG(("[%s] Clearing runcode [%s]", ident.str, currentRC ? currentRC->ident.str : "<NONE>"));

  // Shutdown the current run code
  if (currentRC && currentRC->fnDone)
  {
    currentRC->fnDone();
  }

  // Clear the current runcode
  currentRC = NULL;
}


//
// RunCodes::Process
// 
void RunCodes::Process()
{
  char buf[256];

  // Run the optional run once function
  if (runOnceProc)
  {
    runOnceProc();
    runOnceProc = NULL;
  }

  // Run code change was requested
  if (changeRC)
  {
    // Ensure we are changing to a valid run code
    if (nextRC == NULL)
    {
      ERR_FATAL(("[%s] Run code is NULL", ident.str));
    }

    // Check with current runcode if we are allowed to leave it
    // However, we are always allowed to quit
    if (currentRC && currentRC->fnNotify && (nextRC->ident.crc != 0xB4729720)) // "Quit"
    {
      if (!currentRC->fnNotify(0x2F312211)) // "CanLeave"
      {
        changeRC = FALSE;
      }
    }
  }

  if (changeRC)
  {
    // Shutdown the current run code
    if (currentRC && currentRC->fnDone)
    {
      currentRC->fnDone();
    }

    // Changed over ok
    currentRC = nextRC;
    nextRC    = NULL;
    changeRC  = FALSE;

    // Initialize the new run code
    LOG_DIAG(("[%s] Entering run code [%s]", ident.str, currentRC->ident.str));

    if (currentRC->fnInit)
    {
      currentRC->fnInit();
    }

    // Execute post-init cfg file (does not need to exist)
    Utils::Sprintf(buf, 256, "runcode_%s_%s.cfg", ident.str, currentRC->ident.str);
    Main::Exec(buf, Main::ScopeHandler, FALSE);

    // And optional user post-init cfg file
    Utils::Sprintf(buf, 256, "user_%s_%s.cfg", ident.str, currentRC->ident.str);
    Main::Exec(buf, Main::ScopeHandler, FALSE);

    // If there's a post load function then go with it
    if (currentRC->fnPost)
    {
      currentRC->fnPost();
    }
  }
  else
  {
    // Execute current runcode processing function
    ASSERT(currentRC);
    if (!currentRC->fnGame)
    {
      ERR_FATAL(("[%s] No process function for [%s] to be processed!", ident.str, currentRC->ident.str))
    }
  
    currentRC->fnGame();
  }
}


//
// RunCodes::Cleanup
//
void RunCodes::Cleanup()
{
  runCodes.DisposeAll();
}


