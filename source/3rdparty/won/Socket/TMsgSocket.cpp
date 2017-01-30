#include "WSSocket.h"
#include "TMsgSocket.h"
#include "common/WONEndian.h"
#include <iostream>
using namespace std;

using namespace WONAPI;
using namespace WONMsg;
using namespace WONCommon;


TMsgSocket::TMsgSocket(WSSocket& socket, unsigned char lngSize, size_t largestDatagram)
	:	recving(0),
		tagSeqNum(1),
		abortingRecv(false),
		doneRecvEvent(true, true),
		actualSocket(&socket),
		ownsSocket(false),
		lengthSize(lngSize),
		largestDatagramSize(largestDatagram)
{
}


TMsgSocket::TMsgSocket(WSSocket* socket, unsigned char lngSize, size_t largestDatagram)
	:	recving(0),
		tagSeqNum(1),
		abortingRecv(false),
		doneRecvEvent(true, true),
		actualSocket(socket),
		ownsSocket(false),
		lengthSize(lngSize),
		largestDatagramSize(largestDatagram)
{
}


TMsgSocket::TMsgSocket(WSSocket* socket, bool takeOwnership, unsigned char lngSize, size_t largestDatagram)
	:	recving(0),
		tagSeqNum(1),
		abortingRecv(false),
		doneRecvEvent(true, true),
		actualSocket(socket),
		ownsSocket(takeOwnership),
		lengthSize(lngSize),
		largestDatagramSize(largestDatagram)
{
}


bool TMsgSocket::AbortRecv()
{
	return actualSocket->AbortRecv();
}


bool TMsgSocket::AbortSend()
{
	return actualSocket->AbortSend();
}


TMsgSocket::~TMsgSocket()
{
	if (ownsSocket)
		delete actualSocket;

	TagExpireMap::iterator itor = tagExpireMap.begin();
	while (itor != tagExpireMap.end())
	{
		TagInfo* tagInfo = (*itor).second;
	//	if (tagInfo->msg)
			delete tagInfo->msg;
	//	tagNumMap.erase(tagInfo->myItor);
		delete tagInfo;
		itor++;
	}

	typedef std::multimap<time_t, class TagInfo*> TagExpireMap;
}


Socket::Type TMsgSocket::GetType()
{
	return actualSocket->GetType();
}


class TMsgSocket::IntermediateOpenCompletion : public Completion<const Socket::OpenResult&>
{
public:
	TMsgSocket*										tmsgsock;
	CompletionContainer<const Socket::OpenResult&>	completion;

	IntermediateOpenCompletion(TMsgSocket* sock, CompletionContainer<const Socket::OpenResult&> cmplt, bool autoDel)
		:	tmsgsock(sock), completion(cmplt), Completion<const Socket::OpenResult&>(autoDel)
	{ };

	virtual void Complete(const Socket::OpenResult& result);
};


void TMsgSocket::IntermediateOpenCompletion::Complete(const Socket::OpenResult& result)
{
	Socket::OpenResult myOpenResult(tmsgsock, result.error);
	completion.Complete(myOpenResult);
	Completion<const Socket::OpenResult&>::Complete(myOpenResult);
}


Error TMsgSocket::Open(const Address* addr, long timeout, bool async, const CompletionContainer<const Socket::OpenResult&>& completion)
{
	if (!completion.empty())
	{
		IntermediateOpenCompletion* intermediateCompletion = new IntermediateOpenCompletion(this, completion, true);
		intermediateCompletion->completion.OwnCompletion();
		return actualSocket->Open(addr, timeout, async, intermediateCompletion);
	}
	return actualSocket->Open(addr, timeout, async);
}


unsigned long TMsgSocket::Send(unsigned long count, const void* buffer, long timeout, bool async, bool copyData, const CompletionContainer<const Socket::TransmitResult&>& completion)
{
	completion.Complete(TransmitResult(this, (void*)buffer, count, 0, false));
	return 0;
}


unsigned long TMsgSocket::SendTo(unsigned long count, const void* buffer, const Address& sendToAddr, long timeout, bool async, bool copyData, const CompletionContainer<const TransmitResult&>& completion)
{
	completion.Complete(TransmitResult(this, (void*)buffer, count, 0, false));
	return 0;
}


unsigned long TMsgSocket::Recv(unsigned long count, void* buffer, long timeout, bool async, const CompletionContainer<const Socket::TransmitResult&>& completion)
{
	completion.Complete(TransmitResult(this, buffer, count, 0, false));
	return 0;
}


unsigned long TMsgSocket::RecvFrom(unsigned long count, void* buffer, Address* recvFromAddr, long timeout, bool async, const CompletionContainer<const TransmitResult&>& completion)
{
	completion.Complete(TransmitResult(this, buffer, count, 0, false));
	return 0;
}


bool TMsgSocket::IsOpen()
{
	return actualSocket->IsOpen();
}


bool TMsgSocket::IsSendOpen()
{
	return actualSocket->IsSendOpen();
}


bool TMsgSocket::IsRecvOpen()
{
	return actualSocket->IsRecvOpen();
}


class TMsgSocket::IntermediateCompletion : public Completion<const Socket::Result&>
{
public:
	TMsgSocket*									tmsgsock;
	CompletionContainer<const Socket::Result&>	completion;
	bool										deletedCompletion;

	IntermediateCompletion(TMsgSocket* sock, CompletionContainer<const Socket::Result&> cmplt, bool autoDel)
		:	tmsgsock(sock), completion(cmplt), Completion<const Socket::Result&>(autoDel), deletedCompletion(false)
	{ };

	virtual void Complete(const Socket::Result& result);
};


void TMsgSocket::IntermediateCompletion::Complete(const Socket::Result& result)
{
	Socket::Result myResult(tmsgsock);
	completion.Complete(myResult);
	Completion<const Socket::Result&>::Complete(myResult);
}


void TMsgSocket::CloseSend(long timeout, bool async, const CompletionContainer<const Result&>& completion)
{
	if (!completion.empty())
	{
		IntermediateCompletion* intermediateCompletion = new IntermediateCompletion(this, completion, true);
		intermediateCompletion->completion.OwnCompletion();
		actualSocket->CloseSend(timeout, async, intermediateCompletion);
	}
	else
		actualSocket->CloseSend(timeout, async);
}


void TMsgSocket::CloseRecv(long timeout, bool async, const CompletionContainer<const Result&>& completion)
{
	if (!completion.empty())
	{
		IntermediateCompletion* intermediateCompletion = new IntermediateCompletion(this, completion, true);
		intermediateCompletion->completion.OwnCompletion();
		actualSocket->CloseRecv(timeout, async, intermediateCompletion);
	}
	else
		actualSocket->CloseRecv(timeout, async);
}


void TMsgSocket::Close(long timeout, bool async, const CompletionContainer<const Result&>& completion)
{
	if (!completion.empty())
	{
		IntermediateCompletion* intermediateCompletion = new IntermediateCompletion(this, completion, true);
		intermediateCompletion->completion.OwnCompletion();
		actualSocket->Close(timeout, async, intermediateCompletion);
	}
	else
		actualSocket->Close(timeout, async);
}


void TMsgSocket::CatchClose(const CompletionContainer<const Result&>& completion, bool reuse)
{
	if (!completion.empty())
	{
		IntermediateCompletion* intermediateCompletion = new IntermediateCompletion(this, completion, true);
		intermediateCompletion->completion.OwnCompletion();
		actualSocket->CatchClose(intermediateCompletion, reuse);
	}
	else
		actualSocket->CatchClose(completion, reuse);
}


void TMsgSocket::CatchCloseRecv(const CompletionContainer<const Result&>& completion, bool reuse)
{
	if (!completion.empty())
	{
		IntermediateCompletion* intermediateCompletion = new IntermediateCompletion(this, completion, true);
		intermediateCompletion->completion.OwnCompletion();
		actualSocket->CatchCloseRecv(intermediateCompletion, reuse);
	}
	else
		actualSocket->CatchCloseRecv(completion, reuse);
}


void TMsgSocket::CatchRecvable(const CompletionContainer<const Result&>& completion, bool reuse)
{
	if (!completion.empty())
	{
		IntermediateCompletion* intermediateCompletion = new IntermediateCompletion(this, completion, true);
		intermediateCompletion->completion.OwnCompletion();
		actualSocket->CatchRecvable(intermediateCompletion, reuse);
	}
	else
		actualSocket->CatchRecvable(completion, reuse);
}


unsigned long TMsgSocket::Available()
{
	return actualSocket->Available();
}


bool TMsgSocket::IsRecving()
{
	return actualSocket->IsRecving();
}


bool TMsgSocket::IsSending()
{
	return actualSocket->IsSending();
}


class TMsgSocket::RecvMsgData
{
public:
	TMsgSocket* thisSocket;
	long timeout;
	unsigned long* length;
	unsigned char** recvMsg;
	unsigned long msgSize;
	bool autoDelete;
	unsigned char* result;
	bool success;
	bool socketClosed;
	CompletionContainer<const TMsgSocket::RecvRawMsgResult&> completion;

	void Done()
	{
		if (length)
			*length = msgSize;
		completion.Complete(success ? TMsgSocket::RecvRawMsgResult(thisSocket, result, msgSize, socketClosed) : TMsgSocket::RecvRawMsgResult(thisSocket, 0, 0, socketClosed));

		if (autoDelete)
			delete this;
	}
};


void TMsgSocket::RecvTitanCompletion(const Socket::TransmitResult& result, RecvMsgData* recvMsgData)
{
	recvMsgData->socketClosed = result.closed;

	if (!result.bytesUsed)
	{
		delete recvMsgData->result;
		recvMsgData->result = 0;
	}
	if (result.theSocket->GetType() == datagram)
	{
		recvMsgData->msgSize = result.bytesUsed;
		recvMsgData->success = true;
	}
	else if (result.bytesUsed == result.bytesRequested)
		recvMsgData->success = true;
	else
		result.theSocket->Close(0, true);
	recvMsgData->Done();
}


void TMsgSocket::RecvTitanLengthCompletion(const Socket::TransmitResult& result, RecvMsgData* recvMsgData)
{
	recvMsgData->socketClosed = result.closed;

	unsigned char lengthSize = result.bytesRequested;

	if (result.bytesUsed != lengthSize)
	{
		if (result.bytesUsed != 0)			// Only close if not on msg boundry
			result.theSocket->Close(0);
		recvMsgData->msgSize = 0;
		recvMsgData->Done();
		return;
	}
	switch (lengthSize)
	{
	case 1:
		{
			recvMsgData->msgSize = *(unsigned char*)(result.buffer) - lengthSize;
			if (recvMsgData->msgSize)
			{
				recvMsgData->result = new unsigned char[recvMsgData->msgSize];
				break;
			}
		}
		recvMsgData->Done();
		return;
	case 2:
		{
			recvMsgData->msgSize = getLittleEndian(*(unsigned short*)(result.buffer)) - lengthSize;
			if (recvMsgData->msgSize)
			{
				recvMsgData->result = new unsigned char[recvMsgData->msgSize];
				break;
			}
		}
		recvMsgData->Done();
		return;
	case 4:
		{
			recvMsgData->msgSize = getLittleEndian(*(unsigned long*)(result.buffer)) - lengthSize;
			if (recvMsgData->msgSize)
			{
				recvMsgData->result = new unsigned char[recvMsgData->msgSize];
				break;
			}
		}
	default:
		result.theSocket->Close(0, true);
		recvMsgData->Done();
		return;
	}
	if (recvMsgData->recvMsg)
		*(recvMsgData->recvMsg) = recvMsgData->result;
	result.theSocket->RecvEx(recvMsgData->msgSize, recvMsgData->result, recvMsgData->timeout, recvMsgData->autoDelete, RecvTitanCompletion, recvMsgData);
}


unsigned char* TMsgSocket::RecvRawMsg(unsigned char** recvMsg, unsigned long* length, long timeout, bool async, const CompletionContainer<const RecvRawMsgResult&>& completion)
{
	RecvMsgData* recvMsgData = 0;
	
	if (lengthSize == 1 || lengthSize == 2 || lengthSize == 4 || actualSocket->GetType() == datagram)
		recvMsgData = new RecvMsgData;

	if (!recvMsgData)
	{
		completion.Complete(RecvRawMsgResult(this, 0, 0, false));
		return 0;
	}

	recvMsgData->socketClosed = false;
	recvMsgData->recvMsg = recvMsg;
	recvMsgData->thisSocket = this;
	recvMsgData->result = 0;
	recvMsgData->success = false;
	recvMsgData->timeout = timeout;
	recvMsgData->length = length;
	recvMsgData->autoDelete = async;
	recvMsgData->completion = completion;

	if (actualSocket->GetType() == datagram)
	{
		recvMsgData->result = new unsigned char[largestDatagramSize ? largestDatagramSize : 2];
		if (!recvMsgData->result)
		{
			delete recvMsgData;
			completion.Complete(RecvRawMsgResult(this, 0, 0, false));
			return 0;
		}

		void (*RecvTitanCompletionFunc)(const Socket::TransmitResult&, RecvMsgData*) = RecvTitanCompletion;

		actualSocket->RecvEx(largestDatagramSize, recvMsgData->result, timeout, async, RecvTitanCompletionFunc, recvMsgData);
	}
	else
	{
		void (*RecvTitanLengthCompletionFunc)(const Socket::TransmitResult&, RecvMsgData*) = RecvTitanLengthCompletion;

		actualSocket->RecvEx(lengthSize, 0, timeout, async, RecvTitanLengthCompletionFunc, recvMsgData);
	}

	if (async)
		return 0;
		
	unsigned char* result = recvMsgData->result;
	
	delete recvMsgData;
	
	return result;
}


class TMsgSocket::RecvBaseMsgData
{
public:
	CompletionContainer<const TMsgSocket::RecvBaseMsgResult&> completion;
	bool autoDelete;
	bool socketClosed;
	BaseMessage* msg;
	BaseMessage** recvMsg;
	TMsgSocket* thisSocket;

	void Done()
	{
		completion.Complete(TMsgSocket::RecvBaseMsgResult(thisSocket, msg, socketClosed));

		if (autoDelete)
			delete this;
	}
};


void TMsgSocket::DoneRecvBaseMsg(const TMsgSocket::RecvRawMsgResult& result, RecvBaseMsgData* recvBaseMsgData)
{
	recvBaseMsgData->socketClosed = result.closed;
	recvBaseMsgData->msg = NULL;

	if (result.msg)
		recvBaseMsgData->msg = result.theSocket->ExtractMessage(result.msg, result.msgLength);

	if (recvBaseMsgData->recvMsg)
		*(recvBaseMsgData->recvMsg) = recvBaseMsgData->msg;

	recvBaseMsgData->Done();

	if (result.msg)
		delete result.msg;
}



BaseMessage* TMsgSocket::RecvBaseMsg(WONMsg::BaseMessage** recvMsg, long timeout, bool async, const CompletionContainer<const RecvBaseMsgResult&>& completion)
{
	RecvBaseMsgData* recvBaseMsgData = new RecvBaseMsgData();
	if (!recvBaseMsgData)
	{
		completion.Complete(RecvBaseMsgResult(this, 0, false));
		return 0;
	}

	recvBaseMsgData->socketClosed = false;
	recvBaseMsgData->recvMsg = recvMsg;
	recvBaseMsgData->completion = completion;
	recvBaseMsgData->autoDelete = async;
	recvBaseMsgData->thisSocket = this;

	void (*DoneRecvBaseMsgFunc)(const TMsgSocket::RecvRawMsgResult&, RecvBaseMsgData*) = DoneRecvBaseMsg;

	RecvRawMsgEx((unsigned char**)0, 0, timeout, async, DoneRecvBaseMsgFunc, recvBaseMsgData);

	if (async)
		return 0;

	BaseMessage* msg = recvBaseMsgData->msg;

	delete recvBaseMsgData;

	return msg;
}



class TMsgSocket::SendMsgData
{
public:
	TMsgSocket* thisSocket;
	const void *msg;
	unsigned long length;
	CompletionContainer<const TMsgSocket::SendRawMsgResult&> completion;
	bool autoDelete;
	bool success;
	bool socketClosed;

	void Done()
	{
		completion.Complete(TMsgSocket::SendRawMsgResult(thisSocket, (unsigned char*)msg, length, success, socketClosed));
		
		if (autoDelete)
			delete this;
	}
};


void TMsgSocket::DoneSendMsg(const Socket::TransmitResult& result, SendMsgData* sendMsgData)
{
	sendMsgData->socketClosed = result.closed;
	sendMsgData->success = (result.bytesRequested == result.bytesUsed);
	if (!sendMsgData->success)
		result.theSocket->Close(0);
	sendMsgData->Done();
}


bool TMsgSocket::SendRawMsg(unsigned long length, const void* msg, long timeout, bool async, bool copyData, const CompletionContainer<const SendRawMsgResult&>& completion)
{
	SendMsgData* sendMsgData = new SendMsgData;
	if (!sendMsgData)
	{
		completion.Complete(SendRawMsgResult(this, (unsigned char*)msg, length, false, false));
		return false;
	}
	unsigned char* buffer = (unsigned char*)msg;
	unsigned long newLength = length;
	if (actualSocket->GetType() == stream)
	{
		buffer = new unsigned char[length + lengthSize];
		if (!buffer)
		{
			delete sendMsgData;
			completion.Complete(SendRawMsgResult(this, (unsigned char*)msg, length, false, false));
			return false;
		}
		unsigned long sendLength = length + lengthSize;	// Will only work on a little-endian(?) machine
		makeLittleEndian(sendLength);
		memcpy(buffer, &sendLength, lengthSize);
		memcpy(buffer+lengthSize, msg, length);
		newLength += lengthSize;
	}
	sendMsgData->socketClosed = false;
	sendMsgData->thisSocket = this;
	sendMsgData->success = false;
	sendMsgData->msg = msg;
	sendMsgData->autoDelete = async;
	sendMsgData->length = length;
	sendMsgData->completion = completion;
	
	void (*DoneSendMsgFunc)(const Socket::TransmitResult&, SendMsgData*) = DoneSendMsg;

	actualSocket->SendEx(newLength, buffer, timeout, async, async, DoneSendMsgFunc, sendMsgData);
	
	if (actualSocket->GetType() == stream)
		delete buffer;

	if (async)
		return false;
	
	bool result = sendMsgData->success;

	delete sendMsgData;

	return result;
}


class SendBaseMsgData
{
public:
	CompletionContainer<const TMsgSocket::SendBaseMsgResult&> completion;
	BaseMessage* msg;
};


static void DoneSendBaseMsg(const TMsgSocket::SendRawMsgResult& result, SendBaseMsgData* sendBaseMsgData)
{
	sendBaseMsgData->completion.Complete(TMsgSocket::SendBaseMsgResult(result.theSocket, sendBaseMsgData->msg, result.success, result.closed));
	delete sendBaseMsgData;
}


bool TMsgSocket::SendBaseMsg(BaseMessage& msg, long timeout, bool async, bool copyData, const CompletionContainer<const SendBaseMsgResult&>& completion)
{
	SendBaseMsgData* sendBaseMsgData = new SendBaseMsgData;
	if (!sendBaseMsgData)
	{
		completion.Complete(SendBaseMsgResult(this, &msg, false, false));
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
		completion.Complete(SendBaseMsgResult(this, &msg, false, false));
		return false;
	}

	return SendRawMsgEx(msg.GetDataLen(), msg.GetDataPtr(), timeout, async, copyData, DoneSendBaseMsg, sendBaseMsgData);
}


BaseMessage* TMsgSocket::ExtractMessage(unsigned char* msg, unsigned long msgLength)
{
	BaseMessage* theMsg = NULL;

	if (msgLength)
	{
		unsigned char headerType = *(msg);
		switch (headerType)
		{
		case 2:	// Encrypted TMessage
		case 4:	// Encrypted MiniMessage
		case 6:	// Encrypted SmallMessage
		case 8:	// Encrypted LargeMessage
			// Disallowed on non-AuthSocket
			break;
		case 3:	// MiniMessage
			theMsg = new MiniMessage(msgLength, msg);
			break;
		case 5:	// SmallMessage
			theMsg = new SmallMessage(msgLength, msg);
			break;
		case 7:	// LargeMessage
			theMsg = new LargeMessage(msgLength, msg);
			break;
		default:	// old TMessage
			theMsg = new TMessage(msgLength, msg);
			break;
		}
		if (theMsg)
		{
			try {
				theMsg->Unpack();
			}
			catch (...)
			{
				delete theMsg;
				theMsg = NULL;
			}
		}
	}
	return theMsg;
}


void TMsgSocket::RequestData::ResetTimeout()
{
	thisSocket->timeoutSet.erase(timeoutSetItor);
	tryStartTime = GetTickCount();
	timeoutSetItor = thisSocket->timeoutSet.insert(this);
}


void TMsgSocket::RequestData::DoCompletion(const RecvRawMsgResult& result)
{
	completion.Complete(result);
	if (autoDel)
		delete this;
	else
		doneEvent.Set();
}


void TMsgSocket::RequestData::Done(const TMsgSocket::RecvRawMsgResult& result)
{
	DoneLL(result);
}


void TMsgSocket::RequestData::DoneLL(const TMsgSocket::RecvRawMsgResult& result)
{
	if (deallocReq)
	{
		delete outgoingMsg;
		outgoingMsg = 0;
	}

	if (usingDatagrams)
	{
		thisSocket->tagMap.erase(tagMapItor);
		thisSocket->timeoutSet.erase(timeoutSetItor);
		delete outgoingMsg;
	}

	if (replyLen)
		*replyLen = result.msgLength;

	*replyMsg = result.msg;

	DoCompletion(result);
}


// Compares timeouts, safe if GetTickCount() loops past 0
bool TMsgSocket::ltRequestData::operator()(const RequestData* data1, const RequestData* data2) const
{
	unsigned long now = GetTickCount();

	unsigned long t1 = now - data1->tryStartTime;
	t1 = (t1 < data1->retryTimeout) ? data1->retryTimeout - t1 : 0;

	unsigned long t2 = now - data2->tryStartTime;
	t2 = (t2 < data2->retryTimeout) ? data2->retryTimeout - t2 : 0;

	return t1 < t2;
}


void TMsgSocket::DoneRecvReliableUDP(const TMsgSocket::RecvRawMsgResult& result, RequestData* requestData)
{
	TMsgSocket* thisSocket = (TMsgSocket*)(result.theSocket);

	if (result.closed)
		requestData->Done(RecvRawMsgResult(thisSocket, 0, 0, true));

	AutoCrit autoCrit(thisSocket->tagCrit);

	thisSocket->recving = false;

	if (!thisSocket->abortingRecv)
	{
		if (!result.msgLength)
		{
			if (GetTickCount() >= requestData->startTime + requestData->timeout)
				requestData->Done(RecvRawMsgResult(thisSocket, 0, 0, false));
			else
			{
				requestData->ResetTimeout();
				thisSocket->TMsgSocket::SendRawMsg(requestData->outgoingMsgLength, requestData->outgoingMsg, -1, true, true);
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
				RequestData* replyTo = (*itor).second;
				unsigned char* actualMsg = new unsigned char[result.msgLength - 3];
				if (actualMsg)
				{
					memcpy(actualMsg, result.msg + 3, result.msgLength - 3);
					replyTo->Done(RecvRawMsgResult(thisSocket, actualMsg, result.msgLength-3, false));
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


void TMsgSocket::IssueRecv()
{
	TimeoutSet::iterator itor = timeoutSet.begin();
	if (itor != timeoutSet.end())
	{
		recving = true;

		RequestData* requestData = *itor;

		void (*DoneRecvReliableUDPProc)(const TMsgSocket::RecvRawMsgResult&, RequestData*) = DoneRecvReliableUDP;

		unsigned long elasped = GetTickCount() - requestData->tryStartTime;
		long retryTimeout = (elasped >= requestData->retryTimeout) ? 0 : requestData->retryTimeout - elasped;
		RecvRawMsgEx((unsigned char**)0, (unsigned long*)0, retryTimeout, true, DoneRecvReliableUDPProc, requestData);
	}
}


void TMsgSocket::DoneOpen(const TMsgSocket::OpenResult& result, RequestData* udpData)
{
	TMsgSocket* thisSocket = (TMsgSocket*)(result.theSocket);

	AutoCrit autoCrit(thisSocket->tagCrit);

	if (result.error != Error_Success)
	{
		udpData->Done(RecvRawMsgResult(thisSocket, 0, 0, true));
	}
	else
	{
		udpData->ResetTimeout();
		thisSocket->SendRawMsg(udpData->outgoingMsgLength, udpData->outgoingMsg, -1, true, true);
		thisSocket->IssueRecv();
	}
}


void TMsgSocket::DoneStreamReqRecv(const TMsgSocket::RecvRawMsgResult& result, RequestData* requestData)
{
	requestData->Done(result);
}


void TMsgSocket::DoneStreamOpen(const TMsgSocket::OpenResult& result, RequestData* requestData)
{
	TMsgSocket* thisSocket = (TMsgSocket*)(result.theSocket);

	if (result.error != Error_Success)
		requestData->Done(RecvRawMsgResult(thisSocket, 0, 0, true));
	else
	{
		thisSocket->SendRawMsg(requestData->outgoingMsgLength, requestData->outgoingMsg, -1, true, true);
		thisSocket->RecvRawMsgEx((unsigned char**)0, (unsigned long*)0, requestData->timeout, true, DoneStreamReqRecv, requestData);
	}
}


unsigned char* TMsgSocket::SendRawMsgRequest(unsigned long length, const void* req, unsigned char** replyBuf,
										  unsigned long* replyLength, unsigned long retryTimeout,
										  long timeout, bool async, bool copyData,
										  const CompletionContainer<const RecvRawMsgResult&>& completion)
{
	if (!req || !length)
	{
		completion.Complete(RecvRawMsgResult(this, 0, 0, false));
		return 0;
	}

	RequestData* requestData = new RequestData;
	if (!requestData)
	{
		completion.Complete(RecvRawMsgResult(this, 0, 0, false));
		return 0;
	}

	unsigned char* theReq = (unsigned char*)req;
	if (copyData)
	{
		theReq = new unsigned char[length];
		if (!theReq)
		{
			delete requestData;
			completion.Complete(RecvRawMsgResult(this, 0, 0, false));
			return 0;
		}
		memcpy(theReq, req, length);
	}

	requestData->deallocReq = copyData;
	requestData->completion = completion;
	requestData->replyMsg = replyBuf ? replyBuf : &(requestData->tempReplyMsg);
	requestData->replyLen = replyLength;
	requestData->autoDel = async;


	if (actualSocket->GetType() == datagram)
	{
		unsigned char* newMsg = new unsigned char[length+4];
		if (!newMsg)
		{
			delete requestData;
			completion.Complete(RecvRawMsgResult(this, 0, 0, false));
			return 0;
		}

		AutoCrit autoCrit(tagCrit);

		// If we're currently authenticating, then do everything, and the recv will be issued when auth is done
		// when it's done
		// ...   If we're already recving, cancel the recv, and it'll be triggered after authentication

		if (recving)
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
		*newMsg = HeaderWithTagAndExpiration;	// 14
		*(unsigned short*)(newMsg + 1) = getLittleEndian(tagSeqNum);

		unsigned long realTimeout = (timeout > 255000 || timeout < 0) ? 255000 : timeout;

		// Add a second to the expiration timeout, just in case
		unsigned long tmpTimeout = (realTimeout / 1000) + 1;
		*(newMsg + 3) = (unsigned char)(tmpTimeout > 0xFF ? 0xFF : tmpTimeout);

		if (copyData)
			delete theReq;

		requestData->retryTimeout = retryTimeout;
		requestData->timeout = realTimeout;
		requestData->startTime = GetTickCount();
		requestData->tryStartTime = requestData->startTime;
		requestData->outgoingMsg = newMsg;
		requestData->outgoingMsgLength = length+4;
		requestData->thisSocket = this;
		requestData->usingDatagrams = true;
		requestData->deallocReq = true;

		requestData->tagMapItor = tagMap.insert(TagMap::value_type(tagSeqNum, requestData)).first;
		requestData->timeoutSetItor = timeoutSet.insert(requestData);

		tagSeqNum++;
		if (!tagSeqNum)	// zero is reserved
			tagSeqNum = 1;

		void (*DoneOpenProc)(const TMsgSocket::OpenResult&, RequestData*) = TMsgSocket::DoneOpen;
		OpenEx(0, timeout, true, DoneOpenProc, requestData);

		autoCrit.Leave();

	}
	else
	{
		requestData->timeout = timeout;
		requestData->outgoingMsg = theReq;
		requestData->outgoingMsgLength  = length;
		requestData->usingDatagrams = false;

		void (*DoneStreamOpenProc)(const TMsgSocket::OpenResult&, RequestData*) = TMsgSocket::DoneStreamOpen;
		OpenEx(0, timeout, true, DoneStreamOpenProc, requestData);
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



class SendRequestData
{
public:
	CompletionContainer<const TMsgSocket::RecvBaseMsgResult&> completion;
	BaseMessage* tmpReply;
	BaseMessage** reply;
	Event doneEvent;
	bool autoDel;

	void Done(const TMsgSocket::RecvBaseMsgResult& result)
	{
		completion.Complete(result);

		if (autoDel)
			delete this;
		else
			doneEvent.Set();
	}
};


static void DoneSendRequest(const TMsgSocket::RecvRawMsgResult& result, SendRequestData* data)
{
	BaseMessage* baseMsg = 0;

	if (result.msg)
		baseMsg = TMsgSocket::ExtractMessage(result.msg, result.msgLength);

	*(data->reply) = baseMsg;

	data->Done(TMsgSocket::RecvBaseMsgResult(result.theSocket, baseMsg, result.closed));

	if (result.msg)
		delete result.msg;
}


BaseMessage* TMsgSocket::SendBaseMsgRequest(WONMsg::BaseMessage& req, WONMsg::BaseMessage** reply,
									 unsigned long retryTimeout, long timeout, bool async, bool copyData,
									 const CompletionContainer<const RecvBaseMsgResult&>& completion)
{
	SendRequestData* data = new SendRequestData();
	if (!data)
	{
		completion.Complete(RecvBaseMsgResult(this, 0, false));
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
		completion.Complete(RecvBaseMsgResult(this, 0, false));
		return 0;
	}

	unsigned long l = req.GetDataLen();
	void* dataPtr = req.GetDataPtr();

	SendRawMsgRequestEx(l, dataPtr, 0, 0, retryTimeout, timeout, true, copyData, DoneSendRequest, data);

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


class TMsgSocket::ReliableRecvRawMsgData
{
public:
	CompletionContainer<const ReliableRecvRawMsgResult&> completion;
	unsigned short* tagNum;
	unsigned short tmpTag;
	unsigned char* result;
	unsigned char** recvMsgBuf;
	unsigned long* length;
	long timeout;
	Event doneEvent;
	bool autoDel;

	void Done(const RecvRawMsgResult& result)
	{
		completion.Complete(ReliableRecvRawMsgResult(*tagNum, result));

		if (autoDel)
			delete this;
		else
			doneEvent.Set();
	}

	ReliableRecvRawMsgData()
	{
	}
};


void TMsgSocket::DoneReliableRecvRawMsg(const RecvRawMsgResult& result, ReliableRecvRawMsgData* reliableRecvRawMsgData)
{
	TMsgSocket* thisSocket = result.theSocket;

	if (!result.msg || *(result.msg) != HeaderWithTagAndExpiration)	// 14
	{
		reliableRecvRawMsgData->Done(result);
		return;
	}

	RecvRawMsgResult newResult(result);

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
			thisSocket->SendRawMsg(tagInfo->msgLength, tagInfo->msg, -1, true, true);
			CompletionWithContents<const RecvRawMsgResult&, ReliableRecvRawMsgData*>* cmplt = new CompletionWithContents<const RecvRawMsgResult&, ReliableRecvRawMsgData*>(TMsgSocket::DoneReliableRecvRawMsg, reliableRecvRawMsgData, true);
			thisSocket->RecvRawMsg(reliableRecvRawMsgData->recvMsgBuf, reliableRecvRawMsgData->length, reliableRecvRawMsgData->timeout, true, cmplt);

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


unsigned char* TMsgSocket::ReliableRecvRawMsg(unsigned short* tagNum, unsigned char** recvMsgBuf,
											  unsigned long* length, long timeout, bool async,
											  const CompletionContainer<const ReliableRecvRawMsgResult&>& completion)
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
		completion.Complete(ReliableRecvRawMsgResult(0, this, 0, 0, false));
		return 0;
	}

	reliableRecvRawMsgData->completion = completion;
	reliableRecvRawMsgData->tagNum = tagNum ? tagNum : &(reliableRecvRawMsgData->tmpTag);
	reliableRecvRawMsgData->autoDel = async;
	reliableRecvRawMsgData->result = 0;
	reliableRecvRawMsgData->recvMsgBuf = recvMsgBuf;
	reliableRecvRawMsgData->length = length;
	reliableRecvRawMsgData->timeout = timeout;
	
	void (*DoneReliableRecvRawMsgProc)(const RecvRawMsgResult&, ReliableRecvRawMsgData*) = DoneReliableRecvRawMsg;

	CompletionWithContents<const RecvRawMsgResult&, ReliableRecvRawMsgData*>* cmplt
		= new CompletionWithContents<const RecvRawMsgResult&, ReliableRecvRawMsgData*>(DoneReliableRecvRawMsgProc, reliableRecvRawMsgData, true);
	RecvRawMsg(recvMsgBuf, length, timeout, true, cmplt);

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
	CompletionContainer<const TMsgSocket::ReliableRecvBaseMsgResult&> completion;
	bool autoDelete;
	bool socketClosed;
	BaseMessage* msg;
	BaseMessage** recvMsg;
	TMsgSocket* thisSocket;
	unsigned short tag;

	void Done()
	{
		completion.Complete(TMsgSocket::ReliableRecvBaseMsgResult(tag, thisSocket, msg, socketClosed));

		if (autoDelete)
			delete this;
	}
};


static void DoneReliableRecvBaseMsg(const TMsgSocket::ReliableRecvRawMsgResult& result, ReliableRecvBaseMsgData* reliableRecvBaseMsgData)
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
	reliableRecvBaseMsgData->Done();
}


BaseMessage* TMsgSocket::ReliableRecvBaseMsg(unsigned short* tagNum, BaseMessage** recvMsg,
											 long timeout, bool async,
											 const CompletionContainer<const ReliableRecvBaseMsgResult&>& completion)
{
	ReliableRecvBaseMsgData* reliableRecvBaseMsgData = new ReliableRecvBaseMsgData;

	if (!reliableRecvBaseMsgData)
	{
		completion.Complete(ReliableRecvBaseMsgResult(0, this, 0, false));
		return 0;
	}

	reliableRecvBaseMsgData->completion = completion;
	reliableRecvBaseMsgData->autoDelete = async;
	reliableRecvBaseMsgData->recvMsg = recvMsg;
	reliableRecvBaseMsgData->thisSocket = this;

	ReliableRecvRawMsgEx(tagNum, 0, 0, timeout, async, DoneReliableRecvBaseMsg, reliableRecvBaseMsgData);

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
	CompletionContainer<const TMsgSocket::SendRawMsgResult&> completion;
};


static void DoneReliableSendRawMsg(const TMsgSocket::SendRawMsgResult& result, ReliableSendRawMsgData* reliableSendRawMsgData)
{
	delete reliableSendRawMsgData->newMsg;
	reliableSendRawMsgData->completion.Complete(TMsgSocket::SendRawMsgResult(result.theSocket, reliableSendRawMsgData->origMsg, result.msgLength - 3, result.success, result.closed));
	delete reliableSendRawMsgData;
}


bool TMsgSocket::ReliableSendRawMsgReply(unsigned short tagNum, unsigned long length, const void* msg, long timeout,
									bool async, bool copyData, const CompletionContainer<const SendRawMsgResult&>& completion)
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
		completion.Complete(SendRawMsgResult(this, (unsigned char*)msg, length, false, false));
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
		reliableSendRawMsgData->newMsg = 0;
	}
	else
	{
		reliableSendRawMsgData->newMsg = newMsg;
		// couldn't find tag in line.  Might have already expired.  Send it anyway
	}

	reliableSendRawMsgData->completion = completion;
	reliableSendRawMsgData->origMsg = (unsigned char*)msg;

	return SendRawMsgEx(length+3, newMsg, timeout, async, false, DoneReliableSendRawMsg, reliableSendRawMsgData);
}


bool TMsgSocket::ReliableSendBaseMsgReply(unsigned short tagNum, WONMsg::BaseMessage& msg, long timeout,
									 bool async, bool copyData, const CompletionContainer<const SendBaseMsgResult&>& completion)
{
	SendBaseMsgData* sendBaseMsgData = new SendBaseMsgData;
	if (!sendBaseMsgData)
	{
		completion.Complete(SendBaseMsgResult(this, &msg, false, false));
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
		completion.Complete(SendBaseMsgResult(this, &msg, false, false));
		return false;
	}

	return ReliableSendRawMsgReplyEx(tagNum, msg.GetDataLen(), msg.GetDataPtr(), timeout, async, copyData, DoneSendBaseMsg, sendBaseMsgData);
}



