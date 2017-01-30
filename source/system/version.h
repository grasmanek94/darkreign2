///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Version System
//
// 18-MAR-1998
//


#ifndef __VERSION_H
#define __VERSION_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "std.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Version
//
namespace Version
{

  void Init();
  void Done();

  U32 GetBuildCRC();
  const char * GetBuildString();
  const char * GetBuildDate();    
  const char * GetBuildTime();    
  const char * GetBuildUser();    
  const char * GetBuildMachine(); 
  const char * GetBuildOS(); 
  const char * GetBuildDefs();
  const char * GetBuildVersion();
  U32 GetBuildNumber();

}


#endif

