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
#include "game.h"

class Matchmaker {
public:
	Matchmaker();
	~Matchmaker();

	bool Init();
	void Close();
	
	// firewall detection
	void DetectFirewall();

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
	
	// game stuff (finding, creating, joining, leaving, starting)
	void ListGames();
	void CreateGame(const char* theGameName, const wchar_t* thePassword);
	void JoinGame(const char* theGameName, const wchar_t* thePassword);
	void LeaveGame();
	void StartGame();
	bool IsInAGame() const;

	// contest/results stuff
	void DisplayTopTen();
	void ReportResults();

	// make a move in the game
	void Move(const char* theMove);

#ifdef DLLSAMPLE
	HWONROUTINGSESSION mRoutingServer;
#else
	WONAPI::RoutingServerClient mRoutingServer;
#endif // DLLSAMPLE
	TicTacToe                   mGameInfo;
private:
	void GetDirServers();
	void GetTitanServerList();
	void JoinChatRoom();
#ifdef DLLSAMPLE
	void AcceptMove(unsigned char thePlayer, WONRoutingClientId theClientId, unsigned int theX, unsigned int theY);
#else
	void AcceptMove(unsigned char thePlayer, WONAPI::RoutingServerClient::ClientId theClientId, unsigned int theX, unsigned int theY);
#endif // DLLSAMPLE

	// asynchronous event catchers
#ifdef DLLSAMPLE
	static void OnClientEnterChat(WONRoutingGroupIdAndClientDataWithReason *theDataP, void* theMatchmakerP);
	static void OnClientLeaveChat(WONRoutingGroupIdAndClientIdWithReason* theDataP, void* theMatchmakerP);
	static void OnDataObjectCreated(WONRoutingDataObjectWithLifespan *theDataP, void* theMatchmakerP);
	static void OnDataObjectDeleted(WONRoutingDataObject* theDataP, void* theMatchmakerP);
	static void OnDataObjectReplaced(WONRoutingDataObject* theDataP, void* theMatchmakerP);
	static void OnPeerDataReceived(WONRoutingMessageWithClientId* theMsgP, void* theMatchmakerP);
	static void OnChatReceived(WONRoutingASCIIChatMessage* theMsgP, void* theMatchmakerP);
	static void OnChatReceived(WONRoutingUnicodeChatMessage* theMsgP, void* theMatchmakerP);
	static void OnReconnectFailure(void* that);
#else
	static void OnClientEnterChat(const WONAPI::RoutingServerClient::GroupIdAndClientDataWithReason& theDataR, Matchmaker* that);
	static void OnClientLeaveChat(const WONAPI::RoutingServerClient::GroupIdAndClientIdWithReason& theDataR, Matchmaker* that);
	static void OnDataObjectCreated(const WONAPI::RoutingServerClient::DataObjectWithLifespan& theObjectR, Matchmaker* that);
	static void OnDataObjectDeleted(const WONAPI::RoutingServerClient::DataObject& theObjectR, Matchmaker* that);
	static void OnDataObjectReplaced(const WONAPI::RoutingServerClient::DataObject& theObjectR, Matchmaker* that);
	static void OnPeerDataReceived(const WONAPI::RoutingServerClient::MessageWithClientId& theMessageR, Matchmaker* that);
	static void OnChatReceived(const WONAPI::RoutingServerClient::ASCIIChatMessage& theMessageR, Matchmaker* that);
	static void OnChatReceived(const WONAPI::RoutingServerClient::UnicodeChatMessage& theMessageR, Matchmaker* that);
	static void OnReconnectFailure(Matchmaker* that);
#endif // DLLSAMPLE

	// callbacks
#ifdef DLLSAMPLE
	static void DetectFirewallCallback(HWONCOMPLETION hCompletion, void* param, void* theMatchmakerP);
	static BOOL PatchProgressCallback(unsigned long progress, unsigned long size, void* callbackPrivData);
	static void PatchDownloadCallback(HWONCOMPLETION hCompletion, void* param);
	static void TitanServerEntityCallback(HWONDIRENTITY theDirEntityH, void* theMatchmakerP);
	static void ChatRoomEntityCallback(HWONDIRENTITY theDirEntityH, void* theMatchmakerP);
	static void ChatRoomListCallback(HWONCOMPLETION hCompletion, void* param, void* theMatchmakerP);
	static void ChatRoomLoginCallback(WONRoutingRegisterClientResult* theResultP, void* theMatchmakerP);
	static void ClientListCallback(WONRoutingClientListResult* theListP, void* theMatchmakerP);
	static void SubscribeDataObjectCallback(WONRoutingReadDataObjectResult* theObjectsP, void* theMatchmakerP);
	static void CreateDataObjectCallback(short theStatus, void* theMatchmakerP);
	static void ReplaceDataObjectCallback(short theStatus, void* theMatchmakerP);
	static void ReportResultsCallback(HWONCOMPLETION hCompletion, void* param, void* theMatchmakerP);
#else
	static void DetectFirewallCallback(const WONAPI::DetectFirewallResult& theResultR, Matchmaker* that);
	static bool PatchProgressCallback(unsigned long progress, unsigned long size, void* callbackPrivData);
	static void PatchDownloadCallback(WONAPI::Error theError);
	static void ProfileQueryCallback(const WONAPI::AuthSocket::AuthRecvBaseMsgResult& theResultR, Matchmaker* that);
	static void ProfileUpdateCallback(const WONAPI::AuthSocket::AuthRecvBaseMsgResult& theResultR, Matchmaker* that);
	static void ProfileCreateCallback(const WONAPI::AuthSocket::AuthRecvBaseMsgResult& theResultR, Matchmaker* that);
	static void ChatRoomListCallback(const WONAPI::DirEntityListResult& theResultR, Matchmaker* that);
	static void ChatRoomLoginCallback(const WONAPI::RoutingServerClient::RegisterClientResult& theResultR, Matchmaker* that);
	static void ClientListCallback(const WONAPI::RoutingServerClient::GetClientListResult& theResultR, Matchmaker* that);
	static void SubscribeDataObjectCallback(const WONAPI::RoutingServerClient::ReadDataObjectResult& theResultR, Matchmaker* that);
	static void CreateDataObjectCallback(short theStatus, Matchmaker* that);
	static void ReplaceDataObjectCallback(short theStatus, Matchmaker* that);
	static void ReportResultsCallback(const WONAPI::ContestDBResult& theResultR, Matchmaker* that);
#endif // DLLSAMPLE
private:
	enum { GAME_PORT = 54321 }; // replace with your game port

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
#ifdef DLLSAMPLE
	WONIPAddress* mAuthServers;
	WONIPAddress* mContestServers;
	WONIPAddress* mDirServers;
	WONIPAddress* mEventServers;
	WONIPAddress* mFactoryServers;
	WONIPAddress* mFirewallServers;
	WONIPAddress* mProfileServers;

	std::list<WONIPAddress> mNewAuthServers;
	std::list<WONIPAddress> mNewContestServers;
	std::list<WONIPAddress> mNewEventServers;
	std::list<WONIPAddress> mNewFactoryServers;
	std::list<WONIPAddress> mNewFirewallServers;
	std::list<WONIPAddress> mNewProfileServers;
#else
	WONAPI::IPSocket::Address* mAuthServers;
	WONAPI::IPSocket::Address* mContestServers;
	WONAPI::IPSocket::Address* mDirServers;
	WONAPI::IPSocket::Address* mEventServers;
	WONAPI::IPSocket::Address* mFactoryServers;
	WONAPI::IPSocket::Address* mFirewallServers;
	WONAPI::IPSocket::Address* mProfileServers;
#endif // DLLSAMPLE
	
	struct ChatServerInfo {
#ifdef DLLSAMPLE
		WONIPAddress              mAddress;
#else
		WONAPI::IPSocket::Address mAddress;
#endif // DLLSAMPLE
		unsigned long             mClientCount;
		bool                      mIsPasswordProtected;
		ChatServerInfo() : mClientCount(0), mIsPasswordProtected(false) {}
	};
	typedef std::map<std::wstring, ChatServerInfo> ChatServerMap;
	ChatServerMap mChatServers;
	
	std::string      mValidVersions;
#ifdef DLLSAMPLE
	HWONAUTH mAuthH;
	BOOL     mBehindFirewall;
#else
	WONAPI::Identity mIdentity;
	bool             mBehindFirewall;
#endif // DLLSAMPLE

	// current chat room info
	std::string               mRoomBeingStarted;
	std::wstring              mPasswordAttempt;
#ifdef DLLSAMPLE
	WONIPAddress mCurChatServerAddr;
#else
	WONAPI::IPSocket::Address mCurChatServerAddr;
#endif // DLLSAMPLE
	WONCommon::WONString      mCurChatServerName;

	// clients & games on current chat server
#ifdef DLLSAMPLE
	typedef std::map<WONRoutingClientId, WONRoutingClientData> ClientMap;
	typedef std::map<std::string, WONRoutingDataObject> GameMap;
#else
	typedef std::map<WONAPI::RoutingServerClient::ClientId, WONMsg::MMsgRoutingGetClientListReply::ClientData> ClientMap;
	typedef std::map<std::string, WONAPI::RoutingServerClient::DataObject> GameMap;
#endif // DLLSAMPLE
	ClientMap mClientMap;
	GameMap   mGameMap;

	// data for Contest Server blobs
#pragma pack(2)
	struct PlayerScore {
		unsigned long mUserSeq;
		unsigned short mWins;
		unsigned short mLosses;
		unsigned short mTies;
		void Clear(bool clearUserSeq =false) { mWins = mLosses = mTies = 0; if (clearUserSeq) mUserSeq = 0; }
	};

	struct ScoreReport {
		PlayerScore mPlayer1;
		PlayerScore mPlayer2;
		void Clear(bool clearUserSeqs =false) { mPlayer1.Clear(clearUserSeqs); mPlayer2.Clear(clearUserSeqs); }
	};

	struct PlayerRecord {
		char mName[66];
		unsigned long mWins;
		unsigned long mLosses;
		unsigned long mTies;
		char mLastUpdateTime[22];
	};

	struct TopTenList {
		unsigned long mNumRecords;
		PlayerRecord mRecords[10];
	};
#pragma pack()

	ScoreReport mScoreSoFar;
};

#endif // MATCHMAKER_H
