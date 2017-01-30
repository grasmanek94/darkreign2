///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// GUIDs
//
// 1-OCT-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "guid.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Guid
//
namespace Guid
{

  // {5ABBA866-594C-11d2-81DB-00600895C7EF}
  const GUID Application = 
  { 0x5abba866, 0x594c, 0x11d2, { 0x81, 0xdb, 0x0, 0x60, 0x8, 0x95, 0xc7, 0xef } };

  // Convert GUID into a string
  const char * GetText(GUID *guid)
  {
    static char buf[40];

    Utils::Sprintf(buf, 40, "{%08X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
      guid->Data1, guid->Data2, guid->Data3, 
      guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3], 
      guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);

    return (buf);
  }

}
