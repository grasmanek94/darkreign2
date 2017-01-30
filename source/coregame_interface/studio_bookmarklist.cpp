///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Bookmark List Editor
//
// 24-NOV-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio_bookmarklist.h"
#include "studio_teameditor.h"
#include "bookmarkobj.h"
#include "iface_util.h"
#include "ifvar.h"
#include "icslider.h"
#include "iface_types.h"
#include "babel.h"
#include "iface_messagebox.h"
#include "bitmap.h"
#include "team.h"
#include "gameobjctrl.h"


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

  //
  // Constructor
  //
  BookmarkList::BookmarkList(IControl *parent) : 
    ICWindow(parent)
  {
    currentBookmark = new IFaceVar(this, CreateString("currentBookmark", ""));
    createBookmark = new IFaceVar(this, CreateString("createBookmark", ""));
    pickBookmark = NULL;
  }


  //
  // Destructor
  //
  BookmarkList::~BookmarkList()
  {
    delete currentBookmark;
    delete createBookmark;
  }


  //
  // HandleEvent
  //
  // Pass any events to the registered handler
  //
  U32 BookmarkList::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          // Do specific handling
          switch (e.iface.p1)
          {
            case 0xA4C4F136: // "OK"
            {
              // Save the pick bookmark if there is one
              if (pickBookmark)
              {
                pickBookmark->SetStringValue(currentBookmark->GetStringValue());
                pickBookmark = NULL;
              }

              // Deactivate this window
              IFace::Deactivate(this);

              break;
            }

            case 0xCCF0EF9B: // "Jump To"
            {
              BookmarkObj *bookMark;

              // Find the bookmark
              if ((bookMark = BookmarkObj::FindBookmark(currentBookmark->GetStringValue())) != NULL)
              {
                bookMark->JumpTo();
              }
              break;
            }

            case 0xAC9184A7: // "Swoop To"
            {
              BookmarkObj *bookMark;

              // Find the bookmark
              if ((bookMark = BookmarkObj::FindBookmark(currentBookmark->GetStringValue())) != NULL)
              {
                bookMark->SwoopTo();
              }
              break;
            }

            case 0x1F2EDF02: // "Set"
            {
              BookmarkObj::CreateBookmark(currentBookmark->GetStringValue(), BookmarkObj::POSITION);
              break;
            }

            case 0x74EF9BE3: // "Destroy"
            {
              BookmarkObj *bookMark;

              // Find the bookmark
              if ((bookMark = BookmarkObj::FindBookmark(currentBookmark->GetStringValue())) != NULL)
              {
                // Delete it
                GameObjCtrl::MarkForDeletion(bookMark);

                // Rebuild the bookmark list
                BuildList();
              }

              break;
            }

            case 0x4C6225C9: // "CreateBookmark"
            {
              BookmarkObj::CreateBookmark(createBookmark->GetStringValue(), BookmarkObj::POSITION);

              // Rebuild the bookmark list
              BuildList();

              break;
            }

            default : 
              ICWindow::HandleEvent(e);
              break;
          }

          return (TRUE);
        }
      }
    }

    return (ICWindow::HandleEvent(e));  
  }


  //
  // Activate
  //
  // Activate this control
  //
  Bool BookmarkList::Activate()
  {
    if (ICWindow::Activate())
    {
      // Find all the important controls
      bookmarkList = IFace::Find<ICListBox>("BookmarkList", this);
      if (!bookmarkList.Alive())
      {
        ERR_CONFIG(("Could not find 'BookmarkList' in the BookmarkList"))
      }

      // Build the list of bookmarks
      BuildList();

      ActivateVar(currentBookmark);
      ActivateVar(createBookmark);

      // If there's a pick bookmark, then select it
      if (pickBookmark)
      {
        bookmarkList->SetSelectedItem(pickBookmark->GetStringValue());
      }
      
      return (TRUE);
    }

    return (FALSE);
  }


  //
  // Deactivate
  //
  // Deactivate this control
  //
  Bool BookmarkList::Deactivate()
  {
    if (ICWindow::Deactivate())
    {
      currentBookmark->Deactivate();
      createBookmark->Deactivate();

      return (TRUE);
    }

    return (FALSE);
  }


  //
  // BookmarkList::BuildList
  //
  void BookmarkList::BuildList()
  {
    ASSERT(bookmarkList.Alive())
    bookmarkList->DeleteAllItems();

    for (NBinTree<BookmarkObj>::Iterator i(&BookmarkObj::allBookmarks); *i; i++)
    {
      if (!(*i)->deathNode.InUse())
      {
        if ((*i)->GetType() == BookmarkObj::POSITION)
        {
          bookmarkList->AddTextItem((*i)->GetName(), NULL, 0);
        }
      }
    }
  }


  //
  // BookmarkList::SetPickBookmark
  //
  // Set the pick bookmark
  //
  void BookmarkList::SetPickBookmark(IFaceVar *pick)
  {
    pickBookmark = pick;
  }

}
