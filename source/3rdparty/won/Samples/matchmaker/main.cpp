#define MAIN_CPP
#pragma warning (disable : 4786)

#ifdef WIN32
#include <conio.h>
#elif defined(_LINUX)
//#include <sys/poll.h>
#include <stdio.h>
#include <termios.h> 
#elif defined(macintosh) && (macintosh == 1)
#include <console.h>
#else
#error unknown platform 
#endif // platform switch
#include <common/won.h>
#include <stdarg.h>
#include "main.h"
#include "matchmaker.h"
#include "OutputQueue.h"

#if defined(_LINUX) && defined(_USE_CURSES)
#include <curses.h>
#endif // _LINUX && _USE_CURSES

namespace {
	using namespace std;
	using WONCommon::WONString;
};

void HandleCommand(char* theCommandP, unsigned int theLength);

Matchmaker* gMatchmaker;
OutputQueue gOutputQueue;
#ifdef WIN32
HANDLE      gStdOutH = GetStdHandle(STD_OUTPUT_HANDLE);
#endif // WIN32

const char* COMMAND_CREATE_ACCOUNT = "CREATEACCOUNT";
const char* COMMAND_LOGIN          = "LOGIN";
const char* COMMAND_HELP           = "HELP";
const char* COMMAND_QUIT           = "QUIT";
const char* COMMAND_LIST_ROOMS     = "ROOMS";
const char* COMMAND_CREATE_ROOM    = "CREATEROOM";
const char* COMMAND_JOIN_ROOM      = "JOINROOM";
const char* COMMAND_GET_ROOM_NAME  = "ROOMNAME";
const char* COMMAND_LIST_USERS     = "USERS";
const char* COMMAND_LIST_GAMES     = "GAMES";
const char* COMMAND_CREATE_GAME    = "CREATEGAME";
const char* COMMAND_JOIN_GAME      = "JOINGAME";
const char* COMMAND_LEAVE_GAME     = "LEAVEGAME";
const char* COMMAND_START_GAME     = "STARTGAME";
const char* COMMAND_MOVE           = "MOVE";
const char* COMMAND_TOP_TEN        = "TOPTEN";
const char* COMMAND_IGNORE_USER    = "IGNORE";
const char* COMMAND_UNIGNORE_USER  = "UNIGNORE";

#ifdef _LINUX
#ifndef _USE_CURSES
int getch()
{
	int c, i, fd=0;
	struct termios term, oterm;
  
	// get the terminal settings
	tcgetattr(fd, &oterm);

	// get a copy of the settings, which we modify
	memcpy(&term, &oterm, sizeof(term));

	// put the terminal in non-canonical mode, any 
	// reads will wait until a character has been
	// pressed. This function will not time out
	term.c_lflag = term.c_lflag & (!ICANON);
	term.c_cc[VMIN] = 1;
	term.c_cc[VTIME] = 0;
	tcsetattr(fd, TCSANOW, &term);

	// get a character. c is the character
	c=getchar();

	// reset the terminal to its original state
	tcsetattr(fd, TCSANOW, &oterm);

	// return the charcter
	return c;
}
#endif // !_USE_CURSES
int kbhit(void)
{
	int c = getch();

	/* if we retrieved a character, put it back on
	   the input stream */
	if (c != -1)
#if defined(_LINUX) && defined (_USE_CURSES)
		ungetch(c);
#else
		ungetc(c, stdin);
#endif // _LINUX && _USE_CURSES

	/* return 1 if the keyboard was hit, or 0 if it
	   was not hit */
	return ((c!=-1)?1:0);
}
/*
bool kbhit()
{
	pollfd aPollStruct;
	aPollStruct.fd = 0; // stdin
	aPollStruct.events = POLLIN;
	aPollStruct.revents = 0;
	return poll(&aPollStruct, 1, 0) > 0;
}
*/
#endif // _LINUX

int main(void)
{
	WONInitialize();
	gMatchmaker = new Matchmaker;
	if (!gMatchmaker->Init())
	{
		OutputString("Failed to Initialize.  Shutting down.");
		gOutputQueue.Flush();
		return 1;
	}
	
	// Check if the current version of the application is valid.  If the version is not valid,
	// the user should be instructed to patch.  
	if (!gMatchmaker->IsValidVersion("Version1"))
	{
		OutputString("Your version is invalid.  Retrieving patch.");
		gMatchmaker->GetPatch();
		// You would want to sit here until the patch download completed.  Keep the UI 
		// responsive of course, but don't allow the user to do any matchmaking using the
		// out of date version.
	}

#if defined(_LINUX) && defined(_USE_CURSES)
	initscr();
	int aCursesError = start_color();
	aCursesError = cbreak();
	aCursesError = noecho();
	aCursesError = intrflush(stdscr, FALSE);
	aCursesError = nodelay(stdscr, TRUE);
	aCursesError = keypad(stdscr, TRUE);

	short COLOR_PROMPT = 1, COLOR_COMMAND = 2, COLOR_OUTPUT = 3;
	aCursesError = init_pair(COLOR_PROMPT,  COLOR_RED,   COLOR_BLACK);
	aCursesError = init_pair(COLOR_COMMAND, COLOR_BLUE,  COLOR_BLACK);
	aCursesError = init_pair(COLOR_OUTPUT,  COLOR_GREEN, COLOR_BLACK);
#endif // _LINUX && _USE_CURSES

	const int aStrSize = 1024;
	char aStr[aStrSize+1];
	char aChar;
	int aPos = 0;

	while(true) {
		// output any lines of text that have queued up while the user was entering data
		gOutputQueue.Flush();

//#if defined(macintosh) && (macintosh == 1)
//		while(aPos>0)
//		{
//			WONPump(1);
//#else
		while(aPos>0 || kbhit())
		{
//#endif // mac?
//#ifdef _LINUX
//			aChar = getchar();
//#else
			aChar = getch();
//#endif // _LINUX
			if (aChar == '\b')      // handle backspace
			{
				if(aPos>0) {
					aPos--;
					if(aPos==0) SAMPLE_PRINTF("\b\b\b   \b\b\b"); // erase the '> ';
					else SAMPLE_PRINTF("\b \b"); // just erase the last character
				}
			}
#ifdef _LINUX
			else if (aChar == '\n')
#else
			else if (aChar == '\r')      // handle EOL
#endif // _LINUX
			{
				SAMPLE_PRINTF("\n");

				if(aPos>0) {
					if(aStr[0]=='/')  // commands start with '/'
					{
#ifdef WIN32
						SetConsoleTextAttribute(gStdOutH,FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#elif defined(_LINUX) && defined(_USE_CURSES)
						color_set(COLOR_OUTPUT, NULL);
#endif // platform switch
						HandleCommand(aStr+1,aPos-1);
					}
#ifdef DLLSAMPLE
					else if (WONRoutingIsRegistered(gMatchmaker->mRoutingServer))
					{
						aStr[aPos] = '\0';
						WONRoutingBroadcastChatA(gMatchmaker->mRoutingServer, aStr, FALSE);
					}
#else
					else if (gMatchmaker->mRoutingServer.IsRegistered())
						gMatchmaker->mRoutingServer.BroadcastChat(string(aStr,aPos), false);
#endif // DLLSAMPLE
					else
						cout << "Not connected to chat" << endl;

					aPos = 0;
				}

				break;
			}
			else
			{
				if(aPos<aStrSize) {
#ifdef WIN32
					SetConsoleTextAttribute(gStdOutH,FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#elif defined(_LINUX) && defined(_USE_CURSES)
					color_set(COLOR_PROMPT, NULL);
#endif // platform switch
					if(aPos==0) SAMPLE_PRINTF("> ");
					aStr[aPos] = aChar;
					aPos++;

#ifdef WIN32
					SetConsoleTextAttribute(gStdOutH,FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#elif defined(_LINUX) && defined(_USE_CURSES)
					color_set(COLOR_COMMAND, NULL);
#endif // platform switch
					SAMPLE_PRINTF("%c",aChar);
				}
			}
		}

#ifdef WIN32
		Sleep(50); // FIX: bogus!
#elif defined _LINUX
		usleep(50);
#elif defined(macintosh) && (macintosh == 1)
# ifdef DLLSAMPLE
		WONPump(50);
		sleep(1);
# else
		WSSocket::Pump();
# endif // DLLSAMPLE
#endif // platform switch
	}

#if defined(_LINUX) && defined(_USE_CURSES)
	aCursesError = endwin();
#endif // _LINUX && _USE_CURSES

	return 0;
}

void HandleCommand(char* theCommandP, unsigned int theLength)
{
	int i = 0;
	for( ; i < theLength && !isspace(theCommandP[i]); ++i) 
		theCommandP[i] = toupper(theCommandP[i]);

	theCommandP[i] = '\0';  
	theCommandP[theLength] = '\0';

	if (strcmp(theCommandP, COMMAND_QUIT) == 0)
	{
		OutputString("Bye");
		gOutputQueue.Flush();
		delete gMatchmaker;
		WONTerminate();
		exit(0);
	}
	else if (strcmp(theCommandP, COMMAND_CREATE_ACCOUNT) == 0)
	{
		if (strlen(COMMAND_CREATE_ACCOUNT) + 1 < theLength)
		{
			char* aUsernameP = theCommandP + strlen(COMMAND_CREATE_ACCOUNT) + 1;
			char* aPasswordP = strchr(aUsernameP, ' ');
			if (aPasswordP) *(aPasswordP++) = '\0';
			gMatchmaker->CreateWONAccount(aUsernameP, aPasswordP);
		}
		else OutputString("You must specify a username and password for the new account.");
	}
	else if (strcmp(theCommandP, COMMAND_LOGIN) == 0)
	{
		if (strlen(COMMAND_LOGIN) + 1 < theLength)
		{
			char* aLoginDataP = theCommandP + strlen(COMMAND_LOGIN) + 1;
			char* aSpaceP = strchr(aLoginDataP, ' ');
			if (aSpaceP) *aSpaceP = '\0';
			gMatchmaker->Login(aLoginDataP, aSpaceP ? aSpaceP+1 : "");
		}
		else OutputString ("You must specify the username and password of the account that you want to use.");
	}
	else if (strcmp(theCommandP, COMMAND_LIST_USERS) == 0)
		gMatchmaker->ListUsers();
	else if (strcmp(theCommandP, COMMAND_CREATE_ROOM) == 0)
	{
		if (strlen(COMMAND_CREATE_ROOM) + 1 < theLength)
		{
			char* aRoomDataP = theCommandP + strlen(COMMAND_CREATE_ROOM) + 1;
			char* aSpaceP = strchr(aRoomDataP, ' ');
			WONString aPassword;
			if (aSpaceP)
			{
				*aSpaceP = '\0';
				aPassword.assign(aSpaceP+1);
			}
 
			gMatchmaker->CreateRoom(aRoomDataP, aPassword.GetUnicodeCString());
		}
		else OutputString("You must specify the name of your new room.");
	}
	else if (strcmp(theCommandP, COMMAND_LIST_ROOMS) == 0)
		gMatchmaker->ListRooms();
	else if (strcmp(theCommandP, COMMAND_JOIN_ROOM) == 0)
	{
		if (strlen(COMMAND_JOIN_ROOM) + 1 < theLength)
		{
			char* aRoomDataP = theCommandP + strlen(COMMAND_JOIN_ROOM) + 1;
			char* aSpaceP = strchr(aRoomDataP, ' ');
			WONString aPassword;
			if (aSpaceP)
			{
				*aSpaceP = '\0';
				aPassword.assign(aSpaceP+1);
			}

			gMatchmaker->JoinRoom(aRoomDataP, aPassword.GetUnicodeCString());
		}
		else OutputString("You must specify the name of the room to join.");
	}
	else if (strcmp(theCommandP, COMMAND_GET_ROOM_NAME) == 0)
	{
		if (gMatchmaker->GetCurrentChatRoomName().empty())
			OutputString("Not connected to a chat room.");
		else
			OutputStringF("You are currently in room: %S", gMatchmaker->GetCurrentChatRoomName().GetUnicodeCString());
	}
	else if (strcmp(theCommandP, COMMAND_CREATE_GAME) == 0)
	{
		if (strlen(COMMAND_CREATE_GAME) + 1 < theLength)
		{
			char* aGameDataP = theCommandP + strlen(COMMAND_CREATE_GAME) + 1;
			char* aSpaceP = strchr(aGameDataP, ' ');
			WONString aPassword;
			if (aSpaceP)
			{
				*aSpaceP = '\0';
				aPassword.assign(aSpaceP+1);
			}

			gMatchmaker->CreateGame(aGameDataP, aPassword.GetUnicodeCString());
		}
		else OutputString("You must specify the name of your new game.");
	}
	else if (strcmp(theCommandP, COMMAND_JOIN_GAME) == 0)
	{
		if (strlen(COMMAND_JOIN_GAME) + 1 < theLength)
		{
			char* aGameDataP = theCommandP + strlen(COMMAND_JOIN_GAME) + 1;
			char* aSpaceP = strchr(aGameDataP, ' ');
			WONString aPassword;
			if (aSpaceP)
			{
				*aSpaceP = '\0';
				aPassword.assign(aSpaceP+1);
			}

			gMatchmaker->JoinGame(aGameDataP, aPassword.GetUnicodeCString());
		}
		else OutputString("You must specify the name of the game to join.");
	}
	else if (strcmp(theCommandP, COMMAND_LEAVE_GAME) == 0)
		gMatchmaker->LeaveGame();
	else if (strcmp(theCommandP, COMMAND_LIST_GAMES) == 0)
		gMatchmaker->ListGames();
	else if (strcmp(theCommandP, COMMAND_START_GAME) == 0)
		gMatchmaker->StartGame();
	else if (strcmp(theCommandP, COMMAND_MOVE) == 0)
		gMatchmaker->Move(theCommandP + strlen(COMMAND_MOVE) + 1);
	else if (strcmp(theCommandP, COMMAND_TOP_TEN) == 0)
		gMatchmaker->DisplayTopTen();
	else if (strcmp(theCommandP, COMMAND_IGNORE_USER) == 0)
		gMatchmaker->IgnoreUser(theCommandP + strlen(COMMAND_IGNORE_USER) + 1);
	else if (strcmp(theCommandP, COMMAND_UNIGNORE_USER) == 0)
		gMatchmaker->Move(theCommandP + strlen(COMMAND_UNIGNORE_USER) + 1);
	else if (strcmp(theCommandP, COMMAND_HELP) == 0 || strcmp(theCommandP, "?") == 0) {
		OutputString("Commands:");
		OutputStringF("/%s - get this help message.", COMMAND_HELP);
		OutputStringF("/%s - quit the program.", COMMAND_QUIT);
		OutputStringF("/%s <username> <password> - create a new WON acccount.", COMMAND_CREATE_ACCOUNT);
		OutputStringF("/%s <username> <password> - specify username and password to use.", COMMAND_LOGIN);
		OutputStringF("/%s - get a list of rooms.", COMMAND_LIST_ROOMS);
		OutputStringF("/%s <room name> <password> - create a new chat room.", COMMAND_CREATE_ROOM);
		OutputStringF("/%s <room name> <password> - join the specified chat room.", COMMAND_JOIN_ROOM);
		OutputStringF("/%s - display the room name.", COMMAND_GET_ROOM_NAME);
		OutputStringF("/%s - get a list of users in current chat room.", COMMAND_LIST_USERS);
		OutputStringF("/%s <username> - ignore the specified user.", COMMAND_IGNORE_USER);
		OutputStringF("/%s <username> - unignore the specified user.", COMMAND_UNIGNORE_USER);
		OutputStringF("/%s <game name> <password> - create a new game.", COMMAND_CREATE_GAME);
		OutputStringF("/%s <game name> <password> - join the specified game.", COMMAND_JOIN_GAME);
		OutputStringF("/%s - leave the game that you are currently in.", COMMAND_LEAVE_GAME);
		OutputStringF("/%s - get a list of games in current chat room.", COMMAND_LIST_GAMES);
		OutputStringF("/%s - start the game if you're the captain.", COMMAND_START_GAME);
		OutputStringF("/%s <position> - claim square specified by position (ex. A1)", COMMAND_MOVE);
		OutputStringF("/%s - outputs top ten list.", COMMAND_TOP_TEN);
	}
	else 
		OutputString("Unknown command.");
}

void OutputString(const char* string)
{
	gOutputQueue.push_back(string);
}

void OutputString(const wchar_t* string)
{
	OutputString(WONCommon::WStringToString(string).c_str());
}

void OutputStringF(const char* format, ...)
{
	static char buffer[512];

	va_list arglist;
	va_start(arglist, format);
	vsprintf(buffer, format, arglist);
	OutputString(buffer);
	va_end(arglist);
}

void OutputError(const char* string)
{
	OutputString(string);
}

void OutputError(const char* string, int error)
{
	static char buffer[512];
	sprintf(buffer, "%s: %s", string, WONErrorToString(error));
	OutputError(buffer);
}

#ifdef DLLSAMPLE
void OutputServerError(const char* string, const WONIPAddress& theAddrR, int error)
#else
void OutputServerError(const char* string, const WONAPI::IPSocket::Address& theAddrR, int error)
#endif // DLLSAMPLE
{
	static char buffer[512];
#ifdef DLLSAMPLE
	unsigned short aBufLen = 512;
	WONIPAddressGetDottedIP(&theAddrR, buffer, &aBufLen, TRUE);
	sprintf(buffer, "%s (server: %s)", string, buffer);
#else
	sprintf(buffer, "%s (server: %s)", string, theAddrR.GetAddressString().c_str());
#endif // DLLSAMPLE
	OutputError(buffer, error);
}

void OutputFile(const char* theFilePath)
{
	FILE* aFileH = fopen(theFilePath, "r");
	char buf[256];
	while (TRUE)
	{
		size_t aBytesRead = fread(buf, 1, 255, aFileH);
		
		if (aBytesRead == 0)
			break;

		buf[aBytesRead] = '\0';
		OutputString(buf);
	}
}
