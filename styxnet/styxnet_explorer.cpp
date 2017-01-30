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
  // Class Explorer
  //


  //
  // Explorer::Explorer
  //
  // Constructor
  //
  Explorer::Explorer()
  : sessions(&Session::nodeExplorer),
    socket(Win32::Socket::UDP)
  {
    // Associate events with this socket
    socket.EventSelect(event, FD_READ);

    // Bind the socket to any local address
    socket.Bind(Win32::Socket::Address(ADDR_ANY, 0));

    // Allow broadcasts on this socket
    Bool option = TRUE;
    socket.SetSockOpt(SOL_SOCKET, SO_BROADCAST, (char *) &option, sizeof (Bool));

    // Start the thread
    thread.Start(ThreadProc, this);
  }


  //
  // Explorer::~Explorer
  //
  // Destructor
  //
  Explorer::~Explorer()
  {
    // Tell the threat to quit
    eventQuit.Signal();

    // Wait for the threat to terminate
    thread.Stop();

    // Delete all sessions
    sessions.DisposeAll();

    // Make sure the socket is closed
    socket.Close();
  }


  //
  // Explorer::Broadcast
  //
  // Explore the internal network
  //
  void Explorer::Broadcast(U16 port)
  {
    Unicast(Win32::Socket::Address(INADDR_BROADCAST, port));
  }


  //
  // Explorer::Unicast
  //
  // Explore a specific address
  //
  void Explorer::Unicast(const Win32::Socket::Address &address)
  {
    U32 data = 0x55378008;
    socket.Send(address, (U8 *) &data, sizeof (U32));
  }


  //
  // Explorer::RemoveSessions
  //
  // Remove sessions older than the given time
  //
  void Explorer::RemoveSessions(U32 lifeTime)
  {
    sessionMutex.Wait();

    // Get the current time
    U32 currentTime = Clock::Time::Ms();

    // Go through the sessions to see if there's any which are too old
    NList<Session>::Iterator s(&sessions); 
    while (Session *session = s++)
    {
      if (currentTime - session->lastUpdateTime > lifeTime)
      {
//        LDIAG(session->address << " " << session->name << " is too old, throwing it away")
//        LDIAG("Current: " << currentTime << " Last: " << session->lastUpdateTime << " LifeTime: " << lifeTime)
        sessions.Dispose(session);
      }
    }
    sessionMutex.Signal();
  }


  //
  // Explorer::CopySessions
  //
  // Get a COPY of the current set of sessions
  //
  void Explorer::CopySessions(NList<Session> &copy)
  {
    // Throw away existing sessions in the copy
    copy.DisposeAll();

    // Make sure that the list node is setup for this list
    copy.SetNodeMember(&Session::nodeExplorer);

    sessionMutex.Wait();

    // Iterate the sessions and copy them over
    for (NList<Session>::Iterator s(&sessions); *s; ++s)
    {
      copy.Append(new Session(**s));
    }

    sessionMutex.Signal();
  }


  //
  // Explorer::ThreadProc
  //
  // Thread procedure
  //
  U32 STDCALL Explorer::ThreadProc(void *context)
  {
    Explorer *explorer = static_cast<Explorer *>(context);
    Bool quit = FALSE;
    Win32::EventIndex::List<2> events;
    events.AddEvent(explorer->eventQuit, explorer);
    events.AddEvent(explorer->event, NULL);

    // Enter processing loop
    while (!quit)
    {
      void *context;
      
      if (events.Wait(context))
      {
        if (context == explorer)
        {
          // Quit event
          quit = TRUE;
        }
        else
        {
          Win32::Socket::NetworkEvents networkEvents;
          explorer->socket.EnumEvents(explorer->event, networkEvents);

          if (networkEvents.GetEvents() & FD_READ)
          {
            // Received a response from a server!
            SessionData data;

            Win32::Socket::Address address;

            if (explorer->socket.Recv(address, reinterpret_cast<U8 *>(&data), sizeof (SessionData)) == sizeof (SessionData))
            {
              #ifndef DEVELOPMENT
              // Hide games with a different build number
              if (data.version == U16(Version::GetBuildNumber()))
              #endif

              {
//              LDIAG("Got a response from:")
//              LDIAG(" Address: " << address)
//              LDIAG(" Name:    " << data.name)
//              LDIAG(" Users:   " << data.numUsers << "/" << data.maxUsers)

                Bool found = FALSE;

                explorer->sessionMutex.Wait();

                // Look through the session list to see 
                // if this session is already in the list
                for (NList<Session>::Iterator s(&explorer->sessions); *s; ++s)
                {
                  if 
                  (
                    (*s)->address == address &&
                    (*s)->name.crc == data.name.crc
                  )
                  {
                    // Update the existing session
                    (*s)->lastUpdateTime = Clock::Time::Ms();
                    (*s)->flags = data.flags;
                    (*s)->version = data.version;
                    (*s)->numUsers = data.numUsers;
                    (*s)->maxUsers = data.maxUsers;
                    found = TRUE;
                    break;
                  }
                }

                if (!found)
                {
                  // This is a new session, create a new entry for it
                  explorer->sessions.Append
                  (
                    new Session
                    (
                      data.name,
                      data.flags,
                      data.version,
                      data.numUsers,
                      data.maxUsers,
                      address,
                      Clock::Time::Ms()
                    )
                  );
                }

                explorer->sessionMutex.Signal();
              }
            }
          }
        }
      }
      else
      {
        // The wait failed ?
      }
    }

    return (TRUE);
  }


  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Explorer::Session
  //
  

  //
  // Explorer::Session::Session
  //
  // Initializing Constructor
  //
  Explorer::Session::Session(const SessionName &name, U16 flags, U16 version, U32 numUsers, U32 maxUsers, const Win32::Socket::Address &address, U32 lastUpdateTime)
  : StyxNet::Session(name, flags, version, numUsers, maxUsers, address),
    lastUpdateTime(lastUpdateTime)
  {
  }


  //
  // Explorer::Session::Session
  //
  // Copy Constructor
  //
  Explorer::Session::Session(const Session &session)
  : StyxNet::Session(session),
    lastUpdateTime(session.lastUpdateTime)
  {
  }

}
