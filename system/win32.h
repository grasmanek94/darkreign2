///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Windows Header
//
// 28-DEC-1998
//


#ifndef __WIN32_H
#define __WIN32_H

#ifndef STRICT
  #define STRICT
#endif
#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif
#ifndef WIN32_EXTRA_LEAN
  #define WIN32_EXTRA_LEAN
#endif

// Ensure that including windows doesn't include "winsock.h"
#define _WINSOCKAPI_


//
// Windows Inclusions
//
#pragma pack(push, _save_win32_h_)
#pragma warning(push)

#include <windows.h>

#pragma warning(pop)
#pragma pack(pop, _save_win32_h_)

#endif