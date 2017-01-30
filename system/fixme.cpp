///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// FIXME
//
// 18-NOV-98
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "fixme.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace FixMe
//
namespace FixMe
{

  // System initialized
  static Bool sysInit = FALSE;

  // Running under debugger?
  static Bool underDebugger;


  //
  // Init
  //
  void Init()
  {
    underDebugger = Debug::UnderDebugger();
    sysInit = TRUE;
  }


  //
  // Done
  //
  void Done()
  {
    sysInit = FALSE;
  }


  // Check a fixme to see if its expired yet
  void Check(const char *file, U32 line, U32 date, const char *user)
  {
    ASSERT(sysInit)

    // Only check if under debugger
    if (!underDebugger)
    {
      return;
    }

    U32 utc = Clock::Date::GetUTC();
    if (utc - date > 2592000)
    {
      utc -= 2592000;
      if (utc - date > 2592000)
      {
        ERR_INDIRECT(file, line, Debug::Error::FATAL, ("FIXME [%s] ... EXPIRED !", user))
      }
      else
      {
        utc -= date;
        utc = 2592000 - utc;
        LOG_INDIRECT(file, line, Log::WARN, ("FIXME [%s] ... %g days remaining", user, ((F32) utc) / 86400.0f))
      }
    }
  }
}

