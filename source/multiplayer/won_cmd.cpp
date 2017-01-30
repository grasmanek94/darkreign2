///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// WON Commands
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "won_private.h"
#include "won_cmd.h"
#include "won_controls_roomlist.h"
#include "won_controls_gamelist.h"
#include "won_controls_playerlist.h"
#include "varsys.h"
#include "woniface.h"
#include "iface.h"
#include "iclistbox.h"
#include "user.h"
#include "multiplayer_cmd_private.h"
#include "multiplayer_download.h"
#include "stdload.h"
#include "babel.h"
#include "version.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Won
//
namespace Won
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

    // Username/Password
    static VarString wonUsername;
    static VarString wonPassword;

    // Is the cd key valid?
    static Bool validKey = FALSE;

    // Room name
    static WonIface::RoomName roomName;

    // Config idents
    static GameIdent configUser("WON::User");


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

      VarSys::RegisterHandler("won", Handler);

      // Initial setup
      VarSys::RegisterHandler("won.register", Handler);
      VarSys::CreateCmd("won.register.connect");
      VarSys::CreateCmd("won.register.players");
      VarSys::CreateCmd("won.register.games");
      VarSys::CreateCmd("won.register.chat");

      // Connect to Won

      VarSys::CreateCmd("won.log");

      VarSys::CreateCmd("won.abort");
      VarSys::CreateCmd("won.setup");
      VarSys::CreateCmd("won.connect");
      VarSys::CreateCmd("won.disconnect");
      VarSys::CreateCmd("won.login");
      VarSys::CreateCmd("won.createaccount");
      VarSys::CreateCmd("won.changepassword");
      VarSys::CreateCmd("won.updaterooms");

      VarSys::CreateCmd("won.rebuildrooms");
      VarSys::CreateCmd("won.rebuildgames");
      VarSys::CreateCmd("won.rebuildplayers");
      
      VarSys::CreateCmd("won.createroom");
      VarSys::CreateCmd("won.joinroom");
      VarSys::CreateCmd("won.joinroompassword");
      VarSys::CreateCmd("won.joingame");

      VarSys::CreateCmd("won.ignore");
      VarSys::CreateCmd("won.unignore");

      VarSys::CreateCmd("won.pingallgames");

      VarSys::CreateCmd("won.keepalive");

      VarSys::CreateCmd("won.checkkey");

      // Chat messages
      VarSys::RegisterHandler("won.cmd", Handler);
      VarSys::CreateCmd("won.cmd.broadcast", VarSys::RAWDATA);
      VarSys::CreateCmd("won.cmd.me", VarSys::RAWDATA);
      VarSys::CreateCmd("won.cmd.msg", VarSys::RAWDATA);

      VarSys::CreateCmd("won.cmd.ignore");
      VarSys::CreateCmd("won.cmd.ig");
      VarSys::CreateCmd("won.cmd.unignore");
      VarSys::CreateCmd("won.cmd.unig");

      VarSys::CreateCmd("won.cmd.help");

      VarSys::CreateString("won.username", "", VarSys::DEFAULT, &wonUsername);
      VarSys::CreateString("won.password", "", VarSys::DEFAULT, &wonPassword);

      initialized = TRUE;
    }


    //
    // Shutdown
    //
    void Done()
    {
      ASSERT(initialized)

      // Delete the multiplayer scope
      VarSys::DeleteItem("won");

      initialized = FALSE;
    }


    //
    // GetUsername
    //
    const char * GetUsername()
    {
      return (wonUsername);
    }


    //
    // Is the cd key valid?
    //
    Bool IsKeyValid()
    {
      return (validKey);
    }


    //
    // Handler
    //
    void Handler(U32 patchCrc)
    {
      switch (patchCrc)
      {
        case 0xB353C7D6: // "won.register.connect"
        {
          const char *name;
          if (Console::GetArgString(1, name))
          {
            wonConnectCtrl = IFace::Find<IControl>(name, NULL, TRUE);
          }
          break;
        }

        case 0x24336AB1: // "won.register.players"
        {
          const char *name;
          if (Console::GetArgString(1, name))
          {
            wonPlayersCtrl = IFace::Find<ICListBox>(name, NULL, TRUE);
          }
          break;
        }

        case 0x3BF301F0: // "won.register.games"
        {
          const char *name;
          if (Console::GetArgString(1, name))
          {
            wonGamesCtrl = IFace::Find<IControl>(name, NULL, TRUE);
          }
          break;
        }

        case 0x714D1A77: // "won.register.chat"
        {
          const char *name;
          if (Console::GetArgString(1, name))
          {
            wonChatCtrl = IFace::Find<IControl>(name, NULL, TRUE);
          }
          break;
        }

        case 0x0342792D: // "won.log"
        {
          S32 on = TRUE;
          Console::GetArgInteger(1, on);
          WonIface::Logging(on);
          break;
        }

        case 0x94DB728D: // "won.abort"
          Abort();
          WonIface::Abort();
          MultiPlayer::Download::Abort();
          break;

        case 0xAC7C1C97: // "won.setup"
        {
          // Upload the usernames from the current registered user
          FScope *fScope = User::GetConfigScope(configUser.crc);
          if (fScope)
          {
            wonUsername = StdLoad::TypeString(fScope, "UserName");
            wonPassword = StdLoad::TypeString(fScope, "Password");
          }
          else
          {
            wonUsername = User::GetName();
            wonPassword = "";
          }

          // Load the won configuration
          LoadConfig();
          break;
        }

        case 0x97606E06: // "won.connect"
        {
          // Tell the interface to connect us to WON
          WonIface::Connect();
          break;
        }

        case 0x732C5B6E: // "won.disconnect"
        {
          // Tell the interface to disconnect us from WON
          WonIface::Disconnect();
          break;
        }

        case 0x60153131: // "won.login"
        {
          const char *username;
          const char *password;
          if (
            Console::GetArgString(1, username) && 
            Console::GetArgString(2, password))
          {
            WonIface::LoginAccount(username, password);

            // Save the WON username/password
            FScope fScope(NULL, configUser.str);
            StdSave::TypeString(&fScope, "UserName", username);
            StdSave::TypeString(&fScope, "Password", password);
            User::SetConfigScope(configUser.crc, &fScope);

            wonUsername = username;
            wonPassword = password;
          }
          else
          {
            ERR_FATAL(("won.login username password"))
          }
          break;
        }

        case 0x112FA496: // "won.createaccount"
        {
          const char *username;
          const char *password1;
          const char *password2;

          if (
            Console::GetArgString(1, username) && 
            Console::GetArgString(2, password1) && 
            Console::GetArgString(3, password2))
          {
            // Check to see that they made both passwords the same
            if (Utils::Strcmp(password1, password2))
            {
              // Tell the interface about it
              IFace::PostNotifyModal(NULL, 0x35A9A18D); // "Error::CreateAccountPasswordMismatch""
            }
            else
            {
              // Tell the interface to create an account
              WonIface::CreateAccount(username, password1);

              // Save the WON username/password
              FScope fScope(NULL, configUser.str);
              StdSave::TypeString(&fScope, "UserName", username);
              StdSave::TypeString(&fScope, "Password", password1);
              User::SetConfigScope(configUser.crc, &fScope);

              wonUsername = username;
              wonPassword = password1;
            }
          }
          else
          {
            ERR_FATAL(("won.createaccount username password passwordverify"))
          }
          break;
        }

        case 0xB607B837: // "won.changepassword"
        {
          const char *username;
          const char *oldPassword;
          const char *newPassword1;
          const char *newPassword2;
          if (
            Console::GetArgString(1, username) && 
            Console::GetArgString(2, oldPassword) && 
            Console::GetArgString(3, newPassword1) && 
            Console::GetArgString(4, newPassword2))
          {
            // Check to see that they made both passwords the same
            if (Utils::Strcmp(newPassword1, newPassword2))
            {
              // Tell the interface about it
              IFace::PostNotifyModal(NULL, 0x38F60295); // "Error::ChangePasswordPasswordMismatch""
            }
            else
            {
              // Tell the interface to create 
              WonIface::ChangePassword(username, oldPassword, newPassword2);

              // Save the WON username/password
              FScope fScope(NULL, configUser.str);
              StdSave::TypeString(&fScope, "UserName", username);
              StdSave::TypeString(&fScope, "Password", newPassword1);
              User::SetConfigScope(configUser.crc, &fScope);

              wonUsername = username;
              wonPassword = newPassword1;
            }
          }
          else
          {
            ERR_FATAL(("won.changepassword usename oldpassword password passwordverify"))
          }
          break;
        }

        case 0x50012714: // "won.updaterooms"
        {
          WonIface::UpdateRooms();
          break;
        }

        case 0x1E5E8E38: // "won.rebuildrooms"
        {
          const char *control;
          if (Console::GetArgString(1, control))
          {
            Controls::RoomList *roomList = IFace::Find<Controls::RoomList>(control, NULL, TRUE);

            NList<WonIface::Room> rooms(&WonIface::Room::node);
            WonIface::GetRoomList(rooms);

            ICListBox::Rebuild *rebuild = roomList->PreRebuild();
            roomList->DeleteAllItems();

            for (NList<WonIface::Room>::Iterator r(&rooms); *r; ++r)
            {
              IControl *newCtrl = new Controls::RoomList::Item(*roomList, **r, roomList);

              if ((*r)->lobby)
              {
                char buf[80];
                Utils::Sprintf(buf, 80, "\x1%s", Utils::Unicode2Ansi((*r)->name.str));
                roomList->AddItem(buf, newCtrl);
              }
              else
              {
                roomList->AddItem(Utils::Unicode2Ansi((*r)->name.str), newCtrl);
              }
            }

            roomList->Sort();
            roomList->PostRebuild(&rebuild);

            rooms.DisposeAll();
          }
          break;
        }

        case 0x448982B0: // "won.rebuildgames"
        {
          const char *control;
          if (Console::GetArgString(1, control))
          {
            Controls::GameList *gameList = IFace::Find<Controls::GameList>(control, NULL, TRUE);

            NList<WonIface::Game> games(&WonIface::Game::node);
            WonIface::GetGameList(games);

            ICListBox::Rebuild *rebuild = gameList->PreRebuild();
            gameList->ClearSelected();

            // Mark all of the games as old
            NList<IControl>::Iterator i(&gameList->GetItems());
            for (; *i; ++i)
            {
              Controls::GameList::Item *item = IFace::Promote<Controls::GameList::Item>(*i);
              if (item)
              {
                item->old = TRUE;
              }
            }

            // Add all of the new games and also check to see 
            // if any of the new games are already in the list
            for (NList<WonIface::Game>::Iterator g(&games); *g; ++g)
            {
              StyxNet::Session *session = (StyxNet::Session *) (*g)->data;

              #ifndef DEVELOPMENT

              // We're only interested in games with a matching version
              if (session->version != U16(Version::GetBuildNumber()))
              {
                break;
              }

              #endif

              char buf[80];

              U8 val = 1;
              if (session->flags & StyxNet::SessionFlags::Password)
              {
                val += 1;
              }
              if (session->flags & StyxNet::SessionFlags::Locked)
              {
                val += 2;
              }

              Utils::Sprintf(buf, 80, "%d%s", val, (*g)->name.str);
              Bool dupe = FALSE;
              for (!i; *i; ++i)
              {
                Controls::GameList::Item *item = IFace::Promote<Controls::GameList::Item>(*i);
                if (item)
                {
                  if (item->GetSession().address == (*(StyxNet::Session *) (*g)->data).address)
                  {
                    item->old = FALSE;
                    dupe = TRUE;

                    // Copy over updated session information
                    item->GetSession() = *session;
                    item->SetName(buf);
                    break;
                  }
                }
              }

              if (!dupe)
              {
                IControl *newCtrl = new Controls::GameList::Item(*gameList, **g, gameList);
                gameList->AddItem(buf, newCtrl);
              }
            }

            // Remove all of the items which are still dead
            for (!i; *i; ++i)
            {
              Controls::GameList::Item *item = IFace::Promote<Controls::GameList::Item>(*i);
              if (item)
              {
                if (item->old)
                {
                  item->MarkForDeletion();
                }
              }
            }

            gameList->Sort();
            gameList->PostRebuild(&rebuild);

            games.DisposeAll();
          }
          break;
        }

        case 0xA47E5DD1: // "won.rebuildplayers"
        {
          const char *control;
          if (Console::GetArgString(1, control))
          {
            Controls::PlayerList *playerList = IFace::Find<Controls::PlayerList>(control, NULL, TRUE);

            NList<WonIface::Player> players(&WonIface::Player::node);
            WonIface::GetPlayerList(players);

            ICListBox::Rebuild *rebuild = playerList->PreRebuild();
            playerList->DeleteAllItems();

            for (NList<WonIface::Player>::Iterator p(&players); *p; ++p)
            {
              CH buff[64];
              Utils::Sprintf(buff, 64, L"%s", (*p)->name.str);
              IControl *newCtrl = new Controls::PlayerList::Item(*playerList, (*p)->moderator, (*p)->muted, (*p)->ignored);
              newCtrl->SetTextString((*p)->name.str, TRUE);
              playerList->AddItem(Utils::Unicode2AnsiEncode(buff), newCtrl);
            }

            playerList->Sort();
            playerList->PostRebuild(&rebuild);

            players.DisposeAll();
          }
          break;
        }

        case 0x9164D365: // "won.createroom"
        {
          const char *name;
          const char *password;
          if (
            Console::GetArgString(1, name) && 
            Console::GetArgString(2, password))
          {
            // Check the name
            if (Utils::Strlen(name) < 3)
            {
              // Send it to the registered interface control
              if (wonConnectCtrl.Alive())
              {
                IFace::SendEvent(wonConnectCtrl, NULL, IFace::NOTIFY, 0xE7F0BC86); // "Error::CreateRoomNameTooShort"
              }
            }
            else
            {
              WonIface::RoomName room = Utils::Ansi2Unicode(name);
              WonIface::PasswordStr passwd = Utils::Ansi2Unicode(password);

              WonIface::CreateRoom(room.str, passwd.str);
            }
          }
          else
          {
            ERR_FATAL(("won.createroom name password"))
          }
          break;
        }

        case 0xB39DCC76: // "won.joinroom"
        {
          const char *control;
          if (Console::GetArgString(1, control))
          {
            Controls::RoomList *roomList = IFace::Find<Controls::RoomList>(control, NULL, TRUE);

            // Get the currently selected room
            WonIface::Room *room = roomList->GetSelectedRoom();

            if (room)
            {
              roomName = room->name.str;

              if (room->password)
              {
                // Send it to the registered interface control
                if (wonConnectCtrl.Alive())
                {
                  IFace::SendEvent(wonConnectCtrl, NULL, IFace::NOTIFY, 0x259B0041); // "JoinRoom::PasswordRequired"
                }
              }
              else
              {
                WonIface::JoinRoom(room->name.str, L"");
              }
            }
          }
          else
          {
            ERR_FATAL(("won.joinroom control"))
          }
          break;
        }

        case 0x1280A8B4: // "won.joinroompassword"
        {
          const char *password;

          if (Console::GetArgString(1, password))
          {
            WonIface::PasswordStr passwd = Utils::Ansi2Unicode(password);
            WonIface::JoinRoom(roomName.str, passwd.str);
          }
          else
          {
            ERR_FATAL(("won.joinroompassword password"))
          }
          break;
        }

        case 0x68FDCBFA: // "won.joingame"
        {
          const char *control;
          if (Console::GetArgString(1, control))
          {
            Controls::GameList *gameList = IFace::Find<Controls::GameList>(control, NULL, TRUE);

            // Get the currently selected game
            Controls::GameList::Item *game = gameList->GetSelectedGame();

            if (game)
            {
              LOG_DIAG(("Joining game %s on machine %s:%d", game->GetSession().name.str, game->GetSession().address.GetText(), game->GetSession().address.GetPort()))

              if (MultiPlayer::Cmd::joinInfo)
              {
                LOG_WARN(("Old joinInfo found"))
                delete MultiPlayer::Cmd::joinInfo;
              }
              MultiPlayer::Cmd::joinInfo = new MultiPlayer::Cmd::JoinInformation(game->GetSession().address, game->GetSession().name, game->GetSession().flags);
            }
          }
          else
          {
            ERR_FATAL(("won.joingame control"))
          }
          break;
        }

        case 0x7B0B35F9: // "won.ignore"
        {
          const char *control;
          if (Console::GetArgString(1, control))
          {
            ICListBox *playerList = IFace::Find<ICListBox>(control, NULL, TRUE);

            // Get the currently selected game
            IControl *ctrl = playerList->GetSelectedItem();
            if (ctrl)
            {
              WonIface::IgnorePlayer(ctrl->GetTextString());
              CONSOLE(0x4ECF28BA, ("Now ignoring '%s'", Utils::Unicode2Ansi(ctrl->GetTextString())) ) // "WonChatLocal"
            }
          }
          else
          {
            ERR_FATAL(("won.ignore control"))
          }
          break;
        }

        case 0x52F999F8: // "won.unignore"
        {
          const char *control;
          if (Console::GetArgString(1, control))
          {
            ICListBox *playerList = IFace::Find<ICListBox>(control, NULL, TRUE);

            // Get the currently selected game
            IControl *ctrl = playerList->GetSelectedItem();
            if (ctrl)
            {
              WonIface::UnignorePlayer(ctrl->GetTextString());
              CONSOLE(0x4ECF28BA, ("Now unignoring '%s'", Utils::Unicode2Ansi(ctrl->GetTextString())) ) // "WonChatLocal"
            }
          }
          else
          {
            ERR_FATAL(("won.unignore control"))
          }
          break;
        }

        case 0x9ABE39E6: // "won.pingallgames"
        {
          const char *control;
          if (Console::GetArgString(1, control))
          {
            Controls::GameList *gameList = IFace::Find<Controls::GameList>(control, NULL, TRUE);
            gameList->PingAll();
          }          
          break;
        }

        case 0x1D2ECE5B: // "won.keepalive"
        {
          WonIface::KeepAlive();
          break;
        }

        case 0xE97B9899: // "won.checkkey"
        {
          const char *key;
          const char *control;
          if (Console::GetArgString(1, key) && Console::GetArgString(2, control))
          {
            IControl *ctrl = IFace::Find<IControl>(control, NULL, TRUE);

            if (WonIface::CheckKey(key))
            {
              // Key is now valid
              validKey = TRUE;

              IFace::SendEvent(ctrl, NULL, IFace::NOTIFY, 0xD037F5EE); // "Won::KeySuccess"
            }
            else
            {
              IFace::SendEvent(ctrl, NULL, IFace::NOTIFY, 0x35F01904); // "Won::KeyFailure"
            }
          }
          else
          {
            ERR_FATAL(("won.checkkey key control"))
          }
          break;
        }

        case 0x0773FF7B: // "won.cmd.broadcast"
        {
          const char *str;
          if (Console::GetArgString(1, str))
          {
            U32 len = Utils::Strlen(str);
            if (len)
            {
              // Check to see if there's a player selected
              if (wonPlayersCtrl.Alive())
              {
                IControl *p = wonPlayersCtrl->GetSelectedItem();
                if (p)
                {
                  if (wonPlayersCtrl->GetSelectedItem())
                  {
                    CH chatBuff[512];
                    Utils::Ansi2Unicode(chatBuff, 512, str);

                    WonIface::PrivateMessage(Utils::Unicode2Ansi(p->GetTextString()), chatBuff);

                    // Echo locally
                    CONSOLE(0x4ECF28BA, (TRANSLATE(("#won.chat.privmsg", 2, p->GetTextString(), chatBuff)) )) // "WonChatLocal"
                    break;
                  }
                }
              }
              WonIface::BroadcastMessage(Utils::Ansi2Unicode(str));
            }
          }
          break;
        }

        case 0xD83D177A: // "won.cmd.me"
        {
          const char *str;
          if (Console::GetArgString(1, str))
          {
            U32 len = Utils::Strlen(str);
            if (len)
            {
              WonIface::EmoteMessage(Utils::Ansi2Unicode(str));
            }
          }
          break;
        }

        case 0xFC162E3D: // "won.cmd.msg"
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

              if (MultiPlayer::Cmd::ExtractPlayerName(str, len, &playerName, &chat))
              {
                WonIface::PrivateMessage(playerName, Utils::Ansi2Unicode(chat));

                CH playerBuff[128];
                Utils::Ansi2Unicode(playerBuff, 128, playerName);

                CH chatBuff[512];
                Utils::Ansi2Unicode(chatBuff, 512, chat);

                // Echo locally
                CONSOLE(0x4ECF28BA, (TRANSLATE(("#won.chat.privmsg", 2, playerBuff, chatBuff)) )) // "WonChatLocal"
              }
            }
          }
          break;
        }

        case 0x57238681: // "won.cmd.ignore"
        case 0x949B0DBD: // "won.cmd.ig"
        {
          const char *playerName;
          if (Console::GetArgString(1, playerName))
          {
            WonIface::IgnorePlayer(playerName);
            CONSOLE(0x4ECF28BA, ("Now ignoring '%s'", playerName) ) // "WonChatLocal"
          }
          break;
        }

        case 0xB065967D: // "won.cmd.unignore"
        case 0xDD1A763A: // "won.cmd.unig"
        {
          const char *playerName;
          if (Console::GetArgString(1, playerName))
          {
            WonIface::UnignorePlayer(playerName);
            CONSOLE(0x4ECF28BA, ("Now unignoring '%s'", playerName) ) // "WonChatLocal"
          }
          break;
        }

        case 0xBABBEF90: // "won.cmd.help"
        {
          CONSOLE(0x4ECF28BA, ("Available commands:")) // "WonChatLocal"

          CONSOLE(0x4ECF28BA, ("/me message")) // "WonChatLocal"
          CONSOLE(0x4ECF28BA, ("  Emote a message")) // "WonChatLocal"

          CONSOLE(0x4ECF28BA, ("/msg player message")) // "WonChatLocal"
          CONSOLE(0x4ECF28BA, ("  Send a private message to a player")) // "WonChatLocal"

          CONSOLE(0x4ECF28BA, ("/ig(nore) player")) // "WonChatLocal"
          CONSOLE(0x4ECF28BA, ("  Ignore messages from a player")) // "WonChatLocal"

          CONSOLE(0x4ECF28BA, ("/unig(nore) player")) // "WonChatLocal"
          CONSOLE(0x4ECF28BA, ("  Unignore messages from a player")) // "WonChatLocal"
          
          break;
        }
      }
    }
  }
}
