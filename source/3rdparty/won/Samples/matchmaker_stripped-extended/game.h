#ifndef GAME_H
#define GAME_H

#ifdef DLLSAMPLE
#include <wondll.h>
#else
#include <Routing/RoutingAPI.h>
#endif // DLLSAMPLE

struct TicTacToe {
#ifdef DLLSAMPLE
	TicTacToe();
	void Init(HWONROUTINGSESSION theRoutingServer);
#else
	TicTacToe(const WONAPI::RoutingServerClient& theRoutingServerR);
#endif // DLLSAMPLE

	void Reset();
	void ResetBoard();
	void DisplayBoard() const;

	bool IAmCaptain() const;

	bool ValidateMove(unsigned int theX, unsigned int theY) const;
	bool IsWinningMove(unsigned char thePlayer, unsigned int theX, unsigned int theY) const;
	bool IsBoardFull() const;

	enum { MAX_PLAYERS_PER_GAME = 2 };
	enum { BOARD_SIZE = 3 };

	unsigned char mGameBoard[BOARD_SIZE][BOARD_SIZE];
	
	std::string mMyGameName;
	
#ifdef DLLSAMPLE
	WONRoutingClientId    mCaptainId;
	WONRoutingClientId    mJoiningClientId;
	WONRoutingClientId    mPlayersInGame[MAX_PLAYERS_PER_GAME];
#else
	WONAPI::RoutingServerClient::ClientId    mCaptainId;
	WONAPI::RoutingServerClient::ClientId    mJoiningClientId;
	WONAPI::RoutingServerClient::ClientId    mPlayersInGame[MAX_PLAYERS_PER_GAME];
#endif // DLLSAMPLE

	bool         mGameHasStarted;
	unsigned int mNumPlayersInGame;
	unsigned int mWhoseTurnIsIt;
private:
#ifdef DLLSAMPLE
	HWONROUTINGSESSION mRoutingServer;
#else
	const WONAPI::RoutingServerClient& mRoutingServerR;
#endif // DLLSAMPLE
};

const unsigned char PLAYER_NONE = (unsigned char)' ';
const unsigned char PLAYER_O = (unsigned char)'O';
const unsigned char PLAYER_X = (unsigned char)'X';

typedef unsigned char MessageType;
const MessageType MSGTYPE_INVALID        = 0;
const MessageType MSGTYPE_JOIN_REQUEST   = 1;
const MessageType MSGTYPE_JOIN_REPLY     = 2;
const MessageType MSGTYPE_YOUR_TURN      = 3;
const MessageType MSGTYPE_MOVE_REQUEST   = 4;
const MessageType MSGTYPE_MOVE_ACCEPTED  = 5;
const MessageType MSGTYPE_MOVE_REJECTED  = 6;
const MessageType MSGTYPE_GAME_OVER      = 7;
const MessageType MSGTYPE_START_GAME     = 8;
const MessageType MSGTYPE_LEAVE_GAME     = 9;
const MessageType MSGTYPE_GAME_DISSOLVED = 10;
struct GameMessage {
	MessageType mMessageType;
	GameMessage(MessageType theMessageType =MSGTYPE_INVALID) : mMessageType(theMessageType) {}
};
struct JoinRequestMsg : public GameMessage {
	JoinRequestMsg() : GameMessage(MSGTYPE_JOIN_REQUEST) {}
};
struct JoinReplyMsg : public GameMessage {
	bool mAcceptRejectFlag;
	JoinReplyMsg(bool flag =false) : GameMessage(MSGTYPE_JOIN_REPLY), mAcceptRejectFlag(flag) {}
};
struct YourTurnMsg : public GameMessage {
	YourTurnMsg() : GameMessage(MSGTYPE_YOUR_TURN) {}
};
struct MoveRequestMsg : public GameMessage {
	unsigned int mX;
	unsigned int mY;
	MoveRequestMsg() : GameMessage(MSGTYPE_MOVE_REQUEST), mX(0), mY(0) {}
	MoveRequestMsg(unsigned int theX, unsigned int theY) : GameMessage(MSGTYPE_MOVE_REQUEST), mX(theX), mY(theY) {}
};
struct MoveAcceptedMsg : public GameMessage {
#ifdef DLLSAMPLE
	WONRoutingClientId mClientId;
#else
	WONAPI::RoutingServerClient::ClientId mClientId;
#endif // DLLSAMPLE
	unsigned char mPlayer;
	unsigned int  mX;
	unsigned int  mY;
	MoveAcceptedMsg() : GameMessage(MSGTYPE_MOVE_ACCEPTED), mClientId(0), mPlayer(0), mX(0), mY(0) {}
#ifdef DLLSAMPLE
	MoveAcceptedMsg(WONRoutingClientId theClientId, unsigned char thePlayer, unsigned int theX, unsigned int theY) : GameMessage(MSGTYPE_MOVE_ACCEPTED), mClientId(theClientId), mPlayer(thePlayer), mX(theX), mY(theY) {}
#else
	MoveAcceptedMsg(WONAPI::RoutingServerClient::ClientId theClientId, unsigned char thePlayer, unsigned int theX, unsigned int theY) : GameMessage(MSGTYPE_MOVE_ACCEPTED), mClientId(theClientId), mPlayer(thePlayer), mX(theX), mY(theY) {}
#endif // DLLSAMPLE
};
struct MoveRejectedMsg : public GameMessage {
	MoveRejectedMsg() : GameMessage(MSGTYPE_MOVE_REJECTED) {}
};
struct GameOverMsg : public GameMessage {
#ifdef DLLSAMPLE
	WONRoutingClientId mClientId;
	GameOverMsg(WONRoutingClientId theWinner =0) :
#else
	WONAPI::RoutingServerClient::ClientId mClientId;
	GameOverMsg(WONAPI::RoutingServerClient::ClientId theWinner =0) :
#endif // DLLSAMPLE
		GameMessage(MSGTYPE_GAME_OVER), mClientId(theWinner) {}
};
struct StartGameMsg : public GameMessage {
	StartGameMsg() : GameMessage(MSGTYPE_START_GAME) {}
};
struct LeaveGameMsg : public GameMessage {
	LeaveGameMsg() : GameMessage(MSGTYPE_LEAVE_GAME) {}
};
struct GameDissolvedMsg : public GameMessage {
	GameDissolvedMsg() : GameMessage(MSGTYPE_GAME_DISSOLVED) {}
};

#endif // GAME_H