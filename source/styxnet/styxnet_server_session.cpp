///////////////////////////////////////////////////////////////////////////////
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
#include "styxnet_private.h"
#include "styxnet_server_private.h"
#include "styxnet_servermessage.h"
#include "styxnet_clientmessage.h"
#include "styxnet_serverresponse.h"
#include "styxnet_explorer.h"
#include "version.h"


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Server::Session
  //

  //
  // Constructor
  //
  Server::Session::Session(Server &server, User *host, const SessionName &name, CRC password, U32 maxUsers)
  : SessionData
    (
      name, 
      U16
      (
        ((server.flags & ServerFlags::StandAlone) ? SessionFlags::RoutingServer : 0) |
        (password ? SessionFlags::Password : 0)
      ),
      U16(Version::GetBuildNumber()),
      0, 
      Clamp<U32>(minimumSessionUsers, maxUsers, maximumSessionUsers)
    ),
    server(server),
    host(host),
    password(password),
    sequenceNumber(1),
    users(&User::nodeSession),
    items(&Item::nodeSession),
    oldPktsIndex(0),
    migration(NULL)
  {
    Utils::Memset(&oldPkts, 0x00, sizeof (oldPkts));

    server.SendEvent
    (
      EventMessage::SessionAdded, 
      new EventMessage::Data::SessionAdded(name)
    );

    if (host)
    {
      AddUser(*host);
    }
  }


  //
  // Destructor
  //
  Server::Session::~Session()
  {
    // There should only be users if the session is being closed, 
    // if the server is being shutdown then there should be no users
    if (users.GetCount())
    {
      // Tell all of the users that the session is closed
      Packet &pkt = Packet::Create(ServerResponse::SessionClosed);
      SendToAll(pkt);
      pkt.Destroy();

      // Remove all the users
      users.UnlinkAll();
    }

    // Cleanup items
    items.DisposeAll();

    // Flush the data
    data.Flush();

    // Tell the server that the session is removed
    server.SendEvent
    (
      EventMessage::SessionRemoved, 
      new EventMessage::Data::SessionRemoved(name)
    );

    // Destroy any packets in the circular buffer
    for (U32 i = 0; i < maximumOldData; i++)
    {
      if (oldPkts[i])
      {
        oldPkts[i]->Destroy();
      }
    }
  }


  //
  // Add User
  //
  void Server::Session::AddUser(User &user)
  {
    ASSERT(!users.Find(user.name.crc))

    // If the user is in a session, remove the from it
    if (user.session)
    {
      user.session->RemoveUser(user);
      user.session = NULL;
    }

    // Send info about the session to this user
    SendInfo(user.socket);

    // Send this user all of our stored data
    data.SendAll(user.socket, 0x079E8594); // "StyxNet::Server"

    // Tell this user about all of the users already in the session
    NBinTree<User, CRC>::Iterator u(&users);
    for (!u; *u; ++u)
    {
      ServerMessage::Data::SessionUserAdded *sessionUserAdded;
      Packet &pkt = Packet::Create(ServerMessage::SessionUserAdded, sessionUserAdded);
      sessionUserAdded->who = (*u)->name;
      pkt.Send(user.socket);
    }

    // Add this user to the session
    users.Add(user.name.crc, &user);
    ++numUsers;
    user.session = this;

    // Tell all of the users in the session that there's a new user
    ServerMessage::Data::SessionUserAdded *sessionUserAdded;
    Packet &pkt = Packet::Create(ServerMessage::SessionUserAdded, sessionUserAdded);
    sessionUserAdded->who = user.name;
    SendToAll(pkt);
    pkt.Destroy();

    // Send an update
    server.SendEvent
    (
      EventMessage::SessionModified, 
      new EventMessage::Data::SessionModified(name, host ? host->name : "NoHost", numUsers, maxUsers)
    );
  }


  //
  // Remove User
  //
  void Server::Session::RemoveUser(User &user)
  {
    ASSERT(users.Find(user.name.crc) == &user)
    ASSERT(user.session == this)

    // Tell all of the users in the session that a user has left
    ServerMessage::Data::SessionUserRemoved *sessionUserRemoved;
    Packet &pkt = Packet::Create(ServerMessage::SessionUserRemoved, sessionUserRemoved);
    sessionUserRemoved->who = user.name.crc;
    SendToAll(pkt);
    pkt.Destroy();

    users.Unlink(&user);
    --numUsers;
    user.session = NULL;

    // If we're migrating, tell the migration
    if (migration)
    {
      migration->RemoveUser(user);
    }

    // Tell the user that the session has been closed
    Packet::Create(ServerResponse::SessionClosed).Send(user.socket);

    // Send an update
    server.SendEvent
    (
      EventMessage::SessionModified, 
      new EventMessage::Data::SessionModified(name, host ? host->name : "NoHost", users.GetCount(), maxUsers)
    );

    // Is this host leaving ? 
    if (&user == host)
    {
      // If we are a standalone server, get a new host, otherwise
      if (server.flags & ServerFlags::StandAlone)
      {
        host = users.GetFirst();
      }
      else
      {
        host = NULL;
      }
    }
  }


  //
  // Update the session
  //
  Bool Server::Session::Process()
  {
    // If we're migrating to, don't send updates yet
    if (flags & SessionFlags::MigratingTo)
    {
      return (FALSE);
    }

    // Are there any users in this session ?
    if (!numUsers)
    {
      // This session should be terminated
      return (TRUE);
    }

    // If we're migrating don't send more updates
    if (flags & SessionFlags::MigratingFrom)
    {
      return (FALSE);
    }

    // Pull out sync data from the incomming queue 
    // and build an update packet to send to all users

    // LDIAG("Sending SessionData Seq:" << sequenceNumber << " Size:" << size)

    Packet &pkt = Packet::Create(ServerMessage::SessionSyncData, maxSyncDataSize);
    U8 *ptr = pkt.GetData();
    U32 remaining = maxSyncDataSize;

    CAST(ServerMessage::Data::SessionSyncData *, sessionSyncData, ptr);
    sessionSyncData->seq = sequenceNumber++;
    sessionSyncData->interval = server.config.updateInterval;
    ptr += sizeof (ServerMessage::Data::SessionSyncData);
    remaining -= sizeof (ServerMessage::Data::SessionSyncData);

    // Process the queued items by copying them into the update packet and
    // performing any local data operations with the data
    Bool ranOut = FALSE;
    NList<Item>::Iterator i(&items);

    Item *item;
    while ((item = i++) != NULL && !ranOut)
    {
      Packet &p = item->packet;
      Bool fromHost = (host && item->user == host->name.crc) ? TRUE : FALSE;

      // Process this packet
      switch (p.GetCommand())
      {
        case ServerMessage::SessionMigrate:
        {
          CAST(ServerMessage::Data::SessionMigrate *, sessionMigrateData, p.GetData())
          U32 totalLength = sizeof (ServerMessage::Data::SessionSyncDataMigrate);
          if (remaining < totalLength)
          {
            ranOut = TRUE;
            break;
          }

          CAST(ServerMessage::Data::SessionSyncDataMigrate *, d, ptr);
          d->command = EventMessage::SyncMigrate;
          d->address = sessionMigrateData->address;
          d->key = sessionMigrateData->key;

          flags |= SessionFlags::MigratingFrom;

          // Adjust pointer and remaining
          ptr += totalLength;
          remaining -= totalLength;
          break;
        }

        case ClientMessage::SessionSyncData:
        {
          CAST(ClientMessage::Data::SessionSyncData *, sessionSyncData, p.GetData())
          U32 length = p.GetLength() - sizeof (ClientMessage::Data::SessionSyncData);
          U32 totalLength = length + sizeof (ServerMessage::Data::SessionSyncDataData);
          if (remaining < totalLength)
          {
            ranOut = TRUE;
            break;
          }
          CAST(ServerMessage::Data::SessionSyncDataData *, d, ptr);

          d->command = EventMessage::SyncData;
          d->from = item->user;
          d->key = sessionSyncData->key;
          ASSERT(length < U16_MAX)
          d->length = U16(length);

          if (length)
          {
            Utils::Memcpy(d->data, sessionSyncData->data, length);
          }

          ptr += totalLength;
          remaining -= totalLength;
          break;
        }

        case ClientMessage::SessionStoreData:
        {
          CAST(ClientMessage::Data::SessionStoreData *, sessionStoreData, p.GetData())
          U32 length = p.GetLength() - sizeof (ClientMessage::Data::SessionStoreData);
          U32 totalLength = length + sizeof (ServerMessage::Data::SessionSyncDataStoreData);
          if (remaining < totalLength)
          {
            ranOut = TRUE;
            break;
          }

          // Attempt to store into our database
          if (data.Store(sessionStoreData->key, sessionStoreData->index, length, sessionStoreData->data, fromHost))
          {
            CAST(ServerMessage::Data::SessionSyncDataStoreData *, d, ptr);

            d->command = EventMessage::SyncStore;
            d->from = item->user;
            d->key = sessionStoreData->key;
            d->index = sessionStoreData->index;
            ASSERT(length < U16_MAX)
            d->length = U16(length);
            Utils::Memcpy(d->data, sessionStoreData->data, length);

            ptr += totalLength;
            remaining -= totalLength;
          }
          else
          {
            LDIAG("Data " << sessionStoreData->key << ":" << sessionStoreData->index << " [" << length << "] could not be stored by " << item->user)
          }
          break;
        }

        case ClientMessage::SessionClearData:
        {
          CAST(ClientMessage::Data::SessionClearData *, sessionClearData, p.GetData())
          U32 totalLength = sizeof (ServerMessage::Data::SessionSyncDataClearData);
          if (remaining < totalLength)
          {
            ranOut = TRUE;
            break;
          }

          // Attempt to clear into our database
          if (data.Clear(sessionClearData->key, sessionClearData->index, fromHost))
          {
            CAST(ServerMessage::Data::SessionSyncDataClearData *, d, ptr);

            d->command = EventMessage::SyncClear;
            d->from = item->user;
            d->key = sessionClearData->key;
            d->index = sessionClearData->index;

            // Adjust pointer and remaining
            ptr += totalLength;
            remaining -= totalLength;
          }
          break;
        }

        case ClientMessage::SessionFlushData:
        {
          U32 totalLength = sizeof (ServerMessage::Data::SessionSyncDataFlushData);
          if (remaining < totalLength)
          {
            ranOut = TRUE;
            break;
          }

          // Flush all of the data in our data base
          if (fromHost)
          {
            data.Flush();
            CAST(ServerMessage::Data::SessionSyncDataFlushData *, d, ptr);
            d->command = EventMessage::SyncFlush;
            d->from = item->user;

            // Adjust pointer and remaining
            ptr += totalLength;
            remaining -= totalLength;
          }
          break;
        }

        default:
          LWARN("Wasn't expecting command " << p.GetCommand())
      }

      if (!ranOut)
      {
        // We're finished with this item
        items.Dispose(item);
      }
    }

    // Resize the packet based on where ptr is now
    U32 size = ptr - pkt.GetData();
    ASSERT(size <= maxSyncDataSize)
    pkt.Resize(U16(size));

    /*
    if (ranOut)
    {
      LWARN("Ran out of space building data packet: size[" << size << "] remaining[" << remaining << "]")
    }
    */

    // Send this packet to all members of the session
    SendToAll(pkt);

    // Save this pkt into the old packet list
    AddOldPkt(pkt);

    return (FALSE);
  }


  //
  // Migrate
  //
  void Server::Session::Migrate()
  {
    if (server.flags & ServerFlags::StandAlone)
    {
      // Merely change hosts

      // Not implemented
    }
    else
    {
      // We're not standalone, migrate to another client
      LDIAG("Initiating migration sequence for session '" << name << "'")

      // If there's only one user in the session tell them that migration was a success
      if (users.GetCount() <= 1)
      {
        LDIAG("Only one user, no need to migrate")

        User *user = users.GetFirst();
        if (user)
        {
          // Tell the use that no migration was needed
          Packet::Create(ServerResponse::UserMigrateNotNeeded).Send(user->socket);
        }
      }
      else
      {
        LDIAG("Migration Commencing")

        // Make the current host as an old host
        if (host)
        {
          host->flags |= UserFlags::PreviousHost;
        }
        server.migrations.Append(migration = new Migration(*this));
      }
    }
  }


  //
  // Send a packet to all users in the session
  //
  void Server::Session::SendToAll(Packet &pkt)
  {
    // Send this packet to all members of the 
    NBinTree<User, CRC>::Iterator u(&users);
    for (!u; *u; ++u)
    {
      pkt.Send((*u)->socket, FALSE);
    }
  }


  //
  // Send information to everyone
  //
  void Server::Session::SendInfo()
  {
    ServerMessage::Data::SessionInfo *info;
    Packet &pkt = Packet::Create(ServerMessage::SessionInfo, info);

    info->name = name;
    info->flags = flags;
    info->numUsers = numUsers;
    info->maxUsers = maxUsers;

    SendToAll(pkt);
    pkt.Destroy();
  }


  //
  // Send information on the socket to the address about this session
  //
  void Server::Session::SendInfo(Win32::Socket &socket)
  {
    ServerMessage::Data::SessionInfo *info;
    Packet &pkt = Packet::Create(ServerMessage::SessionInfo, info);

    info->name = name;
    info->flags = flags;
    info->numUsers = numUsers;
    info->maxUsers = maxUsers;

    pkt.Send(socket);
  }


  //
  // Send information on the socket to the address about this session
  //
  void Server::Session::SendInfo(Win32::Socket &socket, const Win32::Socket::Address &address)
  {
    SessionData data(*this);
    
    // Send of the information
    socket.Send(address, reinterpret_cast<U8 *>(&data), sizeof (SessionData));
  }


  //
  // Add old packet
  //
  void Server::Session::AddOldPkt(Packet &pkt)
  {
    // Is the buffer full ?
    if (oldPkts[oldPktsIndex])
    {
      // Blow away oldest packet
      oldPkts[oldPktsIndex]->Destroy();
    }

    // Add this packet
    oldPkts[oldPktsIndex] = &pkt;

    // Adjust index
    oldPktsIndex = GetNextIndex(oldPktsIndex);
  }

}
