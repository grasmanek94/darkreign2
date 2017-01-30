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
#include "styxnet_private.h"
#include "styxnet_server_private.h"
#include "styxnet_servermessage.h"


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Server::Migration
  //

  //
  // Constructor
  //
  Server::Migration::Migration(Session &session)
  : session(session),
    users(&User::nodeMigration)
  {
    // 1: Sort the users by suitability
    for (NBinTree<User, CRC>::Iterator u(&session.users); *u; ++u)
    {
      // Is this user an old host ?
      // Could we reverse connect to this user ?
      if 
      (
        ((*u)->flags & UserFlags::AcceptMigration) &&
        !((*u)->flags & UserFlags::PreviousHost)
      )
      {
        U32 score = (*u)->connection.pingSmooth;

        LDIAG("Migration Candidate '" << (*u)->GetName().str << "' Rating [" << score << "]")

        // Use the smooth ping to sort the players
        users.Add(score, *u);
      }
      else
      {
        LDIAG
        (
          "Migration Non-Candidtate '" << (*u)->GetName().str << 
          "' Accept: " << U32((*u)->flags & UserFlags::AcceptMigration) <<
          " Previous: " << U32((*u)->flags & UserFlags::PreviousHost)
        )
      }
    }

    // 2: Offer to the best client
    OfferMigration();
  }


  //
  // Destructor
  //
  Server::Migration::~Migration()
  {
    // Remove users from the tree
    users.UnlinkAll();
  }


  //
  // Offer migration to a user
  //
  void Server::Migration::OfferMigration()
  {
    offer = users.GetFirst();
    if (offer)
    {
      ServerMessage::Data::SessionRequestMigrate *sessionRequestMigrate;
      Packet &pkt = Packet::Create(ServerMessage::SessionRequestMigrate, sessionRequestMigrate);

      LDIAG("Requesting migration to '" << offer->GetName() << "'")

      sessionRequestMigrate->session = session.name.crc;
      sessionRequestMigrate->seq = session.sequenceNumber;
      pkt.Send(offer->socket);

      // We only make a single offer to each user
      users.Unlink(offer);
    }
    else
    {
      LDIAG("No migration candidates")
      session.server.SendEvent(EventMessage::ServerMigrateFailed);

      // Clear migration flag and restart the session
      session.flags &= ~SessionFlags::MigratingFrom;
    }
  }


  //
  // Remove User
  //
  void Server::Migration::RemoveUser(User &user)
  {
    // Is this user in the list of migration candidates?
    if (user.nodeMigration.InUse())
    {
      users.Unlink(&user);
    }

    // Was this the user we were offering to?
    if (offer == &user)
    {
      // Offer to someone else
      OfferMigration();
    }
  }


  //
  // User accepted migration request
  //
  void Server::Migration::RequestAccepted(const Win32::Socket::Address &address, U32 key)
  {
    // A user has accepted migration, tell all of the users to migrate
    ServerMessage::Data::SessionMigrate *sessionMigrate;

    Packet &pkt = Packet::Create(ServerMessage::SessionMigrate, sessionMigrate);
    sessionMigrate->address = address;
    sessionMigrate->key = key;

    LOG_DIAG(("Appending session migrate data to the sync data stream"))

    // We need to tell everyone that the session is migrating though the sync data stream
    session.items.Append(new Item(0, pkt));

    // The session is now considered migrated, we can delete the migration
    offer = NULL;
  }


  //
  // User denied migration request
  //
  void Server::Migration::RequestDenied()
  {
    // The denied us, offer to the next user
    OfferMigration();
  }


  //
  // Process migration, return TRUE to terminate the migration
  //
  Bool Server::Migration::Process()
  {
    if (offer)
    {
      return (FALSE);
    }
    else
    {
      // No user on offer, bail out, kill ourselves
      return (TRUE);
    }
  }

}
