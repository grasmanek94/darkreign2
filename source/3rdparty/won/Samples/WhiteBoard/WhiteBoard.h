// WhiteBoard.h : main header file for the WHITEBOARD application
//

#if !defined(AFX_WHITEBOARD_H__C5231912_0C6C_11D4_803B_0050046D274E__INCLUDED_)
#define AFX_WHITEBOARD_H__C5231912_0C6C_11D4_803B_0050046D274E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CWhiteBoardApp:
// See WhiteBoard.cpp for the implementation of this class
//

class CWhiteBoardApp : public CWinApp
{
public:
	CWhiteBoardApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWhiteBoardApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CWhiteBoardApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WHITEBOARD_H__C5231912_0C6C_11D4_803B_0050046D274E__INCLUDED_)
