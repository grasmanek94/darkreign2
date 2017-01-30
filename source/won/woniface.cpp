///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Networking WON Stuff
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "defines.h"

#include "win32.h"
#undef _WINSOCKAPI_
#include "woninc.h"

#include "std.h"
#include "woniface.h"
#include "system.h"
#include "queue.h"
#include "utils.h"


///////////////////////////////////////////////////////////////////////////////
//
// Libraries
//

#pragma comment(lib, "wsock32.lib")


//#define ALLOW_BETA_KEYS
//#pragma message("ALLOW BETA KEYS")


///////////////////////////////////////////////////////////////////////////////
//
// Logging
//

#ifdef ALLOW_BETA_KEYS
  #define LOG_WON LOG_DIAG
#else
  #ifdef DEVELOPMENT
    #define LOG_WON LOG_DIAG
  #else
    #define LOG_WON(x) \
    if (logging)       \
    {                  \
      LOG_DIAG(x)      \
    }
  #endif
#endif

///////////////////////////////////////////////////////////////////////////////
//
// NameSpace WonIface
//
namespace WonIface
{

  LOGDEFLOCAL("WON")


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Constants
  //

  // Default server port number
  static const U16 portNumber = 0x6666;

  // Amount of time to wait for a timeout
  static const S32 requestTimeout = -1;

  // Public key
  static const char *publicKey = "library\\won\\kver.pub";

  // Login key
  static const char *loginKey = "library\\won\\login.ks";

  // Community name
  static const char *community = "DarkReign2";

  // Directory of DarkReign2 Servers
  static const CH *dirDarkReign2 = L"/DarkReign2";

  // Directory of Titan Servers
  static const CH *dirTitanServers = L"/TitanServers";

  // Server configuration (for new routing servers)
  static const char *chatConfiguration = "RoutingServDarkReign2Chat";
  
  // Servers
  static const CH *serverAuth = L"AuthServer";
  static const CH *serverRouting = L"TitanRoutingServer";
  static const CH *serverFactory = L"TitanFactoryServer";
  static const CH *serverProfile = L"TitanProfileServer";
  static const CH *serverFirewall = L"TitanFirewallDetector";

  // Game prefix
  static const WONCommon::RawBuffer dataGamePrefix = (U8 *) "DR2";

  // Valid versions
	static const WONCommon::RawBuffer dataValidVersions = (U8 *) "SampleValidVersions";

  // Client counts
  static const WONCommon::RawBuffer dataClientCount = (U8 *) "__RSClientCount";

  // Flags
  static const WONCommon::RawBuffer dataFlags = (U8 *) "__RSRoomFlags";

  // Permanent
  static const WONCommon::RawBuffer dataPermanent = (U8 *) "IsPerm";

  // Product name
  static const char *productName = "DarkReign2";

  // The max users in a lobby to be considered
  static U32 maxLobbyUsers = 50;

  // Logging
  static Bool logging = FALSE;

  // Ignored players
  static BinTree<void> ignoredPlayers;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct ServerArray
  //
  struct ServerArray
  {
    // Number of servers
    U16 num;

    // Array of servers
    WONAPI::IPSocket::Address *servers;

    // Constructor
    ServerArray()
    : num(0),
      servers(NULL)
    {
    }

    // Destructor
    ~ServerArray()
    {
      ASSERT(!num)
    }

    // Clear
    void Clear()
    {
      if (num)
      {
        ASSERT(servers)
        delete servers;
        servers = NULL;
        num = 0;
      }
      else
      {
        ASSERT(!servers)
      }
    }

    // Resize
    void Resize(U16 size)
    {
      Clear();

      if (size)
      {
        num = size;
        servers = new WONAPI::IPSocket::Address[num];
      }
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct ChatServer
  //
  struct ChatServer
  {
    // List node
    NBinTree<ChatServer>::Node node;

    // Name of the chat server
    RoomName name;

    // Address of the chat server
    WONAPI::IPSocket::Address address;

    // Num players
    U32 numPlayers;

    // Password protected
    Bool password;

    // Permanent
    Bool permanent;

    // Constructor
    ChatServer(const CH *name, const WONAPI::IPSocket::Address &address, U32 numPlayers, Bool password, Bool permanent)
    : name(name), 
      address(address),
      numPlayers(numPlayers),
      password(password),
      permanent(permanent)
    {
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct RoomClient
  //
  struct RoomClient
  {
    // Tree node
    NBinTree<RoomClient>::Node node;

    // Name of the client
    PlayerName name;

    // Id of the client
    U32 id;

    // Is this player a moderator
    Bool moderator;

    // Is this player muted 
    Bool muted;

    // Constructor
    RoomClient(const CH *name, U32 id, Bool moderator, Bool muted)
    : name(name), 
      id(id),
      moderator(moderator),
      muted(muted)
    {
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct RoomGame
  //
  struct RoomGame : public Game
  {
    // Tree Node
    NBinTree<RoomGame>::Node treeNode;

    // Constructor
    RoomGame(const GameName &name, const PlayerName &host, U32 size, const U8 *data)
    : Game(name, host, size, data)
    {
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct AbortableContet
  //
  struct AbortableContext : public Debug::Memory::UnCached
  {
    // Abort flag
    System::Event abort;

    // List node
    NList<AbortableContext>::Node node;

    // Constructor
    AbortableContext();

    // Destructor
    ~AbortableContext();

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct CreateRoomContext
  //
  struct CreateRoomContext : public AbortableContext
  {
    // Address
    WONAPI::IPSocket::Address address;

    // Port
    U16 port;

    // Room name
    RoomName room;

    // Password
    PasswordStr password;

    // Constructor
    CreateRoomContext(const CH *room, const CH *password)
    : room(room),
      password(password)
    {
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct ConnectRoomContext
  //
  struct ConnectRoomContext : public AbortableContext
  {
    // Room Name
    RoomName roomName;

    // Password
    PasswordStr password;

    // Routing server we're connecting to
    WONAPI::RoutingServerClient *routingServer;

    // Constructor
    ConnectRoomContext
    (
      const CH *roomName, 
      const CH *password, 
      WONAPI::RoutingServerClient *routingServer
    )
    : roomName(roomName),
      password(password),
      routingServer(routingServer)
    {
    }

    // Destructor
    ~ConnectRoomContext()
    {
      if (routingServer)
      {
        routingServer->Close();
        delete routingServer;
      }
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct RoomClientList
  //
  struct RoomClientList : public NBinTree<RoomClient>
  {
    // Constructor
    RoomClientList() 
    : NBinTree<RoomClient>(&RoomClient::node) 
    {
    }

    // Add client, return ptr if added, NULL if exists
    RoomClient *AddNoDup(const WONMsg::MMsgRoutingGetClientListReply::ClientData &client)
    {
      PlayerName name;
      Utils::Strmcpy(name.str, (const CH *) client.mClientName.c_str(), Min<U32>(client.mClientName.length() / sizeof (CH) + 1, name.GetSize()));

      RoomClient *rc = Find(client.mClientId);

      if (rc)
      {
        // Update the player information
        rc->moderator = client.mIsModerator;
        rc->muted = client.mIsMuted;

        LOG_WON(("Updated RoomClient [%s] mod:%d mute:%d", Utils::Unicode2Ansi(name.str), rc->moderator, rc->muted))
      }
      else
      {
        RoomClient *rc = new RoomClient(name.str, client.mClientId, client.mIsModerator, client.mIsMuted);
        Add(client.mClientId, rc);

#ifdef DEVELOPMENT
        in_addr addr;
        addr.s_addr = client.mIPAddress;

        LOG_WON(("Added RoomClient [%s] ip:%s mod:%d mute:%d", 
          Utils::Unicode2Ansi(name.str), inet_ntoa(addr), rc->moderator, rc->muted))
#else
        LOG_WON(("Added RoomClient [%s] mod:%d mute:%d", 
          Utils::Unicode2Ansi(name.str), rc->moderator, rc->muted))
#endif

        return (rc);
      }
      return (NULL);
    }

    // Remove client, return TRUE if existed (and fill the name buffer), FALSE if not
    Bool Remove(U32 clientId, PlayerName &name)
    {
      RoomClient *c = Find(clientId);

      if (c)
      {
        name = c->name;
        Dispose(c);

        LOG_WON(("Removed RoomClient [%s]", Utils::Unicode2Ansi(name.str)))

        return (TRUE);
      }
      return (FALSE);
    }

    // Find a client by name
    RoomClient * FindByName(PlayerName &name)
    {
      for (Iterator i(this); *i; ++i)
      {
        if ((*i)->name == name)
        {
          return (*i);
        }
      }
      return (NULL);
    }
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct HTTPdata
  //
  struct HTTPdata
  {
    // Handle
    U32 handle;
    
    // Has the file been modified
    bool isNew;
    
    // What is the time on the file
    time_t time;

    // Flag for aborting the transfer
    Bool abort;

    NList<HTTPdata>::Node node;

    HTTPdata(U32 handle)
    : handle(handle),
      isNew(FALSE),
      time(1),
      abort(FALSE)
    {
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // Initialization flag
  static Bool initialized = FALSE;

  // Connected
  static Bool connected = FALSE;

  // Event queue
  static SafeQueue<Event, 256> eventQueue;

  // Firewall status
  static U32 firewallStatus;

  // Critical section for access data which is shared accross threads
  static System::CritSect critData;

  // Critical section for accessing the current routing server
  static System::CritSect critRouting;

  // Our WON identity
  static WONAPI::Identity identity;

  // Routing server we are currently dealing with
  static WONAPI::RoutingServerClient *currentRoutingServer = NULL;

  // Abortable contexts
  static NList<AbortableContext> abortableContexts(&AbortableContext::node);

  // Critsec for modifying the abortable contexts list
  static System::CritSect critAbortable;

  // Directory servers
  static ServerArray directoryServers;

  // Authentication servers
  static ServerArray authServers;

  // Factory servers
  static ServerArray factoryServers;

  // Firewall servers
  static ServerArray firewallServers;

  // Profile servers
  static ServerArray profileServers;

  // Chat servers
  static NBinTree<ChatServer> chatServers(&ChatServer::node);

  // Room clients
  static RoomClientList roomClients;
  static NBinTree<RoomGame> roomGames(&RoomGame::treeNode);

  // Http handle
  static U32 httpHandle;

  // Http data list
  static NList<HTTPdata> httpData(&HTTPdata::node);

  // Http data critical section
  static System::CritSect httpDataCritSect;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Prototypes
  //
  static void Reset();
  static void ConnectRoom(const WONAPI::IPSocket::Address &address, const CH *room, const CH *password, AbortableContext *context);
  static void PostEvent(U32 message, void *data = NULL);
  static const char * DecodeMessage(U32 message);

  static WONAPI::RoutingServerClient * CreateRoutingServer();
  static void UpdateRooms(AbortableContext *context);
  static void JoinLobby(AbortableContext *context);


  static void TitanServerListCallback(const WONAPI::DirEntityListResult &result, AbortableContext *);
  static void CreateAccountCallback(const WONAPI::Identity::AuthResult &result, AbortableContext *);
  static void LoginAccountCallback(const WONAPI::Identity::AuthResult &result, AbortableContext *);
  static void ChangePasswordCallback(const WONAPI::Identity::AuthResult &result, AbortableContext *);
  static void DetectFirewallCallback(const WONAPI::DetectFirewallResult &result, void *);
  static void UpdateRoomsCallback(const WONAPI::DirEntityListResult &result, AbortableContext *);
  static void StartTitanServerCallback(const WONAPI::StartTitanServerResult &result, CreateRoomContext *);
  static void ConnectRoomCallback(WONAPI::Error result, ConnectRoomContext *);
  static void RegisterPlayerCallback(const WONAPI::RoutingServerClient::RegisterClientResult &result, ConnectRoomContext *);
  static void ClientListCallback(const WONAPI::RoutingServerClient::GetClientListResult &result, void *);
  static void HTTPGetCallback(WONAPI::Error result, HTTPdata *);
  static bool HTTPProgressCallback(unsigned long progress, unsigned long size, void* callbackPrivData);

  static void CreateDataObjectCallback(short status, void *);
  static void DeleteDataObjectCallback(short status, void *);
  static void ReplaceDataObjectCallback(short status, void *);
  static void SubscribeDataObjectCallback(const WONAPI::RoutingServerClient::ReadDataObjectResult &result, void *);

  static void GetNumUsersCallback(const WONAPI::RoutingServerClient::GetNumUsersResult &result, void *);

  static void ClientEnterExCatcher(const WONAPI::RoutingServerClient::ClientDataWithReason &data, void *);
  static void ClientLeaveCatcher(const WONAPI::RoutingServerClient::ClientIdWithReason &data, void *);

  static void ClientJoinAttemptCatcher(const WONAPI::RoutingServerClient::ClientDataWithReason &reason, void *);
  static void MuteClientCatcher(const WONAPI::RoutingServerClient::ClientIdWithFlag &reason, void *);
  static void BecomeModeratorCatcher(const WONAPI::RoutingServerClient::ClientIdWithFlag &reason, void *);
  static void HostChangeCatcher(const WONAPI::RoutingServerClient::ClientId client, void *);
  static void DataObjectCreationCatcher(const WONAPI::RoutingServerClient::DataObjectWithLifespan &reason, void *);
  static void DataObjectDeletionCatcher(const WONAPI::RoutingServerClient::DataObject &reason, void *);
  static void DataObjectModificationCatcher(const WONAPI::RoutingServerClient::DataObjectModification &reason, void *);
  static void DataObjectReplacementCatcher(const WONAPI::RoutingServerClient::DataObject &reason, void *);
  static void KeepAliveCatcher(void *);
  static void ASCIIPeerChatCatcher(const WONAPI::RoutingServerClient::ASCIIChatMessage &message, void *);
  static void UnicodePeerChatCatcher(const WONAPI::RoutingServerClient::UnicodeChatMessage &message, void *);
  static void ReconnectFailureCatcher(void *);


  //
  // Init
  //
  void Init()
  {
    ASSERT(!initialized)

    LOG_WON(("Initilizing WON"))

    // Initialized WON
  	WONInitialize();

    LOG_WON(("WON Initialized"))

    // We haven't checked our firewall status
    firewallStatus = Firewall::Unchecked;

    // Reset the HTTP handle
    httpHandle = 1;

    // Set initialization flag
    initialized = TRUE;
	}


  //
  // Done
  //
  void Done()
  {
    ASSERT(initialized)

    LOG_WON(("Shutting Down WON"))

    // Reset
    Reset();

    LOG_WON(("Clearing server lists"))

    // Clear auth servers and firewall server
    authServers.Clear();
    firewallServers.Clear();
    profileServers.Clear();

    LOG_WON(("Terminating WON"))

		// Shutdown WON
		WONTerminate();

    LOG_WON(("Cleaning up abortable contexts"))

    // Delete any remaining abortable contexts (this is slightly evil cause it doesnt 
    // enter the abortableCrit)
    NList<AbortableContext>::Iterator a(&abortableContexts);
    while (AbortableContext *context = a++)
    {
      delete context;
    }

    LOG_WON(("Clearing Directory Servers"))

    // Clear out directory servers
    directoryServers.Clear();

    LOG_WON(("Aborting any remaining HTTP transfers"))

    // Clear out any remaining http transfers
    LOG_WON(("httpDataCritSect.Wait"))
    httpDataCritSect.Wait();
    httpData.DisposeAll();
    LOG_WON(("httpDataCritSect.Signal"))
    httpDataCritSect.Signal();

    LOG_WON(("WON Shut Down"))

    // Clear ignored players
    ignoredPlayers.UnlinkAll();

    // Clear initialized flag
    initialized = FALSE;
  }


  //
  // Set the directory servers to use
  //
  void SetDirectoryServers(const List<char> &servers)
  {
    LOG_WON(("Setting Directory Servers"))

    ASSERT(servers.GetCount() < U16_MAX)
    directoryServers.Resize((U16) servers.GetCount());
    U32 index = 0;

    for (List<char>::Iterator s(&servers); *s; s++)
    {
      LOG_WON(("Added Server %s", *s))
      directoryServers.servers[index++] = WONAPI::IPSocket::Address(*s);
    }
  }


  //
  // Connect
  //
  void Connect()
  {
    // Reset data
    Reset();

    // Data is the valid versions
    WONCommon::DataObjectTypeSet data;
    data.insert(WONCommon::DataObject(dataValidVersions));

    LOG_WON(("Getting Main Server List"))

    LOG_WON(("Calling GetDirectoryEx: %d servers", directoryServers.num))

    // Get the list of default servers
    WONAPI::Error error = WONAPI::GetDirectoryEx
    (
      NULL,                         // Identity* identity
      directoryServers.servers,     // const IPSocket::Address* directoryServers
      directoryServers.num,         // unsigned int numAddrs
      NULL,                         // IPSocket::Address* fromDirServer
      dirTitanServers,              // const WONCommon::WONString& path
      NULL,                         // WONMsg::DirEntityList* result
      WONMsg::GF_DECOMPROOT |       // long flags
      WONMsg::GF_DECOMPRECURSIVE | 
      WONMsg::GF_DECOMPSERVICES | 
      WONMsg::GF_ADDTYPE | 
      WONMsg::GF_SERVADDNAME | 
      WONMsg::GF_SERVADDNETADDR | 
      WONMsg::GF_ADDDOTYPE | 
      WONMsg::GF_ADDDODATA,
      data,                         // const WONCommon::DataObjectTypeSet& dataSet
      0,                            // DirEntityCallback callback
      0,                            // void* callbackPrivData
      requestTimeout,               // long timeout
      TRUE,                         // bool async
      TitanServerListCallback,      // void (*f)(const DirEntityListResult&, privsType)
      new AbortableContext          // privsType privs
    );

    switch (error)
    {
      case WONAPI::Error_Success:
      case WONAPI::Error_Pending:
        break;

      default:
        ERR_FATAL(("GetDirectoryEx: %d %s", error, WONErrorToString(error)))
    }
  }


  //
  // Disconnect
  //
  void Disconnect()
  {
    LOG_WON(("Disconnecting"))
    Abort();
    Reset();
  }


  //
  // Abort what ever we're doing
  //
  void Abort()
  {
    LOG_WON(("Aborting"))

    critAbortable.Wait();

    // Throw the abort flag in all abortable contexts
    for (NList<AbortableContext>::Iterator a(&abortableContexts); *a; ++a)
    {
      (*a)->abort.Signal();
    }

    critAbortable.Signal();
  }


  //
  // Process
  //
  Bool Process(U32 &message, void *&data)
  {
    // Are there any events in the event queue ?
    if (Event *e = eventQueue.RemovePre(0))
    {
      // We got one
      message = e->message;
      data = e->data;
      eventQueue.RemovePost();

      return (TRUE);
    }
    else
    {
      // There were no messages
      return (FALSE);
    }
  }


  //
  // Event queue
  //
  SafeQueue<Event, 256> & GetEventQueue()
  {
    return (eventQueue);
  }


  //
  // Create a Won account
  //
  void CreateAccount(const char *username, const char *password)
  {
    LOG_WON(("Create Identity: Username %s Community %s AuthServers %d", username, community, authServers.num))

    // Build an Identity
    identity = WONAPI::Identity(username, community, password, "", authServers.servers, authServers.num);

    // Ast WON to create the account
    WONAPI::Error error = identity.AuthenticateNewAccountEx
    (
      requestTimeout, 
      TRUE, 
      CreateAccountCallback, 
      new AbortableContext
    );

    // Ensure the operation started
    switch (error)
    {
      case WONAPI::Error_Success:
      case WONAPI::Error_Pending:
      case WONMsg::StatusAuth_InvalidCDKey:
        break;

      default:
        ERR_FATAL(("Identity::AuthenticateNewAccountEx: %d %s", error, WONErrorToString(error)))
    }
  }


  //
  // Login to a Won account
  //
  void LoginAccount(const char *username, const char *password)
  {
    LOG_WON(("Login Identity: Username %s Community %s AuthServers %d", username, community, authServers.num))

    // Build an Identity
    identity = WONAPI::Identity
    (
      username, 
      community, 
      password, 
      "", 
      authServers.servers, 
      authServers.num
    );

    // Ask WON to authenticate the account
    WONAPI::Error error = identity.AuthenticateEx
    (
      FALSE, 
      FALSE, 
      requestTimeout, 
      TRUE, 
      LoginAccountCallback, 
      new AbortableContext
    );

    // Ensure the operatation started
    switch (error)
    {
      case WONAPI::Error_Success:
      case WONAPI::Error_Pending:
      case WONMsg::StatusAuth_InvalidCDKey:
        break;

      default:
        ERR_FATAL(("Identity::AuthenticateEx: %d %s", error, WONErrorToString(error)))
    }
  }


  //
  // Change the password of an existing Won account
  //
  void ChangePassword(const char *username, const char *oldPassword, const char *newPassword)
  {
    LOG_WON(("Change Identity: Username %s Community %s AuthServers %d", username, community, authServers.num))

    // Build an Identity
    identity = WONAPI::Identity
    (
      username, 
      community, 
      oldPassword, 
      "", 
      authServers.servers, 
      authServers.num
    );

    // Ask WON to change this accounts password
    WONAPI::Error error = identity.AuthenticateNewPasswordEx
    (
      newPassword,
      requestTimeout,
      TRUE,
      ChangePasswordCallback,
      new AbortableContext
    );

    // Ensure that the operation started
    switch (error)
    {
      case WONAPI::Error_Success:
      case WONAPI::Error_Pending:
      case WONMsg::StatusAuth_InvalidCDKey:
        break;

      case WONAPI::Error_BadNewPassword:
        PostEvent(Error::ChangePasswordBadNewPassword);
        break;

      default:
        ERR_FATAL(("Identity::AuthenticateNewPasswordEx: %d %s", error, WONErrorToString(error)))
    }
  }


  //
  // DetectFirewall
  //
  void DetectFirewall()
  {
    LOG_WON(("Detecting Firewall"))

    // We are now checking for a firewall
    firewallStatus = Firewall::Checking;

    LOG_WON(("DetectFirewallEx: Num Servers %d", firewallServers.num))

    // Detect the presence of the a firewall
    WONAPI::Error error = WONAPI::DetectFirewallEx
    (
      firewallServers.servers, 
      firewallServers.num,
			NULL,
			portNumber,
			requestTimeout,
			TRUE,
      DetectFirewallCallback,
      (void *) NULL
    );

    switch (error)
    {
      case WONAPI::Error_Success:
      case WONAPI::Error_Pending:
        break;

      default:
        ERR_FATAL(("DetectFirewallEx: %d %s", error, WONErrorToString(error)))
    }
  }


  //
  // Are we detected as being behind a firewall
  //
  U32 GetFirewallStatus()
  {
    return (firewallStatus);
  }


  //
  // Keep our Won connection alive
  //
  void KeepAlive()
  {
    LOG_WON(("critRouting.Wait"))
    critRouting.Wait();

    LOG_WON(("Keeping connection alive using GetNumUsersEx"))

    if (currentRoutingServer)
    {
      currentRoutingServer->GetNumUsersEx
      (
        0,                    // unsigned short theTag
        GetNumUsersCallback,  // void (*f)(const GetNumUsersResult&, privsType t)
        (void *) NULL         // privsType t
      );
    }

    LOG_WON(("critRouting.Signal"))
    critRouting.Signal();
  }


  //
  // Request the list of rooms
  //
  void UpdateRooms()
  {
    UpdateRooms(NULL);
  }


  //
  // Get the current list of rooms
  //
  void GetRoomList(NList<Room> &rooms)
  {
    // Wait for data mutex
    LOG_WON(("critData.Wait"))
    critData.Wait();

    // Make sure the room list is clear
    rooms.DisposeAll();

    // Transfer the list of rooms over
    for (NBinTree<ChatServer>::Iterator c(&chatServers); *c; c++)
    {
      rooms.Append(new Room((*c)->name.str, (*c)->numPlayers, (*c)->password, (*c)->permanent));
    }

    // Signal data mutex
    LOG_WON(("critData.Signal"))
    critData.Signal();
  }


  //
  // Create a room
  //
  void CreateRoom(const CH *roomname, const CH *password)
  {
    LOG_WON(("Creating Room: %s", Utils::Unicode2Ansi(roomname)))

    if (!identity.GetLoginName().size())
    {
      ERR_FATAL(("No Identity!"))
    }

    if (factoryServers.num == 0)
    {
      // There are no factory servers

    }
    else
    {
		  wstring commandLine;
		  if (password && password[0] != '\0')
		  {
			  commandLine += L"-Password \"";
        commandLine += password;
        commandLine += L"\" ";
		  }
      commandLine += L"-DirRegDisplayName \"";
      commandLine += roomname;
      commandLine += L"\" ";

      CreateRoomContext *context = new CreateRoomContext(roomname, password);

      WONAPI::Error error = WONAPI::StartTitanServerEx
      (
        &identity,                // Identity* identity
        factoryServers.servers,   // const IPSocket::Address* factories
        factoryServers.num,       // unsigned int numFactories
        &context->address,        // IPSocket::Address* startedOnFactory
        chatConfiguration,        // const string& configurationName
        &context->port,           // unsigned short* startedOnPorts
        NULL,                     // unsigned short* numStartedPorts
        commandLine,              // const WONCommon::WONString& commandLineFragment
        FALSE,                    // bool replaceCommandLine
        1,                        // unsigned char numPortsRequested
        0,                        // unsigned char numSpecificPorts
        NULL,                     // unsigned short* specificPortsArray
        NULL,                     // const IPSocket::Address* authorizedIPs
        0,                        // unsigned short numAuthorizedIPs
        requestTimeout,           // long timeout
        TRUE,                     // bool async
        StartTitanServerCallback, // void (*f)(const StartTitanServerResult&, privsType)
        context                   // privsType t
      );

      switch (error)
      {
        case WONAPI::Error_Success:
        case WONAPI::Error_Pending:
          break;

        default:
          ERR_FATAL(("StartTitanServerEx: %d %s", error, WONErrorToString(error)))
      }
	  }
  }


  //
  // Join a room
  //
  void JoinRoom(const CH *roomname, const CH *password)
  {
    LOG_WON(("Joining Room '%s' '%s'", Utils::Unicode2Ansi(roomname), password ? "password" : ""))

    if (!identity.GetLoginName().size())
    {
      ERR_FATAL(("No Identity!"))
    }

    // Wait for data mutex
    LOG_WON(("critData.Wait"))
    critData.Wait();

    // Attempt to find the room in the chat server tree
    ChatServer *server = chatServers.Find(Crc::CalcStr(roomname));

    // Signal data mutex
    LOG_WON(("critData.Signal"))
    critData.Signal();

    // Verify that we're not currently in that room
    LOG_WON(("currentRoutingServer=0x%.8X", currentRoutingServer))

    if (server)
    {
      ConnectRoom(server->address, roomname, password, NULL);
    }
    else
    {
      LOG_WON(("Room '%s' not found", roomname))
      PostEvent(Error::NoRoom);
    }
  }


  //
  // Get the list of players in the current room
  //
  void GetPlayerList(NList<Player> &players)
  {
    // Wait for data mutex
    LOG_WON(("critData.Wait"))
    critData.Wait();

    // Make sure the room list is clear
    players.DisposeAll();

    // Transfer the list of rooms over
    for (RoomClientList::Iterator c(&roomClients); *c; c++)
    {
      players.Append(new Player((*c)->name.str, (*c)->id, (*c)->moderator, (*c)->muted, ignoredPlayers.Exists((*c)->name.crc)));
    }

    // Signal data mutex
    LOG_WON(("critData.Signal"))
    critData.Signal();
  }


  //
  // Add a game to the list of games
  //
  void AddGame(const GameName &name, U32 size, const U8 *d)
  {
    WONCommon::RawBuffer data;
    data.assign(d, size);

    LOG_WON(("critRouting.Wait"))
    critRouting.Wait();

    if (currentRoutingServer)
    {
      currentRoutingServer->CreateDataObjectEx
      (
        WONAPI::RoutingServerClient::GROUPID_ALLUSERS,  // ClientOrGroupId theLinkId
        dataGamePrefix + (U8 *) name.str,               // const WONCommon::RawBuffer& theDataTypeR
        currentRoutingServer->GetClientId(),            // ClientOrGroupId theOwnerId
        0,                                              // unsigned short theLifespan
        data,                                           // const WONCommon::RawBuffer& theDataR
        CreateDataObjectCallback,                       // void (*f)(short, privsType)
        (void *) NULL                                   // privsType t)
      );
    }

    LOG_WON(("critRouting.Signal"))
    critRouting.Signal();
  }


  //
  // Update a game
  //
  void UpdateGame(const GameName &name, U32 size, const U8 *d)
  {
    WONCommon::RawBuffer data;
    data.assign(d, size);

    LOG_WON(("critRouting.Wait"))
    critRouting.Wait();

    if (currentRoutingServer)
    {
      currentRoutingServer->ReplaceDataObjectEx
      (
        WONAPI::RoutingServerClient::GROUPID_ALLUSERS,  // ClientOrGroupId theLinkId
        dataGamePrefix + (U8 *) name.str,               // const WONCommon::RawBuffer& theDataTypeR
        data,                                           // const WONCommon::RawBuffer& theDataR
        ReplaceDataObjectCallback,                      // void (*f)(short, privsType)
        (void *) NULL                                   // privsType t
      );
    }

    LOG_WON(("critRouting.Signal"))
    critRouting.Signal();
  }


  //
  // Remove a game from the list of games
  //
  void RemoveGame(const GameName &name)
  {
    LOG_WON(("critRouting.Wait"))
    critRouting.Wait();

    if (currentRoutingServer)
    {
      currentRoutingServer->DeleteDataObjectEx
      (
        WONAPI::RoutingServerClient::GROUPID_ALLUSERS,  // ClientOrGroupId theLinkId, 
        dataGamePrefix + (U8 *) name.str,               // const WONCommon::RawBuffer& theDataTypeR,
        DeleteDataObjectCallback,                       // void (*f)(short, privsType), 
        (void *) NULL                                   // privsType t)
      );
    }

    LOG_WON(("critRouting.Signal"))
    critRouting.Signal();
  }


  //
  // Get the list of games in the current room
  //
  void GetGameList(NList<Game> &games)
  {
    // Wait for data mutex
    LOG_WON(("critData.Wait"))
    critData.Wait();

    // Make sure the room list is clear
    games.DisposeAll();

    // Transfer the list of rooms over
    for (NBinTree<RoomGame>::Iterator g(&roomGames); *g; g++)
    {
      games.Append(new Game((*g)->name, (*g)->host, (*g)->size, (*g)->data));
    }

    // Signal data mutex
    LOG_WON(("critData.Signal"))
    critData.Signal();
  }


  //
  // Get the address which is connecting us to the internet
  //
  Bool GetLocalAddress(U32 &ip, U16 &port)
  {
    if (currentRoutingServer)
    {
      WONAPI::WSSocket *socket = currentRoutingServer->GetSocket();

      if (socket)
      {
        SOCKET sock = socket->GetSocket();
        if (sock != INVALID_SOCKET)
        {
          sockaddr_in addr;
          int size = sizeof (sockaddr_in);
          if (getsockname(sock, (sockaddr *) &addr, &size) != SOCKET_ERROR)
          {
            ip = ntohl(addr.sin_addr.s_addr);
            port = ntohs(addr.sin_port);
            return (TRUE);
          }
        }
      }
    }
    return (FALSE);
  }


  //
  // Send broadcast chat message
  //
  void BroadcastMessage(const CH *text)
  {
    LOG_WON(("critRouting.Wait"))
    critRouting.Wait();

    if (currentRoutingServer)
    {
      currentRoutingServer->BroadcastChat(wstring(text), FALSE);
    }

    LOG_WON(("critRouting.Signal"))
    critRouting.Signal();
  }


  //
  // Send emote chat message
  //
  void DLL_DECL EmoteMessage(const CH *text)
  {
    LOG_WON(("critRouting.Wait"))
    critRouting.Wait();

    if (currentRoutingServer)
    {
      wstring message(text);
      currentRoutingServer->BroadcastChat(
        WONAPI::RoutingServerClient::Message
        (
          reinterpret_cast<const unsigned char *>(message.data()), 
          U16(message.size() * sizeof (CH))
        ), 
        WONMsg::CHATTYPE_UNICODE_EMOTE, 
        FALSE);
    }

    LOG_WON(("critRouting.Signal"))
    critRouting.Signal();
  }


  //
  // Send private chat message
  //
  void PrivateMessage(const char *player, const CH *text)
  {
    LOG_WON(("critRouting.Wait"))
    critRouting.Wait();

    if (currentRoutingServer)
    {
      PlayerName name;
      Utils::Ansi2Unicode(name.str, name.GetSize(), player);
      name.Update();

      static CH buff[512];
      Utils::Strcpy(buff, text);

      LOG_WON(("critData.Wait"))
      critData.Wait();

      // Get the Id of the player
      RoomClient *client = roomClients.FindByName(name);

      if (client)
      {
        WONAPI::RoutingServerClient::ClientOrGroupId ids[1];
        ids[0] = U16(client->id);
        wstring message(buff);

    		currentRoutingServer->WhisperChat
        (
          ids,                                  // const ClientOrGroupId theRecipients[]
          1,                                    // unsigned short theNumRecipients
          TRUE,                                 // bool flagIncludeExclude
          WONAPI::RoutingServerClient::Message  // const Message& theMessageR
          (
            reinterpret_cast<const unsigned char *>(message.data()), 
            U16(message.size() * sizeof (CH))
          ), 
          WONMsg::ChatType(CHATTYPE_UNICODE),   // WONMsg::ChatType theChatType
          FALSE                                 // bool shouldSendReply
        );
      }

      LOG_WON(("critData.Signal"))
      critData.Signal();
    }

    LOG_WON(("critRouting.Signal"))
    critRouting.Signal();
  }


  //
  // Ignore a player
  //
  void IgnorePlayer(const char *player)
  {
    PlayerName name;
    Utils::Ansi2Unicode(name.str, name.GetSize(), player);
    name.Update();
    IgnorePlayer(name);
  }

  void IgnorePlayer(const PlayerName &name)
  {
    // Signal data mutex
    LOG_WON(("critData.Wait"))
    critData.Wait();

    // Clear ignored players
    if (!ignoredPlayers.Exists(name.crc))
    {
      ignoredPlayers.Add(name.crc, NULL);
    }

    // Signal data mutex
    LOG_WON(("critData.Signal"))
    critData.Signal();

    PostEvent(Message::PlayersChanged);
  }


  //
  // Unignore a player
  //
  void UnignorePlayer(const char *player)
  {
    PlayerName name;
    Utils::Ansi2Unicode(name.str, name.GetSize(), player);
    name.Update();
    UnignorePlayer(name);
  }

  void UnignorePlayer(const PlayerName &name)
  {
    // Signal data mutex
    LOG_WON(("critData.Wait"))
    critData.Wait();

    // Remove the player from the ignored players
    ignoredPlayers.Unlink(name.crc);

    // Signal data mutex
    LOG_WON(("critData.Signal"))
    critData.Signal();

    PostEvent(Message::PlayersChanged);
  }


  //
  // Check the key in the registry
  //
  Bool CheckStoredKey()
  {
	  WONCDKey::ClientCDKey key(productName);

    key.Load();

#ifdef ALLOW_BETA_KEYS
    if (key.IsValid())
#else
    if (key.IsValid() && !key.IsBeta())
#endif
    {
      // Set up the Authentication API for CD key usage
      WONAPI::Identity::SetCDKey(key);
	    WONAPI::Identity::SetLoginKeyFile(loginKey);
      if (!WONAPI::Identity::LoadVerifierKeyFromFile(publicKey))
      {
        ERR_FATAL(("LoadVerifierKeyFromFile failed"))
      }

      return (TRUE);
    }
    else
    {
      return (FALSE);
    }
  }


  //
  // Check the given key
  //
  Bool CheckKey(const char *text)
  {
	  WONCDKey::ClientCDKey key(productName);
    key.Init(text);

#ifdef ALLOW_BETA_KEYS
    if (key.IsValid())
#else
    if (key.IsValid() && !key.IsBeta())
#endif
	  {
	    key.Save();

      // Set up the Authentication API for CD key usage
      WONAPI::Identity::SetCDKey(key);
	    WONAPI::Identity::SetLoginKeyFile(loginKey);
      if (!WONAPI::Identity::LoadVerifierKeyFromFile(publicKey))
      {
        ERR_FATAL(("LoadVerifierKeyFromFile failed"))
      }

      return (TRUE);
    }
    else
    {
      return (FALSE);
	  }
  }



  //
  // Retrieve a HTTP file and store it locally in the download directory
  //
  //U32 HTTPGet(U32 proxyIP, U16 proxyPort, const char *hostName, U16 hostPort, const char *path, const char *local, Bool allowResume)
  U32 HTTPGet(const char *proxy, const char *hostName, U16 hostPort, const char *path, const char *local, Bool allowResume)
  {
    LOG_WON(("HTTPGet: %s %s %d %s %s %d", proxy, hostName, hostPort, path, local, allowResume))

    HTTPdata *data = new HTTPdata(httpHandle++);

    WONAPI::HTTPGet
    (
      WONAPI::IPSocket::Address(proxy), // const IPSocket::Address& proxyAddr,
      hostName,                         // const std::string& hostName
      hostPort,                         // unsigned short httpPort
      path,                             // const std::string& getPath 
      local,                            // const std::string& saveAsFile
      &data->isNew,                     // bool* isNew
      &data->time,                      // time_t* modTime
      allowResume ? true : false,       // bool allowResume
      HTTPProgressCallback,             // ProgressCallback callback
      data,                             // void* callbackPrivData
      requestTimeout,                   // long timeout
      TRUE,                             // bool async
      HTTPGetCallback,                  // void (*f)(Error, privsType), 
      data                              // privsType privs
    );

    LOG_WON(("httpDataCritSect.Wait"))
    httpDataCritSect.Wait();
    httpData.Append(data);
    LOG_WON(("httpDataCritSect.Signal"))
    httpDataCritSect.Signal();

    LOG_WON(("HTTPGet: handle %d", data->handle))

    return (data->handle);
  }


  //
  // Retrieve a HTTP file and store it locally in the download directory
  //
  U32 DLL_DECL HTTPGet(const char *hostName, U16 hostPort, const char *path, const char *local, Bool allowResume)
  {
    LOG_WON(("HTTPGet: %s %d %s %s %d", hostName, hostPort, path, local, allowResume))

    HTTPdata *data = new HTTPdata(httpHandle++);

    WONAPI::HTTPGet
    (
      hostName,                         // const std::string& hostName
      hostPort,                         // unsigned short httpPort
      path,                             // const std::string& getPath 
      local,                            // const std::string& saveAsFile
      &data->isNew,                     // bool* isNew
      &data->time,                      // time_t* modTime
      allowResume ? true : false,       // bool allowResume
      HTTPProgressCallback,             // ProgressCallback callback
      data,                             // void* callbackPrivData
      requestTimeout,                   // long timeout
      TRUE,                             // bool async
      HTTPGetCallback,                  // void (*f)(Error, privsType), 
      data                              // privsType privs
    );

    LOG_WON(("httpDataCritSect.Wait"))
    httpDataCritSect.Wait();
    httpData.Append(data);
    LOG_WON(("httpDataCritSect.Signal"))
    httpDataCritSect.Signal();

    LOG_WON(("HTTPGet: handle %d", data->handle))

    return (data->handle);
  }


  //
  // Abort a HTTP get
  //
  void HTTPAbortGet(U32 handle)
  {
    LOG_WON(("HTTPGet: aborting %d", handle))

    LOG_WON(("httpDataCritSect.Wait"))
    httpDataCritSect.Wait();

    for (NList<HTTPdata>::Iterator d(&httpData); *d; ++d)
    {
      if ((*d)->handle == handle)
      {
        (*d)->abort = TRUE;
        break;
      }
    }

    LOG_WON(("httpDataCritSect.Signal"))
    httpDataCritSect.Signal();
  }


  //
  // Enable/Disable logging
  //
  void Logging(Bool on)
  {
    logging = on;
  }


  //
  // Reset
  //
  void Reset()
  {
    LOG_WON(("Reset in"))

    LOG_WON(("critRouting.Wait"))
    critRouting.Wait();

    if (currentRoutingServer)
    {
      LOG_WON(("Deleting current routing server"))

      currentRoutingServer->Close();
      delete currentRoutingServer;
      currentRoutingServer = NULL;
    }

    LOG_WON(("critRouting.Signal"))
    critRouting.Signal();

    // Wait for data mutex
    LOG_WON(("critData.Wait"))
    critData.Wait();

    // Cleanup
    factoryServers.Clear();
    chatServers.DisposeAll();
    roomClients.DisposeAll();
    roomGames.DisposeAll();

    // Clear ignored players
    ignoredPlayers.UnlinkAll();

    // Signal data mutex
    LOG_WON(("critData.Signal"))
    critData.Signal();

    LOG_WON(("Reset out"))
  }


  //
  // Connect to a room
  //
  void ConnectRoom(const WONAPI::IPSocket::Address &address, const CH *roomName, const CH *password, AbortableContext *context)
  {
    WONAPI::RoutingServerClient *routingServer = CreateRoutingServer();

    ConnectRoomContext *connectRoomContext = new ConnectRoomContext(roomName, password, routingServer);

    // If there's a context transfer its abort status
    if (context)
    {
      critAbortable.Wait();
      if (context->abort.Test())
      {
        connectRoomContext->abort.Signal();
      }
      critAbortable.Signal();

      delete context;
      context = NULL;
    }

	  // Connect to the new server
    WONAPI::Error error = routingServer->ConnectEx
    (
      address,                // const TCPSocket::Address& theRoutingAddress
      &identity,              // Identity* theIdentityP
      FALSE,                  // bool isReconnectAttempt
      requestTimeout,         // long theTimeout
      TRUE,                   // bool async
      ConnectRoomCallback,    // void (*f)(Error, privsType)
      connectRoomContext      // privsType t
    );         

    switch (error)
    {
      case WONAPI::Error_Success:
      case WONAPI::Error_Pending:
        break;

      default:
        ERR_FATAL(("ConnectEx: %d %s", error, WONErrorToString(error)))
    }
  }


  //
  // PostEvent
  //
  void PostEvent(U32 message, void *data)
  {
    ASSERT(initialized)

    //printf("Posting Event [%08x] %s\n", message, DecodeMessage(message));

    Event *e = eventQueue.AddPre();
    ASSERT(e)
    e->message = message;
    e->data = data;
    eventQueue.AddPost();
  }


  //
  // DecodeMessage
  //
  const char * DecodeMessage(U32 message)
  {
    switch (message)
    {
      case Message::RetrievedServerList:
        return ("Message::RetrievedServerList");

      case Message::FirewallStatus:
        return ("Message::FirewallStatus");

      case Message::LoggedIn:
        return ("Message::LoggedIn");

      case Message::CreatedAccount:
        return ("Message::CreatedAccount");

      case Message::ChangedPassword:
        return ("Message::ChangedPassword");

      case Message::Chat:    
        return ("Message::Chat");

      case Message::InitialRoomUpdate:
        return ("Message::InitialRoomUpdate");

      case Message::RoomsUpdated:
        return ("Message::RoomsUpdated");

      case Message::CreatedRoom:
        return ("Message::CreatedRoom");

      case Message::RegisteredRoom:
        return ("Message::RegisteredRoom");

      case Message::ConnectedRoom:
        return ("Message::ConnectedRoom");

      case Message::EnteredRoom:
        return ("Message::EnteredRoom");

      case Message::PlayersChanged:
        return ("Message::PlayersChanged");

      case Message::GamesChanged:
        return ("Message::GamesChanged");

      case Message::CreatedGame:
        return ("Message::CreatedGame");

      case Message::HTTPProgressUpdate:
        return ("Message::HTTPProgressUpdate");

      case Message::HTTPCompleted:
        return ("Message::HTTPCompleted");


      case Error::ConnectionFailure:
        return ("Error::ConnectionFailure");

      case Error::LoginInvalidUsername:
        return ("Error::LoginInvalidUsername");

      case Error::LoginInvalidPassword:
        return ("Error::LoginInvalidPassword");

      case Error::LoginFailure:
        return ("Error::LoginFailure");

      case Error::KeyExpired:
        return ("Error::KeyExpired");

      case Error::VerifyFailed:
        return ("Error::VerifyFailed");

      case Error::LockedOut:
        return ("Error::LockedOut");

      case Error::KeyInUse:
        return ("Error::KeyInUse");

      case Error::KeyInvalid:
        return ("Error::KeyInvalid");

      case Error::CreateAccountFailure:
        return ("Error::CreateAccountFailure");

      case Error::CreateAccountBadUser:
        return ("Error::CreateAccountBadUser");

      case Error::CreateAccountBadPassword:
        return ("Error::CreateAccountBadPassword");

      case Error::ChangePasswordFailure:
        return ("Error::ChangePasswordFailure");

      case Error::ChangePasswordBadNewPassword:
        return ("Error::ChangePasswordBadNewPassword");

      case Error::NoLobby:
        return ("Error::NoLobby");

      case Error::NoRoom:
        return ("Error::NoRoom");

      case Error::JoinRoomFailure:
        return ("Error::JoinRoomFailure");

      case Error::JoinRoomBadUsername:
        return ("Error::JoinRoomBadUsername");

      case Error::JoinRoomBadPassword:
        return ("Error::JoinRoomBadPassword");

      case Error::JoinRoomFull:
        return ("Error::JoinRoomFull");

      case Error::CreateRoomFailure:
        return ("Error::CreateRoomFailure");

      case Error::CreateGameFailure:
        return ("Error::CreateGameFailure");

      case Error::ReconnectFailure:
        return ("Error::ReconnectFailure");

      case Error::HTTPFailed:
        return ("Error::HTTPFailed");

      default:
        return ("Unknown!");
    }
  }


  //
  // CreateRoutingServer
  //
  WONAPI::RoutingServerClient * CreateRoutingServer()
  {
    WONAPI::RoutingServerClient *routingServer = new WONAPI::RoutingServerClient;

    // Setup catchers for routing server
    routingServer->InstallClientEnterExCatcherEx(ClientEnterExCatcher, (void *) NULL);
    routingServer->InstallClientLeaveCatcherEx(ClientLeaveCatcher, (void *) NULL);

    //routingServer->InstallClientJoinAttemptCatcherEx(ClientJoinAttemptCatcher, (void *) NULL);
    routingServer->InstallMuteClientCatcherEx(MuteClientCatcher, (void *) NULL);
    routingServer->InstallBecomeModeratorCatcherEx(BecomeModeratorCatcher, (void *) NULL);
    //routingServer->InstallHostChangeCatcherEx(HostChangeCatcher, (void *) NULL);
    routingServer->InstallDataObjectCreationCatcherEx(DataObjectCreationCatcher, (void *) NULL);
    routingServer->InstallDataObjectDeletionCatcherEx(DataObjectDeletionCatcher, (void *) NULL);
    //routingServer->InstallDataObjectModificationCatcherEx(DataObjectModificationCatcher, (void *) NULL);
    routingServer->InstallDataObjectReplacementCatcherEx(DataObjectReplacementCatcher, (void *) NULL);
    routingServer->InstallKeepAliveCatcherEx(KeepAliveCatcher, (void *) NULL);
    routingServer->InstallASCIIPeerChatCatcherEx(ASCIIPeerChatCatcher, (void *) NULL);
    routingServer->InstallUnicodePeerChatCatcherEx(UnicodePeerChatCatcher, (void *) NULL);
    routingServer->InstallReconnectFailureCatcherEx(ReconnectFailureCatcher, (void *) NULL);

    // Return the new routing server
    return (routingServer);
  }



  //
  // Request the list of rooms
  //
  void UpdateRooms(AbortableContext *context)
  {
    LOG_WON(("Updating Room List"))

    if (!identity.GetLoginName().size())
    {
      ERR_FATAL(("No Identity!"))
    }

    // Data is the valid versions
    WONCommon::DataObjectTypeSet data;
    data.insert(WONCommon::DataObject(dataClientCount));
    data.insert(WONCommon::DataObject(dataFlags));
    data.insert(WONCommon::DataObject(dataPermanent));

    LOG_WON(("GetDirectoryEx: Num Servers %d", directoryServers.num))

    WONAPI::Error error = WONAPI::GetDirectoryEx
    (
      &identity,                    // Identity* identity
      directoryServers.servers,     // const IPSocket::Address* directoryServers
      directoryServers.num,         // unsigned int numAddrs
	    NULL,                         // IPSocket::Address* fromDirServer
	    dirDarkReign2,                // const WONCommon::WONString& path
	    NULL,                         // WONMsg::DirEntityList* result
	    WONMsg::GF_DECOMPSERVICES |   // long flags
      WONMsg::GF_ADDTYPE | 
      WONMsg::GF_ADDDISPLAYNAME | 
      WONMsg::GF_SERVADDNAME | 
      WONMsg::GF_SERVADDNETADDR |
      WONMsg::GF_ADDDOTYPE | 
      WONMsg::GF_ADDDODATA,
      data,                         // const WONCommon::DataObjectTypeSet& dataSet
			0,                            // DirEntityCallback callback 
      0,                            // void* callbackPrivData
			requestTimeout,               // long timeout
			TRUE,                         // bool async
			UpdateRoomsCallback,          // void (*f)(const DirEntityListResult&, privsType)
      context                       // privsType privs
    );

    switch (error)
    {
      case WONAPI::Error_Success:
      case WONAPI::Error_Pending:
        break;

      default:
        ERR_FATAL(("GetDirectoryEx: %d %s", error, WONErrorToString(error)))
    }
  }


  //
  // Join a lobby
  //
  void JoinLobby(AbortableContext *context)
  {
    ASSERT(context)

    LOG_WON(("Joining a Lobby"))

    if (!identity.GetLoginName().size())
    {
      ERR_FATAL(("No Identity!"))
    }

    ChatServer *lobby = NULL;
    
    // Wait for data mutex
    LOG_WON(("critData.Wait"))
    critData.Wait();

    // Look through the list of rooms for a suitable lobby
    // - the permanent flag
    // - has the most players
    // - if all lobies are over the limit, pick the lobby with the least players

    for (NBinTree<ChatServer>::Iterator c(&chatServers); *c; ++c)
    {
      if ((*c)->permanent)
      {
        if (lobby)
        {
          if ((*c)->numPlayers < maxLobbyUsers)
          {
            if 
            (
              lobby->numPlayers < (*c)->numPlayers ||
              lobby->numPlayers >= maxLobbyUsers
            )
            {
              lobby = *c;
            }
          }
          else
          {
            if 
            (
              lobby->numPlayers >= maxLobbyUsers &&
              lobby->numPlayers > (*c)->numPlayers
            )
            {
              lobby = *c;
            }
          }
        }
        else
        {
          lobby = *c;
        }
      }
    }

    // Signal data mutex
    LOG_WON(("critData.Signal"))
    critData.Signal();

    if (lobby)
    {
      ConnectRoom(lobby->address, lobby->name.str, L"", context);
    }
    else
    {
      PostEvent(Error::NoLobby);
      delete context;
    }
  }


  //
  // TitanServerListCallback
  //
	void TitanServerListCallback(const WONAPI::DirEntityListResult &result, AbortableContext *context)
  { 
    ASSERT(context)

    LOG_WON(("ServerListCallback in"))
    LOG_WON(("GetDirectoryEx CB: %d %s", result.error, WONErrorToString(result.error)))

    if (context->abort.Test())
    {
      LOG_WON(("Was Aborted"))
    }
    else
    {
		  switch (result.error) 
		  {
	  	  case WONAPI::Error_Success:
			  {
          U16 numAuth = 0;
          U16 numFirewall = 0;
          U16 numProfile = 0;

          WONMsg::DirEntityList::const_iterator i;

          // Iterate the server list and count the number of various server types
          for 
          (
            i = result.entityList->begin(); 
            i != result.entityList->end(); 
            ++i
          )
          {
            if (i->mName == serverAuth)
            {
              numAuth++;
            }
            else if (i->mName == serverFirewall)
            {
              numFirewall++;
            }
            else if (i->mName == serverProfile)
            {
              numProfile++;
            }
          }

          // Wait for data mutex
          LOG_WON(("critData.Wait"))
          critData.Wait();

          // Resize our server arrays to accomodate these new sizes
          authServers.Resize(numAuth);
          firewallServers.Resize(numFirewall);
          profileServers.Resize(numProfile);

          // Reset nums so that they can be used as indexes
          numAuth = 0;
          numFirewall = 0;
          numProfile = 0;

          // Fill in the new servers
          for 
          (
            i = result.entityList->begin(); 
            i != result.entityList->end(); 
            ++i
          )
          {
            if (i->mName == serverAuth)
            {
              authServers.servers[numAuth] = WONAPI::IPSocket::Address(*(unsigned long*)(i->mNetAddress.data() + 2), ntohs(*(unsigned short*)i->mNetAddress.data()));
              LOG_WON(("Authentication Server: %s", authServers.servers[numAuth].GetAddressString(TRUE).c_str()))
              numAuth++;
            }
            else if (i->mName == serverFirewall)
            {
              firewallServers.servers[numFirewall] = WONAPI::IPSocket::Address(*(unsigned long*)(i->mNetAddress.data() + 2), ntohs(*(unsigned short*)i->mNetAddress.data()));
              LOG_WON(("Firewall Server: %s", firewallServers.servers[numFirewall].GetAddressString(TRUE).c_str()))
              numFirewall++;
            }
            else if (i->mName == serverProfile)
            {
              profileServers.servers[numProfile] = WONAPI::IPSocket::Address(*(unsigned long*)(i->mNetAddress.data() + 2), ntohs(*(unsigned short*)i->mNetAddress.data()));
              LOG_WON(("Profile Server: %s", profileServers.servers[numProfile].GetAddressString(TRUE).c_str()))
              numProfile++;
            }
          }

          // Signal data mutex
          LOG_WON(("critData.Signal"))
          critData.Signal();

          // If there are firewall servers detect to see if we're behind a firewall
          if (firewallServers.num && firewallStatus == Firewall::Unchecked)
          {
            // Detect firewall
            DetectFirewall();
          }

          // Are there any authentication servers
          if (authServers.num)
          {
            // We are now connected
            connected = TRUE;
            PostEvent(Message::RetrievedServerList);
          }
          else
          {
            // No auth servers could be found
            PostEvent(Error::ConnectionFailure);
          }
  	      break;
			  }

  		  case WONAPI::Error_Timeout:
	  	  case WONMsg::StatusDir_DirNotFound:
          PostEvent(Error::ConnectionFailure);
  			  break;

		    default:
          LOG_ERR(("GetDirectoryEx CB: %d %s", result.error, WONErrorToString(result.error)))
          PostEvent(Error::ConnectionFailure);
  			  break;
		  }
    }

    delete context;

    LOG_WON(("ServerListCallback out"))
	}


  //
  // CreateAccountCallback
  //
  void CreateAccountCallback(const WONAPI::Identity::AuthResult &result, AbortableContext *context)
  {
    LOG_WON(("CreateAccountCallback in"))
    LOG_WON(("AuthenticateNewAccountEx CB: %d %s", result.error, WONErrorToString(result.error)))

    if (context->abort.Test())
    {
      LOG_WON(("Was Aborted"))
    }
    else
    {
      switch (result.error)
      {
        case WONAPI::Error_Success:
          PostEvent(Message::CreatedAccount);
          break;

        case WONAPI::Error_UserExists:
          PostEvent(Error::CreateAccountBadUser);
          break;

        case WONMsg::StatusAuth_ExpiredKey:
          PostEvent(Error::KeyExpired);
          break;

        case WONMsg::StatusAuth_VerifyFailed:
          PostEvent(Error::VerifyFailed);
          break;

        case WONMsg::StatusAuth_LockedOut:
          PostEvent(Error::LockedOut);
          break;

        case WONMsg::StatusAuth_KeyInUse:
          PostEvent(Error::KeyInUse);
          break;

        case WONMsg::StatusAuth_InvalidCDKey:
          PostEvent(Error::KeyInvalid);
          break;

        default:
          LOG_ERR(("AuthenticateNewAccountEx CB: %d %s", result.error, WONErrorToString(result.error)))
          PostEvent(Error::CreateAccountFailure);
          break;
      }
    }

    delete context;

    LOG_WON(("CreateAccountCallback out"))
  }


  //
  // LoginAccountCallback
  //
  void LoginAccountCallback(const WONAPI::Identity::AuthResult &result, AbortableContext *context)
  {
    ASSERT(context)

    LOG_WON(("LoginAccountCallback in"))
    LOG_WON(("AuthenticateEx CB: %d %s", result.error, WONErrorToString(result.error)))

    if (context->abort.Test())
    {
      LOG_WON(("Was Aborted"))
    }
    else
    {
      switch (result.error)
      {
        case WONAPI::Error_Success:
          PostEvent(Message::LoggedIn);

          // Start a room update
          UpdateRooms(context);

          // Context has been passed to update rooms
          context = NULL;
          break;

        case WONMsg::StatusAuth_UserNotFound:	
          PostEvent(Error::LoginInvalidUsername);
          break;

        case WONMsg::StatusAuth_BadPassword:	
          PostEvent(Error::LoginInvalidPassword);
          break;

        case WONMsg::StatusAuth_ExpiredKey:
          PostEvent(Error::KeyExpired);
          break;

        case WONMsg::StatusAuth_VerifyFailed:
          PostEvent(Error::VerifyFailed);
          break;

        case WONMsg::StatusAuth_LockedOut:
          PostEvent(Error::LockedOut);
          break;

        case WONMsg::StatusAuth_KeyInUse:
          PostEvent(Error::KeyInUse);
          break;

        case WONMsg::StatusAuth_InvalidCDKey:
          PostEvent(Error::KeyInvalid);
          break;

        default:
          LOG_ERR(("AuthenticateEx CB: %d %s", result.error, WONErrorToString(result.error)))
          PostEvent(Error::LoginFailure);
          break;
      }
    }

    if (context)
    {
      delete context;
    }

    LOG_WON(("LoginAccountCallback out"))
  }


  //
  // ChangePasswordCallback
  //
  void ChangePasswordCallback(const WONAPI::Identity::AuthResult &result, AbortableContext *context)
  {
    LOG_WON(("ChangePasswordCallback in"))
    LOG_WON(("AuthenticateNewPasswordEx CB: %d %s", result.error, WONErrorToString(result.error)))

    if (context->abort.Test())
    {
      LOG_WON(("Was Aborted"))
    }
    else
    {
      switch (result.error)
      {
        case WONAPI::Error_Success:
          PostEvent(Message::ChangedPassword);
          break;

        default:
          LOG_ERR(("AuthenticateNewPasswordEx CB: %d %s", result.error, WONErrorToString(result.error)))
          PostEvent(Error::ChangePasswordFailure);
          break;
      }
    }

    delete context;

    LOG_WON(("ChangePasswordCallback out"))
  }


  //
  // DetectFirewallCallback
  //
	void DetectFirewallCallback(const WONAPI::DetectFirewallResult &result, void *)
  {
    LOG_WON(("DetectFirewallCallback in"))
    LOG_WON(("DetectFirewallEx CB: %d %s", result.error, WONErrorToString(result.error)))

    switch (result.error)
    {
      case WONAPI::Error_Success:
        firewallStatus = *(result.behindFirewall) ? Firewall::Behind : Firewall::None;
        PostEvent(Message::FirewallStatus);
        break;

      default:
        LOG_ERR(("DetectFirewallEx CB: %d %s", result.error, WONErrorToString(result.error)))
        firewallStatus = Firewall::Unchecked;
        break;
    }

    LOG_WON(("DetectFirewallCallback out"))
  }


  //
  // UpdateRoomCallback
  //
  void UpdateRoomsCallback(const WONAPI::DirEntityListResult &result, AbortableContext *context)
  {
    LOG_WON(("UpdateRoomsCallback in"))
    LOG_WON(("DirectoryEx CB: %d %s", result.error, WONErrorToString(result.error)))

    if (context && context->abort.Test())
    {
      LOG_WON(("Was Aborted"))
    }
    else
    {
	    switch(result.error)
	    {
		    case WONAPI::Error_Success:
		    {
          U16 numFactory = 0;

          WONMsg::DirEntityList::const_iterator i;

          // Iterate the server list and count the number of various server types
          for 
          (
            i = result.entityList->begin(); 
            i != result.entityList->end(); 
            ++i
          )
          {
            if (i->mName == serverFactory)
            {
              numFactory++;
            }
          }

          // Enter a data safe zone
          LOG_WON(("critData.Wait"))
          critData.Wait();

          // Resize our server arrays to accomodate these new sizes
          chatServers.DisposeAll();
          factoryServers.Resize(numFactory);

          // Reset nums so that they can be used as indexes
          numFactory = 0;

          // Fill in the new servers
          for 
          (
            i = result.entityList->begin(); 
            i != result.entityList->end(); 
            ++i
          )
          {
            U32 count = 0;
            Bool password = FALSE;
            Bool permanent = FALSE;

            if (i->mName == serverRouting)
            {
              WONCommon::DataObjectTypeSet::const_iterator d;

              for 
              (
                d = i->mDataObjects.begin();
                d != i->mDataObjects.end(); 
                ++d
              )
              {
                if (d->GetDataType() == dataClientCount)
                {
                  count = *(U32 *) d->GetData().c_str();
                }
                else

                if (d->GetDataType() == dataFlags)
                {
							    password = ((*d->GetData().c_str()) & 0x1 != 0);
                }

                else

                if (d->GetDataType() == dataPermanent)
                {
                  permanent = TRUE;
                }
              }

              ChatServer *server = new ChatServer(i->mDisplayName.c_str(), WONAPI::IPSocket::Address(*(unsigned long*)(i->mNetAddress.data() + 2), ntohs(*(unsigned short*)i->mNetAddress.data())), count, password, permanent);

              char buf[128];
              Utils::Unicode2Ansi(buf, 128, server->name.str);

              // LOG_WON(("Routing Server: %s [%s] [%d] [%s]", buf, server->address.GetAddressString(TRUE).c_str(), count, password ? "locked" : "unlocked"))
              chatServers.Add(server->name.crc, server);
            }
            else if (i->mName == serverFactory)
            {
              factoryServers.servers[numFactory] = WONAPI::IPSocket::Address(*(unsigned long*)(i->mNetAddress.data() + 2), ntohs(*(unsigned short*)i->mNetAddress.data()));
              // LOG_WON(("Factory Server: %s", factoryServers.servers[numFactory].GetAddressString(TRUE).c_str()))
              numFactory++;
            }
          }

          // Leaving a data safe zone
          LOG_WON(("critData.Signal"))
          critData.Signal();

          if (context)
          {
            PostEvent(Message::InitialRoomUpdate);
            JoinLobby(context);
            context = NULL;
          }
          else
          {
            PostEvent(Message::RoomsUpdated);
          }

          break;
        }

        default:
          LOG_ERR(("DirectoryEx CB: %d %s", result.error, WONErrorToString(result.error)))
          break;
      }
    }

    if (context)
    {
      delete context;
    }

    LOG_WON(("UpdateRoomsCallback out"))
  }


  //
  // StartTitanServerCallback
  //
  void StartTitanServerCallback(const WONAPI::StartTitanServerResult &result, CreateRoomContext *context)
  {
    ASSERT(context)

    LOG_WON(("StartTitanServerCallback in"))
    LOG_WON(("StartTitanServerEx CB: %d %s", result.error, WONErrorToString(result.error)))

    if (context->abort.Test())
    {
      LOG_WON(("Was Aborted"))
    }
    else
    {
      switch (result.error)
      {
        case WONAPI::Error_Success:
        {
          PostEvent(Message::CreatedRoom);

          context->address.SetPort(context->port);
          LOG_WON(("Started Server: %s", context->address.GetAddressString(TRUE).c_str()))

          // Connect to newly created room
          ConnectRoom(context->address, context->room.str, context->password.str, context);
          context = NULL;
          break;
        }

        default:
          LOG_ERR(("StartTitanServerEx CB: %d %s", result.error, WONErrorToString(result.error)))
          PostEvent(Error::CreateRoomFailure);
          break;
      }
    }

    if (context)
    {
      delete context;
    }

    LOG_WON(("StartTitanServerCallback out"))
  }


  //
  // ConnectRoomCallback
  //
  void ConnectRoomCallback(WONAPI::Error error, ConnectRoomContext *context)
  {
    ASSERT(context)

    LOG_WON(("ConnectRoomCallback in"))
    LOG_WON(("ConnectEx CB: %d %s", error, WONErrorToString(error)))

    if (context->abort.Test())
    {
      LOG_WON(("Was Aborted"))
    }
    else
    {
      switch (error)
      {
        case WONAPI::Error_Success:
        {
          // From our Identity, compose our username
          WONCommon::RawBuffer loginName;
          loginName.assign
          (
            (const U8 *) identity.GetLoginName().GetUnicodeString().data(), 
            (identity.GetLoginName().GetUnicodeString().size() + 1) * 2
          );
  
          context->routingServer->RegisterEx
          (
            loginName,                  // const ClientName& theClientNameR
            std::wstring(
              context->password.str),   // const std::string& thePasswordR
            FALSE,                      // bool becomeHost
            FALSE,                      // bool becomeSpectator
            TRUE,                       // bool joinChat
            RegisterPlayerCallback,     // void (*f)(const RegisterClientResult&, privsType)
            context                     // privsType t
          );                   

          // Context is no longer our problem, problem of the RegisterPlayerCallback
          context = NULL;

          // Notify of successful connection to room
          PostEvent(Message::ConnectedRoom);
          break;
        }

        case WONAPI::Error_HostUnreachable:
          PostEvent(Error::JoinRoomFailure);
          break;

        default:
          LOG_ERR(("ConnectEx CB: %d %s", error, WONErrorToString(error)))
          PostEvent(Error::JoinRoomFailure);
          break;
      }
    }

    delete context;

    LOG_WON(("ConnectRoomCallback out"))
  }


  //
  // RegisterPlayerCallback
  //
  void RegisterPlayerCallback(const WONAPI::RoutingServerClient::RegisterClientResult &result, ConnectRoomContext *context)
  {
    ASSERT(context)

    LOG_WON(("RegisterPlayerCallback in"))
    LOG_WON(("RegisterEx CB: %d %s", result.mStatus, WONErrorToString(result.mStatus)))

    if (context->abort.Test())
    {
      LOG_WON(("Was Aborted"))
    }
    else
    {
      switch (result.mStatus)
      {
        case WONMsg::StatusRouting_ClientAlreadyRegistered:
//        OutputError("Software error: already logged in");

        // fall through
        case WONMsg::StatusCommon_Success:
        {
          // Wait for data mutex
          LOG_WON(("critRouting.Wait"))
          critRouting.Wait();

          // If there is a current server, leave it and delete it
          if (currentRoutingServer)
          {
            currentRoutingServer->Close();
            delete currentRoutingServer;
          }

          // Move the next server to the current server
          currentRoutingServer = context->routingServer;

          LOG_WON(("critRouting.Signal"))
          critRouting.Signal();

          LOG_WON(("critData.Wait"))
          critData.Wait();

          // Clear the next server
          context->routingServer = NULL;

          // We are now in a new room!

          // Remove all players from the list
          roomClients.DisposeAll();

          // Remove all games from the list
          roomGames.DisposeAll();

          // Signal for data mutex
          LOG_WON(("critData.Signal"))
          critData.Signal();

          // Begin player list update
          currentRoutingServer->GetClientListEx
          (
            FALSE,
            TRUE,
            TRUE,
            ClientListCallback,
            (void *) NULL
          );

          // Begin game list updates
          currentRoutingServer->SubscribeDataObjectEx
          (
            WONAPI::RoutingServerClient::GROUPID_ALLUSERS,  // ClientOrGroupId theLinkId
            dataGamePrefix,                                 // const WONCommon::RawBuffer& theDataTypeR
            FALSE,                                          // bool flagExactOrRecursive
            TRUE,                                           // bool flagGroupOrMembers
            SubscribeDataObjectCallback,                    // void (*f)(const ReadDataObjectResult&, privsType)
            (void *) NULL                                   // privsType t
          );
      
          PostEvent(Message::EnteredRoom, new Message::Data::EnteredRoom(context->roomName.str));

          PostEvent(Message::PlayersChanged);
          PostEvent(Message::GamesChanged);
          break;
        }

        case WONMsg::StatusRouting_ClientAlreadyExists:
          PostEvent(Error::JoinRoomBadUsername);
          break;

        case WONMsg::StatusRouting_InvalidPassword:
          PostEvent(Error::JoinRoomBadPassword);
          break;

        case WONMsg::StatusRouting_ServerFull:
          PostEvent(Error::JoinRoomFull);
          break;

        default:
          LOG_ERR(("RegisterEx CB: %d %s", result.mStatus, WONErrorToString(result.mStatus)))
          PostEvent(Error::JoinRoomFailure);
          break;
      }
    }

    if (context)
    {
      delete context;
    }

    LOG_WON(("RegisterPlayerCallback out"))
  }


  //
  // ClientListCallback
  //
  void ClientListCallback(const WONAPI::RoutingServerClient::GetClientListResult &result, void *)
  {
    LOG_WON(("ClientListCallback in"))
    LOG_WON(("GetClientListEx CB: %d %s", result.mStatus, WONErrorToString(result.mStatus)))

    switch (result.mStatus)
    {
      case WONMsg::StatusCommon_Success:
      {
        LOG_WON(("critData.Wait"))
        critData.Wait();
        for (std::list<WONMsg::MMsgRoutingGetClientListReply::ClientData>::const_iterator i = result.mClientList.begin(); i != result.mClientList.end(); i++)
        {
          roomClients.AddNoDup(*i);
        }
        LOG_WON(("critData.Signal"))
        critData.Signal();

        PostEvent(Message::PlayersChanged);
        break;
      }

      default:
        LOG_ERR(("GetClientListEx CB: %d %s", result.mStatus, WONErrorToString(result.mStatus)))
        break;
    }

    LOG_WON(("ClientListCallback out"))
  }


  //
  // HTTPGetCallback
  //
  void HTTPGetCallback(WONAPI::Error result, HTTPdata *data)
  {
    LOG_WON(("HTTPGetCallback in"))

    switch (result)
    {
      case WONMsg::StatusCommon_Success:
		  {
        // The file was downloaded successfully
        PostEvent(Message::HTTPCompleted, new Message::Data::HTTPCompleted(data->handle, data->isNew));
			  break;
		  }

		  default:
      {
        LOG_ERR(("HTTPGetCallback CB: %d %s", result, WONErrorToString(result)))
        PostEvent(Error::HTTPFailed, new Error::Data::HTTPFailed(data->handle));
        break;
      }
    }

    LOG_WON(("httpDataCritSect.Wait"))
    httpDataCritSect.Wait();
    httpData.Dispose(data);
    LOG_WON(("httpDataCritSect.Signal"))
    httpDataCritSect.Signal();

    LOG_WON(("HTTPGetCallback out"))
  }


  //
  // HTTPProgressCallback
  //
  bool HTTPProgressCallback(unsigned long progress, unsigned long size, void* callbackPrivData)
  {
    HTTPdata *data = static_cast<HTTPdata *>(callbackPrivData);

    LOG_WON(("httpDataCritSect.Wait"))
    httpDataCritSect.Wait();
    Bool abort = data->abort;
    LOG_WON(("httpDataCritSect.Signal"))
    httpDataCritSect.Signal();

    // If this updates for every byte we'll probably need to throttle it back a tad
    PostEvent(Message::HTTPProgressUpdate, new Message::Data::HTTPProgressUpdate(data->handle, progress, size));

    if (abort)
    {
      LOG_WON(("HTTPProgressCallback: aborting download"))
    }

    return (abort ? FALSE : TRUE);
  }


  //
  // CreateDataObjectCallback
  //
  void CreateDataObjectCallback(short status, void *)
  {
    LOG_WON(("CreateDataObjectCallback in"))
    LOG_WON(("CreateDataObjectEx CB: %d %s", status, WONErrorToString(status)))

	  switch(status)
	  {
		  case WONMsg::StatusCommon_Success:
        PostEvent(Message::CreatedGame);
			  break;

		  case WONMsg::StatusRouting_ObjectAlreadyExists:
        PostEvent(Error::CreateGameFailure);
			  break;

		  default:
        LOG_ERR(("CreateDataObjectEx CB: %d %s", status, WONErrorToString(status)))
			  break;
	  }

    LOG_WON(("CreateDataObjectCallback out"))
  }


  //
  // DeleleteDataObjectCallback
  //
  void DeleteDataObjectCallback(short status, void *)
  {
    LOG_WON(("DeleteDataObjectCallback in"))
    LOG_WON(("DeleteDataObjectEx CB: %d %s", status, WONErrorToString(status)))

	  switch (status)
	  {
		  case WONMsg::StatusCommon_Success:
        // We successfully delete the game (woopee do)
        break;

		  default:
        LOG_ERR(("DeleteDataObjectEx CB: %d %s", status, WONErrorToString(status)))
        break;
    }

    LOG_WON(("DeleteDataObjectCallback out"))
  }


  //
  // ReplaceDataObjectCallback
  //
  void ReplaceDataObjectCallback(short status, void *)
  {
    LOG_WON(("ReplaceDataObjectCallback in"))
    LOG_WON(("ReplaceDataObjectEx CB: %d %s", status, WONErrorToString(status)))

    switch (status)
    {
		  case WONMsg::StatusCommon_Success:
        // We successfully replaced the game (woopee do)
        break;

		  default:
        LOG_ERR(("ReplaceDataObjectEx CB: %d %s", status, WONErrorToString(status)))
        break;
    }

    LOG_WON(("ReplaceDataObjectCallback out"))
  }


  //
  // SubscribeDataObjectCallback
  //
  void SubscribeDataObjectCallback(const WONAPI::RoutingServerClient::ReadDataObjectResult &result, void *)
  {
    LOG_WON(("SubscribeDataObjectCallback in"))
    LOG_WON(("SubscribeDataObjectEx CB: %d %s", result.mStatus, WONErrorToString(result.mStatus)))

    switch (result.mStatus)
    {
		  case WONMsg::StatusCommon_Success:
		  {
        LOG_WON(("Subscribed to game list"))

        LOG_WON(("critData.Wait"))
        critData.Wait();

        for 
        (
          std::list<WONAPI::RoutingServerClient::DataObject>::const_iterator i = result.mDataObjectList.begin(); 
          i != result.mDataObjectList.end(); 
          ++i
        )
			  {
          GameName name = reinterpret_cast<const char *>(i->mDataType.data() + dataGamePrefix.size());

          RoomClient *host = roomClients.Find(i->mOwnerId);

          if (host)
          {
            RoomGame *game = new RoomGame
            (
              name,
              host->name,
              i->mData.size(),
              i->mData.data()
            );
            roomGames.Add(name.crc, game);
            LOG_WON(("Game: %s", name.str))

            RoomName gameName;
            Utils::Ansi2Unicode(gameName.str, gameName.GetSize(), name.str);

            PlayerName name = host->name.str;

            LOG_WON(("critData.Signal"))
            critData.Signal();
            PostEvent(Message::Chat, new Message::Data::Chat(Message::Data::Chat::GameCreated, gameName.str, name.str));
            PostEvent(Message::GamesChanged);
            LOG_WON(("critData.Wait"))
            critData.Wait();
          }
			  }

        LOG_WON(("critData.Signal"))
        critData.Signal();

			  break;
		  }

      default:
        LOG_ERR(("SubscribeDataObjectEx CB: %d %s", result.mStatus, WONErrorToString(result.mStatus)))
        break;
    }

    LOG_WON(("SubscribeDataObjectCallback out"))
  }


  //
  // GetNumUsersCallback
  //
  void GetNumUsersCallback(const WONAPI::RoutingServerClient::GetNumUsersResult &, void *)
  {
  }


  //
  // ClientJoinAttemptCatcher
  //
  void ClientJoinAttemptCatcher(const WONAPI::RoutingServerClient::ClientDataWithReason &reason, void *)
  {
    LOG_WON(("ClientJoinAttempt"))
    reason;
  }


  //
  // ClientEnterExCatcher
  //
  void ClientEnterExCatcher(const WONAPI::RoutingServerClient::ClientDataWithReason &data, void *)
  {
    RoomClient *newClient;

    LOG_WON(("critData.Wait"))
    critData.Wait();

    if ((newClient = roomClients.AddNoDup(data)) != NULL)
    {
      LOG_WON(("critData.Signal"))
      critData.Signal();

      // Notify interface of change of players
      PostEvent(Message::PlayersChanged);

      // Print a message with this player's name
      PostEvent(Message::Chat, new Message::Data::Chat(Message::Data::Chat::PlayerEntered, (const CH *)NULL, newClient->name.str));
    }
    else
    {
      LOG_WON(("critData.Signal"))
      critData.Signal();
    }
  }


  //
  // ClientLeaveCatcher
  //
  void ClientLeaveCatcher(const WONAPI::RoutingServerClient::ClientIdWithReason &data, void *)
  {
    PlayerName name;

    LOG_WON(("critData.Wait"))
    critData.Wait();

    if (roomClients.Remove(data.mClientId, name))
    {
      LOG_WON(("critData.Signal"))
      critData.Signal();

      // Notify interface of change of players
      PostEvent(Message::PlayersChanged);

      // Print a message with this player's name
      PostEvent(Message::Chat, new Message::Data::Chat(Message::Data::Chat::PlayerLeft, (const CH *)NULL, name.str));
    }
    else
    {
      LOG_WON(("critData.Signal"))
      critData.Signal();
    }
  }


  //
  // MuteClientCatcher
  //
  void MuteClientCatcher(const WONAPI::RoutingServerClient::ClientIdWithFlag &reason, void *)
  {
    LOG_WON(("MuteClient [%d] %d", reason.mClientId, reason.mFlagOnOrOff))

    LOG_WON(("critData.Wait"))
    critData.Wait();
    RoomClient *rc = roomClients.Find(reason.mClientId);
    if (rc)
    {
      rc->muted = reason.mFlagOnOrOff;
    }
    LOG_WON(("critData.Signal"))
    critData.Signal();

    PostEvent(Message::PlayersChanged);
  }


  //
  // BecomeModeratorCatcher
  //
  void BecomeModeratorCatcher(const WONAPI::RoutingServerClient::ClientIdWithFlag &reason, void *)
  {
    LOG_WON(("BecomeModerator [%d] %d", reason.mClientId, reason.mFlagOnOrOff))

    LOG_WON(("critData.Wait"))
    critData.Wait();
    RoomClient *rc = roomClients.Find(reason.mClientId);
    if (rc)
    {
      rc->moderator = reason.mFlagOnOrOff;
    }
    LOG_WON(("critData.Signal"))
    critData.Signal();

    PostEvent(Message::PlayersChanged);
  }


  //
  // HostChangeCatcher
  //
  void HostChangeCatcher(const WONAPI::RoutingServerClient::ClientId reason, void *)
  {
    LOG_WON(("HostChange"))
    reason;
  }


  //
  // DataObjectCreationCatcher
  //
  void DataObjectCreationCatcher(const WONAPI::RoutingServerClient::DataObjectWithLifespan &reason, void *)
  {
    LOG_WON(("DataObjectCreationCatcher in"))
    GameName name = reinterpret_cast<const char *>(reason.mDataType.data() + dataGamePrefix.size());
    LOG_WON(("Game Created : %s by [%d]", name.str, reason.mOwnerId))

    LOG_WON(("critData.Wait"))
    critData.Wait();

    RoomClient *host = roomClients.Find(reason.mOwnerId);

    if (host)
    {
      RoomName gameName;
      Utils::Ansi2Unicode(gameName.str, gameName.GetSize(), name.str);

      RoomGame *game = new RoomGame
      (
        name.str,
        host->name,
        reason.mData.size(),
        reason.mData.data()
      );
      roomGames.Add(game->name.crc, game);

      PlayerName name = host->name.str;

      LOG_WON(("critData.Signal"))
      critData.Signal();
      PostEvent(Message::Chat, new Message::Data::Chat(Message::Data::Chat::GameCreated, gameName.str, name.str));
      PostEvent(Message::GamesChanged);
      LOG_WON(("critData.Wait"))
      critData.Wait();
    }
    else
    {
      LOG_WON(("Couldn't resolve the host!"))
    }
    LOG_WON(("critData.Signal"))
    critData.Signal();

    LOG_WON(("DataObjectCreationCatcher out"))
  }


  //
  // DataObjectDeletionCatcher
  //
  void DataObjectDeletionCatcher(const WONAPI::RoutingServerClient::DataObject &reason, void *)
  {
    LOG_WON(("DataObjectDeletionCatcher in"))
    GameName name = reinterpret_cast<const char *>(reason.mDataType.data() + dataGamePrefix.size());
    LOG_WON(("Game Deleted : %s by [%d]", name.str, reason.mOwnerId))

    LOG_WON(("critData.Wait"))
    critData.Wait();

    RoomGame *game = roomGames.Find(name.crc);
    if (game)
    {
      roomGames.Dispose(game);

      RoomName gameName;
      Utils::Ansi2Unicode(gameName.str, gameName.GetSize(), name.str);

      LOG_WON(("critData.Signal"))
      critData.Signal();
      PostEvent(Message::Chat, new Message::Data::Chat(Message::Data::Chat::GameDestroyed, gameName.str, (const CH *)NULL));
      PostEvent(Message::GamesChanged);
    }
    else
    {
      LOG_WON(("critData.Signal"))
      critData.Signal();
    }

    LOG_WON(("DataObjectDeletionCatcher out"))
  }


  //
  // DataObjectModificationCatcher
  //
  void DataObjectModificationCatcher(const WONAPI::RoutingServerClient::DataObjectModification &reason, void *)
  {
    LOG_WON(("DataObjectModificationCatcher in"))
    GameName name = reinterpret_cast<const char *>(reason.mDataType.data() + dataGamePrefix.size());
    LOG_WON(("Game Modified : %s", name.str))
    LOG_WON(("DataObjectModificationCatcher out"))
  }


  //
  // DataObjectReplacementCatcher
  //
  void DataObjectReplacementCatcher(const WONAPI::RoutingServerClient::DataObject &reason, void *)
  {
    LOG_WON(("DataObjectReplacementCatcher in"))
    GameName name = reinterpret_cast<const char *>(reason.mDataType.data() + dataGamePrefix.size());
    LOG_WON(("Game Updated : %s by [%d]", name.str, reason.mOwnerId))

    LOG_WON(("critData.Wait"))
    critData.Wait();

    RoomGame *game = roomGames.Find(name.crc);
    if (game)
    {
      if (game->size == reason.mData.size())
      {
        Utils::Memcpy(game->data, reason.mData.data(), game->size);
        LOG_WON(("critData.Signal"))
        critData.Signal();
        PostEvent(Message::GamesChanged);
        LOG_WON(("critData.Wait"))
        critData.Wait();
      }
      else
      {
        LOG_WON(("Data is a different size"))
      }
    }
    else
    {
      LOG_WON(("Could not find game"))
    }

    LOG_WON(("critData.Signal"))
    critData.Signal();

    LOG_WON(("DataObjectReplacementCatcher out"))
  }


  //
  // KeepAliveCatcher
  //
  void KeepAliveCatcher(void *)
  {
    LOG_WON(("KeepAlive"))
  }


  //
  // RawPeerChatCatcher
  //
  void RawPeerChatCatcher(const WONAPI::RoutingServerClient::RawChatMessage &reason, void *)
  {
    LOG_WON(("RawPeerChat"))
    reason;
  }


  //
  // ASCIIPeerChatCatcher
  //
  void ASCIIPeerChatCatcher(const WONAPI::RoutingServerClient::ASCIIChatMessage &message, void *)
  {
    LOG_WON(("critData.Wait"))
    critData.Wait();

    RoomClient *from = roomClients.Find(message.mSenderId);

    if (from)
    {
      // Are we ignoring messages from this player ?
      if (ignoredPlayers.Exists(from->name.crc))
      {
        LOG_WON(("critData.Signal"))
        critData.Signal();
      }
      else
      {
        U32 type = Message::Data::Chat::Broadcast;

        switch (message.mChatType)
        {
          case CHATTYPE_ASCII_EMOTE:
            type = Message::Data::Chat::Emote;
            break;
         
          case CHATTYPE_ASCII:
            type = Message::Data::Chat::Broadcast;
            break;
        }

        if (message.IsWhisper())
        {
          type = Message::Data::Chat::Private;
        }

        PlayerName name = from->name.str;

        LOG_WON(("critData.Signal"))
        critData.Signal();
        PostEvent(Message::Chat, new Message::Data::Chat(type, message.mData.c_str(), name.str));
      }
    }
    else
    {
      LOG_WON(("critData.Signal"))
      critData.Signal();
    }
  }


  //
  // UnicodePeerChatCatcher
  //
  void UnicodePeerChatCatcher(const WONAPI::RoutingServerClient::UnicodeChatMessage &message, void *)
  {
    LOG_WON(("critData.Wait"))
    critData.Wait();

    RoomClient *from = roomClients.Find(message.mSenderId);

    if (from)
    {
      // Are we ignoring messages from this player ?
      if (ignoredPlayers.Exists(from->name.crc))
      {
        LOG_WON(("critData.Signal"))
        critData.Signal();
      }
      else
      {
        U32 type = Message::Data::Chat::Broadcast;

        switch (message.mChatType)
        {
          case CHATTYPE_UNICODE_EMOTE:
            type = Message::Data::Chat::Emote;
            break;
         
          case CHATTYPE_UNICODE:
            type = Message::Data::Chat::Broadcast;
            break;
        }

        if (message.IsWhisper())
        {
          type = Message::Data::Chat::Private;
        }

        PlayerName name = from->name.str;

        LOG_WON(("critData.Signal"))
        critData.Signal();
        PostEvent(Message::Chat, new Message::Data::Chat(type, message.mData.c_str(), name.str));
      }
    }
    else
    {
      LOG_WON(("critData.Signal"))
      critData.Signal();
    }
  }


  //
  // ReconnectFailureCatcher
  //
  void ReconnectFailureCatcher(void *)
  {
    LOG_WON(("ReconnectFailure"))

    LOG_WON(("critRouting.Wait"))
    critRouting.Wait();
    if (currentRoutingServer)
    {
      delete currentRoutingServer;
      currentRoutingServer = NULL;
    }
    LOG_WON(("critRouting.Signal"))
    critRouting.Signal();

    PostEvent(Error::ReconnectFailure);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct AbortableContext
  //

  //
  // Constructor
  //
  AbortableContext::AbortableContext()
  {
    critAbortable.Wait();
    abortableContexts.Append(this);
    critAbortable.Signal();
  }


  //
  // Destructor
  //
  AbortableContext::~AbortableContext()
  {
    critAbortable.Wait();
    abortableContexts.Unlink(this);
    critAbortable.Signal();
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Message
  //
  namespace Message
  {

    ///////////////////////////////////////////////////////////////////////////
    //
    // Struct Chat
    //
    namespace Data
    {

      //
      // Duplicate a string using uncached memory
      //
      static CH *SafeDup(const CH *str, U32 maxLen = 128)
      {
        U32 len = Min<U32>(Utils::Strlen(str), maxLen);
        U32 size = (len + 1) * 2;

        CH *text = reinterpret_cast<CH *>(Debug::Memory::UnCached::Alloc(size));
        Utils::Strmcpy(text, str, len+1);
        text[len] = 0;

        return (text);
      }


      //
      // Duplicate an ANSI string using uncached memory and convert to Unicode
      //
      static CH *SafeDup(const char *str, U32 maxLen = 128)
      {
        U32 len = Min<U32>(Utils::Strlen(str), maxLen);
        U32 size = (len + 1) * 2;

        CH *text = reinterpret_cast<CH *>(Debug::Memory::UnCached::Alloc(size));
        Utils::Ansi2Unicode(text, size, str, len);
        text[len] = 0;

        return (text);
      }


      /////////////////////////////////////////////////////////////////////////
      //
      // Struct Chat
      //

      const U32 MAX_CHAT_STR = 200;

      //
      // Constructor
      //
      Chat::Chat(U32 id, const CH *textIn, const CH *userIn)
      : id(id),
        text(NULL),
        user(NULL)
      {
        if (textIn)
        {
          text = SafeDup(textIn, MAX_CHAT_STR);
        }

        if (userIn)
        {
          user = SafeDup(userIn);
        }
      }


      //
      // Constructor
      //
      Chat::Chat(U32 id, const char *textIn, const CH *userIn)
      : id(id),
        text(NULL),
        user(NULL)
      {
        if (textIn)
        {
          text = SafeDup(textIn, MAX_CHAT_STR);
        }

        if (userIn)
        {
          user = SafeDup(userIn);
        }
      }


      //
      // Destructor
      //
      Chat::~Chat()
      {
        if (text)
        {
          Debug::Memory::UnCached::Free(text);
        }
        if (user)
        {
          Debug::Memory::UnCached::Free(user);
        }
      }


      /////////////////////////////////////////////////////////////////////////
      //
      // Struct EnteredRoom
      //


      //
      // Constructor
      //
      EnteredRoom::EnteredRoom(const CH *text)
      : text(SafeDup(text))
      {
      }


      //
      // Destructor
      //
      EnteredRoom::~EnteredRoom()
      {
        Debug::Memory::UnCached::Free(text);
      }

    }
  }
}
