#if !defined(AFX_STROKESIZEDIALOG_H__28CE4FCF_15F4_11D4_803B_0050046D274E__INCLUDED_)
#define AFX_STROKESIZEDIALOG_H__28CE4FCF_15F4_11D4_803B_0050046D274E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StrokeSizeDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStrokeSizeDialog dialog

class CStrokeSizeDialog : public CDialog
{
// Construction
public:
	CStrokeSizeDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CStrokeSizeDialog)
	enum { IDD = IDD_STROKESIZEDIALOG };
	int		m_StrokeSize;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStrokeSizeDialog)
	public:
	virtual int DoModal(int);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStrokeSizeDialog)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STROKESIZEDIALOG_H__28CE4FCF_15F4_11D4_803B_0050046D274E__INCLUDED_)
