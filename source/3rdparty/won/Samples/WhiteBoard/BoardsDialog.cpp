// BoardsDialog.cpp : implementation file
//

#include "stdafx.h"

#include "resource.h"
#include "BoardsDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBoardsDialog dialog
CBoardsDialog::CBoardsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CBoardsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBoardsDialog)
	m_RoomName = _T("");
	//}}AFX_DATA_INIT
}


void CBoardsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBoardsDialog)
	DDX_Control(pDX, IDC_LIST_ROOMS, m_RoomList);
	DDX_Text(pDX, IDC_EDIT_ROOMNAME, m_RoomName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBoardsDialog, CDialog)
	//{{AFX_MSG_MAP(CBoardsDialog)
	ON_LBN_SELCHANGE(IDC_LIST_ROOMS, OnSelchangeListRooms)
	ON_LBN_DBLCLK(IDC_LIST_ROOMS, OnOK)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBoardsDialog message handlers

int CBoardsDialog::DoModal(CString& selectedRoomName, GameMap& gameMap) 
{
	// Store the game names for later loading
	m_GameMap = gameMap;

	// Run the dialog box
	int result = CDialog::DoModal();

	if(result != IDCANCEL)
	{
		selectedRoomName = m_RoomName;
	}

	return result;
}

BOOL CBoardsDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if (!m_GameMap.empty())
	{
		// Load the chat rooms into the list control
		for (GameMap::const_iterator itr = m_GameMap.begin(); itr != m_GameMap.end(); ++itr)
			m_RoomList.AddString( itr->first.c_str() );
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBoardsDialog::OnSelchangeListRooms() 
{
	// Copy the selected text into the edit control
	if( m_RoomList.GetCurSel() != LB_ERR )
		m_RoomList.GetText( m_RoomList.GetCurSel(), m_RoomName );
	else
		m_RoomName.Empty();

	UpdateData(false);	
}
