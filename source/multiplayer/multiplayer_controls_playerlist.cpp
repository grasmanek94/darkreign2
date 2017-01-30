///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
// 1-JUL-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "multiplayer_controls_playerlist.h"
#include "multiplayer_private.h"
#include "multiplayer_data.h"
#include "multiplayer_host.h"
#include "multiplayer_network_private.h"
#include "multiplayer_cmd_private.h"
#include "multiplayer_transfer.h"
#include "multiplayer_pingdisplay.h"
#include "iface.h"
#include "iface_types.h"
#include "iface_util.h"
#include "icstatic.h"
#include "font.h"
#include "stdload.h"
#include "sides.h"
#include "babel.h"
#include "missions.h"
#include "difficulty.h"

#include "unitobj.h"
#include "worldctrl.h"
#include "team.h"
#include "mods.h"
#include "input.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Controls
  //
  namespace Controls
  {
    
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class PlayerList
    //


    // List of all PlayerList controls
    NList<PlayerList> PlayerList::allControls(&node);


    //
    // PlayerList::PlayerList
    //
    // Constructor
    //
    PlayerList::PlayerList(IControl *parent)
    : ICListBox(parent),
      offsetName(20),
      offsetSide(0),
      offsetDifficulty(180),
      offsetPing(120),
      widthPing(32),
      widthTeam(100),
      widthName(100),
      offsetLaunchReady(180),
      offsetHaveMission(200),
      teamItemCfg(NULL),
      playerItemCfg(NULL),
      aiPlayerItemCfg(NULL),
      groupItemCfg(NULL),
      dirty(TRUE),
      basic(FALSE)
    {
      // No selection for this list
      //listBoxStyle |= STYLE_NOSELECTION;
      allControls.Append(this);
    }


    //
    // PlayerList::~PlayerList
    //
    // Destructor
    //
    PlayerList::~PlayerList()
    {
      allControls.Unlink(this);

      if (teamItemCfg)
      {
        delete teamItemCfg;
      }
      if (playerItemCfg)
      {
        delete playerItemCfg;
      }
      if (aiPlayerItemCfg)
      {
        delete aiPlayerItemCfg;
      }
      if (groupItemCfg)
      {
        delete groupItemCfg;
      }

    }


    //
    // PlayerList::HandleEvent
    //
    // Pass any events to the registered handler
    //
    U32 PlayerList::HandleEvent(Event &e)
    {
      if (e.type == IFace::EventID())
      {
        switch (e.subType)
        {
          case IFace::DISPLAYMODECHANGED:
          {
            // Reactivate child controls
            dirty = TRUE;
            break;
          }
        }
      }
      return (ICListBox::HandleEvent(e));  
    }


    //
    // PlayerList::DrawSelf
    //
    // Redraw self
    //
    void PlayerList::DrawSelf(PaintInfo &pi)
    {
      if (dirty)
      {
        Rebuild *rebuild = PreRebuild();

        // Rebuild the list before redrawing
        DeleteAllItems();

        // Figure out which teams players are on and which groups the teams are in
        BinTree<GroupTree> groups;
        BuildPlayerHierachy(groups);

        Bool first = TRUE;
        IControlName ident;

        U32 numGroups = 0;
        U32 numTeams = 0;
        U32 numPlayers = 0;
        U32 numAI = 0;

        // Now we have all the data, add it to the list box
        for (BinTree<GroupTree>::Iterator gti(&groups); *gti; gti++)
        {
          numGroups++;

          // The first group doesn't get a seperator
          if (first)
          {
            first = FALSE;
          }
          else
          {
            // Group[id]
            Utils::Sprintf(ident.str, ident.GetSize(), "Group[%08X]", gti.GetKey());
            ident.Update();

            // Add group seperator
            IControl *newCtrl = new GroupItem(this);
            AddItem(ident.str, newCtrl);

            if (groupItemCfg)
            {
              groupItemCfg->InitIterators();
              newCtrl->Configure(groupItemCfg);
            }
          }

          // Add the teams which are in this group
          for (BinTree<TeamTree>::Iterator tri(&(*gti)->teams); *tri; tri++)
          {
            // Add the team
            numTeams++;

            if (!basic)
            {
              // Group[id].Team[id]
              Utils::Sprintf(ident.str, 64, "Group[%08X].Team[%08X]", gti.GetKey(), tri.GetKey());
              ident.Update();

              // Add the team
              IControl *newCtrl = new TeamItem(*this, this, tri.GetKey(), gti.GetKey());

              if (teamItemCfg)
              {
                teamItemCfg->InitIterators();
                newCtrl->Configure(teamItemCfg);
              }
              AddItem(ident.str, newCtrl);

              // Notify control if we are the captain
              if 
              (
                // Owner of this team
                (Network::GetCurrentPlayer().GetId() == (*tri)->team->ownerId)
                ||
                // Host and this team is AI
                ((*tri)->team->ai && Cmd::isHost)
              )
              {
                SendNotify(newCtrl, PlayerListNotify::Captain);
              }

              // Get the local players team
              const Team *team;
              const PlayerInfo *playerInfo;

              if (Data::Get(&playerInfo, Network::GetCurrentPlayer().GetId()))
              {
                if 
                (              
                  Data::Get(&team, playerInfo->teamId) &&
                  Network::GetCurrentPlayer().GetId() == team->ownerId
                )
                {
                  if (team->groupId == (*tri)->team->groupId)
                  {
                    if (team->color != (*tri)->team->color)
                    {
                      // If we're in the same group, we could unally with them
                      SendNotify(newCtrl, PlayerListNotify::UnAlly);
                    }
                  }
                  else
                  {
                    // If we're in a different group, we could ally with them
                    SendNotify(newCtrl, PlayerListNotify::Ally);
                  }
                }

                // If they're not AI)
                if (!(*tri)->team->ai)
                {
                  if (playerInfo->teamId == tri.GetKey())
                  {
                    // Only show the UnCoOp button if theres more than one team
                    if ((*tri)->players.GetCount() > 1)
                    {
                      // If we're a player on the same team we could unco-op with them
                      SendNotify(newCtrl, PlayerListNotify::UnCoOp);
                    }
                  }
                  else
                  {
                    // If we're a player in another team we could co-op with them 
                    SendNotify(newCtrl, PlayerListNotify::CoOp);
                  }
                }
              }

              // If we're the host and if they're ai;
              // - add controls for each other ai team who then could ally with
              // - add unally with if the ai is in a group with more than one team
              if (Cmd::isHost && (*tri)->team->ai)
              {
                U32 group = gti.GetKey();
                U32 num = 0;

                if ((*gti)->teams.GetCount() > 1)
                {
                  // If we're in the same group, we could unally with them
                  SendNotify(newCtrl, PlayerListNotify::UnAllyWith);
                }

                for (BinTree<GroupTree>::Iterator gti(&groups); *gti; gti++)
                {
                  // If this is a different group to the ai's group
                  if (gti.GetKey() != group)
                  {
                    // Iterate the teams in that group
                    for (BinTree<TeamTree>::Iterator tri(&(*gti)->teams); *tri; tri++)
                    {
                      // Is this AI ?
                      if ((*tri)->team->ai)
                      {
                        char name[32];
                        Utils::Sprintf(name, 32, "AllyWith%d", num++);

                        // Set the team and color of the control and activate it
                        AllyWithButton *button = IFace::Find<AllyWithButton>(name, newCtrl, TRUE);

                        if (Data::Get(&team, tri.GetKey()))
                        {
                          button->color = GetTeamColor(team->color);
                        }
                        else
                        {
                          button->color = Color(U32(0), U32(0), U32(0));
                        }

                        button->group = gti.GetKey();
                        button->Activate();
                      }
                    }
                  }
                }
              }

            }

            // Add AI player control
            if ((*tri)->team->ai)
            {
              numAI++;

              // Group[id].Team[id].AIPlayer[id]
              Utils::Sprintf(ident.str, ident.GetSize(), "Group[%08X].Team[%08X].AIPlayer[%08X]", gti.GetKey(), tri.GetKey(), tri.GetKey());
              ident.Update();

              // Add the AI player
              IControl *newCtrl = new AIPlayerItem(*this, this, tri.GetKey());

              if (aiPlayerItemCfg)
              {
                aiPlayerItemCfg->InitIterators();
                newCtrl->Configure(aiPlayerItemCfg);
              }
              AddItem(ident.str, newCtrl);

              // Notify the control if we are the host
              if (Cmd::isHost)
              {
                SendNotify(newCtrl, 0xD2084290); // "MultiPlayerList::Message::Host"
              }
            }
            else
            {
              // Add the players which are on this team
              for (BinTree<const Player>::Iterator pi(&(*tri)->players); *pi; pi++)
              {
                numPlayers++;

                // Group[id].Team[id].Player[id]
                Utils::Sprintf(ident.str, ident.GetSize(), "Group[%08X].Team[%08X].Player[%08X]", gti.GetKey(), tri.GetKey(), pi.GetKey());
                ident.Update();

                // Add the players
                IControl *newCtrl = new PlayerItem(*this, this, pi.GetKey(), tri.GetKey());

                if (playerItemCfg)
                {
                  playerItemCfg->InitIterators();
                  newCtrl->Configure(playerItemCfg);
                }
                AddItem(ident.str, newCtrl);

                // Notify the control if this is us
                if (pi.GetKey() == Network::GetCurrentPlayer().GetId())
                {
                  SendNotify(newCtrl, 0x553F5A14); // "MultiPlayerList::Message::Me"
                }
                else
                {
                  // If its not us and we're the host then they're kickable
                  if (Cmd::isHost)
                  {
                    SendNotify(newCtrl, PlayerListNotify::Kickable);
                  }

                  // If this player doesn't have the map and we do, we could send it to them
                  const PlayerInfo *newPlayerInfo;

                  if (Data::Get(&newPlayerInfo, pi.GetKey()))
                  {
                    if (!newPlayerInfo->haveMission && PrivData::haveMission)
                    {
                      SendNotify(newCtrl, PlayerListNotify::SendMap);
                    }
                  }
                }
              }
            }
          }
        }

        // Cleanup
        groups.DisposeAll();
        PostRebuild(&rebuild);
        dirty = FALSE;

        // Update the players window
        if (PrivData::playersCtrl.Alive())
        {
          PrivData::playersCtrl->SetTextString(TRANSLATE(("#multiplayer.config.players.title", 4, numGroups, numTeams, numPlayers, numAI)), TRUE);
        }
      }

      ICListBox::DrawSelf(pi);
    }


    //
    // PlayerList::Setup
    //
    void PlayerList::Setup(FScope *fScope)
    {
      switch (fScope->NameCrc())
      {
        case 0x44349763: // "IconLaunchReadyOff"
          IFace::FScopeToTextureInfo(fScope, iconLaunchReady[0]);
          break;

        case 0x780F14EF: // "IconLaunchReadyOn"
          IFace::FScopeToTextureInfo(fScope, iconLaunchReady[1]);
          break;

        case 0xD34E076D: // "IconHaveMissionOff"
          IFace::FScopeToTextureInfo(fScope, iconHaveMission[0]);
          break;

        case 0xD8694129: // "IconHaveMissionOn"
          IFace::FScopeToTextureInfo(fScope, iconHaveMission[1]);
          break;

        case 0xD6ABAE33: // "OffsetName"
          offsetName = StdLoad::TypeU32(fScope);
          break;

        case 0x2E592B9E: // "OffsetSide"
          offsetSide = StdLoad::TypeU32(fScope);
          break;

        case 0x6D167FC3: // "OffsetPing"
          offsetPing = StdLoad::TypeU32(fScope);
          break;

        case 0xF4CB2C55: // "WidthPing"
          widthPing = StdLoad::TypeU32(fScope);
          break;

        case 0x13945D8A: // "WidthTeam"
          widthTeam = StdLoad::TypeU32(fScope);
          break;

        case 0x4F76FDA5: // "WidthName"
          widthName = StdLoad::TypeU32(fScope);
          break;

        case 0x962837AD: // "OffsetDifficulty"
          offsetDifficulty = StdLoad::TypeU32(fScope);
          break;

        case 0xAF52591A: // "OffsetLaunchReady"
          offsetLaunchReady = StdLoad::TypeU32(fScope);
          break;

        case 0x7BE8FD77: // "OffsetHaveMission"
          offsetHaveMission = StdLoad::TypeU32(fScope);
          break;

        case 0x9C2B9399: // "TeamItemConfig"
          if (!teamItemCfg)
          {
            teamItemCfg = fScope->Dup();
          }
          break;

        case 0x3DF1C886: // "PlayerItemConfig"
          if (!playerItemCfg)
          {
            playerItemCfg = fScope->Dup();
          }
          break;

        case 0xBC2DB202: // "AIPlayerItemConfig"
          if (!aiPlayerItemCfg)
          {
            aiPlayerItemCfg = fScope->Dup();
          }
          break;

        case 0xD4A203F3: // "GroupItemConfig"
          if (!groupItemCfg)
          {
            groupItemCfg = fScope->Dup();
          }
          break;

        case 0x024271E8: // "Basic"
          basic = StdLoad::TypeU32(fScope, Range<U32>::flag);
          break;

        default:
          ICListBox::Setup(fScope);
          break;


      }
    }


    //
    // PlayerList::Dirty
    //
    // Marks all PlayerList controls as dirty
    //
    void PlayerList::Dirty()
    {
      for (NList<PlayerList>::Iterator i(&allControls); *i; i++)
      {
        (*i)->dirty = TRUE;
      }
    }



    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class PlayerList::PlayerItem
    //


    //
    // Constructor
    //
    PlayerList::PlayerItem::PlayerItem(PlayerList &playerList, IControl *parent, U32 networkId, U32 teamId) 
    : playerList(playerList),
      ICStatic(parent),
      networkId(networkId),
      teamId(teamId),
      tipReady(NULL),
      tipNotReady(NULL),
      tipHasMap(NULL),
      tipDoesntHaveMap(NULL)
    { 
      controlStyle &= ~STYLE_INERT;
    }


    //
    // Destructor
    //
    PlayerList::PlayerItem::~PlayerItem()
    {
      if (tipReady)
      {
        delete tipReady;
      }
      if (tipNotReady)
      {
        delete tipNotReady;
      }
      if (tipHasMap)
      {
        delete tipHasMap;
      }
      if (tipDoesntHaveMap)
      {
        delete tipDoesntHaveMap;
      }
    }


    //
    // PlayerList::PlayerItem::DrawSelf
    //
    // DrawSelf
    //
    void PlayerList::PlayerItem::DrawSelf(PaintInfo &pi)
    {
      // Retrieve the team information for this team
      const Team *team;
      if (Data::Get(&team, teamId))
      {
        if (Cmd::fillInterface)
        {
          // Fill the background
          IFace::RenderRectangle
          (
            pi.client, 
            GetPlayerBgColor(team->color),
            GetTexture(),
            pi.alphaScale
          );
        }

        if (pi.font)
        {
          // Is there a network player with this id ?
          Network::Player *networkPlayer;
          networkPlayer = Network::GetPlayers().Find(networkId);

          if (networkPlayer)
          {
            // Display the name of the player

            // Add the player name
            const CH *ch;
            U32 yoffs = (pi.client.Height() - pi.font->Height()) / 2;

            // Add Game host
            ClipRect nameClip
            (
              pi.client.p0.x + playerList.offsetName,
              pi.client.p0.y,
              pi.client.p0.x + playerList.offsetName + playerList.widthName,
              pi.client.p1.y
            );

            // Add Player name
            ch = Utils::Ansi2Unicode(networkPlayer->GetName());
            pi.font->Draw(
              pi.client.p0.x + playerList.offsetName,
              pi.client.p0.y + yoffs, 
              ch, 
              Utils::Strlen(ch),
              pi.colors->fg[ColorIndex()],
              &nameClip
            );

            // Is there player information ?
            const PlayerInfo *playerInfo;

            if (Data::Get(&playerInfo, networkId))
            {
              TextureInfo *icon = &playerList.iconLaunchReady[playerInfo->launchReady];
              S32 midY = (pi.client.p0.y + pi.client.p1.y) >> 1;

              if (!playerList.basic)
              {
                if (icon->texture)
                {
                  // Add launch ready icon
                  IFace::RenderRectangle
                  (
                    ClipRect(
                      pi.client.p0.x + playerList.offsetLaunchReady, 
                      midY - (icon->pixels.Height() >> 1),
                      pi.client.p0.x + playerList.offsetLaunchReady + icon->pixels.Width(),
                      midY - (icon->pixels.Height() >> 1) + icon->pixels.Height()),
                    Color(1.0f, 1.0f, 1.0f), 
                    icon,
                    pi.alphaScale
                  );
                }

                // Is there a map selected ?

                const Mission *mission;
                if (Data::Get(&mission))
                {
                  if (mission->mission != Settings::Mission::NoMission)
                  {
                    icon = &playerList.iconHaveMission[playerInfo->haveMission];

                    if (Cmd::online)
                    {
                      if (icon->texture)
                      {
                        // Add the have map icon
                        IFace::RenderRectangle
                        (
                          ClipRect(
                            pi.client.p0.x + playerList.offsetHaveMission, 
                            midY - (icon->pixels.Height() >> 1),
                            pi.client.p0.x + playerList.offsetHaveMission + icon->pixels.Width(),
                            midY - (icon->pixels.Height() >> 1) + icon->pixels.Height()),
                          Color(1.0f, 1.0f, 1.0f), 
                          icon,
                          pi.alphaScale
                        );
                      }
                    }
                  }
                }
              }

              if (Network::client)
              {
                const StyxNet::Std::Data::UserConnection *userConnection = Network::client->GetUserConnection(networkId);
                if (userConnection)
                {
                  PingDisplay::Draw
                  (
                    StyxNet::Std::Data::maxPings,
                    userConnection->pings,
                    ClipRect
                    (
                      pi.client.p0.x + playerList.offsetPing,
                      pi.client.p0.y + 1,
                      pi.client.p0.x + playerList.offsetPing + playerList.widthPing,
                      pi.client.p1.y - 2
                    ),
                    pi.alphaScale
                  );
                }
             
                /*
                // Add Ping 
                CH buf[30];
                ch = buf;
                Utils::Sprintf(buf, 30, L"%d", playerPing->pingSmooth);
                pi.font->Draw(
                  pi.client.p0.x + playerList.offsetPing + 100,
                  pi.client.p0.y + yoffs, 
                  ch, 
                  Utils::Strlen(ch), 
                  pi.colors->fg[ColorIndex()],
                  &pi.client
                );
                */
              }
            }
          }
        }
      }
    }


    //
    // PlayerList::PlayerItem::HandleEvent
    //
    U32 PlayerList::PlayerItem::HandleEvent(Event &e)
    {
      if (e.type == IFace::EventID())
      {
        switch (e.subType)
        {
          case IFace::NOTIFY:
          {
            // Do specific handling
            switch (e.iface.p1)
            {
              case PlayerListMsg::Player::Kick:
              {
                Network::Player *p = Network::GetPlayers().Find(networkId);

                if (p)
                {
                  char buf[200];
                  Utils::Sprintf(buf, 200, "kicked '%s' from the game", p->GetName());
                  Data::Send(Commands::MessageQuote, Utils::Strlen(buf) + 1, (const U8 *) buf, FALSE);
                  Network::client->KickUser(networkId);
                }
                break;
              }

              case PlayerListMsg::Player::SendMap:
              {
                if (PrivData::haveMission)
                {
                  const Mission *mission;
                  if (Data::Get(&mission))
                  {
                    if (const Missions::Mission *info = Missions::FindMission(mission->mission, mission->missionFolder))
                    {
                      if (const char *name = info->GetPackedFileName())
                      {
                        // Make an offer on the current mission
                        Transfer::MakeOffer(networkId, Transfer::Type::Mission, name);
                        break;
                      }
                    }
                  }
                }

                // You don't have the mission or the mission is not transportable
                SendNotify(this, 0xC52B9C66); // "MultiPlayerList::Message::CantSendMap"
                break;
              }
            }
            break;
          }

          case IFace::DISPLAYTIP:
          {
            const PlayerInfo *playerInfo;
            if (Data::Get(&playerInfo, networkId))
            {
              Point<S32> mouse = Input::MousePos();
              mouse = ScreenToClient(mouse);

              // Is the mouse in the map area ?
              const Mission *mission;
              if (Data::Get(&mission))
              {
                if (mission->mission != Settings::Mission::NoMission)
                {
                  TextureInfo *icon = &playerList.iconHaveMission[playerInfo->haveMission];
                  if 
                  (
                    icon->texture &&
                    mouse.x >= playerList.offsetHaveMission &&
                    mouse.x < playerList.offsetHaveMission + icon->pixels.Width()
                  )
                  {
                    ActivateTip(playerInfo->haveMission ? tipHasMap : tipDoesntHaveMap);
                    break;
                  }
                }
              }

              // Is the mouse in the ready to go area ?
              TextureInfo *icon = &playerList.iconLaunchReady[playerInfo->launchReady];
              if 
              (
                icon->texture &&
                mouse.x >= playerList.offsetLaunchReady &&
                mouse.x < playerList.offsetLaunchReady + icon->pixels.Width()
              )
              {
                ActivateTip(playerInfo->launchReady ? tipReady : tipNotReady);
              }
            }
            break;
          }
        }
      }

      return (ICStatic::HandleEvent(e));  
    }


    //
    // Setup
    //
    void PlayerList::PlayerItem::Setup(FScope *fScope)
    {
      switch (fScope->NameCrc())
      {
        case 0xFBEE7B81: // "TipReady"
          tipReady = Utils::Strdup(TRANSLATE((StdLoad::TypeString(fScope))));
          break;

        case 0xE8FCFCAA: // "TipNotReady"
          tipNotReady = Utils::Strdup(TRANSLATE((StdLoad::TypeString(fScope))));
          break;

        case 0xBA64C7C4: // "TipHasMap"
          tipHasMap = Utils::Strdup(TRANSLATE((StdLoad::TypeString(fScope))));
          break;

        case 0xEA4CA09A: // "TipDoesntHaveMap"
          tipDoesntHaveMap = Utils::Strdup(TRANSLATE((StdLoad::TypeString(fScope))));
          break;

        default:
          ICStatic::Setup(fScope);
      }
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class PlayerList::AIPlayerItem
    //

    //
    // PlayerList::AIPlayerItem::DrawSelf
    //
    void PlayerList::AIPlayerItem::DrawSelf(PaintInfo &pi)
    {
      // Retrieve the team information for this team
      const Team *team;
      if (Data::Get(&team, teamId))
      {
        if (Cmd::fillInterface)
        {
          // Fill the background
          IFace::RenderRectangle
          (
            pi.client, 
            GetPlayerBgColor(team->color),
            GetTexture(),
            pi.alphaScale
          );
        }

        if (pi.font)
        {
          const CH *ch;

          if (team->personality == 0xB8586215) // "Random"
          {
            ch = TRANSLATE(("#mods.personality.random"));
          }
          else
          {
            Mods::Mod *mod = Mods::GetMod(Mods::Types::Personality, team->personality);
            if (mod)
            {
              ch = TRANSLATE((mod->GetDescription().str));
            }
            else
            {
              ch = Utils::Ansi2Unicode("???");
            }
          }

          U32 yoffs = (pi.client.Height() - pi.font->Height()) / 2;

          // Add Player name
          pi.font->Draw(
            pi.client.p0.x + playerList.offsetName,
            pi.client.p0.y + yoffs, 
            ch, 
            Utils::Strlen(ch),
            pi.colors->fg[ColorIndex()],
            &pi.client
          );
        }
      }
    }


    //
    // PlayerList::AIPlayerItem::HandleEvent
    //
    U32 PlayerList::AIPlayerItem::HandleEvent(Event &e)
    {
      if (e.type == IFace::EventID())
      {
        switch (e.subType)
        {
          case IFace::NOTIFY:
          {
            // Do specific handling
            switch (e.iface.p1)
            {
              case PlayerListMsg::AI::Delete:
              {
                Host::DeleteAITeam(teamId);
                break;
              }
            }
            break;
          }
        }
      }

      return (ICStatic::HandleEvent(e));  
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class PlayerList::TeamItem
    //


    //
    // PlayerList::TeamItem::DrawSelf
    //
    // DrawSelf
    //
    void PlayerList::TeamItem::DrawSelf(PaintInfo &pi)
    {
      // Retrieve the team information for this team
      const Team *team;
      if (Data::Get(&team, teamId))
      {
        if (Cmd::fillInterface)
        {
          // Fill the background
          IFace::RenderRectangle
          (
            ClipRect
            (
              pi.client.p0.x, 
              pi.client.p0.y, 
              pi.client.p1.x, 
              pi.client.p1.y
            ), 
            GetTeamBgColor(team->color),
            GetTexture(),
            pi.alphaScale
          );
        }
        else
        {
          // Fill the background
          IFace::RenderRectangle
          (
            ClipRect
            (
              pi.client.p0.x, 
              pi.client.p0.y, 
              pi.client.p0.x + playerList.widthTeam, 
              pi.client.p1.y
            ), 
            GetTeamBgColor(team->color),
            GetTexture(),
            pi.alphaScale
          );
        }

        // Add side group
        const CH *ch;
        U32 yoffs = (pi.client.Height() - pi.font->Height()) / 2;
        Sides::Side *side;

        if ((side = Sides::GetSides().Find(team->side.crc)) != NULL)
        {
          ch = TRANSLATE((side->GetDescription().str));
          pi.font->Draw(
            pi.client.p0.x + playerList.offsetSide,
            pi.client.p0.y + yoffs, 
            ch, 
            Utils::Strlen(ch),
            pi.colors->fg[ColorIndex()],
            &pi.client
          );
        }
        else
        {
          if (team->side.crc == 0xB8586215) // "Random"
          {
            ch = TRANSLATE(("#game.sides.random"));
            pi.font->Draw(
              pi.client.p0.x + playerList.offsetSide,
              pi.client.p0.y + yoffs, 
              ch, 
              Utils::Strlen(ch),
              pi.colors->fg[ColorIndex()],
              &pi.client
            );
          }
        }

        const Options *options;
        if (Data::Get(&options))
        {
          if (options->difficulty == Settings::Options::AllowDifficulty)
          {
            // Add difficulty
            Difficulty::Setting *setting = Difficulty::FindSetting(team->difficulty);
            if (setting)
            {
              ch = TRANSLATE((setting->GetDescription().str));
              pi.font->Draw(
                pi.client.p0.x + playerList.offsetDifficulty,
                pi.client.p0.y + yoffs, 
                ch, 
                Utils::Strlen(ch),
                pi.colors->fg[IControl::STATE_DISABLED],
                &pi.client
              );
            }
          }
        }
      }
    }


    //
    // PlayerList::TeamItem::HandleEvent
    //
    U32 PlayerList::TeamItem::HandleEvent(Event &e)
    {
      if (e.type == IFace::EventID())
      {
        switch (e.subType)
        {
          case IFace::NOTIFY:
          {
            // Do specific handling
            switch (e.iface.p1)
            {
              case PlayerListMsg::Team::Ally:
              {
                // Send a command to ally
                Commands::Data::JoinGroup data;
                data.groupId = GetGroupId();
                Data::Send(Commands::JoinGroup, data, TRUE);
                return (TRUE);
              }

              case PlayerListMsg::Team::UnAlly:
              {
                // Leave the group
                Data::Send(Commands::LeaveGroup, 0, NULL, TRUE);
                return (TRUE);
              }

              case PlayerListMsg::Team::CoOp:
              {
                // Send a command to coop
                Commands::Data::JoinTeam data;
                data.teamId = GetTeamId();
                Data::Send(Commands::JoinTeam, data, TRUE);
                return (TRUE);
              }

              case PlayerListMsg::Team::UnCoOp:
              {
                // Send a command to uncoop
                Data::Send(Commands::LeaveTeam, 0, NULL, TRUE);
                return (TRUE);
              }

              case PlayerListMsg::Team::AllyWith:
              {
                AllyWithButton *button = IFace::Promote<AllyWithButton>(static_cast<IControl *>(e.iface.from), TRUE);

                Commands::Data::JoinGroupAI data;
                data.teamId = GetTeamId();
                data.groupId = button->group;
                Data::Send(Commands::JoinGroupAI, data, TRUE);
                return (TRUE);
              }

              case PlayerListMsg::Team::UnAllyWith:
              {
                // Leave the group
                Commands::Data::LeaveGroupAI data;
                data.teamId = GetTeamId();
                Data::Send(Commands::LeaveGroupAI, data, TRUE);
                return (TRUE);
              }
            }
            break;
          }
        }
      }

      return (ICStatic::HandleEvent(e));  
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class PlayerList::GroupItem
    //


    //
    // PlayerList::GroupItem::DrawSelf
    //
    // DrawSelf
    //
    void PlayerList::GroupItem::DrawSelf(PaintInfo &pi)
    {
      ClipRect rect(pi.client);
      rect.p0.x += 2;
      rect.p0.y = (pi.client.p1.y + pi.client.p0.y) / 2;
      rect.p1.x -= 2;
      rect.p1.y = rect.p0.y + 1;

      // Draw a filled rectangle
      IFace::RenderRectangle(rect, pi.colors->fg[ColorIndex()]);

    }


    //
    // PlayerList::GroupItem::AutoSize
    //
    // Automatically resize geometry
    //
    void PlayerList::GroupItem::AutoSize()
    {
      size.x = 0;
      size.y = 2;
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class PlayerList::AllyWithButton
    //

    //
    // Constructor
    //
    PlayerList::AllyWithButton::AllyWithButton(IControl *parent) 
    : ICButton(parent)
    { 
    }


    //
    // Redraw self
    //
    void PlayerList::AllyWithButton::DrawSelf(PaintInfo &pi)
    {
      IFace::RenderGradient(pi.client, color, 150);
    }

  }

}
