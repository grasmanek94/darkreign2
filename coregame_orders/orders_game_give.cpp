///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game Orders
// 29-JUL-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "orders_game.h"
#include "resolver.h"
#include "taskctrl.h"
#include "team.h"
#include "client.h"
#include "message.h"
#include "tasks_unitupgrade.h"
#include "worldctrl.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Orders
//
namespace Orders
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Game
  //
  namespace Game
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class GiveUnits
    //
    U32 GiveUnits::orderId;


    //
    // Generate
    //
    void GiveUnits::Generate(Player &player, const Player *giveTo)
    {
      ASSERT(giveTo)

      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.giveTo = (U8) giveTo->GetId();

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 GiveUnits::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      // Give units to the player's team
      if (Player *newPlayer = Player::Id2Player(d->giveTo))
      {
        // Can we give units to this team
        if ((&player != newPlayer) && player.GetTeam() && newPlayer->GetTeam())
        {
          // Ensure teams are allied
          if (Team::TestRelation(player.GetTeam(), newPlayer->GetTeam(), Relation::ALLY))
          {
            UnitObjPtr first;
            
            // Give the units
            if (U32 amount = player.GetTeam()->GiveUnits(newPlayer->GetTeam(), player.GetSelectedList(), first))
            {
              // Trigger confirmation for the giver
              if (player.GetTeam() == Team::GetDisplayTeam())
              {
                Message::TriggerGameMessage(0x4ED94B91, 2, newPlayer->GetDisplayName(), amount); // "Team::GaveUnits"
              }
              else 
              
              // Trigger object message for the receiver
              if (newPlayer->GetTeam() == Team::GetDisplayTeam())
              {
                Message::TriggerObjMessage(0x66EC1C1D, first, NULL, 2, player.GetDisplayName(), amount); // "Team::GivenUnits"
              }
            }
          }
        }
      }

      return (sizeof (Data));
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class GiveResource
    //
    U32 GiveResource::orderId;


    //
    // Generate
    //
    void GiveResource::Generate(Player &player, const Player *giveTo, U32 amount)
    {
      ASSERT(giveTo)

      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.giveTo = (U8) giveTo->GetId();
      data.amount = amount;

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 GiveResource::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      ASSERT(player.GetTeam())

      // Give units to the team
      if (Player *newPlayer = Player::Id2Player(d->giveTo))
      {
        ASSERT(newPlayer->GetTeam())

        if (&player != newPlayer)
        {
          U32 amount = player.GetTeam()->GiveMoney(newPlayer->GetTeam(), d->amount);

          // Trigger game message?
          if (amount)
          {
            if (player.GetTeam() == Team::GetDisplayTeam())
            {
              Message::TriggerGameMessage(0xB4223958, 2, newPlayer->GetDisplayName(), amount); // "Team::GaveCredits"
            }
            else if (newPlayer->GetTeam() == Team::GetDisplayTeam())
            {
              Message::TriggerGameMessage(0x8AB24AD7, 2, player.GetDisplayName(), amount); // "Team::GivenCredits"
            }
          }
        }
      }

      return (sizeof (Data));
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class PlayerMarker - Move or clear player marker position
    //
    U32 PlayerMarker::orderId;


    //
    // Generate - to clear use cx=S32_MAX cz=S32_MAX
    //
    void PlayerMarker::Generate(Player &player, S32 cx, S32 cz)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      data.cx = cx;
      data.cz = cz;

      Add(data, sizeof (Data), player.IsRoute());
    }


    //
    // Execute
    //
    U32 PlayerMarker::Execute(const U8 *data, Player &player)
    {
      const Data *d = (Data *) data;

      if (d->cx == S32_MAX && d->cz == S32_MAX)
      {
        // Remove marker
        Client::Display::ClearPlayerMarker(&player);

        // Signal team radio
        player.GetTeam()->GetRadio().Trigger(0x891ED61C, Radio::Event()); // "ClearMarker"
      }
      else
      {
        // Set marker
        Client::Display::SetPlayerMarker(&player, d->cx, d->cz);

        // Location
        Vector location;
        location.x = WorldCtrl::CellToMetresX(d->cx);
        location.y = 0.0f;
        location.z = WorldCtrl::CellToMetresZ(d->cz);

        // Signal team radio
        player.GetTeam()->GetRadio().Trigger(0x0923F630, Radio::Event(location)); // "SetMarker"
      }

      return (sizeof (Data));
    }
  }
}