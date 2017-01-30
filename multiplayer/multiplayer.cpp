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
#include "multiplayer.h"
#include "multiplayer_private.h"
#include "multiplayer_controls.h"
#include "multiplayer_controls_playerlist.h"
#include "multiplayer_cmd_private.h"
#include "multiplayer_data_private.h"
#include "multiplayer_host.h"
#include "multiplayer_network.h"
#include "multiplayer_transfer.h"
#include "multiplayer_addressbook.h"
#include "multiplayer_settings.h"
#include "multiplayer_download.h"
#include "hardware.h"
#include "gameconstants.h"
#include "game_rc.h"
#include "game.h"
#include "console.h"
#include "player.h"
#include "tagobjdec.h"
#include "strategic.h"
#include "strategic_object.h"
#include "resourceobj.h"
#include "babel.h"

#include "unitobj.h"
#include "worldctrl.h"
#include "team.h"
#include "game.h"
#include "main.h"
#include "vid.h"
#include "user.h"
#include "difficulty.h"
#include "gameobjctrl.h"
#include "mods.h"
#include "sides.h"
#include "savegame.h"

#include "won.h"



///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{
  LOGDEFLOCAL("MultiPlayer")


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Prototypes
  //
  static void Handler(CRC from, U32 key, U32 size, const U8 *data);
  static void CmdLineHookProc(const Main::CmdLineArg &arg, const Main::CmdLineVal &val);


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Data key statics
  //
  const CRC Player::key         = 0xEB4CB671; // "Key::Player"
  const CRC PlayerInfo::key     = 0xB9A7AA13; // "Key::PlayerInfo"
  const CRC Team::key           = 0x59AFB560; // "Key::Team"
  const CRC TeamMap::key        = 0xEF7DDBA1; // "Key::TeamMap"
  const CRC Mission::key        = 0x473B444B; // "Key::Mission"
  const CRC PropertyFilter::key = 0x2D845A35; // "Key::PropertyFilter"
  const CRC Options::key        = 0x4FADF9EF; // "Key::Options"


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace PrivData
  //
  namespace PrivData
  {
  
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Exported Data
    //

    // Have current mission
    Bool haveMission;

    // Preview for the mission, if haveMission is TRUE
    Game::Preview *preview = NULL;

    // Local Player info
    Player player;

    // Recent pings to the server
    U16 localPings[maxLocalPings];

    // Game setup control
    IControlPtr gameSetupCtrl;

    // Client control
    IControlPtr clientCtrl;

    // Server control
    IControlPtr serverCtrl;

    // Transfer control
    IControlPtr transferCtrl;

    // Transfer control
    IControlPtr transferListCtrl;

    // Player list window
    IControlPtr playersCtrl;

    // Game chat window
    IControlPtr chatCtrl;

    // Update window
    IControlPtr updateCtrl;

    // Download window
    IControlPtr downloadCtrl;

    // Info window
    IControlPtr infoCtrl;


    //
    // Clear data when a mission changes
    //
    void ClearMission()
    {
      haveMission = FALSE;

      // Clear the vars for the mission description
      Cmd::UploadMapInfo(NULL, NULL, 0, 0);

      if (preview)
      {
        delete preview;
        preview = NULL;
      }
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace CmdLine
  //
  namespace CmdLine
  {
    Bool host;
    Bool join;
    FileIdent user;
    FileIdent ip;
    FileIdent port;
    FileIdent session;
    FileIdent password;
    U32 maxPlayers;

    // Reset the command line
    void Reset()
    {
      host = FALSE;
      join = FALSE;
      user = "";
      ip = "";
      port = "";
      session = "";
      password = "";
      maxPlayers = 16;
    }

  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // Initialized Flag
  static Bool initialized = FALSE;  

  static U32 dataCrc;
  static BinTree<void> invalidPlayers;

  static Color teamColors[] =
  {
    Color(1.0f, 0.0f, 0.0f),            // Red
    Color(1.0f, 1.0f, 0.0f),            // Yellow
    Color(0.5f, 1.0f, 0.5f),            // Green
    Color(0.0f, 0.5f, 0.0f),            // DkGreen
    Color(0.0f, 1.0f, 1.0f),            // Cyan
    Color(0.0f, 0.0f, 1.0f),            // Blue
    Color(0.5f, 0.0f, 0.5f),            // Purple
    Color(1.0f, 0.5f, 1.0f),            // Pink
    Color(1.0f, 1.0f, 1.0f)             // White
  };

  static Color teamBgColors[] =
  {
    Color(0.80f, 0.00f, 0.00f, 0.40f),  // Red
    Color(0.80f, 0.80f, 0.00f, 0.40f),  // Yellow
    Color(0.40f, 0.80f, 0.40f, 0.40f),  // Green
    Color(0.00f, 0.40f, 0.00f, 0.40f),  // DkGreen
    Color(0.00f, 0.80f, 0.80f, 0.40f),  // Cyan
    Color(0.00f, 0.00f, 0.80f, 0.40f),  // Blue
    Color(0.40f, 0.00f, 0.40f, 0.40f),  // Purple
    Color(0.80f, 0.40f, 0.80f, 0.40f),  // Pink
    Color(0.80f, 0.80f, 0.80f, 0.40f)   // White
  };

  static Color playerBgColors[] =
  {
    Color(0.50f, 0.00f, 0.00f, 0.30f),  // Red
    Color(0.50f, 0.50f, 0.00f, 0.30f),  // Yellow
    Color(0.25f, 0.50f, 0.25f, 0.30f),  // Green
    Color(0.00f, 0.25f, 0.00f, 0.30f),  // DkGreen
    Color(0.00f, 0.50f, 0.50f, 0.30f),  // Cyan
    Color(0.00f, 0.00f, 0.50f, 0.30f),  // Blue
    Color(0.25f, 0.00f, 0.25f, 0.30f),  // Purple
    Color(0.50f, 0.25f, 0.50f, 0.30f),  // Pink
    Color(0.50f, 0.50f, 0.50f, 0.30f)   // White
  };


  //
  // PreInit
  //
  void PreInit()
  {
    CmdLine::Reset();

    Main::RegisterCmdLineHandler("host", CmdLineHookProc);
    Main::RegisterCmdLineHandler("join", CmdLineHookProc);
    Main::RegisterCmdLineHandler("ip", CmdLineHookProc);
    Main::RegisterCmdLineHandler("port", CmdLineHookProc);
    Main::RegisterCmdLineHandler("user", CmdLineHookProc);
    Main::RegisterCmdLineHandler("session", CmdLineHookProc);
    Main::RegisterCmdLineHandler("password", CmdLineHookProc);
    Main::RegisterCmdLineHandler("maxplayers", CmdLineHookProc);
  }



  //
  // Init
  //
  void Init()
  {
    ASSERT(!initialized)

    // Initialize controls
    Controls::Init();

    // Intialize downloads
    Download::Init();

    // Initialize settings
    Settings::Init();

    // Initialize commands
    Cmd::Init();

    // Initialize data
    Data::Init();

    // Initialize Network
    Network::Init();

    // Initialize Transfers
    Transfer::Init();

    // Initialize address book
    AddressBook::Init();

    // Initialize WON
    Won::Init();

    // Register data handlers
    MultiPlayer::Data::RegisterHandler(Commands::IntegrityChallenge, Handler);
    MultiPlayer::Data::RegisterHandler(Commands::Message, Handler);
    MultiPlayer::Data::RegisterHandler(Commands::MessageQuote, Handler);
    MultiPlayer::Data::RegisterHandler(Commands::MessagePrivate, Handler);
    MultiPlayer::Data::RegisterHandler(Commands::MessageTeam, Handler);
    MultiPlayer::Data::RegisterHandler(Commands::MessageGroup, Handler);

    MultiPlayer::Data::RegisterHandler(Options::key, Handler);
    MultiPlayer::Data::RegisterHandler(Mission::key, Handler);
    MultiPlayer::Data::RegisterHandler(Player::key, Handler);
    MultiPlayer::Data::RegisterHandler(PlayerInfo::key, Handler);
    MultiPlayer::Data::RegisterHandler(Team::key, Handler);
    MultiPlayer::Data::RegisterHandler(PropertyFilter::key, Handler);

    // Register transfer data handlers
    MultiPlayer::Data::RegisterHandler(Commands::TransferOffer, Transfer::Handler);
    MultiPlayer::Data::RegisterHandler(Commands::TransferOfferAccept, Transfer::Handler);
    MultiPlayer::Data::RegisterHandler(Commands::TransferOfferDeny, Transfer::Handler);
    MultiPlayer::Data::RegisterHandler(Commands::TransferInitiate, Transfer::Handler);

    // Register host data handlers
    MultiPlayer::Data::RegisterHandler(Commands::IntegrityResponse, Host::Handler);
    MultiPlayer::Data::RegisterHandler(Commands::Launch, Host::Handler);
    MultiPlayer::Data::RegisterHandler(Commands::LaunchReady, Host::Handler);
    MultiPlayer::Data::RegisterHandler(Commands::HaveMission, Host::Handler);
    MultiPlayer::Data::RegisterHandler(Commands::JoinTeam, Host::Handler);
    MultiPlayer::Data::RegisterHandler(Commands::LeaveTeam, Host::Handler);
    MultiPlayer::Data::RegisterHandler(Commands::SetTeamColor, Host::Handler);
    MultiPlayer::Data::RegisterHandler(Commands::SetTeamSide, Host::Handler);
    MultiPlayer::Data::RegisterHandler(Commands::SetStartLocation, Host::Handler);
    MultiPlayer::Data::RegisterHandler(Commands::SetDifficulty, Host::Handler);
    MultiPlayer::Data::RegisterHandler(Commands::SetPersonality, Host::Handler);
    MultiPlayer::Data::RegisterHandler(Commands::JoinGroup, Host::Handler);
    MultiPlayer::Data::RegisterHandler(Commands::LeaveGroup, Host::Handler);
    MultiPlayer::Data::RegisterHandler(Commands::JoinGroupAI, Host::Handler);
    MultiPlayer::Data::RegisterHandler(Commands::LeaveGroupAI, Host::Handler);

    initialized = TRUE;
  }


  //
  // Done
  //
  void Done()
  {
    ASSERT(initialized)

    // Terminate
    //Terminate();

    PrivData::ClearMission();

    // Cleanup
    Host::Reset();

    // Shutdown WON
    Won::Done();

    // Shutdown Addressbook
    AddressBook::Done();

    // Shutdown Transfers
    Transfer::Done();

    // Shutdown Network
    Network::Done();

    // Shutdown commands
    Cmd::Done();

    // Shutdown data
    Data::Done();

    // Shutdown settings
    Settings::Done();

    // Shutdown downloads
    Download::Done();

    // Shutdown controls
    Controls::Done();

    // Clean up invalid players
    invalidPlayers.UnlinkAll();

    initialized = FALSE;
  }


  //
  // Terminate the network connection
  //
  void Terminate()
  {
    ASSERT(initialized)

    // Reset networking
    Network::Reset();
  }


  //
  // Process multiplayer
  //
  void Process()
  {
    // Process Network
    Network::Process();

    // Process Transfers
    Transfer::Process();

    // Process WON
    Won::Process();
  }


  //
  // Reset multiplayer
  //
  void Reset()
  {
    // Reset commands
    Cmd::Reset();

    // Reset networking
    Network::Reset();

    // Reset Transfers
    Transfer::Reset();

    // Have the current mission
    PrivData::ClearMission();

    // Reset ping data
    Utils::Memset(&PrivData::localPings, 0x00, sizeof (PrivData::localPings));

    // Reset host data
    Host::Reset();

    // Reset local data
    Data::Reset();

    // Load settings from user
    Settings::LoadFromUser();

  }


  //
  // Is multiplayer ready to enter the game
  //
  Bool IsReady()
  {
    ASSERT(initialized)

    // Get the list of players from the network system
    for (NBinTree<Network::Player>::Iterator np(&Network::GetPlayers()); *np; ++np)
    {
      // Update the player info
      const Player *player;

      // Store the information about the player
      if (Data::Get(&player, (*np)->GetId()))
      {
        if (!player->ready)
        {
          return (FALSE);
        }

        if (player->dataCrc != dataCrc)
        {
          if (!invalidPlayers.Exists((*np)->GetId()))
          {
            CH playerBuff[128];
            Utils::Ansi2Unicode(playerBuff, 128, (*np)->GetName());

            invalidPlayers.Add((*np)->GetId());
            CON_MSG((TRANSLATE(("#game.info.data", 1, playerBuff )) ))
          }
        }
      }
      else
      {
        // A player with no data isn't ready
        return (FALSE);
      }
    }

    LOG_DIAG(("All players are ready to go!"))

    // Clear our readyness flag
    PrivData::player.ready = FALSE;
    Data::Store(PrivData::player, Network::GetCurrentPlayer().GetId());

    return (TRUE);
  }


  //
  // Set the readyness of this player
  //
  void SetReady()
  {
    ASSERT(initialized)

    if (!PrivData::player.ready)
    {
      LOG_DIAG(("We're ready for action!"))
      PrivData::player.ready = TRUE;
      PrivData::player.dataCrc = dataCrc;
      Data::Store(PrivData::player, Network::GetCurrentPlayer().GetId());
    }
  }


  //
  // Clear the readyness of this player
  //
  void ClearReady()
  {
    ASSERT(initialized)
    PrivData::player.ready = FALSE;
    if (Network::HaveCurrentPlayer())
    {
      Data::Store(PrivData::player, Network::GetCurrentPlayer().GetId());
    }
  }


  //
  // Pre setup the mission
  //
  void SetupPreMission()
  {
    ASSERT(initialized)

    // Make sure transfer system is reset
    Transfer::Reset();

    // Do we have the current mission
    if (PrivData::haveMission)
    {
      const Mission *mission;
      const Options *options;

      if (Data::Get(&mission) && Data::Get(&options))
      {
        LOG_DIAG(("Options->RandomSeed == %d", options->randomSeed));
        Random::sync.SetSeed(options->randomSeed);

        // Clean up invalid players
        invalidPlayers.UnlinkAll();

        // Initialize the data CRC
        dataCrc = Random::sync.Raw();

        LOG_DIAG(("Data CRC initialized to %08X", dataCrc));

        Mods::Mod *mod = NULL;
        if (options->ruleSet != Settings::Options::UseMissionRules)
        {
          mod = Mods::GetMod(Mods::Types::RuleSet, options->ruleSet);
          if (mod)
          {
            mod->Initialize();
          }
        }
      }
    }

  }


  //
  // Setup the mission
  //
  // This has been called once the game is underway and Team information is valid
  //
  void SetupMission()
  {
    ASSERT(initialized)

    // Do we have the current mission
    if (PrivData::haveMission)
    {
      const Mission *mission;
      const Options *options;

      if (Data::Get(&mission) && Data::Get(&options))
      {
        ASSERT(Game::MissionPreLoaded())

        Mods::Mod *mod = NULL;
        if (options->ruleSet != Settings::Options::UseMissionRules)
        {
          mod = Mods::GetMod(Mods::Types::RuleSet, options->ruleSet);
          if (mod)
          {
            mod->Load();
          }
        }

        // Do we need to enable all-regen resources
        if (options->resourceType == Settings::Options::ResourceTypeRegen)
        {
          // From the designers 'spec':
          //
          // "Default" does nothing, "All Regen" makes non-regen resources regenerate at
          // a rate of 3/sec, and any other regen resource regenerate at double the
          // normal rate (i.e. our regen resource will regen at 6/sec instead of 3/sec).
          //
          ResourceObjType::EnableAllRegen(2, 3);
          LOG_DIAG(("Regen resources"));
        }
        else
        {
          LOG_DIAG(("Normal resources"));
        }

        // Build the list of types which match the filterered properties
        const PropertyFilter *propertyList;
        if (Data::Get(&propertyList))
        {
          for (List<GameObjType>::Iterator t(&GameObjCtrl::objTypesList); *t; ++t)
          {
            UnitObjType *ut = Promote::Type<UnitObjType>(*t);

            if (ut)
            {
              for (U32 p = 0; p < propertyList->numFilters; ++p)
              {
                if (ut->HasProperty(propertyList->filters[p]))
                {
                  ut->SetFiltered(TRUE);
                  //LOG_DIAG(("Filtering '%s'", ut->GetName()))
                  break;
                }
              }
            }
          }
        }

        BinTree<GroupTree> groups;
        BuildPlayerHierachy(groups);

        /*
        {
          for (BinTree<GroupTree>::Iterator gti(&groups); *gti; gti++)
          {
            LOG_DIAG(("Group %d", gti.GetKey()))

            for (BinTree<TeamTree>::Iterator tti(&(*gti)->teams); *tti; tti++)
            {
              LOG_DIAG((" Team %d", tti.GetKey()))
              LOG_DIAG((" - group id: %d", (*tti)->team->groupId))
              LOG_DIAG((" - owner id: %08X", (*tti)->team->ownerId))
              LOG_DIAG((" - color: %d", (*tti)->team->color))
              LOG_DIAG((" - startlocation: %08X", (*tti)->team->startLocation))
              LOG_DIAG((" - ai: %d", (*tti)->team->ai))
              LOG_DIAG((" - personality: %08X", (*tti)->team->personality))
              LOG_DIAG((" - difficulty: %08X", (*tti)->team->difficulty))
              LOG_DIAG((" - side: %s", (*tti)->team->side.str))

              for (BinTree<const Player>::Iterator pi(&(*tti)->players); *pi; pi++)
              {
                LOG_DIAG(("  Player %08X", pi.GetKey()))
                LOG_DIAG(("  - CPU: %s", (*pi)->cpu.str))
                LOG_DIAG(("  - Memory: %d", (*pi)->memory))
                LOG_DIAG(("  - Video: %s", (*pi)->video.str))
                LOG_DIAG(("  - OS: %s", (*pi)->os.str))
                LOG_DIAG(("  - Ready: %d", (*pi)->ready))
              }
            }
          }
        }
        */

        // Execute loading config
        U32 teamId = 0;
  
        List<::Team> teams;
        for (teamId = 0; teamId < Game::MAX_TEAMS; teamId++)
        {
          ::Team *team = ::Team::Id2Team(teamId);
          if (team && team->IsAvailablePlay())
          {
            LOG_DIAG(("Team %d is available", teamId))
            teams.Append(team);
          }
        }

        // Execute loading config
        for (BinTree<GroupTree>::Iterator gti(&groups); *gti; gti++)
        {
          for (BinTree<TeamTree>::Iterator tti(&(*gti)->teams); *tti; tti++)
          {
            // Get the next available team
            ::Team *team = NULL;

            switch (options->placement)
            {
              case Settings::Options::Fixed:
                team = ::Team::NameCrc2Team((*tti)->team->startLocation);
                if (!team)
                {
                  ERR_FATAL(("Could not find team"))
                }
                break;

              case Settings::Options::Random:

                if (!teams.GetCount())
                {
                  ERR_FATAL(("More players than teams"))
                }

                U32 index = Random::sync.Integer(teams.GetCount());

                LOG_DIAG(("Randomly picked team index %d", index))

                team = teams[index];
                teams.Unlink(team);
                break;
            }

            const TeamMap *mapping;

            if (Data::Get(&mapping, tti.GetKey()))
            {
              team = ::Team::NameCrc2Team(mapping->teamName.crc);

              if (!team)
              {
                ERR_FATAL(("Unable to resolve team [%s]", mapping->teamName.str));
              }

              LOG_DIAG(("Resolved team [%s]", team->GetName()));
            }
            else

            if (!Data::Online())
            {
              TeamMap mapping;

              // Set the name of the team
              mapping.teamName = team->GetName();

              // Store the new team info
              Data::Store(mapping, tti.GetKey());

              LOG_DIAG(("Saved entry [%s][%d]", mapping.teamName.str, tti.GetKey()));
            }

            LOG_DIAG(("Network Team %d using team %d", tti.GetKey(), team->GetId()))

            // Download the team color to the team
            team->SetColor(GetTeamColor((*tti)->team->color));

            if (options->difficulty == Settings::Options::AllowDifficulty)
            {
              // Download the difficulty to the team
              Difficulty::Setting *setting = Difficulty::FindSetting((*tti)->team->difficulty);
              if (setting)
              {
                LOG_DIAG(("Setting difficulty to %f", setting->GetValue()))
                team->SetDifficulty(setting->GetValue());
              }
              else
              {
                LOG_DIAG(("Unknown difficulty setting 0x%08x", (*tti)->team->difficulty))
              }
            }

            // If this team isn't fixed, set its side
            if (!team->GetSideFixed())
            {
              if ((*tti)->team->side.crc == 0xB8586215) // "Random"
              {
                Sides::Side &side = Sides::GetRandomSide();

                LOG_DIAG(("Setting side to %s", side.GetName().str))

                // Download the randomly picked side
                team->SetSide(side.GetName().str);
              }
              else
              {
                LOG_DIAG(("Setting side to %s", (*tti)->team->side.str))

                // Download the side to the team
                team->SetSide((*tti)->team->side.str);
              }
            }

            // Create an actual player in the game
            ::Player *player = NULL;

            if ((*tti)->team->ai)
            {
              U32 personality;

              if ((*tti)->team->personality == 0xB8586215) // "Random"
              {
                Mods::Mod *mod = Mods::GetRandomMod(Mods::Types::Personality);

                if (mod)
                {
                  personality = mod->GetName().crc;
                }
                else
                {
                  personality = 0;
                }
              }
              else
              {
                personality = (*tti)->team->personality;
              }

              // Create an AI player
              Strategic::Object *object = Strategic::Create(personality);

              // Place this player on this team
              object->SetTeam(team);

              // Set the name of this personality into the team
              if (object->GetMod())
              {
                team->SetPersonality(object->GetMod()->GetName().str);
              }

              player = object;

              LOG_DIAG(("Creating AI [%08X] for Team '%s' [%d]", (*tti)->team->personality, team->GetName(), team->GetId()))
            }

            // Stuff this team into the teamTree data (slight hack)
            (*tti)->team = (Team *) team;

            // Go through all the players on this team
            for (BinTree<const Player>::Iterator pi(&(*tti)->players); *pi; pi++)
            {
              // Get the network player with this id
              Network::Player *networkPlayer = Network::GetPlayers().Find(pi.GetKey());
              ASSERT(networkPlayer)

              // Create an actual player in the game
              ::Player *player = new ::Player(networkPlayer->GetName());

              LOG_DIAG(("Creating Player '%s' for Team '%s' [%d]", networkPlayer->GetName(), team->GetName(), team->GetId()))

              // If this is the local player then set the current player
              if (networkPlayer->GetId() == Network::GetCurrentPlayer().GetId())
              {
                ::Player::SetCurrentPlayer(player);
              }

              // Place this player on this team
              player->SetTeam(team);
            }

          }
        }

        teams.UnlinkAll();

        // Now go through and setup the relations, teams which are in the same group 
        // get to be allied and teams which are in different groups are enemies

        for (BinTree<GroupTree>::Iterator gi(&groups); *gi; gi++)
        {
          for (BinTree<TeamTree>::Iterator ti(&(*gi)->teams); *ti; ti++)
          {
            for (BinTree<GroupTree>::Iterator gsi(&groups); *gsi; gsi++)
            {
              for (BinTree<TeamTree>::Iterator tsi(&(*gsi)->teams); *tsi; tsi++)
              {
                // Don't change the relationship with ourself
                if (*ti != *tsi)
                {
                  ::Team *teamWho = ((::Team *)(*ti)->team);
                  ::Team *teamWith = ((::Team *)(*tsi)->team);
                  Relation relation = (*gi == *gsi) ? Relation::ALLY : Relation::ENEMY;

                  LOG_DIAG(("Setting relation between %d and %d to %s", teamWho->GetId(), teamWith->GetId(), relation.GetName()))

                  teamWho->SetRelation(teamWith->GetId(), relation);

                  // Don't set the resource if we're loading a saved game
                  if (!SaveGame::LoadActive())
                  {
                    // If starting credits have been specified, set the starting credits
                    if (options->startCredits)
                    {
                      teamWho->SetResourceStore(options->startCredits);
                    }
                  }

                  // Set the unit limit
                  teamWho->SetUnitLimit(options->unitLimit);
                }
              }
            }
          }
        }

        // Cleanup
        groups.DisposeAll();

        // Purge teams which don't have players
        ::Team::Purge();
      }
      else
      {
        ERR_FATAL(("How is it that we have the map but no mission?"))
      }
    }
  }


  //
  // Apply multiplayer data setup to the mission
  //
  void SetupPostMission()
  {
    // Do we have the current mission
    const Options *options;

    if (Data::Get(&options))
    {
      ASSERT(Game::MissionPreLoaded())

      Mods::Mod *mod = NULL;
      if (options->ruleSet != Settings::Options::UseMissionRules)
      {
        mod = Mods::GetMod(Mods::Types::RuleSet, options->ruleSet);
      }

      if (mod)
      {
        for (U32 teamId = 0; teamId < Game::MAX_TEAMS; teamId++)
        {
          ::Team *team = ::Team::Id2Team(teamId);
          if (team)
          {
            mod->Setup(team);
          }
        }
      }
    }
  }


  //
  // Save multiplayer system
  //
  void Save(FScope *scope)
  {
    // Save multiplayer data
    MultiPlayer::Data::Save(scope);
  }


  //
  // Load multiplayer system
  //
  void Load(FScope *scope)
  {
    // Reset everything
    MultiPlayer::Reset();

    // We are the host
    Cmd::isHost = TRUE;

    // Go through the motions
    Network::CreateCurrentPlayer();
    HostedSession();
    Host::EnterPlayer(Network::GetCurrentPlayer().GetId());
    JoinedSession();

    // Load the multiplayer data
    MultiPlayer::Data::Load(scope);

    UpdateMission();
  }


  //
  // Is this the host
  //
  Bool IsHost()
  {
    return (Cmd::isHost ? TRUE : FALSE);
  }


  //
  // Get data crc
  //
  U32 GetDataCrc()
  {
    return (dataCrc);
  }


  //
  // Set data crc
  //
  void SetDataCrc(U32 crc)
  {
    dataCrc = crc;
  }


  //
  // GetTeamColor
  //
  const Color & GetTeamColor(U32 index)
  {
    ASSERT(index < (sizeof (teamColors) / sizeof (Color)));
    return (teamColors[index]);
  }


  //
  // GetTeamBgColor
  //
  const Color & GetTeamBgColor(U32 index)
  {
    ASSERT(index < (sizeof (teamBgColors) / sizeof (Color)));
    return (teamBgColors[index]);
  }


  //
  // GetPlayerBgColor
  //
  const Color & GetPlayerBgColor(U32 index)
  {
    ASSERT(index < (sizeof (teamBgColors) / sizeof (Color)));
    return (playerBgColors[index]);
  }


  //
  // Get the start location usage
  //
  void GetStartLocations(BinTree<const Team> &teams)
  {
    ASSERT(!teams.GetCount())

    if (PrivData::haveMission)
    {
      // Get all the teams
      BinTree<GroupTree> groups;
      BuildPlayerHierachy(groups);

      for (BinTree<GroupTree>::Iterator gti(&groups); *gti; gti++)
      {
        for (BinTree<TeamTree>::Iterator tti(&(*gti)->teams); *tti; tti++)
        {
          // Check to see if that teams start location resolves to a team
          if (PrivData::preview->FindTeamCrc((*tti)->team->startLocation))
          {
            // Add that team to the team tree
            teams.Add((*tti)->team->startLocation, (*tti)->team);
          }
        }
      }
      groups.DisposeAll();
    }
  }


  //
  // Construct the player hierachy
  //
  void BuildPlayerHierachy(BinTree<GroupTree> &groups)
  {
    // Get the list of players from the network system
    for (NBinTree<Network::Player>::Iterator np(&Network::GetPlayers()); *np; ++np)
    {
      const Player *player;
      const PlayerInfo *playerInfo;
      const Team *team;

      U32 playerId = (*np)->GetId();

      // Does this network player have Player and PlayerInfo and a Team ?
      if 
      (
        Data::Get(&player, playerId) &&
        Data::Get(&playerInfo, playerId) &&
        Data::Get(&team, playerInfo->teamId)
      )
      {
        // Do we have an entry for this teams group ?
        GroupTree *groupTree;
        groupTree = groups.Find(team->groupId);

        if (!groupTree)
        {
          groupTree = new GroupTree;
          groups.Add(team->groupId, groupTree);
        }

        // Is this team in this group ?
        TeamTree *teamTree;
        teamTree = groupTree->teams.Find(playerInfo->teamId);

        if (!teamTree)
        {
          teamTree = new TeamTree;
          teamTree->team = team;
          groupTree->teams.Add(playerInfo->teamId, teamTree);
        }

        // Make sure that this player isn't in the team tree
        ASSERT(!teamTree->players.Exists(playerId));

        // If this player isn't the owner of the team then add him
        //if (team->ownerId != playerId)
        {
          // Add this player to the team
          teamTree->players.Add(playerId, player);
        }
      }
    }

    // Iterate through AI teams
    for (U32 i = 0; i < Game::MAX_TEAMS; i++)
    {
      const Team *team;

      if (Data::Get(&team, i))
      {
        if (team->ai)
        {
          // Do we have an entry for this teams group ?
          GroupTree *groupTree = groups.Find(team->groupId);

          if (!groupTree)
          {
            groupTree = new GroupTree;
            groups.Add(team->groupId, groupTree);
          }

          // Is this team in this group ?
          TeamTree *teamTree = groupTree->teams.Find(i);

          if (!teamTree)
          {
            teamTree = new TeamTree;
            teamTree->team = team;
            groupTree->teams.Add(i, teamTree);
          }
        }
      }
    }
  }


  //
  // Update the mission
  //
  void UpdateMission()
  {
    const Mission *mission;
    if (Data::Get(&mission))
    {
      if (mission->mission == Settings::Mission::NoMission)
      {
        // We should not have this map cause there's no map
        if (PrivData::haveMission)
        {
          PrivData::ClearMission();
          Data::Store(PrivData::player, Network::GetCurrentPlayer().GetId());
        }
      }
      else
      {
        const Missions::Mission *info = Missions::FindMission(mission->mission, mission->missionFolder);

        if (info && info->GetDataCrc() == mission->crc)
        {
          // Setup the client for this map
          Missions::SetSelected(info);

          // Clear our current mission
          PrivData::ClearMission();

          // We have the mission
          PrivData::haveMission = TRUE;

          // Create a new preview
          PrivData::preview = new Game::Preview(info);

          // Tell the host that we have this mission
          Data::Send(0, NULL, Commands::HaveMission, 0, NULL);

          // Rebuild the vars
          Cmd::UploadMapInfo(info->GetGroup().GetPath().str, info->GetName().str, U32(PrivData::preview->GetSize() * WorldCtrl::CellSize()), PrivData::preview->GetTeams());
        }
        else
        {
          if (PrivData::haveMission)
          {
            PrivData::ClearMission();
            Data::Store(PrivData::player, Network::GetCurrentPlayer().GetId());
          }
        }
      }
    }
  }


  //
  // Update local player information
  //
  void UpdatePlayerInfo()
  {
    // Reset player data
    PrivData::player.cpu = Hardware::CPU::GetDesc();
    PrivData::player.memory = Hardware::Memory::GetPhysical();
    PrivData::player.os = Hardware::OS::GetDesc();
    PrivData::player.ready = FALSE;
    PrivData::player.longitude = Settings::GetLongitude();
    PrivData::player.latitude = Settings::GetLatitude();

    // Video card
    ASSERT( Vid::isStatus.initialized);

    char buf[256];

    Utils::Sprintf
    (
      buf, 256, "%dMB %s", 
      U32(F32(Vid::CurDD().desc.dwVidMemTotal) / (1024.0F * 1024.0F) + 0.5F),
      Vid::CurDD().ident.szDescription
    );
    PrivData::player.video = buf;

    Data::Store(PrivData::player, Network::GetCurrentPlayer().GetId());
  }


  //
  // HostedSession
  //
  void HostedSession()
  {
    LOG_DIAG(("HostedSession"))

    Host::Hosted();

    // Setup host var
    Cmd::isHost = TRUE;
  }


  //
  // JoinedSession
  //
  void JoinedSession()
  {
    LOG_DIAG(("JoinedSession"))

    UpdatePlayerInfo();
  }


  //
  // Handler
  //
  void Handler(CRC from, CRC key, U32 size, const U8 *data)
  {
    size;

    switch (key)
    {
      case Commands::IntegrityChallenge:
      {
        CAST(const Commands::Data::IntegrityChallenge *, challenge, data)
        Commands::Data::IntegrityResponse response;

        // Calculate our code crc from using the given initial key
        response.crc = Debug::Memory::GetCodeIntegrity(challenge->crc);

        // Send our response to the challenge
        Data::Send(0, NULL, Commands::IntegrityResponse, response);
        break;
      }

      case Commands::Message:
      case Commands::MessageQuote:
      case Commands::MessagePrivate:
      case Commands::MessageTeam:
      case Commands::MessageGroup:
      {
        Network::Player *player = Network::GetPlayers().Find(from);

        if (player)
        {
          switch (key)
          {
            case Commands::Message:
              CONSOLE(0x4DBB67F6, ("[%s] %s", player->GetName(), data)) // "ChatMessage"
              break;

            case Commands::MessageQuote:
              CONSOLE(0x18B19FE6, ("%s %s", player->GetName(), data)) // "ChatQuote"
              break;

            case Commands::MessagePrivate:
            case Commands::MessageTeam:
            case Commands::MessageGroup:
              CONSOLE(0xCBA10203, ("[%s] %s", player->GetName(), data)) // "ChatPrivate" 
          }
        }
        break;
      }

      case Player::key:
      case PlayerInfo::key:
      case Team::key:
        Controls::PlayerList::Dirty();
        break;

      case PropertyFilter::key:
      {
        const PropertyFilter *propertyList;
        if (Data::Get(&propertyList))
        {
          Cmd::UploadFiltering(propertyList->numFilters > 0 ? TRUE : FALSE);
        }
        break;
      }

      case Options::key:
      {
        const Options *options;

        if (Data::Get(&options))
        {
          // For clients, update the interface vars
          Cmd::UploadOptions(options);

          if (options->launch)
          {
            // Ensure that we're disconnected from WON
            Won::Disconnect();

            // We're launching, transfer all of the settings to the game
            Missions::LaunchMission(FALSE);
          }
        }
        break;
      };

      case Mission::key:
      {
        UpdateMission();

        break;
      }
    }
  }


  //
  // CmdLineHookProc
  //
  void CmdLineHookProc(const Main::CmdLineArg &arg, const Main::CmdLineVal &val)
  {
    switch (arg.crc)
    {
      case 0x4CF40B36: // "host"
        CmdLine::host = TRUE;
        break;

      case 0xA99E6807: // "join"
        CmdLine::join = TRUE;
        break;

      case 0x26485E81: // "ip"
        CmdLine::ip = val.str;
        break;

      case 0x90137F43: // "port"
        CmdLine::port = val.str;
        break;

      case 0x19BE1F7E: // "user"
        CmdLine::user = val.str;
        break;

      case 0x250A4530: // "session"
        CmdLine::session = val.str;
        break;

      case 0x4FA7EDBB: // "password"
        CmdLine::password = val.str;
        break;

      case 0x4E94574C: // "maxplayers"
        CmdLine::maxPlayers = Utils::AtoI(val.str);
        break;
    }
  }
}
