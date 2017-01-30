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
#include "multiplayer_controls_transferlist.h"
#include "multiplayer_transfer_private.h"
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
    // Class TransferList::Item
    //
    class TransferList::Item : public ICStatic
    {
      PROMOTE_LINK(TransferList::Item, ICStatic, 0x2BAF755E) // "TransferList::Item"

    protected:

      // Transfer list this item is a member of
      TransferList &transferList;

      // Transfer Offer
      Transfer::Offer *offer;

    public:

      // Constructor
      Item(TransferList &transferList, Transfer::Offer *offer, IControl *parent) 
      : transferList(transferList),
        offer(offer),
        ICStatic(parent)
      { 
        SetColorGroup(IFace::data.cgListItem);
      }

      // Redraw self
      void DrawSelf(PaintInfo &pi);

      friend TransferList;

    };

    
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class TransferList
    //


    //
    // TransferList::TransferList
    //
    // Constructor
    //
    TransferList::TransferList(IControl *parent)
    : ICListBox(parent),
      heightProgress(10)
    {
      listBoxStyle |= STYLE_NOSELECTION;
    }


    //
    // TransferList::Setup
    //
    void TransferList::Setup(FScope *fScope)
    {
      switch (fScope->NameCrc())
      {
        case 0xF39D93DD: // "OffsetFile"
          StdLoad::TypePoint<S32>(fScope, offsetFile);
          break;

        case 0x2E6FADAE: // "OffsetPlayer"
          StdLoad::TypePoint<S32>(fScope, offsetPlayer);
          break;

        case 0xEFC39332: // "OffsetTransferred"
          StdLoad::TypePoint<S32>(fScope, offsetTransferred);
          break;

        case 0xECA04E02: // "OffsetRate"
          StdLoad::TypePoint<S32>(fScope, offsetRate);
          break;

        case 0x92BD62DD: // "OffsetETA"
          StdLoad::TypePoint<S32>(fScope, offsetETA);
          break;

        case 0x5592337B: // "OffsetProgress"
          StdLoad::TypePoint<S32>(fScope, offsetProgress);
          break;

        case 0x4669855C: // "HeightProgress"
          heightProgress = StdLoad::TypeU32(fScope);
          break;

        default:
          ICListBox::Setup(fScope);
          break;
      }
    }


    //
    // HandleEvent
    //
    U32 TransferList::HandleEvent(Event &e)
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
              case TransferListMsg::Halt:
                if (e.iface.from)
                {
                  IControl *from = static_cast<IControl *>(e.iface.from);
                  TransferList::Item *item = IFace::Promote<TransferList::Item>(from);
                  if (item)
                  {
                    item->offer->transfer.Abort();
                  }
                }

                // Handled
                return (TRUE);
            }
          }
        }
      }
      return (ICListBox::HandleEvent(e));
    }



    //
    // Add Transfer
    //
    IControl * TransferList::AddTransfer(Transfer::Offer *offer)
    {
      Item *item = new Item(*this, offer, this);
      AddItem("", item);

      return (item);
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class TransferList::Item
    //


    //
    // TransferList::Item::DrawSelf
    //
    // DrawSelf
    //
    void TransferList::Item::DrawSelf(PaintInfo &pi)
    {
      // Fill the background
      DrawCtrlBackground(pi, GetTexture());

      // Draw the frame
      DrawCtrlFrame(pi);

      if (!pi.font)
      {
        return;
      }

      CH buff[128];
      const CH *ch;

      // Draw the name of the file
      ch = Utils::Ansi2Unicode(offer->path.str);
      pi.font->Draw
      (
        pi.client.p0.x + transferList.offsetFile.x,
        pi.client.p0.y + transferList.offsetFile.y, 
        ch, 
        Utils::Strlen(ch),
        pi.colors->fg[ColorIndex()],
        &pi.client
      );

      Network::Player *player = Network::GetPlayers().Find(offer->who);
      if (player)
      {
        Utils::Sprintf(buff, 128, L"%s %s", offer->from ? L"From" : L"To", Utils::Ansi2Unicode(player->GetName()));
        ch = buff;
        pi.font->Draw
        (
          pi.client.p0.x + transferList.offsetPlayer.x,
          pi.client.p0.y + transferList.offsetPlayer.y, 
          ch, 
          Utils::Strlen(ch),
          pi.colors->fg[ColorIndex()],
          &pi.client
        );
      }

      ch = Utils::Ansi2Unicode(offer->path.str);
      pi.font->Draw
      (
        pi.client.p0.x + transferList.offsetFile.x,
        pi.client.p0.y + transferList.offsetFile.y, 
        ch, 
        Utils::Strlen(ch),
        pi.colors->fg[ColorIndex()],
        &pi.client
      );

      U32 state;
      U32 remaining;
      U32 rate;

      offer->transfer.Progress(state, remaining, rate);

      switch (state)
      {
        case StyxNet::TransferState::Transferring:
          break;

        default:
          remaining = offer->size;
          rate = 0;
          break;
      }

      U32 transferred = offer->size - remaining;

      // Draw the amount transferred
      if (transferred < 1000)
      {
        Utils::Sprintf(buff, 128, L"%.1fB", F32(transferred));
      }
      else if (transferred < 1000000)
      {
        Utils::Sprintf(buff, 128, L"%.1fkB", F32(transferred) / 1000.0f);
      }
      else
      {
        Utils::Sprintf(buff, 128, L"%.1fMB", F32(transferred) / 1000000.0f);
      }
      ch = buff;
      pi.font->Draw
      (
        pi.client.p0.x + transferList.offsetTransferred.x,
        pi.client.p0.y + transferList.offsetTransferred.y, 
        ch, 
        Utils::Strlen(ch),
        pi.colors->fg[ColorIndex()],
        &pi.client
      );

      // Draw the rate
      if (rate < 1000)
      {
        Utils::Sprintf(buff, 128, L"%.1fB/s", F32(rate));
      }
      else if (rate < 1000000)
      {
        Utils::Sprintf(buff, 128, L"%.1fkB/s", F32(rate) / 1000.0f);
      }
      else
      {
        Utils::Sprintf(buff, 128, L"%.1fMB/s", F32(rate) / 1000000.0f);
      }
      pi.font->Draw
      (
        pi.client.p0.x + transferList.offsetRate.x,
        pi.client.p0.y + transferList.offsetRate.y, 
        ch, 
        Utils::Strlen(ch),
        pi.colors->fg[ColorIndex()],
        &pi.client
      );

      // Draw the ETA
      if (rate)
      {
        S32 eta = remaining / rate;
        Utils::Sprintf(buff, 128, L"%d:%02d", eta / 60, eta % 60);
      }
      else
      {
        ch = L"?:??";
      }
      pi.font->Draw
      (
        pi.client.p0.x + transferList.offsetETA.x,
        pi.client.p0.y + transferList.offsetETA.y, 
        ch, 
        Utils::Strlen(ch),
        pi.colors->fg[ColorIndex()],
        &pi.client
      );

      // Draw a progress bar
      ClipRect c
      (
        pi.client.p0.x + transferList.offsetProgress.x, pi.client.p0.y + transferList.offsetProgress.y,
        pi.client.p1.x - transferList.offsetProgress.x, pi.client.p0.y + transferList.offsetProgress.y + transferList.heightProgress
      );

      IFace::RenderRectangle
      (
        c,
        Color(0.0f, 0.0f, 0.0f, pi.alphaScale), 
        NULL, 
        pi.alphaScale
      );

      IFace::RenderGradient
      (
        ClipRect
        (
          c.p0.x, c.p0.y,
          c.p1.x - (c.Width() * remaining / offer->size), c.p1.y
        ), 
        Color(0.2f, 1.0f, 1.0f, pi.alphaScale), 
        Color(0.1f, 0.5f, 0.5f, pi.alphaScale)
      );

    }

  }

}
