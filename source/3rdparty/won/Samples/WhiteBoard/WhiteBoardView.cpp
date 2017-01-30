// WhiteBoardView.cpp : implementation of the CWhiteBoardView class
//
#pragma warning (disable : 4786)

#include "stdafx.h"

#include "resource.h"
#include "WhiteBoardDoc.h"
#include "WhiteBoardView.h"
#include "Stroke.h"
#include "BoardsDialog.h"

namespace
{
	using namespace WONAPI;
	using namespace WONCommon;
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CWhiteBoardView

IMPLEMENT_DYNCREATE(CWhiteBoardView, CScrollView)

BEGIN_MESSAGE_MAP(CWhiteBoardView, CScrollView)
	//{{AFX_MSG_MAP(CWhiteBoardView)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP() 
	ON_WM_MOUSEMOVE() 
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWhiteBoardView construction/destruction

CWhiteBoardView::CWhiteBoardView()
{
	bIsDrawing = false;
}

CWhiteBoardView::~CWhiteBoardView()
{}


/////////////////////////////////////////////////////////////////////////////
// CWhiteBoardView drawing

void CWhiteBoardView::OnDraw(CDC* pDC)
{
	CWhiteBoardDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Render all strokes, don't worry about clipping for simplicity
	CStroke* pStroke;
	
	// If there aren't any stroke's then return
	if (pDoc->m_StrokeList.IsEmpty())
		return; 

	// Set us up at the start
	pStroke = pDoc->m_StrokeList.GetHead();

	// Tell each stroke to render itself
	for (POSITION pos = pDoc->m_StrokeList.GetHeadPosition(); pos != NULL;)
	{
		pStroke = pDoc->m_StrokeList.GetNext(pos);
		pStroke->DrawStroke(pDC);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWhiteBoardView diagnostics

#ifdef _DEBUG
void CWhiteBoardView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CWhiteBoardView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CWhiteBoardDoc* CWhiteBoardView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWhiteBoardDoc)));
	return (CWhiteBoardDoc*)m_pDocument;
}
#endif //_DEBUG

/********************************************************************************
 * Begin store the mouse movements into our stroke objects						*
 ********************************************************************************/
void CWhiteBoardView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CWhiteBoardDoc* pDoc = GetDocument();		// the Document

	// Note: Do NOT use GetDC when sending coordinates from one 
	// scroll view to another, the stroke will not display correctly
	CClientDC theDC(this);						// the Device Context
	OnPrepareDC(&theDC);						// prepare the Device Context

	bIsDrawing = true;							// We are now drawing
	SetCapture();								// Keep the mouse focus

	// Covert the point to logical units (prevent mangling due to scrolling)
	theDC.DPtoLP(&point);

	// Create a new stroke starting at the current mouse location
	if (pDoc->m_pCurStroke == NULL)
	{
		pDoc->m_pCurStroke = new CStroke(point);
		pDoc->m_pCurStroke->mStrokeColor = pDoc->m_CurStrokeColor;
		pDoc->m_pCurStroke->mStrokeSize = pDoc->m_nCurStrokeSize;
		pDoc->m_StrokeList.AddTail( pDoc->m_pCurStroke );
	}

	// Move the graphics pt to the stroke's first point (mouse position)
	theDC.MoveTo(point);
	mCurPoint = point;

	// Call the base case implementation
	CScrollView::OnLButtonDown(nFlags, point);
}





/********************************************************************************
 * Whenever the mouse moves. We need to track the path of each stroke			*
 ********************************************************************************/
void CWhiteBoardView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// Our Document
	CWhiteBoardDoc* pDoc = GetDocument();

	// If we're not drawing we don't need to process a mouse release
	if (bIsDrawing==false)
	{
		CScrollView::OnLButtonUp(nFlags, point);
		return;
	}

	// Finish the stroke
	

	// If we're connected to a board room let's relay this stroke to any observers
	if (pDoc->bOnBoard)
	{
		// Allocate a linear buffer for data storage (5 = size of message header)
		int buf_size = pDoc->m_pCurStroke->GetSize();
		unsigned char* pDataBuffer = new unsigned char[buf_size];

		// Store the stroke data into the buffer
		pDoc->m_pCurStroke->StoreToBuffer(pDataBuffer);

		// Copy the character buffer into a server readable rawbuffer for net transfer
		RawBuffer theRawBuf;
		theRawBuf.assign(pDataBuffer, buf_size);
		
		// Relay the stroke to all observers
		pDoc->mRoutingServer.SendData(pDoc->mObserverIds, pDoc->mNumObservers, true, theRawBuf, false);

		// delete our linear buffer
		delete[] pDataBuffer;
	}

	// Release the mouse, we don't need it anymore
	ReleaseCapture();

	// No Longer Drawing
	bIsDrawing = false;
	pDoc->m_pCurStroke = NULL;

	// Call the base class implementation
	CScrollView::OnLButtonUp(nFlags, point);
}





/********************************************************************************
 * Whenever the mouse moves. We need to track the path of each stroke			*
 ********************************************************************************/
void CWhiteBoardView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CStroke* pCurStroke = GetDocument()->m_pCurStroke;

	// Are we drawing?
	if (bIsDrawing)
	{
		// Prepare a DC
		CClientDC theDC(this);
		OnPrepareDC(&theDC);

		// Convert the point to Logical coords due to possible mangling (from scrolling)
		theDC.DPtoLP(&point);

		// Move the graphics position to the previous point
		theDC.MoveTo(mCurPoint);

		// Create the stroke's pen for drawing
		CPen thePen(PS_SOLID,pCurStroke->mStrokeSize,pCurStroke->mStrokeColor); 
		CPen* pOldPen = theDC.SelectObject(&thePen);

		// Draw a line to the current mouse position
		theDC.LineTo(point);

		// Restore the old pen
		theDC.SelectObject(pOldPen);

		// Set the new current point
		mCurPoint = point;

		// If the current stroke isn't NULL (which it shouldn't be when drawing)
		// add the new point to the end of it's path
		if (pCurStroke != NULL)
			pCurStroke->m_PointList.AddTail( new CPoint(point) );
	}

	// Call the base class implementation
	CScrollView::OnMouseMove(nFlags, point);
}





/********************************************************************************
 * Post-creation Initalization, Set state for before first drawing.				*
 ********************************************************************************/
void CWhiteBoardView::OnInitialUpdate() 
{
	CScrollView::OnInitialUpdate();

	// Init the scroll bars to a size equal to the full client area
	RECT rect;
	GetClientRect(&rect);

	SIZE sz;
	sz.cx = rect.right - rect.left;
	sz.cy = rect.bottom - rect.top;

	SetScrollSizes(MM_TEXT, sz);

	// Setup the document's reference to this view
	GetDocument()->m_pStrokeView = this;
}





/********************************************************************************
 * Renders a single stroke. Used after receiving a stroke to void a full redraw *
 ********************************************************************************/
void CWhiteBoardView::DrawSingleStroke(CStroke *pStroke)
{
	// Create a device context for the client window 
	// (Do NOT use GetDC() when sending from one ScrollView to another)
	CClientDC theDC(this);
	OnPrepareDC(&theDC);

	// Tell the stroke to draw itself
	pStroke->DrawStroke(&theDC);
}
