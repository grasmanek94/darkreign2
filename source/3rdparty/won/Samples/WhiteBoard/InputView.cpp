// InputView.cpp : implementation file
//
#include "stdafx.h"

#include "WhiteBoardDoc.h"
#include "InputView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInputView

IMPLEMENT_DYNCREATE(CInputView, CEditView)

CInputView::CInputView()
{
}

CInputView::~CInputView()
{
}


BEGIN_MESSAGE_MAP(CInputView, CEditView)
	//{{AFX_MSG_MAP(CInputView)
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CInputView diagnostics

#ifdef _DEBUG
void CInputView::AssertValid() const
{
	CEditView::AssertValid();
}

void CInputView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CInputView message handlers
void CInputView::OnInitialUpdate() 
{
	CEditView::OnInitialUpdate();
	
	// Disable scroll bars and set a text limit of 255 characters
	EnableScrollBarCtrl( SB_VERT, FALSE );
	EnableScrollBarCtrl( SB_HORZ, FALSE );
	GetEditCtrl().SetLimitText( 255 );

	// Set the focus to this window at startup
	SetFocus();
}

void CInputView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// Process the key in usual fashion before we add our functionality
	CEditView::OnChar(nChar, nRepCnt, nFlags);

	if (nChar == 13) // Enter Key
	{
		CWhiteBoardDoc* pDoc = (CWhiteBoardDoc*)GetDocument();		// the document
		CEdit* pEdit = &(GetEditCtrl());							// the edit control
		
		// Are we logged in?
		if(!pDoc->bIsLoggedIn)
		{
			pDoc->ReceiveText("<System> ***Error: Not logged in***\r\n");
			return;
		}

		// Are we in a chat room?
		if(!pDoc->bIsChatting)
		{
			pDoc->ReceiveText("<System> ***Error: Not in chat room***\r\n");
			return;
		}

		// Broadcast the text to the other users
		// Copy the text to the doc buffer
		CString inBuf;
		pEdit->GetWindowText(inBuf);
		inBuf = "<"+pDoc->mLoginName + "> " + inBuf;
	
		// Send the text to the other users (including self)
		pDoc->SendText(inBuf);

		// Clear Selection
		pEdit->SetWindowText(NULL);
		return;
	}


}
