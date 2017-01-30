///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// FootPrint Editor
//
// 24-NOV-1998
//


#ifndef __STUDIO_BOOKMARKLIST_H
#define __STUDIO_BOOKMARKLIST_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icwindow.h"
#include "iclistbox.h"


///////////////////////////////////////////////////////////////////////////////
//
// Forward Declarations
//
class Team;


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
  class BookmarkList : public ICWindow
  {
    PROMOTE_LINK(BookmarkList, ICWindow, 0x0013DE4B) // "BookmarkList"

  protected:

    // Bookmark list
    ICListBoxPtr bookmarkList;

    // Current bookmark
    IFaceVar *currentBookmark;

    // Create bookmark
    IFaceVar *createBookmark;

    // Var to use when picking bookmarks
    IFaceVar *pickBookmark;

  public:

    // Constructor
    BookmarkList(IControl *parent);
    ~BookmarkList();

    // Event handling
    U32 HandleEvent(Event &e);

    // Activate and deactivate this control
    Bool Activate();
    Bool Deactivate();

    // Build the list of bookmarks
    void BuildList();

    // Set the pick bookmark
    void SetPickBookmark(IFaceVar *pick);

  };

}

#endif
