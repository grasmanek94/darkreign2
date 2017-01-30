#if !defined(AFX_BOARDSDIALOG_H__B815FDC5_1128_11D4_803B_0050046D274E__INCLUDED_)
#define AFX_BOARDSDIALOG_H__B815FDC5_1128_11D4_803B_0050046D274E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BoardsDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBoardsDialog dialog
namespace
{
	using namespace WONAPI;
}

typedef std::map<WONAPI::RoutingServerClient::ClientId, WONMsg::MMsgRoutingGetClientListReply::ClientData> ClientMap;
typedef std::map<std::string, WONAPI::RoutingServerClient::DataObject> GameMap;

class CBoardsDialog : public CDialog
{
// Construction
public:
	GameMap m_GameMap;
	CBoardsDialog(CWnd* pParent = NULL);   // standard constructor

// Typedefs for ClientMap and GameMap

// Dialog Data
	//{{AFX_DATA(CBoardsDialog)
	enum { IDD = IDD_BOARDS };
	CListBox	m_RoomList;
	CString	m_RoomName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBoardsDialog)
	public:
	virtual int DoModal(CString&, GameMap&);

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBoardsDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeListRooms();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BOARDSDIALOG_H__B815FDC5_1128_11D4_803B_0050046D274E__INCLUDED_)
