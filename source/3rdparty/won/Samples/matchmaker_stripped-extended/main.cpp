#define MAIN_CPP
#pragma warning (disable : 4786)

// TITAN INCLUDES
#include <common/won.h>
#include <stdarg.h>
#include "main.h"
#include "matchmaker.h"
#include "OutputQueue.h"
#include "matchmakerex.h"

// PLATFORM SPECIFIC INCLUDES
#ifdef WIN32
	#include <conio.h>

#elif defined(_LINUX)
	// *** LINUX ***
	//#include <sys/poll.h>
	#include <stdio.h>
	#include <termios.h> 
	
	#ifdef _USE_CURSES
		#include <curses.h>
	#endif

#elif defined(macintosh) && (macintosh == 1)
	// *** MACINTOSH ***
	#include <console.h>

#else
	// *** UNKNOWN PLATFORM ***
	#error unknown platform 
#endif // platform switch

// MISC INCLUDES
#if defined(_LINUX) && defined(_USE_CURSES)
#include <curses.h>
#endif // _LINUX && _USE_CURSES





// TITAN NAMESPACES
namespace {
	using namespace std;
	using WONCommon::WONString;
};

// GLOBALS and PROTOTYPES
MatchmakerEx* gMatchmaker;
OutputQueue gOutputQueue;

#ifdef WIN32
HANDLE      gStdOutH = GetStdHandle(STD_OUTPUT_HANDLE);
#endif // WIN32

void HandleCommand(char* theCommand, unsigned int theLength);
int GetCommandToken(string& theDestString, char* theSourceBuffer, 
					int theTokenNum, BOOL copyToEnd);



// COMMAND LIST
const char* COMMAND_HELP           = "HELP";
const char* COMMAND_QUIT           = "QUIT";

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
	gMatchmaker = new MatchmakerEx;

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





/********************************************************************************
 * Command processor.															*
 ********************************************************************************/
void HandleCommand(char* theInputBuf, unsigned int theLength)
{
	// Stores the command string
	string theCommand;

	// Append a null terminator to theInputBuf
	theInputBuf[theLength] = '\0';

	// Extract the command string from theInputBuf
	GetCommandToken(theCommand, theInputBuf,0,FALSE);

	// Make the command all uppercase
	for (int charnum =0; charnum < theCommand.size(); charnum++)
		theCommand[charnum] = toupper(theCommand[charnum]);

	// Handler routines (APPLICATION COMMANDS)
	if (theCommand==COMMAND_QUIT)
	{
		OutputString("Bye");
		gOutputQueue.Flush();
		delete gMatchmaker;
		WONTerminate();
		exit(0);
	}
	else if (theCommand==COMMAND_HELP || theCommand== "?") 
	{
		OutputString("Commands:");
		OutputStringF("/%s - get this help message.", COMMAND_HELP);
		OutputStringF("/%s - quit the program.", COMMAND_QUIT);

		// Forward Help commands to other objects
		gMatchmaker->HandleCommand(theCommand,theInputBuf);
	}
	else if (!gMatchmaker->HandleCommand(theCommand,theInputBuf))
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


/********************************************************************************
 * Isolates a single token (word) from a char*, stores result in a string		*
 ********************************************************************************/
int GetCommandToken(string& theDestString, char* theSourceBuffer, 
					int theTokenNum, BOOL copyToEnd)
{
	int aTokenStartPos		= 0;	// the position of the first letter in the token
	int aTokenEndPos		=-2;	// the position of the last letter in the token
	int aTokenCounter		=-1;	// number of last token found (Base 0)

	// Create a string copy of the source buffer
	string theSourceString;
	theSourceString.assign(theSourceBuffer);

	do
	{
		// Extract a single token, skip over double delimiters (exa: "/login one  two")
		do
		{
			// A token starts at *2* spaces after the previous end (to skip the space)
			aTokenStartPos = aTokenEndPos+2;

			// A token ends at the character *before* the separating space
			aTokenEndPos = theSourceString.find_first_of(' ',aTokenStartPos)-1;
		}
		// The start and end positions are the same when it's a double space "1  2"
		while(aTokenEndPos+1 == aTokenStartPos);

		// one token extracted
		aTokenCounter++;
	}
	// Loop until we've either extract the token specified or we're out of input	
	while(aTokenCounter<theTokenNum && aTokenEndPos != -2);

	// Did we run out of input?
	if(aTokenCounter != theTokenNum)
		return -1; 

	// Should we copy the rest of the string?
	if(aTokenEndPos==-2 || copyToEnd==TRUE)
		theDestString = theSourceString.substr(aTokenStartPos);
	else 
		// or just the single token?		 (start position,-------length to copy--------)
		theDestString = theSourceString.substr(aTokenStartPos,aTokenEndPos-aTokenStartPos+1);

	// Send back the number of bytes extracted 
	return theDestString.size();
}

