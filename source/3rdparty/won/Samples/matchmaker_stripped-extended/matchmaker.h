#ifndef MATCHMAKER_H
#define MATCHMAKER_H

#ifdef DLLSAMPLE
#include <wondll.h>
#include <common/WONString.h>
#else
#include <WONHTTP.h>
#include <Authentication/Identity.h>
#include <Contest/ContestAPI.h>
#include <Directory/DirectoryAPI.h>
#include <Factory/FactoryAPI.h>
#include <Firewall/FirewallAPI.h>
#include <Routing/RoutingAPI.h>
#endif // DLLSAMPLE

#include <list>
#include <map>
#include <string>

#ifdef DLLSAMPLE
	typedef WONIPAddress				SOCKETADDRESS;
#else
	typedef WONAPI::IPSocket::Address	SOCKETADDRESS;
#endif 

class Matchmaker {
public:
	Matchmaker();
	~Matchmaker();

	// Operations to override
	virtual bool Init();
	virtual bool HandleCommand(string& theCommand, char* theInputBuf);


	void Close();
	

	// MOTD
	void GetMOTD();

	// Patching
	void GetPatch();

	// create/use WON user accounts
	bool IsValidVersion(const char* version);
	void CreateWONAccount(const char* playerName, const char* password);
	void Login(const char* playerName, const char* password);
	void SaveEmailAddressInProfile(bool reset =true);
	bool IsLoggedIn() const;
	
	// chat room stuff (finding, creating, joining)
	void ListRooms();
	void CreateRoom(const char* theRoomName, const wchar_t* thePassword);
	void JoinRoom(const char* theRoomName, const wchar_t* thePassword);
	const WONCommon::WONString& GetCurrentChatRoomName() const { return mCurChatServerName; }
	void ListUsers();
	void IgnoreUser(const char* theUserName);
	void UnignoreUser(const char* theUserName);
	
#ifdef DLLSAMPLE
	HWONAUTH mAuthH;
#else
	WONAPI::Identity mIdentity;
#endif // DLLSAMPLE

#ifdef DLLSAMPLE
	HWONROUTINGSESSION mRoutingServer;
#else
	WONAPI::RoutingServerClient mRoutingServer;
#endif // DLLSAMPLE
//	TicTacToe                   mGameInfo;
protected:
	void GetDirServers();
	void GetTitanServerList();
	void JoinChatRoom();

	// asynchronous event catchers
#ifdef DLLSAMPLE
	static void OnClientEnterChat(WONRoutingGroupIdAndClientDataWithReason *theDataP, void* theMatchmakerP);
	static void OnClientLeaveChat(WONRoutingGroupIdAndClientIdWithReason* theDataP, void* theMatchmakerP);
	static void OnChatReceived(WONRoutingASCIIChatMessage* theMsgP, void* theMatchmakerP);
	static void OnChatReceived(WONRoutingUnicodeChatMessage* theMsgP, void* theMatchmakerP);
	static void OnReconnectFailure(void* that);
#else
	static void OnClientEnterChat(const WONAPI::RoutingServerClient::GroupIdAndClientDataWithReason& theDataR, Matchmaker* that);
	static void OnClientLeaveChat(const WONAPI::RoutingServerClient::GroupIdAndClientIdWithReason& theDataR, Matchmaker* that);
	static void OnAsciiChatReceived(const WONAPI::RoutingServerClient::ASCIIChatMessage& theMessageR, Matchmaker* that);
	static void OnUnicodeChatReceived(const WONAPI::RoutingServerClient::UnicodeChatMessage& theMessageR, Matchmaker* that);
	static void OnReconnectFailure(Matchmaker* that);
#endif // DLLSAMPLE

	// callbacks
#ifdef DLLSAMPLE
	static BOOL PatchProgressCallback(unsigned long progress, unsigned long size, void* callbackPrivData);
	static void PatchDownloadCallback(HWONCOMPLETION hCompletion, void* param);
	static void TitanServerEntityCallback(HWONDIRENTITY theDirEntityH, void* theMatchmakerP);
	static void ChatRoomEntityCallback(HWONDIRENTITY theDirEntityH, void* theMatchmakerP);
	static void ChatRoomListCallback(HWONCOMPLETION hCompletion, void* param, void* theMatchmakerP);
	static void ChatRoomLoginCallback(WONRoutingRegisterClientResult* theResultP, void* theMatchmakerP);
	static void ClientListCallback(WONRoutingClientListResult* theListP, void* theMatchmakerP);
#else
	static bool PatchProgressCallback(unsigned long progress, unsigned long size, void* callbackPrivData);
	static void PatchDownloadCallback(WONAPI::Error theError);
	static void ProfileQueryCallback(const WONAPI::AuthSocket::AuthRecvBaseMsgResult& theResultR, Matchmaker* that);
	static void ProfileUpdateCallback(const WONAPI::AuthSocket::AuthRecvBaseMsgResult& theResultR, Matchmaker* that);
	static void ProfileCreateCallback(const WONAPI::AuthSocket::AuthRecvBaseMsgResult& theResultR, Matchmaker* that);
	static void ChatRoomListCallback(const WONAPI::DirEntityListResult& theResultR, Matchmaker* that);
	static void ChatRoomLoginCallback(const WONAPI::RoutingServerClient::RegisterClientResult& theResultR, Matchmaker* that);
	static void ClientListCallback(const WONAPI::RoutingServerClient::GetClientListResult& theResultR, Matchmaker* that);
#endif // DLLSAMPLE
protected:
	// servers
	unsigned int mNumAuthServers;
	unsigned int mNumChatServers;
	unsigned int mNumContestServers;
	unsigned int mNumDirServers;
	unsigned int mNumEventServers;
	unsigned int mNumFactoryServers;
	unsigned int mNumFirewallServers;
	unsigned int mNumProfileServers;
	
	unsigned int mNumProfileServersTried;
	unsigned int mCurProfileServer;

	SOCKETADDRESS* mAuthServers;
	SOCKETADDRESS* mContestServers;
	SOCKETADDRESS* mDirServers;
	SOCKETADDRESS* mEventServers;
	SOCKETADDRESS* mFactoryServers;
	SOCKETADDRESS* mFirewallServers;
	SOCKETADDRESS* mProfileServers;

#ifdef DLLSAMPLE
	std::list<SOCKETADDRESS> mNewAuthServers;
	std::list<SOCKETADDRESS> mNewContestServers;
	std::list<SOCKETADDRESS> mNewEventServers;
	std::list<SOCKETADDRESS> mNewFactoryServers;
	std::list<SOCKETADDRESS> mNewFirewallServers;
	std::list<SOCKETADDRESS> mNewProfileServers;
#endif
	
	struct ChatServerInfo {
		SOCKETADDRESS	mAddress;
		unsigned long	mClientCount;
		bool			mIsPasswordProtected;
		ChatServerInfo() : mClientCount(0), mIsPasswordProtected(false) {}
	};
	typedef std::map<std::wstring, ChatServerInfo> ChatServerMap;
	ChatServerMap mChatServers;
	
	std::string      mValidVersions;


	// current chat room info
	std::string             mRoomBeingStarted;
	std::wstring            mPasswordAttempt;
	SOCKETADDRESS			mCurChatServerAddr;
	WONCommon::WONString    mCurChatServerName;

	// clients & games on current chat server
#ifdef DLLSAMPLE
	typedef std::map<WONRoutingClientId, WONRoutingClientData> ClientMap;
#else
	typedef std::map<WONAPI::RoutingServerClient::ClientId, WONMsg::MMsgRoutingGetClientListReply::ClientData> ClientMap;
#endif // DLLSAMPLE
	ClientMap mClientMap;


};

#endif // MATCHMAKER_H
