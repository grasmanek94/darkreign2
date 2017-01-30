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
#include "styxnet_server.h"
#include "styxnet_server_private.h"
#include "styxnet_clientmessage.h"
#include "styxnet_clientresponse.h"
#include "styxnet_servermessage.h"
#include "styxnet_serverresponse.h"
#include "styxnet_private.h"
#include "styxnet_event.h"
#include "clock.h"
#include "version.h"


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Server
  //


  //
  // Server::Server
  //
  // Constructor
  //
  Server::Server(const Config &config, Bool standalone)
  : config(config),
    flags(standalone ? ServerFlags::StandAlone : 0),
    users(&User::nodeServer),
    disconnected(&User::nodeServer),
    sessions(&Session::nodeServer),
    migrations(&Migration::nodeServer)
  {
    AddServer();

    LDIAG("Starting Server ...")
    LDIAG("Listening on port " << config.port)
    thread.Start(ThreadProc, this);
    thread.SetPriority(Win32::Thread::ABOVE_NORMAL);
  }


  //
  // Server::~Server
  //
  // Destructor
  //
  Server::~Server()
  {
    RemoveServer();
  }


  //
  // Get the local address
  //
  Bool Server::GetLocalAddress(Win32::Socket::Address &address, CRC who)
  {
    Bool success = FALSE;

    mutexUsers.Wait();

    for (NList<User>::Iterator u(&users); *u; ++u)
    {
      if ((*u)->name.crc == who)
      {
        success = (*u)->socket.GetLocalAddress(address);
        break;
      }
    }
    mutexUsers.Signal();

    return (success);
  }


  //
  // Shutdown the server
  //
  void Server::Shutdown()
  {
    LDIAG("Stopping Server ...")

    // Tell thread to quit
    eventQuit.Signal();
  }


  //
  // Setup a session for migration
  //
  void Server::SetupMigration(const SessionName &name, U32 maxUsers, U32 seq, U32 &key)
  {
    mutexSessions.Wait();

    LDIAG("Migration Session " << name << " created")

    // Create the session
    Session *session = new Session(*this, NULL, name, 0, maxUsers);

    // The session is locked and is migrating
    session->flags |= SessionFlags::Locked | SessionFlags::MigratingTo;

    // Set the sequence number of this session
    session->sequenceNumber = seq;

    // Add the session
    sessions.Add(name.crc, session);

    // Build the migration key
    key = name.crc;

    mutexSessions.Signal();
  }


  //
  // Server::ThreadProc
  //
  // Thread procedure
  //
  U32 STDCALL Server::ThreadProc(void *context)
  {
    LDIAG("Starting Server Thread")

    Server *server = static_cast<Server *>(context);

    // Setup a socket to listen for incomming connections
    Win32::Socket listenerSocket;
    listenerSocket.Bind(Win32::Socket::Address(ADDR_ANY, server->config.port));

    Bool flag = TRUE;
    listenerSocket.SetSockOpt(SOL_SOCKET, SO_REUSEADDR, (const char *) &flag, sizeof (Bool));
    listenerSocket.Listen(10);
    Win32::EventIndex listenerEvent;
    listenerSocket.EventSelect(listenerEvent, FD_ACCEPT);

    flag = TRUE;
    listenerSocket.SetSockOpt(SOL_SOCKET, U32(SO_DONTLINGER), (char *) &flag, sizeof (Bool));

    // Setup a socket to reply to session searchers
    Win32::Socket explorerSocket(Win32::Socket::UDP);
    explorerSocket.Bind(Win32::Socket::Address(ADDR_ANY, server->config.port));
    Win32::EventIndex explorerEvent;
    explorerSocket.EventSelect(explorerEvent, FD_READ);

    // At this point the we can accept connections so tell the world that the server has started
    server->SendEvent(EventMessage::ServerStarted);

    Bool quit = FALSE;
    U32 nextTime = Clock::Time::Ms() + server->config.updateInterval;

    Win32::EventIndex::List<maximumUsers * 2 + 3> events;

    events.AddEvent(server->eventQuit, server);
    events.AddEvent(listenerEvent, &listenerSocket);
    events.AddEvent(explorerEvent, &explorerSocket);

    // Enter processing loop
    while (!quit || server->migrations.GetCount())
    {
      void *context;
      
      S32 remaining = nextTime - Clock::Time::Ms();

      if (remaining > 0 && events.Wait(context, FALSE, remaining))
      {
        if (context == server)
        {
          // Quit event
          quit = TRUE;

          // Close the listenening socket so we don't accept any more connections
          listenerSocket.Close();

          // Close the explorer socket so no one else finds us
          explorerSocket.Close();

          // Tell the server we're shutting down
          server->flags |= ServerFlags::ShuttingDown;
        }
        else if (context == &listenerSocket)
        {
          // Listener event
          Win32::Socket::NetworkEvents networkEvents;
          listenerSocket.EnumEvents(listenerEvent, networkEvents);

          if (networkEvents.GetEvents() & FD_ACCEPT)
          {
            Win32::Socket::Address address;

            // Create a new user which uses this socket
            User *user = new User(listenerSocket.Accept(address), address);

            // Is this a local user ?
            if (address.IsLocal())
            {
              user->flags |= UserFlags::Local;
              LDIAG("New Connection from " << address << " who is local")
            }
            else
            {
              LDIAG("New Connectiong from " << address << " who is remote")
            }

            server->mutexUsers.Wait();

            // Add to the list of users connected to this server
            server->users.Append(user);

            server->mutexUsers.Signal();

            // Add the new users event handle
            events.AddEvent(user->event, user);
          }
        }
        else if (context == &explorerSocket)
        {
          Win32::Socket::NetworkEvents networkEvents;
          explorerSocket.EnumEvents(explorerEvent, networkEvents);

          if (networkEvents.GetEvents() & FD_READ)
          {
            // Call Recv merely to find out who its from
            Win32::Socket::Address address;
            U32 data;
            explorerSocket.Recv(address, (U8 *) &data, sizeof (U32));

            // Tell them about all of our sessions (if any)
            NBinTree<Session, CRC>::Iterator s(&server->sessions);
            for (!s; *s; ++s)
            {
              (*s)->SendInfo(explorerSocket, address);
            }
          }          
        }
        else
        {
          // User event (the context is a user pointer)
          User *user = static_cast<User *>(context);

          Win32::Socket::NetworkEvents networkEvents;
          user->socket.EnumEvents(user->event, networkEvents);

          if (networkEvents.GetEvents() & FD_READ)
          {
            // Get the packet system to accept the data from the socket
            Packet::Accept(*user->packetBuffer, user->socket);

            // Extract the packets out of the buffer
            while (const Packet *pkt = Packet::Extract(*user->packetBuffer))
            {
              if (!server->ProcessPacket(*user, *pkt))
              {
                // The user has been discarded
                LDIAG("User " << user->address << " tried to pass a bogus packet!");

                // Close their socket immediately
                user->socket.Close();

                // Remove from the events
                events.RemoveEvent(user->event);

                // Dispose of the user
                server->mutexUsers.Wait();
                server->users.Dispose(user);
                server->mutexUsers.Signal();
                break;
              }
            }
          }

          if (networkEvents.GetEvents() & FD_WRITE)
          {
            LDIAG("Able to write to user")
          }

          if (networkEvents.GetEvents() & FD_CLOSE)
          {
            LDIAG("User has closed their connection")

            // Close the socket on this side immediately
            user->socket.Close();

            // Remove this user from the event list
            events.RemoveEvent(user->event);

            /*
            if (user->flags & UserFlags::LoggedIn)
            {
              LDIAG("User was logged in, keeping their info incase of reconnect")

              // User was logged in, then the disconnection 
              // was temporary, place the user into the pile 
              // of users who may attempt reconnection

              server->mutexUsers.Wait();

              // Move the user to the disconnected list
              server->users.Unlink(user);
              server->disconnected.Append(user);
              user->disconnectTime = Clock::Time::Ms();

              server->mutexUsers.Signal();

              // If the user is in a session tell everyone 
              // in the session that they disconnected
              if (user->session)
              {
                ServerMessage::Data::SessionUserDisconnected *disconnected;
                Packet &pkt = Packet::Create(ServerMessage::SessionUserDisconnected, disconnected);
                disconnected->who = user->GetName().crc;
                user->session->SendToAll(pkt);
                pkt.Destroy();
              }
            }
            else
            */
            {
              LDIAG("User had logged out, disposing of them")

              // User has logged out, dispose of them
              server->mutexUsers.Wait();

              // Delete the user
              server->users.Dispose(user);
              user = NULL;

              server->mutexUsers.Signal();
            }
          }
        }
      }
      else
      {
        // The wait failed or timed out ... lets assumed it timed out
        nextTime += server->config.updateInterval;

        // Allow each migration to update
        NList<Migration>::Iterator m(&server->migrations);
        while (Migration *migration = m++)
        {
          if (migration->Process())
          {
            migration->session.migration = NULL;
            server->migrations.Dispose(migration);
          }
        }

        server->mutexSessions.Wait();

        // Allow each session to update
        NBinTree<Session, CRC>::Iterator s(&server->sessions);

        while (Session *session = s++)
        {
          if (session->Process())
          {
            server->sessions.Dispose(session);

            // Did we just delete the session and we're not stand alone ?
            if (!(server->flags & ServerFlags::StandAlone))
            {
              server->SendEvent(EventMessage::ServerNoSessions);
            }
          }
        }

        server->mutexSessions.Signal();

        /*
        // Check the disconnected users to see if they've timed out
        U32 time = Clock::Time::Ms();
        server->mutexUsers.Wait();
        NList<User>::Iterator u(&server->disconnected); 
        
        while (User *user = u++)
        {
          if ((time - user->disconnectTime) > maximumDisconnectTime)
          {
            server->disconnected.Dispose(user);
          }
        }
        server->mutexUsers.Signal();
        */
      }
    }

    // Cleanup (the order is important!)
    server->mutexUsers.Wait();
    for (NList<User>::Iterator u(&server->users); *u; ++u)
    {
      Packet::Create(ServerMessage::ServerShutdown).Send((*u)->socket);
    }
    server->users.DisposeAll();
    //server->disconnected.DisposeAll();
    server->mutexUsers.Signal();

    server->mutexSessions.Wait();
    server->sessions.DisposeAll();
    server->mutexSessions.Signal();

    LDIAG("Stopping Server Thread")

    // Delete the server
    server->thread.Clear();
    delete server;

    return (0x6666);
  }


  //
  // Server::ProcessPacket
  //
  // Handle an incomming packet
  //
  Bool Server::ProcessPacket(User &user, const Packet &packet)
  {
    // Is this a server command or a custom command
    switch (packet.GetCommand())
    {
      //
      // Client wishes to login
      //
      case ClientMessage::UserLogin:
      {
        ClientMessage::Data::UserLogin *data;
        if (packet.GetData(data))
        {
          LDIAG("Received UserLogin: " << data->name.str)

          // Does this user have the correct version ?
          if (data->version == U16(Version::GetBuildNumber()))
          {
            user.remoteAddress = data->address;
            user.name = data->name.str;
            user.flags |= UserFlags::LoggedIn;
            user.flags |= data->migratable ? UserFlags::AcceptMigration : 0;

            // Generate a secret which can be used by the client for reconnecting
            user.secret = Clock::Time::UsLwr();

            // Tell the user that they are connected
            ServerResponse::Data::UserConnected *connected;
            Packet &response = Packet::Create(ServerResponse::UserConnected, connected);
            connected->secret = user.secret;
            response.Send(user.socket);

            LDIAG("Secret for reconnection is " << user.secret)

            if (!(user.remoteAddress == user.address))
            {
              user.flags |= UserFlags::BehindNAT;
              LDIAG("User is behind an Address Translator!")
            }

            SendEvent
            (
              EventMessage::ServerUserAdded, 
              new EventMessage::Data::ServerUserAdded(user.name, user.address)
            );
          }
          else
          {
            LDIAG("Version mismatch! us:" << U16(Version::GetBuildNumber()) << " them: " << data->version);
            return (FALSE);
          }
        }
        else
        {
          return (FALSE);
        }
        break;
      }


      //
      // Client wishes to logout
      //
      case ClientMessage::UserLogout:
      {
        LDIAG("Received UserLogout: " << user.name)

        // Clear login flags
        user.flags &= ~UserFlags::LoggedIn;

        // Tell the server that the user has been removed
        SendEvent
        (
          EventMessage::ServerUserRemoved, 
          new EventMessage::Data::ServerUserRemoved(user.name)
        );

        break;
      }


      //
      // Client is migrating
      //
      case ClientMessage::UserMigrating:
      {
        ClientMessage::Data::UserMigrating *data;
        if (packet.GetData(data))
        {
          LDIAG("Received UserMigrating")

          // Find the session which has that name
          Session *session = sessions.Find(data->migrationKey);

          if (session)
          {
            if (session->flags & SessionFlags::MigratingTo)
            {
              LDIAG("User " << user.GetName() << " migrated")

              // Add the user
              session->AddUser(user);

              // If this user is local, make them the host
              if (user.address.IsLocal())
              {
                LDIAG("User is local, making them the host")
                session->host = &user;
              }

              // Tell the user that they are migrated
              Packet::Create(ServerResponse::UserMigrated).Send(user.socket);
            }
            else
            {
              LDIAG("Session isn't migrating")
              Packet::Create(ServerResponse::UserMigrationFailed).Send(user.socket);
            }
          }
          else
          {
            LDIAG("Couldn't migrate, session not found")
            Packet::Create(ServerResponse::UserMigrationFailed).Send(user.socket);
          }
        }
        else
        {
          return (FALSE);
        }

        break;
      }


      //
      // Client wishes to reconnect
      //
      case ClientMessage::UserReconnect:
      {
        ClientMessage::Data::UserReconnect *data;
        if (packet.GetData(data))
        {
          LDIAG("Received UserReconnect : Secret " << data->secret)

          // Rummage through the existing pile of disconnected users
          Bool found = FALSE;
        
          mutexUsers.Wait();
          for (NList<User>::Iterator u(&disconnected); *u; ++u)
          {
            if ((*u)->address.GetIP() == user.address.GetIP() && (*u)->secret == data->secret)
            {
              LDIAG("User " << (*u)->GetName() << " is attempting reconnection")

              // Were they in a session ?
              // Do we have the data their missing
              if (!(*u)->session || ((*u)->session->sequenceNumber - data->sequence) < maximumOldData)
              {
                found = TRUE;

                // Copy over information from the old user
                user.remoteAddress = (*u)->remoteAddress;
                user.name = (*u)->name.str;
                user.session = (*u)->session;
                user.flags = (*u)->flags;

                // Dispose of the old user
                (*u)->session = NULL;
                disconnected.Dispose(*u);

                // Tell the user that they are reconnected
                Packet::Create(ServerResponse::UserReconnected).Send(user.socket);

                // If they're in a session tell everyone
                if (user.session)
                {
                  ServerMessage::Data::SessionUserReconnected *reconnected;
                  Packet &pkt = Packet::Create(ServerMessage::SessionUserReconnected, reconnected);
                  reconnected->who = user.GetName().crc;
                  user.session->SendToAll(pkt);
                  pkt.Destroy();
                }

                // Calculate the number of sequences they missed out on and send them over
                if (user.session)
                {
                  U32 missing = user.session->sequenceNumber - data->sequence;
                  LDIAG("User is missing " << missing << " sync data packets")

                  U32 index = user.session->GetOldIndex(missing);

                  LDIAG("Session index is " << user.session->oldPktsIndex << " our index is " << index)
                  while (missing--)
                  {
                    LDIAG("Sending index " << index)
                    user.session->oldPkts[index]->Send(user.socket);
                    index = user.session->GetNextIndex(index);
                  }
                }
              }
              break;
            }
          }

          // Did they reconnect ?
          if (!found)
          {
            // Tell them they failed
            Packet::Create(ServerResponse::UserReconnectFailed).Send(user.socket);

            // Sever them from the game
            users.Dispose(&user);
          }

          mutexUsers.Signal();
        }
        else
        {
          return (FALSE);
        }
        break;
      }


      //
      // Client wants to create a session
      //
      case ClientMessage::SessionCreate:
      {
        // Convert the packet data into the SessionCreate data
        ClientMessage::Data::SessionCreate *data;
        if (packet.GetData(data))
        {
          LDIAG("User " << user.GetName() << " wants to create a session " << data->name << " max " << data->maxUsers)

          // If this isn't a standalone server there can be only one session
          // created and it can only be created by a user who is connected locally
          if (!(flags & ServerFlags::StandAlone))
          {
            if (!(user.flags & UserFlags::Local))
            {
              LDIAG("Only local users can create sessions on non standalone server!")
              Packet::Create(ServerResponse::SessionLocalOnly).Send(user.socket);
              break;
            }

            mutexSessions.Wait();

            if (sessions.GetCount())
            {
              LDIAG("We already have a session and we're not a stand alone server")
              Packet::Create(ServerResponse::SessionSingleOnly).Send(user.socket);
              mutexSessions.Signal();
              break;
            }

            mutexSessions.Signal();
          }

          mutexSessions.Wait();

          // Check to see if there's already a session with that name
          if (sessions.Find(data->name.crc))
          {
            LDIAG("Session " << data->name << " already exists")

            // Tell the client that a session already exists with that name
            Packet::Create(ServerResponse::SessionAlreadyExists).Send(user.socket);
          }
          else
          {
            LDIAG("Session " << data->name << " created")

            // Tell the client that the session has been created
            Packet::Create(ServerResponse::SessionCreated).Send(user.socket);

            // Create the new session and place the client which created the session into it
            sessions.Add(data->name.crc, new Session(*this, &user, data->name, data->password, data->maxUsers));
          }

          mutexSessions.Signal();
        }
        else
        {
          return (FALSE);
        }
        break;
      }


      //
      // Client want to connect to a session
      //
      case ClientMessage::SessionConnect:
      {
        // Convert the packet data into the SessionCreate data
        ClientMessage::Data::SessionConnect *data;
        if (packet.GetData(data))
        {
          LDIAG("User " << user.GetName() << " wants to connect to session " << data->name << " max " << data->maxUsers)

          mutexSessions.Wait();

          // Find the session which has that name
          Session *session = sessions.Find(data->name.crc);

          // Check to see if there's already a session with that name
          if (session)
          {
            LDIAG("Session exists, attempting to join")

            // Since the session exists we merely want to join it

            // Does the password match ?
            if (session->password == data->password)
            {
              // Is this user alread in the session ?
              if (session->users.Find(user.name.crc))
              {
                LDIAG("User already in session")
                Packet::Create(ServerResponse::SessionBadUser).Send(user.socket);
              }
              // Would this user cause us to go over the maximum count
              else if (session->users.GetCount() == session->maxUsers)
              {
                LDIAG("Session full")
                Packet::Create(ServerResponse::SessionFull).Send(user.socket);
              }
              else
              {
                LDIAG("User " << user.GetName() << " connected")
                Packet::Create(ServerResponse::SessionConnected).Send(user.socket);

                // Add the user
                session->AddUser(user);
              }
            }
            else
            {
              LDIAG("Bad password")
              Packet::Create(ServerResponse::SessionBadPassword).Send(user.socket);
            }
          }
          else
          {
            LDIAG("Session didn't exist, creating a new session")

            // Tell the client that the session has been created
            Packet::Create(ServerResponse::SessionConnected).Send(user.socket);

            // Create the new session and place the client which created the session into it
            sessions.Add(data->name.crc, new Session(*this, &user, data->name, data->password, data->maxUsers));
          }

          mutexSessions.Signal();
        }
        else
        {
          return (FALSE);
        }
        break;
      }


      //
      // Client wants to destroy a session
      //
      case ClientMessage::SessionDestroy:
      {
        // Destroy a session
        ClientMessage::Data::SessionDestroy *data;
        if (packet.GetData(data))
        {
          LDIAG("User " << user.GetName() << " wants to destroy session " << data->name)

          mutexSessions.Wait();

          // Find the session which has that name
          Session *session = sessions.Find(data->name);

          if (session)
          {
            // Is this user the host of this session ?
            if (session->host == &user)
            {
              LDIAG("User is host, deleting the session")

              // Destroy the session
              sessions.Dispose(session);
            }
            else
            {
              LDIAG("User isn't host")
              Packet::Create(ServerResponse::SessionHostOnly).Send(user.socket);
            }
          }
          else
          {
            LDIAG("Session not found")
            Packet::Create(ServerResponse::SessionNotFound).Send(user.socket);
          }
          mutexSessions.Signal();
        }
        else
        {
          return (FALSE);
        }
        break;
      }


      //
      // Client wants to get the list of sessions
      //
      case ClientMessage::SessionList:
      {
        mutexSessions.Wait();

        // Compose a packet which gives the current list of sessions
        Packet &response = Packet::Create(
          ServerResponse::SessionList, 
          sizeof (ServerResponse::Data::SessionList) + 
          sizeof (ServerResponse::Data::Session) * sessions.GetCount());
      
        ServerResponse::Data::SessionList *sessionList = 
          reinterpret_cast<ServerResponse::Data::SessionList *>(response.GetData()); 

        sessionList->numSessions = sessions.GetCount();
      
        ServerResponse::Data::Session *session = 
          reinterpret_cast<ServerResponse::Data::Session *>
          (response.GetData() + sizeof (ServerResponse::Data::SessionList));

        NBinTree<Session, CRC>::Iterator s(&sessions);
        for (!s; *s; ++s)
        {
          session->name = (*s)->name;
          session->numUsers = (*s)->users.GetCount();
          session->maxUsers = (*s)->maxUsers;
          session->password = (*s)->password ? 1 : 0;
          session++;
        }

        mutexSessions.Signal();

        // Send the packet to the user who requested the information
        response.Send(user.socket);

        break;
      }


      //
      // Client wants to join a session
      //
      case ClientMessage::SessionJoin:
      {
        // Join a session
        ClientMessage::Data::SessionJoin *data;
        if (packet.GetData(data))
        {
          LDIAG("User " << user.GetName() << " wants to join to session " << data->name)

          mutexSessions.Wait();

          // Find the session which has that name
          Session *session = sessions.Find(data->name);

          if (session)
          {
            // Is the session closed
            if (session->flags & SessionFlags::Locked)
            {
              LDIAG("Session is locked")
              Packet::Create(ServerResponse::SessionIsLocked).Send(user.socket);
            }
            else

            // Does the password match ?
            if (session->password == data->password)
            {
              // Is this user alread in the session ?
              if (session->users.Find(user.name.crc))
              {
                LDIAG("User already in session")
                Packet::Create(ServerResponse::SessionBadUser).Send(user.socket);
              }
              // Would this user cause us to go over the maximum count
              else if (session->users.GetCount() == session->maxUsers)
              {
                LDIAG("Session full")
                Packet::Create(ServerResponse::SessionFull).Send(user.socket);
              }
              else
              {
                LDIAG("User " << user.GetName() << " joined")

                Packet::Create(ServerResponse::SessionJoined).Send(user.socket);

                // Add the user
                session->AddUser(user);
              }
            }
            else
            {
              LDIAG("Bad password")
              Packet::Create(ServerResponse::SessionBadPassword).Send(user.socket);
            }
          }
          else
          {
            LDIAG("Session not found")
            Packet::Create(ServerResponse::SessionNotFound).Send(user.socket);
          }

          mutexSessions.Signal();
        }
        else
        {
          return (FALSE);
        }
        break;
      }


      //
      // Client wants to lock the session they are in
      //
      case ClientMessage::SessionLock:
        if (user.session)
        {
          // Is this user the host of this session ?
          if (user.session->host == &user)
          {
            LDIAG("Locking session " << user.session->name)
            user.session->flags |= SessionFlags::Locked;
            user.session->SendInfo();
          }
          else
          {
            LDIAG("User isn't host")
            Packet::Create(ServerResponse::SessionHostOnly).Send(user.socket);
          }
        }
        else
        {
          LDIAG("Locking: User Not In Session")

          // Can't send a lock a session if you're not in one
          Packet::Create(ServerResponse::UserNotInSession).Send(user.socket);
        }
        break;


      //
      // Client wants to unlock the session they are in
      //
      case ClientMessage::SessionUnlock:
        if (user.session)
        {
          // Is this user the host of this session ?
          if (user.session->host == &user)
          {
            LDIAG("Unlocking session " << user.session->name)
            user.session->flags &= ~SessionFlags::Locked;
            user.session->SendInfo();
          }
          else
          {
            LDIAG("User isn't host")
            Packet::Create(ServerResponse::SessionHostOnly).Send(user.socket);
          }
        }
        else
        {
          LDIAG("Unlocking: User Not In Session")

          // Can't send a unlock a session if you're not in one
          Packet::Create(ServerResponse::UserNotInSession).Send(user.socket);
        }
        break;


      //
      // Host wants to kick and unrully user
      //
      case ClientMessage::SessionKick:

        if (user.session)
        {
          // Is this user the host of this session ?
          if (user.session->host == &user)
          {
            // Resolve the user
            ClientMessage::Data::SessionKick *data;
            if (packet.GetData(data))
            {
              if (User *kickUser = user.session->users.Find(data->user))
              {
                // Prevent people from kicking themselves
                if (kickUser != &user)
                {
                  // Tell the user they are kicked
                  Packet::Create(ServerMessage::SessionKicked).Send(kickUser->socket);

                  // Remove the from the session
                  user.session->RemoveUser(*kickUser);
                }
              }
            }
            else
            {
              return (FALSE);
            }
          }
          else
          {
            LDIAG("User isn't host")
            Packet::Create(ServerResponse::SessionHostOnly).Send(user.socket);
          }
        }
        else
        {
          LDIAG("Kick: User Not In Session")

          // Can't send a unlock a session if you're not in one
          Packet::Create(ServerResponse::UserNotInSession).Send(user.socket);
        }
        break;


      //
      // Client want to migrate the session to someone else
      //
      case ClientMessage::SessionMigrate:

        if (user.session)
        {
          // Is this use the host of this session ?
          if (user.session->host == &user)
          {
            // Migrate the session
            user.session->Migrate();
          }
          else
          {
            LDIAG("User isn't host")
            Packet::Create(ServerResponse::SessionHostOnly).Send(user.socket);
          }
        }
        else
        {
          LDIAG("ChangeHost: User Not In Session")

          // Can't change host for a session if you're no in one
          Packet::Create(ServerResponse::UserNotInSession).Send(user.socket);
        }
        break;


      //
      // Client want to migrate the session to someone else
      //
      case ClientMessage::SessionMigrated:

        if (user.session)
        {
          // Is this use the host of this session ?
          if (user.session->host == &user)
          {
            // Migrate the session
            user.session->flags &= ~SessionFlags::MigratingTo;
          }
          else
          {
            LDIAG("User isn't host")
            Packet::Create(ServerResponse::SessionHostOnly).Send(user.socket);
          }
        }
        else
        {
          LDIAG("SessionMigrated: User Not In Session")

          // Can't change host for a session if you're no in one
          Packet::Create(ServerResponse::UserNotInSession).Send(user.socket);
        }
        break;


      //
      // Client wants to send an asynchornous message to all other clients in the session
      //
      case ClientMessage::SessionData:

        // A message to all users inside the session
        if (user.session)
        {
          CAST(const ClientMessage::Data::SessionData *, data, packet.GetData())

          U32 length = packet.GetLength() - sizeof (ClientMessage::Data::SessionData);

          // Reflect this to all of the users in the session 
          Packet &message = Packet::Create
          (
            ServerMessage::SessionData, 
            sizeof (ServerMessage::Data::SessionData) + length
          );

          CAST(ServerMessage::Data::SessionData *, sessionData, message.GetData())

          // Set who the message is from
          sessionData->from = user.name.crc;
          sessionData->key = data->key;

          if (length)
          {
            // Copy the message over
            Utils::Memcpy(sessionData->data, data->data, length);
          }

          NBinTree<User, CRC>::Iterator u(&user.session->users);
          for (!u; *u; ++u)
          {
            message.Send((*u)->socket, FALSE);
          }

          message.Destroy();

          // Is this data we're interested in ?
          switch (data->key)
          {
            case Std::UserPing:
            {
              CAST(const Std::Data::UserPing *, userPing, data->data)
              user.connection.ProcessPing(userPing->ping, userPing->hops);
              break;
            }
          }
        }
        else
        {
          LDIAG("SessionData: User Not In Session")

          // Can't send a message to a session if you're not in one
          Packet::Create(ServerResponse::UserNotInSession).Send(user.socket);
        }
        break;


      //
      // Client wants to send an asynchronous message to a set of recipients
      //
      case ClientMessage::SessionPrivateData:

        // A message to a list of recipients
        if (user.session)
        {
          // Reflect this to all of the users listed as recipients
          CAST(const ClientMessage::Data::SessionPrivateData *, privateData, packet.GetData())

          U32 headerSize = sizeof (ClientMessage::Data::SessionPrivateData) +
            privateData->numRecipients * sizeof (CRC);
          U32 length = packet.GetLength() - headerSize;

          Packet &message = Packet::Create(
            ServerMessage::SessionPrivateData, 
            sizeof (ServerMessage::Data::SessionPrivateData) + length);

          CAST(ServerMessage::Data::SessionPrivateData *, sessionPrivateData, message.GetData())

          // Set who the message is from
          sessionPrivateData->from = user.name.crc;
          sessionPrivateData->key = privateData->key;

          if (length)
          {
            // Copy the message over
            Utils::Memcpy(sessionPrivateData->data, packet.GetData() + headerSize, length);
          }
        
          // For each recipient send it to them
          const CRC *r = privateData->recipients;

          if (privateData->numRecipients)
          {
            for (U32 c = 0; c < privateData->numRecipients; c++)
            {
              User *u = user.session->users.Find(*(r++));
              if (u)
              {
                message.Send(u->socket, FALSE);
              }
            }
            message.Destroy();
          }
          else
          {
            if (user.session->host)
            {
              // Send it to the host
              message.Send(user.session->host->socket);
            }
          }
        }
        else
        {
          LDIAG("SessionPrivateData: User Not In Session")

          // Can't send a message to a session if you're not in one
          Packet::Create(ServerResponse::UserNotInSession).Send(user.socket);
        }
        break;


      //
      // Client is sending synchronous session data
      //
      case ClientMessage::SessionSyncData:
      case ClientMessage::SessionStoreData:
      case ClientMessage::SessionClearData:
      case ClientMessage::SessionFlushData:

        // Is this user inside a session ?
        if (user.session)
        {
          // Add this data to the queue which we'll process periodically
          user.session->items.Append(new Item(user.name.crc, Packet::Copy(packet)));
        }
        else
        {
          LDIAG("SessionSync/Store/Clear/FlushData: User Not In Session")

          // Those queries are only valid withing a session
          Packet::Create(ServerResponse::UserNotInSession).Send(user.socket);
        }
        break;

      case ClientResponse::SessionRequestMigrateAccept:
        // Is this user inside a migrating session ?
        if (user.session && user.session->migration)
        {
          LDIAG("Migration Accepted by '" << user.GetName() << "'")

          // Reflect this to all of the users listed as recipients
          CAST(const ClientResponse::Data::SessionRequestMigrateAccept *, sessionRequestMigrateAccept, packet.GetData())
          user.session->migration->RequestAccepted(sessionRequestMigrateAccept->address, sessionRequestMigrateAccept->key);
        }
        break;

      case ClientResponse::SessionRequestMigrateDeny:
        // Is this user inside a migrating session ?
        if (user.session && user.session->migration)
        {
          user.session->migration->RequestDenied();
        }
        break;

      default:
        // Unknown packet command
        LDIAG("Unknown Packet Command " << HEX(packet.GetCommand(), 8) << " from " << user.GetName())
        return (FALSE);
    }
    return (TRUE);
  }
}

