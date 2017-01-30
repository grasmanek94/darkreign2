/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Key viewer
//
// 23-MAY-2000
//


#ifndef __ICKEYVIEW_H
#define __ICKEYVIEW_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icwindow.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class KeyViewer
//
class KeyViewer : public ICWindow
{
  PROMOTE_LINK(KeyViewer, ICWindow, 0x66FD3C39) // "KeyViewer"

public:

  // Constructor
  KeyViewer(IControl *parent);

  // Event handler
  U32 HandleEvent(Event &e);

};

#endif
