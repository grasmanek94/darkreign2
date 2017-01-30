///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// FIXME
//
// 18-NOV-98
//

#ifndef __FIXME
#define __FIXME


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
#ifndef DEVELOPMENT
  #define FIXME(date, user)
#else
  #define FIXME(date, user) FixMe::Check(__FILE__, __LINE__, date, user)
#endif


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace FixMe
//
namespace FixMe
{
  // Initialize
  void Init();
  void Done();

  // Check a fixme to see if its expired yet
  void Check(const char *file, U32 line, U32 date, const char *user);
}


#endif