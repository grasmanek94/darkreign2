// WhiteBoardDoc.h : interface of the CWhiteBoardDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_WHITEBOARDDOC_H__C5231918_0C6C_11D4_803B_0050046D274E__INCLUDED_)
#define AFX_WHITEBOARDDOC_H__C5231918_0C6C_11D4_803B_0050046D274E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/************** EXTERNALLY DEFINED CLASSES & NAMESPACES **************/
class CChatView;
class CStroke;
class CWhiteBoardView;

namespace
{
	using namespace WONAPI;
}

/**************							                **************/

// Maximum number of observers on the board
#define	MAX_OBSERVERS			5

// Map of the downloaded chat servers
struct ChatServerInfo {
	WONAPI::IPSocket::Address mAddress;
	unsigned long             mClientCount;
	bool                      mIsPasswordProtected;
	ChatServerInfo() : mClientCount(0), mIsPasswordProtected(false) {}
};
typedef std::map<std::wstring, ChatServerInfo> ChatServerMap;

class CWhiteBoardDoc : public CDocument
{
protected: // create from serialization only
	CWhiteBoardDoc();
	~CWhiteBoardDoc();
	DECLARE_DYNCREATE(CWhiteBoardDoc)

public:
	/****************************************************************
	 * Titan Attributes												*
	 ****************************************************************/

	// Identity Attributes
	Identity mIdentity;
	CString mLoginName;
	CString mPassword;
	CString mValidVersions;

	// Server Lists -- must be linear
	CArray<WONAPI::IPSocket::Address,WONAPI::IPSocket::Address> mAuthServers;
	CArray<WONAPI::IPSocket::Address,WONAPI::IPSocket::Address> mContestServers;
	CArray<WONAPI::IPSocket::Address,WONAPI::IPSocket::Address> mDirServers;
	CArray<WONAPI::IPSocket::Address,WONAPI::IPSocket::Address> mEventServers;
	CArray<WONAPI::IPSocket::Address,WONAPI::IPSocket::Address> mFactoryServers;
	CArray<WONAPI::IPSocket::Address,WONAPI::IPSocket::Address> mFirewallServers;
	CArray<WONAPI::IPSocket::Address,WONAPI::IPSocket::Address> mProfileServers;

	// Server Counters
	unsigned int mNumChatServers;
	unsigned int mNumProfileServersTried;
	unsigned int mCurProfileServer;

	// Chat members
	ChatServerMap				mChatServers;
	WONAPI::IPSocket::Address	mCurChatServerAddr;		// our current chat server
	WONCommon::WONString		mCurChatServerName;		// current channel name
	WONAPI::RoutingServerClient mRoutingServer;			// our current routing server

	// Maps for Client IDs (people in chat room) and Games (games made in chat room)
	std::map<WONAPI::RoutingServerClient::ClientId, WONMsg::MMsgRoutingGetClientListReply::ClientData> mClientMap;
	std::map<std::string, WONAPI::RoutingServerClient::DataObject> mGameMap;

	// List of brush strokes on the WhiteBoardView
	CTypedPtrList<CPtrList,CStroke*> m_StrokeList;

	// Inbuffer for testing
	CString mInBuf;

	// State Variables
	BOOL bIsLoggedIn;
	BOOL bIsChatting;
	BOOL bIsCaptain;
	
	// Our stroke list
	CStroke* m_pCurStroke;

	// Board info
	BOOL									bOnBoard;
	CChatView*								m_pChatView;
	CWhiteBoardView*						m_pStrokeView;
	int										mNumObservers;
	WONAPI::RoutingServerClient::ClientId	mObserverIds[MAX_OBSERVERS];
	static void OnDataObjectCreated(const RoutingServerClient::DataObjectWithLifespan&, CWhiteBoardDoc*);
	static void OnDataObjectDeleted(const RoutingServerClient::DataObject&,  CWhiteBoardDoc*);
	static void OnRoutingMessageReceived(const RoutingServerClient::MessageWithClientId&, CWhiteBoardDoc*);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWhiteBoardDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	COLORREF m_CurStrokeColor;
	int m_nCurStrokeSize;

	// Initialization
	void InitDocument();
	bool LoadVerifierKey();
	void DownloadServerLists();
	void LoadDirectoryServers();
	Error GetChatServerList();

	// Chat methods
	void ReceiveText( const CString& );
	void ReceiveText( LPSTR );
	void SendText( CString& );

	// Board Methods
	void ProcessMsgStroke(const unsigned char*);
	void ProcessMsgJoinRequest(const RoutingServerClient::MessageWithClientId&);
	void ProcessMsgJoinReply(const RoutingServerClient::MessageWithClientId& theMessageR);
	Error CreateRoom(const CString&);

	// Misc
	void PerformLogin();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CWhiteBoardDoc)
	afx_msg void OnOptionsLogin();
	afx_msg void OnOptionsRooms();
	afx_msg void OnOptionsBoards();
	afx_msg void OnOptionsStrokeColor();
	afx_msg void OnOptionsStrokeSize();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WHITEBOARDDOC_H__C5231918_0C6C_11D4_803B_0050046D274E__INCLUDED_)
