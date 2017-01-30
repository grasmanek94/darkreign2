
#pragma warning (disable : 4786)

#include <sys/types.h>
#include <sys/stat.h>
#include <common/won.h>
#ifndef DLLSAMPLE
#include <wondll.h>
#include <WONHTTP.h>
#include <msg/Dir/DirEntity.h>
#include <msg/Profile/SMsgProfileMsgs.h>
#include <Profile/ProfileAPI.h>
#endif // !DLLSAMPLE
#include <msg/ServerStatus.h>

namespace {
	using namespace std;
	using namespace WONMsg;
	using namespace WONCommon;
#ifndef DLLSAMPLE
	using namespace WONAPI;
#endif // !DLLSAMPLE
};

#include "main.h"
#include "matchmaker.h"
#include "matchmakerex.h"

extern long gRequestTimeout; // =-1 from matchmaker.cpp
const RawBuffer         OBJ_GAMEPREFIX           = (unsigned char*)"GAME";
const short             CONTESTPROC_REPORTRESULT = 8; // p_ttt_saveresults
const short             CONTESTPROC_TOPTENLIST   = 9; // p_ttt_gethighscores

// COMMAND LIST
const char* COMMAND_LIST_GAMES     = "GAMES";
const char* COMMAND_CREATE_GAME    = "CREATEGAME";
const char* COMMAND_JOIN_GAME      = "JOINGAME";
const char* COMMAND_LEAVE_GAME     = "LEAVEGAME";
const char* COMMAND_START_GAME     = "STARTGAME";
const char* COMMAND_MOVE           = "MOVE";
const char* COMMAND_TOP_TEN        = "TOPTEN";
extern const char* COMMAND_HELP;// = "HELP";			// also handled by main.cpp

// UTILITY (main.cpp)
extern int GetCommandToken(string& theDestString, char* theSourceBuffer, 
					int theTokenNum, BOOL copyToEnd);



MatchmakerEx::MatchmakerEx() :
	mGameInfo(mRoutingServer),
	mBehindFirewall(true)
{
	
}

MatchmakerEx::~MatchmakerEx()
{
}


bool MatchmakerEx::Init()
{
	// Init the base class methods
	if (Matchmaker::Init()==FALSE)
		return FALSE; /* abort */

	// Do our extened initialization here
#ifdef DLLSAMPLE
	WONRoutingInstallDataObjectCreationCallback(mRoutingServer, OnDataObjectCreated, this);
	WONRoutingInstallDataObjectDeletionCallback(mRoutingServer, OnDataObjectDeleted, this);
	WONRoutingInstallDataObjectReplacementCallback(mRoutingServer, OnDataObjectReplaced, this);
	WONRoutingInstallPeerDataCallback(mRoutingServer, OnPeerDataReceived, this);
#else
	mRoutingServer.InstallDataObjectCreationCatcherEx((void(*)(const RoutingServerClient::DataObjectWithLifespan&, MatchmakerEx*))OnDataObjectCreated, this);
	mRoutingServer.InstallDataObjectDeletionCatcherEx((void(*)(const RoutingServerClient::DataObject&, MatchmakerEx*))OnDataObjectDeleted, this);
	mRoutingServer.InstallDataObjectReplacementCatcherEx((void(*)(const RoutingServerClient::DataObject&, MatchmakerEx*))OnDataObjectReplaced, this);
	mRoutingServer.InstallPeerDataCatcherEx((void(*)(const RoutingServerClient::MessageWithClientId&, MatchmakerEx*))OnPeerDataReceived, this);
#endif // DLLSAMPLE

	// initiate firewall detection if there are firewall servers available
	if (mNumFirewallServers)
		DetectFirewall();

	// not subscribed to the games list
	mSubscribedToDataObjects = FALSE;


	return TRUE;
}

bool MatchmakerEx::HandleCommand(string& theCommand, char* theInputBuf)
{
		// Game routines
	if (theCommand==COMMAND_CREATE_GAME)
	{
		string aGameName, aGamePassword;

		if (GetCommandToken(aGameName,	   theInputBuf, 1, FALSE)==-1)
		{
			OutputString("You must specify the name of your new game.");
		}
		else
		{
			// Get the password
			GetCommandToken(aGamePassword, theInputBuf, 2, FALSE);

			// Convert game password unicode
			WONString aUnicodePassword(aGamePassword);
			CreateGame(aGameName.c_str(), aUnicodePassword.GetUnicodeCString());
		}
	}
	else if (theCommand==COMMAND_JOIN_GAME)
	{
		string aGameName, aGamePassword;

		if (GetCommandToken(aGameName,	   theInputBuf, 1, FALSE)==-1)
		{
			OutputString("You must specify the name of the game to join.");
		}
		else
		{
			// Get the password
			GetCommandToken(aGamePassword, theInputBuf, 2, FALSE);

			// Convert game password unicode
			WONString aUnicodePassword(aGamePassword);
			JoinGame(aGameName.c_str(), aUnicodePassword.GetUnicodeCString());
		}
	}
	else if (theCommand==COMMAND_LEAVE_GAME) 
		LeaveGame();
	else if (theCommand==COMMAND_LIST_GAMES) 
		ListGames();
	else if (theCommand==COMMAND_START_GAME) 
		StartGame();
	else if (theCommand==COMMAND_MOVE) 
	{
		string theMove;
		GetCommandToken(theMove,theInputBuf,1,TRUE);
		Move(theMove.c_str());
	}
	else if (theCommand==COMMAND_TOP_TEN)
		DisplayTopTen();
	else if (theCommand==COMMAND_HELP)
	{
		OutputString("\r\n* Game Options *\r\n");
		OutputStringF("/%s <game name> <password> - create a new game.", COMMAND_CREATE_GAME);
		OutputStringF("/%s <game name> <password> - join the specified game.", COMMAND_JOIN_GAME);
		OutputStringF("/%s - leave the game that you are currently in.", COMMAND_LEAVE_GAME);
		OutputStringF("/%s - get a list of games in current chat room.", COMMAND_LIST_GAMES);
		OutputStringF("/%s - start the game if you're the captain.", COMMAND_START_GAME);
		OutputStringF("/%s <position> - claim square specified by position (ex. A1)", COMMAND_MOVE);
		OutputStringF("/%s - outputs top ten list.", COMMAND_TOP_TEN);
		return FALSE;
	}
	else
		// Call the base class implementation
		return Matchmaker::HandleCommand(theCommand,theInputBuf);

	return TRUE;
}

void MatchmakerEx::CreateGame(const char* theGameName, const wchar_t* thePassword)
{
#ifdef DLLSAMPLE
	if (!WONRoutingIsRegistered(mRoutingServer))
#else
	if (!mRoutingServer.IsRegistered())
#endif // DLLSAMPLE
		OutputError("Not connected to a chat room.");
	else if (!mGameInfo.mMyGameName.empty())
		OutputError("You're already in a game.");
	else
	{
		if (thePassword != NULL && thePassword[0] != '\0')
			OutputString("FYI: Game passwords are not currently supported");

		RawBuffer aObjectName(OBJ_GAMEPREFIX + (unsigned char*)theGameName);

		RawBuffer aObject;
#ifdef DLLSAMPLE
		mGameInfo.mPlayersInGame[0] = WONRoutingGetClientId(mRoutingServer);
#else
		mGameInfo.mPlayersInGame[0] = mRoutingServer.GetClientId();
#endif // DLLSAMPLE
		mGameInfo.mNumPlayersInGame = 1;
		mGameInfo.mMyGameName = theGameName;
		aObject.assign((unsigned char*)mGameInfo.mPlayersInGame, 
#ifdef DLLSAMPLE
			sizeof(WONRoutingClientId));
#else
			sizeof(RoutingServerClient::ClientId));
#endif // DLLSAMPLE
#ifdef DLLSAMPLE
		WONRoutingCreateDataObjectAndGetResult(mRoutingServer, GROUPID_ALLUSERS, aObjectName.data(), aObjectName.size(), WONRoutingGetClientId(mRoutingServer), 0, aObject.data(), aObject.size(), CreateDataObjectCallback, this);
#else
		mRoutingServer.CreateDataObjectEx(RoutingServerClient::GROUPID_ALLUSERS, aObjectName, mRoutingServer.GetClientId(), 0, aObject, (void(*)(short, MatchmakerEx*))CreateDataObjectCallback, this);
#endif // DLLSAMPLE

		// initialize score info
		mScoreSoFar.Clear(true);
#ifdef DLLSAMPLE
		mScoreSoFar.mPlayer1.mUserSeq = WONAuthGetUserID(mAuthH);
#else
		mScoreSoFar.mPlayer1.mUserSeq = mIdentity.GetUserId();
#endif // DLLSAMPLE
	}
}

void MatchmakerEx::JoinGame(const char* theGameName, const wchar_t* thePassword)
{
#ifdef DLLSAMPLE
	if (!WONRoutingIsRegistered(mRoutingServer))
#else
	if (!mRoutingServer.IsRegistered())
#endif // DLLSAMPLE
		OutputError("Not connected to a chat room.");
	else
	{
		if (thePassword != NULL && thePassword[0] != '\0')
			OutputString("FYI: Game passwords are not currently supported");

		GameMap::const_iterator itr = mGameMap.find(theGameName);
		if (itr != mGameMap.end())
		{
			OutputString("Sending join request.");
			JoinRequestMsg aJoinRequest;
			RawBuffer aMsg;
            aMsg.assign((unsigned char*)&aJoinRequest, sizeof(aJoinRequest));
#ifdef DLLSAMPLE
			WONRoutingSendData(mRoutingServer, &(itr->second.mOwnerId), 1, TRUE, aMsg.data(), aMsg.size());
#else
            mRoutingServer.SendData(&(itr->second.mOwnerId), 1, true, aMsg, false);		
#endif // DLLSAMPLE
		}
		else
			OutputError("Specified game not found");
	}
}

void MatchmakerEx::LeaveGame()
{
#ifdef DLLSAMPLE
	if (!WONRoutingIsRegistered(mRoutingServer))
#else
	if (!mRoutingServer.IsRegistered())
#endif // DLLSAMPLE
		OutputError("Not connected to a chat room.");
	else if (!IsInAGame())
		OutputError("Not currently in a game.");
	else if (mGameInfo.IAmCaptain())
	{
		// remove the game from the game list
		RawBuffer aObjectType(OBJ_GAMEPREFIX + (unsigned char*)mGameInfo.mMyGameName.c_str());
#ifdef DLLSAMPLE
		WONRoutingDeleteDataObject(mRoutingServer, GROUPID_ALLUSERS, aObjectType.data(), aObjectType.size());
#else
		mRoutingServer.DeleteDataObject(RoutingServerClient::GROUPID_ALLUSERS, aObjectType);
#endif // DLLSAMPLE

		// tell current players that the game has been dissolved
		GameDissolvedMsg aGameDissolved;
		RawBuffer aMsg;
		aMsg.assign((unsigned char*)&aGameDissolved, sizeof(aGameDissolved));
#ifdef DLLSAMPLE
		WONRoutingSendData(mRoutingServer, mGameInfo.mPlayersInGame, mGameInfo.mNumPlayersInGame, TRUE, aMsg.data(), aMsg.size());
#else
		mRoutingServer.SendData(mGameInfo.mPlayersInGame, mGameInfo.mNumPlayersInGame, true, aMsg, false);
#endif // DLLSAMPLE
		mGameInfo.Reset();

		// save results via Contest Server
		ReportResults();

		OutputString("Game dissolved.");
	}
	else
	{
		LeaveGameMsg aLeaveGame;
		RawBuffer aMsg;
        aMsg.assign((unsigned char*)&aLeaveGame, sizeof(aLeaveGame));
#ifdef DLLSAMPLE
		WONRoutingSendData(mRoutingServer, &mGameInfo.mCaptainId, 1, TRUE, aMsg.data(), aMsg.size());
#else
        mRoutingServer.SendData(&mGameInfo.mCaptainId, 1, true, aMsg, false);
#endif // DLLSAMPLE
		mGameInfo.Reset();
		OutputString("Ok");
	}
}

void MatchmakerEx::ListGames()
{
#ifdef DLLSAMPLE
	if (!WONRoutingIsRegistered(mRoutingServer))
#else
	if (!mRoutingServer.IsRegistered())
#endif // DLLSAMPLE
		OutputError("Not connected to a chat room.");
	else if (!mSubscribedToDataObjects)
	{
		// Subscribe to the Data Objects
		OutputString("Loading game list...");
#ifdef DLLSAMPLE
		WONRoutingSubscribeDataObject(mRoutingServer, GROUPID_ALLUSERS, OBJ_GAMEPREFIX.data(), OBJ_GAMEPREFIX.size(), FALSE, TRUE, SubscribeDataObjectCallback, this);
#else
		mRoutingServer.SubscribeDataObjectEx(RoutingServerClient::GROUPID_ALLUSERS, OBJ_GAMEPREFIX, false, true, (void(*)(const RoutingServerClient::ReadDataObjectResult& result, MatchmakerEx*))SubscribeDataObjectCallback, this);
#endif // DLLSAMPLE
	}
	else if (mGameMap.size() == 0)
		OutputError("No games available.");
	else
	{
		GameMap::const_iterator itr = mGameMap.begin();
		OutputString("Name\tPlayer Count");
		for ( ; itr != mGameMap.end(); ++itr)
#ifdef DLLSAMPLE
			OutputStringF("%s\t%d", itr->first.c_str(), itr->second.mDataLen/2);
#else
			OutputStringF("%s\t%d", itr->first.c_str(), itr->second.mData.size()/2);
#endif // DLLSAMPLE
	}
}

#ifdef DLLSAMPLE
void MatchmakerEx::AcceptMove(unsigned char thePlayer, WONRoutingClientId theClientId, unsigned int theX, unsigned int theY)
#else
void MatchmakerEx::AcceptMove(unsigned char thePlayer, RoutingServerClient::ClientId theClientId, unsigned int theX, unsigned int theY)
#endif // DLLSAMPLE
{
	MoveAcceptedMsg aMoveAccepted(theClientId, thePlayer, theX, theY);
	RawBuffer aMsg;
    aMsg.assign((unsigned char*)&aMoveAccepted, sizeof(aMoveAccepted));
#ifdef DLLSAMPLE
		WONRoutingSendData(mRoutingServer, mGameInfo.mPlayersInGame, mGameInfo.mNumPlayersInGame, TRUE, aMsg.data(), aMsg.size());
#else
    mRoutingServer.SendData(mGameInfo.mPlayersInGame, mGameInfo.mNumPlayersInGame, true, aMsg, false);
#endif // DLLSAMPLE
	
	mGameInfo.mWhoseTurnIsIt = (++mGameInfo.mWhoseTurnIsIt) % mGameInfo.mNumPlayersInGame;
	YourTurnMsg aYourTurn;
	aMsg.assign((unsigned char*)&aYourTurn, sizeof(aYourTurn));
#ifdef DLLSAMPLE
	WONRoutingSendData(mRoutingServer, &mGameInfo.mPlayersInGame[mGameInfo.mWhoseTurnIsIt], 1, TRUE, aMsg.data(), aMsg.size());
#else
    mRoutingServer.SendData(&mGameInfo.mPlayersInGame[mGameInfo.mWhoseTurnIsIt], 1, true, aMsg, false);
#endif // DLLSAMPLE
}

void MatchmakerEx::StartGame()
{
	if (!mGameInfo.IAmCaptain())
		OutputError("You're not the captain of a/the game");
	else if (mGameInfo.mNumPlayersInGame != TicTacToe::MAX_PLAYERS_PER_GAME)
		OutputError("Waiting for more players...");
	else if (mGameInfo.mGameHasStarted)
		OutputError("Game has already been started.");
	else
	{
		mGameInfo.mGameHasStarted = true;
		mGameInfo.mWhoseTurnIsIt = 0;
		StartGameMsg aStartGame;
		RawBuffer aMsg;
        aMsg.assign((unsigned char*)&aStartGame, sizeof(aStartGame));
#ifdef DLLSAMPLE
		WONRoutingSendData(mRoutingServer, mGameInfo.mPlayersInGame, mGameInfo.mNumPlayersInGame, TRUE, aMsg.data(), aMsg.size());
#else
        mRoutingServer.SendData(mGameInfo.mPlayersInGame, mGameInfo.mNumPlayersInGame, true, aMsg, false);
#endif // DLLSAMPLE
	}
}

void MatchmakerEx::DetectFirewall()
{
#ifdef DLLSAMPLE
	HWONCOMPLETION aCompletionH = WONCreateCallbackWithDataCompletion(DetectFirewallCallback, this);
	WONFirewallDetectAsync(mFirewallServers, mNumFirewallServers, &this->mBehindFirewall, GAME_PORT, gRequestTimeout, aCompletionH);
#else
	WONAPI::DetectFirewallEx(mFirewallServers, mNumFirewallServers,
	                         &this->mBehindFirewall, GAME_PORT, 
	                         gRequestTimeout, true, (void(*)(const DetectFirewallResult&, MatchmakerEx*))DetectFirewallCallback, this);
#endif // DLLSAMPLE
}

void MatchmakerEx::Move(const char* theMove)
{
	if (!IsInAGame())
		OutputString("Not currently in a game.");
	else if (strlen(theMove) != 2)
		OutputError("Invalid move.  Try again");
	else
	{
		char aColumn = toupper(theMove[0]);
		char aRow = theMove[1];

		if (aColumn < 'A' || aColumn > 'Z' || 
			aRow < '1'    || aRow > '9')
			OutputError("Invalid move.  Try again");
		else
		{
			unsigned int aX = aColumn - 'A';
			unsigned int aY = aRow - '1';
			if (!mGameInfo.IAmCaptain())
			{
				MoveRequestMsg aMoveRequest(aX, aY);
				RawBuffer aMsg;
				aMsg.assign((unsigned char*)&aMoveRequest, sizeof(aMoveRequest));
#ifdef DLLSAMPLE
				WONRoutingSendData(mRoutingServer, &mGameInfo.mCaptainId, 1, TRUE, aMsg.data(), aMsg.size());
#else
				mRoutingServer.SendData(&mGameInfo.mCaptainId, 1, true, aMsg, false);		
#endif // DLLSAMPLE
			}
			else
			{
#ifdef DLLSAMPLE
				if (mGameInfo.mPlayersInGame[mGameInfo.mWhoseTurnIsIt] != WONRoutingGetClientId(mRoutingServer) || !mGameInfo.ValidateMove(aX, aY))
#else
				if (mGameInfo.mPlayersInGame[mGameInfo.mWhoseTurnIsIt] != mRoutingServer.GetClientId() || !mGameInfo.ValidateMove(aX, aY))
#endif // DLLSAMPLE
				{
					OutputString("Invalid move (or maybe it's not your turn?)");
					return;
				}
#ifdef DLLSAMPLE
				AcceptMove(PLAYER_O, WONRoutingGetClientId(mRoutingServer), aX, aY);
#else
				AcceptMove(PLAYER_O, mRoutingServer.GetClientId(), aX, aY);
#endif // DLLSAMPLE
			}
		}
	}
}

void MatchmakerEx::DisplayTopTen()
{
	if (!IsLoggedIn())
		OutputError("Need username and password.");
	else if (mNumFactoryServers == 0)
		OutputString("No contest servers available.");
	else
	{
		TopTenList aTopTenList;
		memset(&aTopTenList, 0, sizeof(aTopTenList));
		unsigned short aRecvSize = sizeof(aTopTenList);

		// retrieve top ten list
		GUID aGuid;	memset(&aGuid,0,sizeof(aGuid));
#ifdef DLLSAMPLE
		WONError aError = WONContestDBCall(mContestServers, mNumContestServers, mAuthH,
										   gRequestTimeout, CONTESTPROC_TOPTENLIST, &aGuid, 
										   0, NULL, &aRecvSize, &aTopTenList);
#else
		Error aError = ContestDBCall(&mIdentity, mContestServers, mNumContestServers, 
 					  CONTESTPROC_TOPTENLIST, &aGuid,
					  0, NULL, &aRecvSize, &aTopTenList, 
					  gRequestTimeout, false);
#endif // DLLSAMPLE

		switch (aError)
		{
			case StatusCommon_Success:
			{
				// display the list
				OutputString ("    Name            Wins Losses Ties");
				for (int i = 0; i < aTopTenList.mNumRecords; ++i)
					OutputStringF("%2d. %-15s %4d  %4d  %4d", i+1, aTopTenList.mRecords[i].mName, aTopTenList.mRecords[i].mWins, aTopTenList.mRecords[i].mLosses, aTopTenList.mRecords[i].mTies);
				break;
			}
			default:
				OutputError("Failed to get top ten list", aError);
				break;
		}
	}
}

void MatchmakerEx::ReportResults()
{
	if (!IsLoggedIn())
		OutputError("Need username and password.");
	else if (mNumFactoryServers == 0)
		OutputString("No contest servers available.");
	else
	{
		GUID aGuid;	memset(&aGuid,0,sizeof(aGuid));
#ifdef DLLSAMPLE
		HWONCOMPLETION aCompletionH = WONCreateCallbackWithDataCompletion(ReportResultsCallback, this);
		WONContestDBCallAsync(mContestServers, mNumContestServers, mAuthH, 
							  gRequestTimeout, CONTESTPROC_REPORTRESULT,
							  &aGuid, sizeof(ScoreReport), &mScoreSoFar,
							  NULL, NULL, aCompletionH);
#else
		Error aError = ContestDBCallEx(&mIdentity, mContestServers, mNumContestServers, 
 					  CONTESTPROC_REPORTRESULT, &aGuid,
					  sizeof(ScoreReport), &mScoreSoFar, NULL, NULL, 
					  gRequestTimeout, true, (void(*)(const ContestDBResult&, MatchmakerEx*))ReportResultsCallback, this);
#endif // DLLSAMPLE
	}
}


//
// CALLBACKS
//
#ifdef DLLSAMPLE
void MatchmakerEx::DetectFirewallCallback(HWONCOMPLETION hCompletion, void* param, void* theMatchmakerP)
{
	WONError aError = reinterpret_cast<WONError>(param);
	MatchmakerEx* that = reinterpret_cast<MatchmakerEx*>(theMatchmakerP);
#else
void MatchmakerEx::DetectFirewallCallback(const DetectFirewallResult& theResultR, MatchmakerEx* that)
{
	Error aError = theResultR.error;
#endif // DLLSAMPLE
	if (aError == StatusCommon_Success)
	{
		if (that->mBehindFirewall)
			OutputString("You are behind a firewall.");
		else
			OutputString("You are not behind a firewall.");
	}
	else
		OutputError("Failed to complete firewall detection!", aError);
}

#ifdef DLLSAMPLE
void MatchmakerEx::SubscribeDataObjectCallback(WONRoutingReadDataObjectResult* theObjectsP, void* theMatchmakerExP)
{
	MatchmakerEx* that = reinterpret_cast<MatchmakerEx*>(theMatchmakerExP);
	short aStatus = theObjectsP->mStatus;
#else
	void MatchmakerEx::SubscribeDataObjectCallback(const RoutingServerClient::ReadDataObjectResult& theResultR, MatchmakerEx* that)
{
	short aStatus = theResultR.mStatus;
#endif // DLLSAMPLE
	switch (aStatus)
	{
		case StatusCommon_Success:
		{
			OutputError("Subscribed to game list successfully");
			that->mSubscribedToDataObjects = TRUE;
#ifdef DLLSAMPLE
			for (int iObject = 0; iObject < theObjectsP->mNumObjects; ++iObject)
			{
				string gamename = (char*)theObjectsP->mObjects[iObject].mDataType + OBJ_GAMEPREFIX.size();
				that->mGameMap[gamename] = theObjectsP->mObjects[iObject];
			}
#else
			for (std::list<RoutingServerClient::DataObject>::const_iterator itr = theResultR.mDataObjectList.begin(); itr != theResultR.mDataObjectList.end(); itr++)
			{
				string gamename = (char*)itr->mDataType.data() + OBJ_GAMEPREFIX.size();
				that->mGameMap[gamename] = *itr; 
			}
#endif // DLLSAMPLE

			that->ListGames();
			break;
		}
		case StatusRouting_SubscriptionAlreadyExists:
			OutputError("Can not get game list again");
			break;
		default:
			OutputError("Fail to retrieve game list", aStatus);
			break;
	}
}

#ifdef DLLSAMPLE
void MatchmakerEx::CreateDataObjectCallback(short theStatus, void* theMatchmakerExP)
{
	MatchmakerEx* that = reinterpret_cast<MatchmakerEx*>(theMatchmakerExP);
#else
void MatchmakerEx::CreateDataObjectCallback(short theStatus, MatchmakerEx* that)
{
#endif // DLLSAMPLE
	switch(theStatus)
	{
		case StatusCommon_Success:
			OutputError("Created game successfully");
#ifdef DLLSAMPLE
			that->mGameInfo.mCaptainId = WONRoutingGetClientId(that->mRoutingServer);
#else
			that->mGameInfo.mCaptainId = that->mRoutingServer.GetClientId();
#endif // DLLSAMPLE
			break;
		case StatusRouting_ObjectAlreadyExists:
			OutputError("Game with that name already exists.  Choose another name.");
			break;
		case StatusRouting_ObjectTooLarge:
			OutputError("Too much game data");
			break;
		default:
			OutputError("Fail to create game", theStatus);
			break;
	}
}

#ifdef DLLSAMPLE
void MatchmakerEx::ReplaceDataObjectCallback(short theStatus, void* theMatchmakerExP)
{
	MatchmakerEx* that = reinterpret_cast<MatchmakerEx*>(theMatchmakerExP);
#else
void MatchmakerEx::ReplaceDataObjectCallback(short theStatus, MatchmakerEx* that)
{
#endif // DLLSAMPLE
	switch(theStatus)
	{
		case StatusCommon_Success:
		{
			OutputError("Successfully updated game object");
			if (that->mGameInfo.mJoiningClientId)
			{
				JoinReplyMsg aJoinReply(true);
				RawBuffer aMsg;
                aMsg.assign((unsigned char*)&aJoinReply, sizeof(aJoinReply));
#ifdef DLLSAMPLE
				WONRoutingSendData(that->mRoutingServer, &that->mGameInfo.mJoiningClientId, 1, TRUE, aMsg.data(), aMsg.size());
#else
                that->mRoutingServer.SendData(&that->mGameInfo.mJoiningClientId, 1, true, aMsg, false);
#endif // DLLSAMPLE
			}
			break;
		}
		default:
			OutputError("Failed to update game object!", theStatus);
			break;
	}
}

#ifdef DLLSAMPLE
void MatchmakerEx::ReportResultsCallback(HWONCOMPLETION hCompletion, void* param, void* theMatchmakerExP)
{
	MatchmakerEx* that = reinterpret_cast<MatchmakerEx*>(theMatchmakerExP);
	WONError aError = reinterpret_cast<WONError>(param);
#else
void MatchmakerEx::ReportResultsCallback(const ContestDBResult& theResultR, MatchmakerEx* that)
{
	Error aError = theResultR.error;
#endif // DLLSAMPLE
	switch(aError)
	{
		case StatusCommon_Success:
			OutputError("Successfully recorded scores");
			break;
		default:
			OutputError("Failed to record scores via Contest Server!", aError);
			break;
	}
}

//
// CATCHERS
//
#ifdef DLLSAMPLE
void MatchmakerEx::OnDataObjectCreated(WONRoutingDataObjectWithLifespan* theDataP, void* theMatchmakerExP)
{
	MatchmakerEx* that = reinterpret_cast<MatchmakerEx*>(theMatchmakerExP);
	char* gamename = (char*)theDataP->mObject.mDataType + OBJ_GAMEPREFIX.size();
	OutputStringF("New game created: %s", gamename);
	that->mGameMap[gamename] = theDataP->mObject;
}
#else
void MatchmakerEx::OnDataObjectCreated(const RoutingServerClient::DataObjectWithLifespan& theObjectR, MatchmakerEx* that)
{
	char* gamename = (char*)theObjectR.mDataType.c_str() + OBJ_GAMEPREFIX.size();
	OutputStringF("New game created: %s", gamename);
	that->mGameMap[gamename] = theObjectR;
}
#endif // DLLSAMPLE
#ifdef DLLSAMPLE
void MatchmakerEx::OnDataObjectDeleted(WONRoutingDataObject* theDataP, void* theMatchmakerExP)
{
	MatchmakerEx* that = reinterpret_cast<MatchmakerEx*>(theMatchmakerExP);
	char* gamename = (char*)theDataP->mDataType + OBJ_GAMEPREFIX.size();
	OutputStringF("Game deleted: %s", gamename);
	that->mGameMap.erase(gamename);
}
#else
void MatchmakerEx::OnDataObjectDeleted(const RoutingServerClient::DataObject& theObjectR, MatchmakerEx* that)
{
	char* gamename = (char*)theObjectR.mDataType.c_str() + OBJ_GAMEPREFIX.size();
	OutputStringF("Game deleted: %s", gamename);
	that->mGameMap.erase(gamename);
}
#endif // DLLSAMPLE
#ifdef DLLSAMPLE
void MatchmakerEx::OnDataObjectReplaced(WONRoutingDataObject* theDataP, void* theMatchmakerExP)
{
	MatchmakerEx* that = reinterpret_cast<MatchmakerEx*>(theMatchmakerExP);
	char* gamename = (char*)theDataP->mDataType + OBJ_GAMEPREFIX.size();
	OutputStringF("Game data updated: %s", gamename);
	that->mGameMap[gamename].mData = theDataP->mData; 
}
#else
void MatchmakerEx::OnDataObjectReplaced(const RoutingServerClient::DataObject& theObjectR, MatchmakerEx* that)
{
	char* gamename = (char*)theObjectR.mDataType.c_str() + OBJ_GAMEPREFIX.size();
	OutputStringF("Game data updated: %s", gamename);
	that->mGameMap[gamename].mData = theObjectR.mData; 
}
#endif // DLLSAMPLE
#ifdef DLLSAMPLE
void MatchmakerEx::OnPeerDataReceived(WONRoutingMessageWithClientId* theMsgP, void* theMatchmakerExP)
{
	MatchmakerEx* that = reinterpret_cast<MatchmakerEx*>(theMatchmakerExP);

	WONRoutingClientId theSenderId = theMsgP->mClientId;
	const char* theBufP = theMsgP->mData;
	unsigned short theBufLen = theMsgP->mDataLen;
#else
void MatchmakerEx::OnPeerDataReceived(const RoutingServerClient::MessageWithClientId& theMessageR, MatchmakerEx* that)
{
	RoutingServerClient::ClientId theSenderId = theMessageR.mClientId;
	const unsigned char* theBufP = theMessageR.mBufP;
	unsigned short theBufLen = theMessageR.mBufLen;
#endif // DLLSAMPLE
	if (theBufLen == 0)
		return;

	MessageType theGameMessageType = theBufP[0];
	switch (theGameMessageType)
	{
		case MSGTYPE_JOIN_REQUEST:
		{
			if (theBufLen < sizeof(JoinRequestMsg))
				return;
#ifdef DLLSAMPLE
			OutputStringF("Received join request from %S", reinterpret_cast<const unsigned short*>(that->mClientMap.find(theSenderId)->second.mClientNameW));
#else
			OutputStringF("Received join request from %S", reinterpret_cast<const unsigned short*>(that->mClientMap.find(theSenderId)->second.mClientName.c_str()));
#endif // DLLSAMPLE
			if (that->mGameInfo.mNumPlayersInGame >= TicTacToe::MAX_PLAYERS_PER_GAME)
			{
				JoinReplyMsg aJoinReply(false);
				RawBuffer aMsg;
                aMsg.assign((unsigned char*)&aJoinReply, sizeof(aJoinReply));
#ifdef DLLSAMPLE
				WONRoutingSendData(that->mRoutingServer, &theSenderId, 1, TRUE, aMsg.data(), aMsg.size());
#else
                that->mRoutingServer.SendData(&theSenderId, 1, true, aMsg, false);
#endif // DLLSAMPLE
			}
			else
			{
				that->mGameInfo.mPlayersInGame[that->mGameInfo.mNumPlayersInGame++] = theSenderId;
				that->mGameInfo.mJoiningClientId = theSenderId;
				RawBuffer aDataType(OBJ_GAMEPREFIX + (unsigned char*)that->mGameInfo.mMyGameName.c_str());
				RawBuffer aData;
                aData.assign((unsigned char*)that->mGameInfo.mPlayersInGame, 
#ifdef DLLSAMPLE
				             that->mGameInfo.mNumPlayersInGame*sizeof(WONRoutingClientId));
#else
				             that->mGameInfo.mNumPlayersInGame*sizeof(RoutingServerClient::ClientId));
#endif // DLLSAMPLE
#ifdef DLLSAMPLE
				WONRoutingReplaceDataObjectAndGetResult(that->mRoutingServer, GROUPID_ALLUSERS, aDataType.data(), aDataType.size(), aData.data(), aData.size(), ReplaceDataObjectCallback, that);
#else
				that->mRoutingServer.ReplaceDataObjectEx(RoutingServerClient::GROUPID_ALLUSERS, aDataType, aData, (void(*)(short, MatchmakerEx*))ReplaceDataObjectCallback, that);
#endif // DLLSAMPLE

				// set new player's id in score info
				ClientMap::const_iterator itr = that->mClientMap.find(theSenderId);
				that->mScoreSoFar.mPlayer2.mUserSeq = itr->second.mWONUserId;
				that->mScoreSoFar.Clear();
			}
			break;
		}
		case MSGTYPE_JOIN_REPLY:
		{
			if (theBufLen < sizeof(JoinReplyMsg))
				return;
			const JoinReplyMsg* aJoinReplyP = reinterpret_cast<const JoinReplyMsg*>(theBufP);
			if (aJoinReplyP->mAcceptRejectFlag)
			{
				OutputString("Captain accepted you into game");
				that->mGameInfo.mCaptainId = theSenderId;
			}
			else
				OutputString("Captain rejected your join request for game");
			break;
		}
		case MSGTYPE_LEAVE_GAME:
		{
			if (theBufLen < sizeof(LeaveGameMsg))
				return;
			for (unsigned int i = 0; i < that->mGameInfo.mNumPlayersInGame; ++i)
			{
				if (that->mGameInfo.mPlayersInGame[i] == theSenderId)
				{
					memmove(&that->mGameInfo.mPlayersInGame[i], &that->mGameInfo.mPlayersInGame[i+1], 
#ifdef DLLSAMPLE
					        sizeof(WONRoutingClientId) * (that->mGameInfo.mNumPlayersInGame - i - 1));
#else
					        sizeof(RoutingServerClient::ClientId) * (that->mGameInfo.mNumPlayersInGame - i - 1));
#endif // DLLSAMPLE
					--that->mGameInfo.mNumPlayersInGame;
					that->mGameInfo.mJoiningClientId = 0;
					RawBuffer aDataType(OBJ_GAMEPREFIX + (unsigned char*)that->mGameInfo.mMyGameName.c_str());
					RawBuffer aData;
                    aData.assign((unsigned char*)that->mGameInfo.mPlayersInGame, 
#ifdef DLLSAMPLE
					             that->mGameInfo.mNumPlayersInGame*sizeof(WONRoutingClientId));
#else
					             that->mGameInfo.mNumPlayersInGame*sizeof(RoutingServerClient::ClientId));
#endif // DLLSAMPLE
#ifdef DLLSAMPLE
					WONRoutingReplaceDataObjectAndGetResult(that->mRoutingServer, GROUPID_ALLUSERS, aDataType.data(), aDataType.size(), aData.data(), aData.size(), ReplaceDataObjectCallback, that);
#else
                    that->mRoutingServer.ReplaceDataObjectEx(RoutingServerClient::GROUPID_ALLUSERS, aDataType, aData, (void(*)(short, MatchmakerEx*))ReplaceDataObjectCallback, that);
#endif // DLLSAMPLE

					// save results via Contest Server
					that->ReportResults();

					break;
				}
			}
			if (that->mGameInfo.mNumPlayersInGame == 1)
				that->mGameInfo.mGameHasStarted = false;

			break;
		}
		case MSGTYPE_START_GAME:
		{
			if (theBufLen < sizeof(StartGameMsg))
				return;

			OutputString("Game starting...");
			that->mGameInfo.mGameHasStarted = true;
			that->mGameInfo.ResetBoard();
			that->mGameInfo.DisplayBoard();
			break;
		}
		case MSGTYPE_YOUR_TURN:
		{
			if (theBufLen < sizeof(YourTurnMsg))
				return;
			OutputString("It's your turn");
			break;
		}
		case MSGTYPE_MOVE_REQUEST:
		{
			if (theBufLen < sizeof(MoveRequestMsg))
				return;
			const MoveRequestMsg* aMoveRequestP = reinterpret_cast<const MoveRequestMsg*>(theBufP);
			
			if (that->mGameInfo.mPlayersInGame[that->mGameInfo.mWhoseTurnIsIt] == theSenderId && that->mGameInfo.ValidateMove(aMoveRequestP->mX, aMoveRequestP->mY))
				that->AcceptMove(PLAYER_X, theSenderId, aMoveRequestP->mX, aMoveRequestP->mY);
			else
			{
				MoveRejectedMsg aMoveRejected;
				RawBuffer aMsg;
                aMsg.assign((unsigned char*)&aMoveRejected, sizeof(aMoveRejected));
#ifdef DLLSAMPLE
				WONRoutingSendData(that->mRoutingServer, &theSenderId, 1, TRUE, aMsg.data(), aMsg.size());
#else
                that->mRoutingServer.SendData(&theSenderId, 1, true, aMsg, false);
#endif // DLLSAMPLE
			}
			break;
		}
		case MSGTYPE_MOVE_ACCEPTED:
		{
			if (theBufLen < sizeof(MoveAcceptedMsg))
				return;
			const MoveAcceptedMsg* aMoveAcceptedP = reinterpret_cast<const MoveAcceptedMsg*>(theBufP);
#ifdef DLLSAMPLE
			if (aMoveAcceptedP->mClientId != WONRoutingGetClientId(that->mRoutingServer))
				OutputStringF("%S took square %c%d", reinterpret_cast<const unsigned short*>(that->mClientMap.find(aMoveAcceptedP->mClientId)->second.mClientNameW), 'A' + aMoveAcceptedP->mX, aMoveAcceptedP->mY + 1);
#else
			if (aMoveAcceptedP->mClientId != that->mRoutingServer.GetClientId())
				OutputStringF("%S took square %c%d", reinterpret_cast<const unsigned short*>(that->mClientMap.find(aMoveAcceptedP->mClientId)->second.mClientName.c_str()), 'A' + aMoveAcceptedP->mX, aMoveAcceptedP->mY + 1);
#endif // DLLSAMPLE
			that->mGameInfo.mGameBoard[aMoveAcceptedP->mX][aMoveAcceptedP->mY] = aMoveAcceptedP->mPlayer;
			that->mGameInfo.DisplayBoard();
			if (that->mGameInfo.IAmCaptain())
			{
				if (that->mGameInfo.IsWinningMove(aMoveAcceptedP->mPlayer, aMoveAcceptedP->mX, aMoveAcceptedP->mY))
				{
					GameOverMsg aGameOver(aMoveAcceptedP->mClientId);
					RawBuffer aMsg;
                    aMsg.assign((unsigned char*)&aGameOver, sizeof(aGameOver));
#ifdef DLLSAMPLE
					WONRoutingSendData(that->mRoutingServer, that->mGameInfo.mPlayersInGame, that->mGameInfo.mNumPlayersInGame, TRUE, aMsg.data(), aMsg.size());
#else
                    that->mRoutingServer.SendData(that->mGameInfo.mPlayersInGame, that->mGameInfo.mNumPlayersInGame, true, aMsg, false);
#endif // DLLSAMPLE

					// update score info
#ifdef DLLSAMPLE
					if (aMoveAcceptedP->mClientId == WONRoutingGetClientId(that->mRoutingServer))
#else
					if (aMoveAcceptedP->mClientId == that->mRoutingServer.GetClientId())
#endif // DLLSAMPLE
					{
						++that->mScoreSoFar.mPlayer1.mWins;
						++that->mScoreSoFar.mPlayer2.mLosses;
					}
					else
					{
						++that->mScoreSoFar.mPlayer1.mLosses;
						++that->mScoreSoFar.mPlayer2.mWins;
					}
				}
				else if (that->mGameInfo.IsBoardFull())
				{
					GameOverMsg aGameOver;
					RawBuffer aMsg;
                    aMsg.assign((unsigned char*)&aGameOver, sizeof(aGameOver));
#ifdef DLLSAMPLE
					WONRoutingSendData(that->mRoutingServer, that->mGameInfo.mPlayersInGame, that->mGameInfo.mNumPlayersInGame, TRUE, aMsg.data(), aMsg.size());
#else
                    that->mRoutingServer.SendData(that->mGameInfo.mPlayersInGame, that->mGameInfo.mNumPlayersInGame, true, aMsg, false);
#endif // DLLSAMPLE

					// update score info
					++that->mScoreSoFar.mPlayer1.mTies;
					++that->mScoreSoFar.mPlayer2.mTies;
				}
			}
			break;
		}
		case MSGTYPE_MOVE_REJECTED:
		{
			if (theBufLen < sizeof(MoveRejectedMsg))
				return;
			const MoveRejectedMsg* aMoveRejectedP = reinterpret_cast<const MoveRejectedMsg*>(theBufP);
			OutputString("Invalid move (or maybe it's not your turn?)");
			break;
		}
		case MSGTYPE_GAME_OVER:
		{
			if (theBufLen < sizeof(GameOverMsg))
				return;
			const GameOverMsg* aGameOverP = reinterpret_cast<const GameOverMsg*>(theBufP);
			that->mGameInfo.mGameHasStarted = false;
			if (aGameOverP->mClientId)
			{
#ifdef DLLSAMPLE
				if (aGameOverP->mClientId == WONRoutingGetClientId(that->mRoutingServer))
#else
				if (aGameOverP->mClientId == that->mRoutingServer.GetClientId())
#endif // DLLSAMPLE
					OutputString("GAME OVER: You win!!! :)");
				else
#ifdef DLLSAMPLE
					OutputStringF("GAME OVER: %S wins. :(", reinterpret_cast<const unsigned short*>(that->mClientMap.find(theSenderId)->second.mClientNameW));
#else
					OutputStringF("GAME OVER: %S wins. :(", reinterpret_cast<const unsigned short*>(that->mClientMap.find(theSenderId)->second.mClientName.c_str()));
#endif // DLLSAMPLE
			}
			else
				OutputString("GAME OVER: It's a draw.");
			break;
		}
		case MSGTYPE_GAME_DISSOLVED:
		{
			if (theBufLen < sizeof(GameDissolvedMsg))
				return;
			that->mGameInfo.Reset();
			OutputString("Captain has dissolved the game");
		}
	}
}

bool MatchmakerEx::IsInAGame() const
{ return mGameInfo.mCaptainId != 0; }