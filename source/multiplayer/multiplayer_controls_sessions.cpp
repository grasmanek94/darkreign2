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
#include "multiplayer_controls_sessions.h"
#include "multiplayer_addressbook.h"
#include "multiplayer_settings.h"
#include "multiplayer.h"
#include "iface.h"
#include "iface_types.h"
#include "iface_util.h"
#include "font.h"
#include "icstatic.h"
#include "babel.h"
#include "iface_priv.h"
#include "stdload.h"


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
    // Class Sessions::Item
    //
    class Sessions::Item : public ICStatic
    {
      PROMOTE_LINK(Sessions::Item, IControl, 0x76A1B810); // "Sessions::Item"

    public:

      // The Session list (offsets are in here!)
      Sessions &sessionList;

      // The Session
      StyxNet::Explorer::Session &session;

      // Constructor
      Item(Sessions &sessionList, StyxNet::Explorer::Session &session, IControl *parent) 
      : ICStatic(parent),
        sessionList(sessionList),
        session(session)
      { 
        SetColorGroup(IFace::data.cgListItem);
      }

      // Redraw self
      void DrawSelf(PaintInfo &pi)
      {
        DrawCtrlBackground(pi);

        U32 vCentre = (pi.client.Height() - pi.font->Height()) / 2;

        // Is this session locked
        if (session.flags & StyxNet::SessionFlags::Password)
        {
          TextureInfo *icon = &sessionList.iconLocked;
          S32 midY = (pi.client.p0.y + pi.client.p1.y) >> 1;

          if (icon->texture)
          {
            // Add launch ready icon
            IFace::RenderRectangle
            (
              ClipRect(
                pi.client.p0.x + sessionList.offsetLocked,
                midY - (icon->pixels.Height() >> 1),
                pi.client.p0.x + sessionList.offsetLocked + icon->pixels.Width(),
                midY - (icon->pixels.Height() >> 1) + icon->pixels.Height()),
              Color(1.0f, 1.0f, 1.0f), 
              icon,
              pi.alphaScale
            );
          }
        }

        const char * ch;

        /*

        // Find an entry by address
        const AddressBook::Entry *entry = AddressBook::FindEntryByAddress(session.address);

        if (entry)
        {
          ch = entry->GetName().str;
        }
        else
        {
          ch = "#multiplayer.common.lan";
        }

        // Write the address
        const CH *t = TRANSLATE((ch));
        pi.font->Draw
        (
          pi.client.p0.x + sessionList.offsetAddress, 
          pi.client.p0.y + vCentre,
          t,
          Utils::Strlen(t),
          pi.colors->fg[ColorIndex()],
          &pi.client
        );

        */

        ClipRect nameClip
        (
          pi.client.p0.x + sessionList.offsetName,
          pi.client.p0.y,
          pi.client.p0.x + sessionList.offsetName + sessionList.widthName,
          pi.client.p1.y
        );

        // Write the session name
        ch = session.name.str;
        pi.font->Draw
        (
          pi.client.p0.x + sessionList.offsetName, 
          pi.client.p0.y + vCentre,
          Utils::Ansi2Unicode(session.name.str),
          Utils::Strlen(session.name.str),
          pi.colors->fg[ColorIndex()],
          &nameClip
        );

        // Write the num users and the max users
        CH buffer[32];
        Utils::Sprintf(buffer, 32, L"%d/%d", session.numUsers, session.maxUsers);
        pi.font->Draw
        (
          pi.client.p0.x + sessionList.offsetNumber,
          pi.client.p0.y + vCentre,
          buffer,
          Utils::Strlen(buffer),
          pi.colors->fg[ColorIndex()],
          &pi.client
        );

      }

    };

    
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Sessions
    //


    //
    // Constructor
    //
    Sessions::Sessions(IControl *parent)
    : ICListBox(parent)
    {
      offsetAddress = 16;
      widthAddress = 150;
      offsetName = 150;
      widthName = 150;
      offsetNumber = 300;
      offsetLocked = 0;

      // Setup the poll interval
      pollInterval = 2000;
      nextPollTime = pollInterval;
    }


    //
    // Destructor
    //
    Sessions::~Sessions()
    {
      sessions.DisposeAll();
    }


    //
    // Activate
    //
    Bool Sessions::Activate()
    {
      if (ICListBox::Activate())
      {
        AddressBook::LoadFromUser();
        return (TRUE);
      }
      return (FALSE);
    }


    //
    // Deactivate
    //
    Bool Sessions::Deactivate()
    {
      if (ICListBox::Deactivate())
      {
        return (TRUE);
      }
      return (FALSE);
    }


    //
    // Poll
    //
    void Sessions::Poll()
    {
      // Tell the explorer to broadcast a search for sessions
      explorer.Broadcast(Settings::GetPort());
      
      // Check all of the servers in our address book
      for (NBinTree<AddressBook::Entry>::Iterator e(&AddressBook::GetEntries()); *e; ++e)
      {
        Win32::Socket::Address address;
        if ((*e)->GetAddress(address))
        {
          explorer.Unicast(address);
        }
      }
    }


    //
    // Setup
    //
    void Sessions::Setup(FScope *fScope)
    {
      switch (fScope->NameCrc())
      {
        case 0x82ACDEED: // "IconLocked"
          IFace::FScopeToTextureInfo(fScope, iconLocked);
          break;

        case 0xC5183834: // "OffsetAddress"
          offsetName = StdLoad::TypeU32(fScope);
          break;

        case 0x16E37A45: // "WidthAddress"
          widthName = StdLoad::TypeU32(fScope);
          break;

        case 0xD6ABAE33: // "OffsetName"
          offsetName = StdLoad::TypeU32(fScope);
          break;

        case 0x4F76FDA5: // "WidthName"
          widthName = StdLoad::TypeU32(fScope);
          break;

        case 0x0491C000: // "OffsetNumber"
          offsetNumber = StdLoad::TypeU32(fScope);
          break;

        case 0x026FBE4B: // "OffsetLocked"
          offsetLocked = StdLoad::TypeU32(fScope);
          break;

        default:
          ICListBox::Setup(fScope);
          break;
      }
    }
     

    //
    // Draw Self
    //
    void Sessions::DrawSelf(PaintInfo &pi)
    {
      Rebuild *rebuild = PreRebuild();

      DeleteAllItems();

      // Remove all sessions we haven't heard from in 5 seconds
      explorer.RemoveSessions(5000);

      // Copy over all of the sessions
      explorer.CopySessions(sessions);

      // Add all of the non locked sessions
      for (NList<StyxNet::Explorer::Session>::Iterator s(&sessions); *s; ++s)
      {
        if (!((*s)->flags & StyxNet::SessionFlags::Locked))
        {
          // Build the key name from the address and the session name
          // Keys can be up to 64 characters
          // Addresses are 22 characters
          // Session names are 32 characters
          GameIdent key;
          Utils::Sprintf(key.str, key.GetSize(), "%s:%d %s", (*s)->address.GetDisplayText(), (*s)->address.GetPort(), (*s)->name.str);
          Item *item = new Item(*this, **s, this);
          item->SetEnabled(TRUE);
          AddItem(key.str, item);
        }
      }

      // Add all of the locked sessions
      for (!s; *s; ++s)
      {
        if ((*s)->flags & StyxNet::SessionFlags::Locked)
        {
          // Build the key name from the address and the session name
          // Keys can be up to 64 characters
          // Addresses are 22 characters
          // Session names are 32 characters
          GameIdent key;
          Utils::Sprintf(key.str, key.GetSize(), "%s:%d %s", (*s)->address.GetDisplayText(), (*s)->address.GetPort(), (*s)->name.str);
          Item *item = new Item(*this, **s, this);
          item->SetEnabled(FALSE);
          AddItem(key.str, item);
        }
      }

      PostRebuild(&rebuild);

      // Get the list box to draw itself
      ICListBox::DrawSelf(pi);
    }


    //
    // Get the selected session
    //
    StyxNet::Explorer::Session * Sessions::GetSelectedSession()
    {
      IControl *ctrl = GetSelectedItem();

      if (ctrl)
      {
        Item *item = IFace::Promote<Item>(ctrl, TRUE);
        return (&item->session);
      }
      return (NULL);
    }

  }

}
