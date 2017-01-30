///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
// 1-JUL-1999
//

#ifndef __MULTIPLAYER_NETWORK_H
#define __MULTIPLAYER_NETWORK_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Definitions
  //
  typedef StrCrc<16> UserName;
  typedef StrCrc<32> SessionName;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Network
  //
  namespace Network
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Struct Player
    //
    struct Player
    {
      // List node
      NBinTree<Player, CRC>::Node node;

      // Name of the player
      UserName name;

      // Entry number
      U32 entryNumber;


      // Constructor
      Player(const UserName &name)
      : name(name),
        entryNumber(0)
      {
      }

      // Get the id
      U32 GetId() const
      {
        return (name.crc);
      }
      
      // Get the name
      const char * GetName() const
      {
        return (name.str);
      }

    };


    // Initialization
    void Init();

    // Shutdown
    void Done();

    // Reset
    void Reset();

    // Process
    void Process();

    // Get the tree of players
    const NBinTree<Player, CRC> & GetPlayers();

    // Get the ident of the current player
    const UserName & GetCurrentName();

    // Get the current player
    const Player & GetCurrentPlayer();

    // Create the current player (only for instant action!)
    void CreateCurrentPlayer();

    // Do we have a current player yet ?
    Bool HaveCurrentPlayer();

    // Start migration
    void StartMigration();

  }
  
}


#endif
