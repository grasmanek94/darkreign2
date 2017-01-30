///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Player
// 24-JUL-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "player.h"
#include "varsys.h"
#include "console.h"
#include "team.h"
#include "unitobj.h"
#include "client.h"
#include "trailobj.h"
#include "unitobjlist.h"


///////////////////////////////////////////////////////////////////////////////
//
// Internal Data
//
Bool Player::initialized = FALSE;
Player *Player::currentPlayer;
NBinTree<Player> Player::playersByName(&Player::node);
NBinTree<Player> Player::humanPlayersByName(&Player::nodeHuman);
Player *Player::playersById[Game::MAX_PLAYERS];


///////////////////////////////////////////////////////////////////////////////
//
// Class Player
//


//
// Player::Player
//
Player::Player(GameIdent name, U32 type, const CH *display, Bool route) 
: name(name), 
  id(NewId()), 
  team(NULL), 
  type(type), 
  route(route),
  departed(FALSE)
{
  ASSERT(initialized)

  // Add the player to the id array
  playersById[id] = this;

  // Add the player to the tree
  playersByName.Add(name.crc, this);

  if (type != AI)
  {
    humanPlayersByName.Add(name.crc, this);
  }

  // Generate display name
  displayName = Utils::Strdup(display ? display : Utils::Ansi2Unicode(name.str));
}


//
// Player::~Player
//
Player::~Player()
{
  ASSERT(initialized)

  // Remove all objects from the selected list
  selectedObjList.Clear();

  // Free the player id
  playersById[id] = NULL;

  // Remove from the player tree
  playersByName.Unlink(this);

  if (type != AI)
  {
    humanPlayersByName.Unlink(this);
  }

  // Delete display name
  delete[] displayName;
}


//
// Player::PostLoad
//
void Player::PostLoad()
{

}


//
// Player::AddToSelectedList
//
// Adds a MapObject to the selection list
//
void Player::AddToSelectedList(UnitObj* obj)
{
  selectedObjList.Append(obj);
}


//
// Player::ClearSelectedList
//
// Clears the selected MapObject list
//
void Player::ClearSelectedList()
{
  ASSERT(initialized)
  selectedObjList.Clear();
}


//
// Player::GetSelectedList
//
// Returns a reference to the selected objects list of this player
//
UnitObjList &Player::GetSelectedList()
{
  // Remove dead objects and objects not on our team
  UnitObjListUtil::PurgeNonTeamOffMap(selectedObjList, team);
  return (selectedObjList);
}


//
// Player::SetTeam
//
void Player::SetTeam(Team *t)
{
  ASSERT(initialized)
  ASSERT(!team)
  ASSERT(t)
  team = t; 

  if (this == currentPlayer)
  {
    Team::SetDisplayTeam(team);
  }
}


//
// Player::ClearTeam
//
// Clears the team with which this player is associated
//
void Player::ClearTeam() 
{ 
  ASSERT(initialized)
  ASSERT(team)

  team = NULL;

  // If this is the current player and there's a display team
  // (which is not neccesarily the case in the studio) then
  // clear the display team
  if (this == currentPlayer && Team::GetDisplayTeam())
  {
    Team::ClearDisplayTeam();
  }
}


//
// Notification
//
Bool Player::Notify(GameObj *from, U32 message, U32 param1, U32 param2)
{
  // If this is the current player then offer the message to the client for processing
  if (this == currentPlayer)
  {
    return (Client::Notify(from, message, param1, param2));
  }
  return (TRUE);
}


//
// GetLastTrail
//
// Returns the last trail created, or NULL if none
//
TrailObj * Player::GetLastTrail()
{
  return (lastTrail.GetPointer());
}


//
// SetLastTrail
//
// Sets the last trail reaper
//
void Player::SetLastTrail(TrailObj *trail)
{
  lastTrail = trail;
}


//
// Player::Init
//
// Initialize player system
//
void Player::Init()
{
  ASSERT(!initialized)

  // Clear the current player
  currentPlayer = NULL;

  // Ensure used Ids are clear
  Utils::Memset(playersById, 0x00, sizeof (playersById));

  // Define player commands
  VarSys::RegisterHandler("player", CmdHandler);

#ifdef DEVELOPMENT

  // Other commands
  VarSys::CreateCmd("player.list");
  VarSys::CreateCmd("player.reset");

#endif

  // Set the initialized flag
  initialized = TRUE;
}


//
// Player::Done
//
// Shutdown player system
//
void Player::Done()
{
  ASSERT(initialized)

  // Delete Players
  for (int i = 0; i < Game::MAX_PLAYERS; i++)
  {
    if (playersById[i])
    {
      delete playersById[i];
      playersById[i] = NULL;
    }
  }

  // Delete player scope
  VarSys::DeleteItem("player");

  // Clear the initialied flag
  initialized = FALSE;
}


//
// Player::SetAutoPilot
//
// Set the current player to go on auto pilot
//
void Player::SetAutoPilot(const char *pilot)
{
  pilot;

  ASSERT(currentPlayer)

  // Save some of the player information
  //Team *team = currentPlayer->GetTeam();

  // Delete old player 
  delete currentPlayer;
/*
  // Create a Strategic AI
  Strategic::Object *object = Strategic::Create(pilot, TRUE);

  if (team)
  {
    object->SetTeam(team);
  }

  // AI is now this player
  currentPlayer = object;
  */
}


//
// Player::PostLoadAll
//
// Post Load all of the players
//
void Player::PostLoadAll()
{
  ASSERT(initialized)

  // Post load each of the players
  for (int i = 0; i < Game::MAX_PLAYERS; i++)
  {
    if (playersById[i])
    {
      playersById[i]->PostLoad();
    }
  }
}


//
// Player::SetCurrentPlayer
//
// Set the current client-side player
//
void Player::SetCurrentPlayer(Player *player)
{
  ASSERT(initialized)
  currentPlayer = player;

  if (currentPlayer->GetTeam())
  {
    Team::SetDisplayTeam(currentPlayer->GetTeam());
  }
}


//
// GetCommander
//
// Get the the commander of this unit (or NULL)
//
Player * Player::GetCommander(UnitObj *unit)
{
  ASSERT(initialized)
  ASSERT(unit)
  
  // Ignore request if no current player
  if (currentPlayer)
  {
    // Check each players selected list
    for (int i = 0; i < Game::MAX_PLAYERS; i++)
    {
      // Is this an active player
      if (Player *player = playersById[i])
      {
        // Does this player have the unit selected
        if (player->GetSelectedList().Find(unit))
        {
          // If selected by the current player
          return ((player == currentPlayer) ? NULL : player);
        }
      }
    }

    // Is it on a team different to the current player
    if (unit->GetTeam() && (unit->GetTeam() != currentPlayer->GetTeam()))
    {
      // Get the leader of this team
      return (unit->GetTeam()->GetTeamLeader());
    }
  }

  // No commander
  return (NULL);
}


//
// Player::CmdHandler
//
// Console Command handler
//
void Player::CmdHandler(U32 pathCrc)
{
  ASSERT(initialized)

  switch (pathCrc)
  {
    case 0x2717CAD3: // "player.list"
    {
      NBinTree<Player>::Iterator i(&playersByName);

      CON_DIAG(("Current Players", (*i)->GetName))
      for (!i; *i; i++)
      {
        CON_DIAG((" %3d - %s [%s]", (*i)->GetId(), (*i)->GetName(), (*i)->GetTeam() ? (*i)->GetTeam()->GetName() : "----"))
      }
      break;
    }
  }
}


//
// Player::NewId
//
// Returns an unused Id
//
U32 Player::NewId()
{
  ASSERT(initialized)

  for (int i = 0; i < Game::MAX_PLAYERS; i++)
  {
    if (!playersById[i])
    {
      return (i);
    }
  }

  ERR_FATAL(("Ran out of player id's!"))
}
