#if !defined(AFX_CHATVIEW_H__C8B2DC35_0EC8_11D4_803B_0050046D274E__INCLUDED_)
#define AFX_CHATVIEW_H__C8B2DC35_0EC8_11D4_803B_0050046D274E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChatView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChatView view
class CChatView : public CView
{
protected:
	CChatView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CChatView)

// Attributes
public:
	CEdit* m_pEditCtrl;							// the views text box
	HLOCAL m_hEditHandle;						// handle to the text box for easy editing
	int m_nVisibleLines;

// Operations
public:
	void CreateBoard(const CString&);

	void DisplayText(const CString&);
	static void OnChatReceived(const RoutingServerClient::UnicodeChatMessage& theMessageR, CChatView*);
	static void OnChatReceived(const RoutingServerClient::ASCIIChatMessage& theMessageR, CChatView*);
	static void CreateDataObjectCallback(short,CChatView*);
	static void CChatView::ClientListCallback(const RoutingServerClient::GetClientListResult& theResultR, CChatView* pThat);
	static void SubscribeDataObjectCallback(const WONAPI::RoutingServerClient::ReadDataObjectResult& theResultR, CChatView* that);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChatView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CChatView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CChatView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHATVIEW_H__C8B2DC35_0EC8_11D4_803B_0050046D274E__INCLUDED_)
