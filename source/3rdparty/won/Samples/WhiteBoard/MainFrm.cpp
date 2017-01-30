// MainFrm.cpp : implementation of the CMainFrame class
//
#include "stdafx.h"

#include "WhiteBoard.h"
#include "MainFrm.h"
#include "WhiteBoardDoc.h"
#include "WhiteBoardView.h"
#include "ChatView.h"
#include "InputView.h"
#include "BlankView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_COMMAND(ID_OPTIONS_BOARDS, OnOptionsBoards)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::OnOptionsBoards() 
{
	// TODO: Add your command handler code here
	
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	CSize szMin(100,100);

	BOOL result = mSplitterWnd.CreateStatic(this,2,2);
	
	
	mSplitterWnd.CreateView(0,0,RUNTIME_CLASS(CChatView), CSize(0,0), pContext);
	mSplitterWnd.CreateView(0,1,RUNTIME_CLASS(CWhiteBoardView), CSize(0,0), pContext);
	mSplitterWnd.CreateView(1,0,RUNTIME_CLASS(CInputView), CSize(0,0), pContext);
	mSplitterWnd.CreateView(1,1,RUNTIME_CLASS(CBlankView), CSize(0,0), pContext);
	
	CRect rectClient;
	GetClientRect(&rectClient);

	mSplitterWnd.SetRowInfo(0, rectClient.bottom-rectClient.top - 35,0);
	mSplitterWnd.SetRowInfo(1, 35,0);
	
	mSplitterWnd.SetColumnInfo(1, (rectClient.right-rectClient.left)/2,0);
	mSplitterWnd.SetColumnInfo(0, (rectClient.right-rectClient.left)/2,0);
	


   return result;
	//return CFrameWnd::OnCreateClient(lpcs, pContext);
}
