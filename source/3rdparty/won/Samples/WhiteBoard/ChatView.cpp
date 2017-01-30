// ChatView.cpp : implementation file
//
#include "stdafx.h"

#include "WhiteBoardDoc.h"
#include "ChatView.h"

namespace
{
	using namespace WONMsg;			// server status messages
	using namespace WONCommon;
}



// Constants (apply to this object only)
#define		CHATBUFFERSIZE		30000
const RawBuffer	OBJ_GAMEPREFIX	=	(unsigned char*)"GAME";

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChatView

IMPLEMENT_DYNCREATE(CChatView, CView)

CChatView::CChatView()
{
	m_pEditCtrl = new CEdit();
}

CChatView::~CChatView()
{
	// Delete the edit control
	delete m_pEditCtrl;	
}


BEGIN_MESSAGE_MAP(CChatView, CView)
	//{{AFX_MSG_MAP(CChatView)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChatView drawing

void CChatView::OnDraw(CDC* pDC)
{
}

/////////////////////////////////////////////////////////////////////////////
// CChatView diagnostics

#ifdef _DEBUG
void CChatView::AssertValid() const
{
	CView::AssertValid();
}

void CChatView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChatView message handlers

BOOL CChatView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (cs.style & WS_HSCROLL) 
		cs.style -= WS_HSCROLL;
	if (cs.style & WS_VSCROLL) 
		cs.style -= WS_VSCROLL;

	return CView::PreCreateWindow(cs);
}


void CChatView::OnInitialUpdate() 
{

	// Contruct the Edit box for this view
	DWORD dwEditStyle = WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_READONLY;

	// Size the edit box to fill the entire window client area
	CRect theClientRect;
	GetClientRect(&theClientRect);

	// Create the edit control using our parameters
	m_pEditCtrl->Create( dwEditStyle, theClientRect, this, 1);

	// Setup the routing server callbacks
	CWhiteBoardDoc* pDoc = (CWhiteBoardDoc*)GetDocument();
	pDoc->mRoutingServer.InstallASCIIPeerChatCatcherEx((void(*)(const RoutingServerClient::ASCIIChatMessage&, CChatView*))OnChatReceived, this);
	pDoc->mRoutingServer.InstallUnicodePeerChatCatcherEx((void(*)(const RoutingServerClient::UnicodeChatMessage&, CChatView*))OnChatReceived, this);
	
	// Store our pointer in the document
	pDoc->m_pChatView = this;

	CView::OnInitialUpdate();

}

void CChatView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CWhiteBoardDoc* pDoc = (CWhiteBoardDoc*)GetDocument();

	// Route to our text display method
	if(!pDoc->mInBuf.IsEmpty())
		DisplayText(pDoc->mInBuf);
}

void CChatView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	// Resize the edit control to match the client.
	if (m_pEditCtrl->m_hWnd)								// is this a valid window?
		m_pEditCtrl->SetWindowPos(&wndTop, 0,0,cx,cy,SWP_NOMOVE);
}





/********************************************************************************
 * Chat Related Methods	(UNICODE and ASCII)										*
 ********************************************************************************/
void CChatView::OnChatReceived(const RoutingServerClient::UnicodeChatMessage& theMessageR, CChatView* pThat)
{
	pThat->DisplayText(  theMessageR.mData.c_str() );
};

void CChatView::OnChatReceived(const RoutingServerClient::ASCIIChatMessage& theMessageR, CChatView* pThat)
{
	pThat->DisplayText( theMessageR.mData.c_str() );
};

void CChatView::DisplayText(const CString &theText)
{
	// Load New Text into the buffer
	if(m_pEditCtrl->m_hWnd != NULL)
	{
		// Freeze the display of the edit control
		m_pEditCtrl->SetRedraw(FALSE);			

		// Load the old buffer into a temporary CString
		CString temp;
		m_pEditCtrl->GetWindowText(temp);

		// Add the new messages to the buffer
		temp += theText;

		// If the buffer is too large, clip from the top
		if(temp.GetLength() > CHATBUFFERSIZE)
		{
			// Remove partial lines from the top
			int nClippedChars = 0;
			char CHAR_NEWLINE = 10;
			while (temp.GetAt(nClippedChars) != CHAR_NEWLINE)
				nClippedChars++;
			
			// Erase any front floating NEWLINEs 
			while (nClippedChars == CHAR_NEWLINE)
				nClippedChars++;
			
			// Set temp to only the post-clipped portion
			temp = temp.Right(CHATBUFFERSIZE - nClippedChars);
		}
		
		// Load the new buffer into the view 
		m_pEditCtrl->SetWindowText(temp);

		// Update the scroll bars
		int nMin,nLimit;
		m_pEditCtrl->GetScrollRange(SB_VERT,&nMin,&nLimit);
		m_pEditCtrl->SetScrollPos(SB_VERT,nLimit,TRUE);
		m_pEditCtrl->LineScroll(CHATBUFFERSIZE);
		m_pEditCtrl->SetRedraw(TRUE);
	}
	
	
}


void CChatView::CreateDataObjectCallback(short result, CChatView* pThat)
{
	switch(result)
	{
		case StatusCommon_Success:
			pThat->DisplayText("<System> Board Created\r\n");
			((CWhiteBoardDoc*)pThat->GetDocument())->bOnBoard = true;
			break;
		case StatusRouting_ObjectAlreadyExists:
		case StatusRouting_ObjectTooLarge:
		default:
			pThat->DisplayText("<System> ***ERROR Board not created***\r\n");
			break;
	}

}

void CChatView::CreateBoard(const CString& theRoomName)
{
	// Our Document
	CWhiteBoardDoc* pDoc = (CWhiteBoardDoc*)GetDocument();

	// Construct the objects for CreateDataObjectEx
	RawBuffer aObjectName(OBJ_GAMEPREFIX + (unsigned char*)(LPCSTR)theRoomName);
	RawBuffer aObject;

	pDoc->mObserverIds[0] = pDoc->mRoutingServer.GetClientId();
	pDoc->mNumObservers = 1;						/* only 1 since we just created it */
	aObject.assign((unsigned char*)pDoc->mObserverIds);

	pDoc->mRoutingServer.CreateDataObjectEx(RoutingServerClient::GROUPID_ALLUSERS, aObjectName, pDoc->mRoutingServer.GetClientId(), 0, aObject, (void(*)(short, CChatView*))CreateDataObjectCallback, this);
}

void CChatView::ClientListCallback(const RoutingServerClient::GetClientListResult& theResultR, CChatView* pThat)
{
	short aStatus = theResultR.mStatus;
	CWhiteBoardDoc* pDoc = (CWhiteBoardDoc*)pThat->GetDocument();

	switch (aStatus)
	{
		case StatusCommon_Success:
		{
			pThat->DisplayText("<System> Client list downloaded...\r\n");
			for (std::list<WONMsg::MMsgRoutingGetClientListReply::ClientData>::const_iterator itr = theResultR.mClientList.begin(); itr != theResultR.mClientList.end(); itr++)
				pDoc->mClientMap[itr->mClientId] = *itr;

			pDoc->mRoutingServer.SubscribeDataObjectEx(RoutingServerClient::GROUPID_ALLUSERS, OBJ_GAMEPREFIX, false, true, (void(*)(const RoutingServerClient::ReadDataObjectResult& result, CChatView*))SubscribeDataObjectCallback, pThat);
			break;
		}
		case StatusRouting_MustBeClient:
			pThat->DisplayText("<System> ***ERROR Failed to retrieve client list: not logged in***\r\n");
			break;
		default:
			pThat->DisplayText("<System> ***ERROR Failed to retrieve client list***\r\n");
			break;
	}
}





/********************************************************************************
 * Subscribe to the game list in the current chat room							*
 ********************************************************************************/
void CChatView::SubscribeDataObjectCallback(const WONAPI::RoutingServerClient::ReadDataObjectResult& theResultR, CChatView* pThat)
{
	// Check the result from SubscribeDataObject	
	switch (theResultR.mStatus)
	{
		case StatusCommon_Success:
			{
			// List retrieved
			pThat->DisplayText("<System> Subscribed to game list successfully\r\n");

			for (std::list<RoutingServerClient::DataObject>::const_iterator itr = theResultR.mDataObjectList.begin(); itr != theResultR.mDataObjectList.end(); itr++)
			{
				// Store the games in our GameMap
				string gamename = (char*)itr->mDataType.data() + OBJ_GAMEPREFIX.size();
				((CWhiteBoardDoc*)pThat->GetDocument())->mGameMap[gamename] = *itr; 
			}
			break;
			}
		case StatusRouting_SubscriptionAlreadyExists:
			pThat->DisplayText("<System> ***ERROR Attemp to subscribe twice***\r\n");
			break;
		default:
			pThat->DisplayText("<System> ***ERROR Failed to retrieve game list***\r\n");
			break;
	}
};
