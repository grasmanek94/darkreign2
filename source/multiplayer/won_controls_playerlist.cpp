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
#include "won_controls_playerlist.h"
#include "stdload.h"
#include "font.h"
#include "iface.h"
#include "iface_util.h"
#include "iface_priv.h"
#include "network_ping.h"
#include "multiplayer_pingdisplay.h"


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
    // Definitions
    //
    static const U32 pingInterval = 5000;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Prototypes
    //
    static void PingCallback(const Win32::Socket::Address &address, U32 rtt, U32 hopCount, void *context);


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class PlayerList
    //

    //
    // Constructor
    //
    PlayerList::PlayerList(IControl *parent)
    : ICListBox(parent),
      offsetIcon(0),
      offsetName(20)
    {
    }


    //
    // Setup
    //
    void PlayerList::Setup(FScope *fScope)
    {
      switch (fScope->NameCrc())
      {
        case 0x42215C38: // "OffsetIcon"
          offsetIcon = StdLoad::TypeU32(fScope);
          break;

        case 0xD6ABAE33: // "OffsetName"
          offsetName = StdLoad::TypeU32(fScope);
          break;

        case 0xF66754C8: // "IconMuted"
          IFace::FScopeToTextureInfo(fScope, iconMuted);
          break;

        case 0xF3F09996: // "IconIgnored"
          IFace::FScopeToTextureInfo(fScope, iconIgnored);
          break;

        case 0x29D46D8C: // "IconModerator"
          IFace::FScopeToTextureInfo(fScope, iconModerator);
          break;

        default:
          ICListBox::Setup(fScope);
          break;
      }
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class PlayerList::Item
    //


    //
    // Constructor
    //
    PlayerList::Item::Item(PlayerList &playerList, Bool moderator, Bool muted, Bool ignored)
    : playerList(playerList),
      ICStatic(&playerList),
      moderator(moderator),
      muted(muted),
      ignored(ignored)
    { 
      controlStyle &= ~STYLE_INERT;
      SetColorGroup(IFace::data.cgListItem);
    }


    //
    // Draw Icon
    //
    void PlayerList::Item::DrawIcon(PaintInfo &pi, TextureInfo &icon)
    {
      S32 midY = (pi.client.p0.y + pi.client.p1.y) >> 1;

      if (icon.texture)
      {
        // Add launch ready icon
        IFace::RenderRectangle
        (
          ClipRect(
            pi.client.p0.x + playerList.offsetIcon,
            midY - (icon.pixels.Height() >> 1),
            pi.client.p0.x + playerList.offsetIcon + icon.pixels.Width(),
            midY - (icon.pixels.Height() >> 1) + icon.pixels.Height()),
          Color(1.0f, 1.0f, 1.0f), 
          &icon,
          pi.alphaScale
        );
      }
    }


    //
    // Redraw self
    //
    void PlayerList::Item::DrawSelf(PaintInfo &pi)
    {
      U32 savedState = controlState;
      controlState &= ~STATE_DISABLED;
      DrawCtrlBackground(pi, GetTexture());
      controlState = savedState;

      DrawCtrlFrame(pi);

      // Write player name
      if (pi.font)
      {
        U32 yoffs = (pi.client.Height() - pi.font->Height()) / 2;
        pi.font->Draw(
          pi.client.p0.x + playerList.offsetName,
          pi.client.p0.y + yoffs, 
          GetTextString(), 
          Utils::Strlen(GetTextString()),
          pi.colors->fg[ColorIndex()],
          &pi.client
        );
      }

      // Draw icons (if any)
      if (moderator)
      {
        DrawIcon(pi, playerList.iconModerator);
      }
      if (ignored)
      {
        DrawIcon(pi, playerList.iconIgnored);
      } 
      else if (muted)
      {
        DrawIcon(pi, playerList.iconMuted);
      } 

    }

  }
}
