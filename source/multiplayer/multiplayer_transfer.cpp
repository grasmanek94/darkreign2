///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Tranfer
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "multiplayer_transfer_private.h"
#include "multiplayer_controls_transferlist.h"
#include "iface_messagebox.h"
#include "iface.h"
#include "babel.h"
#include "console.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Transfer
  //
  namespace Transfer
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Prototypes
    //
    static void AcceptOffer(U32 id, CRC who);
    static Bool DeleteOffer(U32 id, CRC who);
    static void DenyOffer(U32 id, CRC who);
    static Offer * FindOffer(const NBinTree<Offer> &offers, U32 id, U32 who);
    static void RemoveOffers(NBinTree<Offer> &offers, CRC who);
    static void RemoveExpiredOffers(NBinTree<Offer> &offers);
    static void MsgBoxCallback(U32 event, U32 context);


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Internal Data
    //
    static Bool initialized = FALSE;
    static U32 id;
    static NBinTree<Offer> offersIn(&Offer::node);
    static NBinTree<Offer> offersOut(&Offer::node);
    static NBinTree<Offer> offersAccepted(&Offer::node);
    static NBinTree<Offer> offersSending(&Offer::node);
    static NBinTree<Offer> offersReceiving(&Offer::node);


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Struct Offer
    //
    Offer::Offer(Bool from, CRC type, CRC who, const Commands::Data::TransferOffer &transferOffer)
    : from(from), 
      type(type),
      who(who),
      id(transferOffer.id),
      path(transferOffer.path),
      size(transferOffer.size)
    {
      // Add this to the display list
      if (PrivData::transferListCtrl.Alive())
      {
        Controls::TransferList *transferList = IFace::Promote<Controls::TransferList>(PrivData::transferListCtrl);
        if (transferList)
        {
          ctrl = transferList->AddTransfer(this);
        }
      }
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Struct Context
    //
    struct Context
    {
      U32 id;
      CRC who;

      Context(U32 id, CRC who)
      : id(id),
        who(who)
      {
      }
    };


    //
    // Destructor
    //
    Offer::~Offer()
    {
      if (PrivData::transferListCtrl.Alive())
      {
        Controls::TransferList *transferList = IFace::Promote<Controls::TransferList>(PrivData::transferListCtrl);

        // Remove from the display list
        if (transferList && ctrl.Alive())
        {
          transferList->RemoveItem(ctrl);
        }
      }
    }


    //
    // Initialization
    //
    void Init()
    {
      ASSERT(!initialized)

      Reset();

      initialized = TRUE;
    }


    //
    // Shutdown
    //
    void Done()
    {
      ASSERT(initialized)

      Reset();

      initialized = FALSE;
    }


    //
    // Reset
    //
    void Reset()
    {
      // Initialize Id to something uniqish
      id = Clock::Time::Ms();

      // Kill off any offers
      offersIn.DisposeAll();
      offersOut.DisposeAll();
      offersAccepted.DisposeAll();
      offersSending.DisposeAll();
      offersReceiving.DisposeAll();
    }


    //
    // Process
    //
    void Process()
    {
      // Check the offers for expired ones
      RemoveExpiredOffers(offersIn);
      RemoveExpiredOffers(offersOut);
      RemoveExpiredOffers(offersAccepted);
      RemoveExpiredOffers(offersSending);
      RemoveExpiredOffers(offersReceiving);
    }


    //
    // Interpret a command
    //
    void Handler(CRC from, CRC key, U32 size, const U8 *data)
    {
      ASSERT(initialized)

      Network::Player *player = Network::GetPlayers().Find(from);

      switch (key)
      {
        case Commands::TransferOffer:
        {
          if (player)
          {
            ASSERT(size == sizeof (Commands::Data::TransferOffer))
            CAST(const Commands::Data::TransferOffer *, transferOffer, data)

            LOG_DIAG(("Received an xfer offer from [%08X]", from))
            LOG_DIAG(("File: %s", transferOffer->path.str))
            LOG_DIAG(("Size: %d", transferOffer->size))
            LOG_DIAG(("Type: %08X", transferOffer->type))
            LOG_DIAG(("Id: %d", transferOffer->id))

            // An offer has been made, ask the user if they wish to accept
            StrBuf<PATHNAME_MAX, CH> path = Utils::Ansi2Unicode(transferOffer->path.str);
            IdentCH name = Utils::Ansi2Unicode(player->GetName());

            IFace::MsgBox
            (
              TRANSLATE(("#multiplayer.transfer.offer.title")),
              TRANSLATE(("#multiplayer.transfer.offer.message", 2, path.str, name.str)), 
              0,
              new MBEventCallback("Offer::Accept", TRANSLATE(("#standard.buttons.yes")), MsgBoxCallback, U32(new Context(transferOffer->id, from))),
              new MBEventCallback("Offer::Deny", TRANSLATE(("#standard.buttons.no")), MsgBoxCallback, U32(new Context(transferOffer->id, from)))
            );

            // Add to internal offer database
            offersIn.Add(from, new Offer(TRUE, transferOffer->type, from, *transferOffer));
          }

          break;
        }

        case Commands::TransferOfferAccept:
        {
          ASSERT(size == sizeof (Commands::Data::TransferOfferAccept))
          CAST(const Commands::Data::TransferOfferAccept *, transferOfferAccept, data)

          LOG_DIAG(("Offer %d was accepted by [%08X]", transferOfferAccept->id, from))

          // If we are the host, ask the host for what it thinks our address is
          Win32::Socket::Address address;
          if (!Network::GetLocalAddress(address, from))
          {
            LOG_WARN(("We couldn't get out local address!"))
            break;
          }

          // Transfer was accepted, initiate the transfer
          Offer *offer = FindOffer(offersOut, transferOfferAccept->id, from);
          if (offer)
          {
            // Commence Transfer!
            offersOut.Unlink(offer);
            U16 port;
            U32 size;
            if (offer->transfer.Send(offer->path.str, port, size))
            {
              LOG_DIAG(("Transfer initiated from %s:%d", address.GetText(), address.GetPort()))

              // Initiate the transfer
              Commands::Data::TransferInitiate transferInitiate;
              transferInitiate.id = offer->id;

              transferInitiate.ip = address.GetIP();
              transferInitiate.port = port;
              Data::Send(1, &offer->who, Commands::TransferInitiate, transferInitiate);

              // Add to the transferers who are offering
              offersSending.Add(offer->who, offer);
            }
            else
            {
              LOG_WARN(("Failed to send!"))
              delete offer;
            }
          }
          else
          {
            LOG_DIAG(("Accepted offer could not be resolved!"))
          }
          break;
        }

        case Commands::TransferOfferDeny:
        {
          ASSERT(size == sizeof (Commands::Data::TransferOfferDeny))
          CAST(const Commands::Data::TransferOfferDeny *, transferOfferDeny, data)

          LOG_DIAG(("Offer %d was denied by [%08X]", transferOfferDeny->id, from))

          // Transfer was denied, throw it away
          Offer *offer = FindOffer(offersOut, transferOfferDeny->id, from);
          if (offer)
          {
            offersOut.Dispose(offer);
          }
          break;
        }

        case Commands::TransferInitiate:
        {
          ASSERT(size == sizeof (Commands::Data::TransferInitiate))
          CAST(const Commands::Data::TransferInitiate *, transferInitiate, data)
          
          LOG_DIAG(("Transfer %d inititated by [%08X]", transferInitiate->id, from))

          // Is this a transfer we have accepted ?
          Offer *offer = FindOffer(offersAccepted, transferInitiate->id, from);
          if (offer)
          {
            // Commence Transfer!
            offersAccepted.Unlink(offer);
            if 
            (
              offer->transfer.Recv
              (
                offer->path.str, 
                Win32::Socket::Address(transferInitiate->ip, transferInitiate->port), 
                offer->size
              )
            )
            {
              offersReceiving.Add(offer->who, offer);
            }
            else
            {
              LOG_WARN(("Failed to recv!"))
              delete offer;
            }
          }
          else
          {
            LOG_WARN(("Received a TransferInitiate from [%08X] but we never accepted it!", from))
          }

          break;
        }
      }
    }


    //
    // Player has left
    //
    void PlayerLeft(CRC id)
    {
      // Dispose of any transfers we have with this player
      RemoveOffers(offersIn, id);
      RemoveOffers(offersOut, id);
      RemoveOffers(offersAccepted, id);
      RemoveOffers(offersSending, id);
      RemoveOffers(offersReceiving, id);
    }


    //
    // Make an offer
    //
    void MakeOffer(CRC to, CRC type, const char *path)
    {
      // Find this file and gets its size
      Win32::File file;

      LOG_DIAG(("Making an offer to [%08X]", to))
      
      if (!file.Open(path, Win32::File::MODE_READ | Win32::File::MODE_OPEN_EXISTING | Win32::File::MODE_SHARE_READ))
      {
        LOG_WARN(("Cannot offer %s cause we can't open it", path))
        return;
      }

      // We can't make an offer to another player who we have
      // a) made an offer to that hasn't been accepted/denied
      // b) a transfer in progress to
      if (offersOut.Find(to))
      {
        LOG_DIAG(("Cannot offer to [%08X] since we have a pending offer!"))
        return;
      }
      if (offersSending.Find(to))
      {
        LOG_DIAG(("Cannot offer to [%08X] since we are sending to them!"))
        return;
      }

      // Send the offer
      Commands::Data::TransferOffer transferOffer;
      transferOffer.size = file.GetSize();
      transferOffer.type = type;
      transferOffer.id = id++;
      transferOffer.path = path;
      Data::Send(1, &to, Commands::TransferOffer, transferOffer);

      // Add to internal database
      offersOut.Add(to, new Offer(FALSE, type, to, transferOffer));

      // Make sure the transfer display is visible
      if (PrivData::transferCtrl.Alive())
      {
        PrivData::transferCtrl->Activate();
        PrivData::transferCtrl->SetZPos(0);
      }
      
      file.Close();
    }


    //
    // Accept an offer
    //
    void AcceptOffer(U32 id, CRC who)
    {
      Offer *offer = FindOffer(offersIn, id, who);
      if (offer)
      {
        // Make sure the transfer display is visible
        if (PrivData::transferCtrl.Alive())
        {
          PrivData::transferCtrl->Activate();
          PrivData::transferCtrl->SetZPos(0);
        }

        // Find the mission from the pack file name
        if (Missions::FindMissionFromPack(offer->path.str))
        {
          // The file already exists, do we want to keep it ?
          StrBuf<PATHNAME_MAX, CH> path = Utils::Ansi2Unicode(offer->path.str);

          IFace::MsgBox
          (
            TRANSLATE(("#multiplayer.transfer.existingmission.title")),
            TRANSLATE(("#multiplayer.transfer.existingmission.message", 1, path.str)),
            0,
            new MBEventCallback("Overwrite::Yes", TRANSLATE(("#standard.buttons.yes")), MsgBoxCallback, U32(new Context(offer->id, offer->who))),
            new MBEventCallback("Overwrite::No", TRANSLATE(("#standard.buttons.no")), MsgBoxCallback, U32(new Context(offer->id, offer->who)))
          );
        }
        // Before accepting, check to see if we have that file already
        else if (Win32::File::Exists(offer->path.str))
        {
          // The file already exists, do we want to keep it ?
          StrBuf<PATHNAME_MAX, CH> path = Utils::Ansi2Unicode(offer->path.str);

          IFace::MsgBox
          (
            TRANSLATE(("#multiplayer.transfer.existingmission.title")),
            TRANSLATE(("#multiplayer.transfer.existingmission.message", 1, path.str)),
            0,
            new MBEventCallback("Overwrite::Yes", TRANSLATE(("#standard.buttons.yes")), MsgBoxCallback, U32(new Context(offer->id, offer->who))),
            new MBEventCallback("Overwrite::No", TRANSLATE(("#standard.buttons.no")), MsgBoxCallback, U32(new Context(offer->id, offer->who)))
          );
        }
        else
        {
          // Now place the transfer in the accepted pile and tell them we accepted
          offersIn.Unlink(offer);
          offersAccepted.Add(offer->who, offer);

          Commands::Data::TransferOfferAccept transferOfferAccept;
          transferOfferAccept.id = id;
          Data::Send(1, &offer->who, Commands::TransferOfferAccept, transferOfferAccept);
        }
      }
    }


    //
    // Delete an offer
    //
    Bool DeleteOffer(U32 id, CRC who)
    {
      Offer *offer = FindOffer(offersIn, id, who);
      if (offer)
      {
        PathIdent group;
        FileIdent mission;

        // Find the mission from the pack file name
        if (const Missions::Mission *m = Missions::FindMissionFromPack(offer->path.str, &group, &mission))
        {
          // If this is the current mission, clear the selected mission
          if (Missions::GetSelected() == m)
          {
            PrivData::ClearMission();
          }

          // Ask the mission system to delete it
          if (Missions::DeleteMission(group, mission))
          {
            return (TRUE);
          }
        }
        // Try to delete the file
        else if (Win32::File::Unlink(offer->path.str))
        {
          return (TRUE);
        }

        // We can't delete the file!
        StrBuf<PATHNAME_MAX, CH> path = Utils::Ansi2Unicode(offer->path.str);
        IFace::MsgBox
        (
          TRANSLATE(("#multiplayer.transfer.cantdelete.title")),
          TRANSLATE(("#multiplayer.transfer.cantdelete.message", 1, path.str)),
          0,
          new MBEvent("Ok", TRANSLATE(("#standard.buttons.ok")))
        );
      }
      return (FALSE);
    }


    //
    // Deny an offer 
    //
    void DenyOffer(U32 id, CRC who)
    {
      Offer *offer = FindOffer(offersIn, id, who);
      if (offer)
      {
        Commands::Data::TransferOfferDeny transferOfferDeny;
        transferOfferDeny.id = id;
        Data::Send(1, &offer->who, Commands::TransferOfferDeny, transferOfferDeny);
        offersIn.Dispose(offer);
      }
    }


    //
    // FindOffer
    //
    Offer * FindOffer(const NBinTree<Offer> &offers, U32 id, CRC who)
    {
      for (NBinTree<Offer>::Iterator o(&offers); *o; ++o)
      {
        if ((*o)->id == id && (*o)->who == who)
        {
          return (*o);
        }
      }
      return (NULL);
    }


    //
    // RemoveOffers
    //
    void RemoveOffers(NBinTree<Offer> &offers, CRC who)
    {
      Offer *offer = offers.Find(who);
      if (offer)
      {
        offers.Dispose(offer);
      }
      ASSERT(!offers.Exists(who))
    }


    //
    // RemoveExpiredOffers
    //
    void RemoveExpiredOffers(NBinTree<Offer> &offers)
    {
      NBinTree<Offer>::Iterator o(&offers);
      while (Offer *offer = o++)
      {
        U32 state; 
        U32 remaining; 
        U32 rate;
        offer->transfer.Progress(state, remaining, rate);

        switch (state)
        {
          case StyxNet::TransferState::Completed:
          {
            Network::Player *player = Network::GetPlayers().Find(offer->who);
            if (player)
            {
              CH playerBuf[128];
              Utils::Ansi2Unicode(playerBuf, 128, player->GetName());

              if (offer->from)
              {
                // "MultiMessage"
                CONSOLE(0x7EF342D8, (TRANSLATE(("#multiplayer.transfer.completedfrom", 2, Utils::Ansi2Unicode(offer->path.str), playerBuf)) ))
              }
              else
              {
                // "MultiMessage"
                CONSOLE(0x7EF342D8, (TRANSLATE(("#multiplayer.transfer.completedto", 2, Utils::Ansi2Unicode(offer->path.str), playerBuf)) ))
              }

              // If this is a mission, regiser it
              if (offer->type == Type::Mission)
              {
                PathIdent group;
                FileIdent mission;
                Missions::FindMissionFromPack(offer->path.str, &group, &mission);
                Missions::RegisterMission(group, mission);

                // Update our local mission stuff
                UpdateMission();
              }

              U32 time = Clock::Time::Ms() - offer->transfer.GetStartTime();
              U32 rate = offer->size * 1000 / time;

              #define NICE(x) x < 1000 ? x : x < 1000000 ? x / 1000 : x / 1000000
              #define NICESTR(x) x < 1000 ? L"" : x < 1000000 ? L"k" : L"M"

              // "MultiMessage"
              CONSOLE(0x7EF342D8, (TRANSLATE(("#multiplayer.transfer.completedstats", 7,
                NICE(offer->size), NICESTR(offer->size),
                (time / 1000) / 60, (time / 1000) % 60, time % 1000,
                NICE(rate), NICESTR(rate) )) ))
            }
            offers.Dispose(offer);
            break;
          }

          case StyxNet::TransferState::Error:
          {
            Network::Player *player = Network::GetPlayers().Find(offer->who);
            if (player)
            {
              CH playerBuf[128];
              Utils::Ansi2Unicode(playerBuf, 128, player->GetName());

              if (offer->from)
              {
                // "MultiMessage"
                CONSOLE(0x7EF342D8, (TRANSLATE(("#multiplayer.transfer.failedfrom", 2, Utils::Ansi2Unicode(offer->path.str), playerBuf)) ))
              }
              else
              {
                // "MultiMessage"
                CONSOLE(0x7EF342D8, (TRANSLATE(("#multiplayer.transfer.failedto", 2, Utils::Ansi2Unicode(offer->path.str), playerBuf)) ))
              }
            }
            offers.Dispose(offer);
            break;
          }

          case StyxNet::TransferState::Aborted:
          {
            Network::Player *player = Network::GetPlayers().Find(offer->who);
            if (player)
            {
              CH playerBuf[128];
              Utils::Ansi2Unicode(playerBuf, 128, player->GetName());

              if (offer->from)
              {
                // "MultiMessage"
                CONSOLE(0x7EF342D8, (TRANSLATE(("#multiplayer.transfer.abortedfrom", 2, Utils::Ansi2Unicode(offer->path.str), playerBuf)) ))
              }
              else
              {
                // "MultiMessage"
                CONSOLE(0x7EF342D8, (TRANSLATE(("#multiplayer.transfer.abortedto", 2, Utils::Ansi2Unicode(offer->path.str), playerBuf)) ))
              }
            }
            offers.Dispose(offer);
            break;
          }

          default:
            break;
        }
      }

    }


    //
    // MsgBoxCallback
    //
    void MsgBoxCallback(U32 event, U32 context)
    {
      Context *c = reinterpret_cast<Context *>(context);

      switch (event)
      {
        case 0x87138B3F: // "Offer::Accept"
          AcceptOffer(c->id, c->who);
          break;

        case 0x0DD46D8C: // "Offer::Deny"
          DenyOffer(c->id, c->who);
          break;

        case 0x13014774: // "Overwrite::Yes"
          if (DeleteOffer(c->id, c->who))
          {
            AcceptOffer(c->id, c->who);
          }
          else
          {
            DenyOffer(c->id, c->who);
          }
          break;

        case 0xA188D425: // "OverWrite::No"
          DenyOffer(c->id, c->who);
          break;
      }

      delete c;

    }
  }
}

