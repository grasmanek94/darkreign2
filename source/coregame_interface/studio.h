///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 11-FEB-1999
//

#ifndef __STUDIO_H
#define __STUDIO_H


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Studio - Mission creation environment
//
namespace Studio
{
  // 
  // Conceptually the studio should not expose any more than these four
  // functions.  This is very important! (spaghetti verses meatballs :)
  //
  void Init();
  void PostInit();
  void Done();
  void Process();
}

#endif