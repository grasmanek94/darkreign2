///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Commands
// 1-JUL-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "multiplayer_cmd_private.h"
#include "multiplayer_private.h"
#include "multiplayer_data.h"
#include "multiplayer_host.h"
#include "multiplayer_private.h"
#include "multiplayer_network_private.h"
#include "multiplayer_controls_sessions.h"
#include "multiplayer_download.h"
#include "multiplayer_settings.h"
#include "woniface.h"
#include "varsys.h"
#include "console.h"
#include "user.h"

#include "game_missionselection.h"

#include "game_rc.h"
#include "main.h"
#include "orders.h"
#include "iface.h"
#include "gametime.h"
#include "mods.h"
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
  // NameSpace Cmd
  //
  namespace Cmd
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Local variables
    //

    // Initialization flag
    static Bool initialized = FALSE;

    // Interface vars
    JoinInformation *joinInfo;
    HostInformation *hostInfo;

    VarInteger online;
    VarInteger isHost;
    VarInteger inWon;
    VarInteger isLobby;

    Bool updatingLock = FALSE;
    VarInteger isLocked;

    VarInteger fillInterface;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Prototypes
    //
    static void Handler(U32 pathCrc);


    //
    // Initialization
    // 
    void Init()
    {
      ASSERT(!initialized)

      VarSys::RegisterHandler("multiplayer", Handler);

      VarSys::CreateCmd("multiplayer.lookupname");
      VarSys::CreateCmd("multiplayer.lookupaddress");

      VarSys::CreateCmd("multiplayer.abort");
      VarSys::CreateCmd("multiplayer.instant");

      VarSys::CreateCmd("multiplayer.login");
      VarSys::CreateCmd("multiplayer.launch");

      VarSys::CreateCmd("multiplayer.migrate");
      VarSys::CreateCmd("multiplayer.checkfirewall");
      VarSys::CreateCmd("multiplayer.sizeof");

      VarSys::RegisterHandler("multiplayer.flags", Handler);
      VarSys::CreateInteger("multiplayer.flags.update", FALSE);
      VarSys::CreateInteger("multiplayer.flags.online", FALSE, VarSys::DEFAULT, &online);
      VarSys::CreateInteger("multiplayer.flags.isHost", FALSE, VarSys::DEFAULT, &isHost);
      VarSys::CreateInteger("multiplayer.flags.inWon", FALSE, VarSys::DEFAULT, &inWon);
      VarSys::CreateInteger("multiplayer.flags.isLobby", FALSE, VarSys::DEFAULT, &isLobby);
      VarSys::CreateInteger("multiplayer.flags.isLocked", FALSE, VarSys::DEFAULT | VarSys::NOTIFY, &isLocked);
      VarSys::CreateInteger("multiplayer.flags.fillInterface", FALSE, VarSys::DEFAULT, &fillInterface);

      // Register Commands
      VarSys::RegisterHandler("multiplayer.register", Handler);
      VarSys::CreateCmd("multiplayer.register.client");
      VarSys::CreateCmd("multiplayer.register.server");
      VarSys::CreateCmd("multiplayer.register.setup");
      VarSys::CreateCmd("multiplayer.register.transfer");
      VarSys::CreateCmd("multiplayer.register.transferlist");
      VarSys::CreateCmd("multiplayer.register.players");
      VarSys::CreateCmd("multiplayer.register.chat");
      VarSys::CreateCmd("multiplayer.register.updates");
      VarSys::CreateCmd("multiplayer.register.download");
      VarSys::CreateCmd("multiplayer.register.info");

      // Server Commands
      VarSys::RegisterHandler("multiplayer.server", Handler);
      VarSys::CreateCmd("multiplayer.server.start");
      VarSys::CreateCmd("multiplayer.server.connect");
      VarSys::CreateCmd("multiplayer.server.joinconnect");

      // Session Commands
      VarSys::RegisterHandler("multiplayer.session", Handler);
      VarSys::CreateCmd("multiplayer.session.connect");
      VarSys::CreateCmd("multiplayer.session.joinpassword");
      VarSys::CreateCmd("multiplayer.session.joindownload");
      VarSys::CreateCmd("multiplayer.session.createdownload");

      // Game setup commands
      VarSys::RegisterHandler("multiplayer.setup", Handler);
      VarSys::CreateCmd("multiplayer.setup.created");
      VarSys::CreateCmd("multiplayer.setup.joined");
      VarSys::CreateCmd("multiplayer.setup.leaving");
      VarSys::CreateCmd("multiplayer.setup.addai");
      VarSys::CreateCmd("multiplayer.setup.removeai");
      VarSys::CreateCmd("multiplayer.setup.fillai");

      VarSys::CreateCmd("multiplayer.setup.activated");

      VarSys::CreateCmd("multiplayer.setup.launch");
      VarSys::CreateCmd("multiplayer.setup.suggestmission");
      VarSys::CreateCmd("multiplayer.setup.getmission");
      VarSys::CreateCmd("multiplayer.setup.setmission");
      VarSys::CreateCmd("multiplayer.setup.setrandommission");
      VarSys::CreateCmd("multiplayer.setup.clearmission");

      // Multiplayer commands
      VarSys::RegisterHandler("multiplayer.cmd", Handler);
      VarSys::CreateCmd("multiplayer.cmd.me", VarSys::RAWDATA);
      VarSys::CreateCmd("multiplayer.cmd.msg", VarSys::RAWDATA);
      VarSys::CreateCmd("multiplayer.cmd.broadcast", VarSys::RAWDATA);
      VarSys::CreateCmd("multiplayer.cmd.team", VarSys::RAWDATA);
      VarSys::CreateCmd("multiplayer.cmd.t", VarSys::RAWDATA);
      VarSys::CreateCmd("multiplayer.cmd.enemies", VarSys::RAWDATA);
      VarSys::CreateCmd("multiplayer.cmd.e", VarSys::RAWDATA);
      VarSys::CreateCmd("multiplayer.cmd.allies", VarSys::RAWDATA);
      VarSys::CreateCmd("multiplayer.cmd.a", VarSys::RAWDATA);
      VarSys::CreateCmd("multiplayer.cmd.won", VarSys::RAWDATA);
      VarSys::CreateCmd("multiplayer.cmd.kick", VarSys::RAWDATA);
      VarSys::CreateCmd("multiplayer.cmd.k", VarSys::RAWDATA);
      VarSys::CreateCmd("multiplayer.cmd.help");

      // Download commands
      VarSys::RegisterHandler("multiplayer.download", Handler);
      VarSys::CreateCmd("multiplayer.download.motd");
      VarSys::CreateCmd("multiplayer.download.updates");

      initialized = TRUE;
    }


    //
    // Shutdown
    //
    void Done()
    {
      ASSERT(initialized)

      // Perform a reset
      Reset();

      // Delete the multiplayer scope
      VarSys::DeleteItem("multiplayer");

      initialized = FALSE;
    }


    //
    // Reset
    //
    void Reset()
    {
      if (joinInfo)
      {
        delete joinInfo;
        joinInfo = NULL;
      }
      if (hostInfo)
      {
        delete hostInfo;
        hostInfo = NULL;
      }
    }


    //
    // UploadOptions
    //
    void UploadOptions(const Options *options)
    {
      if (PrivData::infoCtrl.Alive())
      {
        IControl *ctrl = IFace::Find<IControl>("Placement", PrivData::infoCtrl, TRUE);

        // Upload current values
        switch (options->placement)
        {
          case Settings::Options::Random:
            ctrl->SetTextString(TRANSLATE(("#multiplayer.config.options.placement.random")), TRUE);
            ctrl->SetStateItem("Selected", FALSE);
            break;

          case Settings::Options::Fixed:
            ctrl->SetTextString(TRANSLATE(("#multiplayer.config.options.placement.fixed")), TRUE);
            ctrl->SetStateItem("Selected", TRUE);
            break;
        }

        ctrl = IFace::Find<IControl>("Difficulty", PrivData::infoCtrl, TRUE);

        switch (options->difficulty)
        {
          case Settings::Options::AllowDifficulty:
            ctrl->SetTextString(TRANSLATE(("#multiplayer.config.options.difficulty.allow")), TRUE);
            ctrl->SetStateItem("Selected", TRUE);
            break;

          case Settings::Options::DenyDifficulty:
            ctrl->SetTextString(TRANSLATE(("#multiplayer.config.options.difficulty.deny")), TRUE);
            ctrl->SetStateItem("Selected", FALSE);
            break;
        }

        ctrl = IFace::Find<IControl>("RuleSet", PrivData::infoCtrl, TRUE);

        if (options->ruleSet == Settings::Options::UseMissionRules)
        {
          ctrl->SetTextString(L"", FALSE);
        }
        else
        {
          Mods::Mod *mod = Mods::GetMod(Mods::Types::RuleSet, options->ruleSet);

          if (mod)
          {
            ctrl->SetTextString(TRANSLATE((mod->GetDescription().str)), TRUE);
          }
          else if (options->ruleSet == 0xC9EF9119) // "None"
          {
            ctrl->SetTextString(L"", TRUE);
          }
          else
          {
            ctrl->SetTextString(TRANSLATE(("#multiplayer.config.options.unknownruleset")), TRUE);
          }
        }

        ctrl = IFace::Find<IControl>("StartingResource", PrivData::infoCtrl, TRUE);

        if (options->startCredits)
        {
          ctrl->SetTextString(TRANSLATE(("#multiplayer.config.options.startresource", 1, options->startCredits)), TRUE);
          ctrl->SetStateItem("Selected", TRUE);
        }
        else
        {
          ctrl->SetTextString(TRANSLATE(("#multiplayer.config.options.startdefault")), TRUE);
          ctrl->SetStateItem("Selected", FALSE);
        }

        ctrl = IFace::Find<IControl>("UnitLimit", PrivData::infoCtrl, TRUE);
        ctrl->SetTextString(TRANSLATE(("#multiplayer.config.options.unitlimit.info", 1, options->unitLimit)), TRUE);
        ctrl->SetStateItem("Selected", options->unitLimit != UnitLimits::GetLimit());

        ctrl = IFace::Find<IControl>("ResourceType", PrivData::infoCtrl, TRUE);

        switch (options->resourceType)
        {
          case Settings::Options::ResourceTypeNormal:
            ctrl->SetTextString(TRANSLATE(("#multiplayer.config.options.resourcetype.normal")), TRUE);
            ctrl->SetStateItem("Selected", FALSE);
            break;

          case Settings::Options::ResourceTypeRegen:
            ctrl->SetTextString(TRANSLATE(("#multiplayer.config.options.resourcetype.regen")), TRUE);
            ctrl->SetStateItem("Selected", TRUE);
            break;
        }
      }
    }


    //
    // Upload filtering
    //
    void UploadFiltering(Bool on)
    {
      if (PrivData::infoCtrl.Alive())
      {
        IControl *ctrl = IFace::Find<IControl>("Filtering", PrivData::infoCtrl, TRUE);

        if (on)
        {
          ctrl->SetTextString(TRANSLATE(("#multiplayer.config.options.filtering.on")), TRUE);
          ctrl->SetStateItem("Selected", TRUE);
        }
        else
        {
          ctrl->SetTextString(TRANSLATE(("#multiplayer.config.options.filtering.off")), TRUE);
          ctrl->SetStateItem("Selected", FALSE);
        }
      }
    }


    //
    // Upload map infomation
    //
    void UploadMapInfo(const char *, const char *mission, U32 size, U32 teams)
    {
      if (PrivData::infoCtrl.Alive())
      {
        IControl *ctrl;

        if (mission)
        {
          ctrl = IFace::Find<IControl>("MissionName", PrivData::infoCtrl, TRUE);
          ctrl->SetTextString(Utils::Ansi2Unicode(mission), TRUE);
          ctrl = IFace::Find<IControl>("MissionSize", PrivData::infoCtrl, TRUE);
          ctrl->SetTextString(TRANSLATE(("#multiplayer.config.options.size", 3, size, size, teams)), TRUE);
        }
        else
        {
          ctrl = IFace::Find<IControl>("MissionName", PrivData::infoCtrl, TRUE);
          ctrl->SetTextString(L"", FALSE);
          ctrl = IFace::Find<IControl>("MissionSize", PrivData::infoCtrl, TRUE);
          ctrl->SetTextString(L"", FALSE);
        }
      }
    }


    //
    // Error handler for ExtractPlayerName
    //
    static void FASTCALL ErrorHandler(const char *, U32, U32, const char *)
    {
    }


    //
    // Parse a token
    //
    static void ParseTokenBuffer(TBuf tBuf, const char **playerName, const char **rest)
    {
      // Use first token for player name
      if (tBuf.NextToken() != TR_OK)
      {
        return;
      }
      *playerName = tBuf.lastToken;

      // Store remainder of string
      *rest = tBuf.CurrentStr();
    }


    //
    // Extract player name from first arg of a string
    //
    Bool ExtractPlayerName(const char *str, U32 len, const char **playerName, const char **rest)
    {
      TBuf tBuf;

      // Reset pointers
      *playerName = *rest = NULL;

      // Extract first string as player name
      tBuf.Setup(str, len, "Msg", ErrorHandler);

      // Set character types
      tBuf.SetCharType(PTREE_PUNCTUATION, TBuf::PUNCTUATION);

      // Process the command
      try
      {
        ParseTokenBuffer(tBuf, playerName, rest);
      } 
      catch (int) {}

      // Finished parsing
      tBuf.Done();

      // Return TRUE if both strings were found
      return (*playerName && *rest);
    }


    //
    // Handler
    //
    void Handler(U32 patchCrc)
    {
      switch (patchCrc)
      {
        case 0xACBE2E8E: // "multiplayer.abort"
        {
          LOG_DIAG(("Aborting"))

          // Reset everything
          MultiPlayer::Reset();
          break;
        }

        case 0x3F37EA6E: // "multiplayer.instant"
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

          break;
        }

        case 0x58706D32: // "multiplayer.login"
        {
          if (CmdLine::host || CmdLine::join)
          {
            isLobby = TRUE;

            if (CmdLine::user.Null())
            {
              // Login the first user
              User::ExistingUser *user = User::GetExistingUsers().GetFirst();
              if (user)
              {
                User::Login(user);
              }
              else
              {
                ERR_FATAL(("No user was specified on the command line and there are no users set"))
              }
            }
            else
            {
              if (!User::Find(CmdLine::user))
              {
                // Didn't exist, create a new user
                User::Create(CmdLine::user);
              }

              User::ExistingUser *user = const_cast<NBinTree<User::ExistingUser> &>(User::GetExistingUsers()).GetFirst();
              if (!user || !User::Login(user))
              {
                ERR_FATAL(("Could not login user '%s'", CmdLine::user.str))
              }
            }
          }

          if (CmdLine::host)
          {
            U16 port = Settings::GetPort();
            if (!CmdLine::port.Null())
            {
              port = U16(Clamp(0, Utils::AtoI(CmdLine::port.str), U16_MAX));
            }

            if (hostInfo)
            {
              LOG_WARN(("Old hostInfo found"))
              delete hostInfo;
              hostInfo = NULL;
            }

            Win32::Socket::Address address("127.0.0.1", port);
            hostInfo = new HostInformation(address, CmdLine::session.str, CmdLine::password.crc, CmdLine::maxPlayers);
          }
          else if (CmdLine::join)
          {
            if (joinInfo)
            {
              LOG_WARN(("Old joinInfo found"))
              delete joinInfo;
              joinInfo = NULL;
            }

            U16 port = Settings::GetPort();
            if (!CmdLine::port.Null())
            {
              port = U16(Clamp(0, Utils::AtoI(CmdLine::port.str), U16_MAX));
            }

            Win32::Socket::Address address(CmdLine::ip.str, port);
            joinInfo = new JoinInformation(address, CmdLine::session.str, CmdLine::password.crc);
          }
          CmdLine::Reset();
          break;
        }

        case 0xB5DFB58C: // "multiplayer.launch"
        {
          ASSERT(!Network::server)
          if (hostInfo)
          {
            Network::Reset();

            // Create the server
            StyxNet::Server::Config config;
            Network::server = new StyxNet::Server(config);
          }
          else if (joinInfo)
          {
            Network::Reset();

            // Connect to the server
            StyxNet::Client::Config config
            (
              Network::GetCurrentName(), 
              joinInfo->address, 
              Settings::GetFirewallStatus() == 
                Settings::FirewallStatus::NotBehindFirewall ? TRUE : FALSE
            );
            Network::client = new StyxNet::Client(config);
            online = TRUE;
          }
          break;
        }

        case 0x56811C2F: // "multiplayer.migrate"
        {
          if (Network::client)
          {
            Network::client->MigrateSession();
          }
          break;
        }

        case 0x40AC48A9: // "multiplayer.checkfirewall"
        {
          const char *name;
          if (Console::GetArgString(1, name))
          {
            IControl *ctrl = IFace::Find<IControl>(name, NULL, TRUE);

            // Test firewall status
            switch (Settings::GetFirewallStatus())
            {
              case Settings::FirewallStatus::BehindFirewall:
                IFace::SendEvent(ctrl, NULL, IFace::NOTIFY, 0x4F48F16E); // "Firewall::Behind"
                break;

              case Settings::FirewallStatus::NotBehindFirewall:
                IFace::SendEvent(ctrl, NULL, IFace::NOTIFY, 0x8F9D855A); // "Firewall::None"
                break;

              case Settings::FirewallStatus::Unknown:
                IFace::SendEvent(ctrl, NULL, IFace::NOTIFY, 0x3B3C35E3); // "Firewall::Unknown"
                break;

              default:
                ERR_FATAL(("Unknown result from GetFirewallStatus"))
            }
          }
          break;
        }

        case 0xC0C90434: // "multiplayer.sizeof"
        {
          CON_DIAG(("SessionData: %d", sizeof (StyxNet::SessionData)))
          CON_DIAG((" - name %d [%d]", sizeof (StyxNet::SessionName), &StyxNet::SessionData::name))
          CON_DIAG((" - flags %d [%d]", sizeof (U16), &StyxNet::SessionData::flags))
          CON_DIAG((" - version %d [%d]", sizeof (U16), &StyxNet::SessionData::version))
          CON_DIAG((" - numUsers %d [%d]", sizeof (U32), &StyxNet::SessionData::numUsers))
          CON_DIAG((" - maxUsers %d [%d]", sizeof (U32), &StyxNet::SessionData::maxUsers))

          CON_DIAG(("Session: %d", sizeof (StyxNet::Session)))
          CON_DIAG((" - address %d [%d]", sizeof (Win32::Socket::Address), &StyxNet::Session::address))
    
          break;
        }

        case 0xE81812BD: // "multiplayer.flags.islocked"
        {
          if (!Cmd::updatingLock && Network::client && Cmd::isHost)
          {
            if (Cmd::isLocked)
            {
              Network::client->LockSession();
            }
            else
            {
              Network::client->UnlockSession();
            }
          }
          break;
        }

        case 0x070CE316: // "multiplayer.register.client"
        {
          const char *name;
          if (Console::GetArgString(1, name))
          {
            PrivData::clientCtrl = IFace::Find<IControl>(name, NULL, TRUE);
          }
          break;
        }

        case 0x2AE622A7: // "multiplayer.register.server"
        {
          const char *name;
          if (Console::GetArgString(1, name))
          {
            PrivData::serverCtrl = IFace::Find<IControl>(name, NULL, TRUE);
          }
          break;
        }

        case 0x8878C9A3: // "multiplayer.register.setup"
        {
          const char *name;
          if (Console::GetArgString(1, name))
          {
            PrivData::gameSetupCtrl = IFace::Find<IControl>(name, NULL, TRUE);
          }
          break;
        }

        case 0xEB9599CF: // "multiplayer.register.transfer"
        {
          const char *name;
          if (Console::GetArgString(1, name))
          {
            PrivData::transferCtrl = IFace::Find<IControl>(name, NULL, TRUE);
          }
          break;
        }

        case 0x687C14B6: // "multiplayer.register.transferlist"
        {
          const char *name;
          if (Console::GetArgString(1, name))
          {
            PrivData::transferListCtrl = IFace::Find<IControl>(name, NULL, TRUE);
          }
          break;
        }

        case 0xA3B7A232: // "multiplayer.register.players"
        {
          const char *name;
          if (Console::GetArgString(1, name))
          {
            PrivData::playersCtrl = IFace::Find<IControl>(name, NULL, TRUE);
          }
          break;
        }

        case 0x753DC3B5: // "multiplayer.register.chat"
        {
          const char *name;
          if (Console::GetArgString(1, name))
          {
            PrivData::chatCtrl = IFace::Find<IControl>(name, NULL, TRUE);
          }
          break;
        }

        case 0x8A1633FE: // "multiplayer.register.updates"
        {
          const char *name;
          if (Console::GetArgString(1, name))
          {
            PrivData::updateCtrl = IFace::Find<IControl>(name, NULL, TRUE);
          }
          break;
        }

        case 0x47AEB6FD: // "multiplayer.register.download"
        {
          const char *name;
          if (Console::GetArgString(1, name))
          {
            PrivData::downloadCtrl = IFace::Find<IControl>(name, NULL, TRUE);
          }
          break;
        }

        case 0x648BF61F: // "multiplayer.register.info"
        {
          const char *name;
          if (Console::GetArgString(1, name))
          {
            PrivData::infoCtrl = IFace::Find<IControl>(name, NULL, TRUE);
          }
          break;
        }

        case 0x1F1DAB22: // "multiplayer.server.start"
        {
          ASSERT(!Network::server)

          if (hostInfo)
          {
            Network::Reset();

            // Create the server
            StyxNet::Server::Config config;
            config.port = Settings::GetPort();
            Network::server = new StyxNet::Server(config);
          }
          break;
        }

        case 0x1486FE62: // "multiplayer.server.connect"
        {
          StyxNet::Client::Config config
          (
            Network::GetCurrentName(), 
            Win32::Socket::Address("127.0.0.1", Settings::GetPort()),
            Settings::GetFirewallStatus() == 
              Settings::FirewallStatus::NotBehindFirewall ? TRUE : FALSE
          );
          
          const char *address;
          if (Console::GetArgString(1, address))
          {
            S32 port = Settings::GetPort();
            Console::GetArgInteger(2, port);

            config.address = Win32::Socket::Address(address, U16(port));
          }

          // Create the client
          Network::client = new StyxNet::Client(config);
          online = TRUE;
          break;
        }

        case 0x74266931: // "multiplayer.server.joinconnect"
        {
          if (joinInfo)
          {
            Network::Reset();

            // Connect to the server
            StyxNet::Client::Config config
            (
              Network::GetCurrentName(), 
              joinInfo->address, 
              Settings::GetFirewallStatus() == 
                Settings::FirewallStatus::NotBehindFirewall ? TRUE : FALSE
            );

            Network::client = new StyxNet::Client(config);
            online = TRUE;
          }
          break;
        }

        case 0x31E64445: // "multiplayer.session.connect"
        {
          ASSERT(Network::client)

          ASSERT(Network::client)
          ASSERT(hostInfo || joinInfo)

          if (hostInfo)
          {
            Network::client->CreateSession(hostInfo->session.str, hostInfo->password, hostInfo->maxUsers);
          }
          else if (joinInfo)
          {
            // Do we need a password ?
            if (joinInfo->flags & StyxNet::SessionFlags::Password)
            {
              // Send it to the registered interface control
              if (PrivData::clientCtrl.Alive())
              {
                IFace::SendEvent(PrivData::clientCtrl, NULL, IFace::NOTIFY, 0x5A263875); // "MultiPlayer::PasswordRequired"
              }
            }
            else
            {
              Network::client->JoinSession(joinInfo->session.str, 0);
            }
          }
          break;
        }


        case 0xDB3D3B75: // "multiplayer.session.joinpassword"
        {
          ASSERT(Network::client)
          ASSERT(joinInfo)

          const char *password;
          if (Console::GetArgString(1, password))
          {
            Network::client->JoinSession(joinInfo->session.str, Crc::CalcStr(password));
          }
          else
          {
            ERR_FATAL(("multiplayer.session.joinpassword password"))
          }
          break;
        }

        case 0x03F9FFF5: // "multiplayer.session.joindownload"
        {
          const char *ctrl;

          if (Console::GetArgString(1, ctrl))
          {
            // The ctrl should be a multiplayer session list

            Controls::Sessions *sessions = IFace::Find<Controls::Sessions>(ctrl, NULL, TRUE);
            StyxNet::Explorer::Session *session = sessions->GetSelectedSession();

            if (!session)
            {
              // Send it to the registered interface control
              if (PrivData::clientCtrl.Alive())
              {
                IFace::SendEvent(PrivData::clientCtrl, NULL, IFace::NOTIFY, 0xD20EDD94); // "MultiPlayer::NoSelectedSession"
              }
            }
            else
            {
              if (joinInfo)
              {
                LOG_WARN(("Old joinInfo found"))
                delete joinInfo;
              }
              joinInfo = new JoinInformation(session->address, session->name, session->flags);
            }
          }
          else
          {
            ERR_FATAL(("multiplayer.session.download control"))
          }
          break;
        }

        case 0x09A43BF2: // "multiplayer.session.createdownload"
        {
          const char *name;
          const char *password;
          S32 maxUsers;

          if 
          (
            Console::GetArgString(1, name) &&
            Console::GetArgString(2, password) &&
            Console::GetArgInteger(3, maxUsers)
          )
          {
            // Build up the hosting information
            if (hostInfo)
            {
              LOG_WARN(("Old hostInfo found"))
              delete hostInfo;
              hostInfo = NULL;
            }

            // Check the name
            if (Utils::Strlen(name) < 3)
            {
              // Send it to the registered interface control
              if (PrivData::clientCtrl.Alive())
              {
                IFace::SendEvent(PrivData::clientCtrl, NULL, IFace::NOTIFY, 0x8A77211B); // "MultiPlayer::SessionNameTooShort"
              }
            }
            else
            {
              Win32::Socket::Address address;
              hostInfo = new HostInformation(address, name, Crc::CalcStr(password), maxUsers);
            }
          }
          else
          {
            ERR_FATAL(("multiplayer.session.createdownload name password maxUsers"))
          }
          break;
        }

        case 0xFA5A6EC2: // "multiplayer.setup.created"
        {
          if (PrivData::gameSetupCtrl.Alive())
          {
            GameTime::Synchronize();
            Main::SetBackgroundProcessing(TRUE);
          }
          break;
        }

        case 0x3750A06E: // "multiplayer.setup.joined"
        {
          if (PrivData::gameSetupCtrl.Alive())
          {
            GameTime::Synchronize();
            Main::SetBackgroundProcessing(TRUE);
          }
          break;
        }

        case 0xBDE8207B: // "multiplayer.setup.leaving"
        {
          if (PrivData::gameSetupCtrl.Alive())
          {
            PrivData::gameSetupCtrl.Clear();

            // Reset
            MultiPlayer::Reset();

            Main::SetBackgroundProcessing(FALSE);
          }
          break;
        }

        case 0x81A7DD71: // "multiplayer.setup.addai"
          Host::CreateAITeam();
          break;

        case 0x3AC9FEF2: // "multiplayer.setup.removeai"
          break;

        case 0xE79E9A11: // "multiplayer.setup.fillai"
          Host::FillAITeams();
          break;

        case 0xC3D5A5B3: // "multiplayer.setup.activated"
          if (Network::client)
          {
            // Set the title of the game we're in
            const char *session = Network::client->GetSessionName();
            if (session)
            {
              // Update the chat window
              if (PrivData::chatCtrl.Alive())
              {
                PrivData::chatCtrl->SetTextString(TRANSLATE(("#multiplayer.config.chat.title", 1, Utils::Ansi2Unicode(session))), TRUE);
              }
            }
          }
          break;

        case 0x3C76198F: // "multiplayer.setup.launch"
          Data::Send(Commands::LaunchReady, 0, NULL, TRUE);
          if (Cmd::isHost)
          {
            Data::Send(Commands::Launch, 0, NULL, TRUE);
          }
          break;

        case 0xBE14F836: // "multiplayer.setup.suggestmission"
        {
          const char *control;
          if (Console::GetArgString(1, control))
          {
            Game::MissionSelection *missions = IFace::Find<Game::MissionSelection>(control, NULL, TRUE);
            const Missions::Mission *m = missions->GetSelectedMission();
            char buf[200];
            Utils::Sprintf(buf, 200, "suggests %s [%s]", m->GetName().str, m->GetGroup().GetPath().str);
            Data::Send(Commands::MessageQuote, Utils::Strlen(buf) + 1, (const U8 *) buf, FALSE);
          }
          break;
        }

        case 0x641AC993: // "multiplayer.setup.getmission"
          if (Cmd::isHost)
          {
            const char *control;

            if (Console::GetArgString(1, control))
            {
              Game::MissionSelection *missions = IFace::Find<Game::MissionSelection>(control, NULL, TRUE);

              if (const Missions::Mission *m = MultiPlayer::Host::GetMission())
              {
                missions->SetSelectedMission(m);
              }
            }
          }
          break;

        case 0x40D898A9: // "multiplayer.setup.setmission"
          if (Cmd::isHost)
          {
            const char *control;

            if (Console::GetArgString(1, control))
            {
              Game::MissionSelection *missions = IFace::Find<Game::MissionSelection>(control, NULL, TRUE);
              const Missions::Mission *m = missions->GetSelectedMission();

              VarString varPlacement("$.placement", missions);
              VarString varDifficulty("$.difficulty", missions);
              VarString varRuleSet("$.ruleSet", missions);
              VarInteger varStartCredits("$.credits", missions);
              VarString varUnitLimit("$.unitLimit", missions);
              VarString varResourceType("$.resourceType", missions);

              U32 startCredits = Clamp<U32>(0, varStartCredits, 1000000);
              U32 unitLimit = Clamp<U32>(1, Utils::AtoI(varUnitLimit), 2000);

              MultiPlayer::Host::InvalidateLaunchReadyness();
              if (m)
              {
                MultiPlayer::Host::ChangeMission(m);
              }

              MultiPlayer::Host::SendUpdatedSettings
              (
                Crc::CalcStr(varPlacement),
                Crc::CalcStr(varDifficulty),
                Crc::CalcStr(varRuleSet),
                startCredits,
                unitLimit,
                Crc::CalcStr(varResourceType)
              );
            }
          }
          break;

        case 0xE8837E00: // "multiplayer.setup.setrandommission"
          if (Cmd::isHost)
          {
            const char *folder;
            if (Console::GetArgString(1, folder))
            {
              const Missions::Mission *m = Missions::GetRandomMission(Crc::CalcStr(folder));

              if (m)
              {
                MultiPlayer::Host::ChangeMission(m);
              }

              // Using the preview information, update the ruleset
              if (PrivData::preview)
              {
                MultiPlayer::Host::SendUpdatedRuleSet(PrivData::preview->GetRuleSet().crc);
              }
            }
          }
          break;

        case 0x35B2721E: // "multiplayer.setup.clearmission"
          if (Cmd::isHost)
          {
            MultiPlayer::Host::InvalidateLaunchReadyness();
            MultiPlayer::Host::ClearMission();
          }
          break;

        case 0x4F5B9452: // "multiplayer.cmd.me"
        {
          if (MultiPlayer::Data::Online())
          {
            const char * str;
            if (Console::GetArgString(1, str))
            {
              U32 len = Utils::Strlen(str);
              if (len)
              {
                Data::Send(Commands::MessageQuote, len + 1, (const U8 *) str, FALSE);
              }
            }
          }
          break;
        }

        case 0xA1CF6DA6: // "multiplayer.cmd.msg"
        {
          if (MultiPlayer::Data::Online())
          {
            // Send a private message to a player
            const char *str;;
            if (Console::GetArgString(1, str))
            {
              U32 len = Utils::Strlen(str);
              if (len)
              {
                // The second argument is the name of the player to send to
                const char *playerName;
                const char *chat;

                if (ExtractPlayerName(str, len, &playerName, &chat))
                {
                  // Find the player whose name matches (case insensitive)
                  Network::Player *player = Network::GetPlayers().Find(Crc::CalcStr(playerName));

                  CH playerBuff[128];
                  Utils::Ansi2Unicode(playerBuff, 128, playerName);

                  if (player)
                  {
                    U32 id = player->GetId();
                    Data::Send(1, &id, Commands::MessagePrivate, Utils::Strlen(chat) + 1, (const U8 *) chat);
                   
                    // Echo locally
                    CONSOLE(0xAD95B2DF, (TRANSLATE(("#multiplayer.chat.privmsg", 2, playerBuff, Utils::Ansi2Unicode(chat) )) )) // "ChatLocal"
                  }
                  else
                  {
                    // Could not find player with the name!
                    // "MultiError"
                    CONSOLE(0xB2178C6E, (TRANSLATE(("#multiplayer.chat.error.noplayer", 1, playerBuff)) ))
                  }
                }
              }
            }
          }
          break;
        }

        case 0x030326B9: // "multiplayer.cmd.broadcast"
        {
          if (MultiPlayer::Data::Online())
          {
            const char *str;
            if (Console::GetArgString(1, str))
            {
              U32 len = Utils::Strlen(str);
              if (len)
              {
                Data::Send(Commands::Message, len + 1, (const U8 *) str, FALSE);
              }
            }
          }
          break;
        }

        case 0x6333DFF1: // "multiplayer.cmd.whois"
          // Get some information on a player
          break;

        case 0x38005E6C: // "multiplayer.cmd.team"
        case 0x491B83E0: // "multiplayer.cmd.t"
        {
          if (MultiPlayer::Data::Online() && Network::HaveCurrentPlayer())
          {
            // Send a message to all team members
            const char *str;
            if (Console::GetArgString(1, str))
            {
              U32 len = Utils::Strlen(str);
              if (len)
              {
                // Send a message to all team members
                BinTree<GroupTree> groups;
                BuildPlayerHierachy(groups);

                const PlayerInfo *playerInfo;
                if (Data::Get(&playerInfo, Network::GetCurrentPlayer().GetId()))
                {
                  // Find the group which contains the team this player is on
                  for (BinTree<GroupTree>::Iterator gti(&groups); *gti; gti++)
                  {
                    TeamTree *teamTree = (*gti)->teams.Find(playerInfo->teamId);
                    if (teamTree)
                    {
                      U32 ids[256];
                      U8 index = 0;

                      // Add the other players on the team
                      for (BinTree<const Player>::Iterator pi(&teamTree->players); *pi; pi++)
                      {
                        if (pi.GetKey() != Network::GetCurrentPlayer().GetId())
                        {
                          ids[index++] = pi.GetKey();
                        }
                      }

                      // Echo locally
                      CONSOLE(0xAD95B2DF, (TRANSLATE(("#multiplayer.chat.teammsg", 1, Utils::Ansi2Unicode(str) )) )) // "ChatLocal"

                      if (index)
                      {
                        Data::Send(index, ids, Commands::MessageTeam, len + 1, (const U8 *) str);
                      }
                    }
                  }
                }
                groups.DisposeAll();
              }
            }
          }
          break;
        }

        case 0x6C3D2DF2: // "multiplayer.cmd.enemies"
        case 0x01CB4527: // "multiplayer.cmd.e"
        {
          if (MultiPlayer::Data::Online() && Network::HaveCurrentPlayer())
          {
            // Send a message to all other group members
            const char *str;
            if (Console::GetArgString(1, str))
            {
              BinTree<GroupTree> groups;
              BuildPlayerHierachy(groups);

              const PlayerInfo *playerInfo;
              if (Data::Get(&playerInfo, Network::GetCurrentPlayer().GetId()))
              {
                U32 ids[256];
                U8 index = 0;

                for (BinTree<GroupTree>::Iterator gti(&groups); *gti; gti++)
                {
                  // Does this group not contain this players teams ?
                  TeamTree *teamTree = (*gti)->teams.Find(playerInfo->teamId);

                  if (!teamTree)
                  {
                    // Iterate all of the teams in this group
                    for (BinTree<TeamTree>::Iterator tti(&(*gti)->teams); *tti; tti++)
                    {
                      const Team *team;
                      Data::Get(&team, tti.GetKey());
                      ASSERT(team)

                      // Add all of the players in the team
                      for (BinTree<const Player>::Iterator pi(&(*tti)->players); *pi; pi++)
                      {
                        if (pi.GetKey() != Network::GetCurrentPlayer().GetId())
                        {
                          ids[index++] = pi.GetKey();
                        }
                      }
                    }
                  }
                }

                // Echo locally
                CONSOLE(0xAD95B2DF, (TRANSLATE(("#multiplayer.chat.enemymsg", 1, Utils::Ansi2Unicode(str) )) )) // "ChatLocal"

                if (index)
                {
                  Data::Send(index, ids, Commands::MessageGroup, Utils::Strlen(str) + 1, (const U8 *) str);
                }
              }
              groups.DisposeAll();
            }
          }
          break;
        }

        case 0x978C9A44: // "multiplayer.cmd.allies"
        case 0x12CF33FB: // "multiplayer.cmd.a"
        {
          if (MultiPlayer::Data::Online() && Network::HaveCurrentPlayer())
          {
            // Send a message to all group members
            const char *str;
            if (Console::GetArgString(1, str))
            {
              BinTree<GroupTree> groups;
              BuildPlayerHierachy(groups);

              const PlayerInfo *playerInfo;
              if (Data::Get(&playerInfo, Network::GetCurrentPlayer().GetId()))
              {
                for (BinTree<GroupTree>::Iterator gti(&groups); *gti; gti++)
                {
                  // Does this group contain this players teams ?
                  TeamTree *teamTree = (*gti)->teams.Find(playerInfo->teamId);

                  if (teamTree)
                  {
                    U32 ids[256];
                    U8 index = 0;

                    // Iterate all of the teams in this group
                    for (BinTree<TeamTree>::Iterator tti(&(*gti)->teams); *tti; tti++)
                    {
                      const Team *team;
                      Data::Get(&team, tti.GetKey());
                      ASSERT(team)

                      // Add all of the players in the team
                      for (BinTree<const Player>::Iterator pi(&(*tti)->players); *pi; pi++)
                      {
                        if (pi.GetKey() != Network::GetCurrentPlayer().GetId())
                        {
                          ids[index++] = pi.GetKey();
                        }
                      }
                    }

                    // Echo locally
                    CONSOLE(0xAD95B2DF, (TRANSLATE(("#multiplayer.chat.allymsg", 1, Utils::Ansi2Unicode(str) )) )) // "ChatLocal"
  
                    if (index)
                    {
                      Data::Send(index, ids, Commands::MessageGroup, Utils::Strlen(str) + 1, (const U8 *) str);
                    }
                  }
                }
              }
              groups.DisposeAll();
            }
          }
          break;
        }

        case 0x4CC59107: // "multiplayer.cmd.won"
        {
          const char *str;
          if (Console::GetArgString(1, str))
          {
            U32 len = Utils::Strlen(str);
            if (len)
            {
              WonIface::BroadcastMessage(Utils::Ansi2Unicode(str));
            }
          }
          break;
        }

        case 0x99AF1037: // "multiplayer.cmd.kick"
        case 0x3D45E52D: // "multiplayer.cmd.k"

          if (Cmd::isHost)
          {
            if (MultiPlayer::Data::Online())
            {
              // Attempt to kick the player
              const char *playerName;
              if (Console::GetArgString(1, playerName))
              {
                U32 player = Crc::CalcStr(playerName);
                Network::Player *p = Network::GetPlayers().Find(player);

                if (p)
                {
                  char buf[200];
                  Utils::Sprintf(buf, 200, "kicked '%s' from the game", p->GetName());
                  Data::Send(Commands::MessageQuote, Utils::Strlen(buf) + 1, (const U8 *) buf, FALSE);

                  Network::client->KickUser(player);
                }
              }
            }
          }
          break;

        case 0x2AA25944: // "multiplayer.cmd.help"
          CONSOLE(0xAD95B2DF, ("Available commands:")) // "ChatLocal"

          CONSOLE(0xAD95B2DF, ("/me message")) // "ChatLocal"
          CONSOLE(0xAD95B2DF, ("  Emote a message")) // "ChatLocal"

          CONSOLE(0xAD95B2DF, ("/msg player message")) // "ChatLocal"
          CONSOLE(0xAD95B2DF, ("  Send a private message to a player")) // "ChatLocal"

          CONSOLE(0xAD95B2DF, ("/team (or t) message")) // "ChatLocal"
          CONSOLE(0xAD95B2DF, ("  Send a message to your co-op teammates")) // "ChatLocal"

          CONSOLE(0xAD95B2DF, ("/allies (or a) message")) // "ChatLocal"
          CONSOLE(0xAD95B2DF, ("  Send a message to your allies")) // "ChatLocal"

          CONSOLE(0xAD95B2DF, ("/enemies (or e) message")) // "ChatLocal"
          CONSOLE(0xAD95B2DF, ("  Send a message to your enemies")) // "ChatLocal"

          CONSOLE(0xAD95B2DF, ("/won message")) // "ChatLocal"
          CONSOLE(0xAD95B2DF, ("  Send a message to the WON room")) // "ChatLocal"

          CONSOLE(0xAD95B2DF, ("/kick (or k) player")) // "ChatLocal"
          CONSOLE(0xAD95B2DF, ("  Kick a player from the game (host only)")) // "ChatLocal"
          break;

        case 0x53AEBE8C: // "multiplayer.download.motd"
          Download::GetMotd();
          break;

        case 0x35733C9E: // "multiplayer.download.updates"
          Download::GetUpdates();
          break;
      }
    }
  }
}
