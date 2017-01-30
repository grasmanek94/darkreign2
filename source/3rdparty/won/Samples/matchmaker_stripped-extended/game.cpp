#pragma warning(disable : 4786)
#include <common/won.h>
#include <common/WONString.h>
#include "main.h"
#include "game.h"

#ifdef DLLSAMPLE
TicTacToe::TicTacToe()
#else
TicTacToe::TicTacToe(const WONAPI::RoutingServerClient& theRoutingServerR) :
	mRoutingServerR(theRoutingServerR)
#endif // DLLSAMPLE
{
	Reset();
}

#ifdef DLLSAMPLE
void TicTacToe::Init(HWONROUTINGSESSION theRoutingServer)
{ mRoutingServer = theRoutingServer; }
#endif // DLLSAMPLE

void TicTacToe::Reset()
{
	mCaptainId = 0;
	mJoiningClientId = 0;
	mGameHasStarted = false;
	mWhoseTurnIsIt = 0;
	mMyGameName.erase();
	mNumPlayersInGame = 0;
	ResetBoard();
}

void TicTacToe::ResetBoard()
{
	for (unsigned int x = 0; x < BOARD_SIZE; ++x)
		for (unsigned int y = 0; y < BOARD_SIZE; ++y)
			mGameBoard[x][y] = PLAYER_NONE;
}

void TicTacToe::DisplayBoard() const
{
	static std::string aHorizLine(BOARD_SIZE * 4 - 1, '-');
	for (int y = -1; y < BOARD_SIZE; ++y) // y == -1 is the header row (A B C)
	{
#ifdef _LINUX
		WONCommon::wonstringstream aRow;
#else
		std::stringstream aRow;
#endif // _LINUX
		for (unsigned int x = 0; x < BOARD_SIZE; ++x)
		{
			if (y >= 0)
			{
				if (x == 0)
					aRow << y+1 << " ";

				aRow << " ";
				aRow << mGameBoard[x][y];
				aRow << " ";

				if (x != BOARD_SIZE - 1)
					aRow << '|';
			}
			else
			{
				if (x == 0)
					aRow << "  ";
				aRow << " " << (char)(x+'A') << "  ";
			}
		}
		OutputString(aRow.str().c_str());
		if (y == -1)
			OutputString("");
		else if (y != BOARD_SIZE - 1)
			OutputStringF("  %s", aHorizLine.c_str());
	}
}

bool TicTacToe::IAmCaptain() const
{
	return mCaptainId != 0 &&
#ifdef DLLSAMPLE
           mCaptainId == WONRoutingGetClientId(mRoutingServer);
#else
           mCaptainId == mRoutingServerR.GetClientId();
#endif // DLLSAMPLE
}

bool TicTacToe::ValidateMove(unsigned int theX, unsigned int theY) const
{ return mGameBoard[theX][theY] == PLAYER_NONE; }

bool TicTacToe::IsWinningMove(unsigned char thePlayer, unsigned int theX, unsigned int theY) const
{
	bool aWin = true;
	unsigned int x, y;

	// check for horizantal win
	for (x = 0; x < BOARD_SIZE; ++x)
	{
		if (mGameBoard[x][theY] != thePlayer)
		{
			aWin = false;
			break;
		}
	}

	if (aWin) return true;

	// check for vertical win
	aWin = true;
	for (y = 0; y < BOARD_SIZE; ++y)
	{
		if (mGameBoard[theX][y] != thePlayer)
		{
			aWin = false;
			break;
		}
	}

	if (aWin) return true;

	// check for top-bottom diagonal win
	aWin = true;
	if (theX == theY)
	{
		for (x = 0, y = 0; x < BOARD_SIZE; ++x, ++y)
		{
			if (mGameBoard[x][y] != thePlayer)
			{
				aWin = false;
				break;
			}
		}

		if (aWin) return true;
	}

	// check for bottom-top diagonal win
	aWin = true;
	if (theX == BOARD_SIZE - 1 - theY)
	{
		for (x = 0, y = BOARD_SIZE - 1; x < BOARD_SIZE; ++x, --y)
		{
			if (mGameBoard[x][y] != thePlayer)
			{
				aWin = false;
				break;
			}
		}

		if (aWin) return true;
	}

	return false;
}

bool TicTacToe::IsBoardFull() const
{
	for (unsigned int x = 0; x < BOARD_SIZE; ++x)
		for (unsigned int y = 0; y < BOARD_SIZE; ++y)
			if (mGameBoard[x][y] == PLAYER_NONE)
				return false;

	return true;
}
