#if !defined(AFX_INPUTVIEW_H__1277C268_0F91_11D4_803B_0050046D274E__INCLUDED_)
#define AFX_INPUTVIEW_H__1277C268_0F91_11D4_803B_0050046D274E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InputView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInputView view

class CInputView : public CEditView
{
protected:
	CInputView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CInputView)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInputView)
	public:
	virtual void OnInitialUpdate();
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CInputView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CInputView)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INPUTVIEW_H__1277C268_0F91_11D4_803B_0050046D274E__INCLUDED_)
