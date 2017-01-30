//////////////////////////////////////////////////////////////////////
// ChatSampleApp.cpp: implementation of the CChatSampleApp class.   //
//																	//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable:4786)		   // STL bug reports warning:4786
#pragma warning (disable:4275)

#include "stdafx.h"
#include "Console.h"
#include "User.h"
#include "ChatSampleApp.h"


// The file where the verifier key is stored
#define FILE_VERIFIER_KEY		"kver.pub"	


// From: ChatSample.cpp
// Purpose: Extracts a word (token) from a space delimited string
extern int GetCommandToken(string&,char*,int,BOOL = FALSE);





/********************************************************************************
 * Initializes the application													*
 ********************************************************************************/
BOOL CChatSampleApp::Initialize()
{
	// Clear the input buffer and client list
	memset(m_InputBuf,0,sizeof(m_InputBuf));
	m_ClientMap.clear();

	// Initialize the console
	m_Console.Init();

	// Open the logfile
	m_pLogFile = fopen("logfile.txt","w");

	// Is the file valid?
	if(m_pLogFile == 0)
	{
		// No, exit the application
		printf("Failed to open logfile. Aborting...\r\n");
		return FALSE;
	} 

	// Logfile created
	ReportLog("Logfile created...\r\n");

	// Load the verifier key from a file
	if (!LoadVerifierKey())
		return FALSE;
	
	// Install the directory server IPs
	if (!LoadDirectoryServers())
		return FALSE;

	// Install the callbacks which the routing server will call
	if (!InstallCallbacks())
		return FALSE;

	// All set
	return TRUE;

}//end Initialize()





/********************************************************************************
 * Load the verifier key.														*
 ********************************************************************************/
BOOL CChatSampleApp::LoadVerifierKey()
{
	if (!Identity::LoadVerifierKeyFromFile(FILE_VERIFIER_KEY))
	{
		// Load wasn't successful, abort application.
		ReportLog("LoadVerifierKeyFromFile() FAILED! Aborting...\r\n");
		return FALSE;
	} 
	
	// Verifier key loaded, continue
	ReportLog("Verifier Key Loaded...\r\n");
	return TRUE;
}





/********************************************************************************
 * Load the Directory Server addresses.											*
 ********************************************************************************/
BOOL CChatSampleApp::LoadDirectoryServers()
{	
	// Directory server are used to obtain changing titan server addresses.
	// (Authentication,Contest,Event,Factory,Firewall,Profile)
	m_pDirServers		= new IPSocket::Address[3];
	m_pDirServers[m_nNumDirServers++]	= IPSocket::Address("wontest.east.won.net", 15101);
	m_pDirServers[m_nNumDirServers++]	= IPSocket::Address("wontest.central.won.net", 15101);
	m_pDirServers[m_nNumDirServers++]	= IPSocket::Address("wontest.west.won.net", 15101);
	ReportLog("Directory Servers Loaded...\r\n");

	return TRUE;
}





/********************************************************************************
 * Install callback functions for WON Events.									*
 ********************************************************************************/
BOOL CChatSampleApp::InstallCallbacks()
{
	mRoutingServer.InstallClientEnterExCatcherEx((void(*)(const RoutingServerClient::ClientDataWithReason&, CChatSampleApp*))OnClientEnterChat, this);
	mRoutingServer.InstallClientLeaveCatcherEx((void(*)(const RoutingServerClient::ClientIdWithReason&, CChatSampleApp*))OnClientLeaveChat, this);
	mRoutingServer.InstallASCIIPeerChatCatcherEx((void(*)(const RoutingServerClient::ASCIIChatMessage&, CChatSampleApp*))OnChatReceived, this);
	mRoutingServer.InstallUnicodePeerChatCatcherEx((void(*)(const RoutingServerClient::UnicodeChatMessage&, CChatSampleApp*))OnChatReceived, this);

	return TRUE;
}

 
 
 
 
/********************************************************************************
 * Run the application. Contains the main event loop.							*
 ********************************************************************************/
int CChatSampleApp::Run()
{
	// Load the Titan Server address (download from a directory server)
	// (Authentication,Contest,Event,Factory,Firewall,Profile)
	if ( LoadTitanServers()==FALSE )
	{
		// Failed, abort.
		ReportLog("LoadTitanServers() FAILED! Aborting...\r\n");
		return FALSE;
	} else
		// Server IP's loaded, continue
		ReportLog("Titan Servers Loaded...\r\n");

	// Initialize the CUser object
	if (m_User.Init() == FALSE)
		return FALSE;						// abort if User is invalid 					

	/* the MAIN loop */
	ReportLog("Entering Main Loop...\r\n");
	
	// loop until (ProcessCommands()==FALSE)
	while(1)
	{
		switch(m_Status)
		{
		case AS_INITIALIZING:
			{
				// At this point the App is done initializing
				ReportLog("Finished Initializing...\r\n");

				// Display the CmdList to the user
				m_User.DoHelpCmd();

				// Go into normal state of command input and process
				m_Status = AS_NORMAL;
			
			break;
			}
		case AS_NORMAL:
			{
				// Draw the prompt
				DrawPrompt();

				// Read the input, process the commands
				ReadInput();

				// Did the user request an exit?
				if (ProcessCommands()==FALSE)
					return TRUE;		

			break;
			}
		default:
			// Should never reach this point
			ReportLog("Application has entered an unknown state! Terminating...\r\n");
			return FALSE;
		}
	}
}//end CChatSampleApp::Run()





/********************************************************************************
 * Receives a line of text from the user										*
 ********************************************************************************/
void CChatSampleApp::ReadInput()
{
	// Set the echo color
	m_Console.SetColor(CC_USERECHO);

	// read in a string to the input buffer (null terminated)
	gets( m_InputBuf );
}





/********************************************************************************
 * Draws the command prompt						 								*
 ********************************************************************************/
void CChatSampleApp::DrawPrompt()
{
	// Just a simple "> " prompt
	m_Console.SetColor(CC_SYSTEM);			// draw prompt in system color
	printf("\r\n> ");
	m_Console.SetColor(CC_USERECHO);		// echo user input in special color
}





/********************************************************************************
 * Load the Titan Servers (only store the Auth servers in this sample			*
 ********************************************************************************/
bool CChatSampleApp::LoadTitanServers()
{
	WONMsg::DirEntityList	anEntityList;		// To hold the list of Directory Entities

	// Retrieve the directory information
	Error aResult = GetDirectory(
						NULL,					// identity (not needed for titan servers)
						m_pDirServers,			// dir server ips to retrieve from
						m_nNumDirServers,		// number of dir server ips in the array
						NULL,					// dir server this is being called from
						L"/TitanServers/Auth",	// name of the directory we want to retrieve
						&anEntityList,			// a place to store the results
						WONMsg::GF_DECOMPROOT | WONMsg::GF_DECOMPRECURSIVE | 
							WONMsg::GF_DECOMPSERVICES | WONMsg::GF_ADDTYPE | 
							WONMsg::GF_SERVADDNAME | WONMsg::GF_SERVADDNETADDR | 
							WONMsg::GF_ADDDOTYPE | WONMsg::GF_ADDDODATA);
	
	// Synchronous call. Will block until call has completed.			
	if (aResult==Error_Success)
	{
		// Allocate memory for the auth server ip's
		m_pAuthServers = new IPSocket::Address[anEntityList.size()];

		// Succeeded, store the auth servers
		DirEntityList::const_iterator	aDirEntityListItr;

		aDirEntityListItr = anEntityList.begin();
		for (; aDirEntityListItr != anEntityList.end(); ++aDirEntityListItr)
		{
			// Is this Entity and AuthServer?
			if (aDirEntityListItr->mName == L"AuthServer")
				// Add it to the list
				m_pAuthServers[m_nNumAuthServers++] = IPSocket::Address(*aDirEntityListItr);
		}

		// Auth servers Loaded
		return TRUE;
	}
	//else

	// Couldn't load auth servers, can't login without them
	return FALSE; /* signal abort */

}//end LoadTitanServers()





/********************************************************************************
 * Display the rooms list.														*
 ********************************************************************************/
int CChatSampleApp::DisplayRooms()
{
	// Set to system color for this command
	m_Console.SetColor(CC_SYSTEM);

	// We have to be logged in first
	if(!m_User.m_IsLoggedIn)
	{
		printf("** Not Logged In **\r\n");
		return TRUE;
	}

	// List of the entities in the directory
	WONMsg::DirEntityList	theGetDirResult;
	
	WONCommon::DataObjectTypeSet aDataObjectSet;							// Specify our data objects
	aDataObjectSet.insert(DataObject((unsigned char*)"__RSClientCount"));	// number of clients
	aDataObjectSet.insert(DataObject((unsigned char*)"__RSLoginACL"));		// ?
	aDataObjectSet.insert(DataObject((unsigned char*)"__RSRoomFlags"));		// room flags (password protected?)

	// Retrieve the directories
	Error result = GetDirectory(&m_User.m_Identity,		// Who we are
							m_pDirServers,				// Directory Servers IP's
							m_nNumDirServers,			// Number of IPs
	                        NULL,
	                        L"/APISample",				// Which directory do we want?
							&theGetDirResult,			// to store the result
	                        WONMsg::GF_DECOMPSERVICES | WONMsg::GF_ADDTYPE | WONMsg::GF_ADDDISPLAYNAME | 
							WONMsg::GF_SERVADDNAME | WONMsg::GF_SERVADDNETADDR | WONMsg::GF_ADDDODATA | 
							WONMsg::GF_ADDDOTYPE,		// various flags
							aDataObjectSet);			// the data set to retrieve

	// Process the results
	if (result != StatusCommon_Success)					// GetDirectory() Failed!!
	{
		printf("Could not load chat server data.\r\n");
		return TRUE;
	}

	// Clear the chat servers map for reloading
	m_ChatServersMap.clear(); 

	// Clear the list of factory servers for reloading
	delete[] m_pFactoryServers;
	m_nNumFactoryServers = 0;
	m_pFactoryServers = new IPSocket::Address[theGetDirResult.size()];
		
	// Process the entitylist from beginning to end
	for( DirEntityList::const_iterator aDirEntityItr = theGetDirResult.begin();		
			aDirEntityItr != theGetDirResult.end(); ++aDirEntityItr)					
	{
		// Is this data object a Chat Server?
		if (aDirEntityItr->mName == L"TitanRoutingServer")
		{
			//...yes.
			ChatServerInfo chatInfo;
			chatInfo.mAddress = IPSocket::Address(*aDirEntityItr);

			// Process the DataObjects within each entitylist
			for( DataObjectTypeSet::const_iterator aDataObjectItr = aDirEntityItr->mDataObjects.begin();
					aDataObjectItr != aDirEntityItr->mDataObjects.end(); ++aDataObjectItr)
			{
				// Is this the ClientCount data item?
				if (aDataObjectItr->GetDataType() == (unsigned char*)"__RSClientCount")
				{
					// Copy into the chatInfo object
					chatInfo.mClientCount = *(unsigned long*)aDataObjectItr->GetData().c_str();		
				}
				// is this the RoomFlags data item?
				else if (aDataObjectItr->GetDataType() == (unsigned char*)"__RSRoomFlags")
				{
					// Copy over password protection flag
					chatInfo.mIsPasswordProtected = ((*aDataObjectItr->GetData().c_str()) & 0x1 != 0);
				}
			}
			// Store the server into our map and increment server total
			m_ChatServersMap[aDirEntityItr->mDisplayName] = chatInfo;
		}

		// ...not a CHAT ROOM, is it a FACTORY?
		else if (aDirEntityItr->mName == L"TitanFactoryServer")
		{
			// ...yes, add it to the array.
			m_pFactoryServers[m_nNumFactoryServers++] = (*aDirEntityItr);
		}
	}

	// If there aren't any chat rooms we're done
	if(m_ChatServersMap.empty())
	{
		// notify user and return
		printf("No rooms available.\r\n");
		return TRUE;
	}

	// Display the room names
	printf("Available chat rooms:\r\n");

	ChatServerMap::iterator theChatServersMapItr = m_ChatServersMap.begin();
	for(; theChatServersMapItr != m_ChatServersMap.end(); theChatServersMapItr++)
	{
		printf("    %S\r\n", theChatServersMapItr->first.c_str());
	}

	return TRUE; // StatusCommon_Sucess
}//end DisplayRooms()

 
 
 
 
 /********************************************************************************
 *	ProcessCommands()															*
 *																				*
 *	Purpose:																	*
 *		First processing to user input. Passes control to above functions		*
 ********************************************************************************/
 int CChatSampleApp::ProcessCommands()
{
	// Isolate the first token (space delimited word) in the input buffer
	string theCommandString;
	
	// Extract the command string
	GetCommandToken(theCommandString, m_InputBuf, 0);
	
	// Is the command string bound to a function?
	return m_User.ProcessCommand(theCommandString);
	
}





/********************************************************************************
 * A simple utility to log and display a message								*
 ********************************************************************************/
void CChatSampleApp::ReportLog(const char* theMessageP)
{
	printf(theMessageP);					//display the message
	fprintf(m_pLogFile,theMessageP);		//print the message to the logfile
}//end reportlog()

 
 
 
 
/********************************************************************************
 * Create a chat room	(called by user object)									*
 ********************************************************************************/
int CChatSampleApp::CreateRoom(const string& theRoomNameR, const string& theRoomPasswordR)
{
	string aCommandLine;
	unsigned short aRoomPort;
	IPSocket::Address aChatServerFactory;

	//Assemble a string command line to send to the titan server
	if(!theRoomPasswordR.empty())
	{
		aCommandLine += " -Password \"";
		aCommandLine += theRoomPasswordR;
		aCommandLine += "\"";
	  //aCommandLine  = " -Password "<roompassword>""
	}
		
	// Start a titan routing server for this chat room
	Error aResult = StartTitanServer(&m_User.m_Identity, 
									m_pFactoryServers,
									m_nNumFactoryServers, 
									&aChatServerFactory,
									"RoutingServSampleChat", 
									&aRoomPort, 
									NULL,
									aCommandLine.c_str());

	if (aResult == StatusCommon_Success)
	{
		// Store the address and port
		theCurrentChatServer = aChatServerFactory;
		theCurrentChatServer.SetPort(aRoomPort);
		
		// Register the new routing server with a directory server
		aResult = RegisterTitanServer(theCurrentChatServer, 2, m_pDirServers, 
										m_nNumDirServers, L"/APISample", theRoomNameR);
		
		if(aResult == StatusCommon_Success)
		{
			// Routing server started and registered
			printf("Started chat server at %s\r\n", theCurrentChatServer.GetAddressString().c_str());

			// Add the new room to the room list
			WONString aUnicodeRoomName(theRoomNameR);		// unicode version of room name

			ChatServerInfo aChatServerInfo;
			aChatServerInfo.mAddress = theCurrentChatServer;
			//aChatServerInfo.mIsPasswordProtected = (thePassword && thePassword[0] != '\0');
			m_ChatServersMap[aUnicodeRoomName] = aChatServerInfo;						
		}
		else
		{
			printf("Could not register chat room.\r\n");
			m_User.m_IsConnectedToChat = FALSE;
		}
	} 
	else // aResult != StatusCommon_Success
		printf("Could not start chat server.\r\n");

	// return TRUE for Application to continue
	return TRUE;
}





/********************************************************************************
 * Join a chat room	(called by user object)										*
 ********************************************************************************/
int CChatSampleApp::JoinRoom(const string& theRoomNameR, const string& theRoomPasswordR)
{
	// Convert to unicode
	WONString aUnicodeRoomName(theRoomNameR);

	// Get the info the room specified
	ChatServerMap::iterator theChatServersMapItr = m_ChatServersMap.find(aUnicodeRoomName);

	// Does the room exist? (info will exist)
	if(theChatServersMapItr == m_ChatServersMap.end() )
	{
		printf("Could not find room: %s\r\n", theRoomNameR.c_str());
		return FALSE;
	}
	 
	// close previous connection;
	mRoutingServer.Close();
	m_User.m_IsConnectedToChat = FALSE;
	
	// Connect to the specified chat server
	Error aResult = mRoutingServer.Connect(theChatServersMapItr->second.mAddress, &m_User.m_Identity, false, -1);

	// If we didn't connect...
	if (aResult != StatusCommon_Success)
	{
		printf("Could not connect to routing server.\r\n");
		return TRUE;
	}
	else
	{
		printf("Connected to routing server.\r\n");
		m_User.m_IsConnectedToChat = TRUE;
	}

	// Register the username with the chat server
	RawBuffer aClientName((unsigned char*)m_User.m_Identity.GetLoginName().GetTitanUnicodeString().data(), 
		m_User.m_Identity.GetLoginName().GetTitanUnicodeString().size() * 2);
	
	// Register the username with the routing server
	mRoutingServer.Register( aClientName, "", false, false, true);
	
	// Store the address of the chat server
	theCurrentChatServer = theChatServersMapItr->second.mAddress;

	// Set the user flag
	m_User.m_IsConnectedToChat = TRUE;

	// Clear the client list
	m_ClientMap.clear();

	// Load list of room occupants
	mRoutingServer.GetClientListEx(false, // no auth info
	                               false, // no mute flag
	                               false, // no moderator flag
	                               (void(*)(const RoutingServerClient::GetClientListResult&, CChatSampleApp*))ClientListCallback, this);

	// Display the list of room occupants
	printf("Loading user list...");
	
	return TRUE;
}





//////////////////////////////////////////////////////////////////////
// Callbacks														//
//////////////////////////////////////////////////////////////////////

// User has joined chat
void CChatSampleApp::OnClientEnterChat	(const RoutingServerClient::ClientDataWithReason& theDataR,	CChatSampleApp* thatP)
{
	// Set the console message color
	thatP->m_Console.SetColor(CC_SYSTEM);

	// Notifify the user that another has entered the room
	printf("\r\n%S has entered the room.\r\n", theDataR.mClientName.c_str());

	// Restore the prompt and color to input
	thatP->DrawPrompt();

	// Store the new user in the client list
	thatP->m_ClientMap[theDataR.mClientId] = theDataR;
}

// User has left chat
void CChatSampleApp::OnClientLeaveChat	(const RoutingServerClient::ClientIdWithReason& theDataR,	CChatSampleApp* thatP)
{
	// Set the console message color
	thatP->m_Console.SetColor(CC_SYSTEM);

	// Notifify the user that another has left the room
	printf("\r\n%S has left the room.\r\n", thatP->m_ClientMap.find(theDataR.mClientId)->second.mClientName.c_str());

	// Restore the prompt
	thatP->DrawPrompt();

	// Remove the user from the client list
	thatP->m_ClientMap.erase(theDataR.mClientId);
}

// ASCII Chat message received
void CChatSampleApp::OnChatReceived		(const RoutingServerClient::ASCIIChatMessage& theMessageR,		CChatSampleApp* thatP)
{
	// Get the ClientId for this chat message
	ClientMap::const_iterator aClientMapItr = thatP->m_ClientMap.find(theMessageR.mSenderId);

	// Is it valid?
	if (aClientMapItr != thatP->m_ClientMap.end())
	{
		// Set console color to received chat message (actually doesn't work)
		thatP->m_Console.SetColor(CC_OTHERCHAT);

		// Display the message
		printf("\r\n%S: %s\r\n", aClientMapItr->second.mClientName.c_str(), theMessageR.mData.c_str());

		// Restore the command prompt
		thatP->DrawPrompt();
	}

}

// WIDE CHAR Chat message received
void CChatSampleApp::OnChatReceived		(const RoutingServerClient::UnicodeChatMessage& theMessageR,	CChatSampleApp* thatP)
{
	// Get the ClientId for this chat message
	ClientMap::const_iterator aClientMapItr = thatP->m_ClientMap.find(theMessageR.mSenderId);

	// Is it valid?
	if (aClientMapItr != thatP->m_ClientMap.end())
	{
		// Set console color to received chat message (actually doesn't work)
		thatP->m_Console.SetColor(CC_OTHERCHAT);

		WONString aChatText(theMessageR.mData);
		printf("\r\n%S: %S\r\n", aClientMapItr->second.mClientName.c_str(), aChatText.GetUnicodeCString());

		// Restore the command prompt
		thatP->DrawPrompt();
	}
}

// GetClientList callback
void CChatSampleApp::ClientListCallback  (const RoutingServerClient::GetClientListResult& theResultR,	CChatSampleApp* thatP)
{
	// Was the call successful?
	if (!theResultR.mStatus == StatusCommon_Success)
		return;

	// Server has sent the client list information, copy into m_ClientMap
	std::list<WONMsg::MMsgRoutingGetClientListReply::ClientData>::const_iterator aListItr;
	for (aListItr = theResultR.mClientList.begin(); 
		aListItr != theResultR.mClientList.end(); aListItr++)
	{
		thatP->m_ClientMap[aListItr->mClientId] = *aListItr;
	}

	// Display the user list to the user
	printf("\r\n");
	thatP->m_User.DoUsersCmd();

	// Restore the command prompt
	thatP->DrawPrompt();

}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChatSampleApp::CChatSampleApp()
{
	m_pDirServers			= NULL;
	m_nNumDirServers		= 0;

	m_pAuthServers			= NULL;
	m_nNumAuthServers		= 0;

	m_pFactoryServers		= NULL;
	m_nNumFactoryServers	= 0;

	m_pLogFile				= NULL;
	m_Status				= AS_INITIALIZING;
}

CChatSampleApp::~CChatSampleApp()
{
	// Clean up the server lists
	delete m_pDirServers;
	delete m_pAuthServers;
	delete m_pFactoryServers;

	// Close the log file
	fclose(m_pLogFile);

}
