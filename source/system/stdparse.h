///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Standard Token Buffer Parsing Routines
//
// 24-NOV-1997
//

#ifndef __STDPARSE_H
#define __STDPARSE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vnode.h"


///////////////////////////////////////////////////////////////////////////////
//
//  Class StdParse - Contains standard TBuf parsing routines
//

class StdParse
{ 
private:

  // Private parsing methods
  static VNode* ParseNumericVNode(TBuf *tBuf);
  static VNode* ParseStringVNode(TBuf *tBuf);

public:

  // Parse atomic VNode data from 'tBuf', or return NULL if not valid
  static VNode* ParseAtomicVNode(TBuf *tBuf);
};


#endif
