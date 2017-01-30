///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Won
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "won_controls_roomlist.h"
#include "stdload.h"
#include "font.h"
#include "iface.h"
#include "iface_util.h"
#include "iface_priv.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Won
//
namespace Won
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Controls
  //
  namespace Controls
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class RoomList
    //

    //
    // Constructor
    //
    RoomList::RoomList(IControl *parent)
    : ICListBox(parent),
      offsetLocked(0),
      offsetName(20),
      widthName(100),
      offsetUsers(150)
    {
    }


    //
    // Setup
    //
    void RoomList::Setup(FScope *fScope)
    {
      switch (fScope->NameCrc())
      {
        case 0xD6ABAE33: // "OffsetName"
          offsetName = StdLoad::TypeU32(fScope);
          break;

        case 0x4F76FDA5: // "WidthName"
          widthName = StdLoad::TypeU32(fScope);
          break;

        case 0x87CA6B98: // "OffsetUsers"
          offsetUsers = StdLoad::TypeU32(fScope);
          break;

        case 0x026FBE4B: // "OffsetLocked"
          offsetLocked = StdLoad::TypeU32(fScope);
          break;

        case 0x82ACDEED: // "IconLocked"
          IFace::FScopeToTextureInfo(fScope, iconLocked);
          break;

        default:
          ICListBox::Setup(fScope);
          break;
      }
    }


    //
    // Get the selected room
    //
    WonIface::Room * RoomList::GetSelectedRoom()
    {
      IControl *ctrl = GetSelectedItem();

      if (ctrl)
      {
        Item *item = IFace::Promote<Item>(ctrl, TRUE);
        return (&item->room);
      }
      return (NULL);
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class RoomList::Item
    //


    //
    // Constructor
    //
    RoomList::Item::Item(RoomList &roomList, const WonIface::Room &room, IControl *parent)
    : roomList(roomList),
      room(room),
      ICStatic(parent)
    { 
      controlStyle &= ~STYLE_INERT;

      SetColorGroup(IFace::data.cgListItem);
      SetTipText(room.name.str, TRUE);
    }

    // Redraw self
    void RoomList::Item::DrawSelf(PaintInfo &pi)
    {
      DrawCtrlBackground(pi, GetTexture());
      DrawCtrlFrame(pi);

      if (pi.font)
      {
        const CH *ch;
        U32 yoffs = (pi.client.Height() - pi.font->Height()) / 2;

        // Does session have a password ?
        if (room.password)
        {
          TextureInfo *icon = &roomList.iconLocked;
          S32 midY = (pi.client.p0.y + pi.client.p1.y) >> 1;

          if (icon->texture)
          {
            // Add launch ready icon
            IFace::RenderRectangle
            (
              ClipRect(
                pi.client.p0.x + roomList.offsetLocked,
                midY - (icon->pixels.Height() >> 1),
                pi.client.p0.x + roomList.offsetLocked + icon->pixels.Width(),
                midY - (icon->pixels.Height() >> 1) + icon->pixels.Height()),
              Color(1.0f, 1.0f, 1.0f), 
              icon,
              pi.alphaScale
            );
          }
        }

        // Add Game name
        ClipRect nameClip
        (
          pi.client.p0.x + roomList.offsetName,
          pi.client.p0.y,
          pi.client.p0.x + roomList.offsetName + roomList.widthName,
          pi.client.p1.y
        );

        // Add Room name
        ch = room.name.str;
        pi.font->Draw(
          pi.client.p0.x + roomList.offsetName,
          pi.client.p0.y + yoffs, 
          ch, 
          Utils::Strlen(ch),
          pi.colors->fg[ColorIndex()],
          &nameClip
        );

        // Add the number of players in the room
        CH buff[10];
        Utils::Sprintf(buff, 10, L"%d", room.numPlayers);
        ch = buff;
        pi.font->Draw(
          pi.client.p0.x + roomList.offsetUsers,
          pi.client.p0.y + yoffs, 
          ch, 
          Utils::Strlen(ch),
          pi.colors->fg[ColorIndex()],
          &pi.client
        );

      }

    }

  }

}
