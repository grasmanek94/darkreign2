///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Commands
// 1-JUL-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "won_private.h"
#include "won_cmd.h"
#include "won_controls.h"
#include "woniface.h"
#include "stdload.h"
#include "ptree.h"
#include "filesys.h"
#include "iface.h"
#include "babel.h"
#include "win32_dns.h"
#include "multiplayer_download.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
#define CAST(type, var, value) type var = reinterpret_cast<type>(value);


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Won
//
namespace Won
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct DirectoryServer
  //
  struct DirectoryServer
  {
    NList<DirectoryServer>::Node node;
    GameIdent name;
    U32 port;
    const Win32::DNS::Host *host;
    Bool aborted;

    DirectoryServer(FScope *fScope)
    : host(NULL),
      aborted(FALSE)
    {
      name = StdLoad::TypeString(fScope);
      port = StdLoad::TypeU32(fScope);
    }
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Local variables
  //

  // Initialization flag
  static Bool initialized = FALSE;

  // Won Stream
  static const char *WON_STREAM = "Won";

  // Won Path
  static const char *WON_PATH = "library\\won";

  // Config file name
  static const char *WON_CONFIGFILE = "won.cfg";

  // Directory Servers
  static NList<DirectoryServer> unresolvedServers(&DirectoryServer::node);

  // Directory Servers
  static NList<DirectoryServer> resolvedServers(&DirectoryServer::node);


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Exported data
  //

  // Control to send WON messages to
  IControlPtr wonConnectCtrl;

  // Control which contains player list
  ICListBoxPtr wonPlayersCtrl;

  // Control for setting the title of chat to
  IControlPtr wonChatCtrl;

  // Control for setting the title of games to
  IControlPtr wonGamesCtrl;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Prototypes
  //
  static void DNSCallback(const Win32::DNS::Host *host, void *context);


  //
  // Initialization
  // 
  void Init()
  {
    ASSERT(!initialized)

    // Bring up the WON subsystems
    Cmd::Init();
    Controls::Init();

    // Initialize the WON interface
    WonIface::Init();

    // We are now initalized
    initialized = TRUE;
  }


  //
  // Shutdown
  //
  void Done()
  {
    ASSERT(initialized)

    // Delete the won stream
    FileSys::DeleteStream(WON_STREAM);

    // Shutdown the WON interface
    WonIface::Done();

    // Take down the WON subsystems
    Controls::Done();
    Cmd::Done();

    // Clear the ctrl reapers
    wonConnectCtrl.Clear();
    wonPlayersCtrl.Clear();

    // Clear the directory servers
    unresolvedServers.DisposeAll();
    resolvedServers.DisposeAll();

    initialized = FALSE;
  }


  //
  // Process WON
  //
  void Process()
  {
    // Get any events which have occured and handle them
    U32 message;
    void *data;

    while (WonIface::Process(message, data))
    {
      switch (message)
      {
        case WonIface::Message::HTTPProgressUpdate:
        case WonIface::Message::HTTPCompleted:
        case WonIface::Error::HTTPFailed:
          MultiPlayer::Download::Message(message, data);
          break;

        case WonIface::Message::FirewallStatus:
          // Update our firewall status

          break;

        case WonIface::Message::Chat:
        {
          if (data)
          {
            // Message passed from WON dll
            CAST(WonIface::Message::Data::Chat *, chat, data)

            switch (chat->id)
            {
              case WonIface::Message::Data::Chat::Private:
              {
                if (chat->user && chat->text)
                {
                  CONSOLE(0x975BA2F3, (L"[%s] %s", chat->user, chat->text)) // "WonChatPrivate"
                }
                break;
              }

              case WonIface::Message::Data::Chat::Emote:
              {
                if (chat->user && chat->text)
                {
                  CONSOLE(0xFBEB0583, (L"%s %s", chat->user, chat->text)) // "WonChatQuote"
                }
                break;
              }

              case WonIface::Message::Data::Chat::Broadcast:
              {
                if (chat->user && chat->text)
                {
                  CONSOLE(0x1141C706, (L"[%s] %s", chat->user, chat->text)) // "WonChatMessage"
                }
                break;
              }

              case WonIface::Message::Data::Chat::PlayerEntered:
              {
                if (chat->user)
                {
                  CONSOLE(0x0BA030DB, (L"'%s' entered the room", chat->user)) // "WonMessage"
                }
                break;
              }

              case WonIface::Message::Data::Chat::PlayerLeft:
              {
                if (chat->user)
                {
                  CONSOLE(0x0BA030DB, (L"'%s' left the room", chat->user)) // "WonMessage"
                }
                break;
              }

              case WonIface::Message::Data::Chat::GameCreated:
              {
                if (chat->user && chat->text)
                {
                  CONSOLE(0x0BA030DB, (TRANSLATE(("#won.chat.gamecreated", 2, chat->text, chat->user)) )) // "WonMessage"
                }
                break;
              }

              case WonIface::Message::Data::Chat::GameDestroyed:
              {
                if (chat->text)
                {
                  CONSOLE(0x0BA030DB, (TRANSLATE(("#won.chat.gameended", 1, chat->text)) )) // "WonMessage"
                }
                break;
              }
            }

            delete chat;
          }
          break;
        }

        case WonIface::Message::EnteredRoom:
          if (data)
          {
            CAST(WonIface::Message::Data::EnteredRoom *, enteredRoom, data)

            // Update the name of the room
            if (wonChatCtrl.Alive())
            {
              wonChatCtrl->SetTextString(TRANSLATE(("#won.chat.title", 1, enteredRoom->text)), TRUE);
            }

            // Update the name of the games
            if (wonGamesCtrl.Alive())
            {
              wonGamesCtrl->SetTextString(TRANSLATE(("#won.chat.game.title", 1, enteredRoom->text)), TRUE);
            }
          }
          break;
      }

      // Send the message to the WON window to update the interface
      if (wonConnectCtrl.Alive())
      {
        IFace::SendEvent(wonConnectCtrl, NULL, IFace::NOTIFY, message);
      }
    }
  }


  //
  // Disconnect
  //
  void Disconnect()
  {
    WonIface::Disconnect();
  }


  //
  // Abort
  //
  void Abort()
  {
    Win32::DNS::AbortByNameCallback(DNSCallback);
    unresolvedServers.DisposeAll();
    resolvedServers.DisposeAll();
  }


  //
  // Load config
  //
  void LoadConfig()
  {
    // Load the config file
    PTree pTree;

    // Save the currently active stream
    const char * oldStream = FileSys::GetActiveStream();

    // Setup the preview stream
    FileSys::AddSrcDir(WON_STREAM, WON_PATH);

    // Set the won stream to be the current stream
    FileSys::SetActiveStream(WON_STREAM);

    // Parse the file
    if (pTree.AddFile(WON_CONFIGFILE))
    {
      // Get the global scope
      FScope *gScope = pTree.GetGlobalScope();
      FScope *fScope;

      // Process each function
      while ((fScope = gScope->NextFunction()) != NULL)
      {
        switch (fScope->NameCrc())
        {
          case 0x7F6E34C2: // "DirectoryServers"
          {
            FScope *sScope;
            while ((sScope = fScope->NextFunction()) != NULL)
            {
              switch (sScope->NameCrc())
              {
                case 0x9F1D54D0: // "Add"
                {
                  unresolvedServers.Append(new DirectoryServer(sScope));
                  break;
                }
              }
            }
            break;
          }

          default:
            break;
        }
      }
    }

    Win32::DNS::Host *host;
    NList<DirectoryServer>::Iterator d(&unresolvedServers);
    while (DirectoryServer *unres = d++)
    {
      Win32::DNS::GetByName(unres->name.str, host, DNSCallback, unres);
    }

    // Restore the old active stream
    FileSys::SetActiveStream(oldStream);
  }


  //
  // DNSCallback
  //
  void DNSCallback(const Win32::DNS::Host *host, void *context)
  {
    if (initialized)
    {
      // Context is an unresolved directory server
      DirectoryServer *server = static_cast<DirectoryServer *>(context);

      if (server->aborted)
      {
        unresolvedServers.Dispose(server);
        return;
      }

      if (host && host->GetAddress())
      {
        // This is a resolved server
        unresolvedServers.Unlink(server);
        resolvedServers.Append(server);
        server->host = host;
      }
      else
      {
        // This is a never unresolvable server
        unresolvedServers.Dispose(server);
      }

      // Are there any more unresolved servers ?
      if (!unresolvedServers.GetCount())
      {
        // Do we have any resolved servers ?
        if (resolvedServers.GetCount())
        {
          // Using the list of resolved servers, tell WON what all of the servers are
          List<char> servers;

          for (NList<DirectoryServer>::Iterator s(&resolvedServers); *s; ++s)
          {
            char *buff = new char[256];
            Utils::Sprintf(buff, 256, "%s:%d", (*s)->host->GetAddress()->GetText(), (*s)->port);
            servers.Append(buff);
          }

          // Pass the list of directory servers to the WON interface
          WonIface::SetDirectoryServers(servers);
          servers.DisposeAll();

          if (wonConnectCtrl.Alive())
          {
            IFace::SendEvent(wonConnectCtrl, NULL, IFace::NOTIFY, 0x4D4CE9FA); // "Message::ResolvedDirectoryServers"
          }
        }
        else
        {
          LOG_DIAG(("Could not resolve DNS servers"))
          if (wonConnectCtrl.Alive())
          {
            IFace::SendEvent(wonConnectCtrl, NULL, IFace::NOTIFY, 0x5BFE1EB7); // "Error::CouldNotResolveDirectoryServers"
          }
        }
      }
    }
  }
}
