///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// meshview.h
//
// 10-JUN-1999
//

#ifndef _MESHVIEW_H
#define _MESHVIEW_H

#include "meshoptions.h"
//----------------------------------------------------------------------------

namespace MeshView
{
  void Init();
  void Process();
  void Done();

  void SetMessage( const char * mess0, const char * mess1 = NULL, const char * mess2 = NULL);

}
//----------------------------------------------------------------------------

#endif      // _MESHVIEW_H