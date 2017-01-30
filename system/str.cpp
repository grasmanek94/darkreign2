///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// String Class
// 1-DEC-1997
//


///////////////////////////////////////////////////////////////////////////////
//
// Class String
//

#include "str.h"


//
// Make
//
String CDECL String::Make(const char *format, ...)
{
  String str(512);
  va_list args;

  va_start(args, format);
  str.ref->length = vsprintf(str.ref->data, format, args);
  va_end(args);
  ASSERT(str.ref->length < 512)

  return (str);
}
