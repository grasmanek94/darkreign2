

#include "game.h"

class MatchmakerEx: public Matchmaker
{
public:
	MatchmakerEx();
	~MatchmakerEx();

	// game stuff (finding, creating, joining, leaving, starting)
	void ListGames();
	void CreateGame(const char* theGameName, const wchar_t* thePassword);
	void JoinGame(const char* theGameName, const wchar_t* thePassword);
	void LeaveGame();
	void StartGame();
	bool IsInAGame() const;
	bool mSubscribedToDataObjects;

	// firewall detection
	void DetectFirewall();

	// contest/results stuff
	void DisplayTopTen();
	void ReportResults();

	// make a move in the game
	void Move(const char* theMove);

#ifdef DLLSAMPLE
	BOOL     mBehindFirewall;
#else
	bool             mBehindFirewall;
#endif // DLLSAMPLE


	TicTacToe mGameInfo;

#ifdef DLLSAMPLE
	void AcceptMove(unsigned char thePlayer, WONRoutingClientId theClientId, unsigned int theX, unsigned int theY);
#else
	void AcceptMove(unsigned char thePlayer, WONAPI::RoutingServerClient::ClientId theClientId, unsigned int theX, unsigned int theY);
#endif // DLLSAMPLE

#ifdef DLLSAMPLE
	static void OnDataObjectCreated(WONRoutingDataObjectWithLifespan *theDataP, void* theMatchmakerP);
	static void OnDataObjectDeleted(WONRoutingDataObject* theDataP, void* theMatchmakerP);
	static void OnDataObjectReplaced(WONRoutingDataObject* theDataP, void* theMatchmakerP);
	static void OnPeerDataReceived(WONRoutingMessageWithClientId* theMsgP, void* theMatchmakerP);
#else
	static void OnDataObjectCreated(const WONAPI::RoutingServerClient::DataObjectWithLifespan& theObjectR, MatchmakerEx* that);
	static void OnDataObjectDeleted(const WONAPI::RoutingServerClient::DataObject& theObjectR, MatchmakerEx* that);
	static void OnDataObjectReplaced(const WONAPI::RoutingServerClient::DataObject& theObjectR, MatchmakerEx* that);
	static void OnPeerDataReceived(const WONAPI::RoutingServerClient::MessageWithClientId& theMessageR, MatchmakerEx* that);
#endif // DLLSAMPLE

		// callbacks
#ifdef DLLSAMPLE
	static void DetectFirewallCallback(HWONCOMPLETION hCompletion, void* param, void* theMatchmakerP);
	static void SubscribeDataObjectCallback(WONRoutingReadDataObjectResult* theObjectsP, void* theMatchmakerP);
	static void CreateDataObjectCallback(short theStatus, void* theMatchmakerP);
	static void ReplaceDataObjectCallback(short theStatus, void* theMatchmakerP);
	static void ReportResultsCallback(HWONCOMPLETION hCompletion, void* param, void* theMatchmakerP);
#else
	static void DetectFirewallCallback(const WONAPI::DetectFirewallResult& theResultR, MatchmakerEx* that);
	static void SubscribeDataObjectCallback(const WONAPI::RoutingServerClient::ReadDataObjectResult& theResultR, MatchmakerEx* that);
	static void CreateDataObjectCallback(short theStatus, MatchmakerEx* that);
	static void ReplaceDataObjectCallback(short theStatus, MatchmakerEx* that);
	static void ReportResultsCallback(const WONAPI::ContestDBResult& theResultR, MatchmakerEx* that);
#endif // DLLSAMPLE

	enum { GAME_PORT = 54321 }; // replace with your game port

	// clients & games on current chat server
#ifdef DLLSAMPLE
	typedef std::map<std::string, WONRoutingDataObject> GameMap;
#else
	typedef std::map<std::string, WONAPI::RoutingServerClient::DataObject> GameMap;
#endif // DLLSAMPLE
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
	// OVERRIDES
	bool Init();
	bool HandleCommand(string& theCommand, char* theInputBuf);
	




};