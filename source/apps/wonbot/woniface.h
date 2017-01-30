///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Networking WON Stuff
//


#ifndef __WONIFACE_H
#define __WONIFACE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "queue.h"


///////////////////////////////////////////////////////////////////////////////
//
// Forward Declarations
//
namespace WONAPI
{
  class RoutingServerClient;
}


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




    // Only found in a room's event queue

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
        ~Chat();
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
        ~EnteredRoom();
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


    // Only found in a room's event queue

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
  struct Player
  {
    // List node
    NBinTree<Player>::Node node;

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
  // Struct Game
  //
  struct Game
  {
    // List Node
    NBinTree<Game>::Node node;

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


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Room
  //
  class Room
  {
  private:

    // Name of the chat server
    RoomName name;

    // Num players
    U32 numPlayers;

    // Is this server password protected ?
    Bool hasPassword;

    // Is this server permanent ?
    Bool permanent;

    // Ignored players in this room
    BinTree<void> ignoredPlayers;

  public:

    struct Internal;
    Internal &internal;

    // List node
    NBinTree<Room>::Node node;

  public:

    // Constructor
    Room(const CH *name);

    // Destructor
    ~Room();


    // Find a player by id
    Player * GetPlayerById(U32 id);

    // Find a player by name
    Player * GetPlayerByName(const CH *name);


    // Add a game to this server
    void AddGame(const GameName &name, U32 size, const U8 *d);

    // Update a game on this server
    void UpdateGame(const GameName &name, U32 size, const U8 *d);

    // Remove a game from this server
    void RemoveGame(const GameName &name);

    // Get the list of games on this server
    void GetGameList(NList<Game> &games);


    // Broadcast a message
    void BroadcastMessage(const CH *text);

    // Emote a message
    void EmoteMessage(const CH *text);

    // Send a private message to a player
    void PrivateMessage(const char *player, const CH *text);


    // Ignore a player
    void IgnorePlayer(const char *player);
    void IgnorePlayer(const PlayerName &name);

    // Unignore a player
    void UnignorePlayer(const char *player);
    void UnignorePlayer(const PlayerName &name);


    // Keep our Won connection alive
    void KeepAlive();

    // Get our local address 
    Bool GetLocalAddress(U32 &ip, U16 &port);
  
    // Post an event to this room's event queue
    void PostEvent(U32 message, void *data = NULL);

  public:

    // Friends
    friend void CreateRoom(const CH *name, const CH *password);
    friend void ConnectRoom(Room &room);

  };



  // Initialize Won
  void Init();

  // Shutdown Won
  void Done();

  // Set the directory servers to use
  void SetDirectoryServers(const List<char> &servers);

  // Process Won
  Bool Process(U32 &message, void *&data);

  // Event queue
  SafeQueue<Event, 256> & GetEventQueue();

  // Connect to Won
  void Connect();

  // Disconnect from Won
  void Disconnect();

  // Abort what ever we're doing
  void Abort();


  // Create a Won account
  void CreateAccount(const char *username, const char *password);

  // Login to a Won account
  void LoginAccount(const char *username, const char *password);

  // Change the password of an existing Won account
  void ChangePassword(const char *username, const char *oldPassword, const char *newPassword);


  // Detect if we're behind a firewall
  void DetectFirewall();

  // What's our firewall status
  U32 GetFirewallStatus();


  // Keep our Won connection alive
  void KeepAlive();

  // Update the list of rooms from the server
  void UpdateRooms();


  // Get the current list of rooms
  void GetRoomList(List<Room> &rooms);

  // Create a room
  void CreateRoom(const CH *roomname, const CH *password);

  // Join a room
  void JoinRoom(const CH *roomname, const CH *password);

  // Connect to a room
  void ConnectRoom(Room &room);

  // Check the key in the registry
  Bool CheckStoredKey();

  // Check the given key
  Bool CheckKey(const char *key);

}

#endif
