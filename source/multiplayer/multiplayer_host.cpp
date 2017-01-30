///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Host Stuff
// 1-JUL-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "multiplayer_private.h"
#include "multiplayer_host.h"
#include "multiplayer_data.h"
#include "multiplayer_cmd_private.h"
#include "multiplayer_network.h"
#include "gameconstants.h"
#include "console.h"
#include "difficulty.h"

#include "unitobj.h"
#include "mapobj.h"
#include "game.h"
#include "team.h"
#include "sides.h"
#include "babel.h"
#include "unitlimits.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Host
  //
  namespace Host
  {

    // Challenges sent to players
    BinTree<U32> challenges;

    // Player info
    BinTree<PlayerInfo> players;

    // Team info
    BinTree<Team> teams;

    // The current mission
    Mission mission;

    // The options
    Options options;


    //
    // Reset
    //
    void Reset()
    {
      challenges.DisposeAll();
      players.DisposeAll();
      teams.DisposeAll();
    }


    //
    // Hosted
    //
    void Hosted()
    {
      // Create team data
      for (U32 i = 0; i < Game::MAX_TEAMS; i++)
      {
        ResetTeam(i);
      }

      // Create mission data
      mission.missionFolder = 0;
      mission.mission = Settings::Mission::NoMission;
      mission.crc = 0;

      Data::Store(mission);

      // Create options data
      options.launch = FALSE;
      options.placement = Settings::Options::Random;
      options.difficulty = Settings::Options::DenyDifficulty;
      options.randomSeed = Clock::Time::UsLwr();
      options.ruleSet = Settings::Options::UseMissionRules;
      options.startCredits = 0;
      options.unitLimit = UnitLimits::GetLimit();
      options.resourceType = Settings::Options::ResourceTypeNormal;

      Data::Store(options);

      // Create default property filter data
      PropertyFilter propertyFilter;
      propertyFilter.numFilters = 0;
      Utils::Memset(propertyFilter.filters, 0x00, maxFilters * sizeof (CRC));

      Data::Store(propertyFilter);
    }


    //
    // EnterPlayer
    //
    void EnterPlayer(U32 id)
    {
      // Are we the host ?
      if (!Cmd::isHost)
      {
        return;
      }

      LOG_DIAG(("Player [%08X] Entered", id))

      // Are we launch ?
      if (options.launch)
      {
        LOG_DIAG(("We're launched, kicking new player out"))
        ASSERT(Network::client)
        Network::client->KickUser(id);
        return;
      }

      // We only create the player information if the integrity check is completed!

      // This is a new player
      PlayerInfo *playerInfo = new PlayerInfo;
      playerInfo->teamId = FindEmptyTeam(id);
      playerInfo->launchReady = FALSE;
      playerInfo->haveMission = FALSE;
      playerInfo->correctVersion = FALSE;

      players.Add(id, playerInfo);

      // Store the information about the player
      Data::Store(*playerInfo, id);

      // Issue a challenge to this player
      U32 *challenge;
      challenge = challenges.Find(id);
      if (!challenge)
      {
        challenge = new U32;
      }
      *challenge = Clock::Time::Ms() ^ Clock::Time::UsLwr();
      challenges.Add(id, challenge);

      Commands::Data::IntegrityChallenge integrityChallenge;
      integrityChallenge.crc = *challenge;
      Data::Send(1, &id, Commands::IntegrityChallenge, integrityChallenge);

    }


    //
    // ExitPlayer
    //
    void ExitPlayer(U32 id)
    {
      // Are we the host ?
      if (!Cmd::isHost)
      {
        return;
      }

      LOG_DIAG(("Player [%08X] Exited", id))

      // Get the player info
      PlayerInfo *pi = players.Find(id);

      if (pi)
      {
        // How many players are on this team
        const U32 *teamCounts = GetTeamCounts();

        // Is there only one player on this players team ?
        if (teamCounts[pi->teamId] == 1)
        {
          // Nope, reset this team
          ResetTeam(pi->teamId);
        }
        else
        {
          // Is this player the owner of their current team ?
          Team *team = teams.Find(pi->teamId);
          ASSERT(team)

          // Is this player the team leader ?
          if (team->ownerId == id)
          {
            // This is the team owner, find another team owner
            for (BinTree<PlayerInfo>::Iterator p(&players); *p; p++)
            {
              if ((p.GetKey() != id) && ((*p)->teamId == pi->teamId))
              {
                team->ownerId = p.GetKey();

                // Store the new team info
                Data::Store(*team, pi->teamId);
                break;
              }
            }
          }
        }

        // Dispose of the local copy of the player data
        players.Dispose(id);

        // Remove the player data from the router
        Data::Clear(Player::key, id);

        // Remove the playerinfo data from the router
        Data::Clear(PlayerInfo::key, id);

      }
      else
      {
        LOG_DIAG(("Player [%08X] could not be resolved", id))
      }
    }


    //
    // Interpret a command
    //
    void Handler(CRC from, CRC key, U32 size, const U8 *data)
    {
      // Are we the host ?
      if (!Cmd::isHost)
      {
        return;
      }

      // Are launch ?
      if (options.launch)
      {
        return;
      }

      // Get the player info for this player
      PlayerInfo *pi = players.Find(from);

      // We should have player info (why wouldn't we?)
      if (!pi)
      {
        return;
      }

      switch (key)
      {
        case Commands::IntegrityResponse:
        {
          LOG_DIAG(("Integrity response from [%08X]", from))
          CAST(const Commands::Data::IntegrityResponse *, response, data)

          U32 *challenge = challenges.Find(from);

          if (challenge)
          {
            Network::Player *player = Network::GetPlayers().Find(from);

            if (response->crc == Debug::Memory::GetCodeIntegrity(*challenge))
            {
              LOG_DIAG(("%s has the same build", player ? player->GetName() : "???"))
              // Utils::Sprintf(buf, 200, "has verified the integrity of '%s'", player ? player->GetName() : "???");

              // Set the same version status to TRUE
              pi->correctVersion = TRUE;

              // Store the information about the player
              Data::Store(*pi, from);
            }
            else
            {
              char buf[200];

              LOG_DIAG(("%s has a different build", player ? player->GetName() : "???"))
              Utils::Sprintf(buf, 200, "notes that '%s' had a different version", player ? player->GetName() : "???");
              Data::Send(Commands::MessageQuote, Utils::Strlen(buf) + 1, (const U8 *) buf, FALSE);
            }

            // Remove the challenge
            challenges.Dispose(from);
          }
          else
          {
            LOG_DIAG(("Got a response from [%08X] who hasn't been challenged!"))
          }
         
          break;
        }

        case Commands::Launch:
        {
          LOG_DIAG(("Launch from [%08X]", from))

          ASSERT(!size)

          if (from == Network::GetCurrentPlayer().GetId())
          {
            // Check to see if we can launch at the moment
            if (CheckLaunch())
            {
              options.launch = TRUE;
              Data::Store(options);

              if (Network::client)
              {
                Network::client->LockSession();
              }
            }
            else
            {
              // Clear our launch readyness
              pi->launchReady = FALSE;
              Data::Store(*pi, from);
            }
          }
          break;
        }

        case Commands::LaunchReady:
        {
          LOG_DIAG(("LaunchReady from [%08X]", from))

          ASSERT(!size)

          // Set the launch readyness status of the player
          pi->launchReady = TRUE;

          // Store the information about the player
          Data::Store(*pi, from);
          break;
        }

        case Commands::HaveMission:
        {
          LOG_DIAG(("HaveMission from [%08X]", from))

          ASSERT(!size)

          // Set the have mission status of the player
          pi->haveMission = TRUE;

          // Store the information about the player
          Data::Store(*pi, from);
          break;
        }

        case Commands::JoinTeam:
        {
          LOG_DIAG(("JoinTeam from [%08X]", from))

          ASSERT(size == sizeof (U32))

          InvalidateLaunchReadyness();

          // Get the team id from the command
          U32 teamId = *(U32 *) data;
          ASSERT(teamId < Game::MAX_TEAMS)

          Team *newTeam = teams.Find(teamId);
          ASSERT(newTeam)

          // Can't join an AI team
          if (newTeam->ai)
          {
            LOG_DIAG(("Can't join AI teams!"))
            break;
          }

          // Is this player the owner of their current team ?
          Team *team = teams.Find(pi->teamId);
          ASSERT(team)

          if (team->ownerId == from)
          {
            // If this player is the owner of the team they are currently on, find another owner
            for (BinTree<PlayerInfo>::Iterator p(&players); *p; p++)
            {
              if ((p.GetKey() != from) && ((*p)->teamId == pi->teamId))
              {
                team->ownerId = p.GetKey();

                // Store the new team info
                Data::Store(*team, pi->teamId);
                break;
              }
            }
          }

          // Set the team for this player
          pi->teamId = teamId;

          // Store the information about the player
          Data::Store(*pi, from);

          // Check to see if there's any other players on this team
          const U32 *teamCounts = GetTeamCounts();

          if (teamCounts[teamId] == 1)
          {
            // This player is now the owner of this team, update
            Team *team = teams.Find(teamId);
            ASSERT(team)

            team->ownerId = from;

            // Store the new team info
            Data::Store(*team, teamId);
          }
          break;
        }

        case Commands::LeaveTeam:
        {
          LOG_DIAG(("LeaveTeam from [%08X]", from))

          ASSERT(size == 0)

          InvalidateLaunchReadyness();

          // Is this player the owner of their current team ?
          Team *team = teams.Find(pi->teamId);
          ASSERT(team)

          // Can't leave an AI team
          if (team->ai)
          {
            break;
          }

          if (team->ownerId == from)
          {
            // This is the team owner, make all the other players on this team find another team
            for (BinTree<PlayerInfo>::Iterator p(&players); *p; p++)
            {
              if ((p.GetKey() != from) && ((*p)->teamId == pi->teamId))
              {
                // Assign them a new team
                (*p)->teamId = FindEmptyTeam(p.GetKey());

                // Store the information about the player
                Data::Store(**p, p.GetKey());
                break;
              }
            }
          }
          else
          {
            // Find an empty team
            pi->teamId = FindEmptyTeam(from);

            // Store the information about the player
            Data::Store(*pi, from);
          }
          break;
        }

        case Commands::SetTeamColor:
        {
          LOG_DIAG(("SetTeamColor from [%08X]", from))

          ASSERT(size == sizeof(Commands::Data::SetTeamColor))

          Commands::Data::SetTeamColor *cmdData = (Commands::Data::SetTeamColor *)data;
          ASSERT(cmdData->color <= Game::MAX_TEAMS)

          InvalidateLaunchReadyness();

          // What team is this player on ?
          const Team *fromTeam = teams.Find(cmdData->teamId);
          ASSERT(fromTeam)

          // Is this player the owner of that team ?
          if (fromTeam->ownerId == from)
          {
            Team *team = teams.Find(cmdData->teamId);
            if (team)
            {
              const Bool *colors = GetColorUsage();

              // Is that color free ?
              if (!colors[cmdData->color])
              {
                // Its free, set the team color
                team->color = cmdData->color;

                // Store the new team info
                Data::Store(*team, cmdData->teamId);
              }
              else
              {
                if (team->color != cmdData->color)
                {
                  LOG_DIAG(("Sorry, that color is already taken"))
                }
              }
            }
          }
          else
          {
            LOG_DIAG(("NON-OWNER [%08X] attempted to change color for team %d", from, cmdData->teamId))
          }
          break;
        }

        case Commands::SetTeamSide:
        {
          LOG_DIAG(("SetTeamSide from [%08X]", from))

          ASSERT(size == sizeof(Commands::Data::SetTeamSide))

          Commands::Data::SetTeamSide *cmdData = (Commands::Data::SetTeamSide *)data;

          InvalidateLaunchReadyness();

          // What team is this player on ?
          const Team *fromTeam = teams.Find(cmdData->teamId);
          ASSERT(fromTeam)

          // Is this player the owner of that team ?
          if (fromTeam->ownerId == from)
          {
            Team *team = teams.Find(cmdData->teamId);
            if (team)
            {
              // Set the side ident
              team->side = cmdData->side;

              // Store the new team info
              Data::Store(*team, cmdData->teamId);
            }
          }
          else
          {
            LOG_DIAG(("NON-OWNER [%08X] attempted to change side group %d", from, cmdData->teamId))
          }
          break;
        }

        case Commands::SetStartLocation:
        {
          LOG_DIAG(("SetStartLocation from [%08X]", from))

          ASSERT(size == sizeof(Commands::Data::SetStartLocation))
          Commands::Data::SetStartLocation *cmdData = (Commands::Data::SetStartLocation *)data;

          InvalidateLaunchReadyness();

          // What team is this player on ?
          const Team *fromTeam = teams.Find(cmdData->teamId);
          ASSERT(fromTeam)

          // Is this player the owner of that team ?
          if (fromTeam->ownerId == from)
          {
            Team *team = teams.Find(cmdData->teamId);
            if (team)
            {
              // Go through the teams to see if another team has already claimed this spot
              Bool found = FALSE;

              // Clear the team start locations
              for (BinTree<Team>::Iterator t(&teams); *t; t++)
              {
                if ((*t)->startLocation == cmdData->startLocation)
                {
                  LOG_DIAG(("Location is taken"))
                  found = TRUE;
                  break;
                }
              }

              if (!found)
              {
                // Its free, set the team start location
                team->startLocation = cmdData->startLocation;

                // Store the new team info
                Data::Store(*team, cmdData->teamId);
              }
            }
          }
          else
          {
            LOG_DIAG(("NON-OWNER attempted to change start location"))
          }
          break;
        }

        case Commands::SetDifficulty:
        {
          LOG_DIAG(("SetDifficulty from [%08X]", from))

          ASSERT(size == sizeof(Commands::Data::SetDifficulty))
          Commands::Data::SetDifficulty *cmdData = (Commands::Data::SetDifficulty *) data;

          InvalidateLaunchReadyness();

          // What team is this player on ?
          const Team *fromTeam = teams.Find(cmdData->teamId);
          ASSERT(fromTeam)

          // Is this player the owner of that team ?
          if (fromTeam->ownerId == from)
          {
            Team *team = teams.Find(cmdData->teamId);
            if (team)
            {
              // Its free, set the team start location
              team->difficulty = cmdData->difficulty;

              // Store the new team info
              Data::Store(*team, cmdData->teamId);
            }
          }
          else
          {
            LOG_DIAG(("NON-OWNER attempted to change difficulty"))
          }
          break;
        }

        case Commands::SetPersonality:
        {
          LOG_DIAG(("SetPersonality from [%08X]", from))

          ASSERT(size == sizeof(Commands::Data::SetPersonality))
          Commands::Data::SetPersonality *cmdData = (Commands::Data::SetPersonality *) data;

          InvalidateLaunchReadyness();

          // What team is this player on ?
          const Team *fromTeam = teams.Find(cmdData->teamId);
          ASSERT(fromTeam)

          // Is this player the owner of that team ?
          if (fromTeam->ownerId == from)
          {
            Team *team = teams.Find(cmdData->teamId);
            if (team)
            {
              // Is this an AI team
              if (team->ai)
              {
                // Its free, set the team start location
                team->personality = cmdData->personality;

                // Store the new team info
                Data::Store(*team, cmdData->teamId);
              }
              else
              {
                LOG_DIAG(("Can't change the personality of a NON-AI team"))
              }
            }
          }
          else
          {
            LOG_DIAG(("NON-OWNER attempted to change personality"))
          }
          break;
        }

        case Commands::JoinGroup:
        {
          LOG_DIAG(("JoinGroup from [%08X]", from))

          ASSERT(size == sizeof (Commands::Data::JoinGroup))

          InvalidateLaunchReadyness();

          // Get the group id
          U32 group = *(U32 *) data;

          ASSERT(group < Game::MAX_TEAMS)

          // Find this players team
          Team *team = teams.Find(pi->teamId);
          ASSERT(team)

          // Is this player the owner of the team
          if (team->ownerId == from)
          {
            // Set this teams group
            team->groupId = group;

            // Store the new team info
            Data::Store(*team, pi->teamId);
          }
  
          break;
        }

        case Commands::JoinGroupAI:
        {
          LOG_DIAG(("JoinGroupAI from [%08X]", from))

          ASSERT(size == sizeof (Commands::Data::JoinGroupAI))

          InvalidateLaunchReadyness();

          // Get the team id
          U32 teamId = *(U32 *) data;

          // Get the group id
          U32 group = *(((U32 *) data) + 1);

          ASSERT(group < Game::MAX_TEAMS)

          // Find this players team
          Team *team = teams.Find(teamId);
          ASSERT(team)

          // Is this player the owner of the team
          if (team->ownerId == from)
          {
            // Set this teams group
            team->groupId = group;

            // Store the new team info
            Data::Store(*team, teamId);
          }
  
          break;
        }

        case Commands::LeaveGroup:
        {
          LOG_DIAG(("LeaveGroup from [%08X]", from))

          ASSERT(size == 0)

          InvalidateLaunchReadyness();

          // Find this players team
          Team *team = teams.Find(pi->teamId);
          ASSERT(team)

          // Is this player the owner of the team
          if (team->ownerId == from)
          {
            // Set this teams group
            team->groupId = FindEmptyGroup();

            // Store the new team info
            Data::Store(*team, pi->teamId);
          }
          break;
        }

        case Commands::LeaveGroupAI:
        {
          LOG_DIAG(("LeaveGroupAI from [%08X]", from))

          ASSERT(size == sizeof (Commands::Data::LeaveGroupAI))

          InvalidateLaunchReadyness();

          // Get the team id
          U32 teamId = *(U32 *) data;

          // Find this players team
          Team *team = teams.Find(teamId);
          ASSERT(team)

          // Is this player the owner of the team
          if (team->ownerId == from)
          {
            // Set this teams group
            team->groupId = FindEmptyGroup();

            // Store the new team info
            Data::Store(*team, teamId);
          }
          break;
        }

      }
    }


    //
    // FindEmptyTeam
    //
    U32 FindEmptyTeam(U32 playerId)
    {
      ASSERT(Cmd::isHost)

      U32 teamId = 0;
      const U32 *teamCounts = GetTeamCounts();

      // Find the team with the least people on it
      for (int t = 0; t < Game::MAX_TEAMS; t++)
      {
        // Does this team have less players than the current team ?
        if (teamCounts[t] < teamCounts[teamId])
        {
          teamId = t;
        }
      }

      // Is this player the first player on the team ?
      if (!teamCounts[teamId])
      {
        LOG_DIAG(("Formed new team %d with owner [%08X]", teamId, playerId))

        ResetTeam(teamId);
        Team *team = teams.Find(teamId);
        team->ownerId = playerId;

        // Store the information about the player
        Data::Store(*team, teamId);
      }

      LOG_DIAG(("Found unused team %d for player [%08X]", teamId, playerId))
      
      return (teamId);
    }


    //
    // FindEmptyGroup
    //
    U32 FindEmptyGroup()
    {
      ASSERT(Cmd::isHost)

      U32 groupId = 0;
      const U32 *groupCounts = GetGroupCounts();

      // Find the group with the least teams in it
      for (int g = 0; g < Game::MAX_TEAMS; g++)
      {
        // Does this group have less teams than the current group ?
        if (groupCounts[g] < groupCounts[groupId])
        {
          groupId = g;
        }
      }
      
      LOG_DIAG(("Found unused group %d", groupId))

      return (groupId);
    }


    //
    // FindUnusedColor
    //
    U32 FindUnusedColor()
    {
      ASSERT(Cmd::isHost)

      const Bool *colors = GetColorUsage();

      for (int t = 0; t <= Game::MAX_TEAMS; t++)
      {
        if (!colors[t])
        {
          LOG_DIAG(("Found unused color %d", t))
          return (t);
        }
      }

      ERR_FATAL(("Couldn't find an unused color"))
    }


    //
    // ResetTeam
    //
    // Reset a team
    //
    void ResetTeam(U32 teamId)
    {
      LOG_DIAG(("Reseting team %d", teamId))

      Team *team = teams.Find(teamId);
      Bool newTeam = FALSE;

      if (!team)
      {
        newTeam = TRUE;
        team = new Team;
      }

      team->groupId = FindEmptyGroup();
      team->ownerId = U32(-1);
      team->color = FindUnusedColor();
      team->side = "Random";
      team->difficulty = Difficulty::GetDefaultSetting().GetName().crc;
      team->startLocation = Settings::Mission::NoStartLocation;
      team->ai = FALSE;
      team->personality = 0;

      if (newTeam)
      {
        teams.Add(teamId, team);
      }
    }


    //
    // CreateAITeam
    //
    // Create a new AI team in an unused slot
    //
    Bool CreateAITeam()
    {
      if (Cmd::isHost)
      {
        const U32 *teamCounts = GetTeamCounts();

        // Find the team with the least people on it
        for (int t = 0; t < Game::MAX_TEAMS; t++)
        {
          // Does this team have less players than the current team ?
          if (teamCounts[t] == 0)
          {
            // Reset to defaults
            ResetTeam(t);

            Team *team = teams.Find(t);

            ASSERT(team)

            // Allow host to modify settings
            team->ownerId = Network::GetCurrentPlayer().GetId();

            // Setup AI defaults
            team->ai = TRUE;
            team->personality = 0xB8586215; // "Random"

            // Store the new team info
            Data::Store(*team, t);

            // Invalidate launch readyness
            InvalidateLaunchReadyness();

            return (TRUE);
          }
        }
      }
      return (FALSE);
    }


    //
    // FillAITeams
    //
    // Fill all of the reamining teams with AI
    //
    void FillAITeams()
    {
      if (Cmd::isHost)
      {
        const U32 *teamCounts = GetTeamCounts();

        // How many AI teams are required ?
        if (PrivData::preview)
        {
          U32 num = PrivData::preview->GetTeams();

          if (num)
          {
            num -= 1;
          }

          // Find the team with the least people on it
          for (int t = 0; t < Game::MAX_TEAMS; t++)
          {
            // Is this team empty or ai ?
            if (teamCounts[t] == 0 || teamCounts[t] == U32_MAX)
            {
              // Reset to defaults
              ResetTeam(t);

              Team *team = teams.Find(t);
              ASSERT(team)

              if (num)
              {
                --num;

                // Allow host to modify settings
                team->ownerId = Network::GetCurrentPlayer().GetId();

                // Setup AI defaults
                team->ai = TRUE;
                team->personality = 0xB8586215; // "Random"
              }

              // Store the new team info
              Data::Store(*team, t);
            }
          }
        }
      }
    }


    //
    // DeleteAITeam
    //
    // Delete an AI team
    //
    Bool DeleteAITeam(U32 teamId)
    {
      if (Cmd::isHost)
      {
        Team *team = teams.Find(teamId);

        if (team && team->ai)
        {
          // Clear team data
          ResetTeam(teamId);

          // Store the new team info
          Data::Store(*team, teamId);

          return (TRUE);
        }
        else
        {
          LOG_DIAG(("DeleteAITeam: tried to delete non AI team 0x%.8X", team))
        }
      }
      return (FALSE);
    }


    //
    // ChangeMission
    //
    // Change the mission and reset team data which may be incorrect
    //
    void ChangeMission(const Missions::Mission *mission)
    {
      if (Cmd::isHost)
      {
        // Has anything changed
        if 
        (
          (Host::mission.missionFolder != mission->GetGroup().GetPath().crc)
          ||
          (Host::mission.mission != mission->GetName().crc)
          ||
          (Host::mission.crc != mission->GetDataCrc())
        )
        {
          // Clear the have mission flags
          for (BinTree<PlayerInfo>::Iterator p(&players); *p; p++)
          {
            (*p)->haveMission = FALSE;
            (*p)->launchReady = FALSE;

            // Store the information about the player
            Data::Store(**p, p.GetKey());
          }

          // Clear the team start locations
          for (BinTree<Team>::Iterator t(&teams); *t; t++)
          {
            (*t)->startLocation = Settings::Mission::NoStartLocation;

            // Store the information about the team
            Data::Store(**t, t.GetKey());
          }

          // Change the mission
          Host::mission.missionFolder = mission->GetGroup().GetPath().crc;
          Host::mission.mission = mission->GetName().crc;
          Host::mission.crc = mission->GetDataCrc();

          Data::Store(Host::mission);
        }
      }
    }


    //
    // GetMission
    //
    // Get the currently selected mission
    //
    const Missions::Mission * GetMission()
    {
      return (Missions::FindMission(Host::mission.mission, Host::mission.missionFolder));
    }


    //
    // ClearMission
    //
    void ClearMission()
    {
      if (Cmd::isHost)
      {
        mission.mission = Settings::Mission::NoMission;
        Data::Store(mission);
      }
    }


    //
    // CheckLaunch
    //
    // Check to see if we can launch
    //
    Bool CheckLaunch()
    {
      LOG_DIAG(("Checking Launch Status ..."))

      // Do we have a map ?
      if (!PrivData::haveMission && Game::MissionPreLoaded())
      {
        // "MultiError"
        CONSOLE(0xB2178C6E, (TRANSLATE(("#multiplayer.setup.error.nomission")) ))
        LOG_DIAG(("There was no mission selected"))
        return (FALSE);
      }

      #ifndef DEVELOPMENT

      // Are there enough players ?
      if (Data::Online() && players.GetCount() < 2)
      {
        // "MultiError"
        CONSOLE(0xB2178C6E, (TRANSLATE(("#multiplayer.setup.error.moreplayers")) ))
        LOG_DIAG(("There weren't enough players"))
        return (FALSE);
      }

      #endif

      //
      // Firstly, check to see that all players 
      // - have the same version
      // - have the current map 
      // - are ready to launch
      //
      for (BinTree<PlayerInfo>::Iterator p(&players); *p; p++)
      {
        LOG_DIAG(("Checking player [%08X]", p.GetKey()))

        // Player doesn't have the current map
        Network::Player *np = Network::GetPlayers().Find(p.GetKey());

        if (!np)
        {
          // "MultiError"
          CONSOLE(0xB2178C6E, ("Player [%08X] not found", p.GetKey()))
          LOG_DIAG(("Player [%08X] was not found", p.GetKey()))
          return (FALSE);
        }

        if (!(*p)->correctVersion)
        {
          // "MultiError"
          CONSOLE(0xB2178C6E, (TRANSLATE(("#multiplayer.setup.error.wrongversion", 1, Utils::Ansi2Unicode(np->GetName()) )) ))
          LOG_DIAG(("%s has the a different version", np->GetName()))
          return (FALSE);
        }

        if (!(*p)->launchReady)
        {
          // "MultiError"
          CONSOLE(0xB2178C6E, (TRANSLATE(("#multiplayer.setup.error.notready", 1, Utils::Ansi2Unicode(np->GetName()) )) ))

          const char *msg = Utils::Unicode2Ansi(TRANSLATE(("#multiplayer.setup.error.launch")));
          Data::Send(Commands::MessageQuote, Utils::Strlen(msg) + 1, (const U8 *) msg, FALSE);

          LOG_DIAG(("%s isn't ready to launch", np->GetName()))
          return (FALSE);
        }

        if (!(*p)->haveMission)
        {
          // "MultiError"
          CONSOLE(0xB2178C6E, (TRANSLATE(("#multiplayer.setup.error.havemission", 1, Utils::Ansi2Unicode(np->GetName()) )) ))
          LOG_DIAG(("%s doesn't have the current mission", np->GetName()))
          return (FALSE);
        }
      }

      // Check the number of teams
      const U32 *teamCounts = GetTeamCounts();
      U32 numTeams = 0;
      for (int t = 0; t < Game::MAX_TEAMS; t++)
      {
        if (teamCounts[t])
        {
          numTeams++;
        }
      }

      if (numTeams > PrivData::preview->GetTeams())
      {
        // "MultiError"
        CONSOLE(0xB2178C6E, (TRANSLATE(("#multiplayer.setup.error.toomanyteams")) ))
        LOG_DIAG(("Too many teams for the current mission."))
        return (FALSE);
      }

      // If we're in fixed placement mode, make sure all 
      // the teams have selected VALID start locations
      if (options.placement == Settings::Options::Fixed)
      {
        // Get all the teams
        BinTree<GroupTree> groups;
        BuildPlayerHierachy(groups);

        for (BinTree<GroupTree>::Iterator gti(&groups); *gti; gti++)
        {
          for (BinTree<TeamTree>::Iterator tti(&(*gti)->teams); *tti; tti++)
          {
            // Check to see if that teams start location resolves to a team
            if (!PrivData::preview->FindTeamCrc((*tti)->team->startLocation))
            {
              // Is this AI ?
              if ((*tti)->team->ai)
              {
                // "MultiError"
                CONSOLE(0xB2178C6E, (TRANSLATE(("#multiplayer.setup.error.noaistartlocation")) ))
              }
              else
              {
                // Get the team leader
                Network::Player *player = Network::GetPlayers().Find((*tti)->team->ownerId);

                if (player)
                {
                  // "MultiError"
                  CONSOLE(0xB2178C6E, (TRANSLATE(("#multiplayer.setup.error.nostartlocation", 1, Utils::Ansi2Unicode(player->GetName()) )) ))
                }
                else
                {
                  // "MultiError"
                  CONSOLE(0xB2178C6E, (TRANSLATE(("#multiplayer.setup.error.noteamstartlocation")) ))
                }
              }
              LOG_DIAG(("Team hasn't selected a start location."))
              groups.DisposeAll();
              return (FALSE);
            }
          }
        }
        groups.DisposeAll();
      }

      return (TRUE);
    }


    //
    // InvalidateLaunchReadyness
    //
    void InvalidateLaunchReadyness()
    {
      for (BinTree<PlayerInfo>::Iterator p(&players); *p; p++)
      {
        (*p)->launchReady = FALSE;

        // Store the information about the player
        Data::Store(**p, p.GetKey());
      }
    }


    //
    // Send updated settings to all players
    //
    void SendUpdatedSettings(U32 placement, U32 difficulty, U32 ruleSet, U32 startCredits, U32 unitLimit, U32 resourceType)
    {
      // Make changes in the host
      MultiPlayer::Host::InvalidateLaunchReadyness();

      options.placement = placement;
      options.difficulty = difficulty;
      options.ruleSet = ruleSet;
      options.startCredits = startCredits;
      options.unitLimit = unitLimit;
      options.resourceType = resourceType;

      Data::Store(options);
    }


    //
    // Update the ruleset
    //
    void SendUpdatedRuleSet(U32 ruleSet)
    {
      // Make changes in the host
      MultiPlayer::Host::InvalidateLaunchReadyness();

      options.ruleSet = ruleSet;

      Data::Store(options);
    }


    //
    // Get team counts
    //
    const U32 * GetTeamCounts()
    {
      static U32 teamCounts[Game::MAX_TEAMS];

      for (int t = 0; t < Game::MAX_TEAMS; t++)
      {
        teamCounts[t] = 0;
      }

      // Figure out how many people are on each team
      for (BinTree<PlayerInfo>::Iterator pi(&players); *pi; pi++)
      {
        teamCounts[(*pi)->teamId]++;
      }

      // Set all AI teams to 1 player
      for (t = 0; t < Game::MAX_TEAMS; t++)
      {
        Team *team = teams.Find(t);

        if (team && team->ai)
        {
          ASSERT(!teamCounts[t])
          teamCounts[t] = U32_MAX;
        }
      }

      return (teamCounts);
    }


    //
    // Get group counts
    //
    const U32 * GetGroupCounts()
    {
      static U32 groupCounts[Game::MAX_TEAMS];
      const U32 *teamCounts = GetTeamCounts();

      for (int g = 0; g < Game::MAX_TEAMS; g++)
      {
        groupCounts[g] = 0;
      }

      // Figure out how many teams are in each group
      for (BinTree<Team>::Iterator ti(&teams); *ti; ti++)
      {
        // Only teams which have players on them count
        if (teamCounts[ti.GetKey()] > 0)
        {
          groupCounts[(*ti)->groupId]++;
        }
      }

      return (groupCounts);
    }


    //
    // GetColorUsage
    //
    const Bool * GetColorUsage()
    {
      static Bool colors[Game::MAX_TEAMS + 1];
      const U32 *teamCounts = GetTeamCounts();

      for (int t = 0; t <= Game::MAX_TEAMS; t++)
      {
        colors[t] = FALSE;
      }

      for (BinTree<Team>::Iterator ti(&teams); *ti; ti++)
      {
        if (teamCounts[ti.GetKey()])
        {
          colors[(*ti)->color] = TRUE;
        }
      }

      return (colors);
    }

  }

}
