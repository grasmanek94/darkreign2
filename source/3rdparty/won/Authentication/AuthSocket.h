
#ifndef __WON_AUTHENTICATION_TITANSOCKET_H__
#define __WON_AUTHENTICATION_TITANSOCKET_H__

#pragma warning(disable : 4786)


#include "common/won.h"
#include "Socket/WONWS.h"
#include "Socket/TMsgSocket.h"
#include "Socket/UDPSocket.h"
#include "Socket/TCPSocket.h"
#include "auth/Auth1Certificate.h"
#include "crypt/BFSymmetricKey.h"
#include "Identity.h"


namespace WONAPI {


class AuthSocket : public TMsgSocket
{
protected:
	// Hidden functions of TMsgSocket...  Use the AuthSocket versions instead (just one more bool).
	virtual Error Open(long timeout = -1, bool async = false, const CompletionContainer<const OpenResult&>& completion = DEFAULT_COMPLETION );

	virtual unsigned char* RecvRawMsg(unsigned char** recvMsgBuf, unsigned long* length, long timeout, bool async = false, const CompletionContainer<const TMsgSocket::RecvRawMsgResult&>& completion = DEFAULT_COMPLETION );
	virtual WONMsg::BaseMessage* RecvBaseMsg(WONMsg::BaseMessage** recvMsg = NULL, long timeout = -1, bool async = false, const CompletionContainer<const TMsgSocket::RecvBaseMsgResult&>& completion = DEFAULT_COMPLETION );

	virtual bool SendRawMsg(unsigned long length, const void* msg, long timeout, bool async = false, bool copyData = false, const CompletionContainer<const TMsgSocket::SendRawMsgResult&>& completion = DEFAULT_COMPLETION );
	virtual bool SendBaseMsg(WONMsg::BaseMessage& msg, long timeout, bool async = false, bool copyData = false, const CompletionContainer<const TMsgSocket::SendBaseMsgResult&>& completion = DEFAULT_COMPLETION );

	virtual unsigned char* SendRawMsgRequest(unsigned long length, const void* req, unsigned char** replyBuf, unsigned long* replyLength = 0, unsigned long retryTimeout = 2000, long timeout = -1, bool async = false, bool copyData = false, const CompletionContainer<const TMsgSocket::RecvRawMsgResult&>& completion = DEFAULT_COMPLETION );
	virtual WONMsg::BaseMessage* SendBaseMsgRequest(WONMsg::BaseMessage& req, WONMsg::BaseMessage** reply = 0, unsigned long retryTimeout = 2000, long timeout= -1, bool async = false, bool copyData = false, const CompletionContainer<const TMsgSocket::RecvBaseMsgResult&>& completion = DEFAULT_COMPLETION );

	virtual unsigned char* ReliableRecvRawMsg(unsigned short* tagNum, unsigned char** recvMsgBuf, unsigned long* length, long timeout = -1, bool async = false, const CompletionContainer<const TMsgSocket::ReliableRecvRawMsgResult&>& completion = DEFAULT_COMPLETION );
	virtual WONMsg::BaseMessage* ReliableRecvBaseMsg(unsigned short* tagNum, WONMsg::BaseMessage** recvMsg = NULL, long timeout = -1, bool async = false, const CompletionContainer<const TMsgSocket::ReliableRecvBaseMsgResult&>& completion = DEFAULT_COMPLETION );

	virtual bool ReliableSendRawMsgReply(unsigned short tagNum, unsigned long length, const void* msg, long timeout = -1, bool async = false, bool copyData = false, const CompletionContainer<const TMsgSocket::SendRawMsgResult&>& completion = DEFAULT_COMPLETION );
	virtual bool ReliableSendBaseMsgReply(unsigned short tagNum, WONMsg::BaseMessage& msg, long timeout = -1, bool async = false, bool copyData = false, const CompletionContainer<const TMsgSocket::SendBaseMsgResult&>& completion = DEFAULT_COMPLETION );

public:

	// If useAuthSession == true, uses light-weight session-based authentication.
	// If useAuthSession == false, uses persistent authentication (only valid over single persistent connectoin)

	// If beAuthHost == true, will accept authentication initiated by remote
 
	// useClientSessionKey is currently ignored, but will ultimately be used to 
	// determine who's secret to use as session key.

	// authSessionLife == timeout (in seconds) of unused accepted lightweight sessions

	AuthSocket(TCPSocket& sock, unsigned char lngSize = 4, bool useAuthSession = true,
		bool encrypted = true, bool allowUnencryptedMsgs = false, bool sequed = true,
		bool beAuthHost = false, unsigned long authSessionLife = 1800, bool useClientSessionKey = false);

	AuthSocket(TCPSocket* sock, unsigned char lngSize = 4, bool useAuthSession = true,
		bool encrypted = true, bool allowUnencryptedMsgs = false, bool sequed = true,
		bool beAuthHost = false, unsigned long authSessionLife = 1800, bool useClientSessionKey = false);

	AuthSocket(TCPSocket* sock, bool takeOwnership, unsigned char lengthSize = 4,
		bool useAuthSession = true, bool encrypted = true, bool allowUnencryptedMsgs = false,
		bool sequed = true, bool beAuthHost = false, unsigned long authSessionLife = 1800,
		bool useClientSessionKey = false);

	AuthSocket(UDPSocket& sock, const IPSocket::Address& refreshTCPAddr, unsigned char refreshLngSize = 4,
		bool encrypted = true, bool allowUnencryptedMsgs = false, bool sequed = false,
		bool beAuthHost = false, unsigned long authSessionLife = 1800, bool useClientSessionKey = false,
		size_t largestDatagram = 0x0000FFFF);

	AuthSocket(UDPSocket* sock, const IPSocket::Address& refreshTCPAddr, unsigned char refreshLngSize = 4,
		bool encrypted = true, bool allowUnencryptedMsgs = false, bool sequed = false,
		bool beAuthHost = false, unsigned long authSessionLife = 1800, bool useClientSessionKey = false,
		size_t largestDatagram = 0x0000FFFF);

	AuthSocket(UDPSocket* sock, bool takeOwnership, const IPSocket::Address& refreshTCPAddr,
		unsigned char refreshLngSize = 4, bool encrypted = false, bool allowUnencryptedMsgs = false,
		bool sequed = true, bool beAuthHost = false, unsigned long authSessionLife = 1800,
		bool useClientSessionKey = false, size_t largestDatagram = 0x0000FFFF);

	AuthSocket(Identity* ident, TCPSocket& sock, unsigned char lngSize = 4, bool useAuthSession = true,
		bool encrypted = true, bool allowUnencryptedMsgs = false, bool sequed = true,
		bool beAuthHost = false, unsigned long authSessionLife = 1800, bool useClientSessionKey = false);

	AuthSocket(Identity* ident, TCPSocket* sock, unsigned char lngSize = 4, bool useAuthSession = true,
		bool encrypted = true, bool allowUnencryptedMsgs = false, bool sequed = true,
		bool beAuthHost = false, unsigned long authSessionLife = 1800, bool useClientSessionKey = false);

	AuthSocket(Identity* ident, TCPSocket* sock, bool takeOwnership, unsigned char lengthSize = 4,
		bool useAuthSession = true, bool encrypted = true, bool allowUnencryptedMsgs = false,
		bool sequed = true, bool beAuthHost = false, unsigned long authSessionLife = 1800,
		bool useClientSessionKey = false);

	AuthSocket(Identity* ident, UDPSocket& sock, const IPSocket::Address& refreshTCPAddr,
		unsigned char refreshLngSize = 4, bool encrypted = true, bool allowUnencryptedMsgs = false,
		bool sequed = false, bool beAuthHost = false, unsigned long authSessionLife = 1800,
		bool useClientSessionKey = false, size_t largestDatagram = 0x0000FFFF);

	AuthSocket(Identity* ident, UDPSocket* sock, const IPSocket::Address& refreshTCPAddr,
		unsigned char refreshLngSize = 4, bool encrypted = true, bool allowUnencryptedMsgs = false,
		bool sequed = false, bool beAuthHost = false, unsigned long authSessionLife = 1800,
		bool useClientSessionKey = false, size_t largestDatagram = 0x0000FFFF);

	AuthSocket(Identity* ident, UDPSocket* sock, bool takeOwnership, const IPSocket::Address& refreshTCPAddr,
		unsigned char refreshLngSize = 4, bool encrypted = true, bool allowUnencryptedMsgs = false,
		bool sequed = false, bool beAuthHost = false, unsigned long authSessionLife = 1800,
		bool useClientSessionKey = false, size_t largestDatagram = 0x0000FFFF);

	// Only use to copy non-sequenced sessions
	AuthSocket(TCPSocket& sock, const AuthSocket& toCopy);
	AuthSocket(TCPSocket* sock, const AuthSocket& toCopy);
	AuthSocket(TCPSocket* sock, bool takeOwnership, const AuthSocket& toCopy);

	void SetRefreshAddress(const IPSocket::Address& refreshTCPAddr);

	void SetIdentity(Identity* ident);
	Identity* GetIdentity();	// Do not delete.  Owned by AuthSocket object
	

	Error Open(bool authenticate, long timeout = -1, bool async = false, const CompletionContainer<const OpenResult&>& completion = DEFAULT_COMPLETION );

	template <class privsType>
	Error OpenEx(bool authenticate, long timeout, bool async, void (*f)(const OpenResult&, privsType), privsType t)
	{ return Open(authenticate, timeout, async, new CompletionWithContents<const OpenResult&, privsType>(t, f, true)); }


	Error Open(Identity* ident, bool authenticate, long timeout = -1, bool async = false, const CompletionContainer<const OpenResult&>& completion = DEFAULT_COMPLETION )
	{ SetIdentity(ident); return Open(authenticate, timeout, async, completion); }

	template <class privsType>
	Error OpenEx(Identity* ident, bool authenticate, long timeout, bool async, void (*f)(const OpenResult&, privsType), privsType t)
	{ SetIdentity(ident); return Open(authenticate, timeout, async, new CompletionWithContents<const OpenResult&, privsType>(t, f, true)); }


	Error Open(Identity& ident, long timeout = -1, bool async = false, const CompletionContainer<const OpenResult&>& completion = DEFAULT_COMPLETION )
	{ SetIdentity(&ident); return Open(true, timeout, async, completion); }

	template <class privsType>
	Error OpenEx(Identity& ident, long timeout, bool async, void (*f)(const OpenResult&, privsType), privsType t)
	{ SetIdentity(&ident); return Open(true, timeout, async, new CompletionWithContents<const OpenResult&, privsType>(t, f, true)); }


	virtual ~AuthSocket();

	class AuthResult
	{
	public:
		Error error;
		AuthSocket* theSocket;

		AuthResult(AuthSocket* sock, Error err)
			:	error(err),
				theSocket(sock)
		{ }
	};


	// Warning : When authenticating over TCP, and socket is already be open, there must not be
	// any messages pending, or any unexpected messages recv'ed.  If TCP socket is not already open,
	// it will be opened, and left open after authentication.

	Error Authenticate(bool force = false, long timeout = -1, bool async = false, const CompletionContainer<const AuthResult&>& completion = DEFAULT_COMPLETION );
	
	template <class privsType>
	Error AuthenticateEx(bool force, long timeout, bool async, void (*f)(const AuthResult&, privsType), privsType t)
	{ return Authenticate(force, timeout, async, new CompletionWithContents<const AuthResult&, privsType>(f, t, true)); }


	Error Authenticate(Identity* ident, bool force = false, long timeout = -1, bool async = false, const CompletionContainer<const AuthResult&>& completion = DEFAULT_COMPLETION )
	{ SetIdentity(ident); return Authenticate(force, timeout, async, completion); }
	
	template <class privsType>
	Error AuthenticateEx(Identity* ident, bool force, long timeout, bool async, void (*f)(const AuthResult&, privsType), privsType t)
	{ SetIdentity(ident); return Authenticate(force, timeout, async, new CompletionWithContents<const AuthResult&, privsType>(f, t, true)); }
	


	Error Authenticate(Identity& ident, bool force = false, long timeout = -1, bool async = false, const CompletionContainer<const AuthResult&>& completion = DEFAULT_COMPLETION )
	{ SetIdentity(&ident); return Authenticate(force, timeout, async, completion); }
	
	template <class privsType>
	Error AuthenticateEx(Identity& ident, bool force, long timeout, bool async, void (*f)(const AuthResult&, privsType), privsType t)
	{ SetIdentity(&ident); return Authenticate(force, timeout, async, new CompletionWithContents<const AuthResult&, privsType>(f, t, true)); }
	


	class AuthRecvRawMsgResult : public TMsgSocket::RecvRawMsgResult
	{
	public:
		bool wasEncrypted;
		bool authExpired;

		AuthRecvRawMsgResult(TMsgSocket* sock, unsigned char* buf, unsigned long msgLen, bool encrypted, bool authExp = false, bool socketClosed = false)
			:	TMsgSocket::RecvRawMsgResult(sock, buf, msgLen, socketClosed), wasEncrypted(encrypted), authExpired(authExp)
		{}
	};

	
	class AuthRecvBaseMsgResult : public TMsgSocket::RecvBaseMsgResult
	{
	public:
		bool wasEncrypted;
		bool authExpired;

		AuthRecvBaseMsgResult(TMsgSocket* sock, WONMsg::BaseMessage* theMsg, bool encrypted, bool authExp = false, bool socketClosed = false)
			:	TMsgSocket::RecvBaseMsgResult(sock, theMsg, socketClosed), wasEncrypted(encrypted), authExpired(authExp)
		{ }
	};

	class AuthReliableRecvRawMsgResult : public TMsgSocket::ReliableRecvRawMsgResult
	{
	public:
		bool wasEncrypted;
		bool authExpired;

		AuthReliableRecvRawMsgResult(unsigned short tag, TMsgSocket* sock, unsigned char* buf, unsigned long msgLen, bool encrypted, bool authExp = false, bool socketClosed = false)
			:	TMsgSocket::ReliableRecvRawMsgResult(tag, sock, buf, msgLen, socketClosed), wasEncrypted(encrypted), authExpired(authExp)
		{}
		
		AuthReliableRecvRawMsgResult(unsigned short tag, const AuthRecvRawMsgResult& toCopy)
			:	TMsgSocket::ReliableRecvRawMsgResult(tag, toCopy), wasEncrypted(toCopy.wasEncrypted), authExpired(toCopy.authExpired)
		{}
	};

	
	class AuthReliableRecvBaseMsgResult : public TMsgSocket::ReliableRecvBaseMsgResult
	{
	public:
		bool wasEncrypted;
		bool authExpired;

		AuthReliableRecvBaseMsgResult(unsigned short tag, TMsgSocket* sock, WONMsg::BaseMessage* theMsg, bool encrypted, bool authExp = false, bool socketClosed = false)
			:	TMsgSocket::ReliableRecvBaseMsgResult(tag, sock, theMsg, socketClosed), wasEncrypted(encrypted), authExpired(authExp)
		{ }
	};

	class AuthSendRawMsgResult : public TMsgSocket::SendRawMsgResult
	{
	public:
		bool wasEncrypted;

		AuthSendRawMsgResult(TMsgSocket* sock, unsigned char* buf, unsigned long msgLen, bool s, bool encrypted, bool socketClosed = false)
			:	TMsgSocket::SendRawMsgResult(sock, buf, msgLen, s, socketClosed), wasEncrypted(encrypted)
		{}
	};


	class AuthSendBaseMsgResult : public TMsgSocket::SendBaseMsgResult
	{
	public:
		bool wasEncrypted;

		AuthSendBaseMsgResult(TMsgSocket* sock, WONMsg::BaseMessage* theMsg, bool s, bool encrypted, bool socketClosed = false)
			:	TMsgSocket::SendBaseMsgResult(sock, theMsg, s, socketClosed), wasEncrypted(encrypted)
		{ }
	};
	

	// RecvRawMsg - Recv raw titan message

	virtual unsigned char* RecvRawMsg(unsigned char** recvMsgBuf, unsigned long* length = 0, bool allowUnencrypted = true, long timeout= -1, bool async = false, const CompletionContainer<const AuthRecvRawMsgResult&>& completion = DEFAULT_COMPLETION );
	
	template <class privsType>
	unsigned char* RecvRawMsgEx(unsigned char** recvMsgBuf, unsigned long* length, bool allowUnencrypted, long timeout, bool async, void (*f)(const AuthRecvRawMsgResult&, privsType), privsType t)
	{ return RecvRawMsg(recvMsgBuf, length, allowUnencrypted, timeout, async, new CompletionWithContents<const AuthRecvRawMsgResult&, privsType>(t, f, true)); }
	
	
	
	// RecvBaseMsg - Recv titan WONMsg::BaseMessage

	virtual WONMsg::BaseMessage* RecvBaseMsg(WONMsg::BaseMessage** recvMsg, bool allowUnencrypted = true, long timeout = -1, bool async = false, const CompletionContainer<const AuthRecvBaseMsgResult&>& completion = DEFAULT_COMPLETION );
	
	template <class privsType>
	WONMsg::BaseMessage* RecvBaseMsgEx(WONMsg::BaseMessage** recvMsg, bool allowUnencrypted, long timeout, bool async, void (*f)(const AuthRecvBaseMsgResult&, privsType), privsType t)
	{ return RecvBaseMsg(recvMsg, allowUnencrypted, timeout, async, new CompletionWithContents<const AuthRecvBaseMsgResult&, privsType>(t, f, true)); }


	// SendRawMsg - Send raw titan message
	
	virtual bool SendRawMsg(unsigned long length, const void* msg, bool encrypt = true, long timeout = -1, bool async = false, bool copyData = false, const CompletionContainer<const AuthSendRawMsgResult&>& completion = DEFAULT_COMPLETION );
	
	template <class privsType>
	bool SendRawMsgEx(unsigned long length, const void* msg, bool encrypt, long timeout, bool async, bool copyData, void (*f)(const AuthSendRawMsgResult&, privsType), privsType t)
	{ return SendRawMsg(length, msg, encrypt, timeout, async, copyData, new CompletionWithContents<const AuthSendRawMsgResult&, privsType>(t, f, true)); }


	// SendBaseMsg - Send titan WONMsg::BaseMessage

	virtual bool SendBaseMsg(WONMsg::BaseMessage& msg, bool encrypt = true, long timeout = -1, bool async = false, bool copyData = false, const CompletionContainer<const AuthSendBaseMsgResult&>& completion = DEFAULT_COMPLETION );

	template <class privsType>
	bool SendBaseMsgEx(WONMsg::BaseMessage& msg, bool encrypt, long timeout, bool async, bool copyData, void (*f)(const AuthSendBaseMsgResult&, privsType), privsType t)
	{ return SendBaseMsg(msg, encrypt, timeout, async, copyData, new CompletionWithContents<const AuthSendBaseMsgResult&, privsType>(t, f, true)); }


	// SendRawMsgRequest - Send raw titan message, and recv reply.  Does reliable UDP

	virtual unsigned char* SendRawMsgRequest(unsigned long length, const void* req, unsigned char** replyBuf, unsigned long* replyLength = 0, bool encrypt = true, bool allowUnencrypted = true, unsigned long retryTimeout = 2000, long timeout = -1, bool async = false, bool copyData = false, const CompletionContainer<const AuthRecvRawMsgResult&>& completion = DEFAULT_COMPLETION );

	template <class privsType>
	unsigned char* SendRawMsgRequestEx(unsigned long length, const void* req, unsigned char** replyBuf, unsigned long* replyLength, bool encrypt, bool allowUnencrypted, unsigned long retryTimeout, long timeout, bool async, bool copyData, void (*f)(const AuthRecvRawMsgResult&, privsType), privsType t)
	{ return SendRawMsgRequest(length, req, replyBuf, replyLength, encrypt, allowUnencrypted, retryTimeout, timeout, async, copyData, new CompletionWithContents<const AuthRecvRawMsgResult&, privsType>(t, f, true)); }



	// SendBaseMsgRequest - Send titan WONMsg::BaseMessage, and recv reply.  Does reliable UDP

	virtual WONMsg::BaseMessage* SendBaseMsgRequest(WONMsg::BaseMessage& req, WONMsg::BaseMessage** reply = 0, bool encrypt = true, bool allowUnencrypted = true, unsigned long retryTimeout = 2000, long timeout= -1, bool async = false, bool copyData = false, const CompletionContainer<const AuthRecvBaseMsgResult&>& completion = DEFAULT_COMPLETION );

	template <class privsType>
	WONMsg::BaseMessage* SendBaseMsgRequestEx(WONMsg::BaseMessage& req, WONMsg::BaseMessage** reply, bool encrypt, bool allowUnencrypted, unsigned long retryTimeout, long timeout, bool async, bool copyData, void (*f)(const AuthRecvBaseMsgResult&, privsType), privsType t)
	{ return SendBaseMsgRequest(req, reply, encrypt, allowUnencrypted, retryTimeout, timeout, async, copyData, new CompletionWithContents<const AuthRecvBaseMsgResult&, privsType>(t, f, true)); }


	// ReliableRecvMsg - Similar to RecvMsg(), but uses server side of reliable UDP protocol.
	// If tagNum is zero, the incoming message did not include a tag value
	// In order to support reliable UDP, you must send the reply with ReliableSendRawMsgReply() or
	// ReliableSendBaseMsgReply(), and pass in the recv'ed tagNum.

	virtual unsigned char* ReliableRecvRawMsg(unsigned short* tagNum, unsigned char** recvMsgBuf, unsigned long* length, bool allowUnencrypted = true, long timeout = -1, bool async = false, const CompletionContainer<const AuthReliableRecvRawMsgResult&>& completion = DEFAULT_COMPLETION );
	
	template <class privsType>
	unsigned char* ReliableRecvRawMsgEx(unsigned short* tagNum, unsigned char** recvMsgBuf, unsigned long* length, bool allowUnencrypted, long timeout, bool async, void (*f)(const AuthReliableRecvRawMsgResult&, privsType), privsType t)
	{ return ReliableRecvRawMsg(tagNum, recvMsgBuf, length, allowUnencrypted, timeout, async, new CompletionWithContents<const AuthReliableRecvRawMsgResult&, privsType>(t, f, true)); }


	virtual WONMsg::BaseMessage* ReliableRecvBaseMsg(unsigned short* tagNum, WONMsg::BaseMessage** recvMsg = NULL, bool allowUnencrypted = true, long timeout = -1, bool async = false, const CompletionContainer<const AuthReliableRecvBaseMsgResult&>& completion = DEFAULT_COMPLETION );
	
	template <class privsType>
	WONMsg::BaseMessage* ReliableRecvBaseMsgEx(unsigned short* tagNum, WONMsg::BaseMessage** recvMsg, bool allowUnencrypted, long timeout, bool async, void (*f)(const AuthReliableRecvBaseMsgResult&, privsType), privsType t)
	{ return ReliableRecvBaseMsg(tagNum, recvMsg, allowUnencrypted, timeout, async, new CompletionWithContents<const AuthReliableRecvBaseMsgResult&, privsType>(t, f, true)); }


	// SendRawMsg - send a Titan messages from a raw message buffer

	virtual bool ReliableSendRawMsgReply(unsigned short tagNum, unsigned long length, const void* msg, bool encrypt = true, long timeout = -1, bool async = false, bool copyData = false, const CompletionContainer<const AuthSendRawMsgResult&>& completion = DEFAULT_COMPLETION );

	template <class privsType>
	bool ReliableSendRawMsgReplyEx(unsigned short tagNum, unsigned long length, const void* msg, bool encrypt, long timeout, bool async, bool copyData, void (*f)(const AuthSendRawMsgResult&, privsType), privsType t)
	{ return ReliableSendRawMsgReply(tagNum, length, msg, encrypt, timeout, async, copyData, new CompletionWithContents<const AuthSendRawMsgResult&, privsType>(t, f, true)); }



	// SendBaseMsg - send a Titan message from a WONMsg::BaseMessage

	virtual bool ReliableSendBaseMsgReply(unsigned short tagNum, WONMsg::BaseMessage& msg, bool encrypt = true, long timeout = -1, bool async = false, bool copyData = false, const CompletionContainer<const AuthSendBaseMsgResult&>& completion = DEFAULT_COMPLETION );

	template <class privsType>
	bool ReliableSendBaseMsgReplyEx(unsigned short tagNum, WONMsg::BaseMessage& msg, bool encrypt, long timeout, bool async, bool copyData, void (*f)(const AuthSendBaseMsgResult&, privsType), privsType t)
	{ return ReliableSendBaseMsgReply(tagNum, msg, encrypt, timeout, async, copyData, new CompletionWithContents<const AuthSendBaseMsgResult&, privsType>(t, f, true)); }




	// DecryptMessage - Takes a raw msg buffer, and decrypts it if necessary

	unsigned char* DecryptMessage(bool allowUnencryptedThisTime, unsigned char* msg, unsigned long length, unsigned long& newLength, bool& wasEncrypted);

	
	class RequestData : public TMsgSocket::RequestData
	{
	public:
		CompletionContainer<const AuthRecvRawMsgResult&>	authCompletion;

		virtual void DoCompletion(const TMsgSocket::RecvRawMsgResult& result);
		void Done(const AuthRecvRawMsgResult& result);
	};

	WONCrypt::BFSymmetricKey GetSessionKey(void) { return sessionKey; }

private:
	Identity*					identity;
	WONCommon::CriticalSection	authCrit;
	class AuthSession*			session;
	unsigned long				acceptedSessionExpiration;

	bool						sequenced;
	bool						encrypting;
	bool						allowUnencrypted;
	bool						acceptAuth;
	bool						useClientKey;
	WONCrypt::BFSymmetricKey	sessionKey;

	bool						hasRefreshAddr;
	IPSocket::Address			refreshAddress;
	unsigned char				refreshLengthSize;

	bool						authenticated;	// false if Authenticate hasn't been called yet
	bool						reuseSession;

	typedef std::queue<CompletionContainer<const AuthResult&> > AuthQueue;
	AuthQueue					authQueue;
	WONCommon::Event			authDoneEvent;
	Error						authDoneError;

	bool						authenticating;

	bool						isAuthenticating;
	unsigned int				numAuthsPending;

	static void DoneStreamAuthenticate(const AuthSocket::AuthResult& result, RequestData* requestData);
	static void DoneStreamReqRecv(const AuthSocket::AuthRecvRawMsgResult& result, RequestData* requestData);
	static void DoneStreamReqAuth(const AuthSocket::OpenResult& result, RequestData* requestData);
	static void DoneRecvReliableUDP(const AuthSocket::AuthRecvRawMsgResult& result, RequestData* udpData);
	static void DoneAuthenticate(const AuthSocket::AuthResult& result, RequestData* udpData);
	static void DoneAuthenticateOpen(const AuthSocket::OpenResult& result, RequestData* udpData);
	void IssueRecv();

	class AuthenticationData;
	friend class AuthenticationData;

	void FinishQueued(Error err);
	static void DoneRecvAuth1Complete(const TMsgSocket::RecvBaseMsgResult& result, AuthenticationData* authData);
	static void DoneRecvChallenge1(const TMsgSocket::RecvBaseMsgResult& result, AuthenticationData* authData);
	static void DoneOpen(const Socket::OpenResult& result, AuthenticationData* authData);
	static void DoneRefresh(const Identity::AuthResult& result, AuthenticationData* authData);

	class AuthRecvMsgData;
	friend class AuthRecvMsgData;
	static void DoneRecvMsg(const TMsgSocket::RecvRawMsgResult& result, AuthRecvMsgData* recvMsgData);

	class AuthRecvBaseMsgData;
	friend class AuthRecvBaseMsgData;
	static void DoneRecvBaseMsg(const AuthSocket::AuthRecvRawMsgResult& result, AuthRecvBaseMsgData* recvBaseMsgData);

	class ReliableRecvRawMsgData;
	static void DoneReliableRecvRawMsg(const AuthRecvRawMsgResult& result, ReliableRecvRawMsgData* reliableRecvRawMsgData);

	friend class AuthSession;
public:
	WONAuth::Auth1Certificate* GetRemoteCertificate();
	void InvalidateSession();
	void ReleaseSession();
	bool IsAuthenticated()			{ return authenticated; }
};


// Peer-to-peer 1-off authentication

	// server side
Error ListenForPeerAuthentication(Identity& ident, unsigned short listenPort, const void* sendAfter = 0,
								  unsigned short sendAfterSize = 0);

void StopListeningForPeerAuthentication();


Error AcceptPeerAuthentication(WONAuth::Auth1Certificate* remoteCertBuf, void* recvedAfterBuf,
							   unsigned short* recvedAfterBufSize, long timeout, bool async,
							   const CompletionContainer<Error>& completion);

inline Error AcceptPeerAuthentication(WONAuth::Auth1Certificate* remoteCertBuf = 0, void* recvedAfterBuf = 0,
									  unsigned short* recvedAfterBufSize = 0, long timeout = -1, bool async = false)
{ return AcceptPeerAuthentication(remoteCertBuf, recvedAfterBuf, recvedAfterBufSize, timeout, async, DEFAULT_COMPLETION); }


template <class privsType>
inline Error AcceptPeerAuthenticationEx(WONAuth::Auth1Certificate* remoteCertBuf, void* recvedAfterBuf,
										unsigned short* recvedAfterBufSize, long timeout, bool async,
										void (*f)(Error, privsType), privsType privs)
{ return AcceptPeerAuthentication(remoteCertBuf, recvedAfterBuf, recvedAfterBufSize, timeout, async, new CompletionWithContents<Error, privsType>(f, privs, true)); }


	// client side
Error PeerAuthenticate(Identity& ident, const IPSocket::Address& host, WONAuth::Auth1Certificate* hostCertBuf = 0,
					   const void* sendAfter = 0, unsigned short sendAfterSize = 0,
					   void* recvedAfterBuf = 0, unsigned short* recvedAfterBufSize = 0,
					   long timeout = -1, bool async = false,
					   const CompletionContainer<Error>& completion = DEFAULT_COMPLETION );


template <class privsType>
inline Error PeerAuthenticateEx(Identity& ident, const IPSocket::Address& host,
								WONAuth::Auth1Certificate* hostCertBuf,
								const void* sendAfter, unsigned short sendAfterSize,
								void* recvedAfterBuf, unsigned short* recvedAfterBufSize,
								long timeout, bool async, void (*f)(Error, privsType), privsType privs )
{ return PeerAuthenticate(ident, host, hostCertBuf, sendAfter, sendAfterSize, recvedAfterBuf, recvedAfterBufSize, timeout, async, new CompletionWithContents<Error, privsType>(f, privs, true)); }



// Peer-to-peer 1-off authentication
// State machines, no IO

// Construct an object per peer-to-peer auth on client side
class PeerAuthClientState
{
private:
	WONCrypt::BFSymmetricKey clientSecret;
	WONCrypt::BFSymmetricKey sessionKey;
	WONAuth::Auth1Certificate ourCertificate;
	WONAuth::Auth1Certificate serverCertificate;
	WONCrypt::EGPrivateKey privateKey;
	WONCommon::RawBuffer buf;
	time_t authDelta;

	// void** arguments return pointer to buffer contained within class.
	// Beware that if you delete the object, the buffer will be also released.
public:
	// stage 1, call Init() and send req to the server
	// (You can get the certificate and private key out of an identity object.
	// Be sure it hasn't expired.)
	Error Init(Identity& ident,	const void** req, unsigned short* reqSize);	// Make sure Identity is already refreshed

	Error Init(const void* certificate, unsigned short certificateSize,
			   const void* privKey, unsigned short privKeySize,		 
			   const void** req, unsigned short* reqSize, time_t authServerDelta);

	// stage 3, client recvs challenge #1, and must send challenge #2 to server
	Error RecvChallenge1(const void* challenge1, unsigned short challenge1Size,
						 const void** challenge2, unsigned short* challenge2Size);

	// stage 5, client recvs final result from server.
	// If it checks out, client side of peer-to-peer authentication is complete.
	Error RecvResult(const void* result, unsigned short resultSize);

	const WONCrypt::BFSymmetricKey& GetSessionKey() const;
	void GetSessionKey(void* keyBuf, unsigned short* keyBufSize) const;
	unsigned long GetRemoteCommunityID(void) { return serverCertificate.GetCommunityId(); }
};

// Construct an object per peer-to-peer auth on server side
class PeerAuthServerState
{
private:
	WONCrypt::BFSymmetricKey sessionKey;
	WONCrypt::EGPrivateKey privateKey;
	WONAuth::Auth1Certificate remoteCertificate;
	WONCommon::RawBuffer buf;
	time_t authDelta;

public:

	// stage 2, server recvs req, and must sent challenge #1 to client
	// (You can get the certificate and private key out of an identity object.
	// Be sure it hasn't expired.)
	Error RecvAuth1Request(Identity& ident, const void* req, unsigned short reqSize,
						   const void** challenge1, unsigned short* challenge1Size);	// Make sure Identity* is already refreshed

	Error RecvAuth1Request(const void* req, unsigned short reqSize,
						   const void* serverCertificate, unsigned short serverCertificateSize,
						   const void* serverPrivateKey, unsigned short serverPrivateKeySize,
						   const void** challenge1, unsigned short* challenge1Size, time_t theAuthDelta);

	// stage 4, server recvs challenge #2, and must return final result to server.
	// Server side of peer-to-peer authentication is complete.
	Error RecvChallenge2(const void* challenge2, unsigned short challenge2Size,
						 const void** result, unsigned short* resultSize);

	const WONCrypt::BFSymmetricKey& GetSessionKey() const;
	void GetSessionKey(void* keyBuf, unsigned short* keyBufSize) const;
	unsigned long GetRemoteCommunityID(void) { return remoteCertificate.GetCommunityId(); }
};



};


#endif
