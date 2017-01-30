// Standards includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string>

// Titan includes
//#include <common/won.h>
#include "Factory/FactoryAPI.h"
#include "Directory/DirectoryAPI.h"			// for DirServer calls
#include "Routing/RoutingAPI.h"				// for Routing server calls (chat)
#include <msg/ServerStatus.h>				// included for WONMsg namespace

namespace
{
	using namespace std;
	using namespace WONAPI;
	using namespace WONMsg;
	using namespace WONCommon;
}

// App and User status 
#define AS_INITIALIZING			0				// setting up
#define AS_NORMAL				1				// normal command input and process

// Console colors
#define CC_SYSTEM				WORD(FOREGROUND_GREEN | BACKGROUND_BLUE)
#define CC_USERCHAT				WORD(FOREGROUND_RED | BACKGROUND_BLUE)
#define CC_OTHERCHAT			WORD(FOREGROUND_RED | BACKGROUND_BLUE)
#define CC_USERECHO				WORD(FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE | BACKGROUND_BLUE)
