#if !defined(AFX_BLANKVIEW_H__1277C269_0F91_11D4_803B_0050046D274E__INCLUDED_)
#define AFX_BLANKVIEW_H__1277C269_0F91_11D4_803B_0050046D274E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BlankView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBlankView view

class CBlankView : public CView
{
protected:
	CBlankView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CBlankView)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBlankView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CBlankView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CBlankView)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BLANKVIEW_H__1277C269_0F91_11D4_803B_0050046D274E__INCLUDED_)
