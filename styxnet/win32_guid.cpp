///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1999
// Matthew Versluys
//
// Win32 GUID
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "win32_guid.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Win32
//
namespace Win32
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace GUID
  //
  namespace GUID
  {

    //
    // GUID2Text
    //
    // Convert GUID into a string
    //
    const char * GUID2Text(const ::GUID &guid)
    {
      static char buf[40];

      Utils::Sprintf(buf, 40, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        guid.Data1, guid.Data2, guid.Data3, 
        guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], 
        guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

      return (buf);
    }


    //
    // Text2GUID
    //
    // Convert string into a GUID
    //
    const ::GUID & Text2GUID(const char *text)
    {
      static ::GUID guid;

      sscanf(text, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        &guid.Data1, &guid.Data2, &guid.Data3, 
        &guid.Data4[0], &guid.Data4[1], &guid.Data4[2], &guid.Data4[3], 
        &guid.Data4[4], &guid.Data4[5], &guid.Data4[6], &guid.Data4[7]);

      return (guid);
    }
  }
}
