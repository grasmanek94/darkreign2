///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Bink Inclusions
//
// 14-OCT-1999
//

#ifndef __DRBINK_H
#define __DRBINK_H

#ifndef STRICT
  #define STRICT
#endif
#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif
#ifndef WIN32_EXTRA_LEAN
  #define WIN32_EXTRA_LEAN
#endif

#pragma pack(push, _save_bink_h_)
#pragma warning(push)

//#include <windows.h>

#pragma warning(pop)
#pragma warning(push)

//#include "..\3rdparty\bink\rad.h"
#include "..\3rdparty\bink\bink.h"

#pragma warning(pop)
#pragma pack(pop, _save_bink_h_)

#pragma comment(lib, "..\\3rdparty\\bink\\binkw32")

#endif // DRBINK_H