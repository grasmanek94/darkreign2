// WhiteBoardDoc.cpp : implementation of the CWhiteBoardDoc class
//
#pragma warning (disable : 4786)

#include "stdafx.h"

#include "WhiteBoard.h"
#include "WhiteBoardDoc.h"
#include "LoginDialog.h"
#include "ChatRoomsDialog.h"
#include "BoardsDialog.h"
#include "StrokeSizeDialog.h"
#include "Stroke.h"
#include "WhiteBoardView.h"
#include "ChatView.h"

namespace
{
	using namespace std;			// 
	using namespace WONMsg;			// server status messages
	using namespace WONCommon;		//
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Message Structures (used to create linear data buffers)
struct SMsgJoinReply
{
	unsigned char mType;
	int mNumIds;
	unsigned short mIds[MAX_OBSERVERS];
};

// The file where the verifier key is stored
const char*             FILE_VERIFIER_KEY        = "kver.pub";

// Address and port of the patchserver
const char*             ADDR_PATCHSERVER         = "sample.update.won.net";
const unsigned short    PORT_PATCHSERVER         = 80;

// Address and port of the message of the day server
const char*             ADDR_MOTDSERVER          = "www.won.net";
const unsigned short    PORT_MOTDSERVER          = 80;

const char*             COMMUNITY_SAMPLE         = "WON";
const char*             CONFIG_SAMPLECHATSERVER  = "RoutingServSampleChat";
const wchar_t*          DIR_TITANSERVER          = L"/TitanServers"; // Parent directory of dirs holding auth, firewall, and event servers.
const wchar_t*          DIR_APISAMPLE            = L"/APISample";
const RawBuffer         OBJ_GAMEPREFIX           = (unsigned char*)"GAME";
const RawBuffer         OBJ_VALIDVERSIONS        = (unsigned char*)"SampleValidVersions";
const RawBuffer         OBJ_CHATROOM_CLIENTCOUNT = (unsigned char*)"__RSClientCount";
const RawBuffer         OBJ_CHATROOM_LOGINACL    = (unsigned char*)"__RSLoginACL";
const RawBuffer         OBJ_CHATROOM_ROOMFLAGS   = (unsigned char*)"__RSRoomFlags";
const wchar_t*          SERVERNAME_AUTH          = L"AuthServer";            // Service name for auth servers
const wchar_t*          SERVERNAME_CHAT          = L"TitanRoutingServer";    // Service name for chat rooms (chat Routing Servers)
const wchar_t*          SERVERNAME_CONTEST       = L"TitanContestServer";    // Service name for contest servers
const wchar_t*          SERVERNAME_EVENT         = L"TitanEventServer";      // Service name for event servers
const wchar_t*          SERVERNAME_FACTORY       = L"TitanFactoryServer";    // Service name for factory servers
const wchar_t*          SERVERNAME_FIREWALL      = L"TitanFirewallDetector"; // Service name for firewall servers
const wchar_t*          SERVERNAME_PROFILE       = L"TitanProfileServer";    // Service name for profile servers
const short             CONTESTPROC_REPORTRESULT = 8; // p_ttt_saveresults
const short             CONTESTPROC_TOPTENLIST   = 9; // p_ttt_gethighscores

// Message Types
const unsigned char MSGTYPE_INVALID			= 0;
const unsigned char MSGTYPE_JOIN_REQUEST	= 1;
const unsigned char MSGTYPE_JOIN_REPLY		= 2;
const unsigned char MSGTYPE_YOUR_TURN		= 3;
const unsigned char MSGTYPE_STROKE			= 4;

const unsigned char MSGTYPE_PLAYER_JOIN		= 8;
const unsigned char MSGTYPE_PLAYER_LEAVE	= 9;
const unsigned char MSGTYPE_GAME_DISSOLVED	= 10;


//namespace
//{
//	using namespace ChatServerMap;
//}

/////////////////////////////////////////////////////////////////////////////
// CWhiteBoardDoc

IMPLEMENT_DYNCREATE(CWhiteBoardDoc, CDocument)

BEGIN_MESSAGE_MAP(CWhiteBoardDoc, CDocument)
	//{{AFX_MSG_MAP(CWhiteBoardDoc)
	ON_COMMAND(ID_OPTIONS_LOGIN, OnOptionsLogin)
	ON_COMMAND(ID_OPTIONS_ROOMS, OnOptionsRooms)
	ON_COMMAND(ID_OPTIONS_BOARDS, OnOptionsBoards)
	ON_COMMAND(ID_OPTIONS_STROKE_COLOR, OnOptionsStrokeColor)
	ON_COMMAND(ID_OPTIONS_STROKE_SIZE, OnOptionsStrokeSize)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWhiteBoardDoc construction/destruction

CWhiteBoardDoc::CWhiteBoardDoc()
{
	// Initialize pointers and arrays to NULL value
	// non pointer member initailize can be found in InitDocument()
	m_pCurStroke		= NULL;
	m_pChatView			= NULL;
	m_pStrokeView		= NULL;
}

CWhiteBoardDoc::~CWhiteBoardDoc()
{
	// Delete the stroke lists, call each stroke to delete it's points
	CStroke* pStroke;

	while (!m_StrokeList.IsEmpty())
	{
		pStroke = m_StrokeList.RemoveHead();
		pStroke->DeleteContents();
		delete pStroke;
	}
	delete m_pCurStroke;

	// Delete the dir servers
	mDirServers.RemoveAll();

	// Delete the server lists
	mAuthServers.RemoveAll();
 
	// Clear the remaining titan server arrays
	mContestServers.RemoveAll();
	mEventServers.RemoveAll();
	mFirewallServers.RemoveAll();
	mProfileServers.RemoveAll();
	mFactoryServers.RemoveAll();
 
	// Empty the chat and game server maps
	mChatServers.clear();
	mGameMap.clear();
   
} 

BOOL CWhiteBoardDoc::OnNewDocument()
{
	// Display the splash screen
	CDialog theDialog;
	theDialog.Create(IDD_SPLASHSCREEN, NULL);

	if (!CDocument::OnNewDocument())
		return FALSE;

	// Initialization Procedures
	theDialog.SetDlgItemText(IDC_SPLASHSTATIC,"Initializing...");
	InitDocument();						// Initialize non-pointer members
	theDialog.SetDlgItemText(IDC_SPLASHSTATIC,"Loading Verifier Key...");
	LoadVerifierKey();					// Load the verifier key from kver.pub
	theDialog.SetDlgItemText(IDC_SPLASHSTATIC,"Loading Directory Servers...");
	LoadDirectoryServers();				// Load the server directory servers (hardcoded)
	
	// Download the server list from a server directory server
	theDialog.SetDlgItemText(IDC_SPLASHSTATIC,"Downloading Server Lists...");
	DownloadServerLists();				// these are the auth, routing,  etc servers

	// Install our peer message catcher callback
	mRoutingServer.InstallPeerDataCatcherEx((void(*)(const RoutingServerClient::MessageWithClientId&, CWhiteBoardDoc*))OnRoutingMessageReceived, this);

	// Hide the splash screen
	theDialog.DestroyWindow();

	// All set, let's rock.
	return TRUE;
}

 

/////////////////////////////////////////////////////////////////////////////
// CWhiteBoardDoc serialization

void CWhiteBoardDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWhiteBoardDoc diagnostics

#ifdef _DEBUG
void CWhiteBoardDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CWhiteBoardDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWhiteBoardDoc commands

void CWhiteBoardDoc::PerformLogin()
{
	CLoginDialog theLoginDialog;

	// Ask for login information
	if (theLoginDialog.DoModal()==IDOK)
	{
		// Copy password and loginname from the dialog box
		mPassword = theLoginDialog.m_Password;
		mLoginName = theLoginDialog.m_LoginName;
	} 
	else // Login cancelled
		return;

	// Notification message (redraw to post immediately)
	ReceiveText("<System> Attempting Login...\r\n");
	m_pChatView->RedrawWindow();

	// Login using the Auth servers downloaded from the Directory Server
	mIdentity = Identity( (LPCSTR)mLoginName, "WON", (LPCSTR)mPassword, "", &mAuthServers[0], mAuthServers.GetSize());
	Error aError = mIdentity.Authenticate(false , false, 3000);

	switch (aError)
	{
		case StatusCommon_Success:
		{
			// Send a notification to the screen
			CString buf = "<System> Now logged in as ";
			buf += mLoginName;
			buf += "\r\n";

			ReceiveText(buf);

			// Set the logged in flag
			bIsLoggedIn = true;

			// Modify document title
			SetTitle(mLoginName);
			break;
		}
		case StatusAuth_UserNotFound:
			ReceiveText("<System> ***Login Error: Password/Username did not match!***\r\n");
			break;
		case StatusAuth_BadPassword:
			ReceiveText("<System> ***Login Error: Password/Username did not match!***\r\n");
			break;
		case StatusAuth_CDKeyInUse:
			ReceiveText("<System> ***Login Error: CD Key already in use!***\r\n");
			break;
		case StatusAuth_CRCFailed:
			ReceiveText("<System> ***Login Error: CRC Failed!***\r\n");
			break;
		default:
			ReceiveText("<System> ***Login Error: Unknown reason***\r\n");
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Menu handlers

void CWhiteBoardDoc::OnOptionsLogin() 
{
	// Re-route to the same login method as at program startup
	PerformLogin();
}//end OnOptionsLogin()

 
 
 
 
/********************************************************************************
 * Display a list of rooms and options for creating/joining them.				*
 ********************************************************************************/
void CWhiteBoardDoc::OnOptionsRooms() 
{
	// Are we logged in?...
	if (!bIsLoggedIn)	
	{
		// ...nope! 
		ReceiveText("<System> ***Error Not Logged In *** \r\n");
		return;
	}
	
	//...yes. We can continue.
	ReceiveText("<System> Retrieving room list...\r\n");
	m_pChatView->RedrawWindow();		// post message immediately

	// Get the chat room list
	GetChatServerList();

	// Create the RoomsDialog box
	CChatRoomsDialog theRoomsDialog;								// join/creat room dialog box
	CString selectedRoomName;										// stores the selected room name

	// Are there any chat rooms running?
	if (mNumChatServers == 0)
	{
		MessageBox(NULL, "No rooms available.", "Status", MB_OK);
		theRoomsDialog.DoModal(selectedRoomName);					// use an empty chat room list
	}
	else 
		theRoomsDialog.DoModal(selectedRoomName, mChatServers);		// use the retreived chat room list

	// selectedRoomName should now contain the specified room name (not validated)
	if(selectedRoomName=="")
	{
		// No Room Specified or user pressed CANCEL
		ReceiveText("<System> Join/Create room action cancelled.\r\n");
		return;
	} 
	
	// Notification to user
	ReceiveText("<System> Attemping to create room...\r\n");
	m_pChatView->RedrawWindow();

	// cleanup previous Routing Server connection (if any)
	mClientMap.clear();
	mRoutingServer.Close();

	// Search for the room in the chat room list
	map<wstring, ChatServerInfo>::const_iterator itr = mChatServers.find( WONString(selectedRoomName) );
	
	// Does the room exist?
	if( itr == mChatServers.end() )
	{
		// No. Create the room
		Error result = CreateRoom(selectedRoomName);

		if (result != StatusCommon_Success)
		{
			ReceiveText("<System> ***Error, could not create room.***");
			return;
		}

	} else
	{
		// Copy info to our current connection
		mCurChatServerName = selectedRoomName;
		mCurChatServerAddr = itr->second.mAddress;
	}//end (if !(room exists))


	/********** JOIN THE ROOM **********/


	// connect to the new server
	Error aError = mRoutingServer.Connect(mCurChatServerAddr, &mIdentity, false, -1);

	switch (aError)
	{
		case StatusCommon_Success:
			{
				// Notification
				ReceiveText("<System> Now speaking in room: ");
				ReceiveText(selectedRoomName);
				ReceiveText("\r\n");
				
				// login
				RawBuffer aClientName((unsigned char*)mIdentity.GetLoginName().GetTitanUnicodeString().data(), mIdentity.GetLoginName().GetTitanUnicodeString().size() * 2);
				mRoutingServer.Register( aClientName, "", false, false, true );

				// Set the window caption to reflect room name
				CString newTitle;
				newTitle = mLoginName + "--" + selectedRoomName;
				SetTitle(newTitle);	

				// Set the chatting flag
				bIsChatting = true;

				// Get Client Info
				mRoutingServer.GetClientListEx(false, // no auth info
											false, // no mute flag
											false, // no moderator flag
											(void(*)(const RoutingServerClient::GetClientListResult&, CChatView*))CChatView::ClientListCallback, m_pChatView);


				break;
			}
		case Error_HostUnreachable:
			ReceiveText("<System> Could not connect to chat room: Host Unreachable.\r\n");
			break;
		default:
			ReceiveText("<System> Could not connect to chat room: Reason Unknown.\r\n");
			break;
	}
}//end OnOptionsRooms()

 
 
 
 
/********************************************************************************
 * Retrieves the chat and factory server listings								*
 ********************************************************************************/
Error CWhiteBoardDoc::GetChatServerList()
{
	WONMsg::DirEntityList	theDirEntityList;
	WONMsg::DirEntityList*	pDirEntityResult = &theDirEntityList;			// Stores our results
	
	WONCommon::DataObjectTypeSet aDataObjectSet;							// Specify our data objects
	aDataObjectSet.insert(DataObject(OBJ_CHATROOM_CLIENTCOUNT));			//		number of clients
	aDataObjectSet.insert(DataObject(OBJ_CHATROOM_LOGINACL));				//		?
	aDataObjectSet.insert(DataObject(OBJ_CHATROOM_ROOMFLAGS));				//		room flags (password protected?)

	// Retrieve the directories
	Error result = GetDirectory(&mIdentity,									// Who we are
	                              &mDirServers[0], mDirServers.GetSize(),		// Server IP's and the number of servers
	                              NULL,
	                              DIR_APISAMPLE,							// Which directory do we want?
								  pDirEntityResult,							// the place to store our results
	                              WONMsg::GF_DECOMPSERVICES | WONMsg::GF_ADDTYPE | WONMsg::GF_ADDDISPLAYNAME | 
								  WONMsg::GF_SERVADDNAME | WONMsg::GF_SERVADDNETADDR | WONMsg::GF_ADDDODATA | 
								  WONMsg::GF_ADDDOTYPE,						// various flags
								  aDataObjectSet);							// the data set to retrieve

	// Process the results (pDirEntityResult)
	if (result != StatusCommon_Success)										// GetDirectory() Failed!!
		return result;

	mChatServers.clear(); 
	mNumChatServers = 0;
	
	mFactoryServers.RemoveAll();
	mFactoryServers.SetSize(pDirEntityResult->size());
	
	// Process the entitylist from beginning to end
	for( DirEntityList::const_iterator DirEntityItr = pDirEntityResult->begin();		
			DirEntityItr != pDirEntityResult->end(); ++DirEntityItr)					
	{
		if (DirEntityItr->mName == SERVERNAME_CHAT)	// Is this data object a CHAT ROOM?
		{
			//...yes.
			ChatServerInfo chatInfo;
			chatInfo.mAddress = IPSocket::Address(*DirEntityItr);

			// Process the DataObjects within each entitylist
			for( DataObjectTypeSet::const_iterator DataObjectItr = DirEntityItr->mDataObjects.begin();
					DataObjectItr != DirEntityItr->mDataObjects.end(); ++DataObjectItr)
			{
				// Is this the ClientCount data item?
				if (DataObjectItr->GetDataType() == OBJ_CHATROOM_CLIENTCOUNT)
				{
					// Copy into the chatInfo object
					chatInfo.mClientCount = *(unsigned long*)DataObjectItr->GetData().c_str();		
				}
				// is this the RoomFlags data item?
				else if (DataObjectItr->GetDataType() == OBJ_CHATROOM_ROOMFLAGS)
				{
					// Copy over password protection flag
					chatInfo.mIsPasswordProtected = ((*DataObjectItr->GetData().c_str()) & 0x1 != 0);
				}
			}
			
			// Store the server into our map and increment server total
			mChatServers[DirEntityItr->mDisplayName] = chatInfo;
			++mNumChatServers;
		}

		// ...not a CHAT ROOM, is it a FACTORY?
		else if (DirEntityItr->mName == SERVERNAME_FACTORY)
		{
			// ...yes, add it to the array.
			mFactoryServers.Add(IPSocket::Address(*DirEntityItr));
		}
	}

	return result; // StatusCommon_Sucess
}//end GetChatServerList()

 
 
 
 
/********************************************************************************
 * Create a room before login.													*
 ********************************************************************************/
Error CWhiteBoardDoc::CreateRoom(const CString& selectedRoomName)
{
		IPSocket::Address hostFactory;							// the factory which created the chat room
		unsigned short hostPort;								// the port on which the server was created
		wstring command;
		command += wstring(L" -DirRegDisplayName \"") + wstring(WONString(selectedRoomName)) + wstring(L"\"");

		// Call to Start the chat server
		Error result = StartTitanServer(&mIdentity, &mFactoryServers[0], mFactoryServers.GetSize(),
										&hostFactory, "RoutingServSampleChat", &hostPort,
										NULL, command, false, 1, 0, NULL, NULL, 0, -1);

		// process return value
		switch(result)
		{
		case StatusCommon_Success:
			// Created the chat room, notify and set data members
			ReceiveText("<System> Room: ");
			ReceiveText(selectedRoomName);
			ReceiveText(", created...attemping to access...\r\n");
			mCurChatServerAddr = hostFactory;
			mCurChatServerAddr.SetPort(hostPort);
			mCurChatServerName = selectedRoomName;

			break; // continue to Join
		case StatusCommon_InvalidParameters:
			ReceiveText("<System> StartTitanServer() FAILED! Request format is invalid.\r\n");
			return result;
		case StatusCommon_ResourcesLow:
			ReceiveText("<System> StartTitanServer() FAILED! Resources low.\r\n");
			return result;
		case StatusDir_ServiceExists:
			// not currently supported
			break; // continue to Join
		default:
			ReceiveText("<System> StartTitanServer() FAILED! Unknown reason.\r\n");
			return result;
		};

		return result;
}//end CreateRoom()

 
 
 
 
/********************************************************************************
 * Document Initialization.														*
 ********************************************************************************/
void CWhiteBoardDoc::InitDocument()
{
	bIsLoggedIn				= false;
	bIsChatting				= false;
	bOnBoard				= false;
	bIsCaptain				= false;

	mNumObservers			= 0;
	mNumChatServers			= 0;
	
	mNumProfileServersTried	= 0;
	mCurProfileServer		= 0;

	mRoutingServer.InstallDataObjectCreationCatcherEx((void(*)(const RoutingServerClient::DataObjectWithLifespan&, CWhiteBoardDoc*))OnDataObjectCreated, this);
	mRoutingServer.InstallDataObjectDeletionCatcherEx((void(*)(const RoutingServerClient::DataObject&, CWhiteBoardDoc*))OnDataObjectDeleted, this);

	memset(mObserverIds, 0, sizeof(WONAPI::RoutingServerClient::ClientId)*MAX_OBSERVERS);

	m_CurStrokeColor = RGB(0,0,0);
	m_nCurStrokeSize = 2;
}//end InitDocument()

 
 
 
 
/********************************************************************************
 * Load the verifier key from the specified file.								*
 * FILE_VERIFIER_KEY = "kver.pub"												*
 ********************************************************************************/
bool CWhiteBoardDoc::LoadVerifierKey()
{
	if (!Identity::LoadVerifierKeyFromFile(FILE_VERIFIER_KEY))
	{
		MessageBox(NULL, "Failed to load verifier key", "ERROR", MB_OK);
		return false;
	}
	else
		return true;
	
}//end LoadVerifierKey()

 
 
 
 
/********************************************************************************
 * Load a list of directory servers. Most often from a file, in this example	*
 * I just hardcoded the addresses into the array.								*
 ********************************************************************************/
void CWhiteBoardDoc::LoadDirectoryServers()
{
	// Read in Directory Server IPs (HARDCODED for this sample)
	//	Sleep(10000); /*test code*/

	mDirServers.SetSize(3);
	mDirServers[0] = IPSocket::Address("wontest.east.won.net", 15101);
	mDirServers[1] = IPSocket::Address("wontest.central.won.net", 15101);
	mDirServers[2] = IPSocket::Address("wontest.west.won.net", 15101);
}//end LoadDirectoryServers()

 
 
 
 
/********************************************************************************
 * Download the list of servers from the directory server. Process the results	*
 * into various server list member variables									*
 * (Auth,Contest,Event,Firewall,Profile,Factory)								*
 ********************************************************************************/
void CWhiteBoardDoc::DownloadServerLists()
{
	// Specifify which data objects we're interested in, then retrieve the directories
	DataObjectTypeSet		aDataObjectSet;								// A data set to hold special info
	aDataObjectSet.insert(WONCommon::DataObject(OBJ_VALIDVERSIONS));	// additional info we are interested in
	WONMsg::DirEntityList	entityList;									// To hold the list of Directory Entities
	
	// Retrieve the directory information
	// (should be handled asynchronously by using GetDirectoryEx)
	Error result = GetDirectory(NULL,			// no identity needed for titan servers
						&mDirServers[0],		// an array of directory server ips (I hardcoded these in LoadDirecetoryServers() )
						mDirServers.GetSize(),	// the number of directory server ips (hard coded three servers)
						NULL,
						DIR_TITANSERVER,		// the name of the directory we want to retrieve
						&entityList,			// a place to store the results
						WONMsg::GF_DECOMPROOT | WONMsg::GF_DECOMPRECURSIVE | WONMsg::GF_DECOMPSERVICES | WONMsg::GF_ADDTYPE | WONMsg::GF_SERVADDNAME | WONMsg::GF_SERVADDNETADDR | WONMsg::GF_ADDDOTYPE | WONMsg::GF_ADDDODATA,
						aDataObjectSet,			// additional info request (valid versions)
						0, 0,
						10000);					// timeout, -1 means no timeout 

	// Process the result
	switch(result) 
	{
		case Error_Success:
		{
			// Suceeded, clean the lists then fill with the retrieved data
			mAuthServers.RemoveAll();
			mContestServers.RemoveAll();
			mEventServers.RemoveAll();
			mFirewallServers.RemoveAll();
			mProfileServers.RemoveAll();

			// Process the directory entities and store into our above allocated lists
			for( DirEntityList::const_iterator aDirEntityListItr = entityList.begin(); 
					aDirEntityListItr != entityList.end(); ++aDirEntityListItr)
			{
				// Is this entity's type a directory?
				if (aDirEntityListItr->mType == WONMsg::DirEntity::ET_DIRECTORY)
				{
					// Search through the data objects (look for version info)
					for( DataObjectTypeSet::const_iterator aDataObjectSetItr = aDirEntityListItr->mDataObjects.begin();
							aDataObjectSetItr != aDirEntityListItr->mDataObjects.end(); ++aDataObjectSetItr)
					{
						// Is this the data object type we wanted? (specified above)
						if (aDataObjectSetItr->GetDataType() == OBJ_VALIDVERSIONS)
							mValidVersions = reinterpret_cast<const char*>(aDataObjectSetItr->GetData().c_str());
					}
				}
				else if (aDirEntityListItr->mName == SERVERNAME_AUTH)
					// An Authentication server
					mAuthServers.Add( IPSocket::Address(*aDirEntityListItr));
				else if (aDirEntityListItr->mName == SERVERNAME_CONTEST)
					// A Contest server
					mContestServers.Add(IPSocket::Address(*aDirEntityListItr));
				else if (aDirEntityListItr->mName == SERVERNAME_EVENT)
					// An Event server
					mEventServers.Add(IPSocket::Address(*aDirEntityListItr));
				else if (aDirEntityListItr->mName == SERVERNAME_FIREWALL)
					// A Firewall detection server
					mFirewallServers.Add(IPSocket::Address(*aDirEntityListItr));
				else if (aDirEntityListItr->mName == SERVERNAME_PROFILE)
					// A Profile server
					mProfileServers.Add(IPSocket::Address(*aDirEntityListItr));
			}//end Process directory list
			break;
		}//end case Error_Success
		case StatusDir_DirNotFound:
			MessageBox(NULL,"Directory containing Titan servers not found","ERROR", MB_OK);
			break;
		default:
			MessageBox(NULL,"Failed to get list of Titan servers!","ERROR", MB_OK);
			break;

	}//end switch(result) from GetDirectory()
}//end DownloadServerLists()

 
 
 
 
/********************************************************************************
 * Send the text to the chat view, chat window is redrawn automatically			*
 ********************************************************************************/
void CWhiteBoardDoc::ReceiveText(const CString& inBuf)
{
	// Have the chat view append the string to the text control 
	// (chat view updates window automatically)
	m_pChatView->DisplayText(inBuf);
}

void CWhiteBoardDoc::ReceiveText(LPSTR lpText)
{
	// Convert this char* buffer into a CString that the chat view
	// can append to it's window
	m_pChatView->DisplayText( CString(lpText) );
}

void CWhiteBoardDoc::SendText(CString& theString)
{
	if (mRoutingServer.IsRegistered())
		mRoutingServer.BroadcastChat(string(theString,theString.GetLength()), false);
}






void CWhiteBoardDoc::OnOptionsBoards() 
{
	// Are we logged in?...
	if (!bIsLoggedIn)	
	{
		// ...nope! 
		ReceiveText("<System> ***Error Not Logged In *** \r\n");
		return;
	}
	
	// Are we connected to a chat room?
	if (!mRoutingServer.IsRegistered())
	{
		ReceiveText("<System> ***Error Not connected to a chat service*** \r\n");
		return;
	}

	// Create the dialog box
	CBoardsDialog theBoardsDialog;

	// Run the dialog box, selected room name is retrieved
	CString selectedRoomName;
	int result = theBoardsDialog.DoModal(selectedRoomName,mGameMap);

	// Did the user press cancel?
	if (result == IDCANCEL || selectedRoomName.IsEmpty())
	{
		ReceiveText("<System> Join/Create game action cancelled\r\n");
		return;
	}
	
	// Room was specified, does it exist?
	GameMap::const_iterator itr = mGameMap.find( (LPCSTR)selectedRoomName );	

	if (itr == mGameMap.end())
	{
		// Room doesn't exist, create it
		ReceiveText("<System> Attemping to create game...\r\n");
		m_pChatView->CreateBoard(selectedRoomName);
		return; // (return and wait for callback from create game to be received by the view)
	}

	/**************** JOIN GAME ****************/
	ReceiveText("<System> Attempting to join game...\r\n");

	// Construct and send a join request message to the other players
	// Send clientID and clientInfo also
	RawBuffer aMsg;
	aMsg.assign((unsigned char*)&MSGTYPE_JOIN_REQUEST, sizeof(MSGTYPE_JOIN_REQUEST));
	mRoutingServer.SendData( &(itr->second.mOwnerId), 1, true, aMsg, false);
}


void CWhiteBoardDoc::OnOptionsStrokeColor() 
{
	// Display a color selection dialog box
	CColorDialog theColorDialog(m_CurStrokeColor, CC_ANYCOLOR);
	theColorDialog.DoModal();

	m_CurStrokeColor = theColorDialog.GetColor();

}

void CWhiteBoardDoc::OnOptionsStrokeSize() 
{
	// Display a dialog box so the user can input the new string
	CStrokeSizeDialog theDialog;
	m_nCurStrokeSize = theDialog.DoModal(m_nCurStrokeSize);
}


void CWhiteBoardDoc::OnDataObjectCreated(const RoutingServerClient::DataObjectWithLifespan& theObjectR, CWhiteBoardDoc* pThat)
{
	// Add the game name to the GameMap for easy listing 
	char* gamename = (char*)theObjectR.mDataType.c_str() + OBJ_GAMEPREFIX.size();
	pThat->mGameMap[gamename] = theObjectR;
}

void CWhiteBoardDoc::OnDataObjectDeleted(const RoutingServerClient::DataObject& theObjectR, CWhiteBoardDoc* pThat)
{
	// Remove the game from the game map
	char* gamename = (char*)theObjectR.mDataType.c_str() + OBJ_GAMEPREFIX.size();
	pThat->mGameMap.erase(gamename);
}










void CWhiteBoardDoc::OnRoutingMessageReceived(const RoutingServerClient::MessageWithClientId& theMessageR, CWhiteBoardDoc* pThat)
{
	// Retrieve the ClientID of the message sender
	RoutingServerClient::ClientId theSenderId = theMessageR.mClientId;
	int nBufSize = theMessageR.mBufLen;
	unsigned char theMessageType = theMessageR.mBufP[0];

	switch(theMessageType)
	{
	case MSGTYPE_STROKE:
		// A stroke from another client
		pThat->ProcessMsgStroke(theMessageR.mBufP);
		break;
	case MSGTYPE_JOIN_REQUEST:
		// A user wants to join
		pThat->ProcessMsgJoinRequest(theMessageR);
		break;
	case MSGTYPE_JOIN_REPLY:
		// A reply from the server to this instances join request
		pThat->ProcessMsgJoinReply(theMessageR);
		break;
	case MSGTYPE_PLAYER_JOIN:
		// General broadcast, a player has joined 
		
		// Add them to the local observer relay list
		pThat->mObserverIds[pThat->mNumObservers++] = theSenderId;

		// Notify the user 
		
		break;
	case MSGTYPE_PLAYER_LEAVE:
		// General broadcast, a player has parted
		{
			// Search for the parted users id in the observer relay list
			for(int client_num = 0; client_num < pThat->mNumObservers; client_num++)
			{
				// Are they in the list?
				if (pThat->mObserverIds[client_num] == theSenderId)
				{
					// Set id and decrement the total number of observers
					pThat->mObserverIds[client_num] = 0;
					pThat->mNumObservers--;
				}
				// else
				//		they weren't in the list, nothing to do (should never occur)
			}
		}
	default:
		break;
	};

}

void CWhiteBoardDoc::ProcessMsgStroke(const unsigned char* pBuffer)
{
	// Create a new stroke to hold the buffer
	CStroke* pStroke = new CStroke;
	pStroke->LoadFromBuffer(pBuffer);

	// Add the new stroke to the end of the list
	m_StrokeList.AddTail(pStroke);

	// Draw the new stroke (only one stroke) on the screen
	m_pStrokeView->DrawSingleStroke(pStroke);
}

void CWhiteBoardDoc::ProcessMsgJoinRequest(const RoutingServerClient::MessageWithClientId& theMessageR)
{
	RoutingServerClient::ClientId theSenderId = theMessageR.mClientId;
	int nBufSize = theMessageR.mBufLen;
	unsigned char theMessageType = theMessageR.mBufP[0];

	// Add user to the client list
	// Broadcast a user join message so all users will add the new user
	SMsgJoinReply theJoinReply;
	theJoinReply.mType = MSGTYPE_JOIN_REPLY;
	theJoinReply.mNumIds = mNumObservers;
	memcpy( theJoinReply.mIds, mObserverIds, sizeof(unsigned short)*MAX_OBSERVERS);

	RawBuffer theBuffer;
	theBuffer.assign( (unsigned char*)&theJoinReply, sizeof(SMsgJoinReply));

	// Send a JoinReply message back to the joining client
	mRoutingServer.SendData( &theSenderId, 1, true, theBuffer, false);

	// Relay the contents of the stroke buffer to the joining client
	if(m_StrokeList.IsEmpty())
		return;

	CStroke* pStroke = m_StrokeList.GetHead();

	for(POSITION pos = m_StrokeList.GetHeadPosition(); pos != NULL;)
	{
		int buf_size = pStroke->GetSize();
		unsigned char* pDataBuffer = new unsigned char[buf_size];

		// Store the stroke data into the buffer
		pStroke->StoreToBuffer(pDataBuffer);

		// Copy the character buffer into a server readable rawbuffer for net transfer
		RawBuffer theRawBuf;
		theRawBuf.assign(pDataBuffer, buf_size);
		
		// Send this stroke to the newly joined client
		mRoutingServer.SendData( &theSenderId, 1, true, theRawBuf, false);
		
		// delete our linear buffer
		delete[] pDataBuffer;

		// Get the next stroke position
		pStroke = m_StrokeList.GetNext(pos);
	}
}

void CWhiteBoardDoc::ProcessMsgJoinReply(const RoutingServerClient::MessageWithClientId& theMessageR)
{
	// This should be the client list so the new user can add them
	SMsgJoinReply* pJoinReply;
	pJoinReply = (SMsgJoinReply*)theMessageR.mBufP;

	// Transfer the ids from the buffer to the document
	memcpy(mObserverIds, pJoinReply->mIds, sizeof(unsigned short)*MAX_OBSERVERS);
	mNumObservers = pJoinReply->mNumIds;

	// Now add the captains id
	mObserverIds[mNumObservers++] = theMessageR.mClientId; 

	// We now have access to the board
	bOnBoard = true;

	// Relay our Id to the rest of the clients
	RawBuffer theBuffer;
	theBuffer.assign( (unsigned char*)&MSGTYPE_PLAYER_JOIN, sizeof(MSGTYPE_PLAYER_JOIN));
	mRoutingServer.SendData(mObserverIds, mNumObservers, true, theBuffer, false);

	// Notify the user that the connection has been established
	ReceiveText("<System> Access to board granted.\r\n");
}



