///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Networking WON Stuff
//


#ifndef __WONIFACE_H
#define __WONIFACE_H


#ifdef _USRDLL
  #define DLL_DECL DLL_EXPORT
#else
  #define DLL_DECL 
#endif


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "debug_memory.h"
#include "queue.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace WonIface
//
namespace WonIface
{

  //
  // Type definitions
  //
  typedef StrCrc<32, CH> PlayerName;
  typedef StrCrc<32, CH> RoomName;
  typedef StrCrc<32, char> GameName;
  typedef StrBuf<32, CH> PasswordStr;



  /////////////////////////////////////////////////////////////////////////////
  //
  // Struct Event
  //
  struct Event
  {
    // Event's message
    U32 message;

    // Event's data
    void *data;

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Message
  //
  namespace Message
  {
    // Successfully retrieved server list
    const U32 RetrievedServerList = 0x167A1BD4; // "Message::RetrievedServerList"

    // Firewall status updated
    const U32 FirewallStatus      = 0xB29A920F; // "Message::FirewallStatus"

    // Account was logged in successfully
    const U32 LoggedIn            = 0xB8A78CAF; // "Message::LoggedIn"

    // Account was created successfully
    const U32 CreatedAccount      = 0x2CF8742D; // "Message::CreatedAccount"

    // Password was successfully changed
    const U32 ChangedPassword     = 0xC1FDF3FE; // "Message::ChangedPassword"

    // Chat message
    const U32 Chat                = 0x0B3E6240; // "Message::Chat"


    // The first room update was complted
    const U32 InitialRoomUpdate   = 0x217B7065; // "Message::InitialRoomUpdate"

    // A room update was completed
    const U32 RoomsUpdated        = 0x8234728F; // "Message::RoomsUpdated"

    // A new room was created
    const U32 CreatedRoom         = 0xBD6BF24B; // "Message::CreatedRoom"

    // A new room was registered
    const U32 RegisteredRoom      = 0x5418CDEE; // "Message::RegisteredRoom"

    // We have connected to a room
    const U32 ConnectedRoom       = 0x69BC04E9; // "Message::ConnectedRoom"

    // We have entered a room
    const U32 EnteredRoom         = 0xB5704F6A; // "Message::EnteredRoom"


    // Player list changed
    const U32 PlayersChanged      = 0xB4504DF9; // "Message::PlayersChanged"


    // Game list changed
    const U32 GamesChanged        = 0x1CACB3BE; // "Message::GamesChanged"


    // A new game was created
    const U32 CreatedGame         = 0x660BF5C7; // "Message::CreatedGame"


    // A progress update for a HTTP download
    const U32 HTTPProgressUpdate  = 0xBAD75EC2; // "Message::HTTPProgressUpdate"

    // A HTTP download completed
    const U32 HTTPCompleted       = 0xC41D0A90; // "Message::HTTPCompleted"


    ///////////////////////////////////////////////////////////////////////////
    //
    // Struct Chat
    //
    namespace Data
    {

      /////////////////////////////////////////////////////////////////////////
      //
      // Struct Chat
      //
      struct Chat : public Debug::Memory::UnCached
      {
        enum
        {
          Private,
          Broadcast,
          Emote,
          PlayerEntered,
          PlayerLeft,
          GameCreated,
          GameDestroyed
        };

        // Chat type
        U32 id;

        // Text
        CH *text;

        // Username
        CH *user;


        // Constructor
        Chat(U32 id, const CH *text, const CH *user = NULL);
        Chat(U32 id, const char *textIn, const CH *user = NULL);

        // Destructor
        DLL_DECL ~Chat();
      };


      /////////////////////////////////////////////////////////////////////////
      //
      // Struct EnteredRoom
      //
      struct EnteredRoom : public Debug::Memory::UnCached
      {
        CH *text;

        // Constructor
        EnteredRoom(const CH *text);

        // Destructor
        DLL_DECL ~EnteredRoom();
      };


      /////////////////////////////////////////////////////////////////////////
      //
      // Struct HTTPProgressUpdate
      //
      struct HTTPProgressUpdate : public Debug::Memory::UnCached
      {
        // Handle
        U32 handle;

        // Progress
        U32 progress;

        // Size
        U32 size;

        // Constructor
        HTTPProgressUpdate(U32 handle, U32 progress, U32 size)
        : handle(handle),
          progress(progress),
          size(size)
        {
        }

      };


      /////////////////////////////////////////////////////////////////////////
      //
      // Struct HTTPCompleted
      //
      struct HTTPCompleted : public Debug::Memory::UnCached
      {
        // Handle
        U32 handle;

        // Is the file new ?
        Bool isNew;

        // Constructor
        HTTPCompleted(U32 handle, Bool isNew)
        : handle(handle),
          isNew(isNew)
        {
        }

      };

    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Error
  //
  namespace Error
  {
    // Could not connect to WON
    const U32 ConnectionFailure             = 0x825F5FD5; // "Error::ConnectionFailure"

    // User name is not registered
    const U32 LoginInvalidUsername          = 0x5B6E21DC; // "Error::LoginInvalidUsername"

    // Bad password
    const U32 LoginInvalidPassword          = 0xC67B2D75; // "Error::LoginInvalidPassword"

    // General Login failure
    const U32 LoginFailure                  = 0xBC3883EE; // "Error::LoginFailure"

    // Key expired
    const U32 KeyExpired                    = 0xF3E8CDDB; // "Error::KeyExpired"

    // Verify failed
    const U32 VerifyFailed                  = 0x4B26BEE8; // "Error::VerifyFailed"

    // Locked out
    const U32 LockedOut                     = 0x3362E2C3; // "Error::LockedOut"

    // Key In use
    const U32 KeyInUse                      = 0x1923FD31; // "Error::KeyInUse"

    // Invalid CD key
    const U32 KeyInvalid                    = 0xC6081FDB; // "Error::KeyInvalid"


    // Create account failure
    const U32 CreateAccountFailure          = 0xBA707CCD; // "Error::CreateAccountFailure"

    // Create account username in use
    const U32 CreateAccountBadUser          = 0x08748826; // "Error::CreateAccountBadUser"

    // Create account bad password
    const U32 CreateAccountBadPassword      = 0x085A7E97; // "Error::CreateAccountBadPassword"

    // Change password failure
    const U32 ChangePasswordFailure         = 0x84492550; // "Error::ChangePasswordFailure"

    // Change password bad password
    const U32 ChangePasswordBadNewPassword  = 0xEEB52840; // "Error::ChangePasswordBadNewPassword"

    // No lobby could be found
    const U32 NoLobby                       = 0xC9748451; // "Error::NoLobby"

    // Specified room couldnt be found
    const U32 NoRoom                        = 0x2EDB7E3D; // "Error::NoRoom"

    // Could not join room
    const U32 JoinRoomFailure               = 0xB16814BE; // "Error::JoinRoomFailure"
    const U32 JoinRoomBadUsername           = 0x19C7D614; // "Error::JoinRoomBadUsername"
    const U32 JoinRoomBadPassword           = 0x84D2DABD; // "Error::JoinRoomBadPassword"
    const U32 JoinRoomFull                  = 0xF6E0D03C; // "Error::JoinRoomFull"

    // Could not create room 
    const U32 CreateRoomFailure             = 0x5C5305AF; // "Error::CreateRoomFailure"

    // Could not create a game
    const U32 CreateGameFailure             = 0xC1647E57; // "Error::CreateGameFailure"

    // Reconnect failure (lost our connection to WON)
    const U32 ReconnectFailure              = 0xE0E427C0; // "Error::ReconnectFailure"

    // Failed to download file
    const U32 HTTPFailed                    = 0x01BD6C07; // "Error::HTTPFailed"


    ///////////////////////////////////////////////////////////////////////////
    //
    // Struct Chat
    //
    namespace Data
    {

      /////////////////////////////////////////////////////////////////////////
      //
      // Struct HTTPFailed
      //
      struct HTTPFailed : public Debug::Memory::UnCached
      {
        U32 handle;

        HTTPFailed(U32 handle)
        : handle(handle)
        {
        }

      };

    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Firewall
  //
  namespace Firewall
  {
    // We haven't checked to see if we're behind a firewall
    const U32 Unchecked           = 0x3049E0AB; // "Unchecked"

    // We're checking to see if we're behind a firewall
    const U32 Checking            = 0xF9361A7D; // "Checking"

    // We are behind a firewall (for the purpose of this game)
    const U32 Behind              = 0x16AD040D; // "Behind"
    
    // We are not behind a firewall (for the purpose of this game)
    const U32 None                = 0xC9EF9119; // "None"

  };
  

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Player
  //
  struct Player : public Debug::Memory::UnCached
  {
    // List node
    NList<Player>::Node node;

    // Id of the player
    U32 id;

    // Name of the player
    PlayerName name;

    // Is this player a moderator
    Bool moderator;

    // Is this player muted 
    Bool muted;

    // Is this player ignored
    Bool ignored;

    // Initializing Constructor
    Player(const CH *name, U32 id, Bool moderator, Bool muted, Bool ignored)
    : name(name),
      id(id),
      moderator(moderator),
      muted(muted),
      ignored(ignored)
    {
    }
  
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Room
  //
  struct Room : public Debug::Memory::UnCached
  {
    // List Node
    NList<Room>::Node node;

    // Name of the room
    RoomName name;

    // Number of players in the room
    U32 numPlayers;

    // Password protected
    Bool password;

    // Lobby
    Bool lobby;

    // Constructor
    Room(const CH *name, U32 numPlayers, Bool password, Bool lobby)
    : name(name),
      numPlayers(numPlayers),
      password(password),
      lobby(lobby)
    {
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Game
  //
  struct Game : public Debug::Memory::UnCached
  {
    // List Node
    NList<Game>::Node node;

    // Name of the game
    GameName name;

    // Name of the host
    PlayerName host;

    // Size of the data
    U32 size;

    // The data
    U8 *data;

    // Constructor
    Game(const GameName &name, const PlayerName &host, U32 size, const U8 *dataIn)
    : name(name),
      host(host),
      size(size)
    {
      data = reinterpret_cast<U8 *>(Debug::Memory::UnCached::Alloc(size));
      Utils::Memcpy(data, dataIn, size);
    }

    // Destructor
    ~Game()
    {
      Debug::Memory::UnCached::Free(data);
    }

  };

  // Initialize Won
  void DLL_DECL Init();

  // Shutdown Won
  void DLL_DECL Done();

  // Set the directory servers to use
  void DLL_DECL SetDirectoryServers(const List<char> &servers);

  // Process Won
  Bool DLL_DECL Process(U32 &message, void *&data);

  // Event queue
  DLL_DECL SafeQueue<Event, 256> & GetEventQueue();

  // Connect to Won
  void DLL_DECL Connect();

  // Disconnect from Won
  void DLL_DECL Disconnect();

  // Abort what ever we're doing
  void DLL_DECL Abort();


  // Create a Won account
  void DLL_DECL CreateAccount(const char *username, const char *password);

  // Login to a Won account
  void DLL_DECL LoginAccount(const char *username, const char *password);

  // Change the password of an existing Won account
  void DLL_DECL ChangePassword(const char *username, const char *oldPassword, const char *newPassword);


  // Detect if we're behind a firewall
  void DLL_DECL DetectFirewall();

  // What's our firewall status
  U32 DLL_DECL GetFirewallStatus();


  // Keep our Won connection alive
  void DLL_DECL KeepAlive();

  // Update the list of rooms from the server
  void DLL_DECL UpdateRooms();

  // Get the current list of rooms
  void DLL_DECL GetRoomList(NList<Room> &rooms);

  // Create a room
  void DLL_DECL CreateRoom(const CH *roomname, const CH *password);

  // Join a room
  void DLL_DECL JoinRoom(const CH *roomname, const CH *password);

  // Get the list of players in the current room
  void DLL_DECL GetPlayerList(NList<Player> &players);


  // Add a game to the list of games
  void DLL_DECL AddGame(const GameName &name, U32 size, const U8 *data);

  // Update a game
  void DLL_DECL UpdateGame(const GameName &name, U32 size, const U8 *data);

  // Remove a game from the list of games
  void DLL_DECL RemoveGame(const GameName &name);

  // Get the list of games in the current room
  void DLL_DECL GetGameList(NList<Game> &games);


  // Get the address which is connecting us to the internet
  Bool DLL_DECL GetLocalAddress(U32 &ip, U16 &port);


  // Send broadcast chat message
  void DLL_DECL BroadcastMessage(const CH *text);

  // Send emote chat message
  void DLL_DECL EmoteMessage(const CH *text);

  // Send private chat message
  void DLL_DECL PrivateMessage(const char *player, const CH *text);


  // Ignore a player
  void DLL_DECL IgnorePlayer(const char *player);
  void DLL_DECL IgnorePlayer(const PlayerName &player);

  // Unignore a player
  void DLL_DECL UnignorePlayer(const char *player);
  void DLL_DECL UnignorePlayer(const PlayerName &player);

  // Check the key in the registry
  Bool DLL_DECL CheckStoredKey();

  // Check the given key
  Bool DLL_DECL CheckKey(const char *key);



  // Retrieve a HTTP file and store it locally in the download directory
  //U32 DLL_DECL HTTPGet(U32 proxyIP, U16 proxyPort, const char *hostName, U32 hostPort, const char *path, const char *local, Bool allowResume);
  U32 DLL_DECL HTTPGet(const char *proxy, const char *hostName, U16 hostPort, const char *path, const char *local, Bool allowResume);

  // Retrieve a HTTP file and store it locally in the download directory
  U32 DLL_DECL HTTPGet(const char *hostName, U16 hostPort, const char *path, const char *local, Bool allowResume);

  // Stop downloading a HTTP file
  void DLL_DECL HTTPAbortGet(U32 handle);


  // Enable/Disable logging
  void DLL_DECL Logging(Bool on);

}

#endif
