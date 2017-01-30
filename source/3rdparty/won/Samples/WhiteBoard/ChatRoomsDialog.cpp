// RoomsDialog.cpp : implementation file
//

#include "stdafx.h"

#include "resource.h"
#include "WhiteBoardDoc.h"
#include "ChatRoomsDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChatRoomsDialog dialog


CChatRoomsDialog::CChatRoomsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CChatRoomsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChatRoomsDialog)
	m_RoomName = _T("");
	//}}AFX_DATA_INIT
}


void CChatRoomsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChatRoomsDialog)
	DDX_Control(pDX, IDC_LIST_ROOMS, m_RoomsList);
	DDX_Text(pDX, IDC_EDIT_ROOMNAME, m_RoomName);
	//}}AFX_DATA_MAP

}


BEGIN_MESSAGE_MAP(CChatRoomsDialog, CDialog)
	//{{AFX_MSG_MAP(CChatRoomsDialog)
	ON_LBN_SELCHANGE(IDC_LIST_ROOMS, OnSelchangeListRooms)
	ON_LBN_DBLCLK(IDC_LIST_ROOMS, OnOK)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChatRoomsDialog message handlers
int CChatRoomsDialog::DoModal(CString& RoomName) 
{
	// This overload is used when the chat server map is empty
	m_ChatServerMap.clear();

	// Run the dialog box
	int result = CDialog::DoModal();

	// If the user clicked OK store the selected room name
	if(result == IDOK)
		RoomName = m_RoomName;

	return result;
}

int CChatRoomsDialog::DoModal(CString& RoomName, ChatServerMap& ChatServerMap) 
{
	// create a copy of the chat server map 
	// slow I know, but is there a better way to initialize a modal dialog's controls?
	m_ChatServerMap = ChatServerMap;

	int result = CDialog::DoModal();
	if(result == IDOK)
	{
		RoomName = m_RoomName;
	}

	return result;
}

void CChatRoomsDialog::OnSelchangeListRooms() 
{
	// Copy the selected text into the edit control
	m_RoomsList.GetText( m_RoomsList.GetCurSel(), m_RoomName );
	UpdateData(false);
}


BOOL CChatRoomsDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if (!m_ChatServerMap.empty())
	{
		// Load the chat rooms into the list control
		for (ChatServerMap::const_iterator itr = m_ChatServerMap.begin(); itr != m_ChatServerMap.end(); ++itr)
			m_RoomsList.AddString( (CString)itr->first.c_str() );
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
