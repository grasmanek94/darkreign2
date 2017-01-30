// User.cpp: implementation of the CUser class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable:4786)
#pragma warning (disable:4275)

#include "stdafx.h"
#include "Console.h"
#include "User.h"
#include "ChatSampleApp.h"

#define ADDCOMMAND(a,b)			m_CmdMap[a] = b
extern int GetCommandToken(string&,char*,int,BOOL = FALSE);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUser::CUser()
{
	m_IsLoggedIn			= FALSE;
	m_IsConnectedToChat		= FALSE;

}

CUser::~CUser()
{

}





/********************************************************************************
 *	Call the user specified command												*
 ********************************************************************************/
int CUser::ProcessCommand(string& theCommand)
{
	// Call the attached command function in the command map
	int(CUser::*pCommand)(void) = m_CmdMap[theCommand];

	if (pCommand)
		return (this->*pCommand)();
	else
		return DoChatCmd();

	// Should never reach this point
	return FALSE;
}
 
 
 
 
 
/********************************************************************************
 *	Initialize the user object													*
 ********************************************************************************/
BOOL CUser::Init()
{
	// Initialize the command list;
	InitCommands();

	return TRUE;
}





/********************************************************************************
 * Initialize the list of user input triggered commands.						*
 ********************************************************************************/
void CUser::InitCommands()
{
	/*
	ADDCOMMAND(The String,  The Function to call);
	*/
	ADDCOMMAND("/create",	&CUser::DoCreateCmd);		// Start a routing server
	ADDCOMMAND("/help",		&CUser::DoHelpCmd);			// Display help info
	ADDCOMMAND("/?",		&CUser::DoHelpCmd);			// Alias for help command
	ADDCOMMAND("/exit",		&CUser::DoExitCmd);			// Shutdown the app
	ADDCOMMAND("/quit",		&CUser::DoExitCmd);			// Alias for shutdown
	ADDCOMMAND("/join",		&CUser::DoJoinCmd);			// Connect to a routing server
	ADDCOMMAND("/login",	&CUser::DoLoginCmd);		// Login to won
	ADDCOMMAND("/rooms",	&CUser::DoRoomsCmd);		// List routing servers
	ADDCOMMAND("/users",	&CUser::DoUsersCmd);		// List clients on a routing server
	ADDCOMMAND("/whisper",	&CUser::DoWhisperCmd);		// Send chat to a single user
	ADDCOMMAND("/.",		&CUser::DoWhisperCmd);		// Alias for whisper
	ADDCOMMAND("/emote",	&CUser::DoBlankCmd);		// ** Display an action

	// ** Not implemented
};





/********************************************************************************
 *	DoBlankCmd()																*
 *																				*
 *	Purpose:																	*
 *		A default user command function for non implemented commands.			*
 ********************************************************************************/
int CUser::DoBlankCmd()
{
	extern CChatSampleApp* theAppP;

	//Set the System color
	theAppP->m_Console.SetColor(CC_SYSTEM);
	printf("Command not implemented.\r\n");

	return TRUE;
}





/********************************************************************************
 *	DoHelpCmd()																	*
 *																				*
 *	Purpose:																	*
 *		Displays the command list for the user.									*
 ********************************************************************************/
int CUser::DoHelpCmd()
{
	extern CChatSampleApp* theAppP;

	//Set the System color
	theAppP->m_Console.SetColor(CC_SYSTEM);

	// Display a list of commands
	printf("\r\nValid Commands: \r\n");

	// Iterate through the command map and print the trigger string for each one
	for(TCommandMap::const_iterator theCmdMapItr = m_CmdMap.begin();
			theCmdMapItr != m_CmdMap.end();
			++theCmdMapItr)
	{
		printf(theCmdMapItr->first.c_str());
		printf("\t");
	}

	printf("\r\n");
	return TRUE;
}





/********************************************************************************
 *	DoExitCmd()																	*
 *																				*
 *	Purpose:																	*
 *		Called from user "/quit" or "/exit". Terminates the application.		*
 ********************************************************************************/
int CUser::DoExitCmd()
{
	extern CChatSampleApp* theAppP;

	string anExitMessage;							// stores string after "/exit "

	// Extract the exit string
	GetCommandToken(anExitMessage,theAppP->m_InputBuf,1,TRUE);

	//Set the System color
	theAppP->m_Console.SetColor(CC_SYSTEM);

	// Display and log
	theAppP->ReportLog("Exiting with logged string: ");
	theAppP->ReportLog(anExitMessage.c_str());

	// return false so the application will terminate
	return FALSE;
}





/********************************************************************************
 *	DoLoginCmd()																*
 *																				*
 *	Purpose:																	*
 *		Logs the user in to Titan system. Validates username and password.		*
 ********************************************************************************/
int CUser::DoLoginCmd()
{
	extern CChatSampleApp* theAppP;
	
	string aUserName;			// Stores the username
	string aPassword;			// Stores the password

	//Set the System color
	theAppP->m_Console.SetColor(CC_SYSTEM);

	//Check the command syntax (command string plus two tokens)
	if( GetCommandToken(aUserName,theAppP->m_InputBuf,1)==-1 || 
		GetCommandToken(aPassword,theAppP->m_InputBuf,2)==-1)
	{
		printf("usage /login: <username> <password>\r\n");
		return TRUE;
	}

	//Notify user of the login attempt
	theAppP->ReportLog("Attemping login with ");
	theAppP->ReportLog(aUserName.c_str());
	theAppP->ReportLog(" ");
	theAppP->ReportLog(aPassword.c_str());
	theAppP->ReportLog("\r\n");

	//Initialize the user identity
	m_Identity = Identity( (LPCSTR)aUserName.c_str(), "WON", (LPCSTR)aPassword.c_str(), "", 
		theAppP->m_pAuthServers, theAppP->m_nNumAuthServers);

	// Authenticate the user
	Error theResult = m_Identity.Authenticate(false , false, 3000);

	switch(theResult)
	{
	case StatusCommon_Success:
		theAppP->ReportLog("Login success. Now logged in as ");
		theAppP->ReportLog(aUserName.c_str());
		theAppP->ReportLog("\r\n");
		m_IsLoggedIn = TRUE;

		m_Identity = Identity( aUserName, "WON", aPassword, "", 
			theAppP->m_pAuthServers, theAppP->m_nNumAuthServers);

		DoRoomsCmd();

		break;
	case StatusAuth_UserNotFound:
		theAppP->ReportLog("Login failed: Username not found.\r\n");
		break;
	case StatusAuth_BadPassword:
		theAppP->ReportLog("Login failed: Bad Password.\r\n");
		break;
	case StatusAuth_CDKeyInUse:
		theAppP->ReportLog("Login failed: CD Key is in use.\r\n");
		break;
	case StatusAuth_CRCFailed:
		theAppP->ReportLog("Login failed: CRC check failed.\r\n");
		break;
	default:
		theAppP->ReportLog("Login failed: Reason unknown.\r\n");
	}

	return TRUE;
}





/********************************************************************************
 *	DoChatCmd()																	*
 *																				*
 *	Purpose:																	*
 *		Broadcasts a chat message to the chat room.								*
 ********************************************************************************/
int CUser::DoChatCmd()
{
	extern CChatSampleApp* theAppP;

	// User has to be in a chat room to chat
	if(!m_IsConnectedToChat)
	{
		//Set the System color
		theAppP->m_Console.SetColor(CC_SYSTEM);

		printf("Not connected to a chat room.\r\n");
		return TRUE;
	} 
	
	//Set the USERECHO Color
	theAppP->m_Console.SetColor(CC_USERECHO);

	// the user message
	string aChatString;							

	// get the rest of the line
	GetCommandToken(aChatString,theAppP->m_InputBuf,0,TRUE);	

	// Are we registered with the routing server?
	if (theAppP->mRoutingServer.IsRegistered())
		// Broadcast the chat message
		theAppP->mRoutingServer.BroadcastChat(aChatString, false);


	return TRUE;
}





/********************************************************************************
 *	DoRoomsCmd()																*
 *																				*
 *	Purpose:																	*
 *		Display a list of chat rooms that the user may enter.					*
 ********************************************************************************/
int CUser::DoRoomsCmd()
{
	// For this command we need to get deep into some application members
	extern CChatSampleApp* theAppP;
	
	return theAppP->DisplayRooms();
}





/********************************************************************************
 *	DoJoinCmd()																	*
 *																				*
 *	Purpose:																	*
 *		Joins a chat room.														*
 ********************************************************************************/
int CUser::DoJoinCmd()
{
	// For this command we need to get deep into some application members
	extern CChatSampleApp* theAppP;

	// set the console color
	theAppP->m_Console.SetColor(CC_SYSTEM);

	// extract the room name and password (if any) from the input string
	string aRoomName, aRoomPassword;

	// Get the room name (required)
	if (GetCommandToken(aRoomName,theAppP->m_InputBuf,1)==-1)
	{
		// no room was specified
		printf("usage: /join <roomname> [password]\r\n");
		return TRUE;
	} 
	
	// Get the password
	GetCommandToken(aRoomPassword, theAppP->m_InputBuf,2);

	theAppP->JoinRoom(aRoomName, aRoomPassword);

	return TRUE;
}





/********************************************************************************
 *	DoCreateCmd()																*
 *																				*
 *	Purpose:																	*
 *		Creates a chat room.													*
 ********************************************************************************/
int CUser::DoCreateCmd()
{
	extern CChatSampleApp* theAppP;			// Our application object
	string aRoomName, aRoomPassword;		// store the room name and password
	
	// Set the system color for messages
	theAppP->m_Console.SetColor(CC_SYSTEM);

	// User must login before creating a room
	if (!m_IsLoggedIn)
	{
		printf("** Not Logged In **\r\n");
		return TRUE;
	}
	
	// Load the RoomName from the input buffer
	if(GetCommandToken(aRoomName, theAppP->m_InputBuf,1) == -1)
	{
		// Parameters are incorrect
		printf("usage /createroom: <roomname> [password]\r\n");
		return TRUE;
	}

	// Load the Password from the input buffer (if any)
	GetCommandToken(aRoomPassword, theAppP->m_InputBuf,2);
	
	// Tell the application to create the room, then join if successful
	if (theAppP->CreateRoom(aRoomName, aRoomPassword))
		theAppP->JoinRoom(aRoomName, aRoomPassword);

	return TRUE;
}





/********************************************************************************
 *	DoUsersCmd()																*
 *																				*
 *	Purpose:																	*
 *		Called from user "/user" Displays room occupants.						*
 ********************************************************************************/
int CUser::DoUsersCmd()
{
	extern CChatSampleApp* theAppP;

	// Set to system message color
	theAppP->m_Console.SetColor(CC_SYSTEM);

	if( !m_IsConnectedToChat )
	{
		printf("Not connected to a chat room.\r\n");
		return TRUE;
	}

	// Notification
	printf("Users in room:\r\n");

	// Is the user alone?
	if( theAppP->m_ClientMap.empty() )
	{
		printf("   No visible users.\r\n");
		return TRUE;
	}

	// Display all client names
	ClientMap::const_iterator aClientMapItr = theAppP->m_ClientMap.begin();
	for ( ; aClientMapItr != theAppP->m_ClientMap.end(); aClientMapItr++)
	{
		WONString aWideUserName(reinterpret_cast<const wchar*>(aClientMapItr->second.mClientName.data()), aClientMapItr->second.mClientName.size() / 2);
		wprintf(L"     %s\r\n",aWideUserName.GetUnicodeCString());
	}
	
	return TRUE;
}





/********************************************************************************
 *	DoWhisperCmd()																*
 *																				*
 *	Purpose:																	*
 *		Send chat to a single user.												*
 ********************************************************************************/
int CUser::DoWhisperCmd()
{
	extern CChatSampleApp* theAppP;

	string theTargetName;						// to person to whisper to
	unsigned short aTargetId = 0;				// Id of the target
	string theChatText;							// the chat text
	RoutingServerClient::Message theMessage;	// a message structure to send
	

	// Set to system message color
	theAppP->m_Console.SetColor(CC_SYSTEM);

	// Are we connected to a chat room? (routing server)
	if( !m_IsConnectedToChat )
	{
		printf("Not connected to a chat room.\r\n");
		return TRUE;
	}

	// Did we get a TargetName and a chat message from the input?
	if (GetCommandToken(theTargetName, theAppP->m_InputBuf, 1)==-1 ||
		GetCommandToken(theChatText, theAppP->m_InputBuf, 2, TRUE)==-1)
	{
		printf("usage: /whisper <target> <message>");
		return TRUE;
	}

	// Copy theTargetName to a unicode version
	wstring aUnicodeTargetName = StringToWString(theTargetName);

	// Search for a ClientId whose mClientName matches theTargetName
	ClientMap::const_iterator aClientMapItr = theAppP->m_ClientMap.begin();
	for(; aClientMapItr != theAppP->m_ClientMap.end(); aClientMapItr++)
	{
		// mClientName is a RawBuffer (basic_string<unsigned char>) being used
		// to store a wide string. wcscmp will detect an incorrect length so 
		// specify the number of characters to compare using wcsncmp.
		if(wcsncmp(aUnicodeTargetName.c_str(), (const unsigned short*)aClientMapItr->second.mClientName.c_str(), 
					aUnicodeTargetName.size())==0)
			break;
	}

	// Is the target valid?
	if(aClientMapItr == theAppP->m_ClientMap.end())
	{
		printf("Target not found.\r\n");
		return TRUE;
	}

	// Construct the WONMsg
	theMessage.mBufP = (const unsigned char*)theChatText.c_str();
	theMessage.mBufLen = theChatText.size();

	// Send the message to both the client and the user
	theAppP->mRoutingServer.WhisperChat(&aClientMapItr->second.mClientId, 1, TRUE,
										theMessage, WONMsg::CHATTYPE_ASCII,false);

	return TRUE;
}





/********************************************************************************
 *	DoEmoteCmd()																*
 *																				*
 *	Purpose:																	*
 *		Act out an action.	Example for user named "John"						*
 *		/emote smiles = "John smiles." instead of "John: smiles"				*
 ********************************************************************************/
int CUser::DoEmoteCmd()
{
	extern CChatSampleApp* theAppP;

	string theEmoteText;						// the emote text

	// Set to system message color
	theAppP->m_Console.SetColor(CC_SYSTEM);

	// Are we connected to a chat room? (routing server)
	if( !m_IsConnectedToChat )
	{
		printf("Not connected to a chat room.\r\n");
		return TRUE;
	}

	// Did we get a TargetName and a chat message from the input?
	if (GetCommandToken(theEmoteText, theAppP->m_InputBuf, 1, TRUE)==-1)
	{
		printf("usage: /emote <action>");
		return TRUE;
	}

	
	// Send the message to both the client and the user
	//theAppP->mRoutingServer.WhisperChat(&aClientMapItr->second.mClientId, 1, TRUE,
	//									theMessage, WONMsg::CHATTYPE_ASCII,false);
	
	return TRUE;
}