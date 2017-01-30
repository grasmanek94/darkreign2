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
#include "multiplayer_private.h"
#include "multiplayer_network_private.h"
#include "multiplayer_host.h"
#include "multiplayer_data_private.h"
#include "multiplayer_controls_playerlist.h"
#include "multiplayer_settings.h"
#include "user.h"
#include "iface.h"
#include "console.h"
#include "woniface.h"
#include "main.h"

#include "babel.h"

#include "multiplayer_cmd_private.h"
#include "random.h"
  
#include "win32_dns.h"

#include "player.h"
#include "orders_game.h"

#include "won_cmd.h"

#include "version.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Network
  //
  namespace Network
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Exported data
    //

    // The current client
    StyxNet::Client *client = NULL;

    // The current server
    StyxNet::Server *server = NULL;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Internal data
    //

    static NBinTree<Player> players(&Player::node);
    static Player *currentPlayer = NULL;
    static Bool first = TRUE;
    static StyxNet::SessionData localSessionData;
    static U32 migrationNumber;
    static Bool migrationActive;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Prototypes
    //
    static void CheckMigration();
    

    //
    // Initialization
    //
    void Init()
    {
      // Initialize StyxNet
      StyxNet::Init();

      // Setup socket host system
      Win32::DNS::Setup(Main::GetGameWindow(), WM_USER + 0x600);
      Main::RegisterGUIHook("All", Win32::DNS::WindowProc);

      // Reset migration number
      migrationNumber = 1;

      // Reset migration flag
      migrationActive = FALSE;
    }


    //
    // Shutdown
    //
    void Done()
    {
      Reset();

      // Unregister DNS hook
      Main::UnregisterGUIHook("All", Win32::DNS::WindowProc);

      // Shutdown StyxNet
      StyxNet::Done();
    }


    //
    // Reset
    //
    void Reset()
    {
      LOG_DIAG(("Resetting Network"))

      if (client)
      {
        client->Shutdown();
        client = NULL;
        Cmd::online = FALSE;
      }

      if (server)
      {
        server->Shutdown();
        server = NULL;
      }

      if (Cmd::isHost)
      {
        LOG_DIAG(("WON RemoveGame: %s", localSessionData.name.str))
        WonIface::RemoveGame(localSessionData.name.str);
      }

      players.DisposeAll();
      Data::LocalFlush();
      currentPlayer = NULL;
      Cmd::isHost = FALSE;
      first = TRUE;
    }


    //
    // Process
    //
    void Process()
    {
      // Process StyxNet
      StyxNet::Process();

      // If there's a client, process it
      if (client)
      {
        CRC event;
        StyxNet::Event::Data *data;
        while (client && client->GetEvent(event, data))
        {
          switch (event)
          {
            case StyxNet::EventMessage::SessionCreated:
            {
              Cmd::isHost = TRUE;
              HostedSession();
              break;
            }

            case StyxNet::EventMessage::SessionInfo:
            {
              CAST(StyxNet::EventMessage::Data::SessionInfo *, sessionInfo, data)
              localSessionData = *sessionInfo;

              if (Cmd::isHost)
              {
                U32 ip;
                U16 port;

                if (WonIface::GetLocalAddress(ip, port))
                {
                  // If there's a set address use that one
                  if (Settings::GetLocalAddress())
                  {
                    ip = ntohl(Settings::GetLocalAddress());
                  }

                  // Tell WON that we hosted a session
                  Win32::Socket::Address address(ip, Settings::GetPort());
                  StyxNet::Session session(*sessionInfo, address);

                  // Store the build number
                  session.version = U16(Version::GetBuildNumber());
                  
                  if (first)
                  {
                    LOG_DIAG(("WON AddGame: %s [%d/%d] %s:%d", sessionInfo->name.str, sessionInfo->numUsers, sessionInfo->maxUsers, address.GetText(), address.GetPort()))
                    WonIface::AddGame(sessionInfo->name.str, sizeof (StyxNet::Session), reinterpret_cast<U8 *>(&session));
                    first = FALSE;
                  }
                  else
                  {
                    LOG_DIAG(("WON UpdateGame: %s [%d/%d] %s:%d", sessionInfo->name.str, sessionInfo->numUsers, sessionInfo->maxUsers, address.GetText(), address.GetPort()))
                    WonIface::UpdateGame(sessionInfo->name.str, sizeof (StyxNet::Session), reinterpret_cast<U8 *>(&session));
                  }
                }
              }

              // Update the is locked flag
              Cmd::updatingLock = TRUE;
              Cmd::isLocked = sessionInfo->flags & StyxNet::SessionFlags::Locked ? TRUE : FALSE;
              Cmd::updatingLock = FALSE;

              delete data;
              break;
            }

            case StyxNet::EventMessage::SessionJoined:
              Cmd::isHost = FALSE;
              break;

            case StyxNet::EventMessage::SessionLocalUserAdded:
              JoinedSession();
              break;

            case StyxNet::EventMessage::SessionUserAdded:
            {
              CAST(StyxNet::EventMessage::Data::SessionUserAdded *, sessionUserAdded, data)

              Player *player = players.Find(sessionUserAdded->name.crc);
              if (!player)
              {
                player = new Player(sessionUserAdded->name);
                players.Add(sessionUserAdded->name.crc, player);

                // Is this us ?
                if (sessionUserAdded->name.crc == GetCurrentName().crc)
                {
                  currentPlayer = player;
                }

                LOG_DIAG(("'%s' entered the game", player->GetName()))

                // Tell everyone that a player entered
                CONSOLE(0x7EF342D8, (TRANSLATE(("#multiplayer.chat.playerenter", 1, Utils::Ansi2Unicode(player->GetName()) )) )) // "MultiMessage"

                // Tell the host that a player entered
                Host::EnterPlayer(sessionUserAdded->name.crc);

                delete sessionUserAdded;

                Controls::PlayerList::Dirty();
              }

              if (migrationActive)
              {
                // Set the player's migration number
                player->entryNumber = migrationNumber;
              }

              CheckMigration();

              break;
            }

            case StyxNet::EventMessage::SessionUserRemoved:
            {
              CAST(StyxNet::EventMessage::Data::SessionUserRemoved *, sessionUserRemoved, data)
              Player *player = players.Find(sessionUserRemoved->name.crc);
              ASSERT(player)

              if (Cmd::isHost)
              {
                LOG_DIAG(("Issuing player left order"))

                // If we're in the game, issue an order
                ::Player *p = ::Player::Name2HumanPlayer(player->GetName());
                if (p)
                {
                  Orders::Game::PlayerLeft::Generate(*p);
                }
              }

              // Tell everyone that a player exited
              CONSOLE(0x7EF342D8, (TRANSLATE(("#multiplayer.chat.playerexit", 1, Utils::Ansi2Unicode(player->GetName()) )) )) // "MultiMessage"

              // Tell the host that a player left
              Host::ExitPlayer(sessionUserRemoved->name.crc);

              players.Dispose(player);
              delete sessionUserRemoved;

              Controls::PlayerList::Dirty();

              CheckMigration();
              break;
            }

            case StyxNet::EventMessage::SessionUserDisconnected:
            {
              CAST(StyxNet::EventMessage::Data::SessionUserDisconnected *, sessionUserDisconnected, data)
              Player *player = players.Find(sessionUserDisconnected->name.crc);
              ASSERT(player)

              // Tell everyone that a player disconnected
              CONSOLE(0x7EF342D8, (TRANSLATE(("#multiplayer.chat.playerdisconnected", 1, Utils::Ansi2Unicode(player->GetName()) )) )) // "MultiMessage"

              // Mark the player as being disconnceted

              delete sessionUserDisconnected;
              break;
            }

            case StyxNet::EventMessage::SessionUserReconnected:
            {
              CAST(StyxNet::EventMessage::Data::SessionUserReconnected *, sessionUserReconnected, data)
              Player *player = players.Find(sessionUserReconnected->name.crc);
              ASSERT(player)

              // Tell everyone that a player reconnected
              CONSOLE(0x7EF342D8, (TRANSLATE(("#multiplayer.chat.playerreconnected", 1, Utils::Ansi2Unicode(player->GetName()) )) )) // "MultiMessage"

              // Mark the player as being reconnected

              delete sessionUserReconnected;
              break;
            }

            
            case StyxNet::EventMessage::SessionData:
              // Tell the data system that data arrived
              Data::SessionData(reinterpret_cast<StyxNet::EventMessage::Data::SessionData *>(data));
              break;

            case StyxNet::EventMessage::SessionPrivateData:
              // Tell the data system that private data arrived
              Data::SessionPrivateData(reinterpret_cast<StyxNet::EventMessage::Data::SessionPrivateData *>(data));
              break;

            case StyxNet::EventMessage::SessionSyncData:
              // Tell the data system that syncronous data arrived
              Data::SessionSyncData(reinterpret_cast<StyxNet::EventMessage::Data::SessionSyncData *>(data));
              break;

            case StyxNet::EventMessage::SessionMigrateRequest:
            {
              CAST(StyxNet::EventMessage::Data::SessionMigrateRequest *, sessionMigrateRequest, data)

              // Create a new server
              if (server)
              {
                ERR_FATAL(("We are running a server and have been asked to accept a migration!"))
              }

              // Create the server
              StyxNet::Server::Config config;
              Network::server = new StyxNet::Server(config);

              // Setup the session for migrating to on this server
              U32 key;
              Network::server->SetupMigration(localSessionData.name, localSessionData.maxUsers, sessionMigrateRequest->seq, key);

              // Tell the client that we're going to accept the migration
              Network::client->AcceptMigration(config.port, key);

              // We're now the host
              Cmd::isHost = TRUE;

              // Tell this player that they are now the host
              CONSOLE(0x7EF342D8, (TRANSLATE(("#multiplayer.server.migrated")) )) // "MultiMessage"

              delete sessionMigrateRequest;
              break;
            }

            case StyxNet::EventMessage::SessionMigrateComplete:
              Console::ProcessCmd("sys.game.migrate.resync");
              break;

            case StyxNet::EventMessage::SessionMigrateNotNeeded:
              LOG_DIAG(("Migration was not needed"))
              Console::ProcessCmd("sys.game.migrate.end");
              break;

            case StyxNet::EventMessage::Ping:
            {
              CAST(StyxNet::EventMessage::Data::Ping *, ping, data)

              Utils::Memmove(&PrivData::localPings[1], &PrivData::localPings[0], sizeof (U16) * (PrivData::maxLocalPings - 1));
              PrivData::localPings[0] = U16(Clamp<U32>(0, ping->rtt, U16_MAX));

              delete ping;
              break;
            }

            default:
              if (data)
              {
                delete data;
              }
              break;
          }

          // Send it to the registered interface control
          if (PrivData::clientCtrl.Alive())
          {
            IFace::SendEvent(PrivData::clientCtrl, NULL, IFace::NOTIFY, event);
          }
        }
      }

      if (server)
      {
        CRC event;
        StyxNet::Event::Data *data;
        while (server && server->GetEvent(event, data))
        {
          switch (event)
          {
            case StyxNet::EventMessage::ServerNoSessions:
              server->Shutdown();
              server = NULL;

              // We ain't the host any more
              Cmd::isHost = FALSE;
              break;

            default:
              if (data)
              {
                delete data;
              }
              break;
          }

          // Send it to the registered interface control
          if (PrivData::serverCtrl.Alive())
          {
            IFace::SendEvent(PrivData::serverCtrl, NULL, IFace::NOTIFY, event);
          }
        }
      }
    }


    //
    // Get the tree of players
    //
    const NBinTree<Player, CRC> & GetPlayers()
    {
      return (players);
    }


    //
    // Get the ident of the current player
    //
    const UserName & GetCurrentName()
    {
      static UserName ident;

      if (Cmd::inWon)
      {
        // If we're in WON use our WON name
        ident = Won::Cmd::GetUsername();
      }
      else
      {
        // If not, use our user name
        ident = User::GetName();
      }

      return (ident);
    }


    //
    // Get the current player
    //
    const Player & GetCurrentPlayer()
    {
      ASSERT(currentPlayer)
      return (*currentPlayer);
    }


    //
    // Create the current player (only for instant action!)
    //
    void CreateCurrentPlayer()
    {
      currentPlayer = new Player(GetCurrentName());
      players.Add(currentPlayer->name.crc, currentPlayer);
    }


    //
    // Have current player
    //
    Bool HaveCurrentPlayer()
    {
      return (currentPlayer ? TRUE : FALSE);
    }


    //
    // What's our address ?
    //
    Bool GetLocalAddress(Win32::Socket::Address &address, CRC who)
    {
      if (server)
      {
        return (server->GetLocalAddress(address, who));
      }
      else if (client)
      {
        return (client->GetLocalAddress(address));
      }
      else
      {
        return (FALSE);
      }
    }


    //
    // StartMigration
    //
    void StartMigration()
    {
      migrationNumber++;
      migrationActive = TRUE;
    }


    //
    // CheckMigration
    //
    void CheckMigration()
    {
      if (migrationActive)
      {
        // Are all players at the migration number
        Bool all = TRUE;
        for (NBinTree<Player>::Iterator p(&players); *p; ++p)
        {
          if ((*p)->entryNumber != migrationNumber)
          {
            all = FALSE;
            break;
          }
        }

        if (all)
        {
          migrationActive = FALSE;

          LOG_DIAG(("All players migrated"))

          // Tell the game that migration is done !
          Console::ProcessCmd("sys.game.migrate.end");

          // Tell the network that the migration is done !
          if (client && Cmd::isHost)
          {
            LOG_DIAG(("Telling server that migration has completed"))
            client->MigrationComplete();
          }
        }
      }
    }
  }
}
