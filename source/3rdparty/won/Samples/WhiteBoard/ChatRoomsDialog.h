#if !defined(AFX_CHATROOMSDIALOG_H__1BC3E831_1785_11D4_803B_0050046D274E__INCLUDED_)
#define AFX_CHATROOMSDIALOG_H__1BC3E831_1785_11D4_803B_0050046D274E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChatRoomsDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRoomsDialog dialog


class CChatRoomsDialog : public CDialog
{
// Construction
public:
	ChatServerMap	m_ChatServerMap;
	CChatRoomsDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChatRoomsDialog)
	enum { IDD = IDD_ROOMS };
	CListBox	m_RoomsList;
	CString	m_RoomName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChatRoomsDialog)
	public:
	virtual int DoModal(CString&,ChatServerMap&);
	virtual int DoModal(CString&);

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CChatRoomsDialog)
	afx_msg void OnSelchangeListRooms();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHATROOMSDIALOG_H__C523192B_0C6C_11D4_803B_0050046D274E__INCLUDED_)
