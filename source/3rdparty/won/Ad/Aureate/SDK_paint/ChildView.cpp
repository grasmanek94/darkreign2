// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "test.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void __stdcall dll_callback_paint(void *data)
{
	CChildView *view = reinterpret_cast<CChildView*>(data);
	CRect ad_rect(CPoint(10, 10), CSize(468, 60));
	view->InvalidateRect(&ad_rect, FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CChildView

CChildView::CChildView()
{
	ad_handle = dll.Initialize(0 /*software id for testing*/, 1 /*version*/, 1 /*468x60*/, 0 /*flags*/);
	if (ad_handle) dll.SetPaintCallback(ad_handle, dll_callback_paint, this);
	hand = LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_HAND_CURSOR));
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView,CWnd )
	//{{AFX_MSG_MAP(CChildView)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	HDC mdc;
	HBITMAP bmp;
	
	// paint the ad
	if (ad_handle) {
		/*
		// draw rectangle around the ad
		dc.SelectStockObject(NULL_BRUSH);
		dc.SelectStockObject(BLACK_PEN);
		dc.Rectangle(9, 9, 479, 71);
		*/

		// create a memory device context
		mdc = CreateCompatibleDC(0);

		// keep buffer locked only as long as needed
		dll.LockBuffer(ad_handle);							// lock buffer
		bmp = dll.GetBufferBitmap(ad_handle);				// get bmp
		SelectObject(mdc, bmp);								// select bmp into memory DC
		BitBlt(dc, 10, 10, 478, 70, mdc, 0, 0, SRCCOPY);	// blt the bmp onto the screen
		dll.UnlockBuffer(ad_handle);						// unlock buffer

		// delete the memory device context
		DeleteDC(mdc);
	}
	
	// Do not call CWnd::OnPaint() for painting messages
}


void CChildView::OnDestroy() 
{
	CWnd ::OnDestroy();
	if (ad_handle) dll.Shutdown(ad_handle);
	ad_handle = 0;
}

void CChildView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect ad_rect(CPoint(10, 10), CSize(468, 60));
	if (ad_handle && ad_rect.PtInRect(point)) dll.AdActivated(ad_handle);
	CWnd ::OnLButtonDown(nFlags, point);
}

BOOL CChildView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CRect ad_rect(CPoint(10, 10), CSize(468, 60));
	CPoint point;

	GetCursorPos(&point);
	ScreenToClient(&point);

	if (ad_rect.PtInRect(point)) {
		SetCursor(hand);
		return TRUE;
	} else {
		return CWnd ::OnSetCursor(pWnd, nHitTest, message);
	}
}
