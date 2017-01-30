// AdSample.h : main header file for the ADSAMPLE application
//

#if !defined(AFX_ADSAMPLE_H__A716CC19_D8F3_11D3_81A9_00104BD0F24B__INCLUDED_)
#define AFX_ADSAMPLE_H__A716CC19_D8F3_11D3_81A9_00104BD0F24B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CAdSampleApp:
// See AdSample.cpp for the implementation of this class
//

class CAdSampleApp : public CWinApp
{
public:
	CAdSampleApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdSampleApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CAdSampleApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADSAMPLE_H__A716CC19_D8F3_11D3_81A9_00104BD0F24B__INCLUDED_)
