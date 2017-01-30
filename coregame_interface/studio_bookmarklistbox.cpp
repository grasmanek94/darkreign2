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
#include "studio_bookmarklistbox.h"
#include "bookmarkobj.h"
#include "iface.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Studio
//
namespace Studio
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class BookmarkListBox
  //


  //
  // HandleEvent
  //
  // Pass any events to the registered handler
  //
  U32 BookmarkListBox::HandleEvent(Event &e)
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
            case ICListBoxMsg::Rebuild:
            {
              // Rebuild the list
              DeleteAllItems();

              for (NBinTree<BookmarkObj>::Iterator i(&BookmarkObj::allBookmarks); *i; i++)
              {
                if (!(*i)->deathNode.InUse())
                {
                  if ((*i)->GetType() == BookmarkObj::CURVE)
                  {
                    AddTextItem((*i)->GetName(), NULL);
                  }
                }
              }
              // Handled
              return (TRUE);
            }
          }

          // Not handled
          break;
        }
      }
    }
    return (ICListBox::HandleEvent(e));  
  }
}
