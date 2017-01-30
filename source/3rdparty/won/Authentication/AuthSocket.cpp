
#pragma warning(disable : 4786)

#include "AuthSocket.h"
#include "msg/Auth/TMsgAuth1PeerToPeer.h"
#include "msg/Auth/TMsgTypesAuth.h"
#include "msg/Comm/TMsgCommStatusReply.h"
#include "msg/Comm/TMsgTypesComm.h"


#define SessionKeySize 8

using namespace WONAPI;
using namespace WONMsg;
using namespace WONCrypt;
using namespace WONCommon;
using namespace WONAuth;
using namespace std;


class WONAPI::AuthSession
{
public:
	CriticalSection				sessionCrit;
	unsigned short				recvSeqNum;
	unsigned short				sendSeqNum;
	unsigned short				id;
	bool						incoming;		// true if locally hosted session

	WONCrypt::BFSymmetricKey	sessionKey;		// Key for encrypt/decrypt (optional)
	WONAuth::Auth1Certificate	remoteCertificate;
	WONAuth::Auth1Certificate	localCertificate;

	bool						sequenced;
	bool						encrypting;
	bool						allowUnencrypted;
	bool						invalidated;

	unsigned long				refCount;
	time_t						lastUsed;
	unsigned short				timeout;

	static unsigned short		sessionIdSeed;

	AuthSession(AuthSocket* authSocket, unsigned short sessId, const WONCrypt::BFSymmetricKey& sessKey,
		const WONAuth::Auth1Certificate& remoteCert, unsigned long lifetime, bool locallyHosted)
	{
		invalidated = false;
		incoming = locallyHosted;
		timeout = lifetime;
		remoteCertificate = remoteCert;
		sessionKey = sessKey;
		recvSeqNum = 1;
		sendSeqNum = 1;
		refCount = 1;
		time(&lastUsed);
		id = sessId;
		sequenced = authSocket->sequenced;
		encrypting = authSocket->encrypting;
		allowUnencrypted = authSocket->allowUnencrypted;
		WONAuth::Auth1Certificate* tmpCert = authSocket->identity->GetCertificate();
		localCertificate = *tmpCert;
		delete tmpCert;
	}
};


unsigned short AuthSession::sessionIdSeed = 0;


typedef map<unsigned short, AuthSession*>									IncomingSessionMap;
typedef multimap< pair<IPSocket::Address, Auth1Certificate>, AuthSession*>	OutgoingSessionMap;


class AuthSocketStatics	// in case we someone does something with an AuthSocket at static destruction time
{
public:
	IncomingSessionMap incomingSessionMap;
	CriticalSection incomingSessionMapCrit;

	OutgoingSessionMap outgoingSessionMap;
	CriticalSection outgoingSessionMapCrit;

	~AuthSocketStatics()
	{
		IncomingSessionMap::iterator itor1 = incomingSessionMap.begin();
		while (itor1 != incomingSessionMap.end())
		{
			AuthSession* authSession = (*itor1).second;
			AutoCrit autoSessionCrit(authSession->sessionCrit);
			bool doDelete = !--authSession->refCount;
			autoSessionCrit.Leave();
			if (doDelete)
				delete authSession;
			itor1++;
		}
		OutgoingSessionMap::iterator itor2 = outgoingSessionMap.begin();
		while (itor2 != outgoingSessionMap.end())
		{
			AuthSession* authSession = (*itor2).second;
			AutoCrit autoSessionCrit(authSession->sessionCrit);
			bool doDelete = !--authSession->refCount;
			autoSessionCrit.Leave();
			if (doDelete)
				delete authSession;
			itor2++;
		}
	}
};




static AuthSocketStatics* authSocketStatics = 0;
static unsigned long allocateStaticsRef = 0;
static CriticalSection* allocateStaticsRefCrit = 0;



static void AllocateStatics()
{
	static CriticalSection tempCrit;
	tempCrit.Enter();
	if (!allocateStaticsRefCrit)
		allocateStaticsRefCrit = new CriticalSection;
	tempCrit.Leave();

	allocateStaticsRefCrit->Enter();
	
	if (!allocateStaticsRef++)
		authSocketStatics = new AuthSocketStatics;

	allocateStaticsRefCrit->Leave();
}


static void DeallocateStatics()
{
	allocateStaticsRefCrit->Enter();

	if (!--allocateStaticsRef)
	{
		delete authSocketStatics;
		allocateStaticsRefCrit->Leave();
		delete allocateStaticsRefCrit;
	}
	else
		allocateStaticsRefCrit->Leave();
}


class AutoAuthSocketStatics
{
public:
	AutoAuthSocketStatics()		{ AllocateStatics(); }
	~AutoAuthSocketStatics()	{ DeallocateStatics(); }
};

static AutoAuthSocketStatics autoAuthSocketStatics;


class SendBaseMsgData
{
public:
	CompletionContainer<const AuthSocket::AuthSendBaseMsgResult&> completion;
	BaseMessage* msg;
};


struct DoneSendEncryptedMessageData
{
	CompletionContainer<const AuthSocket::AuthSendRawMsgResult&> completion;
	unsigned long originalLength;
	unsigned char* originalMsg;
};


class SendRequestData
{
public:
	CompletionContainer<const AuthSocket::AuthRecvBaseMsgResult&> completion;
	BaseMessage* tmpReply;
	BaseMessage** reply;
	Event doneEvent;
	bool autoDel;

	void Done(const AuthSocket::AuthRecvBaseMsgResult& result)
	{
		completion.Complete(result);

		if (autoDel)
			delete this;
		else
			doneEvent.Set();
	}
};


class AuthSocket::AuthRecvMsgData
{
public:
	bool allowUnencrypted;
	CompletionContainer<const AuthSocket::AuthRecvRawMsgResult&> completion;
	unsigned char** recvMsgBuf;
	unsigned char* recvMsg;
	unsigned long* length;
	bool autoDelete;
	long timeout;
	Auth1Certificate newRemoteCertificate;
	AuthenticationMode authMode;

	void Done(AuthSocket::AuthRecvRawMsgResult result)
	{
		recvMsg = result.msg;
		if (recvMsgBuf)
			*recvMsgBuf = result.msg;
		if (length)
			*length = result.msgLength;

		completion.Complete(result);

		if (autoDelete)
			delete this;
	}
};


class AuthSocket::AuthRecvBaseMsgData
{
public:
	CompletionContainer<const AuthSocket::AuthRecvBaseMsgResult&> completion;
	BaseMessage* msg;
	BaseMessage** recvMsg;
	bool autoDelete;
	AuthSocket* thisSocket;
	bool wasEncrypted;
	bool authExpired;
	bool socketClosed;

	void Done()
	{
		completion.Complete(AuthSocket::AuthRecvBaseMsgResult(thisSocket, msg, wasEncrypted, authExpired, socketClosed));

		if (autoDelete)
			delete this;
	}
};


class AuthSocket::AuthenticationData
{
public:
	AuthSocket* thisSocket;
	Identity* identity;
	bool hasRefreshAddr;
	TCPSocket tcpSocket;
	TMsgSocket tMsgSocket;
	Error result;
	CompletionContainer<const AuthSocket::AuthResult&> completion;
	bool autoDelete;
	long timeout;
	AuthenticationMode authMode;
	EncryptionMode encryptMode;
	unsigned short encryptFlags;
	BFSymmetricKey* sessionKey;
	WONAuth::Auth1Certificate* myCertificate;
	EGPrivateKey* privateKey;
	Auth1Certificate remoteCertificate;
	WONCrypt::BFSymmetricKey mySecret;
	Event doneEvent;

	AuthenticationData(AuthSocket* sock, bool useRefreshAddr, const IPSocket::Address& addr, unsigned char lengthSize)
		:	thisSocket(sock),
			myCertificate(NULL),
			hasRefreshAddr(useRefreshAddr),
			tcpSocket(addr),
			privateKey(0),
			tMsgSocket(useRefreshAddr ? (WSSocket*)&tcpSocket : sock->actualSocket, useRefreshAddr ? lengthSize : sock->GetLengthSize())
	{ }

	~AuthenticationData()
	{
		delete myCertificate;
		delete privateKey;
	}

	void Done(Error err)
	{
		result = err;

		bool hasOthersToComplete = !thisSocket->authQueue.empty();	// If so, caller better not delete socket in completion
		thisSocket->authenticating = false;

		completion.Complete(AuthSocket::AuthResult(thisSocket, err));

		if (hasRefreshAddr)
			tcpSocket.Close(0);

		bool doDelete = autoDelete;

		if (hasOthersToComplete)
			thisSocket->FinishQueued(err);

		if (doDelete)
			delete this;
		else
			doneEvent.Set();
	}
};


class AuthSockOpenData
{
public:
	Error error;
	AuthSocket* authSocket;
	CompletionContainer<const Socket::OpenResult&> completion;
	Event doneEvent;
	bool autoDelete;
	long timeout;

	void Done(Error err)
	{
		error = err;

		completion.Complete(Socket::OpenResult(authSocket, error));	

		if (autoDelete)
			delete this;
		else
			doneEvent.Set();
	}
};


void AuthSocket::RequestData::DoCompletion(const TMsgSocket::RecvRawMsgResult& result)
{
	// intentionally empty
}


void AuthSocket::RequestData::Done(const AuthSocket::AuthRecvRawMsgResult& result)
{
	DoneLL(result);

	authCompletion.Complete(result);

	if (autoDel)
		delete this;
	else
		doneEvent.Set();
}


AuthSocket::AuthSocket(TCPSocket& sock, unsigned char lngSize, bool useAuthSession,
					   bool encrypted, bool allowUnencryptedMsgs, bool sequed, bool beAuthHost,
					   unsigned long authSessionLife, bool useClientSessionKey)
	:	TMsgSocket(sock, lngSize),
		session(0),
		identity(0),
		sequenced(sequed),
		encrypting(encrypted),
		allowUnencrypted(allowUnencryptedMsgs || !encrypted),
		authenticated(false),
		authenticating(false),
		hasRefreshAddr(false),
		reuseSession(useAuthSession),
		refreshLengthSize(lngSize),
		authDoneEvent(true, true),
		isAuthenticating(false),
		numAuthsPending(0),
		acceptAuth(beAuthHost),
		useClientKey(useClientSessionKey),
		acceptedSessionExpiration(authSessionLife)
{
	AllocateStatics();
}


AuthSocket::AuthSocket(TCPSocket* sock, unsigned char lngSize, bool useAuthSession,
					   bool encrypted, bool allowUnencryptedMsgs, bool sequed, bool beAuthHost,
					   unsigned long authSessionLife,  bool useClientSessionKey)
	:	TMsgSocket(sock, lngSize),
		session(0),
		identity(0),
		sequenced(sequed),
		encrypting(encrypted),
		allowUnencrypted(allowUnencryptedMsgs || !encrypted),
		authenticated(false),
		authenticating(false),
		hasRefreshAddr(false),
		reuseSession(useAuthSession),
		refreshLengthSize(lngSize),
		authDoneEvent(true, true),
		isAuthenticating(false),
		numAuthsPending(0),
		acceptAuth(beAuthHost),
		useClientKey(useClientSessionKey),
		acceptedSessionExpiration(authSessionLife)
{
	AllocateStatics();
}


AuthSocket::AuthSocket(TCPSocket* sock, bool takeOwnership, unsigned char lngSize, bool useAuthSession,
					   bool encrypted, bool allowUnencryptedMsgs, bool sequed, bool beAuthHost,
					   unsigned long authSessionLife, bool useClientSessionKey)
	:	TMsgSocket(sock, takeOwnership, lngSize),
		session(0),
		identity(0),
		sequenced(sequed),
		encrypting(encrypted),
		allowUnencrypted(allowUnencryptedMsgs || !encrypted),
		authenticated(false),
		authenticating(false),
		hasRefreshAddr(false),
		reuseSession(useAuthSession),
		refreshLengthSize(lngSize),
		authDoneEvent(true, true),
		isAuthenticating(false),
		numAuthsPending(0),
		acceptAuth(beAuthHost),
		useClientKey(useClientSessionKey),
		acceptedSessionExpiration(authSessionLife)
{
	AllocateStatics();
}


AuthSocket::AuthSocket(UDPSocket& sock, const IPSocket::Address& refreshTCPAddr, unsigned char refreshLngSize,
					   bool encrypted, bool allowUnencryptedMsgs, bool sequed, bool beAuthHost,
					   unsigned long authSessionLife, bool useClientSessionKey, size_t largestDatagram)
	:	TMsgSocket(sock, 4, largestDatagram),
		session(0),
		identity(0),
		sequenced(sequed),
		encrypting(encrypted),
		allowUnencrypted(allowUnencryptedMsgs || !encrypted),
		authenticated(false),
		authenticating(false),
		hasRefreshAddr(true),
		refreshAddress(refreshTCPAddr),
		reuseSession(true),
		refreshLengthSize(refreshLngSize),
		authDoneEvent(true, true),
		isAuthenticating(false),
		numAuthsPending(0),
		acceptAuth(beAuthHost),
		useClientKey(useClientSessionKey),
		acceptedSessionExpiration(authSessionLife)
{
	AllocateStatics();
}


AuthSocket::AuthSocket(UDPSocket* sock, const IPSocket::Address& refreshTCPAddr, unsigned char refreshLngSize,
					   bool encrypted, bool allowUnencryptedMsgs, bool sequed, bool beAuthHost,
					   unsigned long authSessionLife, bool useClientSessionKey, size_t largestDatagram)
	:	TMsgSocket(sock, 4, largestDatagram),
		session(0),
		identity(0),
		sequenced(sequed),
		encrypting(encrypted),
		allowUnencrypted(allowUnencryptedMsgs || !encrypted),
		authenticated(false),
		authenticating(false),
		hasRefreshAddr(true),
		refreshAddress(refreshTCPAddr),
		reuseSession(true),
		refreshLengthSize(refreshLngSize),
		authDoneEvent(true, true),
		isAuthenticating(false),
		numAuthsPending(0),
		acceptAuth(beAuthHost),
		useClientKey(useClientSessionKey),
		acceptedSessionExpiration(authSessionLife)
{
	AllocateStatics();
}


AuthSocket::AuthSocket(UDPSocket* sock, bool takeOwnership, const IPSocket::Address& refreshTCPAddr,
					   unsigned char refreshLngSize, bool encrypted, bool allowUnencryptedMsgs,
					   bool sequed, bool beAuthHost, unsigned long authSessionLife,
					   bool useClientSessionKey, size_t largestDatagram)
	:	TMsgSocket(sock, takeOwnership, 4, largestDatagram),
		session(0),
		identity(0),
		sequenced(sequed),
		encrypting(encrypted),
		allowUnencrypted(allowUnencryptedMsgs || !encrypted),
		authenticated(false),
		authenticating(false),
		hasRefreshAddr(true),
		refreshAddress(refreshTCPAddr),
		reuseSession(true),
		refreshLengthSize(refreshLngSize),
		authDoneEvent(true, true),
		isAuthenticating(false),
		numAuthsPending(0),
		acceptAuth(beAuthHost),
		useClientKey(useClientSessionKey),
		acceptedSessionExpiration(authSessionLife)
{
	AllocateStatics();
}


AuthSocket::AuthSocket(Identity* ident, TCPSocket& sock, unsigned char lngSize, bool useAuthSession,
					   bool encrypted, bool allowUnencryptedMsgs, bool sequed, bool beAuthHost,
					   unsigned long authSessionLife, bool useClientSessionKey)
	:	TMsgSocket(sock, lngSize),
		session(0),
		identity(ident/* ? new Identity(*ident) : 0*/),
		sequenced(sequed),
		encrypting(encrypted),
		allowUnencrypted(allowUnencryptedMsgs || !encrypted),
		authenticated(false),
		authenticating(false),
		hasRefreshAddr(false),
		reuseSession(useAuthSession),
		refreshLengthSize(lngSize),
		authDoneEvent(true, true),
		isAuthenticating(false),
		numAuthsPending(0),
		acceptAuth(beAuthHost),
		useClientKey(useClientSessionKey),
		acceptedSessionExpiration(authSessionLife)
{
	AllocateStatics();
}


AuthSocket::AuthSocket(Identity* ident, TCPSocket* sock, unsigned char lngSize, bool useAuthSession,
					   bool encrypted, bool allowUnencryptedMsgs, bool sequed, bool beAuthHost,
					   unsigned long authSessionLife, bool useClientSessionKey)
	:	TMsgSocket(sock, lngSize),
		session(0),
		identity(ident /*? new Identity(*ident) : 0*/),
		sequenced(sequed),
		encrypting(encrypted),
		allowUnencrypted(allowUnencryptedMsgs || !encrypted),
		authenticated(false),
		authenticating(false),
		hasRefreshAddr(false),
		reuseSession(useAuthSession),
		refreshLengthSize(lngSize),
		authDoneEvent(true, true),
		isAuthenticating(false),
		numAuthsPending(0),
		acceptAuth(beAuthHost),
		useClientKey(useClientSessionKey),
		acceptedSessionExpiration(authSessionLife)
{
	AllocateStatics();
}


AuthSocket::AuthSocket(Identity* ident, TCPSocket* sock, bool takeOwnership, unsigned char lngSize,
					   bool useAuthSession, bool encrypted, bool allowUnencryptedMsgs, bool sequed,
					   bool beAuthHost, unsigned long authSessionLife, bool useClientSessionKey)
	:	TMsgSocket(sock, takeOwnership, lngSize),
		session(0),
		identity(ident /*? new Identity(*ident) : 0*/),
		sequenced(sequed),
		encrypting(encrypted),
		allowUnencrypted(allowUnencryptedMsgs || !encrypted),
		authenticated(false),
		authenticating(false),
		hasRefreshAddr(false),
		reuseSession(useAuthSession),
		refreshLengthSize(lngSize),
		authDoneEvent(true, true),
		isAuthenticating(false),
		numAuthsPending(0),
		acceptAuth(beAuthHost),
		useClientKey(useClientSessionKey),
		acceptedSessionExpiration(authSessionLife)
{
	AllocateStatics();
}


AuthSocket::AuthSocket(Identity* ident, UDPSocket& sock, const IPSocket::Address& refreshTCPAddr,
					   unsigned char refreshLngSize, bool encrypted, bool allowUnencryptedMsgs,
					   bool sequed, bool beAuthHost, unsigned long authSessionLife,
					   bool useClientSessionKey, size_t largestDatagram)
	:	TMsgSocket(sock, 4, largestDatagram),
		session(0),
		identity(ident /*? new Identity(*ident) : 0*/),
		sequenced(sequed),
		encrypting(encrypted),
		allowUnencrypted(allowUnencryptedMsgs || !encrypted),
		authenticated(false),
		authenticating(false),
		hasRefreshAddr(true),
		refreshAddress(refreshTCPAddr),
		reuseSession(true),
		refreshLengthSize(refreshLngSize),
		authDoneEvent(true, true),
		isAuthenticating(false),
		numAuthsPending(0),
		acceptAuth(beAuthHost),
		useClientKey(useClientSessionKey),
		acceptedSessionExpiration(authSessionLife)
{
	AllocateStatics();
}


AuthSocket::AuthSocket(Identity* ident, UDPSocket* sock, const IPSocket::Address& refreshTCPAddr,
					   unsigned char refreshLngSize, bool encrypted, bool allowUnencryptedMsgs,
					   bool sequed, bool beAuthHost, unsigned long authSessionLife,
					   bool useClientSessionKey, size_t largestDatagram)
	:	TMsgSocket(sock, 4, largestDatagram),
		session(0),
		identity(ident /*? new Identity(*ident) : 0*/),
		sequenced(sequed),
		encrypting(encrypted),
		allowUnencrypted(allowUnencryptedMsgs || !encrypted),
		authenticated(false),
		authenticating(false),
		hasRefreshAddr(true),
		refreshAddress(refreshTCPAddr),
		reuseSession(true),
		refreshLengthSize(refreshLngSize),
		authDoneEvent(true, true),
		isAuthenticating(false),
		numAuthsPending(0),
		acceptAuth(beAuthHost),
		useClientKey(useClientSessionKey),
		acceptedSessionExpiration(authSessionLife)
{
	AllocateStatics();
}


AuthSocket::AuthSocket(Identity* ident, UDPSocket* sock, bool takeOwnership, const IPSocket::Address& refreshTCPAddr,
					   unsigned char refreshLngSize, bool encrypted, bool allowUnencryptedMsgs, bool sequed,
					   bool beAuthHost, unsigned long authSessionLife, bool useClientSessionKey,
					   size_t largestDatagram)
	:	TMsgSocket(sock, takeOwnership, 4, largestDatagram),
		session(0),
		identity(ident /*? new Identity(*ident) : 0*/),
		sequenced(sequed),
		encrypting(encrypted),
		allowUnencrypted(allowUnencryptedMsgs || !encrypted),
		authenticated(false),
		authenticating(false),
		hasRefreshAddr(true),
		refreshAddress(refreshTCPAddr),
		reuseSession(true),
		refreshLengthSize(refreshLngSize),
		authDoneEvent(true, true),
		isAuthenticating(false),
		numAuthsPending(0),
		acceptAuth(beAuthHost),
		useClientKey(useClientSessionKey),
		acceptedSessionExpiration(authSessionLife)
{
	AllocateStatics();
}


AuthSocket::AuthSocket(TCPSocket& sock, const AuthSocket& toCopy)
	:	TMsgSocket(&sock, toCopy.lengthSize),
		session(toCopy.session),
		identity(0),
		sequenced(false),
		encrypting(toCopy.encrypting),
		allowUnencrypted(toCopy.allowUnencrypted),
		authenticated(true),
		authenticating(false),
		hasRefreshAddr(false),
		refreshAddress(toCopy.refreshAddress),
		reuseSession(true),
		authDoneEvent(true, true),
		isAuthenticating(false),
		numAuthsPending(0),
		sessionKey(toCopy.sessionKey),
		acceptAuth(toCopy.acceptAuth),
		useClientKey(toCopy.useClientKey),
		acceptedSessionExpiration(toCopy.acceptedSessionExpiration)
{
	SetIdentity(toCopy.identity);
	AllocateStatics();
	if (session)
	{
		AutoCrit autoSessionCrit(session->sessionCrit);
		session->refCount++;
	}
}


AuthSocket::AuthSocket(TCPSocket* sock, const AuthSocket& toCopy)
	:	TMsgSocket(sock, toCopy.lengthSize),
		session(toCopy.session),
		identity(0),
		sequenced(false),
		encrypting(toCopy.encrypting),
		allowUnencrypted(toCopy.allowUnencrypted),
		authenticated(true),
		authenticating(false),
		hasRefreshAddr(false),
		refreshAddress(toCopy.refreshAddress),
		reuseSession(true),
		authDoneEvent(true, true),
		isAuthenticating(false),
		numAuthsPending(0),
		sessionKey(toCopy.sessionKey),
		acceptAuth(toCopy.acceptAuth),
		useClientKey(toCopy.useClientKey),
		acceptedSessionExpiration(toCopy.acceptedSessionExpiration)
{
	SetIdentity(toCopy.identity);
	AllocateStatics();
	if (session)
	{
		AutoCrit autoSessionCrit(session->sessionCrit);
		session->refCount++;
	}
}


AuthSocket::AuthSocket(TCPSocket* sock, bool takeOwnership, const AuthSocket& toCopy)
	:	TMsgSocket(sock, takeOwnership, toCopy.lengthSize),
		session(toCopy.session),
		identity(0),
		sequenced(false),
		encrypting(toCopy.encrypting),
		allowUnencrypted(toCopy.allowUnencrypted),
		authenticated(true),
		authenticating(false),
		hasRefreshAddr(false),
		refreshAddress(toCopy.refreshAddress),
		reuseSession(true),
		authDoneEvent(true, true),
		isAuthenticating(false),
		numAuthsPending(0),
		sessionKey(toCopy.sessionKey),
		acceptAuth(toCopy.acceptAuth),
		useClientKey(toCopy.useClientKey),
		acceptedSessionExpiration(toCopy.acceptedSessionExpiration)
{
	SetIdentity(toCopy.identity);
	AllocateStatics();
	if (session)
	{
		AutoCrit autoSessionCrit(session->sessionCrit);
		session->refCount++;
	}
}


void AuthSocket::ReleaseSession()
{
	if (session)
	{
		AutoCrit autoCrit(session->sessionCrit);
		bool doDelete = !--(session->refCount);
		// Incoming sessions should not hit zero unless already removed from session map for some reason

		autoCrit.Leave();
		if (doDelete)
		{
			if (session->incoming)
				delete session;
			else if (reuseSession && !session->invalidated)
			{
				AutoCrit autoMapCrit(authSocketStatics->outgoingSessionMapCrit);
				session->refCount = 1;	// return to 1 
				authSocketStatics->outgoingSessionMap.insert(OutgoingSessionMap::value_type( pair<IPSocket::Address, Auth1Certificate>(refreshAddress, session->localCertificate), session));
			}
			else
				delete session;
		}
		session = 0;
	}
	authenticated = false;
}


void AuthSocket::InvalidateSession()
{
	if (session)
		session->invalidated = true;
	ReleaseSession();
}


AuthSocket::~AuthSocket()
{
	if (ownsSocket)
		delete actualSocket;
	//delete identity;
	ownsSocket = false;
	ReleaseSession();
	DeallocateStatics();
}


void AuthSocket::SetRefreshAddress(const IPSocket::Address& refreshTCPAddr)
{
	if (actualSocket && actualSocket->GetType() == datagram)
	{
		hasRefreshAddr = true;
		refreshAddress = refreshTCPAddr;
	}
}

static void AuthSocketDoneOpenAuthenticate(const AuthSocket::AuthResult& result, AuthSockOpenData* openData)
{
	if (result.error != Error_Success)
		result.theSocket->Close(0);
	openData->Done(result.error);
}


static void AuthSocketDoneOpen(const Socket::OpenResult& result, AuthSockOpenData* openData)
{
	if (result.error != Error_Success)
		openData->Done(result.error);
	else
		openData->authSocket->AuthenticateEx(false, openData->timeout, true, AuthSocketDoneOpenAuthenticate, openData);
}


Error AuthSocket::Open(bool authenticate, long timeout, bool async, const CompletionContainer<const OpenResult&>& completion)
{
//	if (IsOpen())
//	{
//		completion.Complete(OpenResult(this, Error_InvalidState));
//		return Error_InvalidState;
//	}

	if (!authenticate)
		return TMsgSocket::Open(0, timeout, async, completion);

	if (!reuseSession)
		authenticated = false;

	AuthSockOpenData* openData = new AuthSockOpenData;
	if (!openData)
	{
		completion.Complete(OpenResult(this, Error_OutOfMemory));
		return Error_OutOfMemory;
	}

	openData->authSocket = this;
	openData->completion = completion;
	openData->autoDelete = async;
	openData->timeout = timeout;

	TMsgSocket::Open(0, timeout, true, new CompletionWithContents<const OpenResult&, AuthSockOpenData*>(openData, AuthSocketDoneOpen, true));

	Error err = Error_Pending;

	if (!async)
	{
		WSSocket::PumpUntil(openData->doneEvent, timeout);
		//openData->doneEvent.WaitFor();
		err = openData->error;
		delete openData;
	}

	return err;
}


Error AuthSocket::Open(long timeout, bool async, const CompletionContainer<const OpenResult&>& completion)
{
	return Open(true, timeout, async, completion);
}


void AuthSocket::FinishQueued(Error err)
{
	AutoCrit autoAuthCrit(authCrit);

	while (authQueue.size())
	{
		CompletionContainer<const AuthResult&> queuedCompletion = authQueue.front();
		authQueue.pop();
		queuedCompletion.Complete(AuthResult(this, err));
	}
	authenticating = false;
	authDoneError = err;
	authDoneEvent.Set();
}


void AuthSocket::DoneRecvAuth1Complete(const TMsgSocket::RecvBaseMsgResult& result, AuthSocket::AuthenticationData* authData)
{
	if (!result.msg)
	{
		authData->Done(result.closed ? Error_ConnectionClosed : Error_Timeout);
		return;
	}

	Error err = Error_InvalidMessage;

	if (result.msg->GetServiceType() == Auth1PeerToPeer && result.msg->GetMessageType() == Auth1Complete)
	{
		try
		{
			TMsgAuth1Complete msg(*(TMessage*)(result.msg));

			short status = msg.GetStatus();
			if (status != StatusCommon_Success)
				err = status;
			else
			{
				err = Error_InvalidPrivateKey;

				if (authData->privateKey)
				{
					err = Error_InvalidSecret;

					CryptKeyBase::CryptReturn aCryptRet = authData->privateKey->Decrypt(msg.GetRawBuf(),msg.GetRawBufLen());
					
					if (aCryptRet.first)
					{
						auto_ptr<unsigned char> aDeleteCryptRet(aCryptRet.first);

						unsigned short aLen = getLittleEndian(*((unsigned short*)aCryptRet.first));
						if ((aLen == aCryptRet.second-2)
							&& (aLen == authData->mySecret.GetKeyLen())
							&&  (memcmp(authData->mySecret.GetKey(), aCryptRet.first+2,aLen)==0))
						{
							authData->thisSocket->session = new AuthSession(authData->thisSocket, msg.GetSessionId(),
													authData->thisSocket->sessionKey, authData->remoteCertificate,
													authData->thisSocket->acceptedSessionExpiration, false);
							if (!authData->thisSocket->session)
								err = Error_OutOfMemory;
							else
							{
								err = Error_Success;
								authData->thisSocket->authenticated = true;
							}
						}
					}
				}
			}
		}
		catch (...)
		{
		}
	}
//	WSSocket::CheckCompleted();

	authData->Done(err);
	delete result.msg;
}


void AuthSocket::DoneRecvChallenge1(const TMsgSocket::RecvBaseMsgResult& result, AuthenticationData* authData)
{
	if (!result.msg)
	{
		authData->Done(result.closed ? Error_ConnectionClosed : Error_Timeout);
		return;
	}

	if (result.msg->GetServiceType() == CommonService && result.msg->GetMessageType() == CommStatusReply)
	{
		try
		{
			TMsgCommStatusReply msg(*(TMessage*)(result.msg));

			short status = msg.GetStatus();
			if (status == StatusCommon_InvalidSessionId)
			{
				// Must be a left over error from auth expiration
				delete result.msg;
				authData->tMsgSocket.RecvBaseMsgEx(NULL, authData->timeout, true, DoneRecvChallenge1, authData);
				return;
			}
		}
		catch (...)
		{
		}
	}

	Error err = Error_InvalidMessage;

	if (result.msg->GetServiceType() == Auth1PeerToPeer)
	{
		if (result.msg->GetMessageType() == Auth1Complete)
		{
			try
			{
				TMsgAuth1Complete msg(*(TMessage*)(result.msg));

				err = (short)msg.GetStatus();
			}
			catch (...)
			{
			}
		}
		else if (result.msg->GetMessageType() == Auth1Challenge1)
		{
			try
			{
				TMsgAuth1Challenge1 msg(*(TMessage*)(result.msg));

				err = Error_InvalidPrivateKey;

				AutoCrit autoAuthCrit(authData->thisSocket->authCrit);

				if (authData->privateKey)
				{
					err = Error_InvalidSessionKey;

					CryptKeyBase::CryptReturn cryptReturn = authData->privateKey->Decrypt(msg.GetSecretB(),msg.GetSecretBLen());
					
					if (cryptReturn.first)
					{
						auto_ptr<unsigned char> deleteTheCryptReturn(cryptReturn.first);
						
						if(cryptReturn.second - 2 == getLittleEndian(*((unsigned short*)cryptReturn.first)))
						{
							authData->sessionKey->Create(getLittleEndian(*((unsigned short*)cryptReturn.first)), cryptReturn.first + 2);

							err = Error_InvalidRemoteCertificate;

							authData->remoteCertificate = Auth1Certificate(msg.GetRawBuf(), msg.GetRawBufLen());

							err = Error_InvalidPubKeys;
							Auth1PublicKeyBlock* pubKeys = authData->identity->GetPublicKeyBlock();
							if (pubKeys)
							{
								auto_ptr<Auth1PublicKeyBlock> deletePubKeys(pubKeys);

								err = Error_InvalidRemoteCertificate;
								if (pubKeys->VerifyFamilyBuffer(authData->remoteCertificate))
								{
									err = Error_OutOfMemory;
									unsigned long bufSize = sizeof(unsigned short) + authData->sessionKey->GetKeyLen() + authData->mySecret.GetKeyLen();
									unsigned char* buf = new unsigned char[bufSize];
									if (buf)
									{
										*(unsigned short*)buf = getLittleEndian(authData->sessionKey->GetKeyLen());
										memcpy(buf + sizeof(unsigned short), authData->sessionKey->GetKey(), authData->sessionKey->GetKeyLen());
										memcpy(buf + sizeof(unsigned short) + authData->sessionKey->GetKeyLen(),
											authData->mySecret.GetKey(), authData->mySecret.GetKeyLen());
										err = Error_EncryptFailed;

										CryptKeyBase::CryptReturn aSecretCrypt =
											authData->remoteCertificate.GetPubKey().Encrypt((unsigned char *)buf,
											bufSize);

										delete buf;
										
										if (aSecretCrypt.first)
										{
											auto_ptr<unsigned char> deleteSecretCrypt(aSecretCrypt.first);

											TMsgAuth1Challenge2 reply;
											reply.SetRawBuf(aSecretCrypt.first, aSecretCrypt.second, true);
											
											authData->tMsgSocket.SendBaseMsg(reply, -1, true, true);
											authData->tMsgSocket.RecvBaseMsgEx(NULL, authData->timeout, true, DoneRecvAuth1Complete, authData);
											delete result.msg;
											return;
										}
									}
								}
							}
						}
					}
				}
			}
			catch (...)
			{
			}
		}
	}
	authData->Done(err);
	delete result.msg;
}


void AuthSocket::DoneOpen(const Socket::OpenResult& result, AuthSocket::AuthenticationData* authData)
{
	if (result.error != Error_Success || authData->thisSocket->authenticated)
	{
		authData->Done(result.error);
		return;
	}

	TMsgAuth1Request msg;

	msg.SetAuthMode(authData->authMode);
	msg.SetEncryptMode(authData->encryptMode);
	msg.SetEncryptFlags(authData->encryptFlags);
	msg.SetRawBuf(authData->myCertificate->GetRaw(), authData->myCertificate->GetRawLen());

	authData->tMsgSocket.SendBaseMsg(msg, -1, true, true);
	authData->tMsgSocket.RecvBaseMsgEx(NULL, authData->timeout, true, DoneRecvChallenge1, authData);
}


void AuthSocket::DoneRefresh(const Identity::AuthResult& result, AuthSocket::AuthenticationData* authData)
{
	if (result.error != Error_Success)
	{
		authData->Done(result.error);
		return;
	}

	authData->privateKey = authData->identity->GetPrivateKey();
	authData->myCertificate = authData->identity->GetCertificate();

	AutoCrit autoCrit(authSocketStatics->outgoingSessionMapCrit);
	OutgoingSessionMap::iterator itor = authSocketStatics->outgoingSessionMap.find(pair<IPSocket::Address, Auth1Certificate>(*(IPSocket::Address*)&(authData->thisSocket->actualSocket->GetRemoteAddress()), *(authData->myCertificate)));
	if (itor != authSocketStatics->outgoingSessionMap.end())
	{
		authData->thisSocket->authenticated = true;
		authData->thisSocket->session = (*itor).second;
		authData->thisSocket->sequenced = authData->thisSocket->session->sequenced;
		authData->thisSocket->encrypting = authData->thisSocket->session->encrypting;
		authData->thisSocket->allowUnencrypted = authData->thisSocket->session->allowUnencrypted;
		authData->thisSocket->sessionKey = authData->thisSocket->session->sessionKey;
		authSocketStatics->outgoingSessionMap.erase(itor);
	}

	// use existing socket... 
	if (!authData->tMsgSocket.IsOpen())
		((TCPSocket*)(authData->tMsgSocket.GetSocket()))->OpenEx(0, authData->timeout, true, DoneOpen, authData);
	else
		DoneOpen(Socket::OpenResult(((TCPSocket*)(authData->tMsgSocket.GetSocket())), Error_Success), authData);
}


Error AuthSocket::Authenticate(bool force, long timeout, bool async, const CompletionContainer<const AuthResult&>& completion)
{
	if (!actualSocket)
	{
		completion.Complete(AuthResult(this, Error_InvalidParams));
		return Error_InvalidParams;
	}

	if (!identity)	// Return success, since unauthenticated w/ no identity is valid
	{
		completion.Complete(AuthResult(this, Error_Success));
		return Error_Success;
	}

	if (!hasRefreshAddr)
		refreshAddress = ((TCPSocket*)actualSocket)->GetRemoteAddress();

	AutoCrit autoAuthCrit(authCrit);

	if (authenticated && !force)
	{
		autoAuthCrit.Leave();
		completion.Complete(AuthResult(this, Error_Success));
		return Error_Success;
	}

	if (authenticating)
	{		
		// Already refreshing...  Add this req to a queue, and wait if synch
		authQueue.push(completion);
		autoAuthCrit.Leave();		

		if (async)
			return Error_Pending;

		WSSocket::PumpUntil(authDoneEvent, timeout);
		//authDoneEvent.WaitFor();
		return authDoneError;
	}
	else
	{
		authenticating = true;
		authDoneEvent.Reset();
	}
	autoAuthCrit.Leave();

	AuthenticationData* authData = new AuthenticationData(this, hasRefreshAddr, refreshAddress, refreshLengthSize);
	if (!authData)
	{
		completion.Complete(AuthResult(this, Error_OutOfMemory));
		FinishQueued(Error_OutOfMemory);
		return Error_OutOfMemory;
	}

	try {
		authData->mySecret.Create(SessionKeySize);
	}
	catch (...)
	{
		delete authData;
		completion.Complete(AuthResult(this, Error_FailedToGenerateKey));
		FinishQueued(Error_FailedToGenerateKey);
		return Error_FailedToGenerateKey;
	}

	authData->identity = identity;
	authData->completion = completion;
	authData->autoDelete = async;
	authData->timeout = timeout;
	authData->sessionKey = &sessionKey;
	authData->authMode = reuseSession ? AUTH_SESSION : AUTH_PERSISTENT;
	authData->encryptMode = encrypting ? ENCRYPT_BLOWFISH : ENCRYPT_NONE;
	authData->encryptFlags = EFLAGS_NONE;
	if (allowUnencrypted)
		authData->encryptFlags |= EFLAGS_ALLOWCLEAR;
	if (!sequenced)
		authData->encryptFlags |= EFLAGS_NOTSEQUENCED;
	if (useClientKey)
		authData->encryptFlags |= EFLAGS_CLIENTKEY;

	void (*DoneRefreshFunc)(const Identity::AuthResult&, AuthSocket::AuthenticationData*) = DoneRefresh;
	
	WONAuth::Auth1Certificate* tmpCert = identity->GetCertificate();

	if (identity->IsExpired() || (session && (!tmpCert || *tmpCert != session->localCertificate)))
	{
		ReleaseSession();
		// Just release it case someone else still has a reference to it
		
		identity->RefreshEx(true, -1, async, DoneRefreshFunc, authData);
	}
	else
	{
		authData->myCertificate = authData->identity->GetCertificate();
		authData->privateKey = authData->identity->GetPrivateKey();

		AutoCrit autoCrit(authSocketStatics->outgoingSessionMapCrit);
		OutgoingSessionMap::iterator itor = authSocketStatics->outgoingSessionMap.find(pair<IPSocket::Address, Auth1Certificate>(*(IPSocket::Address*)&(actualSocket->GetRemoteAddress()), *(authData->myCertificate)));
		if (itor != authSocketStatics->outgoingSessionMap.end())
		{
			authenticated = true;
			session = (*itor).second;
			sequenced = session->sequenced;
			encrypting = session->encrypting;
			allowUnencrypted = session->allowUnencrypted;
			sessionKey = session->sessionKey;
			authSocketStatics->outgoingSessionMap.erase(itor);
		}

		TCPSocket* tcpSocket = (TCPSocket*)(authData->tMsgSocket.GetSocket());
		if (!authData->tMsgSocket.IsOpen())
		{
			void (*DoneOpenFunc)(const Socket::OpenResult&, AuthSocket::AuthenticationData*) = DoneOpen;
			tcpSocket->OpenEx(0, authData->timeout, true, DoneOpenFunc, authData);
		}
		else
			DoneOpen(Socket::OpenResult(tcpSocket, Error_Success), authData);

	}
	delete tmpCert;

	if (async)
		return Error_Pending;

	WSSocket::PumpUntil(authData->doneEvent, timeout);
	//authData->doneEvent.WaitFor();

	Error result = authData->result;

	delete authData;

	return result;
}


static void RecvMsgCompletionTranslator(const AuthSocket::AuthRecvRawMsgResult& result, CompletionContainer<const TMsgSocket::RecvRawMsgResult&> completion)
{
	completion.Complete(result);
}


unsigned char* AuthSocket::RecvRawMsg(unsigned char** recvMsgBuf, unsigned long* length, long timeout,
								   bool async, const CompletionContainer<const TMsgSocket::RecvRawMsgResult&>& completion)
{
	return RecvRawMsgEx(recvMsgBuf, length, allowUnencrypted, timeout, async, RecvMsgCompletionTranslator, completion);
}


unsigned char* AuthSocket::DecryptMessage(bool allowUnencryptedThisTime, unsigned char* msg, unsigned long length, unsigned long& newLength, bool& wasEncrypted)
{
	wasEncrypted = false;
	unsigned char* result = 0;
	if (length)
	{
		newLength = 0;

		unsigned char headerType = *(unsigned char*)msg;
		switch (headerType)
		{
		case 2:	// Encrypted TMessage
		case 4:	// Encrypted MiniMessage
		case 6:	// Encrypted SmallMessage
		case 8:	// Encrypted LargeMessage
		case 12:// Encrypted Header, no particular message type
			break;
		default:	// old TMessage
			if (authenticated && !allowUnencryptedThisTime)
			{
				delete msg;
				return 0;
			}
			else
				newLength = length;
			return msg;
		}

		wasEncrypted = true;

		unsigned char* aDecryptPtr = (unsigned char*)msg + 1;
		unsigned long  aDecryptLen = length - 1;
		
		if (aDecryptLen < 2)
		{
			delete msg;
			return 0;
		}

		AuthSession* thisSession = session;

		if (reuseSession)
		{
			unsigned short thisSessionId = getLittleEndian(*(unsigned short*)aDecryptPtr);
			aDecryptLen -= sizeof(unsigned short);
			aDecryptPtr += sizeof(unsigned short);

			if (authenticated)
			{
				if (thisSessionId != session->id)
				{
					delete msg;
					return 0;
				}
			}
			else if (acceptAuth) // && !authenticated // allow any sessions
			{
				AutoCrit autoSessionMapCrit(authSocketStatics->incomingSessionMapCrit);

				IncomingSessionMap::iterator itor = authSocketStatics->incomingSessionMap.find(thisSessionId);
				if (itor == authSocketStatics->incomingSessionMap.end())
				{
					delete msg;
					return 0;
				}
				thisSession = (*itor).second;
				time(&(thisSession->lastUsed));
			}
		}

		AutoCrit autoSessionCrit(thisSession->sessionCrit);

		if (!actualSocket->isConnectionless())
		{

			if (session)
			{
				if (session != thisSession)
				{
					delete msg;	// Sorry, dude.  Can't switch sessions mid-TCP connection
					return 0;
				}
			}
			else
			{
				session = thisSession;
				session->refCount++;
				sequenced = session->sequenced;
				encrypting = session->encrypting;
				allowUnencrypted = session->allowUnencrypted;
				sessionKey = session->sessionKey;
			}
		}

		if (encrypting && thisSession->sessionKey.GetKeyLen() > 0)
		{

			BFSymmetricKey::CryptReturn aCryptReturn;

			if (!aDecryptLen)
			{
				aCryptReturn.first = new unsigned char[2];
				aCryptReturn.second = 0;
			}
			else
				aCryptReturn = thisSession->sessionKey.Decrypt(aDecryptPtr, aDecryptLen);

			if (aCryptReturn.first)
			{
				newLength = aCryptReturn.second + 1;
				result = new unsigned char[newLength];
				if (!result)
				{
					delete aCryptReturn.first;
					delete msg;
					return 0;
				}

				unsigned char* copyFrom = aCryptReturn.first;
				bool inSeq = true;
				if (sequenced)
				{
					if (aCryptReturn.second < 2)
					{
						delete aCryptReturn.first;
						delete msg;
						return 0;
					}

					unsigned short thisSeqNum = getLittleEndian(*(unsigned short*)aCryptReturn.first);
					copyFrom += sizeof(unsigned short);

					if (GetType() == datagram)
						inSeq = (thisSeqNum >= thisSession->recvSeqNum);
					else
						inSeq = (thisSeqNum == thisSession->recvSeqNum);
					if (inSeq)
					{
						thisSession->recvSeqNum = thisSeqNum + 1;
						newLength -= sizeof(unsigned short);
					}
				}
				if (inSeq)
				{
					unsigned char* copyDest = result;
					int copyCount = newLength;
					switch (headerType)
					{
					case 4:								// Encrypted MiniMessage
					case 6:								// Encrypted SmallMessage
					case 8:								// Encrypted LargeMessage
						*copyDest = headerType-1;
						copyDest++;
						copyCount--;
						break;
					//case 2:							// Encrypted TMessage
					//case 12:							// Encrypted Header, no particular message type
					default:
						newLength--;
						// decrypt will include the header type
						break;
					};
					memcpy(copyDest, copyFrom, copyCount);
				}
				else
				{
					InvalidateSession();	// Bad sequence ID, invalidate session!
					delete result;
					result = 0;
				}
				delete aCryptReturn.first;
			}
		}
	}
	delete msg;

	return result;
}


void AuthSocket::DoneRecvMsg(const TMsgSocket::RecvRawMsgResult& result, AuthRecvMsgData* recvMsgData)
{
	AuthSocket* authSocket = (AuthSocket*)(result.theSocket);

	if (!result.msgLength)
		recvMsgData->Done(AuthSocket::AuthRecvRawMsgResult(result.theSocket, 0, 0, false, false, result.closed));
	else
	{
		AutoCrit autoAuthCrit(authSocket->authCrit);
		auto_ptr<unsigned char> autoResultMsg(result.msg);

		if (authSocket->authenticated)
		{
			TitanHeader* titanHeader = (TitanHeader*)result.msg;
			if (titanHeader->_ServiceType == CommonService && titanHeader->_MessageType == CommStatusReply)
			{
				short status = getLittleEndian(*(short*)(titanHeader+1));
				if (status == StatusCommon_SessionNotFound)	// -7
				{
					// Authentication expired!
					authSocket->authenticated = false;
					autoAuthCrit.Leave();
					recvMsgData->Done(AuthSocket::AuthRecvRawMsgResult(result.theSocket, 0, 0, false, true, result.closed));
					return;
				}
			}
		}
		else if (authSocket->acceptAuth && authSocket->GetType() != datagram)
		{
			TitanHeader* titanHeader = (TitanHeader*)result.msg;
			if (titanHeader->_ServiceType == Auth1PeerToPeer)
			{
				TMsgAuth1Complete completeMsg;
				completeMsg.SetStatus(StatusCommon_InvalidParameters);
				bool sendComplete = true;

				if (!authSocket->identity)
					completeMsg.SetStatus(StatusCommon_NotSupported);
				else
				{
					if (titanHeader->_MessageType == Auth1Request)
					{
						TMessage tmsg(result.msgLength, result.msg);
						TMsgAuth1Request msg(tmsg);
				
						recvMsgData->newRemoteCertificate = Auth1Certificate(msg.GetRawBuf(), msg.GetRawBufLen());
						recvMsgData->authMode = msg.GetAuthMode();

						switch (recvMsgData->authMode)
						{
						case AUTH_SESSION:		// Session based (UDP, Lightweight TCP) mode
							if (!authSocket->reuseSession)	// disallowed by AuthSocket args
								break;
						case AUTH_PERSISTENT:	// Persistent (TCP) mode
							{
								authSocket->encrypting = true;
								switch (msg.GetEncryptMode())
								{
								case ENCRYPT_NONE:		// No encryption (Encrypt flags ignored)
									if (!authSocket->allowUnencrypted)
										break;
									authSocket->encrypting = false;
								case ENCRYPT_BLOWFISH:	// Use Blowfish encryption
									{
										unsigned short flags = msg.GetEncryptFlags();
										if (authSocket->allowUnencrypted || !(flags & EFLAGS_ALLOWCLEAR))
										{
											// EFLAGS_CLIENTKEY ???
											authSocket->sequenced = !(flags & EFLAGS_NOTSEQUENCED);
											
											completeMsg.SetStatus(StatusAuth_VerifyFailed);
											Auth1PublicKeyBlock* pubKeys = authSocket->identity->GetPublicKeyBlock();
											if (pubKeys)
											{
												auto_ptr<Auth1PublicKeyBlock> autoPubKeys(pubKeys);

												completeMsg.SetStatus(StatusCommon_InvalidParameters);
												if (pubKeys->VerifyFamilyBuffer(recvMsgData->newRemoteCertificate) &&
													!recvMsgData->newRemoteCertificate.IsExpired(authSocket->identity->GetExpirationDelta()))
												{
													authSocket->sessionKey.Create(SessionKeySize);
													RawBuffer secret;
													unsigned short keyLen = authSocket->sessionKey.GetKeyLen();
													unsigned short tmpKeyLen = getLittleEndian(keyLen);
													secret.append((unsigned char*)&tmpKeyLen, sizeof(unsigned short));
													secret.append(authSocket->sessionKey.GetKey(), keyLen);

													completeMsg.SetStatus(StatusCommon_ResourcesLow);

													// Encrypt secret
													EGPublicKey::CryptReturn block(recvMsgData->newRemoteCertificate.GetPubKey().Encrypt(secret.data(), secret.size()));
													if (block.first)
													{
														auto_ptr<unsigned char> autoBlock(block.first);

														// send challenge
														completeMsg.SetStatus(StatusAuth_VerifyFailed);

														Auth1Certificate* localCert = authSocket->identity->GetCertificate();
														if (localCert)
														{
															auto_ptr<Auth1Certificate> autoLocalCert(localCert);
															TMsgAuth1Challenge1 challenge;
															challenge.SetSecretB(block.first, block.second);
															challenge.SetRawBuf(localCert->GetRaw(), localCert->GetRawLen());
															
															// uses private version, no encryption
															authSocket->SendBaseMsg(challenge, -1, true, true);
															sendComplete = false;
														}
													}
												}
											}
										}
									}
								default:
									// unknown encryption type, fail
									break;
								}

							}
						default:
							break;
						}
					}
					else if (titanHeader->_MessageType == Auth1Challenge2)
					{
						TMessage tmsg(result.msgLength, result.msg);
						TMsgAuth1Challenge2 msg(tmsg);

						unsigned short bufSize = msg.GetRawBufLen();
						const unsigned char* buf = msg.GetRawBuf();

						completeMsg.SetStatus(StatusAuth_VerifyFailed);

						EGPrivateKey* privateKey = authSocket->identity->GetPrivateKey();
						if (privateKey)
						{
							auto_ptr<EGPrivateKey> autoPrivateKey(privateKey);

							CryptKeyBase::CryptReturn decryptResult = privateKey->Decrypt(buf, bufSize);
							if (decryptResult.first)
							{
								auto_ptr<unsigned char> autoDecryptResult(decryptResult.first);

								if (decryptResult.second > sizeof(unsigned short))
								{
									unsigned short secretSize = getLittleEndian(*(unsigned short*)decryptResult.first);
									if (secretSize + sizeof(unsigned short) < decryptResult.second)
									{
										unsigned char* secret = decryptResult.first + sizeof(unsigned short);
										unsigned short clientSecretSize = (decryptResult.second - sizeof(unsigned short)) - secretSize;
										unsigned char* clientSecret = decryptResult.first + sizeof(unsigned short) + secretSize;

										unsigned short origSecretSize = authSocket->sessionKey.GetKeyLen();
										if (secretSize == origSecretSize &&
											memcmp(secret, authSocket->sessionKey.GetKey(), secretSize) == 0)
										{
											RawBuffer rawBuf;
											unsigned short tmpClientSecretSize = getLittleEndian(clientSecretSize);
											rawBuf.append((unsigned char*)&tmpClientSecretSize, sizeof(unsigned short));
											rawBuf.append(clientSecret, clientSecretSize);

											EGPublicKey::CryptReturn block(recvMsgData->newRemoteCertificate.GetPubKey().Encrypt(rawBuf.data(), rawBuf.size()));
											if (block.first)
											{
												auto_ptr<unsigned char> autoBlock(block.first);
												
												unsigned short thisSessionId = 0;
												if (recvMsgData->authMode == AUTH_SESSION)
													thisSessionId = ++AuthSession::sessionIdSeed;

												authSocket->session = new AuthSession(authSocket, thisSessionId,
													authSocket->sessionKey, recvMsgData->newRemoteCertificate,
													authSocket->acceptedSessionExpiration, true);
												if (authSocket->session)
												{
													completeMsg.SetRawBuf(block.first, block.second, true);
													completeMsg.SetStatus(StatusCommon_Success);
													if (recvMsgData->authMode == AUTH_SESSION)
													{
														AutoCrit autoSessionMapCrit(authSocketStatics->incomingSessionMapCrit);
														
														// Use this opportunity to purge old sessions
														IncomingSessionMap::iterator itor = authSocketStatics->incomingSessionMap.begin();													
														while (itor != authSocketStatics->incomingSessionMap.end())
														{
															AuthSession* tmpSession = (*itor).second;
															AutoCrit autoSessionCrit(tmpSession->sessionCrit);
															IncomingSessionMap::iterator thisItor = itor;
															itor++;
															if (tmpSession->refCount == 1)
															{
																time_t now;
																time(&now);
																if ((now - tmpSession->lastUsed > tmpSession->timeout)
																	|| tmpSession->remoteCertificate.IsExpired()
																	|| tmpSession->localCertificate.IsExpired())
																{
																	authSocketStatics->incomingSessionMap.erase(thisItor);
																	autoSessionCrit.Leave();
																	delete tmpSession;
																}
															}
														}

														authSocket->session->refCount++;
														authSocketStatics->incomingSessionMap[authSocket->session->id] = authSocket->session;
														completeMsg.SetSessionId(authSocket->session->id);
													}
												}
												else
													completeMsg.SetStatus(StatusCommon_ResourcesLow);
											}
										}
									}
								}
							}
						}
					}
				}
				if (sendComplete)
				{
					authSocket->SendBaseMsg(completeMsg, -1, true, true);
					if (completeMsg.GetStatus() == StatusCommon_Success)
						authSocket->authenticated = true;
				}

				authSocket->TMsgSocket::RecvRawMsg(recvMsgData->recvMsgBuf, recvMsgData->length,
					recvMsgData->timeout, recvMsgData->autoDelete,
					new CompletionWithContents<const TMsgSocket::RecvRawMsgResult&, AuthRecvMsgData*>(recvMsgData, DoneRecvMsg, true));
				return;
			}
		}
		
		autoAuthCrit.Leave();

		bool socketClosed = false;
		bool wasEncrypted;
		unsigned long newLength;
		autoResultMsg.release();
		unsigned char* msg = authSocket->DecryptMessage(recvMsgData->allowUnencrypted, result.msg, result.msgLength, newLength, wasEncrypted);
		if (!msg && authSocket->GetType() != datagram)
		{
			authSocket->Close(0);
			socketClosed = true;
		}

		recvMsgData->Done(AuthSocket::AuthRecvRawMsgResult(result.theSocket, msg, newLength, wasEncrypted, false, socketClosed));
	}
}


unsigned char* AuthSocket::RecvRawMsg(unsigned char** recvMsgBuf, unsigned long* length, bool allowUnencrypted,
								   long timeout, bool async, const CompletionContainer<const AuthRecvRawMsgResult&>& completion)
{
	AuthRecvMsgData* recvMsgData = new AuthRecvMsgData;
	if (!recvMsgData)
	{
		completion.Complete(AuthRecvRawMsgResult(this, 0, 0, false, false, false));
		return 0;
	}

	recvMsgData->recvMsgBuf = recvMsgBuf;
	recvMsgData->length = length;
	recvMsgData->allowUnencrypted = allowUnencrypted;
	recvMsgData->completion = completion;
	recvMsgData->autoDelete = async;
	recvMsgData->timeout = timeout;

	TMsgSocket::RecvRawMsg(recvMsgBuf, length, timeout, async, new CompletionWithContents<const TMsgSocket::RecvRawMsgResult&, AuthRecvMsgData*>(recvMsgData, DoneRecvMsg, true));

	if (async)
		return 0;

	unsigned char* theMsg = recvMsgData->recvMsg;

	delete recvMsgData;

	return theMsg;
}


static void RecvBaseMsgCompletionTranslator(const AuthSocket::AuthRecvBaseMsgResult& result, CompletionContainer<const TMsgSocket::RecvBaseMsgResult&> completion)
{
	completion.Complete(result);
}


BaseMessage* AuthSocket::RecvBaseMsg(BaseMessage** recvMsg, long timeout, bool async,
								 const CompletionContainer<const TMsgSocket::RecvBaseMsgResult&>& completion)
{
	return RecvBaseMsgEx(recvMsg, allowUnencrypted, timeout, async, RecvBaseMsgCompletionTranslator, completion);
}


void AuthSocket::DoneRecvBaseMsg(const AuthSocket::AuthRecvRawMsgResult& result, AuthRecvBaseMsgData* recvBaseMsgData)
{
	recvBaseMsgData->socketClosed = result.closed;
	recvBaseMsgData->wasEncrypted = result.wasEncrypted;
	recvBaseMsgData->authExpired = result.authExpired;
	recvBaseMsgData->msg = NULL;

	auto_ptr<unsigned char> autoResultMsg(result.msg);

	if (result.msg)
		recvBaseMsgData->msg = result.theSocket->ExtractMessage(result.msg, result.msgLength);

	if (recvBaseMsgData->recvMsg)
		*(recvBaseMsgData->recvMsg) = recvBaseMsgData->msg;

	recvBaseMsgData->Done();
}


BaseMessage* AuthSocket::RecvBaseMsg(BaseMessage** recvMsg, bool allowUnencrypted, long timeout, bool async,
								 const CompletionContainer<const AuthRecvBaseMsgResult&>& completion)
{
	AuthRecvBaseMsgData* recvBaseMsgData = new AuthRecvBaseMsgData;
	if (!recvBaseMsgData)
	{
		completion.Complete(AuthRecvBaseMsgResult(this, 0, false, false, false));
		return 0;
	}

	recvBaseMsgData->recvMsg = recvMsg;
	recvBaseMsgData->completion = completion;
	recvBaseMsgData->autoDelete = async;
	recvBaseMsgData->thisSocket = this;
	recvBaseMsgData->socketClosed = false;

	void (*DoneRecvBaseMsgFunc)(const AuthSocket::AuthRecvRawMsgResult&, AuthRecvBaseMsgData*) = DoneRecvBaseMsg;

	RecvRawMsgEx((unsigned char**)0, 0, allowUnencrypted, timeout, async, DoneRecvBaseMsgFunc, recvBaseMsgData);

	if (async)
		return 0;

	BaseMessage* msg = recvBaseMsgData->msg;

	delete recvBaseMsgData;

	return msg;
}


static void SendMsgCompletionTranslator(const AuthSocket::AuthSendRawMsgResult& result, CompletionContainer<const TMsgSocket::SendRawMsgResult&> completion)
{
	completion.Complete(result);
}


bool AuthSocket::SendRawMsg(unsigned long length, const void* msg, long timeout, bool async, bool copyData,
						 const CompletionContainer<const TMsgSocket::SendRawMsgResult&>& completion)
{
	return SendRawMsgEx(length, msg, encrypting, timeout, async, copyData, SendMsgCompletionTranslator, completion);
}


static void SendRequestTranslator1(const AuthSocket::AuthRecvRawMsgResult& result, CompletionContainer<const TMsgSocket::RecvRawMsgResult&> completion)
{
	completion.Complete(result);
}


unsigned char* AuthSocket::SendRawMsgRequest(unsigned long length, const void* req, unsigned char** replyBuf,
										  unsigned long* replyLength, unsigned long retryTimeout,
										  long timeout, bool async, bool copyData,
										  const CompletionContainer<const TMsgSocket::RecvRawMsgResult&>& completion)
{
	return SendRawMsgRequestEx(length, req, replyBuf, replyLength, true, false, retryTimeout, timeout, async,
						  copyData, SendRequestTranslator1, completion);
}


static void SendRequestTranslator2(const AuthSocket::AuthRecvBaseMsgResult& result, CompletionContainer<const TMsgSocket::RecvBaseMsgResult&> completion)
{
	completion.Complete(result);
}


BaseMessage* AuthSocket::SendBaseMsgRequest(BaseMessage& req, BaseMessage** reply, unsigned long retryTimeout,
									 long timeout, bool async, bool copyData,
									 const CompletionContainer<const TMsgSocket::RecvBaseMsgResult&>& completion)
{
	return SendBaseMsgRequestEx(req, reply, true, false, retryTimeout, timeout, async, copyData, SendRequestTranslator2, completion);
}



static void DoneSendUnencryptedMessage(const TMsgSocket::SendRawMsgResult& result, CompletionContainer<const AuthSocket::AuthSendRawMsgResult&> completion)
{
	completion.Complete(AuthSocket::AuthSendRawMsgResult(result.theSocket, result.msg, result.msgLength, result.success, false, result.closed));
}


static void DoneSendEncryptedMessage(const TMsgSocket::SendRawMsgResult& result, DoneSendEncryptedMessageData data)
{
	delete result.msg;
	data.completion.Complete(AuthSocket::AuthSendRawMsgResult(result.theSocket, data.originalMsg, data.originalLength, result.success, true, result.closed));
}


bool AuthSocket::SendRawMsg(unsigned long length, const void* msg, bool encrypt, long timeout, bool async,
							bool copyData, const CompletionContainer<const AuthSendRawMsgResult&>& completion)
{
	if (!authenticated || !encrypt || !encrypting)
		return TMsgSocket::SendRawMsg(length, msg, timeout, async, copyData, new CompletionWithContents<const TMsgSocket::SendRawMsgResult&, CompletionContainer<const AuthSendRawMsgResult&> >(completion, DoneSendUnencryptedMessage, true));

	if (!msg || !sessionKey.GetKeyLen())
	{
		completion.Complete(AuthSendRawMsgResult(this, (unsigned char*)msg, length, false, true, false));
		return false;
	}

	unsigned char* encryptFrom = (unsigned char*)msg;
	unsigned long encryptLength = length;

	unsigned char headerType = 2;
	if (length != 0)
	{
		switch (*(unsigned char*)msg)
		{
		case 3:	// MiniMessage
		case 5: // SmallMessage
		case 7:	// LargeMessage
			headerType = *(unsigned char*)msg + 1;
			encryptFrom++;
			encryptLength--;
			break;
		default:
		//	headerType = 2;
			break;
		}
	}

	AutoCrit autoSessionCrit(session->sessionCrit);

	time(&(session->lastUsed));

	unsigned char* tempBuf = 0;
	if (sequenced)
	{
		// Gotta copy it all into a new buffer, so the sequence id gets encoded with it

		tempBuf = new unsigned char[encryptLength + sizeof(unsigned short)];
		if (!tempBuf)
		{
			completion.Complete(AuthSendRawMsgResult(this, (unsigned char*)msg, length, false, true, false));
			return false;
		}
		*(unsigned short*)tempBuf = getLittleEndian(session->sendSeqNum++);
		memcpy(tempBuf + sizeof(unsigned short), encryptFrom, encryptLength);

		encryptFrom = tempBuf;
		encryptLength += sizeof(unsigned short);
	}

	// encrypt the message
	BFSymmetricKey::CryptReturn aCryptReturn;
	if (!encryptLength)
	{
		aCryptReturn.first = new unsigned char[2];
		aCryptReturn.second = 0;
	}
	else
		aCryptReturn = sessionKey.Encrypt(encryptFrom, encryptLength);

	if (tempBuf)
		delete tempBuf;

	if (! aCryptReturn.first)
	{
		if (sequenced)
			session->sendSeqNum--;	// Don't invalidate the sequence number just because encryption failed
		completion.Complete(AuthSendRawMsgResult(this, (unsigned char*)msg, length, false, true, false));
		return false;
	}
	
	unsigned char* finalMsg;
	unsigned long finalMsgLength = aCryptReturn.second + 1;

	if (reuseSession)
		finalMsgLength += sizeof(unsigned short);
	
	finalMsg = new unsigned char[finalMsgLength];
	if (!finalMsg)
	{
		delete aCryptReturn.first;
		if (sequenced)
			session->sendSeqNum--;	// Don't invalidate the sequence number just because allocation failed
		completion.Complete(AuthSendRawMsgResult(this, (unsigned char*)msg, length, false, true, false));
		return false;
	}

	*finalMsg = headerType;
	if (reuseSession)
		*(unsigned short*)(finalMsg+1) = getLittleEndian(session->id);
	if (aCryptReturn.second)
		memcpy(finalMsg + 1 + (reuseSession ? sizeof(unsigned short) : 0), aCryptReturn.first, aCryptReturn.second);
	delete aCryptReturn.first;

	DoneSendEncryptedMessageData data;
	data.completion = completion;
	data.originalMsg = (unsigned char*)msg;
	data.originalLength = length;

	return TMsgSocket::SendRawMsg(finalMsgLength, finalMsg, timeout, async, false, new CompletionWithContents<const TMsgSocket::SendRawMsgResult&, DoneSendEncryptedMessageData>(data, DoneSendEncryptedMessage, true));
}


static void SendBaseMsgCompletionTranslator(const AuthSocket::AuthSendBaseMsgResult& result, CompletionContainer<const TMsgSocket::SendBaseMsgResult&> completion)
{
	completion.Complete(result);
}


bool AuthSocket::SendBaseMsg(BaseMessage& msg, long timeout, bool async, bool copyData,
			 const CompletionContainer<const TMsgSocket::SendBaseMsgResult&>& completion)
{
	return SendBaseMsgEx(msg, encrypting, timeout, async, copyData, SendBaseMsgCompletionTranslator, completion);
}


static void DoneSendBaseMsg(const AuthSocket::AuthSendRawMsgResult& result, SendBaseMsgData* sendBaseMsgData)
{
	sendBaseMsgData->completion.Complete(AuthSocket::AuthSendBaseMsgResult(result.theSocket, sendBaseMsgData->msg, result.success, result.wasEncrypted, result.closed));
	delete sendBaseMsgData;
}


bool AuthSocket::SendBaseMsg(BaseMessage& msg, bool encrypt, long timeout, bool async, bool copyData,
						 const CompletionContainer<const AuthSendBaseMsgResult&>& completion)
{
	SendBaseMsgData* sendBaseMsgData = new SendBaseMsgData;
	if (!sendBaseMsgData)
	{
		completion.Complete(AuthSendBaseMsgResult(this, &msg, false, encrypt && encrypting, false));
		return false;
	}

	sendBaseMsgData->msg = &msg;
	sendBaseMsgData->completion = completion;

	try {
		msg.Pack();
	}
	catch (...)
	{
		delete sendBaseMsgData;
		completion.Complete(AuthSendBaseMsgResult(this, &msg, false, encrypt && encrypting, false));
		return false;
	}

	unsigned long l = msg.GetDataLen();
	void* dataPtr = msg.GetDataPtr();
	return SendRawMsgEx(l, dataPtr, encrypt && encrypting, timeout, async, copyData, DoneSendBaseMsg, sendBaseMsgData);
}


Auth1Certificate* AuthSocket::GetRemoteCertificate()
{
	Auth1Certificate* result = 0;

	AutoCrit autoAuthCrit(authCrit);

	if (session)
	{
		AutoCrit autoSessionCrit(session->sessionCrit);
		result = new Auth1Certificate(session->remoteCertificate);
	}

	return result;
}


void AuthSocket::SetIdentity(Identity* ident)
{
	identity = ident /*? new Identity(*ident) : 0*/;
}


Identity* AuthSocket::GetIdentity()
{
	return identity;
}


void AuthSocket::DoneRecvReliableUDP(const AuthSocket::AuthRecvRawMsgResult& result, AuthSocket::RequestData* requestData)
{
	AuthSocket* thisSocket = (AuthSocket*)(result.theSocket);

	if (result.closed)
		requestData->Done(AuthRecvRawMsgResult(thisSocket, 0, 0, false, false, true));

	AutoCrit autoCrit(thisSocket->tagCrit);

	thisSocket->recving = false;

	if (!thisSocket->abortingRecv)
	{
		if (!result.msgLength)
		{
			if (result.authExpired)
			{
				thisSocket->numAuthsPending++;
				thisSocket->AuthenticateEx(thisSocket->identity, false, (requestData->retryTimeout), true, DoneAuthenticate, requestData);
				return;
			}

			if (GetTickCount() >= requestData->startTime + requestData->timeout)
				requestData->Done(AuthRecvRawMsgResult(thisSocket, 0, 0, false, false, false));
			else
			{
				requestData->ResetTimeout();
				thisSocket->AuthSocket::SendRawMsg(requestData->outgoingMsgLength, requestData->outgoingMsg, true, -1, true, true);
			}
			thisSocket->IssueRecv();
			return;
		}
	}

	if (result.msg != 0)
	{
		// handle message
		if (*(result.msg) == HeaderWithTag)	// 13
		{
			unsigned short tag = getLittleEndian(*(unsigned short*)(result.msg + 1));
			TagMap::iterator itor = thisSocket->tagMap.find(tag);
			if (itor != thisSocket->tagMap.end())
			{
				RequestData* replyTo = (AuthSocket::RequestData*)((*itor).second);
				unsigned char* actualMsg = new unsigned char[result.msgLength - 3];
				if (actualMsg)
				{
					memcpy(actualMsg, result.msg + 3, result.msgLength - 3);
					replyTo->Done(AuthRecvRawMsgResult(thisSocket, actualMsg, result.msgLength-3, result.wasEncrypted, false, false));
				}
			}
		}
		delete result.msg;
	}
	if (!(thisSocket->abortingRecv))
		thisSocket->IssueRecv();
	else
		thisSocket->abortingRecv = false;
	thisSocket->doneRecvEvent.Set();
}


void AuthSocket::IssueRecv()
{
	TimeoutSet::iterator itor = timeoutSet.begin();
	if (itor != timeoutSet.end())
	{
		recving = true;

		RequestData* requestData = (AuthSocket::RequestData*)(*itor);

		void (*DoneRecvReliableUDPProc)(const AuthSocket::AuthRecvRawMsgResult&, RequestData*) = DoneRecvReliableUDP;

		unsigned long elasped = GetTickCount() - requestData->tryStartTime;
		long retryTimeout = (elasped >= requestData->retryTimeout) ? 0 : requestData->retryTimeout - elasped;
		RecvRawMsgEx((unsigned char**)0, (unsigned long*)0, allowUnencrypted, retryTimeout, true, DoneRecvReliableUDPProc, requestData);
	}
}


void AuthSocket::DoneAuthenticate(const AuthSocket::AuthResult& result, RequestData* udpData)
{
	AuthSocket* thisSocket = (AuthSocket*)(result.theSocket);

	AutoCrit autoCrit(thisSocket->tagCrit);

	bool lastAuth = !--(thisSocket->numAuthsPending);

	if (lastAuth)
		thisSocket->isAuthenticating = false;

	if (result.error != Error_Success)
		udpData->Done(AuthRecvRawMsgResult(thisSocket, 0, 0, false, true, false));
	else
	{
		udpData->ResetTimeout();
		thisSocket->SendRawMsg(udpData->outgoingMsgLength, udpData->outgoingMsg, true, -1, true, true);
		if (lastAuth)
			thisSocket->IssueRecv();
	}
}


void AuthSocket::DoneAuthenticateOpen(const AuthSocket::OpenResult& result, RequestData* udpData)
{
	AuthSocket* thisSocket = (AuthSocket*)(result.theSocket);

	AutoCrit autoCrit(thisSocket->tagCrit);

	bool lastAuth = !--(thisSocket->numAuthsPending);

	if (lastAuth)
		thisSocket->isAuthenticating = false;

	if (result.error != Error_Success)
		udpData->Done(AuthRecvRawMsgResult(thisSocket, 0, 0, false, true, true));
	else
	{
		udpData->ResetTimeout();
		thisSocket->SendRawMsg(udpData->outgoingMsgLength, udpData->outgoingMsg, true, -1, true, true);
		if (lastAuth)
			thisSocket->IssueRecv();
	}
}


void AuthSocket::DoneStreamAuthenticate(const AuthSocket::AuthResult& result, RequestData* requestData)
{
	AuthSocket* thisSocket = (AuthSocket*)(result.theSocket);

	if (result.error != Error_Success)
		requestData->Done(AuthRecvRawMsgResult(thisSocket, 0, 0, false, true, false));
	else
	{
		thisSocket->SendRawMsg(requestData->outgoingMsgLength, requestData->outgoingMsg, true, -1, true, true);
		thisSocket->RecvRawMsgEx((unsigned char**)0, (unsigned long*)0, thisSocket->allowUnencrypted, requestData->timeout, true, DoneStreamReqRecv, requestData);
	}
}


void AuthSocket::DoneStreamReqRecv(const AuthSocket::AuthRecvRawMsgResult& result, RequestData* requestData)
{
	AuthSocket* thisSocket = (AuthSocket*)(result.theSocket);

	if (result.authExpired)
	{
		thisSocket->Close();
		thisSocket->AuthenticateEx(false, requestData->retryTimeout, true, DoneStreamAuthenticate, requestData);
		return;
	}
	requestData->Done(result);
}


void AuthSocket::DoneStreamReqAuth(const AuthSocket::OpenResult& result, RequestData* requestData)
{
	AuthSocket* thisSocket = (AuthSocket*)(result.theSocket);

	if (result.error != Error_Success)
		requestData->Done(AuthRecvRawMsgResult(thisSocket, 0, 0, false, true, true));
	else
	{
		thisSocket->SendRawMsg(requestData->outgoingMsgLength, requestData->outgoingMsg, true, -1, true, true);
		thisSocket->RecvRawMsgEx((unsigned char**)0, (unsigned long*)0, thisSocket->allowUnencrypted, requestData->timeout, true, DoneStreamReqRecv, requestData);
	}
}


// Can only do one TCP request at a time... Can do multiple UDP requests at the same time

unsigned char* AuthSocket::SendRawMsgRequest(unsigned long length, const void* req, unsigned char** replyBuf,
										  unsigned long* replyLength, bool encrypt, bool allowUnencrypted,
										  unsigned long retryTimeout, long timeout, bool async, bool copyData,
										  const CompletionContainer<const AuthRecvRawMsgResult&>& completion)
{
	if (!req || !length)
	{
		completion.Complete(AuthRecvRawMsgResult(this, 0, 0, false, false, false));
		return 0;
	}

	RequestData* requestData = new AuthSocket::RequestData;
	if (!requestData)
	{
		completion.Complete(AuthRecvRawMsgResult(this, 0, 0, false, false, false));
		return 0;
	}

	requestData->authCompletion = completion;
	requestData->timeout = timeout;
	requestData->replyMsg = replyBuf ? replyBuf : &(requestData->tempReplyMsg);
	requestData->replyLen = replyLength;
	requestData->autoDel = async;


	if (actualSocket->GetType() == datagram)
	{
		unsigned char* newMsg = new unsigned char[length+4];
		if (!newMsg)
		{
			delete requestData;
			completion.Complete(AuthRecvRawMsgResult(this, 0, 0, false, false, false));
			return 0;
		}

		AutoCrit autoCrit(tagCrit);

		// If we're currently authenticating, then do everything, and the recv will be issued when auth is done
		// when it's done
		// ...   If we're already recving, cancel the recv, and it'll be triggered after authentication

		if (!isAuthenticating && recving)
		{
			abortingRecv = true;
			doneRecvEvent.Reset();
			actualSocket->AbortRecv();
			autoCrit.Leave();
			//doneRecvEvent.WaitFor();
			WSSocket::PumpUntil(doneRecvEvent, timeout);
			autoCrit.Enter();
		}

		memcpy(newMsg+4, req, length);
		*newMsg = HeaderWithTagAndExpiration; // 14
		*(unsigned short*)(newMsg + 1) = getLittleEndian(tagSeqNum);

		unsigned long realTimeout = (timeout > 255000 || timeout < 0) ? 255000 : timeout;

		// Add a second to the expiration timeout, just in case
		unsigned long tmpTimeout = (realTimeout / 1000) + 1;
		*(newMsg + 3) = (unsigned char)(tmpTimeout > 0xFF ? 0xFF : tmpTimeout);

		requestData->retryTimeout = retryTimeout;
		requestData->timeout = realTimeout;
		requestData->startTime = GetTickCount();
		requestData->tryStartTime = requestData->startTime;
		requestData->outgoingMsg = newMsg;
		requestData->deallocReq = true;
		requestData->outgoingMsgLength = length+4;
		requestData->thisSocket = this;
		requestData->usingDatagrams = true;
		
		requestData->tagMapItor = tagMap.insert(TagMap::value_type(tagSeqNum, requestData)).first;
		requestData->timeoutSetItor = timeoutSet.insert(requestData);

		tagSeqNum++;

		numAuthsPending++;
		void (*DoneAuthenticateOpenProc)(const AuthSocket::OpenResult&, RequestData*) = AuthSocket::DoneAuthenticateOpen;
		OpenEx(true, timeout, true, DoneAuthenticateOpenProc, requestData);

		autoCrit.Leave();

	}
	else
	{
		unsigned char* theReq = (unsigned char*)req;
		if (copyData)
		{
			theReq = new unsigned char[length];
			if (!theReq)
			{
				delete requestData;
				completion.Complete(AuthRecvRawMsgResult(this, 0, 0, false, false, false));
				return 0;
			}
			memcpy(theReq, req, length);
		}

		requestData->deallocReq = copyData;
		requestData->outgoingMsg = theReq;
		requestData->outgoingMsgLength  = length;
		requestData->usingDatagrams = false;

		void (*DoneStreamReqAuthProc)(const AuthSocket::OpenResult&, RequestData*) = AuthSocket::DoneStreamReqAuth;
		OpenEx(true, timeout, true, DoneStreamReqAuthProc, requestData);
	}

	unsigned char* result = 0;
	if (!async)
	{
		WSSocket::PumpUntil(requestData->doneEvent, timeout);
		//requestData->doneEvent.WaitFor();
		result = *(requestData->replyMsg);
		delete requestData;
	}
	return result;
}



static void DoneSendRequest(const AuthSocket::AuthRecvRawMsgResult& result, SendRequestData* data)
{
	BaseMessage* baseMsg = 0;

	if (result.msg)
		baseMsg = TMsgSocket::ExtractMessage(result.msg, result.msgLength);

	*(data->reply) = baseMsg;

	data->Done(AuthSocket::AuthRecvBaseMsgResult(result.theSocket, baseMsg, result.wasEncrypted, result.authExpired, result.closed));

	if (result.msg)
		delete result.msg;
}


BaseMessage* AuthSocket::SendBaseMsgRequest(BaseMessage& req, BaseMessage** reply, bool encrypt,
									 bool allowUnencrypted, unsigned long retryTimeout, long timeout,
									 bool async, bool copyData, const CompletionContainer<const AuthRecvBaseMsgResult&>& completion)
{
	SendRequestData* data = new SendRequestData();
	if (!data)
	{
		completion.Complete(AuthRecvBaseMsgResult(this, 0, false, false, false));
		return 0;
	}

	data->completion = completion;
	data->autoDel = async;
	data->reply = (reply == NULL) ? &(data->tmpReply) : reply;

	try {
		req.Pack();
	}
	catch (...)
	{
		delete data;
		completion.Complete(AuthRecvBaseMsgResult(this, 0, false, false, false));
		return 0;
	}

	unsigned long l = req.GetDataLen();
	void* dataPtr = req.GetDataPtr();

	SendRawMsgRequestEx(l, dataPtr, 0, 0, encrypt, allowUnencrypted, retryTimeout, timeout, true, copyData, DoneSendRequest, data);

	BaseMessage* result = 0;
	if (!async)
	{
		WSSocket::PumpUntil(data->doneEvent, timeout);
		//data->doneEvent.WaitFor();
		result = *(data->reply);
		delete data;
	}
	return result;
}


static void ReliableRecvMsgCompletionTranslator(const AuthSocket::AuthReliableRecvRawMsgResult& result, CompletionContainer<const TMsgSocket::ReliableRecvRawMsgResult&> completion)
{
	completion.Complete(result);
}


static void RecvBaseMsgCompletionTranslator(const AuthSocket::AuthReliableRecvBaseMsgResult& result, CompletionContainer<const TMsgSocket::ReliableRecvBaseMsgResult&> completion)
{
	completion.Complete(result);
}


unsigned char* AuthSocket::ReliableRecvRawMsg(unsigned short* tagNum, unsigned char** recvMsgBuf,
											  unsigned long* length, long timeout, bool async,
											  const CompletionContainer<const TMsgSocket::ReliableRecvRawMsgResult&>& completion)
{
	return ReliableRecvRawMsg(tagNum, recvMsgBuf, length, allowUnencrypted, timeout, async, new CompletionWithContents<const AuthReliableRecvRawMsgResult&, CompletionContainer<const TMsgSocket::ReliableRecvRawMsgResult&> >(completion, ReliableRecvMsgCompletionTranslator, true));
}


WONMsg::BaseMessage* AuthSocket::ReliableRecvBaseMsg(unsigned short* tagNum, WONMsg::BaseMessage** recvMsg,
													 long timeout, bool async,
													 const CompletionContainer<const TMsgSocket::ReliableRecvBaseMsgResult&>& completion)
{
	return ReliableRecvBaseMsg(tagNum, recvMsg, allowUnencrypted, timeout, async, new CompletionWithContents<const AuthReliableRecvBaseMsgResult&, CompletionContainer<const TMsgSocket::ReliableRecvBaseMsgResult&> >(completion, RecvBaseMsgCompletionTranslator, true));
}


class AuthSocket::ReliableRecvRawMsgData
{
public:
	CompletionContainer<const AuthReliableRecvRawMsgResult&> completion;
	unsigned short* tagNum;
	unsigned short tmpTag;
	unsigned char* result;
	unsigned char** recvMsgBuf;
	unsigned long* length;
	bool allowUnencrypted;
	long timeout;
	Event doneEvent;
	bool autoDel;

	void Done(const AuthRecvRawMsgResult& result)
	{
		completion.Complete(AuthReliableRecvRawMsgResult(*tagNum, result));

		if (autoDel)
			delete this;
		else
			doneEvent.Set();
	}

	ReliableRecvRawMsgData()
	{
	}
};


void AuthSocket::DoneReliableRecvRawMsg(const AuthRecvRawMsgResult& result, ReliableRecvRawMsgData* reliableRecvRawMsgData)
{
	AuthSocket* thisSocket = (AuthSocket*)result.theSocket;

	if (!result.msg || *(result.msg) != HeaderWithTagAndExpiration)	// 14
	{
		reliableRecvRawMsgData->Done(result);
		return;
	}

	AuthRecvRawMsgResult newResult(result);

	unsigned char* msg = result.msg;
	unsigned long msgLength = result.msgLength;
	
	newResult.msg = 0;
	newResult.msgLength = 0;

	unsigned short tagNum = getLittleEndian(*(unsigned short*)(msg + 1));
	
	AutoCrit autoCrit(thisSocket->tagInfoCrit);

	// expire old tags here
	unsigned long now = GetTickCount();
	TagExpireMap::iterator itor = thisSocket->tagExpireMap.begin();
	while (itor != thisSocket->tagExpireMap.end())
	{
		TagExpireMap::iterator nextItor = itor;
		nextItor++;

		TagInfo* tagInfo = (*itor).second;
		unsigned long elapsed = now - tagInfo->startTime;
		if (elapsed > tagInfo->timeout)
		{
			thisSocket->tagExpireMap.erase(itor);
			thisSocket->tagNumMap.erase(tagInfo->myItor);
			delete tagInfo;
		}
		itor = nextItor;
	}


	TagNumMap::iterator tagNumItor = thisSocket->tagNumMap.find(tagNum);
	if (tagNumItor != thisSocket->tagNumMap.end())
	{
		TagInfo* tagInfo = (*tagNumItor).second;

		if (tagInfo->msgLength)
		{
			thisSocket->SendRawMsg(tagInfo->msgLength, tagInfo->msg, tagInfo->encrypt, -1, true, true);
			thisSocket->RecvRawMsgEx(reliableRecvRawMsgData->recvMsgBuf, reliableRecvRawMsgData->length, reliableRecvRawMsgData->allowUnencrypted, reliableRecvRawMsgData->timeout, true, AuthSocket::DoneReliableRecvRawMsg, reliableRecvRawMsgData);

			delete msg;
			return;
		}
	}
	else
	{
		unsigned long expiration = *(unsigned char*)(msg + 3) * (unsigned long)1000;

		// create an entry for this tag and expiration
		TagInfo* tagInfo = new TagInfo;
		if (tagInfo)
		{
			tagInfo->startTime = now;
			tagInfo->timeout = expiration;
			tagInfo->tagNum = tagNum;
			tagInfo->msg = 0;
			tagInfo->msgLength = 0;
			tagInfo->encrypt = false;

			tagInfo->myItor = thisSocket->tagNumMap.insert(TagNumMap::value_type(tagNum, tagInfo)).first;
			thisSocket->tagExpireMap.insert(TagExpireMap::value_type(expiration, tagInfo));

			*(reliableRecvRawMsgData->tagNum) = tagNum;
		}
	}
	unsigned long newLen = msgLength - 4;
	unsigned char* newMsg = new unsigned char[newLen];
	if (newMsg)
	{
		memcpy(newMsg, msg+4, newLen);
		newResult.msg = newMsg;
		newResult.msgLength = newLen;
	}
	delete msg;
	*(reliableRecvRawMsgData->tagNum) = tagNum;
	reliableRecvRawMsgData->Done(newResult);
}


unsigned char* AuthSocket::ReliableRecvRawMsg(unsigned short* tagNum, unsigned char** recvMsgBuf,
											  unsigned long* length, bool allowUnencrypted, long timeout,
											  bool async, const CompletionContainer<const AuthReliableRecvRawMsgResult&>& completion)
{
	if (length)
		*length = 0;
	if (recvMsgBuf)
		*recvMsgBuf = 0;
	if (tagNum)
		*tagNum = 0;

	ReliableRecvRawMsgData* reliableRecvRawMsgData = new ReliableRecvRawMsgData;
	if (!reliableRecvRawMsgData)
	{
		completion.Complete(AuthReliableRecvRawMsgResult(0, this, 0, 0, false, false, false));
		return 0;
	}

	reliableRecvRawMsgData->completion = completion;
	reliableRecvRawMsgData->tagNum = tagNum ? tagNum : &(reliableRecvRawMsgData->tmpTag);
	reliableRecvRawMsgData->autoDel = async;
	reliableRecvRawMsgData->result = 0;
	reliableRecvRawMsgData->recvMsgBuf = recvMsgBuf;
	reliableRecvRawMsgData->allowUnencrypted = allowUnencrypted;
	reliableRecvRawMsgData->length = length;
	reliableRecvRawMsgData->timeout = timeout;
	
	void (*DoneReliableRecvRawMsgProc)(const AuthRecvRawMsgResult&, ReliableRecvRawMsgData*) = DoneReliableRecvRawMsg;
	RecvRawMsgEx(recvMsgBuf, length, allowUnencrypted, timeout, true, DoneReliableRecvRawMsgProc, reliableRecvRawMsgData);

	unsigned char* result = 0;
	if (!async)
	{
		WSSocket::PumpUntil(reliableRecvRawMsgData->doneEvent, timeout);
		//data->doneEvent.WaitFor();
		result = reliableRecvRawMsgData->result;
		delete reliableRecvRawMsgData;
	}
	return result;
}


class ReliableRecvBaseMsgData
{
public:
	CompletionContainer<const AuthSocket::AuthReliableRecvBaseMsgResult&> completion;
	bool autoDelete;
	bool socketClosed;
	bool wasEncrypted;
	bool authExpired;
	BaseMessage* msg;
	BaseMessage** recvMsg;
	AuthSocket* thisSocket;
	unsigned short tag;

	void Done()
	{
		completion.Complete(AuthSocket::AuthReliableRecvBaseMsgResult(tag, thisSocket, msg, wasEncrypted, authExpired, socketClosed));

		if (autoDelete)
			delete this;
	}
};


static void DoneReliableRecvBaseMsg(const AuthSocket::AuthReliableRecvRawMsgResult& result, ReliableRecvBaseMsgData* reliableRecvBaseMsgData)
{
	reliableRecvBaseMsgData->socketClosed = result.closed;
	reliableRecvBaseMsgData->msg = NULL;

	if (result.msg)
	{
		reliableRecvBaseMsgData->msg = result.theSocket->ExtractMessage(result.msg, result.msgLength);
		delete result.msg;
	}

	if (reliableRecvBaseMsgData->recvMsg)
		*(reliableRecvBaseMsgData->recvMsg) = reliableRecvBaseMsgData->msg;

	reliableRecvBaseMsgData->tag = result.tagNum;
	reliableRecvBaseMsgData->wasEncrypted = result.wasEncrypted;
	reliableRecvBaseMsgData->authExpired = result.authExpired;
	reliableRecvBaseMsgData->Done();
}


BaseMessage* AuthSocket::ReliableRecvBaseMsg(unsigned short* tagNum, BaseMessage** recvMsg,
											 bool allowUnencrypted, long timeout, bool async,
											 const CompletionContainer<const AuthReliableRecvBaseMsgResult&>& completion)
{
	ReliableRecvBaseMsgData* reliableRecvBaseMsgData = new ReliableRecvBaseMsgData;

	if (!reliableRecvBaseMsgData)
	{
		completion.Complete(AuthReliableRecvBaseMsgResult(0, this, 0, false));
		return 0;
	}

	reliableRecvBaseMsgData->completion = completion;
	reliableRecvBaseMsgData->autoDelete = async;
	reliableRecvBaseMsgData->recvMsg = recvMsg;
	reliableRecvBaseMsgData->thisSocket = this;

	ReliableRecvRawMsgEx(tagNum, 0, 0, allowUnencrypted, timeout, async, DoneReliableRecvBaseMsg, reliableRecvBaseMsgData);

	if (async)
		return 0;

	BaseMessage* msg = reliableRecvBaseMsgData->msg;

	delete reliableRecvBaseMsgData;

	return msg;
}


class ReliableSendRawMsgData
{
public:
	unsigned char* origMsg;
	unsigned char* newMsg;
	bool success;
	CompletionContainer<const AuthSocket::AuthSendRawMsgResult&> completion;
};


static void DoneReliableSendRawMsg(const AuthSocket::AuthSendRawMsgResult& result, ReliableSendRawMsgData* reliableSendRawMsgData)
{
	delete reliableSendRawMsgData->newMsg;
	reliableSendRawMsgData->completion.Complete(AuthSocket::AuthSendRawMsgResult(result.theSocket, reliableSendRawMsgData->origMsg, result.msgLength - 3, result.success, result.wasEncrypted, result.closed));
	delete reliableSendRawMsgData;
}


bool AuthSocket::ReliableSendRawMsgReply(unsigned short tagNum, unsigned long length, const void* msg, bool encrypt,
									long timeout, bool async, bool copyData, const CompletionContainer<const AuthSendRawMsgResult&>& completion)
{
	unsigned char* newMsg = 0;
	ReliableSendRawMsgData* reliableSendRawMsgData;

	if (msg && length && tagNum)
	{
		reliableSendRawMsgData = new ReliableSendRawMsgData;
		if (reliableSendRawMsgData)
		{
			newMsg = new unsigned char[length + 3];
			if (!newMsg)
				delete reliableSendRawMsgData;
		}
	}

	if (!newMsg)
	{
		completion.Complete(AuthSendRawMsgResult(this, (unsigned char*)msg, length, false, false, false));
		return false;
	}
	
	*(newMsg) = HeaderWithTag;	// 13
	*(unsigned short*)(newMsg+1) = getLittleEndian(tagNum);
	memcpy(newMsg+3, msg, length);

	AutoCrit autoCrit(tagInfoCrit);

	TagNumMap::iterator itor = tagNumMap.find(tagNum);
	if (itor != tagNumMap.end())
	{
		TagInfo* tagInfo = (*itor).second;
		delete tagInfo->msg;
		tagInfo->msg = newMsg;
		tagInfo->msgLength = length + 3;
		tagInfo->encrypt = encrypt;
		reliableSendRawMsgData->newMsg = 0;
	}
	else
	{
		reliableSendRawMsgData->newMsg = newMsg;
		// couldn't find tag in line.  Might have already expired.  Send it anyway
	}

	reliableSendRawMsgData->completion = completion;
	reliableSendRawMsgData->origMsg = (unsigned char*)msg;

	return SendRawMsgEx(length+3, newMsg, encrypt, timeout, async, false, DoneReliableSendRawMsg, reliableSendRawMsgData);
}


bool AuthSocket::ReliableSendBaseMsgReply(unsigned short tagNum, WONMsg::BaseMessage& msg, bool encrypt,
									 long timeout, bool async, bool copyData, const CompletionContainer<const AuthSendBaseMsgResult&>& completion)
{
	SendBaseMsgData* sendBaseMsgData = new SendBaseMsgData;
	if (!sendBaseMsgData)
	{
		completion.Complete(AuthSendBaseMsgResult(this, &msg, false, false));
		return false;
	}

	sendBaseMsgData->msg = &msg;
	sendBaseMsgData->completion = completion;

	try {
		msg.Pack();
	}
	catch (...)
	{
		delete sendBaseMsgData;
		completion.Complete(AuthSendBaseMsgResult(this, &msg, false, false));
		return false;
	}

	return ReliableSendRawMsgReplyEx(tagNum, msg.GetDataLen(), msg.GetDataPtr(), encrypt, timeout, async, copyData, DoneSendBaseMsg, sendBaseMsgData);
}


bool AuthSocket::ReliableSendRawMsgReply(unsigned short tagNum, unsigned long length, const void* msg, long timeout,
									bool async, bool copyData, const CompletionContainer<const TMsgSocket::SendRawMsgResult&>& completion)
{
	return ReliableSendRawMsgReplyEx(tagNum, length, msg, encrypting, timeout, async, copyData, SendMsgCompletionTranslator, completion);
}


bool AuthSocket::ReliableSendBaseMsgReply(unsigned short tagNum, WONMsg::BaseMessage& msg, long timeout,
									 bool async, bool copyData, const CompletionContainer<const TMsgSocket::SendBaseMsgResult&>& completion)
{
	return ReliableSendBaseMsgReplyEx(tagNum, msg, encrypting, timeout, async, copyData, SendBaseMsgCompletionTranslator, completion);
}


static CriticalSection peerAuthCrit;
static unsigned short listeningOnPort = 0;
static TCPSocket* peerAuthTCPSocket;
static Identity* peerAuthIdentity;
static auto_ptr<TCPSocket> autoPeerAuthTCPSocket;
static auto_ptr<Identity> autoPeerAuthIdentity;
static WONCommon::RawBuffer sendData;



Error WONAPI::ListenForPeerAuthentication(Identity& ident, unsigned short listenPort, const void* sendAfter,
								  unsigned short sendAfterSize)
{
	StopListeningForPeerAuthentication();

	AutoCrit autoCrit(peerAuthCrit);

	peerAuthIdentity = new Identity(ident);
	if (!peerAuthIdentity)
		return Error_OutOfMemory;

	peerAuthTCPSocket = new TCPSocket;
	if (!peerAuthTCPSocket)
	{
		delete peerAuthIdentity;
		peerAuthIdentity = 0;
		return Error_OutOfMemory;
	}

	autoPeerAuthTCPSocket = auto_ptr<TCPSocket>(peerAuthTCPSocket);
	autoPeerAuthIdentity = auto_ptr<Identity>(peerAuthIdentity);

	unsigned char c = 0;
	sendData.assign(&c, 1);	// dummy header type
	if (sendAfter && sendAfterSize)
		sendData.append((unsigned char*)sendAfter, sendAfterSize);

	return peerAuthTCPSocket->Listen(listenPort);
}


class AcceptPeerAuthData
{
public:
	WONAuth::Auth1Certificate* remoteCertBuf;
	long timeout;
	bool autoDel;
	Error err;
	CompletionContainer<Error> completion;
	Event doneEvent;
	TCPSocket* accepted;
	void* recvedAfterBuf;
	unsigned short* recvedAfterBufSize;

	void Done(Error error)
	{
		completion.Complete(error);
		
		if (autoDel)
			delete this;
		else
		{
			err = error;
			doneEvent.Set();
		}

		delete accepted;
	}
};


static void DoneClosePeerAuthHostSocket(const Socket::Result& result)
{
	delete (AuthSocket*)(result.theSocket);
}


static void DoneAcceptPeerAuthRecvMsg(const AuthSocket::AuthRecvRawMsgResult& result, AcceptPeerAuthData* acceptPeerAuthData)
{
	AutoCrit autoCrit(peerAuthCrit);

	if (!result.msg)
	{
		autoCrit.Leave();
		delete result.theSocket;
		acceptPeerAuthData->Done(Error_Timeout);
	}
	else
	{
		AuthSocket* authSocket = (AuthSocket*)(result.theSocket);
		if (authSocket->IsAuthenticated())
		{
			Auth1Certificate* remoteCertBuf = authSocket->GetRemoteCertificate();
			if (remoteCertBuf)
			{
				*(acceptPeerAuthData->remoteCertBuf) = *remoteCertBuf;
				delete remoteCertBuf;
			}

			if (acceptPeerAuthData->recvedAfterBuf && acceptPeerAuthData->recvedAfterBufSize
				&& *(acceptPeerAuthData->recvedAfterBufSize))
			{
				unsigned short msgLen = result.msgLength-1;	// skip dummy header
				unsigned short maxSize = *(acceptPeerAuthData->recvedAfterBufSize);
				unsigned short cpySize = maxSize < msgLen ? maxSize : msgLen;
				*(acceptPeerAuthData->recvedAfterBufSize) = msgLen;
				if (cpySize)
					memcpy(acceptPeerAuthData->recvedAfterBuf, ((unsigned char*)result.msg)+1, cpySize);
			}
			authSocket->SendRawMsg(sendData.size(), sendData.data(), true, -1, true, true);
			autoCrit.Leave();
			acceptPeerAuthData->Done(Error_Success);
			void (*DoneClosePeerAuthHostSocketFunc)(const Socket::Result&) = DoneClosePeerAuthHostSocket;
			authSocket->Close(-1, true, DoneClosePeerAuthHostSocketFunc);			
		}
		else
		{
			autoCrit.Leave();
			delete result.theSocket;
			acceptPeerAuthData->Done(Error_InvalidAuthLogin);
		}
		delete result.msg;
	}
}


static void DonePeerAuthRefreshIdent(const Identity::AuthResult& result, AcceptPeerAuthData* acceptPeerAuthData)
{
	AutoCrit autoCrit(peerAuthCrit);

	Error err = result.error;
	if (result.error == Error_Success)
	{
		if (!acceptPeerAuthData)
			err = Error_GeneralFailure;
		else
		{
			AuthSocket* authSocket = new AuthSocket(peerAuthIdentity, acceptPeerAuthData->accepted, true, 4, false, true, true, false, true);
			if (!authSocket)
				err = Error_OutOfMemory;
			else
			{
				acceptPeerAuthData->accepted = 0;

				// Need to issue a recv for the host side of peer authentication to occur automatically.
				// If the recv finished and authenticated != true, then something went wrong
				autoCrit.Leave();
				authSocket->RecvRawMsgEx(0, 0, false, acceptPeerAuthData->timeout, true, DoneAcceptPeerAuthRecvMsg, acceptPeerAuthData);
				return;
			}
		}
	}
	autoCrit.Leave();
	acceptPeerAuthData->Done(err);
}


static void PeerAuthAcceptDone(const TCPSocket::AcceptResult& result, AcceptPeerAuthData* acceptPeerAuthData)
{
	if (result.acceptedSocket)
	{
		acceptPeerAuthData->accepted = (TCPSocket*)result.acceptedSocket;
		peerAuthIdentity->RefreshEx(false, acceptPeerAuthData->timeout, true, DonePeerAuthRefreshIdent, acceptPeerAuthData);
	}
	else
		acceptPeerAuthData->Done(Error_Timeout);
}


Error WONAPI::AcceptPeerAuthentication(WONAuth::Auth1Certificate* remoteCertBuf, void* recvedAfterBuf,
									   unsigned short* recvedAfterBufSize, long timeout, bool async,
									   const CompletionContainer<Error>& completion)
{
	AutoCrit autoCrit(peerAuthCrit);

	if (!peerAuthTCPSocket)
	{
		completion.Complete(Error_InvalidState);
		return Error_InvalidState;
	}

	AcceptPeerAuthData* acceptPeerAuthData = new AcceptPeerAuthData;
	if (!acceptPeerAuthData)
	{		
		completion.Complete(Error_InvalidState);
		return Error_InvalidState;
	}
	acceptPeerAuthData->recvedAfterBufSize = recvedAfterBufSize;
	acceptPeerAuthData->recvedAfterBuf = recvedAfterBuf;
	acceptPeerAuthData->timeout = timeout;
	acceptPeerAuthData->autoDel = async;
	acceptPeerAuthData->remoteCertBuf = remoteCertBuf;
	acceptPeerAuthData->completion = completion;
	acceptPeerAuthData->accepted = 0;
	autoCrit.Leave();

	peerAuthTCPSocket->AcceptEx(0, timeout, true, PeerAuthAcceptDone, acceptPeerAuthData);

	if (async)
		return Error_Pending;

	WSSocket::PumpUntil(acceptPeerAuthData->doneEvent, timeout);
	Error err = acceptPeerAuthData->err;
	delete acceptPeerAuthData;
	return err;
}


void WONAPI::StopListeningForPeerAuthentication()
{
	AutoCrit autoCrit(peerAuthCrit);

	TCPSocket* tcpSocket = peerAuthTCPSocket;
	peerAuthTCPSocket = 0;
	autoPeerAuthTCPSocket.release();
	Identity* ident = peerAuthIdentity;
	peerAuthIdentity = 0;
	autoPeerAuthIdentity.release();

	sendData.erase();

	autoCrit.Leave();

	if (tcpSocket)
	{
		tcpSocket->Close(0);
		delete tcpSocket;
	}
	delete ident;
}


class PeerAuthenticateData
{
public:
	long timeout;
	bool autoDel;
	Error err;
	Event doneEvent;

	WONAuth::Auth1Certificate* hostCertBuf;
	WONCommon::RawBuffer sendData;

	void* recvedAfterBuf;
	unsigned short* recvedAfterBufSize;

	CompletionContainer<Error> completion;
	AuthSocket* authSocket;

	PeerAuthenticateData()
		:	authSocket(0)
	{ }

	~PeerAuthenticateData()
	{
		delete authSocket;
	}

	void Done(Error error)
	{
		completion.Complete(error);

		if (autoDel)
			delete this;
		else
		{
			err = error;
			doneEvent.Set();
		}
	}
};


static void DonePeerAuthRecvMsg(const AuthSocket::AuthRecvRawMsgResult& result, PeerAuthenticateData* peerAuthenticateData)
{
	AutoCrit autoCrit(peerAuthCrit);

	if (!result.msg)
		peerAuthenticateData->Done(Error_Timeout);
	else
	{
		AuthSocket* authSocket = (AuthSocket*)(result.theSocket);
		if (peerAuthenticateData->hostCertBuf)
		{
			Auth1Certificate* remoteCertBuf = authSocket->GetRemoteCertificate();
			if (remoteCertBuf)
			{
				*(peerAuthenticateData->hostCertBuf) = *remoteCertBuf;
				delete remoteCertBuf;
			}
		}

		if (peerAuthenticateData->recvedAfterBuf && peerAuthenticateData->recvedAfterBufSize
			&& *(peerAuthenticateData->recvedAfterBufSize))
		{
			unsigned short msgLen = result.msgLength-1;	// skip dummy header
			unsigned short maxSize = *(peerAuthenticateData->recvedAfterBufSize);
			unsigned short cpySize = maxSize < msgLen ? maxSize : msgLen;
			*(peerAuthenticateData->recvedAfterBufSize) = msgLen;
			if (cpySize)
				memcpy(peerAuthenticateData->recvedAfterBuf, ((unsigned char*)result.msg)+1, cpySize);
		}
		peerAuthenticateData->Done(Error_Success);
		delete result.msg;
	}
}


static void DonePeerAuthOpen(const AuthSocket::OpenResult& result, PeerAuthenticateData* peerAuthenticateData)
{
	if (result.error != Error_Success)
		peerAuthenticateData->Done(result.error);
	else
	{
		if (!peerAuthenticateData->authSocket->IsAuthenticated())
			peerAuthenticateData->Done(Error_InvalidAuthLogin);
		else
		{
			AuthSocket* authSocket = peerAuthenticateData->authSocket;

			authSocket->SendRawMsg(peerAuthenticateData->sendData.size(), peerAuthenticateData->sendData.data(), true, -1, true, true);
			authSocket->RecvRawMsgEx(0, 0, false, peerAuthenticateData->timeout, true, DonePeerAuthRecvMsg, peerAuthenticateData);
		}
	}


}


Error WONAPI::PeerAuthenticate(Identity& ident, const IPSocket::Address& host, WONAuth::Auth1Certificate* hostCertBuf,
							   const void* sendAfter, unsigned short sendAfterSize,
							   void* recvedAfterBuf, unsigned short* recvedAfterBufSize,
							   long timeout, bool async,
							   const CompletionContainer<Error>& completion )
{
	PeerAuthenticateData* peerAuthenticateData = new PeerAuthenticateData;
	if (!peerAuthenticateData)
	{
		completion.Complete(Error_OutOfMemory);
		return Error_OutOfMemory;
	}
	TCPSocket* tcpSocket = new TCPSocket(host);
	if (!tcpSocket)
	{
		delete peerAuthenticateData;
		completion.Complete(Error_OutOfMemory);
		return Error_OutOfMemory;
	}
	AuthSocket* authSocket = new AuthSocket(&ident, tcpSocket, true, 4, false);
	if (!authSocket)
	{
		delete peerAuthenticateData;
		delete tcpSocket;
		completion.Complete(Error_OutOfMemory);
		return Error_OutOfMemory;
	}

	peerAuthenticateData->authSocket = authSocket;
	peerAuthenticateData->timeout = timeout;
	peerAuthenticateData->autoDel = async;
	peerAuthenticateData->hostCertBuf = hostCertBuf;
	peerAuthenticateData->recvedAfterBuf = recvedAfterBuf;
	peerAuthenticateData->recvedAfterBufSize = recvedAfterBufSize;
	peerAuthenticateData->completion = completion;

	unsigned char c = 0;	// dummy header
	peerAuthenticateData->sendData.assign(&c, 1);
	if (sendAfter && sendAfterSize)
		peerAuthenticateData->sendData.append((unsigned char*)sendAfter, sendAfterSize);

	authSocket->OpenEx(true, timeout, true, DonePeerAuthOpen, peerAuthenticateData);

	if (async)
		return Error_Pending;

	WSSocket::PumpUntil(peerAuthenticateData->doneEvent, timeout);
	Error err = peerAuthenticateData->err;
	delete peerAuthenticateData;
	return err;
}


#include "wondll.h"


WONError WONPeerAuthListen(HWONAUTH hWONAuth, unsigned short listenPort, WON_CONST void* sendAfter,
						   unsigned short sendAfterSize)
{
	if (!hWONAuth)
		return Error_InvalidParams;
	
	return ListenForPeerAuthentication(*(Identity*)hWONAuth, listenPort, sendAfter, sendAfterSize);
}


WONError WONPeerAuthAccept(void* certificateBuf, unsigned short* certificateBufSize,
						   void* recvedAfterBuf, unsigned short* recvedAfterBufSize, long timeout)
{
	WONAuth::Auth1Certificate remoteCertBuf;
	Error err = AcceptPeerAuthentication(&remoteCertBuf, recvedAfterBuf, recvedAfterBufSize, timeout);
	if (certificateBufSize)
	{
		if (err == Error_Success && certificateBuf)
		{
			unsigned short maxSize = *certificateBufSize;
			if (maxSize)
			{
				const unsigned char* rawBuf = remoteCertBuf.GetRaw();
				unsigned short rawLen = remoteCertBuf.GetRawLen();
				*certificateBufSize = rawLen;
				if (maxSize >= rawLen)
					memcpy(certificateBuf, rawBuf, rawLen);
				return err;
			}
		}
		*certificateBufSize = 0;
	}
	return err;
}


void WONPeerAuthStopListening()
{
	StopListeningForPeerAuthentication();
}



WONError WONPeerAuthenticate(HWONAUTH hWONAuth, WON_CONST WONIPAddress* host, void* certificateBuf,
							 unsigned short* certificateBufSize, WON_CONST void* sendAfter,
							 unsigned short sendAfterSize, void* recvedAfterBuf,
							 unsigned short* recvedAfterBufSize, long timeout)
{
	if (!hWONAuth || !host)
		return Error_InvalidParams;

	WONAuth::Auth1Certificate remoteCertBuf;

	Error err = PeerAuthenticate(*(Identity*)hWONAuth, *host, &remoteCertBuf, sendAfter, 
		sendAfterSize, recvedAfterBuf, recvedAfterBufSize, timeout);

	if (certificateBufSize)
	{
		if (err == Error_Success && certificateBuf)
		{
			unsigned short maxSize = *certificateBufSize;
			if (maxSize)
			{
				const unsigned char* rawBuf = remoteCertBuf.GetRaw();
				unsigned short rawLen = remoteCertBuf.GetRawLen();
				*certificateBufSize = rawLen;
				if (maxSize >= rawLen)
					memcpy(certificateBuf, rawBuf, rawLen);
				return err;
			}
		}
		*certificateBufSize = 0;
	}
	return err;
}


class CAPIWONPeerAuthData
{
public:
	WONAuth::Auth1Certificate remoteCertBuf;
	void* certificateBuf;
	unsigned short* certificateBufSize;
	HWONCOMPLETION hCompletion;
};


static void TranslatePeerAuthErrorCompletion(Error err, CAPIWONPeerAuthData* peerAuthData)
{
	unsigned short rawLen = 0;
	if (peerAuthData->certificateBufSize)
	{
		if (err == Error_Success && peerAuthData->certificateBuf)
		{
			unsigned short maxSize = *(peerAuthData->certificateBufSize);
			if (maxSize)
			{
				const unsigned char* rawBuf = peerAuthData->remoteCertBuf.GetRaw();
				rawLen = peerAuthData->remoteCertBuf.GetRawLen();
				if (maxSize >= rawLen)
					memcpy(peerAuthData->certificateBuf, rawBuf, rawLen);
			}
		}
		*(peerAuthData->certificateBufSize) = rawLen;
	}

	WONComplete(peerAuthData->hCompletion, (void*)err);
	delete peerAuthData;
}


void WONPeerAuthAcceptAsync(void* certificateBuf, unsigned short* certificateBufSize,
							void* recvedAfterBuf, unsigned short* recvedAfterBufSize,
							long timeout, HWONCOMPLETION hCompletion)
{
	Error err = Error_OutOfMemory;
	CAPIWONPeerAuthData* peerAuthData = new CAPIWONPeerAuthData;
	if (peerAuthData)
	{
		peerAuthData->certificateBuf = certificateBuf;
		peerAuthData->certificateBufSize = certificateBufSize;
		peerAuthData->hCompletion = hCompletion;

		AcceptPeerAuthenticationEx(&(peerAuthData->remoteCertBuf), recvedAfterBuf, recvedAfterBufSize,
			timeout, true, TranslatePeerAuthErrorCompletion, peerAuthData);
		return;
	}
	WONComplete(hCompletion, (void*)err);
}


void WONPeerAuthenticateAsync(HWONAUTH hWONAuth, WON_CONST WONIPAddress* host, void* certificateBuf,
							  unsigned short* certificateBufSize, WON_CONST void* sendAfter,
							  unsigned short sendAfterSize, void* recvedAfterBuf,
							  unsigned short* recvedAfterBufSize, long timeout, HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (hWONAuth && host)
	{
		err = Error_OutOfMemory;
		CAPIWONPeerAuthData* peerAuthData = new CAPIWONPeerAuthData;
		if (peerAuthData)
		{
			peerAuthData->certificateBuf = certificateBuf;
			peerAuthData->certificateBufSize = certificateBufSize;
			peerAuthData->hCompletion = hCompletion;

			PeerAuthenticateEx(*(Identity*)hWONAuth, *host, &(peerAuthData->remoteCertBuf),
				sendAfter, sendAfterSize, recvedAfterBuf, recvedAfterBufSize, timeout, true,
				TranslatePeerAuthErrorCompletion, peerAuthData);
			return;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


/*
inline void FillBuffer(const void* buf, unsigned short size, void* fillTo, unsigned short* fillSize)
{
	if (fillSize && *fillSize)
	{
		unsigned short maxSize = *fillSize;
		*fillSize = 0;
		if (fillTo && buf && size)
		{
			*fillSize = size;
			if (maxSize >= size)
				memcpy(fillTo, buf, size);
		}
	}
}
*/

Error PeerAuthClientState::Init(Identity& ident, const void** req, unsigned short* reqSize)
{
	ident.Refresh();

	char cert[2048];
	unsigned short certSize = 2048;

	ident.GetCertificate(cert, &certSize);
		
	char privKey[2048];
	unsigned short privKeySize = 2048;

	ident.GetPrivateKey(privKey, &privKeySize);

	return Init(cert, certSize, privKey, privKeySize, req, reqSize, ident.GetExpirationDelta());
}


Error PeerAuthClientState::Init(const void* certificate, unsigned short certificateSize,
								const void* privKey, unsigned short privKeySize,		 
								const void** req, unsigned short* reqSize, time_t theAuthDelta)
{
	if (!certificate || !certificateSize || !req || !reqSize)
		return Error_InvalidParams;

	authDelta = theAuthDelta;

	privateKey.Create(privKeySize, (unsigned char*)privKey);
	ourCertificate = Auth1Certificate((unsigned char*)certificate, certificateSize);
	
	TMsgAuth1Request msg;

	msg.SetAuthMode(AUTH_PERSISTENT);
	msg.SetEncryptMode(ENCRYPT_NONE);
	msg.SetEncryptFlags(EFLAGS_NOTSEQUENCED | EFLAGS_ALLOWCLEAR);
	msg.SetRawBuf((unsigned char*)certificate, certificateSize);

	msg.Pack();
	buf.assign((unsigned char*)msg.GetDataPtr(), msg.GetDataLen());

	*req = (void*)buf.data();
	*reqSize = buf.size();

	return Error_Success;
}


Error PeerAuthServerState::RecvAuth1Request(Identity& ident, const void* req, unsigned short reqSize,
					   const void** challenge1, unsigned short* challenge1Size)
{
	ident.Refresh();

	char cert[2048];
	unsigned short certSize = 2048;

	ident.GetCertificate(cert, &certSize);
		
	char privKey[2048];
	unsigned short privKeySize = 2048;

	ident.GetPrivateKey(privKey, &privKeySize);

	return RecvAuth1Request(req, reqSize, cert, certSize, privKey, privKeySize, challenge1, challenge1Size, ident.GetExpirationDelta());
}


Error PeerAuthServerState::RecvAuth1Request(const void* req, unsigned short reqSize,
											const void* serverCertificate, unsigned short serverCertificateSize,
											const void* serverPrivateKey, unsigned short serverPrivateKeySize,
											const void** challenge1, unsigned short* challenge1Size, time_t theAuthDelta)
{
	Error err = Error_InvalidParameters;
	authDelta = theAuthDelta;
	if (!challenge1 || !challenge1Size // || !*challenge1Size
		|| !serverCertificate || !serverCertificateSize || !serverPrivateKey || !serverPrivateKeySize)
	;
	else
	{
		try
		{
			TMessage tmsg(reqSize, req);
			tmsg.Unpack();
			if (tmsg.GetMessageType() == Auth1Request)
			{
				err = Error_VerifyFailed;
				
				TMsgAuth1Request msg(tmsg);
				msg.Unpack();

				remoteCertificate = Auth1Certificate(msg.GetRawBuf(), msg.GetRawBufLen());

				Auth1PublicKeyBlock* pubKeys = Identity::GetPublicKeyBlock();
				if (pubKeys)
				{
					auto_ptr<Auth1PublicKeyBlock> deletePubKeys(pubKeys);

					if (!remoteCertificate.IsExpired(authDelta))
					{
						if (pubKeys->VerifyFamilyBuffer(remoteCertificate))
						{
							privateKey.Create(serverPrivateKeySize, (unsigned char*)serverPrivateKey);
							sessionKey.Create(SessionKeySize);

							RawBuffer secret;
							unsigned short keyLen = sessionKey.GetKeyLen();
							unsigned short tmpKeyLen = getLittleEndian(keyLen);
							secret.append((unsigned char*)&tmpKeyLen, sizeof(unsigned short));
							secret.append(sessionKey.GetKey(), keyLen);

							EGPublicKey::CryptReturn block(remoteCertificate.GetPubKey().Encrypt(secret.data(), secret.size()));
							if (block.first)
							{
								auto_ptr<unsigned char> autoCryptReturn(block.first);

								TMsgAuth1Challenge1 challenge;
								challenge.SetSecretB(block.first, block.second);
								challenge.SetRawBuf((unsigned char*)serverCertificate, serverCertificateSize);

								challenge.Pack();
								buf.assign((unsigned char*)challenge.GetDataPtr(), challenge.GetDataLen());

								*challenge1 = buf.data();
								*challenge1Size = buf.size();

								return Error_Success;
							}
				//			else
				//				err = Error_VerifyFailed;
						}
				//		else
				//			err = Error_VerifyFailed;//Error_InvalidRemoteCertificate;
					}
				//	else
				//		err = Error_VerifyFailed;//Error_InvalidRemoteCertificate;
				}
				//else
				//	err = Error_VerifyFailed;//Error_InvalidPubKeys;
			}
		}
		catch (...)
		{
		}
	}
	TMsgAuth1Complete completeMsg;
	completeMsg.SetStatus((ServerStatus)err);
	completeMsg.Pack();
	buf.assign((unsigned char*)completeMsg.GetDataPtr(), completeMsg.GetDataLen());
	*challenge1 = buf.data();
	*challenge1Size = buf.size();

	return err;
}
	
	
	// stage 3, client recvs challenge #1, and must send challenge #2 to server
Error PeerAuthClientState::RecvChallenge1(const void* challenge1, unsigned short challenge1Size,
										  const void** challenge2, unsigned short* challenge2Size)
{
	if (!challenge1 || !challenge1Size || !challenge2 || !challenge2Size)
		return Error_InvalidParams;
	try
	{
		TMessage tmsg(challenge1Size, challenge1);
		tmsg.Unpack();

		if (tmsg.GetMessageType() == Auth1Complete)
		{
			TMsgAuth1Complete completeMsg(tmsg);
			completeMsg.Unpack();
			return completeMsg.GetStatus();
		}
		if (tmsg.GetMessageType() != Auth1Challenge1)
		{
			return Error_InvalidParams;
		}

		TMsgAuth1Challenge1 challenge(tmsg);
		challenge.Unpack();

		unsigned short secretSize = challenge.GetSecretBLen();
		const unsigned char* secretStart = challenge.GetSecretB();
		unsigned short certificiateSize = challenge.GetRawBufLen();
		const unsigned char* certificateStart = challenge.GetRawBuf();
		
		CryptKeyBase::CryptReturn cryptReturn = privateKey.Decrypt(secretStart, secretSize);
		if (!cryptReturn.first)
			return Error_InvalidSecret;
		
		auto_ptr<unsigned char> deleteTheCryptReturn(cryptReturn.first);
		
		if (cryptReturn.second - sizeof(unsigned short) != getLittleEndian(*((unsigned short*)cryptReturn.first)))
			return Error_InvalidSecret;

//		BFSymmetricKey sessionKey;
		sessionKey.Create(getLittleEndian(*((unsigned short*)cryptReturn.first)), cryptReturn.first + 2);

		serverCertificate = Auth1Certificate(certificateStart, certificiateSize);

		Auth1PublicKeyBlock* pubKeys = Identity::GetPublicKeyBlock();
		if (!pubKeys)
			return Error_InvalidPubKeys;
		auto_ptr<Auth1PublicKeyBlock> deletePubKeys(pubKeys);

		if (serverCertificate.IsExpired(authDelta))
			return Error_InvalidRemoteCertificate;

		if (!pubKeys->VerifyFamilyBuffer(serverCertificate))
			return Error_InvalidRemoteCertificate;

		clientSecret.Create(SessionKeySize);

		RawBuffer result;
		// format of result;
		// <unsigned short> size of session key
		// <blob> session key
		// <blob> client's secret

		unsigned short sessionKeySize = sessionKey.GetKeyLen();
		unsigned short tmpSessionKeySize = getLittleEndian(sessionKeySize);
		result.assign((unsigned char*)&tmpSessionKeySize, sizeof(unsigned short));
		result.append(sessionKey.GetKey(), sessionKeySize);
		result.append(clientSecret.GetKey(), clientSecret.GetKeyLen());

		CryptKeyBase::CryptReturn aSecretCrypt = serverCertificate.GetPubKey().Encrypt(result.data(), result.size());
		if (!aSecretCrypt.first)
			return Error_EncryptFailed;

		auto_ptr<unsigned char> deleteSecretCrypt(aSecretCrypt.first);

		TMsgAuth1Challenge2 challenge2Msg;
		challenge2Msg.SetRawBuf(aSecretCrypt.first, aSecretCrypt.second, true);

		challenge2Msg.Pack();

		buf.assign((unsigned char*)challenge2Msg.GetDataPtr(), challenge2Msg.GetDataLen());
	//	buf.assign(aSecretCrypt.first, aSecretCrypt.second);
		*challenge2Size = buf.size();
		*challenge2 = buf.data();
	//	FillBuffer(aSecretCrypt.first, aSecretCrypt.second, challenge2, challenge2Size);

		return Error_Success;
	}
	catch (...)
	{
	}
	return Error_InvalidParams;
}


Error PeerAuthServerState::RecvChallenge2(const void* challenge2, unsigned short challenge2Size,
										  const void** result, unsigned short* resultSize)
{
	Error err = Error_InvalidParameters;
	if (!challenge2 || !challenge2Size || !result || !resultSize)
		;
	else
	{
		try
		{
			TMessage tmsg(challenge2Size, challenge2);
			tmsg.Unpack();
			if (tmsg.GetMessageType() == Auth1Challenge2)
			{
				err = Error_VerifyFailed;

				TMsgAuth1Challenge2 challenge2Msg(tmsg);
				challenge2Msg.Unpack();

				CryptKeyBase::CryptReturn decryptResult = privateKey.Decrypt((unsigned char*)challenge2Msg.GetRawBuf(), challenge2Msg.GetRawBufLen());
				if (decryptResult.first)
				{
					auto_ptr<unsigned char> autoDecryptResult(decryptResult.first);

					unsigned short sessionKeySize = getLittleEndian(*(unsigned short*)decryptResult.first);
					unsigned char* sessionKeyStart = (unsigned char*)(decryptResult.first) + sizeof(unsigned short);
					unsigned short clientSecretSize = decryptResult.second - (sessionKeySize + sizeof(unsigned short));
					unsigned char* clientSecretStart = sessionKeyStart + (sessionKeySize);

					BFSymmetricKey newSessionKey(sessionKeySize, sessionKeyStart);
					if (sessionKey == newSessionKey)
					{
						BFSymmetricKey clientSecret;
						clientSecret.Create(clientSecretSize, clientSecretStart);

						RawBuffer tmp;
						unsigned short blocksize = clientSecretSize;
						makeLittleEndian(blocksize);
						tmp.assign((unsigned char*)&blocksize, sizeof(unsigned short));
						tmp.append(clientSecretStart, clientSecretSize);

						EGPublicKey::CryptReturn block(remoteCertificate.GetPubKey().Encrypt(tmp.data(), tmp.size()));
						if (block.first)
						{
							auto_ptr<unsigned char> autoBlock(block.first);

							TMsgAuth1Complete completeMsg;

							completeMsg.SetRawBuf(block.first, block.second);
							completeMsg.SetStatus((ServerStatus)Error_Success);
							completeMsg.Pack();

							buf.assign((unsigned char*)completeMsg.GetDataPtr(), completeMsg.GetDataLen());
						//	buf.assign(block.first, block.second);
							*resultSize = buf.size();
							*result = buf.data();
							return Error_Success;
						}
				//		else
				//			err = Error_EncryptFailed;
					}	
				//	else
				//		err = Error_InvalidSecret;
				}
				//else
				//	err = Error_InvalidSecret;
			}
		}
		catch (...)
		{
		}
	}
	TMsgAuth1Complete completeMsg;
	completeMsg.SetStatus((ServerStatus)err);
	completeMsg.Pack();
	buf.assign((unsigned char*)completeMsg.GetDataPtr(), completeMsg.GetDataLen());
	*result = buf.data();
	*resultSize = buf.size();

	return err;
}


Error PeerAuthClientState::RecvResult(const void* result, unsigned short resultSize)
{
	if (!result || !resultSize)
		return Error_InvalidParams;

	TMessage tmsg(resultSize, result);
	tmsg.Unpack();
	if (tmsg.GetMessageType() != Auth1Complete)
		return Error_InvalidParams;
	
	TMsgAuth1Complete msg(tmsg);
	msg.Unpack();
	if (msg.GetStatus() != Error_Success)
		return msg.GetStatus();

	CryptKeyBase::CryptReturn decryptResult = privateKey.Decrypt((unsigned char*)msg.GetRawBuf(), msg.GetRawBufLen());
	if (!decryptResult.first)
		return Error_InvalidSecret;

	auto_ptr<unsigned char> autoDecryptResult(decryptResult.first);

	if (getLittleEndian(*(unsigned short*)decryptResult.first) != decryptResult.second-2)
		return Error_InvalidParams;

	BFSymmetricKey newSecret(getLittleEndian(*(unsigned short*)decryptResult.first), ((unsigned char*)decryptResult.first)+2);

	if (clientSecret == newSecret)
		return Error_Success;
	return Error_InvalidSecret;
}


const WONCrypt::BFSymmetricKey& PeerAuthClientState::GetSessionKey() const
{
	return sessionKey;
}


void PeerAuthClientState::GetSessionKey(void* keyBuf, unsigned short* keyBufSize) const
{
	if (keyBufSize)
	{
		unsigned short keySize = sessionKey.GetKeyLen();
		if (keySize <= *keyBufSize)
			memcpy(keyBuf, sessionKey.GetKey(), keySize);
		*keyBufSize = keySize;
	}
}


const WONCrypt::BFSymmetricKey& PeerAuthServerState::GetSessionKey() const
{
	return sessionKey;
}


void PeerAuthServerState::GetSessionKey(void* keyBuf, unsigned short* keyBufSize) const
{
	if (keyBufSize)
	{
		unsigned short keySize = sessionKey.GetKeyLen();
		if (keySize <= *keyBufSize)
			memcpy(keyBuf, sessionKey.GetKey(), keySize);
		*keyBufSize = keySize;
	}
}


HWONPEERAUTHCLIENT WONPeerAuthClientStateCreate()
{
	return (HWONPEERAUTHCLIENT)new PeerAuthClientState;
}


HWONPEERAUTHSERVER WONPeerAuthServerStateCreate()
{
	return (HWONPEERAUTHSERVER)new PeerAuthServerState;
}


WONError WONPeerAuthClientInit(HWONPEERAUTHCLIENT hState, HWONAUTH hWONAuth, WON_CONST void** req, unsigned short* reqSize)
{
	Identity* ident = (Identity*)hWONAuth;
	if (!ident)
		return Error_InvalidParams;
	return ((PeerAuthClientState*)hState)->Init(*ident, (const void**)req, reqSize);
}


WONError WONPeerAuthClientInitEx(HWONPEERAUTHCLIENT hState, WON_CONST void* certificate, unsigned short certificateSize,
							   WON_CONST void* privKey, unsigned short privKeySize, WON_CONST void** req, unsigned short* reqSize, time_t theAuthDelta)
{
	return ((PeerAuthClientState*)hState)->Init(certificate, certificateSize, privKey, privKeySize, (const void**)req, reqSize, theAuthDelta);
}


WONError WONPeerAuthServerRecvAuth1Request(HWONPEERAUTHSERVER hState, WON_CONST void* req, unsigned short reqSize,
										   HWONAUTH hWONAuth, WON_CONST void** challenge1, unsigned short* challenge1Size)
{
	Identity* ident = (Identity*)hWONAuth;
	if (!ident)
		return Error_InvalidParams;
	return ((PeerAuthServerState*)hState)->RecvAuth1Request(*ident, req, reqSize, (const void**)challenge1, challenge1Size);
}



WONError WONPeerAuthServerRecvAuth1RequestEx(HWONPEERAUTHSERVER hState, WON_CONST void* req, unsigned short reqSize,
											 WON_CONST void* serverCertificate, unsigned short serverCertificateSize,
											 WON_CONST void* serverPrivateKey, unsigned short serverPrivateKeySize,
											 WON_CONST void** challenge1, unsigned short* challenge1Size, time_t theAuthDelta)
{
	return ((PeerAuthServerState*)hState)->RecvAuth1Request(req, reqSize, serverCertificate, serverCertificateSize,
		serverPrivateKey, serverPrivateKeySize, (const void**)challenge1, challenge1Size, theAuthDelta);
}


WONError WONPeerAuthClientRecvChallenge1(HWONPEERAUTHCLIENT hState, WON_CONST void* challenge1, unsigned short challenge1Size,
										 WON_CONST void** challenge2, unsigned short* challenge2Size)
{
	return ((PeerAuthClientState*)hState)->RecvChallenge1(challenge1, challenge1Size, (const void**)challenge2, challenge2Size);
}


WONError WONPeerAuthServerRecvChallenge2(HWONPEERAUTHSERVER hState, WON_CONST void* challenge2, unsigned short challenge2Size,
										 WON_CONST void** result, unsigned short* resultSize)
{
	return ((PeerAuthServerState*)hState)->RecvChallenge2(challenge2, challenge2Size, (const void**)result, resultSize);
}


WONError WONPeerAuthClientRecvResult(HWONPEERAUTHCLIENT hState, WON_CONST void* result, unsigned short resultSize)
{
	return ((PeerAuthClientState*)hState)->RecvResult(result, resultSize);
}


void WONPeerAuthServerGetSessionKey(HWONPEERAUTHSERVER hState, void* keyBuf, unsigned short* keyBufSize)
{
	((PeerAuthServerState*)hState)->GetSessionKey(keyBuf, keyBufSize);
}


void WONPeerAuthClientGetSessionKey(HWONPEERAUTHCLIENT hState, void* keyBuf, unsigned short* keyBufSize)
{
	((PeerAuthClientState*)hState)->GetSessionKey(keyBuf, keyBufSize);
}


void WONPeerAuthClientStateCloseHandle(HWONPEERAUTHCLIENT hState)
{
	delete (PeerAuthClientState*)hState;
}


void WONPeerAuthServerStateCloseHandle(HWONPEERAUTHSERVER hState)
{
	delete (PeerAuthServerState*)hState;
}

