///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Miles Sound System Inclusions
//
// 15-JUN-1998
//

#ifndef __MSS_H
#define __MSS_H

#ifndef STRICT
  #define STRICT
#endif
#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif
#ifndef WIN32_EXTRA_LEAN
  #define WIN32_EXTRA_LEAN
#endif

#pragma pack(push, _save_mss_h_)
#pragma warning(push)

#include <windows.h>

#pragma warning(pop)
#pragma warning(push)

#include "..\3rdparty\mss\mss.h"

#pragma warning(pop)
#pragma pack(pop, _save_mss_h_)

#pragma comment(lib, "..\\3rdparty\\mss\\mss32")

#endif