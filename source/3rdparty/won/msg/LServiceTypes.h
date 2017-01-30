#ifndef _LServiceTypes_H_
#define _LServiceTypes_H_

//
// LargeMessage Service Types
// These values must be fixed in order to communiucate message
// structures to client.  For this reason, each enum value except
// for 'max' must have an assigned value.

// NEVER change an existing enum value and always add new values to
// the end!!!!

#include <limits.h>

namespace WONMsg 
{
	enum LargeServiceType
	{
		LargeUndefined       = 0,

		LargeCommonService   = 1,

		// Let's reserve some services for clients (game use)
		MaxLargeServiceType  = ULONG_MAX-10000
	};
};

#endif