// Console.cpp: implementation of the CConsole class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable:4786)
#pragma warning (disable:4275)

#include "stdafx.h"
#include "Console.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConsole::CConsole()
{

}

CConsole::~CConsole()
{

}

void CConsole::Init()
{
	// Set the handle
	SetHandle( GetStdHandle(STD_OUTPUT_HANDLE) );

	// Set the console colors
	SetColor(CC_SYSTEM);

	// Clear the console
	Clear();
}





/********************************************************************************
 * Clear the console (blue background)			 								*
 ********************************************************************************/
void CConsole::Clear()
{
	COORD aTopLeftCoord = { 0, 0 };		// top left corner
	CONSOLE_SCREEN_BUFFER_INFO csbi;	// console settings
	DWORD numChars;						// number of characters
	DWORD aDiscardedParameter;

	/* get the number of character cells in the current buffer */
	GetConsoleScreenBufferInfo(m_hConsole, &csbi);
	numChars = csbi.dwSize.X * csbi.dwSize.Y;

	// Fill the console with spaces
	FillConsoleOutputCharacter(m_hConsole, (TCHAR)' ',
		numChars, aTopLeftCoord, &aDiscardedParameter);

	/* now set the buffer's attributes accordingly */
	FillConsoleOutputAttribute(m_hConsole, CC_SYSTEM,
		numChars, aTopLeftCoord, &aDiscardedParameter);

	/* put the cursor at (0, 0) */
	SetConsoleCursorPosition(m_hConsole, aTopLeftCoord);
}