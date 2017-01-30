////////////////////////////////////////////////////////////////////////////////
//
// StyxNet
//
// Copyright 1999-2000
// Matthew Versluys
//


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "styxnet_client.h"
#include "styxnet_client_private.h"
#include "styxnet_clientmessage.h"
#include "styxnet_clientresponse.h"
#include "styxnet_serverresponse.h"
#include "styxnet_servermessage.h"
#include "network_ping.h"
#include "network_icmp.h"
#include "win32_critsec.h"
#include "version.h"


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //
  static const U32 maxClients = 16;
  static Win32::CritSec clientsCritSec;
  static Client *clients[maxClients];


  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Client
  //


  //
  // Client::Client
  //
  // Constructor
  //
  Client::Client(const Config &config)
  : config(config),
    flags(0),
    secret(0),
    sequence(0),
    session(NULL),
    socket(Win32::Socket::TCP, TRUE),
    packetBuffer(Packet::Buffer::Create(clientBufferSize))
  {
    AddClient();

    clientsCritSec.Enter();
    index = U32(-1);
    for (U32 c = 0; c < maxClients; c++)
    {
      if (!clients[c])
      {
        index = c;
        clients[index] = this;
      }
    }
    clientsCritSec.Exit();

    // Start the thread
    thread.Start(ThreadProc, this);

    // Make it above normal
    thread.SetPriority(Win32::Thread::ABOVE_NORMAL);
  }


  //
  // Client::~Client
  //
  // Destructor
  //
  Client::~Client()
  {
    clientsCritSec.Enter();
    clients[index] = NULL;
    clientsCritSec.Exit();

    // If we're connected, disconnect us
    if (flags & ClientFlags::Connected)
    {
      Packet::Create(ClientMessage::UserLogout).Send(socket);
    }

    // Make sure the socket is closed
    socket.Close();

    // If there's a session, delete it
    if (session)
    {
      delete session;
    }

    // Dispose of the packet buffer
    delete packetBuffer;

    RemoveClient();
  }


  //
  // Client::CreateSession
  //
  // Create a session
  //
  void Client::CreateSession(const SessionName &name, CRC password, U32 maxUsers)
  {
    ClientMessage::Data::SessionCreate *sessionCreate;
    Packet &pkt = Packet::Create(ClientMessage::SessionCreate, sessionCreate);

    sessionCreate->name = name.str;
    sessionCreate->password = password;
    sessionCreate->maxUsers = maxUsers;

    // Send to the server our desire to create a new session
    pkt.Send(socket);
  }


  //
  // Client::ConnectSession
  //
  // Connect to a session
  //
  void Client::ConnectSession(const SessionName &name, CRC password, U32 maxUsers)
  {
    ClientMessage::Data::SessionConnect *sessionConnect;
    Packet &pkt = Packet::Create(ClientMessage::SessionConnect, sessionConnect);

    sessionConnect->name = name.str;
    sessionConnect->password = password;
    sessionConnect->maxUsers = maxUsers;

    // Send to the server our desire to create a new session
    pkt.Send(socket);
  }


  //
  // Client::JoinSession
  //
  // Join a session
  //
  void Client::JoinSession(const SessionName &name, CRC password)
  {
    ClientMessage::Data::SessionJoin *sessionJoin;
    Packet &pkt = Packet::Create(ClientMessage::SessionJoin, sessionJoin);

    sessionJoin->name = name.crc;
    sessionJoin->password = password;

    // Send to the server our desire to create a new session
    pkt.Send(socket);
  }


  //
  // Send a message to a private recipient list
  //
  void Client::SendData(U32 numRecipients, const CRC *recipients, CRC key, U32 length, const U8 *data)
  {
    Packet &pkt = Packet::Create
    (
      ClientMessage::SessionPrivateData, 
      sizeof (ClientMessage::Data::SessionPrivateData) + 
      length + (numRecipients * sizeof (CRC))
    );

    CAST(ClientMessage::Data::SessionPrivateData *, sessionPrivateData, pkt.GetData())

    // Copy over the key
    sessionPrivateData->key = key;

    // Copy over the number of recipients
    sessionPrivateData->numRecipients = numRecipients;

    if (numRecipients)
    {
      // Copy over the recipient list
      Utils::Memcpy(pkt.GetData() + sizeof (ClientMessage::Data::SessionPrivateData), recipients, numRecipients * sizeof (CRC));
    }

    if (length)
    {
      // Copy over the message
      Utils::Memcpy(pkt.GetData() + sizeof (ClientMessage::Data::SessionPrivateData) + numRecipients * sizeof (CRC), data, length);
    }

    // Send to the server our chat message
    pkt.Send(socket);
  }


  //
  // Send synchronous data
  //
  void Client::SendData(CRC key, U32 length, const U8 *data, Bool sync)
  {
    Packet &pkt = Packet::Create
    (
      sync ? ClientMessage::SessionSyncData : ClientMessage::SessionData, 
      sizeof (ClientMessage::Data::SessionData) + length
    );

    CAST(ClientMessage::Data::SessionData *, sessionData, pkt.GetData())

    // Save the key
    sessionData->key = key;

    // Copy over the data
    if (length)
    {
      Utils::Memcpy(sessionData->data, data, length);
    }

    // Send to the server
    pkt.Send(socket);
  }


  //
  // Store synchronous data
  //
  void Client::StoreData(CRC key, CRC index, U32 length, const U8 *data)
  {
    ASSERT(length)

    Packet &pkt = Packet::Create
    (
      ClientMessage::SessionStoreData, 
      sizeof (ClientMessage::Data::SessionStoreData) + length
    );

    CAST(ClientMessage::Data::SessionStoreData *, sessionStoreData, pkt.GetData())

    // Save the key and index
    sessionStoreData->key = key;
    sessionStoreData->index = index;

    // Copy over the data
    Utils::Memcpy(sessionStoreData->data, data, length);

    // Send to the server
    pkt.Send(socket);
  }


  //
  // Clear synchronous data
  //
  void Client::ClearData(CRC key, CRC index)
  {
    ClientMessage::Data::SessionClearData *sessionClearData;
    Packet &pkt = Packet::Create(ClientMessage::SessionClearData, sessionClearData);

    // Copy the key and index
    sessionClearData->key = key;
    sessionClearData->index = index;

    // Send to the server
    pkt.Send(socket);
  }


  //
  // Flush all synchronous data (HostOnly)
  //
  void Client::FlushData()
  {
    // Send to the server
    Packet::Create(ClientMessage::SessionFlushData).Send(socket);
  }


  //
  // Get the name of the session
  //
  const char * Client::GetSessionName()
  {
    return (session ? session->name.str : NULL);
  }


  //
  // Lock the session
  //
  void Client::LockSession()
  {
    Packet::Create(ClientMessage::SessionLock).Send(socket);
  }


  //
  // Unlock the session
  //
  void Client::UnlockSession()
  {
    Packet::Create(ClientMessage::SessionUnlock).Send(socket);
  }


  //
  // Kick a user
  //
  void Client::KickUser(CRC user)
  {
    ClientMessage::Data::SessionKick *sessionKick;
    Packet &pkt = Packet::Create(ClientMessage::SessionKick, sessionKick);
    sessionKick->user = user;
    pkt.Send(socket);
  }


  //
  // Get connection information for the given user
  //
  Std::Data::UserConnection * Client::GetUserConnection(CRC user)
  {
    if (session)
    {
      User *u = session->users.Find(user);
      if (u)
      {
        return (&u->connection);
      }
    }
    return (NULL);
  }


  //
  // Get the local address
  //
  Bool Client::GetLocalAddress(Win32::Socket::Address &address)
  {
    return (socket.GetLocalAddress(address));
  }


  //
  // Migrate this session to someone else
  //
  void Client::MigrateSession()
  {
    LDIAG("Asking server to migrate session")
    Packet::Create(ClientMessage::SessionMigrate).Send(socket);
  }


  //
  // Migration is complete
  //
  void Client::MigrationComplete()
  {
    LDIAG("Telling server that migration is completed")
    Packet::Create(ClientMessage::SessionMigrated).Send(socket);
  }


  //
  // Accept migration
  //
  void Client::AcceptMigration(U16 port, U32 key)
  {
    LDIAG("Accepting migration")

    ClientResponse::Data::SessionRequestMigrateAccept *sessionRequestMigrateAccept;
    Packet &pkt = Packet::Create(ClientResponse::SessionRequestMigrateAccept, sessionRequestMigrateAccept);

    // User the address of the client and substitute the configured port number
    GetLocalAddress(sessionRequestMigrateAccept->address);
    sessionRequestMigrateAccept->address.SetPort(port);

    // The session key allows the other users to join 
    sessionRequestMigrateAccept->key = key;

    // Flag us as having accepted migration
    flags |= ClientFlags::AcceptedMigration;
        
    pkt.Send(socket);
  }


  //
  // Deny migration
  //
  void Client::DenyMigration()
  {
    Packet::Create(ClientResponse::SessionRequestMigrateDeny).Send(socket);
  }


  //
  // Shutdown the client
  //
  void Client::Shutdown()
  {
    // Tell the threat to quit
    eventQuit.Signal();
  }


  //
  // Bogus data
  //
  void Client::Bogus()
  {
    // Bad data, disconnect
    LDIAG("Received bogus data")
    flags |= ClientFlags::Disconnected;
    SendEvent(EventMessage::ServerDisconnected);
  }


  //
  // Migrate the session
  //
  void Client::MigrateSession(const U8 *data, U32 length)
  {
    if (length == sizeof (ServerMessage::Data::SessionSyncDataMigrate))
    {
      CAST(const ServerMessage::Data::SessionSyncDataMigrate *, sessionMigrate, data)

      LDIAG("Server wants us to migrate to " << sessionMigrate->address << " " << HEX(sessionMigrate->key, 8))

      // We need to connect to a new server
      flags |= ClientFlags::Migrating;

      // If we're connected, disconnect us
      if (flags & ClientFlags::Connected)
      {
        LDIAG("Logging out")
        Packet::Create(ClientMessage::UserLogout).Send(socket);
      }

      LDIAG("Closing socket")

      // Close the connection to the current server
      socket.Close();

      // Save the address we're connecting to
      config.address = sessionMigrate->address;

      // If we accepted, override with local address
      if (flags & ClientFlags::AcceptedMigration)
      {
        config.address.SetLocal();
        flags &= ~ClientFlags::AcceptedMigration;
      }

      // Save the migration key
      migrationKey = sessionMigrate->key;

      LDIAG("Opening new socket")

      // Open the socket
      socket.Open();

      if (!socket.IsValid())
      {
        LERR("Couldn't open new socket?")
      }

      // Make sure its using statistics
      socket.UseStats();

      LDIAG("Binding the new socket")

      // Rebind the socket
      socket.Bind(Win32::Socket::Address(ADDR_ANY, 0));

      LDIAG("Seting up event select")

      socket.EventSelect(event, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE);

      // Connect to the new server
      if (!socket.Connect(config.address))
      {
        LWARN("Could not connect socket to " << config.address)
        SendEvent(EventMessage::ServerConnectFailed);
      }
    }
    else
    {
      LWARN("Migrate data is the wrong size")
      SendEvent(EventMessage::ServerConnectFailed);
    }
  }


  //
  // ExtractSyncData
  //
  Bool Client::ExtractSyncData(const U8 *&ptr, U32 &remaining, CRC &type, CRC &from, CRC &key, CRC &index, U32 &length, const U8 *&data)
  {
    if (!remaining)
    {
      return (FALSE);
    }

    if (remaining >= sizeof (U32))
    {
      type = *reinterpret_cast<const U32 *>(ptr);
      switch (type)
      {
        case EventMessage::SyncMigrate:
        {
          if (remaining >= sizeof (ServerMessage::Data::SessionSyncDataMigrate))
          {
            from = 0;
            key = 0;
            index = 0;
            length = sizeof (ServerMessage::Data::SessionSyncDataMigrate);
            data = ptr;

            ptr += sizeof (ServerMessage::Data::SessionSyncDataMigrate);
            remaining -= sizeof (ServerMessage::Data::SessionSyncDataMigrate);
            return (TRUE);
          }
          else
          {
            LDIAG("SessionSyncData Type MIGRATE, Header was too small")
            LDIAG("remaining[" << remaining << "] sizeof (ServerMessage::Data::SessionSyncDataMigrate)[" << sizeof (ServerMessage::Data::SessionSyncDataMigrate) << "]")
            return (FALSE);
          }
          break;
        }

        case EventMessage::SyncData:
        {
          if (remaining >= sizeof (ServerMessage::Data::SessionSyncDataData))
          {
            CAST(const ServerMessage::Data::SessionSyncDataData *, syncData, ptr)
            from = syncData->from;
            key = syncData->key;
            index = 0;
            length = syncData->length;
            data = syncData->data;

            ptr += sizeof (ServerMessage::Data::SessionSyncDataData);
            remaining -= sizeof (ServerMessage::Data::SessionSyncDataData);

            if (remaining >= length)
            {
              ptr += length;
              remaining -= length;
              return (TRUE);
            }
            else
            {
              LDIAG("SessionSyncData Type DATA, Data was too small")
              LDIAG("remaining[" << remaining << "] length[" << length << "]")
              return (FALSE);
            }
          }
          else
          {
            LDIAG("SessionSyncData Type DATA, Header was too small")
            LDIAG("remaining[" << remaining << "] sizeof (ServerMessage::Data::SessionSyncDataData)[" << sizeof (ServerMessage::Data::SessionSyncDataData) << "]")
            return (FALSE);
          }
          break;
        }

        case EventMessage::SyncStore:
        {
          if (remaining >= sizeof (ServerMessage::Data::SessionSyncDataStoreData))
          {
            CAST(const ServerMessage::Data::SessionSyncDataStoreData *, syncStore, ptr)

            from = syncStore->from;
            key = syncStore->key;
            index = syncStore->index;
            length = syncStore->length;
            data = syncStore->data;

            ptr += sizeof (ServerMessage::Data::SessionSyncDataStoreData);
            remaining -= sizeof (ServerMessage::Data::SessionSyncDataStoreData);
            if (remaining >= length)
            {
              ptr += length;
              remaining -= length;
              return (TRUE);
            }
            else
            {
              LDIAG("SessionSyncData Type STORE, Data was too small")
              LDIAG("remaining[" << remaining << "] length[" << length << "]")
              return (FALSE);
            }
          }
          else
          {
            LDIAG("SessionSyncData Type STORE, Header was too small")
            LDIAG("remaining[" << remaining << "] sizeof (ServerMessage::Data::SessionSyncDataStoreData)[" << sizeof (ServerMessage::Data::SessionSyncDataStoreData) << "]")
            return (FALSE);
          }
          break;
        }

        case EventMessage::SyncClear:
        {
          if (remaining >= sizeof (ServerMessage::Data::SessionSyncDataClearData))
          {
            CAST(const ServerMessage::Data::SessionSyncDataClearData *, syncClear, ptr)

            from = syncClear->from;
            key = syncClear->key;
            index = syncClear->index;
            length = 0;
            data = NULL;

            ptr += sizeof (ServerMessage::Data::SessionSyncDataClearData);
            remaining -= sizeof (ServerMessage::Data::SessionSyncDataClearData);
            return (TRUE);
          }
          else
          {
            LDIAG("SessionSyncData Type CLEAR, Header was too small")
            LDIAG("remaining[" << remaining << "] sizeof (ServerMessage::Data::SessionSyncDataClearData)[" << sizeof (ServerMessage::Data::SessionSyncDataClearData) << "]")
            return (FALSE);
          }
          break;
        }

        case EventMessage::SyncFlush:
        {
          if (remaining >= sizeof (ServerMessage::Data::SessionSyncDataFlushData))
          {
            CAST(const ServerMessage::Data::SessionSyncDataFlushData *, syncFlush, ptr)

            from = syncFlush->from;
            key = 0;
            index = 0;
            length = 0;
            data = NULL;

            ptr += sizeof (ServerMessage::Data::SessionSyncDataFlushData);
            remaining -= sizeof (ServerMessage::Data::SessionSyncDataFlushData);
            return (TRUE);
          }
          else
          {
            LDIAG("SessionSyncData Type CLEAR, Header was too small")
            LDIAG("remaining[" << remaining << "] sizeof (ServerMessage::Data::SessionSyncDataFlushData)[" << sizeof (ServerMessage::Data::SessionSyncDataFlushData) << "]")
            return (FALSE);
          }
          break;
        }

        default:
        {
          // Unknown type
          LWARN("SessionSyncData, Unknown type " << HEX(type, 8))
          return (FALSE);
        }
      }
    }
    else
    {
      LDIAG("SyncData was too small to contain a type")
      LDIAG("remaining[" << remaining << "] sizeof (U32)[" << sizeof (U32) << "]")
      return (FALSE);
    }
  }


  //
  // Client::ThreadProc
  //
  // Thread procedure
  //
  U32 STDCALL Client::ThreadProc(void *context)
  {
    Client *client = static_cast<Client *>(context);

    // Initiate the connection
    client->socket.Bind(Win32::Socket::Address(ADDR_ANY, 0));
    client->socket.EventSelect(client->event, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE);
    if (!client->socket.Connect(client->config.address))
    {
      LWARN("Could not connect socket to " << client->config.address)
      client->SendEvent(EventMessage::ServerConnectFailed);
    }

    Bool quit = FALSE;

    Win32::EventIndex::List<2> events;
    events.AddEvent(client->event, NULL);
    events.AddEvent(client->eventQuit, client);

    U32 nextTime = Clock::Time::Ms();

    // Enter processing loop
    while (!quit)
    {
      void *context;
    
      S32 remaining = nextTime - Clock::Time::Ms();

      if (remaining > 0 && events.Wait(context, FALSE, remaining))
      {
        if (!context)
        {
          Win32::Socket::NetworkEvents networkEvents;
          client->socket.EnumEvents(client->event, networkEvents);

          if (networkEvents.GetEvents() & FD_CONNECT)
          {
            S32 error = networkEvents.GetError(FD_CONNECT_BIT);
            if (error)
            {
              LDIAG("Connection to server failed")
              client->SendEvent(EventMessage::ServerConnectFailed);
            }
            else
            {
              LDIAG("Connection established with server")

              /*
              // Are we reconnecting ?
              if (client->flags & ClientFlags::Reconnecting)
              {
                // Attempt reconnect to the server
                ClientMessage::Data::UserReconnect *reconnect;
                Packet &pkt = Packet::Create(ClientMessage::UserReconnect, reconnect);

                reconnect->secret = client->secret;
                reconnect->sequence = client->sequence;
                pkt.Send(client->socket);

                LDIAG("Sent UserReconnect to server")
              }
              else
              */
              {
                // Attempt to login to the server
                ClientMessage::Data::UserLogin *login;
                Packet &pkt = Packet::Create(ClientMessage::UserLogin, login);

                login->version = Version::GetBuildNumber();
                client->socket.GetLocalAddress(login->address);
                login->name = client->config.userName.str;
                login->migratable = client->config.migratable;
                pkt.Send(client->socket);

                LDIAG("Sent UserLogin to server")
              }
            }
          }

          if (networkEvents.GetEvents() & FD_READ)
          {
            // Get the packet system to accept the data from the socket
            Packet::Accept(*client->packetBuffer, client->socket);

            // Extract the packets out of the buffer
            while (const Packet *pkt = Packet::Extract(*client->packetBuffer))
            {
              // Have the client process the packet
              client->ProcessPacket(*pkt);
            }
          }

          if (networkEvents.GetEvents() & FD_WRITE)
          {
            LDIAG("Able to write to server")
          }

          if (networkEvents.GetEvents() & FD_CLOSE)
          {
            // Close the socket on this side immediately
            client->socket.Close();

            /*
            // Are we logged in to the server
            if (client->flags & ClientFlags::Connected)
            {
              // Attempt reconnection
              LDIAG("We lost our connection to the server, attempting reconnection")

              client->flags |= ClientFlags::Reconnecting;

              client->socket.Close();
              client->socket.Open();
              client->socket.Bind(Win32::Socket::Address(ADDR_ANY, 0));
              client->socket.EventSelect(client->event, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE);
              if (!client->socket.Connect(client->config.address))
              {
                LWARN("Could not connect socket to " << client->config.address)
                client->SendEvent(EventMessage::ServerConnectFailed);
              }
            }
            else
            */

            {
              // We only want a single disconnected message
              if (!(client->flags & ClientFlags::Disconnected))
              {
                client->flags |= ClientFlags::Disconnected;
                LDIAG("Connection to server has been severed")
                client->SendEvent(EventMessage::ServerDisconnected);
              }
            }
          }
        }
        else if (context == client)
        {
          // Quit event
          quit = TRUE;
        }
        else
        {
          // Unkown context ???
        }
      }
      else
      {
        // The wait failed or timed out ... lets assumed it timed out
        nextTime += client->config.pingInterval;

        // Send off a ping
        Network::Ping::Send(client->config.address, PingCallback, (void *) client->index);
      }
    }

    // Delete the client
    client->thread.Clear();
    delete client;

    return (0x6666);
  }


  //
  // Ping callback
  //
  void Client::PingCallback(const Win32::Socket::Address &address, U32 rtt, U32 hopCount, void *context)
  {
    U32 index = reinterpret_cast<U32>(context);
    clientsCritSec.Enter();
    Client *client = clients[index];

    // Is there a client who is in a session ?
    if (client && client->session)
    {
      U16 ping = U16(Clamp<U32>(0, rtt, U16_MAX));
      U8 hops = U8(Clamp<U32>(0, hopCount, U8_MAX));

      // Update client ping information
      client->Ping(ping, hops);

      // Send off an event to the client owner
      client->SendEvent
      (
        EventMessage::Ping, 
        new EventMessage::Data::Ping(address, rtt, hopCount)
      );
    }
    clientsCritSec.Exit();
  }


  //
  // Client::ProcessPacket
  //
  // Handle an incomming packet
  //
  void Client::ProcessPacket(const Packet &packet)
  {
    switch (packet.GetCommand())
    {
      case ServerResponse::UserConnected:
      {
        LDIAG("Successfully connected to server")
        SendEvent(EventMessage::ServerConnected);

        CAST(const ServerResponse::Data::UserConnected *, userConnected, packet.GetData())

        // The user is now connected
        flags |= ClientFlags::Connected;

        // Save the secret for reconnection purposes
        secret = userConnected->secret;

        // Are we migrating ?
        if (flags & ClientFlags::Migrating)
        {
          LDIAG("We're migrating")
          ClientMessage::Data::UserMigrating *userMigrating;
          Packet &pkt = Packet::Create(ClientMessage::UserMigrating, userMigrating);
          userMigrating->migrationKey = migrationKey;
          pkt.Send(socket);

          flags &= ~ClientFlags::Migrating;
        }
        break;
      }

      case ServerResponse::UserNotInSession:
        LDIAG("User is not in a session")
        break;

      case ServerResponse::UserMigrated:
        LDIAG("We successfully migrated")
        SendEvent(EventMessage::SessionMigrateComplete);
        break;

      case ServerResponse::UserMigrationFailed:
        LDIAG("We failed to migrate")
        SendEvent(EventMessage::SessionMigrateFailed);
        break;

      case ServerResponse::UserMigrateNotNeeded:
        LDIAG("No need to migrate")
        SendEvent(EventMessage::SessionMigrateNotNeeded);
        break;

      case ServerResponse::ServerShuttingDown:
        LDIAG("Server is shutting down")
        break;

      case ServerResponse::SessionCreated:
        LDIAG("Session successfully created")
        ASSERT(!session);
        session = new Session;
        SendEvent(EventMessage::SessionCreated);
        break;

      case ServerResponse::SessionConnected:
        LDIAG("Session successfully connected")
        ASSERT(!session);
        session = new Session;
        SendEvent(EventMessage::SessionConnected);
        break;

      case ServerResponse::SessionAlreadyExists:
        LDIAG("Session with that name already exists")
        SendEvent(EventMessage::SessionAlreadyExists);
        break;

      case ServerResponse::SessionJoined:
        LDIAG("Session successfully joined")
        ASSERT(!session);
        session = new Session;
        SendEvent(EventMessage::SessionJoined);
        break;

      case ServerResponse::SessionClosed:
        LDIAG("Session has been closed")
        ASSERT(session);
        delete session;
        session = NULL;
        SendEvent(EventMessage::SessionClosed);
        break;

      case ServerResponse::SessionList:
        LDIAG("Session list retrived")
        break;

      case ServerResponse::SessionHostOnly:
        LDIAG("Only the host can do that")
        break;

      case ServerResponse::SessionNotFound:
        LDIAG("Session was not found")
        break;

      case ServerResponse::SessionBadUser:
        LDIAG("Session reports bad user name")
        SendEvent(EventMessage::SessionBadUser);
        break;

      case ServerResponse::SessionBadPassword:
        LDIAG("Session reports bad password")
        SendEvent(EventMessage::SessionBadPassword);
        break;

      case ServerResponse::SessionFull:
        LDIAG("Session is full")
        SendEvent(EventMessage::SessionFull);
        break;

      case ServerResponse::SessionIsLocked:
        LDIAG("Session is locked")
        SendEvent(EventMessage::SessionIsLocked);
        break;

      case ServerMessage::SessionKicked:
        LDIAG("Kicked from session")
        SendEvent(EventMessage::SessionKicked);
        break;

      case ServerResponse::SessionLocalOnly:
        LDIAG("Attempt to perform an operation which is only allowed for local players")
        break;

      case ServerResponse::SessionSingleOnly:
        LDIAG("Only one session can be created on a non stand alone server")
        break;

      case ServerMessage::ServerShutdown:
        LDIAG("Server is shutting down")
        flags &= ~ClientFlags::Connected;
        break;

      case ServerMessage::SessionUserAdded:
      {
        ServerMessage::Data::SessionUserAdded *sessionUserAdded;
        if (packet.GetData(sessionUserAdded))
        {
          ASSERT(session)

          // Tell everyone that there's a new user
          SendEvent
          (
            EventMessage::SessionUserAdded,
            new EventMessage::Data::SessionUserAdded(sessionUserAdded->who)
          );

          if (!session->FindUser(sessionUserAdded->who.crc))
          {
            session->AddUser(sessionUserAdded->who);

            LDIAG("User " << sessionUserAdded->who << " has entered the sesion")

            // If this is us then tell someone
            if (sessionUserAdded->who.crc == config.userName.crc)
            {
              SendEvent(EventMessage::SessionLocalUserAdded);
            }

            // Update the session information
            SendEvent
            (
              EventMessage::SessionInfo, 
              new EventMessage::Data::SessionInfo(*session)
            );
          }
        }
        else
        {
          Bogus();
        }

        break;
      }

      case ServerMessage::SessionUserRemoved:
      {
        ServerMessage::Data::SessionUserRemoved *sessionUserRemoved;
        if (packet.GetData(sessionUserRemoved))
        {
          User *user = session->FindUser(sessionUserRemoved->who);
          if (user)
          {
            LDIAG("User " << user->GetName() << " has left the session")

            // Inform that the user left
            SendEvent
            (
              EventMessage::SessionUserRemoved,
              new EventMessage::Data::SessionUserRemoved(user->GetName())
            );

            // Remove them from our local session
            session->RemoveUser(sessionUserRemoved->who);

            // Update the session information
            SendEvent
            (
              EventMessage::SessionInfo, 
              new EventMessage::Data::SessionInfo(*session)
            );
          }
          else
          {
            LDIAG("User " << sessionUserRemoved->who << "was not in session")
          }
        }
        else
        {
          Bogus();
        }
        break;
      }

      case ServerMessage::SessionUserDisconnected:
      {
        ServerMessage::Data::SessionUserDisconnected *sessionUserDisconnected;
        if (packet.GetData(sessionUserDisconnected))
        {
          User *user = session->FindUser(sessionUserDisconnected->who);
          if (user)
          {
            // Inform that the user disconnected
            SendEvent
            (
              EventMessage::SessionUserDisconnected,
              new EventMessage::Data::SessionUserDisconnected(user->GetName())
            );
          }
          else
          {
            LDIAG("User " << sessionUserDisconnected->who << "was not in session")
          }
        }
        else
        {
          Bogus();
        }
        break;
      }

      case ServerMessage::SessionUserReconnected:
      {
        ServerMessage::Data::SessionUserReconnected *sessionUserReconnected;
        if (packet.GetData(sessionUserReconnected))
        {
          User *user = session->FindUser(sessionUserReconnected->who);
          if (user)
          {
            // Inform that the user reconnected
            SendEvent
            (
              EventMessage::SessionUserReconnected,
              new EventMessage::Data::SessionUserReconnected(user->GetName())
            );
          }
          else
          {
            LDIAG("User " << sessionUserReconnected->who << "was not in session")
          }
        }
        else
        {
          Bogus();
        }
        break;
      }

      case ServerMessage::SessionInfo:
      {
        CAST(const ServerMessage::Data::SessionInfo *, sessionInfo, packet.GetData())

        ASSERT(session)
        session->name = sessionInfo->name.str;
        session->flags = sessionInfo->flags;
        session->maxUsers = sessionInfo->maxUsers;

        SendEvent
        (
          EventMessage::SessionInfo, 
          new EventMessage::Data::SessionInfo(*session)
        );
        break;
      }

      case ServerMessage::SessionData:
      {
        CAST(const ServerMessage::Data::SessionData *, sessionData, packet.GetData())
        
        EventMessage::Data::SessionData *data = new EventMessage::Data::SessionData;

        data->from = sessionData->from;
        data->key = sessionData->key;
        data->length = packet.GetLength() - sizeof(ServerMessage::Data::SessionData);
        if (data->length)
        {
          data->data = new U8[data->length];
          Utils::Memcpy(data->data, sessionData->data, data->length);
        }
        else
        {
          data->data = NULL;
        }

        SendEvent(EventMessage::SessionData, data);


        // Is this data we're interested in ?
        switch (sessionData->key)
        {
          case Std::UserPing:
          {
            CAST(const Std::Data::UserPing *, userPing, sessionData->data)
            ASSERT(session)
            User *user = session->FindUser(sessionData->from);
            if (user)
            {
              user->connection.ProcessPing(userPing->ping, userPing->hops);
            }
            break;
          }
        }
        break;
      }

      case ServerMessage::SessionPrivateData:
      {
        CAST(const ServerMessage::Data::SessionPrivateData *, sessionPrivateData, packet.GetData())
        EventMessage::Data::SessionPrivateData *data = new EventMessage::Data::SessionPrivateData;

        data->from = sessionPrivateData->from;
        data->key = sessionPrivateData->key;
        data->length = packet.GetLength() - sizeof(ServerMessage::Data::SessionPrivateData);
        if (data->length)
        {
          data->data = new U8[data->length];
          Utils::Memcpy(data->data, sessionPrivateData->data, data->length);
        }
        else
        {
          data->data = NULL;
        }

        SendEvent(EventMessage::SessionPrivateData, data);
        break;
      }

      case ServerMessage::SessionSyncData:
      {
        CAST(const ServerMessage::Data::SessionSyncData *, sessionSyncData, packet.GetData())
        EventMessage::Data::SessionSyncData *data = new EventMessage::Data::SessionSyncData;

        // Save the sequence number
        sequence = sessionSyncData->seq;

        data->time = Clock::Time::UsLwr();
        data->seq = sessionSyncData->seq;
        data->interval = sessionSyncData->interval;
        data->length = packet.GetLength() - sizeof (ServerMessage::Data::SessionSyncData);
        if (data->length)
        {
          data->data = new U8[data->length];
          Utils::Memcpy(data->data, sessionSyncData->data, data->length);
        }
        else
        {
          data->data = NULL;
        }
        SendEvent(EventMessage::SessionSyncData, data);
        break;
      }

      case ServerMessage::SessionRequestMigrate:
      {
        CAST(const ServerMessage::Data::SessionRequestMigrate *, sessionMigrateRequest, packet.GetData())

        // Send an event so that they can make the determination on whether to accept or not
        SendEvent(EventMessage::SessionMigrateRequest, new EventMessage::Data::SessionMigrateRequest(sessionMigrateRequest->seq));
        break;
      }

      default:
        // Unknown packet command
        LDIAG("Unknown Packet Command " << HEX(packet.GetCommand(), 8) << " from server")
        break;
    }
  }


  //
  // Process ping information
  //
  void Client::Ping(U16 ping, U8 hops)
  {
    Std::Data::UserPing userPing;
    userPing.ping = ping;
    userPing.hops = hops;

    // Add to local connection information
    connection.ProcessPing(ping, hops);

    // Save the ping to everyone
    SendData(Std::UserPing, sizeof (Std::Data::UserPing), reinterpret_cast<U8 *>(&userPing), FALSE);
  }

}
