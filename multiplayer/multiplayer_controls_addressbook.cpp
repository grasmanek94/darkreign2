///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
// 1-JUL-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "multiplayer_controls_addressbook.h"
#include "multiplayer_addressbook.h"
#include "icstatic.h"
#include "stdload.h"
#include "iface.h"
#include "font.h"
#include "iface_priv.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Controls
  //
  namespace Controls
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class AddressBook::Item
    //
    class AddressBook::Item : public ICStatic
    {
      PROMOTE_LINK(AddressBook::Item, IControl, 0x9B7D101C); // "AddressBook::Item"

    public:

      // The Address book
      AddressBook &addressBook;

      // The Address entry
      MultiPlayer::AddressBook::Entry &entry;
      
      // Constructor
      Item(AddressBook &addressBook, MultiPlayer::AddressBook::Entry &entry, IControl *parent) 
      : ICStatic(parent),
        addressBook(addressBook),
        entry(entry)
      {
        SetColorGroup(IFace::data.cgListItem);
      }

      // Redraw self
      void DrawSelf(PaintInfo &pi)
      {
        DrawCtrlBackground(pi);

        const char *ch;
        U32 vCentre = (pi.client.Height() - pi.font->Height()) / 2;

        // Write the address name
        ch = entry.GetName().str;
        pi.font->Draw
        (
          pi.client.p0.x + addressBook.offsetName, 
          pi.client.p0.y + vCentre,
          Utils::Ansi2Unicode(ch),
          Utils::Strlen(ch),
          pi.colors->fg[ColorIndex()],
          &pi.client
        );

        char buffer[300];

        // Is the port non zero ?
        if (entry.GetPort())
        {
          ch = buffer;
          Utils::Sprintf(buffer, 300, "%s:%d", entry.GetLocation().str, entry.GetPort());
        }
        else
        {
          ch = entry.GetLocation().str;
        }

        // Write the address address
        pi.font->Draw
        (
          pi.client.p0.x + addressBook.offsetLocation,
          pi.client.p0.y + vCentre,
          Utils::Ansi2Unicode(ch),
          Utils::Strlen(ch),
          pi.colors->fg[ColorIndex()],
          &pi.client
        );

      }

    };

    
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class AddressBook
    //


    //
    // Constructor
    //
    AddressBook::AddressBook(IControl *parent)
    : ICListBox(parent)
    {
      offsetName = 5;
      offsetLocation = 200;
    }


    //
    // Destructor
    //
    AddressBook::~AddressBook()
    {
    }


    //
    // Activate
    //
    Bool AddressBook::Activate()
    {
      if (ICListBox::Activate())
      {
        return (TRUE);
      }
      return (FALSE);
    }


    //
    // Deactivate
    //
    Bool AddressBook::Deactivate()
    {
      if (ICListBox::Deactivate())
      {
        return (TRUE);
      }
      return (FALSE);
    }


    //
    // Setup
    //
    void AddressBook::Setup(FScope *fScope)
    {
      switch (fScope->NameCrc())
      {
        case 0xD6ABAE33: // "OffsetName"
          offsetName = StdLoad::TypeU32(fScope);
          break;

        case 0xC9A77AD3: // "OffsetLocation"
          offsetLocation = StdLoad::TypeU32(fScope);
          break;

        default:
          ICListBox::Setup(fScope);
          break;
      }
    }


    //
    // Event handling function
    //
    U32 AddressBook::HandleEvent(Event &e)
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
                RebuildList();
                break;

              case AddressBookMsg::Remove:
              {
                // Get the currently selected item
                IControl *ctrl = GetSelectedItem();
                if (ctrl)
                {
                  Item *item = IFace::Promote<Item>(ctrl, TRUE);
                  MultiPlayer::AddressBook::RemoveEntry(item->entry.GetName());
                }
                MultiPlayer::AddressBook::SaveToUser();
                RebuildList();
                break;
              }

              case AddressBookMsg::Add:
              {
                VarString name("$.name", e.iface.from);
                VarString address("$.address", e.iface.from);
                VarInteger port("$.port", e.iface.from);
                MultiPlayer::AddressBook::UpdateEntry(*name, *address, U16(port));
                MultiPlayer::AddressBook::SaveToUser();
                RebuildList();
                break;
              }

              case AddressBookMsg::Upload:
              {
                VarString name("$.name", e.iface.from);
                VarString address("$.address", e.iface.from);
                VarInteger port("$.port", e.iface.from);

                // Get the currently selected item
                IControl *ctrl = GetSelectedItem();
                if (ctrl)
                {
                  Item *item = IFace::Promote<Item>(ctrl, TRUE);

                  name = item->entry.GetName().str;
                  address = item->entry.GetLocation().str;
                  port = item->entry.GetPort();
                }
                break;
              }

              case AddressBookMsg::Edit:
              {
                VarString name("$.name", e.iface.from);
                VarString address("$.address", e.iface.from);
                VarInteger port("$.port", e.iface.from);
                MultiPlayer::AddressBook::UpdateEntry(*name, *address, U16(port));
                MultiPlayer::AddressBook::SaveToUser();
                break;
              }
            }
          }
        }
      }
      return (ICListBox::HandleEvent(e));
    }


    //
    // Rebuild the list
    //
    void AddressBook::RebuildList()
    {
      Rebuild *rebuild = PreRebuild();
      DeleteAllItems();

      for (NBinTree<MultiPlayer::AddressBook::Entry>::Iterator e(&MultiPlayer::AddressBook::GetEntries()); *e; ++e)
      {
        Item *item = new Item(*this, **e, this);
        AddItem((*e)->GetName().str, item);
      }

      PostRebuild(&rebuild);
    }
  }

}
