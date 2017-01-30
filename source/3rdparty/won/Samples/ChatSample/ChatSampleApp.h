// ChatSampleApp.h: interface for the CChatSampleApp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHATSAMPLEAPP_H__0E54A3EA_1F5B_11D4_803B_0050046D274E__INCLUDED_)
#define AFX_CHATSAMPLEAPP_H__0E54A3EA_1F5B_11D4_803B_0050046D274E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Map of the downloaded chat servers
struct ChatServerInfo {
	WONAPI::IPSocket::Address mAddress;
	unsigned long             mClientCount;
	bool                      mIsPasswordProtected;
	ChatServerInfo() : mClientCount(0), mIsPasswordProtected(false) {}
};
typedef std::map<std::wstring, ChatServerInfo> ChatServerMap;

// Map of clients, keyed by ClientId
typedef std::map<WONAPI::RoutingServerClient::ClientId, WONMsg::MMsgRoutingGetClientListReply::ClientData> ClientMap;


class CChatSampleApp  
{
// Attributes
public:
	// Server counters
	int m_nNumAuthServers;							// Number of Authentication servers
	int m_nNumDirServers;							// Number of Directory servers
	int m_nNumFactoryServers;						// Number of Factory servers

	// Addresses and ports of the various servers
	IPSocket::Address* m_pAuthServers;				// Auth server IPs
	IPSocket::Address* m_pDirServers;				// Directory server IPs
	IPSocket::Address* m_pFactoryServers;			// Factory server IPs
	
	FILE* m_pLogFile;								// Our logfile
	char  m_InputBuf[255];							// Stores user input
	WORD  m_Status;									// What state is our app in?
	
	CUser			m_User;							// The user object
	CConsole		m_Console;						// The console object
	ChatServerMap	m_ChatServersMap;				// Chat rooms
	ClientMap		m_ClientMap;					// List of room occupants

	WONAPI::RoutingServerClient	mRoutingServer;			// our current routing server
	IPSocket::Address			theCurrentChatServer;	// the chat room we're in

// Operations
public:
	 int CreateRoom(const string&, const string&);	// Create a room
	 int DisplayRooms();							// Display the rooms
	void DrawPrompt();								// Draws the command prompt
	 int JoinRoom(const string&, const string&);	// Join a room
	bool LoadTitanServers();						// Load the authentication servers
	 int ProcessCommands();							// Process commands from theInputBuf
	void ReadInput();								// Reads a line of input from the console
	void ReportLog(const char*);					// Display and log a message
	 int Run();										// Run the main loop
	
	// Initialization routines
	BOOL Initialize();								// Initailize the application
	BOOL LoadVerifierKey();							// Loads the verifier key from a file
	BOOL LoadDirectoryServers();					// Loads IP's for directory servers
	BOOL InstallCallbacks();						// Installs the callbacks defined below

	// Callbacks
	static void ClientListCallback  (const RoutingServerClient::GetClientListResult&,	CChatSampleApp*);
	static void OnChatReceived		(const RoutingServerClient::ASCIIChatMessage&,		CChatSampleApp*);
	static void OnChatReceived		(const RoutingServerClient::UnicodeChatMessage&,	CChatSampleApp*);
	static void OnClientEnterChat	(const RoutingServerClient::ClientDataWithReason&,	CChatSampleApp*);
	static void OnClientLeaveChat	(const RoutingServerClient::ClientIdWithReason&,	CChatSampleApp*);

// Construction/Destruction
public:
	CChatSampleApp();
	virtual ~CChatSampleApp();

};

#endif // !defined(AFX_CHATSAMPLEAPP_H__0E54A3EA_1F5B_11D4_803B_0050046D274E__INCLUDED_)
