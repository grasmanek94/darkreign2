///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// FootPrint Editor
//
// 24-NOV-1998
//


#ifndef __STUDIO_BOOKMARKLISTBOX_H
#define __STUDIO_BOOKMARKLISTBOX_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "iclistbox.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Studio
//
namespace Studio
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class BookmarkList
  //
  class BookmarkListBox : public ICListBox
  {
    PROMOTE_LINK(BookmarkListBox, ICListBox, 0x1865D2EF); // "BookmarkListBox"

  public:

    // Constructor
    BookmarkListBox(IControl *parent) : ICListBox(parent) {}

    // Event handling
    U32 HandleEvent(Event &e);

  };
}

#endif
