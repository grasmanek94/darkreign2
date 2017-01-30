#ifndef __WON_TMSGSOCKET_H__
#define __WON_TMSGSOCKET_H__

#pragma warning(disable : 4786)


#include <map>
#include <set>
#include "WSSocket.h"
#include "common/CriticalSection.h"
#include "common/Event.h"
#include "msg/TMessage.h"

namespace WONAPI {


class TMsgSocket : public Socket
{
public:
	class RequestData;

protected:
	struct ltRequestData
	{
		bool operator()(const RequestData* asyncOp1, const RequestData* asyncOp2) const;
	};

	typedef std::map<unsigned short, RequestData*>	TagMap;
	typedef std::multiset<RequestData*, ltRequestData> TimeoutSet;

protected:

	WSSocket* actualSocket;
	unsigned char lengthSize;
	bool ownsSocket;
	unsigned long largestDatagramSize;

	// Disallow
	virtual unsigned long Send(unsigned long count, const void* buffer, long timeout = -1, bool async = false, bool copyData = false, const CompletionContainer<const TransmitResult&>& completion = DEFAULT_COMPLETION );
	virtual unsigned long SendTo(unsigned long count, const void* buffer, const Address& sendToAddr, long timeout = -1, bool async = false, bool copyData = false, const CompletionContainer<const TransmitResult&>& completion = DEFAULT_COMPLETION );
	virtual unsigned long Recv(unsigned long count, void* buffer, long timeout = -1, bool async = false, const CompletionContainer<const TransmitResult&>& completion = DEFAULT_COMPLETION );
	virtual unsigned long RecvFrom(unsigned long count, void* buffer, Address* recvFromAddr, long timeout = -1, bool async = false, const CompletionContainer<const TransmitResult&>& completion = DEFAULT_COMPLETION );
	virtual bool AbortRecv();
	virtual bool AbortSend();

public:
	TMsgSocket(WSSocket& socket, unsigned char lngSize = 4, size_t largestDatagram = 0x0000FFFF);
	TMsgSocket(WSSocket* socket, unsigned char lngSize = 4, size_t largestDatagram = 0x0000FFFF);
	TMsgSocket(WSSocket* socket, bool takeOwnership, unsigned char lngSize = 4, size_t largestDatagram = 0x0000FFFF);

	virtual ~TMsgSocket();

	void SetLengthSize(unsigned char lngSize)	{ lengthSize = lngSize; }
	unsigned char GetLengthSize()				{ return lengthSize; }

	WSSocket* GetSocket()						{ return actualSocket; }

	class RecvRawMsgResult
	{
	public:
		TMsgSocket*		theSocket;
		unsigned char*	msg;			// Caller must delete when finished.
										// Will be null on failure
		unsigned long	msgLength;		// msg length
		bool			closed;

		RecvRawMsgResult(TMsgSocket* sock, unsigned char* buf, unsigned long msgLen, bool socketClosed = false)
			:	theSocket(sock), msg(buf), msgLength(msgLen), closed(socketClosed)
		{}
	};

	
	class RecvBaseMsgResult
	{
	public:
		TMsgSocket*				theSocket;
		WONMsg::BaseMessage*	msg;
		bool					closed;

		RecvBaseMsgResult(TMsgSocket* sock, WONMsg::BaseMessage* theMsg, bool socketClosed = false)
			:	theSocket(sock), msg(theMsg), closed(socketClosed)
		{ }

		RecvBaseMsgResult(const RecvBaseMsgResult& toCopy)
		{
			theSocket = toCopy.theSocket;
			msg = toCopy.msg;
			closed = toCopy.closed;
		}
	};

	class ReliableRecvRawMsgResult : public RecvRawMsgResult
	{
	public:
		unsigned short tagNum;

		ReliableRecvRawMsgResult(unsigned short tag, TMsgSocket* sock, unsigned char* buf, unsigned long msgLen, bool socketClosed = false)
			:	RecvRawMsgResult(sock, buf, msgLen, socketClosed), tagNum(tag)
		{}

		ReliableRecvRawMsgResult(unsigned short tag, const RecvRawMsgResult& result)
			:	RecvRawMsgResult(result), tagNum(tag)
		{}
	};

	
	class ReliableRecvBaseMsgResult : public RecvBaseMsgResult
	{
	public:
		unsigned short tagNum;

		ReliableRecvBaseMsgResult(unsigned short tag, TMsgSocket* sock, WONMsg::BaseMessage* theMsg, bool socketClosed = false)
			:	RecvBaseMsgResult(sock, theMsg, socketClosed), tagNum(tag)
		{ }
	};


	class SendRawMsgResult : public RecvRawMsgResult
	{
	public:
		bool success;

		SendRawMsgResult(TMsgSocket* sock, unsigned char* buf, unsigned long msgLen, bool s, bool socketClosed = false)
			:	RecvRawMsgResult(sock, buf, msgLen, socketClosed), success(s)
		{}
	};



	class SendBaseMsgResult : public RecvBaseMsgResult
	{
	public:
		bool success;

		SendBaseMsgResult(TMsgSocket* sock, WONMsg::BaseMessage* theMsg, bool s, bool socketClosed = false)
			:	RecvBaseMsgResult(sock, theMsg, socketClosed), success(s)
		{ }
	};

	virtual Type GetType();

	virtual Error Open(const Address* addr = 0, long timeout = -1, bool async = false, const CompletionContainer<const OpenResult&>& completion = DEFAULT_COMPLETION );

	Error Open(const Address& addr, long timeout = -1, bool async = false, const CompletionContainer<const OpenResult&>& completion = DEFAULT_COMPLETION )
	{ return Open(&addr, timeout, async, completion); }

	template <class privsType>
	Error OpenEx(const Address* addr, long timeout, bool async, void (*f)(const OpenResult&, privsType), privsType t)
	{ return Open(addr, timeout, async, new CompletionWithContents<const OpenResult&, privsType>(t, f, true)); }

	template <class privsType>
	Error OpenEx(const Address& addr, long timeout, bool async, void (*f)(const OpenResult&, privsType), privsType t)
	{ return Open(&addr, timeout, async, new CompletionWithContents<const OpenResult&, privsType>(t, f, true)); }


	virtual bool IsOpen();
	virtual bool IsSendOpen();
	virtual bool IsRecvOpen();


	virtual void Close(long timeout = -1, bool async = false, const CompletionContainer<const Result&>& completion = DEFAULT_COMPLETION );

	template <class privsType>
	void CloseEx(long timeout, bool async, void (*f)(const Result&, privsType), privsType t)
	{ Close(timeout, async, new CompletionWithContents<const Result&, privsType>(t, f, true)); }


	virtual void CloseSend(long timeout = -1, bool async = false, const CompletionContainer<const Result&>& completion = DEFAULT_COMPLETION );

	template <class privsType>
	void CloseSendEx(long timeout, bool async, void (*f)(const Result&, privsType), privsType t)
	{ CloseSend(timeout, async, new CompletionWithContents<const Result&, privsType>(t, f, true)); }


	virtual void CloseRecv(long timeout = -1, bool async = false, const CompletionContainer<const Result&>& completion = DEFAULT_COMPLETION );

	template <class privsType>
	void CloseRecvEx(long timeout, bool async, void (*f)(const Result&, privsType), privsType t)
	{ CloseRecv(timeout, async, new CompletionWithContents<const Result&, privsType>(t, f, true)); }


	virtual void CatchClose(const CompletionContainer<const Result&>& completion = DEFAULT_COMPLETION , bool reuse = false);

	template <class privsType>
	void CatchCloseEx(void (*f)(const Result&, privsType), privsType t, bool reuse = false)
	{ CatchClose(new CompletionWithContents<const Result&, privsType>(t, f, true), reuse); }


	virtual void CatchCloseRecv(const CompletionContainer<const Result&>& completion = DEFAULT_COMPLETION , bool reuse = false);

	template <class privsType>
	void CatchCloseRecvEx(void (*f)(const Result&, privsType), privsType t, bool reuse = false)
	{ CatchCloseRecv(new CompletionWithContents<const Result&, privsType>(t, f, true), reuse); }


	virtual void CatchRecvable(const CompletionContainer<const Result&>& completion = DEFAULT_COMPLETION , bool reuse = false);

	template <class privsType>
	void CatchRecvableEx(void (*f)(const Result&, privsType), privsType t, bool reuse = false)
	{ CatchRecvable(new CompletionWithContents<const Result&, privsType>(t, f, true), reuse); }


	virtual unsigned long Available();

	virtual bool IsRecving();
	virtual bool IsSending();


	// RecvRawMsg - recv a Titan message into a buffer

	// Reads a Titan-specific msg...
	// Function result must be deleted, and is the same buffer as is passed to completion.
	// If unable to read entire message before timeout expires, the socket is closed.
	virtual unsigned char* RecvRawMsg(unsigned char** recvMsgBuf, unsigned long* length, long timeout = -1, bool async = false, const CompletionContainer<const RecvRawMsgResult&>& completion = DEFAULT_COMPLETION );

	template <class privsType>
	unsigned char* RecvRawMsgEx(unsigned char** recvMsgBuf, unsigned long* length, long timeout, bool async, void (*f)(const RecvRawMsgResult&, privsType), privsType t)
	{ return RecvRawMsg(recvMsgBuf, length, timeout, async, new CompletionWithContents<const RecvRawMsgResult&, privsType>(t, f, true)); }



	// RecvBaseMsg - recv a Titan message into a WONMsg::BaseMessage

	virtual WONMsg::BaseMessage* RecvBaseMsg(WONMsg::BaseMessage** recvMsg, long timeout = -1, bool async = false, const CompletionContainer<const RecvBaseMsgResult&>& completion = DEFAULT_COMPLETION );
		
	template <class privsType>
	WONMsg::BaseMessage* RecvBaseMsgEx(WONMsg::BaseMessage** recvMsg, long timeout, bool async, void (*f)(const RecvBaseMsgResult&, privsType), privsType t)
	{ return RecvBaseMsg(recvMsg, timeout, async, new CompletionWithContents<const RecvBaseMsgResult&, privsType>(t, f, true)); }


	// SendRawMsg - send a Titan messages from a raw message buffer

	virtual bool SendRawMsg(unsigned long length, const void* msg, long timeout = -1, bool async = false, bool copyData = false, const CompletionContainer<const SendRawMsgResult&>& completion = DEFAULT_COMPLETION );

	template <class privsType>
	bool SendRawMsgEx(unsigned long length, const void* msg, long timeout, bool async, bool copyData, void (*f)(const SendRawMsgResult&, privsType), privsType t)
	{ return SendRawMsg(length, msg, timeout, async, copyData, new CompletionWithContents<const SendRawMsgResult&, privsType>(t, f, true)); }



	// SendBaseMsg - send a Titan message from a WONMsg::BaseMessage

	virtual bool SendBaseMsg(WONMsg::BaseMessage& msg, long timeout = -1, bool async = false, bool copyData = false, const CompletionContainer<const SendBaseMsgResult&>& completion = DEFAULT_COMPLETION );

	template <class privsType>
	bool SendBaseMsgEx(WONMsg::BaseMessage& msg, long timeout, bool async, bool copyData, void (*f)(const SendBaseMsgResult&, privsType), privsType t)
	{ return SendBaseMsg(msg, timeout, async, copyData, new CompletionWithContents<const SendBaseMsgResult&, privsType>(t, f, true)); }


	// SendRawMsgRequest - Send raw titan message, and recv reply.  Does reliable UDP

	virtual unsigned char* SendRawMsgRequest(unsigned long length, const void* req, unsigned char** replyBuf, unsigned long* replyLength = 0, unsigned long retryTimeout = 2000, long timeout = -1, bool async = false, bool copyData = false, const CompletionContainer<const RecvRawMsgResult&>& completion = DEFAULT_COMPLETION );

	template <class privsType>
	unsigned char* SendRawMsgRequestEx(unsigned long length, const void* req, unsigned char** replyBuf, unsigned long* replyLength, unsigned long retryTimeout, long timeout, bool async, bool copyData, void (*f)(const RecvRawMsgResult&, privsType), privsType t)
	{ return SendRawMsgRequest(length, req, replyBuf, replyLength, retryTimeout, timeout, async, copyData, new CompletionWithContents<const RecvRawMsgResult&, privsType>(t, f, true)); }


	// SendBaseMsgRequest - Send titan WONMsg::BaseMessage, and recv reply.  Does reliable UDP

	virtual WONMsg::BaseMessage* SendBaseMsgRequest(WONMsg::BaseMessage& req, WONMsg::BaseMessage** reply = 0, unsigned long retryTimeout = 2000, long timeout= -1, bool async = false, bool copyData = false, const CompletionContainer<const RecvBaseMsgResult&>& completion = DEFAULT_COMPLETION );

	template <class privsType>
	WONMsg::BaseMessage* SendBaseMsgRequestEx(WONMsg::BaseMessage& req, WONMsg::BaseMessage** reply, unsigned long retryTimeout, long timeout, bool async, bool copyData, void (*f)(const RecvBaseMsgResult&, privsType), privsType t)
	{ return SendBaseMsgRequest(req, reply, retryTimeout, timeout, async, copyData, new CompletionWithContents<const RecvBaseMsgResult&, privsType>(t, f, true)); }

	
	// ReliableRecvMsg - Similar to RecvMsg(), but uses server side of reliable UDP protocol.
	// If tagNum is zero, the incoming message did not include a tag value
	// In order to support reliable UDP, you must send the reply with ReliableSendRawMsgReply() or
	// ReliableSendBaseMsgReply(), and pass in the recv'ed tagNum.
	// The client side of reliable UDP is done via SendRawMsgRequest() or SendBaseMsgRequest()

	virtual unsigned char* ReliableRecvRawMsg(unsigned short* tagNum, unsigned char** recvMsgBuf, unsigned long* length, long timeout = -1, bool async = false, const CompletionContainer<const ReliableRecvRawMsgResult&>& completion = DEFAULT_COMPLETION );
	
	template <class privsType>
	unsigned char* ReliableRecvRawMsgEx(unsigned short* tagNum, unsigned char** recvMsgBuf, unsigned long* length, long timeout, bool async, void (*f)(const ReliableRecvRawMsgResult&, privsType), privsType t)
	{ return ReliableRecvRawMsg(tagNum, recvMsgBuf, length, timeout, async, new CompletionWithContents<const ReliableRecvRawMsgResult&, privsType>(t, f, true)); }


	virtual WONMsg::BaseMessage* ReliableRecvBaseMsg(unsigned short* tagNum, WONMsg::BaseMessage** recvMsg = NULL, long timeout = -1, bool async = false, const CompletionContainer<const ReliableRecvBaseMsgResult&>& completion = DEFAULT_COMPLETION );
	
	template <class privsType>
	WONMsg::BaseMessage* ReliableRecvBaseMsgEx(unsigned short* tagNum, WONMsg::BaseMessage** recvMsg, long timeout, bool async, void (*f)(const ReliableRecvBaseMsgResult&, privsType), privsType t)
	{ return ReliableRecvBaseMsg(tagNum, recvMsg, timeout, async, new CompletionWithContents<const ReliableRecvBaseMsgResult&, privsType>(t, f, true)); }


	// SendRawMsg - send a Titan messages from a raw message buffer

	virtual bool ReliableSendRawMsgReply(unsigned short tagNum, unsigned long length, const void* msg, long timeout = -1, bool async = false, bool copyData = false, const CompletionContainer<const SendRawMsgResult&>& completion = DEFAULT_COMPLETION );

	template <class privsType>
	bool ReliableSendRawMsgReplyEx(unsigned short tagNum, unsigned long length, const void* msg, long timeout, bool async, bool copyData, void (*f)(const SendRawMsgResult&, privsType), privsType t)
	{ return ReliableSendRawMsgReply(tagNum, length, msg, timeout, async, copyData, new CompletionWithContents<const SendRawMsgResult&, privsType>(t, f, true)); }



	// SendBaseMsg - send a Titan message from a WONMsg::BaseMessage

	virtual bool ReliableSendBaseMsgReply(unsigned short tagNum, WONMsg::BaseMessage& msg, long timeout = -1, bool async = false, bool copyData = false, const CompletionContainer<const SendBaseMsgResult&>& completion = DEFAULT_COMPLETION );

	template <class privsType>
	bool ReliableSendBaseMsgReplyEx(unsigned short tagNum, WONMsg::BaseMessage& msg, long timeout, bool async, bool copyData, void (*f)(const SendBaseMsgResult&, privsType), privsType t)
	{ return ReliableSendBaseMsgReply(tagNum, msg, timeout, async, copyData, new CompletionWithContents<const SendBaseMsgResult&, privsType>(t, f, true)); }



	// ExtractMessage - extract WONMsg::BaseMessage from a raw message buffer

	static WONMsg::BaseMessage* ExtractMessage(unsigned char* msg, unsigned long length);



	class RequestData
	{
	public:
		TMsgSocket*									thisSocket;
		unsigned long								retryTimeout;
		long										timeout;
		unsigned char*								tempReplyMsg;
		unsigned char**								replyMsg;
		unsigned long*								replyLen;
		WONCommon::Event							doneEvent;
		bool										autoDel;
		TagMap::iterator							tagMapItor;
		TimeoutSet::iterator						timeoutSetItor;
		unsigned long								startTime;
		unsigned long								tryStartTime;
		CompletionContainer<const RecvRawMsgResult&>	completion;
		unsigned char*								outgoingMsg;
		unsigned long								outgoingMsgLength;
		bool										usingDatagrams;
		bool										deallocReq;

		virtual void DoCompletion(const RecvRawMsgResult& result);

		void ResetTimeout();
		void DoneLL(const RecvRawMsgResult& result);
		void Done(const RecvRawMsgResult& result);
	};

	class TagInfo;

	typedef std::multimap<time_t, TagInfo*> TagExpireMap;
	typedef std::map<unsigned short, TagInfo*> TagNumMap;

	class TagInfo
	{
	public:
		unsigned long startTime;
		unsigned long timeout;
		unsigned short tagNum;
		unsigned char* msg;
		unsigned long msgLength;
		bool encrypt;

		TagNumMap::iterator myItor;
	};


	TagExpireMap tagExpireMap;
	TagNumMap tagNumMap;
	WONCommon::CriticalSection tagInfoCrit;

protected:
	unsigned short				tagSeqNum;
	WONCommon::CriticalSection	tagCrit;
	TagMap						tagMap;
	TimeoutSet					timeoutSet;
	bool						abortingRecv;
	bool						recving;
	WONCommon::Event			doneRecvEvent;

private:
	static void DoneOpen(const OpenResult& result, RequestData* udpData);
	static void DoneStreamOpen(const OpenResult& result, RequestData* requestData);
	static void DoneStreamReqRecv(const RecvRawMsgResult& result, RequestData* requestData);
	static void DoneRecvReliableUDP(const RecvRawMsgResult& result, RequestData* udpData);
	void IssueRecv();
	friend class RequestData;

	class RecvMsgData;
	friend class RecvMsgData;
	static void RecvTitanCompletion(const Socket::TransmitResult& result, RecvMsgData* recvMsgData);
	static void RecvTitanLengthCompletion(const Socket::TransmitResult& result, RecvMsgData* recvMsgData);

	class RecvBaseMsgData;
	friend class RecvBaseMsgData;
	static void DoneRecvBaseMsg(const TMsgSocket::RecvRawMsgResult& result, RecvBaseMsgData* recvBaseMsgData);

	class SendMsgData;
	friend class SendMsgData;
	static void DoneSendMsg(const Socket::TransmitResult& result, SendMsgData* sendMsgData);

	class IntermediateCompletion;
	class IntermediateOpenCompletion;
	friend class IntermediateCompletion;
	friend class IntermediateOpenCompletion;

	class ReliableRecvRawMsgData;
	static void DoneReliableRecvRawMsg(const RecvRawMsgResult& result, ReliableRecvRawMsgData* reliableRecvRawMsgData);
};


};

#endif
