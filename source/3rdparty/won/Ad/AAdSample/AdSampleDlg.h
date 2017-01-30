// AdSampleDlg.h : header file
//

#if !defined(AFX_ADSAMPLEDLG_H__A716CC1B_D8F3_11D3_81A9_00104BD0F24B__INCLUDED_)
#define AFX_ADSAMPLEDLG_H__A716CC1B_D8F3_11D3_81A9_00104BD0F24B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AdApi.h"

/////////////////////////////////////////////////////////////////////////////
// CAdSampleDlg dialog

class CAdSampleDlg : public CDialog
{
// Construction
public:
	CAdSampleDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CAdSampleDlg)
	enum { IDD = IDD_ADSAMPLE_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdSampleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CAdSampleDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnPause();
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnTestInstall();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// Add client object
	WONAPI::AdClient	m_AdClient;
	HCURSOR				m_hHandCursor;
	bool				m_bPaused;

	bool				m_AdViewBMP;

	bool				m_TestInstall;
};

void __stdcall Paint_Callback(void *data);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADSAMPLEDLG_H__A716CC1B_D8F3_11D3_81A9_00104BD0F24B__INCLUDED_)
