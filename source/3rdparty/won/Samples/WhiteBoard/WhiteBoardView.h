// WhiteBoardView.h : interface of the CWhiteBoardView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_WHITEBOARDVIEW_H__C523191A_0C6C_11D4_803B_0050046D274E__INCLUDED_)
#define AFX_WHITEBOARDVIEW_H__C523191A_0C6C_11D4_803B_0050046D274E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CWhiteBoardView : public CScrollView
{
protected: // create from serialization only
	CWhiteBoardView();
	DECLARE_DYNCREATE(CWhiteBoardView)

// Attributes
public:
	POINT mCurPoint;
	BOOL bIsDrawing;
	
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWhiteBoardView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual void OnInitialUpdate();
	//}}AFX_VIRTUAL

// Implementation
public:
	void DrawSingleStroke(CStroke* pStroke);
	CWhiteBoardDoc* GetDocument();
	virtual ~CWhiteBoardView();
	
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CWhiteBoardView)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in WhiteBoardView.cpp
inline CWhiteBoardDoc* CWhiteBoardView::GetDocument()
   { return (CWhiteBoardDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WHITEBOARDVIEW_H__C523191A_0C6C_11D4_803B_0050046D274E__INCLUDED_)
