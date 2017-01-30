///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tag listbox
//
// 24-NOV-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio_taglist.h"
#include "iface.h"
#include "iface_types.h"
#include "tagobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Studio
//
namespace Studio
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class TagListBox
  //

  //
  // Constructor
  //
  TagListBox::TagListBox(IControl *parent) : ICListBox(parent)
  {
  }


  //
  // Destructor
  //
  TagListBox::~TagListBox()
  {
  }


  //
  // Build the list of bookmarks
  //
  U32 TagListBox::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          U32 id = e.iface.p1;

          switch (id)
          {
            case ICListBoxMsg::Rebuild:
            {
              // Rebuild the list
              DeleteAllItems();

              for (NBinTree<TagObj>::Iterator i(&TagObj::allTags); *i; i++)
              {
                if (!(*i)->deathNode.InUse())
                {
                  AddTextItem((*i)->TagName(), NULL);
                }
              }

              // Handled
              return (TRUE);
            }
          }
        }
      }
    }

    return (ICListBox::HandleEvent(e));
  }
}
