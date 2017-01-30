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
#include "multiplayer_controls_synclist.h"
#include "multiplayer_private.h"
#include "multiplayer_data.h"
#include "multiplayer_network.h"
#include "iface.h"
#include "iface_types.h"
#include "iface_util.h"
#include "icstatic.h"
#include "font.h"
#include "stdload.h"
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
    // Class SyncList::PlayerItem
    //
    class SyncList::PlayerItem : public ICStatic
    {
      PROMOTE_LINK(SyncList::PlayerItem, ICStatic, 0x2CAF5046); // "SyncList::PlayerItem"

    protected:

      // SyncList this player item is a member of
      SyncList &syncList;

      // Player Network id
      U32 networkId;

    public:

      // Constructor
      PlayerItem(SyncList &syncList, IControl *parent, U32 networkId) 
      : syncList(syncList),
        ICStatic(parent),
        networkId(networkId)
      { 
        SetColorGroup(IFace::data.cgListItem);
      }

      // Redraw self
      void DrawSelf(PaintInfo &pi);

    };

    
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class SyncList
    //


    //
    // SyncList::SyncList
    //
    // Constructor
    //
    SyncList::SyncList(IControl *parent)
    : ICListBox(parent),
      offsetName(20),
      offsetReady(0)
    {
      listBoxStyle |= STYLE_NOSELECTION;
    }


    //
    // SyncList::DrawSelf
    //
    // Redraw self
    //
    void SyncList::DrawSelf(PaintInfo &pi)
    {
      Rebuild *rebuild = PreRebuild();

      // Rebuild the list before redrawing
      DeleteAllItems();

      // Figure out which teams players are on and which groups the teams are in
      BinTree<GroupTree> groups;
      BuildPlayerHierachy(groups);

      Bool first = TRUE;
      IControlName ident;

      // Now we have all the data, add it to the list box
      for (BinTree<GroupTree>::Iterator gti(&groups); *gti; gti++)
      {
        // The first group doesn't get a seperator
        if (first)
        {
          first = FALSE;
        }
        else
        {
          // Group[id]
          Utils::Sprintf(ident.str, ident.GetSize(), "Group[%08X]", gti.GetKey());
          ident.Update();
        }

        // Add the teams which are in this group
        for (BinTree<TeamTree>::Iterator tri(&(*gti)->teams); *tri; tri++)
        {
          // Group[id].Team[id].Player[id]
          Utils::Sprintf(ident.str, ident.GetSize(), "Group[%08X].Team[%08X].Player[%08X]", gti.GetKey(), tri.GetKey(), (*tri)->team->ownerId);
          ident.Update();

          // Add the players which are on this team
          for (BinTree<const Player>::Iterator pi(&(*tri)->players); *pi; pi++)
          {
            // Group[id].Team[id].Player[id]
            Utils::Sprintf(ident.str, ident.GetSize(), "Group[%08X].Team[%08X].Player[%08X]", gti.GetKey(), tri.GetKey(), pi.GetKey());
            ident.Update();

            // Add the players
            AddItem(ident.str, new PlayerItem(*this, this, pi.GetKey()));
          }
        }
      }

      // Cleanup
      groups.DisposeAll();

      PostRebuild(&rebuild);

      ICListBox::DrawSelf(pi);
    }


    //
    // SyncList::Setup
    //
    void SyncList::Setup(FScope *fScope)
    {
      switch (fScope->NameCrc())
      {
        case 0x4ED32653: // "IconReadyOff"
          IFace::FScopeToTextureInfo(fScope, iconReady[0]);
          break;

        case 0xAB192B33: // "IconReadyOn"
          IFace::FScopeToTextureInfo(fScope, iconReady[1]);
          break;

        case 0xD6ABAE33: // "OffsetName"
          offsetName = StdLoad::TypeU32(fScope);
          break;

        case 0x96BD498C: // "OffsetReady"
          offsetReady = StdLoad::TypeU32(fScope);
          break;

        default:
          ICListBox::Setup(fScope);
          break;
      }
    }



    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class SyncList::PlayerItem
    //


    //
    // SyncList::PlayerItem::DrawSelf
    //
    // DrawSelf
    //
    void SyncList::PlayerItem::DrawSelf(PaintInfo &pi)
    {
      // Fill the background
      DrawCtrlBackground(pi, GetTexture());

      // Draw the frame
      DrawCtrlFrame(pi);

      if (!pi.font)
      {
        return;
      }

      // Is there a network player with this id ?
      Network::Player *networkPlayer;
      networkPlayer = Network::GetPlayers().Find(networkId);

      if (networkPlayer)
      {
        // Display the name of the player

        // Add the player name
        const CH *ch;
        U32 yoffs = (pi.client.Height() - pi.font->Height()) / 2;

        // Add Player name
        ch = Utils::Ansi2Unicode(networkPlayer->GetName());
        pi.font->Draw(
          pi.client.p0.x + syncList.offsetName,
          pi.client.p0.y + yoffs, 
          ch, 
          Utils::Strlen(ch),
          pi.colors->fg[ColorIndex()],
          &pi.client
        );

        // Is there player information ?
        const Player *playerInfo;
        if (Data::Get(&playerInfo, networkId))
        {
          TextureInfo *icon = &syncList.iconReady[playerInfo->ready];
          S32 midY = (pi.client.p0.y + pi.client.p1.y) >> 1;

          if (icon->texture)
          {
            // Add launch ready icon
            IFace::RenderRectangle
            (
              ClipRect(
                pi.client.p0.x + syncList.offsetReady, 
                midY - (icon->pixels.Height() >> 1),
                pi.client.p0.x + syncList.offsetReady + icon->pixels.Width(),
                midY - (icon->pixels.Height() >> 1) + icon->pixels.Height()),
              Color(1.0f, 1.0f, 1.0f), 
              icon,
              pi.alphaScale
            );
          }

        }

      }

    }

  }

}
