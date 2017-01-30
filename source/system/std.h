///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Standard Header
// 26-NOV-1997
//


#ifndef __STD_H
#define __STD_H


//
// Application wide preprocessor directives
//
#include "../settings.h"


//
// Some funky defines
//
#include "defines.h"


//
// Includes for Precompilitaion
//

#pragma warning(push)

// Push packing into a var since windoze doesn't
// preserve packing and even better  than that 
// somehow messes up the packing stack
#pragma pack(push, _packing_)

#include "win32.h"
#include <mmsystem.h>
#include "dxinc.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <conio.h>

#pragma warning(pop)
#pragma pack(pop, _packing_)

// Debug (Exceptions, Assertions, Errors)
#include "debug.h"

// List Template
#include "list.h"

// Noded List Template
#include "nlist.h"

// Tree Template
#include "tree.h"

// Noded Tree Template
#include "ntree.h"

// Stack Template
#include "stack.h"

// CRC (core of the game)
#include "crc.h"
     
// Utilities
#include "utils.h"

// String Buffer (Use instad of String)
#include "strbuf.h"

// String CRC (Use instead of String /w CRC support)
#include "strcrc.h"

// String Class (Ugh, kill it and use std::string one day)
#include "str.h"

// Mono
#include "mono.h"

// Logging
#include "log.h"

// Clock (Time, Date)
#include "clock.h"

// FIXME functions
#include "fixme.h"


#endif

