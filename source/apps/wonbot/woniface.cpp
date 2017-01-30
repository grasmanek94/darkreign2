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


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Room::Internal
  //
  struct Room::Internal
  {
    // Room linkage
    Room &room;

    // Players in this room
    NBinTree<Player> players;

    // Password being used 
    PasswordStr password;

    // Event queue for this room
    SafeQueue<Event, 256> eventQueue;

    // Address of this server
    WONAPI::IPSocket::Address address;

    // Port
    U16 port;

    // Routing server pointer
    WONAPI::RoutingServerClient *routingServer;

    // Critical section for accessing the routing server
    System::CritSect critRouting;


    // Constructor
    Internal(Room &room);

    // Add a player to the room
    void AddPlayer(U32 id, const CH *name, Bool muted, Bool moderator, U32 address);

    // Remove a player from the room
    void RemovePlayer(U32 id);

    // Mute/Unmute a player
    void MutePlayer(U32 id, Bool mute);

    // Moderate/Unmoderate a player
    void ModeratePlayer(U32 id, Bool moderate);


    // Add a game to this server
    void AddGame(U32 ownerId, const GameName &name, U32 size, const U8 *d);

    // Update a game on this server
    void UpdateGame(U32 ownerId, const GameName &name, U32 size, const U8 *d);

    // Remove a game from this server
    void RemoveGame(U32 ownerId, const GameName &name);

  };


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
  // Struct AbortableContet
  //
  struct AbortableContext
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

  // Our WON identity
  static WONAPI::Identity identity;

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

  // Rooms
  static NBinTree<Room> rooms(&Room::node);


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Prototypes
  //
  static void Reset();
  static void PostEvent(U32 message, void *data = NULL);
  static const char * DecodeMessage(U32 message);

  static WONAPI::RoutingServerClient * CreateRoutingServer(Room &room);
  static void UpdateRooms(AbortableContext *context);
  static void JoinLobby(AbortableContext *context);


  static void TitanServerListCallback(const WONAPI::DirEntityListResult &result, AbortableContext *);
  static void CreateAccountCallback(const WONAPI::Identity::AuthResult &result, AbortableContext *);
  static void LoginAccountCallback(const WONAPI::Identity::AuthResult &result, AbortableContext *);
  static void ChangePasswordCallback(const WONAPI::Identity::AuthResult &result, AbortableContext *);
  static void DetectFirewallCallback(const WONAPI::DetectFirewallResult &result, void *);
  static void UpdateRoomsCallback(const WONAPI::DirEntityListResult &result, AbortableContext *);

  static void StartTitanServerCallback(const WONAPI::StartTitanServerResult &result, Room *);
  static void ConnectRoomCallback(WONAPI::Error result, Room *);
  static void RegisterPlayerCallback(const WONAPI::RoutingServerClient::RegisterClientResult &result, Room *);
  static void ClientListCallback(const WONAPI::RoutingServerClient::GetClientListResult &result, Room *);

  static void CreateDataObjectCallback(short status, Room *);
  static void DeleteDataObjectCallback(short status, Room *);
  static void ReplaceDataObjectCallback(short status, Room *);
  static void SubscribeDataObjectCallback(const WONAPI::RoutingServerClient::ReadDataObjectResult &result, Room *);

  static void GetNumUsersCallback(const WONAPI::RoutingServerClient::GetNumUsersResult &result, Room *);

  static void ClientEnterExCatcher(const WONAPI::RoutingServerClient::ClientDataWithReason &data, Room *);
  static void ClientLeaveCatcher(const WONAPI::RoutingServerClient::ClientIdWithReason &data, Room *);
  static void ClientJoinAttemptCatcher(const WONAPI::RoutingServerClient::ClientDataWithReason &reason, Room *);
  static void MuteClientCatcher(const WONAPI::RoutingServerClient::ClientIdWithFlag &reason, Room *);
  static void BecomeModeratorCatcher(const WONAPI::RoutingServerClient::ClientIdWithFlag &reason, Room *);
  static void HostChangeCatcher(const WONAPI::RoutingServerClient::ClientId client, Room *);
  static void DataObjectCreationCatcher(const WONAPI::RoutingServerClient::DataObjectWithLifespan &reason, Room *);
  static void DataObjectDeletionCatcher(const WONAPI::RoutingServerClient::DataObject &reason, Room *);
  static void DataObjectModificationCatcher(const WONAPI::RoutingServerClient::DataObjectModification &reason, Room *);
  static void DataObjectReplacementCatcher(const WONAPI::RoutingServerClient::DataObject &reason, Room *);
  static void KeepAliveCatcher(Room *);
  static void ASCIIPeerChatCatcher(const WONAPI::RoutingServerClient::ASCIIChatMessage &message, Room *);
  static void UnicodePeerChatCatcher(const WONAPI::RoutingServerClient::UnicodeChatMessage &message, Room *);
  static void ReconnectFailureCatcher(Room *);


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

    LOG_WON(("WON Shut Down"))

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
  // Request the list of rooms
  //
  void UpdateRooms()
  {
    UpdateRooms(NULL);
  }


  //
  // Get the current list of rooms
  //
  void GetRoomList(List<Room> &rooms)
  {
    // Wait for data mutex
    LOG_WON(("critData.Wait"))
    critData.Wait();

    // Make sure the room list is clear
    rooms.DisposeAll();

    /*
    // Transfer the list of rooms over
    for (NBinTree<Room>::Iterator c(&rooms); *c; c++)
    {
      rooms.Append(new Room((*c)->name.str, (*c)->numPlayers, (*c)->password, (*c)->permanent));
    }
    */

    // Signal data mutex
    LOG_WON(("critData.Signal"))
    critData.Signal();
  }


  //
  // Create a room
  //
  void CreateRoom(const CH *name, const CH *password)
  {
    LOG_WON(("Creating Room: %s", Utils::Unicode2Ansi(name)))

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
      commandLine += name;
      commandLine += L"\" ";

      Room *room = new Room(name);

      WONAPI::Error error = WONAPI::StartTitanServerEx
      (
        &identity,                // Identity* identity
        factoryServers.servers,   // const IPSocket::Address* factories
        factoryServers.num,       // unsigned int numFactories
        &room->internal.address,  // IPSocket::Address* startedOnFactory
        chatConfiguration,        // const string& configurationName
        &room->internal.port,     // unsigned short* startedOnPorts
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
        room                      // privsType t
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
  void JoinRoom(const CH *name, const CH *password)
  {
    LOG_WON(("Joining Room '%s' '%s'", Utils::Unicode2Ansi(name), password ? "password" : ""))

    if (!identity.GetLoginName().size())
    {
      ERR_FATAL(("No Identity!"))
    }

    // Wait for data mutex
    LOG_WON(("critData.Wait"))
    critData.Wait();

    // Attempt to find the room in the chat server tree
    Room *room = rooms.Find(Crc::CalcStr(name));

    // Copy the password over
    room->internal.password = password;

    // Signal data mutex
    LOG_WON(("critData.Signal"))
    critData.Signal();

    if (room)
    {
      ConnectRoom(*room);
    }
    else
    {
      LOG_WON(("Room '%s' not found", name))
      PostEvent(Error::NoRoom);
    }
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

    // Destroy chat servers
    rooms.DisposeAll();

    // Wait for data mutex
    LOG_WON(("critData.Wait"))
    critData.Wait();

    // Cleanup
    factoryServers.Clear();
    rooms.DisposeAll();

    // Signal data mutex
    LOG_WON(("critData.Signal"))
    critData.Signal();

    LOG_WON(("Reset out"))
  }


  //
  // Connect to a room
  //
  void ConnectRoom(Room &room)
  {
    room.internal.routingServer = CreateRoutingServer(room);

	  // Connect to the new server
    ASSERT(room.internal.routingServer)
    WONAPI::Error error = room.internal.routingServer->ConnectEx
    (
      room.internal.address,  // const TCPSocket::Address& theRoutingAddress
      &identity,              // Identity* theIdentityP
      FALSE,                  // bool isReconnectAttempt
      requestTimeout,         // long theTimeout
      TRUE,                   // bool async
      ConnectRoomCallback,    // void (*f)(Error, privsType)
      &room                   // privsType t
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


      default:
        return ("Unknown!");
    }
  }


  //
  // CreateRoutingServer
  //
  WONAPI::RoutingServerClient * CreateRoutingServer(Room &room)
  {
    WONAPI::RoutingServerClient *routingServer = new WONAPI::RoutingServerClient;

    // Setup catchers for routing server
    routingServer->InstallClientEnterExCatcherEx(ClientEnterExCatcher, &room);
    routingServer->InstallClientLeaveCatcherEx(ClientLeaveCatcher, &room);

    //routingServer->InstallClientJoinAttemptCatcherEx(ClientJoinAttemptCatcher, &room);
    routingServer->InstallMuteClientCatcherEx(MuteClientCatcher, &room);
    routingServer->InstallBecomeModeratorCatcherEx(BecomeModeratorCatcher, &room);
    //routingServer->InstallHostChangeCatcherEx(HostChangeCatcher, &room);
    routingServer->InstallDataObjectCreationCatcherEx(DataObjectCreationCatcher, &room);
    routingServer->InstallDataObjectDeletionCatcherEx(DataObjectDeletionCatcher, &room);
    //routingServer->InstallDataObjectModificationCatcherEx(DataObjectModificationCatcher, &room);
    routingServer->InstallDataObjectReplacementCatcherEx(DataObjectReplacementCatcher, &room);
    routingServer->InstallKeepAliveCatcherEx(KeepAliveCatcher, &room);
    routingServer->InstallASCIIPeerChatCatcherEx(ASCIIPeerChatCatcher, &room);
    routingServer->InstallUnicodePeerChatCatcherEx(UnicodePeerChatCatcher, &room);
    routingServer->InstallReconnectFailureCatcherEx(ReconnectFailureCatcher, &room);

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


  /*
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
  */


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
          factoryServers.Resize(numFactory);

          //
          //chatServers.DisposeAll();

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

              /*
              ChatServer *server = new ChatServer(i->mDisplayName.c_str(), WONAPI::IPSocket::Address(*(unsigned long*)(i->mNetAddress.data() + 2), ntohs(*(unsigned short*)i->mNetAddress.data())), count, password, permanent);

              char buf[128];
              Utils::Unicode2Ansi(buf, 128, server->name.str);

              // LOG_WON(("Routing Server: %s [%s] [%d] [%s]", buf, server->address.GetAddressString(TRUE).c_str(), count, password ? "locked" : "unlocked"))
              chatServers.Add(server->name.crc, server);
              */
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
  void StartTitanServerCallback(const WONAPI::StartTitanServerResult &result, Room *room)
  {
    ASSERT(room)

    LOG_WON(("StartTitanServerCallback in"))
    LOG_WON(("StartTitanServerEx CB: %d %s", result.error, WONErrorToString(result.error)))

    switch (result.error)
    {
      case WONAPI::Error_Success:
      {
        room->PostEvent(Message::CreatedRoom);

        room->internal.address.SetPort(room->internal.port);
        LOG_WON(("Started Server: %s", room->internal.address.GetAddressString(TRUE).c_str()))

        // Connect to newly created room
        ConnectRoom(*room);
        break;
      }

      default:
        LOG_ERR(("StartTitanServerEx CB: %d %s", result.error, WONErrorToString(result.error)))
        room->PostEvent(Error::CreateRoomFailure);
        break;
    }

    LOG_WON(("StartTitanServerCallback out"))
  }


  //
  // ConnectRoomCallback
  //
  void ConnectRoomCallback(WONAPI::Error error, Room *room)
  {
    ASSERT(room)

    LOG_WON(("ConnectRoomCallback in"))
    LOG_WON(("ConnectEx CB: %d %s", error, WONErrorToString(error)))

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

        room->internal.routingServer->RegisterEx
        (
          loginName,                                  // const ClientName& theClientNameR
          std::wstring(room->internal.password.str),  // const std::string& thePasswordR
          FALSE,                                      // bool becomeHost
          FALSE,                                      // bool becomeSpectator
          TRUE,                                       // bool joinChat
          RegisterPlayerCallback,                     // void (*f)(const RegisterClientResult&, privsType)
          room                                        // privsType t
        );                   

        // Notify of successful connection to room
        room->PostEvent(Message::ConnectedRoom);
        break;
      }

      case WONAPI::Error_HostUnreachable:
        room->PostEvent(Error::JoinRoomFailure);
        break;

      default:
        LOG_ERR(("ConnectEx CB: %d %s", error, WONErrorToString(error)))
        room->PostEvent(Error::JoinRoomFailure);
        break;
    }

    LOG_WON(("ConnectRoomCallback out"))
  }


  //
  // RegisterPlayerCallback
  //
  void RegisterPlayerCallback(const WONAPI::RoutingServerClient::RegisterClientResult &result, Room *room)
  {
    ASSERT(room)

    LOG_WON(("RegisterPlayerCallback in"))
    LOG_WON(("RegisterEx CB: %d %s", result.mStatus, WONErrorToString(result.mStatus)))

    switch (result.mStatus)
    {
      case WONMsg::StatusRouting_ClientAlreadyRegistered:
//        OutputError("Software error: already logged in");

      // fall through
      case WONMsg::StatusCommon_Success:
      {
        // Wait for data mutex
        LOG_WON(("critRouting.Wait"))

        // Begin player list update
        room->internal.routingServer->GetClientListEx
        (
          FALSE,
          TRUE,
          TRUE,
          ClientListCallback,
          room
        );

        // Begin game list updates
        room->internal.routingServer->SubscribeDataObjectEx
        (
          WONAPI::RoutingServerClient::GROUPID_ALLUSERS,  // ClientOrGroupId theLinkId
          dataGamePrefix,                                 // const WONCommon::RawBuffer& theDataTypeR
          FALSE,                                          // bool flagExactOrRecursive
          TRUE,                                           // bool flagGroupOrMembers
          SubscribeDataObjectCallback,                    // void (*f)(const ReadDataObjectResult&, privsType)
          room                                            // privsType t
        );
    
        room->PostEvent(Message::EnteredRoom);
        room->PostEvent(Message::PlayersChanged);
        room->PostEvent(Message::GamesChanged);
        break;
      }

      case WONMsg::StatusRouting_ClientAlreadyExists:
        room->PostEvent(Error::JoinRoomBadUsername);
        break;

      case WONMsg::StatusRouting_InvalidPassword:
        room->PostEvent(Error::JoinRoomBadPassword);
        break;

      case WONMsg::StatusRouting_ServerFull:
        room->PostEvent(Error::JoinRoomFull);
        break;

      default:
        LOG_ERR(("RegisterEx CB: %d %s", result.mStatus, WONErrorToString(result.mStatus)))
        room->PostEvent(Error::JoinRoomFailure);
        break;
    }

    LOG_WON(("RegisterPlayerCallback out"))
  }


  //
  // ClientListCallback
  //
  void ClientListCallback(const WONAPI::RoutingServerClient::GetClientListResult &result, Room *room)
  {
    LOG_WON(("ClientListCallback in"))
    LOG_WON(("GetClientListEx CB: %d %s", result.mStatus, WONErrorToString(result.mStatus)))

    switch (result.mStatus)
    {
      case WONMsg::StatusCommon_Success:
      {
        for (std::list<WONMsg::MMsgRoutingGetClientListReply::ClientData>::const_iterator i = result.mClientList.begin(); i != result.mClientList.end(); i++)
        {
          PlayerName name;
          Utils::Strmcpy(name.str, (const CH *) (*i).mClientName.c_str(), Min<U32>((*i).mClientName.length() / sizeof (CH) + 1, name.GetSize()));
          room->internal.AddPlayer((*i).mClientId, name.str, (*i).mIsMuted, (*i).mIsModerator, (*i).mIPAddress);
        }
        break;
      }

      default:
        LOG_ERR(("GetClientListEx CB: %d %s", result.mStatus, WONErrorToString(result.mStatus)))
        break;
    }

    LOG_WON(("ClientListCallback out"))
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
  void SubscribeDataObjectCallback(const WONAPI::RoutingServerClient::ReadDataObjectResult &result, Room *room)
  {
    LOG_WON(("SubscribeDataObjectCallback in"))
    LOG_WON(("SubscribeDataObjectEx CB: %d %s", result.mStatus, WONErrorToString(result.mStatus)))

    switch (result.mStatus)
    {
		  case WONMsg::StatusCommon_Success:
		  {
        LOG_WON(("Subscribed to game list"))

        for 
        (
          std::list<WONAPI::RoutingServerClient::DataObject>::const_iterator i = result.mDataObjectList.begin(); 
          i != result.mDataObjectList.end(); 
          ++i
        )
			  {
          GameName name = reinterpret_cast<const char *>(i->mDataType.data() + dataGamePrefix.size());

          /*
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
          */
			  }
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
  void ClientEnterExCatcher(const WONAPI::RoutingServerClient::ClientDataWithReason &data, Room *room)
  {
    PlayerName name;
    Utils::Strmcpy(name.str, (const CH *) data.mClientName.c_str(), Min<U32>(data.mClientName.length() / sizeof (CH) + 1, name.GetSize()));
    room->internal.AddPlayer(data.mClientId, name.str, data.mIsMuted, data.mIsModerator, data.mIPAddress);
  }


  //
  // ClientLeaveCatcher
  //
  void ClientLeaveCatcher(const WONAPI::RoutingServerClient::ClientIdWithReason &data, Room *room)
  {
    room->internal.RemovePlayer(data.mClientId);
  }


  //
  // MuteClientCatcher
  //
  void MuteClientCatcher(const WONAPI::RoutingServerClient::ClientIdWithFlag &reason, Room *room)
  {
    LOG_WON(("MuteClient [%d] %d", reason.mClientId, reason.mFlagOnOrOff))
    room->internal.MutePlayer(reason.mClientId, reason.mFlagOnOrOff);
  }


  //
  // BecomeModeratorCatcher
  //
  void BecomeModeratorCatcher(const WONAPI::RoutingServerClient::ClientIdWithFlag &reason, Room *room)
  {
    LOG_WON(("BecomeModerator [%d] %d", reason.mClientId, reason.mFlagOnOrOff))
    room->internal.ModeratePlayer(reason.mClientId, reason.mFlagOnOrOff);
  }


  //
  // HostChangeCatcher
  //
  void HostChangeCatcher(const WONAPI::RoutingServerClient::ClientId reason, Room *)
  {
    LOG_WON(("HostChange"))
    reason;
  }


  //
  // DataObjectCreationCatcher
  //
  void DataObjectCreationCatcher(const WONAPI::RoutingServerClient::DataObjectWithLifespan &reason, Room *room)
  {
    GameName name = reinterpret_cast<const char *>(reason.mDataType.data() + dataGamePrefix.size());
    room->internal.AddGame(reason.mOwnerId, name.str, reason.mData.size(), reason.mData.data());
  }


  //
  // DataObjectDeletionCatcher
  //
  void DataObjectDeletionCatcher(const WONAPI::RoutingServerClient::DataObject &reason, Room *room)
  {
    GameName name = reinterpret_cast<const char *>(reason.mDataType.data() + dataGamePrefix.size());
    room->internal.RemoveGame(reason.mOwnerId, name.str);
  }


  //
  // DataObjectModificationCatcher
  //
  void DataObjectModificationCatcher(const WONAPI::RoutingServerClient::DataObjectModification &, Room *)
  {
  }


  //
  // DataObjectReplacementCatcher
  //
  void DataObjectReplacementCatcher(const WONAPI::RoutingServerClient::DataObject &reason, Room *room)
  {
    GameName name = reinterpret_cast<const char *>(reason.mDataType.data() + dataGamePrefix.size());
    room->internal.UpdateGame(reason.mOwnerId, name.str, reason.mData.size(), reason.mData.data());
  }


  //
  // KeepAliveCatcher
  //
  void KeepAliveCatcher(Room *)
  {
    LOG_WON(("KeepAlive"))
  }


  //
  // RawPeerChatCatcher
  //
  void RawPeerChatCatcher(const WONAPI::RoutingServerClient::RawChatMessage &, Room *)
  {
    LOG_WON(("RawPeerChat"))
  }


  //
  // ASCIIPeerChatCatcher
  //
  void ASCIIPeerChatCatcher(const WONAPI::RoutingServerClient::ASCIIChatMessage &message, Room *room)
  {
    /*
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
    */
  }


  //
  // UnicodePeerChatCatcher
  //
  void UnicodePeerChatCatcher(const WONAPI::RoutingServerClient::UnicodeChatMessage &message, void *)
  {
    /*
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
    */
  }


  //
  // ReconnectFailureCatcher
  //
  void ReconnectFailureCatcher(void *)
  {
    /*
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
    */
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



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Room
  //


  //
  // Constructor
  //
  Room::Room(const CH *name)
  : name(name),
    internal(*new Internal(*this))
  {
  }


  //
  // Destructor
  //
  Room::~Room()
  {
    if (internal.routingServer)
    {
      internal.routingServer->Close();
      delete internal.routingServer;
      internal.routingServer = NULL;
    }

    // Clear ignored players
    ignoredPlayers.UnlinkAll();

    // Delete internals
    delete &internal;
  }


  //
  // Find a player by id
  //
  Player * Room::GetPlayerById(U32 id)
  {
    return (internal.players.Find(id));
  }


  //
  // Find a player by name
  //
  Player * Room::GetPlayerByName(const CH *name)
  {
    U32 crc = Crc::CalcStr(name);
    for (NBinTree<Player>::Iterator p(&internal.players); *p; ++p)
    {
      if ((*p)->name.crc == crc)
      {
        return (*p);
      }
    }
    return (NULL);
  }


  //
  // Add a game to the list of games
  //
  void Room::AddGame(const GameName &name, U32 size, const U8 *d)
  {
    WONCommon::RawBuffer data;
    data.assign(d, size);

    if (internal.routingServer)
    {
      internal.routingServer->CreateDataObjectEx
      (
        WONAPI::RoutingServerClient::GROUPID_ALLUSERS,  // ClientOrGroupId theLinkId
        dataGamePrefix + (U8 *) name.str,               // const WONCommon::RawBuffer& theDataTypeR
        routingServer->GetClientId(),                   // ClientOrGroupId theOwnerId
        0,                                              // unsigned short theLifespan
        data,                                           // const WONCommon::RawBuffer& theDataR
        CreateDataObjectCallback,                       // void (*f)(short, privsType)
        (void *) NULL                                   // privsType t)
      );
    }
  }


  //
  // Update a game
  //
  void Room::UpdateGame(const GameName &name, U32 size, const U8 *d)
  {
    WONCommon::RawBuffer data;
    data.assign(d, size);

    LOG_WON(("critRouting.Wait"))
    critRouting.Wait();

    if (routingServer)
    {
      routingServer->ReplaceDataObjectEx
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
  void Room::RemoveGame(const GameName &name)
  {
    LOG_WON(("critRouting.Wait"))
    critRouting.Wait();

    if (routingServer)
    {
      routingServer->DeleteDataObjectEx
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
  void Room::GetGameList(NList<Game> &games)
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
  // Send broadcast chat message
  //
  void Room::BroadcastMessage(const CH *text)
  {
    LOG_WON(("critRouting.Wait"))
    critRouting.Wait();

    if (routingServer)
    {
      routingServer->BroadcastChat(wstring(text), FALSE);
    }

    LOG_WON(("critRouting.Signal"))
    critRouting.Signal();
  }


  //
  // Send emote chat message
  //
  void Room::EmoteMessage(const CH *text)
  {
    LOG_WON(("critRouting.Wait"))
    critRouting.Wait();

    if (routingServer)
    {
      wstring message(text);
      routingServer->BroadcastChat(
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
  void Room::PrivateMessage(const char *player, const CH *text)
  {
    LOG_WON(("critRouting.Wait"))
    critRouting.Wait();

    if (routingServer)
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

    		routingServer->WhisperChat
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
  void Room::IgnorePlayer(const char *player)
  {
    PlayerName name;
    Utils::Ansi2Unicode(name.str, name.GetSize(), player);
    name.Update();
    IgnorePlayer(name);
  }

  void Room::IgnorePlayer(const PlayerName &name)
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
  void Room::UnignorePlayer(const char *player)
  {
    PlayerName name;
    Utils::Ansi2Unicode(name.str, name.GetSize(), player);
    name.Update();
    UnignorePlayer(name);
  }

  void Room::UnignorePlayer(const PlayerName &name)
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
  // Keep our Won connection alive
  //
  void Room::KeepAlive()
  {
    LOG_WON(("critRouting.Wait"))
    critRouting.Wait();

    LOG_WON(("Keeping connection alive using GetNumUsersEx"))

    if (routingServer)
    {
      routingServer->GetNumUsersEx
      (
        0,                    // unsigned short theTag
        GetNumUsersCallback,  // void (*f)(const GetNumUsersResult&, privsType t)
        this                  // privsType t
      );
    }

    LOG_WON(("critRouting.Signal"))
    critRouting.Signal();
  }


  //
  // Get the address which is connecting us to the internet
  //
  Bool Room::GetLocalAddress(U32 &ip, U16 &port)
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
  // PostEvent
  //
  void Room::PostEvent(U32 message, void *data)
  {
    //printf("Posting Event [%08x] %s\n", message, DecodeMessage(message));

    Event *e = eventQueue.AddPre();
    ASSERT(e)
    e->message = message;
    e->data = data;
    eventQueue.AddPost();
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Room::Internal
  //


  //
  // Constructor
  //
  void Room::Internal::Internal(Room &room)
  : room(room),
    players(&Player::node),
    games(&Game::node)
  {    
  }


  //
  // Add player
  //
  void Room::Internal::AddPlayer(U32 id, const CH *name, Bool muted, Bool moderator, U32 address)
  {
    Player *player = players.Find(id);

    if (player)
    {
      // Update the player
      player->muted = muted;
      player->moderator = moderator;

      // Notify interface of change of players
      room.PostEvent(Message::PlayersChanged);

      LOG_WON(("Updated Player '%s'[%d] mod:%d mute:%d", Utils::Unicode2Ansi(name), id, player->moderator, player->muted))
    }
    else
    {
      player = new Player(name, id, moderator, muted, FALSE);
      players.Add(id, player);

      // Notify interface of change of players
      room.PostEvent(Message::PlayersChanged);

      // Print a message with this player's name
      room.PostEvent(Message::Chat, new Message::Data::Chat(Message::Data::Chat::PlayerEntered, (const CH *)NULL, player->name.str));

      LOG_WON(("Added Player '%s'[%d] mod:%d mute:%d address:%s", Utils::Unicode2Ansi(name), id, player->moderator, player->muted))
    }
  }


  //
  // Remove a player
  //
  void Room::Internal::RemovePlayer(U32 id)
  {
    Player *player = players.Find(id);

    if (player)
    {
      // Notify interface of change of players
      room.PostEvent(Message::PlayersChanged);

      // Print a message with this player's name
      room.PostEvent(Message::Chat, new Message::Data::Chat(Message::Data::Chat::PlayerLeft, (const CH *)NULL, name.str));

      players.Dispose(player);
    }
  }


  //
  // Mute/Unmute a player
  //
  void Room::Internal::MutePlayer(U32 id, Bool mute)
  {
    Player *player = players.Find(id);

    if (player)
    {
      player->muted = mute;

      // Notify interface of change of players
      room.PostEvent(Message::PlayersChanged);
    }
  }


  //
  // Mute/Unmute a player
  //
  void Room::Internal::ModeratePlayer(U32 id, Bool moderate)
  {
    Player *player = players.Find(id);

    if (player)
    {
      player->moderator = moderate;

      // Notify interface of change of players
      room.PostEvent(Message::PlayersChanged);
    }
  }


  //
  // Add a game to this server
  //
  void Room::Internal::AddGame(U32 ownerId, const GameName &name, U32 size, const U8 *d)
  {
  }


  //
  // Update a game on this server
  //
  void Room::Internal::UpdateGame(U32 ownerId, const GameName &name, U32 size, const U8 *d)
  {
  }


  //
  // Remove a game from this server
  //
  void Room::Internal::RemoveGame(U32 ownerId, const GameName &name)
  {
  }

}
