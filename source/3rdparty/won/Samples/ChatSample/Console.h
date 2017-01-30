// Console.h: interface for the CConsole class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONSOLE_H__0E54A3E9_1F5B_11D4_803B_0050046D274E__INCLUDED_)
#define AFX_CONSOLE_H__0E54A3E9_1F5B_11D4_803B_0050046D274E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CConsole  
{
public:
	HANDLE m_hConsole;

public:
	void Init();
	void Clear();

	void SetHandle(HANDLE theHandle) { m_hConsole = theHandle; };
	BOOL SetColor(WORD Color) {	return SetConsoleTextAttribute(m_hConsole,Color);};

public:
	CConsole();
	virtual ~CConsole();

};

#endif // !defined(AFX_CONSOLE_H__0E54A3E9_1F5B_11D4_803B_0050046D274E__INCLUDED_)
