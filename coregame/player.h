///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Player
// 24-JUL-1998
//

#ifndef __PLAYER_H
#define __PLAYER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "gameconstants.h"
#include "unitobjdec.h"
#include "varsys.h"
#include "trailobjdec.h"
#include "squadobjdec.h"
#include "gameobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Forward declarations
class Team;


///////////////////////////////////////////////////////////////////////////////
//
// Class Player
//
class Player
{
private:

  LOGDEC

public:

  // Types of players
  enum
  {
    LOCAL, NETWORK, AI
  };

protected:

  // General

  // Player Identifer (0..255)
  U32 id;

  // Name of the player
  GameIdent name;

  // Type of player
  U32 type;

  // Team which this player is controlling
  Team *team;

  // Display name (wide char)
  CH *displayName;

  // Do orders get routed for this player ?
  Bool route;

  // Has this player departed ?
  Bool departed;

  // List of objects currently selected
  UnitObjList selectedObjList;

  // The last trail created by this player
  TrailObjPtr lastTrail;

  
  // Misc

  // Tree nodes
  NBinTree<Player>::Node node;
  NBinTree<Player>::Node nodeHuman; 

public:

  // Tree node
  NBinTree<Player>::Node mpNode;

public:

  // Static Data

  // Player System Initialized ?
  static Bool initialized;

  // The current client-side player
  static Player *currentPlayer;

  // Tree containing all of the current teams by name
  static NBinTree<Player> playersByName;

  // Tree containing all human players by name
  static NBinTree<Player> humanPlayersByName;

  // Array of teams by id
  static Player *playersById[Game::MAX_PLAYERS];

public:

  // Player
  Player(GameIdent name, U32 type = LOCAL, const CH *displayName = NULL, Bool route = TRUE);

  // ~Player
  ~Player();

  // PostLoad
  void PostLoad();

  // Adds an object to the selection list
  void AddToSelectedList(UnitObj* obj);

  // Clears the selected object list
  void ClearSelectedList();

  // Returns a reference to the selected objects list of this player
  UnitObjList &GetSelectedList();

  // Set the team with which this player is associated
  virtual void SetTeam(Team *t);

  // Clears the team with which this player is associated
  virtual void ClearTeam();

  // Notification (return TRUE if the message is accepted .. return FALSE if not sure)
  virtual Bool Notify(GameObj *from, U32 message, U32 param1 = 0, U32 param2 = 0);

  // Get / set the last trail created by this player
  TrailObj * GetLastTrail();
  void SetLastTrail(TrailObj *trail);

  // GetName
  const char * GetName() const
  {
    return (name.str);
  }

  // GetNameCrc
  U32 GetNameCrc() const
  {
    return (name.crc);
  }

  // GetId
  U32 GetId() const
  {
    return (id);
  }

  // GetType
  U32 GetType() const
  {
    return (type);
  }

  // GetDisplayName
  const CH *GetDisplayName() const
  {
    return (displayName);
  }

  // Route the orders of this player ?
  Bool IsRoute() const
  {
    return (route);
  }

  // GetTeam : Gets the team with which this player is associated
  Team * GetTeam() const
  { 
    return (team); 
  }

  // Has this player departed ?
  Bool HasDeparted()
  {
    return (departed);
  }

  // This player had departed 
  void Departed()
  {
    departed = TRUE;
  }

public:

  // Init : Initialize player system
  static void Init();

  // Done : Shutdown player system
  static void Done();

  // Set the current player to go on auto pilot
  static void SetAutoPilot(const char *pilot);

  // PostLoadAll : Post Load all of the players
  static void PostLoadAll();

  // Set the current client-side player
  static void SetCurrentPlayer(Player *player);

  // Get the the commander of this unit (or NULL)
  static Player * GetCommander(UnitObj *unit);

  // Get the current client-side player
  static Player * GetCurrentPlayer()
  {
    ASSERT(initialized)
    return (currentPlayer);
  }

  // Name2Player: Converts a player name into a player pointer
  static Player * Name2Player(const char *name)
  {
    return (playersByName.Find(Crc::CalcStr(name)));
  }

  // Name2HumanPlayer: Converts a player name into a player pointer
  static Player * Name2HumanPlayer(const char *name)
  {
    return (humanPlayersByName.Find(Crc::CalcStr(name)));
  }

  // Id2Player: Converts a player Id to a palyer pointer
  static Player * Id2Player(U32 id)
  {
    ASSERT(id < Game::MAX_PLAYERS)
    return (playersById[id]);
  }

private:

  // NewId: Returns an unused Id
  static U32 NewId();

  // CmdHandler : Console Command handler
  static void CmdHandler(U32 pathCrc);


};

#endif