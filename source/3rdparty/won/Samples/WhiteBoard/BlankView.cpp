// BlankView.cpp : implementation file
//

#include "stdafx.h"
#include "BlankView.h"
 
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBlankView

IMPLEMENT_DYNCREATE(CBlankView, CView)

CBlankView::CBlankView()
{
}

CBlankView::~CBlankView()
{
}


BEGIN_MESSAGE_MAP(CBlankView, CView)
	//{{AFX_MSG_MAP(CBlankView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBlankView drawing

void CBlankView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CBlankView diagnostics

#ifdef _DEBUG
void CBlankView::AssertValid() const
{
	CView::AssertValid();
}

void CBlankView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBlankView message handlers
