#pragma warning (disable : 4786)
#ifdef WIN32
#include <crtdbg.h> // FIX: remove eventually
#endif
#include <algorithm>
#include "Authentication/AuthSocket.h"
#include "RoutingAPI.h"
#include "RoutingMessages.h"
#include "msg/Comm/MMsgCommGetNumUsers.h"
#include "msg/Comm/MMsgCommIsUserPresent.h"

using namespace WONCommon;
using namespace WONAPI;
//using RoutingServerClient::ClientId;

const char* IGNOREFILE = "ignored.lst";

// RoutingServerClient::RoutingServerClient
// Constructs a new RoutingServerClient object.  Initializes the message handler maps 
RoutingServerClient::RoutingServerClient(bool useTCP, bool enableShortcuts, bool activateIgnoring, unsigned char theLengthFieldSize) :
	mIsTCP(useTCP),
	mEnableShortcuts(enableShortcuts),
	mIgnoringIsOn(activateIgnoring),
	mLengthFieldSize(theLengthFieldSize),
	mClientId(0), 
	mHostId(0), 
	mSocketP(NULL)
{
	InitMessageHandlers();
	if (mIgnoringIsOn) 
		LoadIgnoredUsersList();
}


// RoutingServerClient::~RoutingServerClient
RoutingServerClient::~RoutingServerClient()
{
	// don't bother disconnecting (server will notice socket closure), however set ClientId
	// to 0 so when socket closes, we won't try to reconnect.
	mClientId = 0;
	Close();

	if (mIgnoringIsOn)
		SaveIgnoredUsersList();
}

// RoutingServerClient::SocketClosed
// Called by socket level when mActualSocketP closes.  Attempts to reconnect if (before the close)
// the client had registered and not done a permanent disconnect.
void RoutingServerClient::SocketClosed(const Socket::Result& result, RoutingServerClient* that)
{
	if (result.theSocket == that->mSocketP)	// Ignore close of discarded socket
	{
		if (that->mClientId == 0)
		{
			that->OnReconnectFailure();
			if (!that->mReconnectFailureCatcher.empty())
				that->mReconnectFailureCatcher.Complete();
		}
		else
			that->ConnectEx(that->mRoutingAddress, &(that->mCurIdentity), true, 5000, true, ReconnectAttemptCallback, that);
	}
}

// RoutingServerClient::ReconnectAttemptCallback
// Callback for reconnects (see SocketClosed).  Note that here we're talking about 
// actually attempting to reestablish the connection, not a ReconnectClient request.
void RoutingServerClient::ReconnectAttemptCallback(Error error, RoutingServerClient* that)
{
	if (error == Error_Success)
		that->ReconnectEx(true, (void(*)(short, RoutingServerClient*))ReconnectRequestCallback, that);
	else
	{
		that->OnReconnectFailure();
		if (!that->mReconnectFailureCatcher.empty())
			that->mReconnectFailureCatcher.Complete();
	}
}

// RoutingServerClient::ReconnectRequestCallback
// Callback for Reconnect requests when trying to automatically reattach to server.
void RoutingServerClient::ReconnectRequestCallback(short theStatus, RoutingServerClient* that)
{
	if (theStatus != Error_Success)
	{
		that->OnReconnectFailure();
		if (!that->mReconnectFailureCatcher.empty())
			that->mReconnectFailureCatcher.Complete();
	}
}

// RoutingServerClient::Connect (just Routing)
// Attempts to connect to a Routing Server at the specified address.
Error RoutingServerClient::Connect(const TCPSocket::Address& theRoutingAddress, Identity* theIdentityP, bool isReconnectAttempt, long theTimeout, bool async, const CompletionContainer<Error>& completion)
{
	// If we're already connected, close that connection.
	Close();
	
	if (mIsTCP)
	{
		TCPSocket* actualSocket = new TCPSocket(theRoutingAddress);
		if (!actualSocket)
		{
			completion.Complete(Error_OutOfMemory); 
			return Error_OutOfMemory;
		}
		mSocketP = new AuthSocket(theIdentityP, actualSocket, true, mLengthFieldSize, true, true, false, false);
		if (!mSocketP)
		{
			delete actualSocket;
			completion.Complete(Error_OutOfMemory); 
			return Error_OutOfMemory;
		}
	}
	else
	{
		UDPSocket* actualSocket = new UDPSocket(theRoutingAddress);
		if (!actualSocket)
		{
			completion.Complete(Error_OutOfMemory); 
			return Error_OutOfMemory;
		}
		if (theIdentityP)
		{
			// Needs to be updated to call appropriate constructor for AuthSocket, provide correct TCP port for authentication
			mSocketP = 0;//new AuthSocket(theIdentityP, actualSocket, true, mLengthFieldSize, true);
		}
		else
			mSocketP = new AuthSocket(theIdentityP, actualSocket, true, theRoutingAddress, mLengthFieldSize, true, false, false);
		if (!mSocketP)
		{
			delete actualSocket;
			completion.Complete(Error_OutOfMemory); 
			return Error_OutOfMemory;
		}
	}

	if (theIdentityP)
		mCurIdentity = *theIdentityP;
	else
		mCurIdentity.Invalidate();

	// setup completion routine to be called when socket closes
	mSocketP->CatchCloseEx((void(*)(const Socket::Result&, RoutingServerClient*))SocketClosed, this, true);

	mRoutingAddress = theRoutingAddress;

	ConnectData* aConnectDataP = new ConnectData;
	aConnectDataP->mRoutingServerClientP = this;
	aConnectDataP->mFinalCompletion = completion;
	aConnectDataP->mTimeout = theTimeout;
	Error aError = Error_Pending;
	aConnectDataP->mAutoDelete = async;
	aConnectDataP->mStatusP = async ? NULL : &aError;
	
	mSocketP->OpenEx(false, theTimeout, true, (void(*)(const Socket::OpenResult&, ConnectData*))OpenCompletion, aConnectDataP);

	if (!async)
	{
		WSSocket::PumpUntil(aConnectDataP->mDoneEvent, theTimeout);
		delete aConnectDataP;
	}

	return aError;
}

void RoutingServerClient::OpenCompletion(const Socket::OpenResult& theResultR, ConnectData* theConnectDataP)
{
	if (theResultR.error != Error_Success)
	{
		if (theConnectDataP->mStatusP)
			*theConnectDataP->mStatusP = theResultR.error;
		theConnectDataP->mFinalCompletion.Complete(theResultR.error);
		if (theConnectDataP->mAutoDelete)
			delete theConnectDataP;
		else
			theConnectDataP->mDoneEvent.Set();
		return;
	}

	theConnectDataP->mRoutingServerClientP->mSocketP->AuthenticateEx((bool)false, theConnectDataP->mTimeout, true, (void(*)(const AuthSocket::AuthResult&, ConnectData*))AuthCompletion, theConnectDataP);
}

void RoutingServerClient::AuthCompletion(const AuthSocket::AuthResult& theResultR, ConnectData* theConnectDataP)
{
	if (theConnectDataP->mStatusP)
		*theConnectDataP->mStatusP = theResultR.error;

	if (theResultR.error != Error_Success)
	{
		theConnectDataP->mFinalCompletion.Complete(theResultR.error);
		if (theConnectDataP->mAutoDelete)
			delete theConnectDataP;
		else
			theConnectDataP->mDoneEvent.Set();
		return;
	}

	AuthSocket::AuthRecvBaseMsgResult aFakeResult(theConnectDataP->mRoutingServerClientP->mSocketP, NULL, false);
	ReceiveLoop(aFakeResult, theConnectDataP->mRoutingServerClientP);

	theConnectDataP->mFinalCompletion.Complete(Error_Success);

	if (theConnectDataP->mAutoDelete)
		delete theConnectDataP;
	else
		theConnectDataP->mDoneEvent.Set();

}

// RoutingServerClient::Close
// Forces the client's connection to the server closed.
void RoutingServerClient::Close()
{
	AuthSocket* deleteMe = mSocketP;
	mSocketP = 0;
	delete deleteMe;	// Circumvent reconnect

	mClientId = 0;
	mRequestCompletionListCrit.Enter();
	while (!mRequestCompletionList.empty())
	{
		RequestCompletionPair aRequestCompletionPair = mRequestCompletionList.front();
		mRequestCompletionList.pop_front();
		delete aRequestCompletionPair.second;
	}
	mRequestCompletionList.clear();
	mRequestCompletionListCrit.Leave();

	mIgnoredClientIds.clear();
}

// RoutingServerClient::ReceiveLoop (static)
// Called whenever a message is received on mActualSocketP.  Processes the message and then
// initiates another async recv with itself as the completion routine.
void RoutingServerClient::ReceiveLoop(const AuthSocket::AuthRecvBaseMsgResult& result, RoutingServerClient* that)//ClientBoolPair* theClientBoolPairP)
{
	if (result.msg)
	{
		unsigned char aServiceType = result.msg->GetServiceType();
		unsigned char aMessageType = result.msg->GetMessageType();
		if (aServiceType == WONMsg::MiniRoutingServer || aServiceType == WONMsg::MiniCommonService)
		{
			if (that->IsReplyMessage(aMessageType))
			{
				that->mRequestCompletionListCrit.Enter();
				if (that->mRequestCompletionList.empty())
					_ASSERT(false); // Bug: server returned reply message, but we didn't send a request?
				RequestCompletionPair aRequestCompletionPair = that->mRequestCompletionList.front();
				that->mRequestCompletionList.pop_front();
				Handler handler = that->mReplyHandlerVector[aRequestCompletionPair.first];
				that->mRequestCompletionListCrit.Leave();
				if (handler)
					(that->*handler)(result);

				// call user specified completion (if any)
				if (!aRequestCompletionPair.second->empty())
				{
					if (aServiceType == WONMsg::MiniCommonService)
					{
						switch (aMessageType)
						{
							case WONMsg::MiniCommGetNumUsersReply:
							{
								MMsgCommGetNumUsersReply aGetNumUsersReply;
								FillMsgFromResult(aGetNumUsersReply, result);
								GetNumUsersResult aGetNumUsersResult(aGetNumUsersReply);
								reinterpret_cast<CompletionContainer<const GetNumUsersResult&>*>(aRequestCompletionPair.second)->Complete(aGetNumUsersResult);
								break;
							}
							case WONMsg::MiniCommIsUserPresentReply:
							{
								MMsgCommIsUserPresentReply aIsUserPresentReply;
								FillMsgFromResult(aIsUserPresentReply, result);
								reinterpret_cast<CompletionContainer<bool>*>(aRequestCompletionPair.second)->Complete(aIsUserPresentReply.GetIsUserPresent());
								break;
							}
						}
					}
					else if (aServiceType == WONMsg::MiniRoutingServer)
					{
						switch (aMessageType)
						{
							case WONMsg::RoutingStatusReply:
							{
								MMsgRoutingStatusReply aStatusReply;
								FillMsgFromResult(aStatusReply, result);
	// FIX: Colen doesn't want to use RTTI, but it would be really cool to be able to catch errors
	//      (my errors) when these completion objects are not what they should be...
								reinterpret_cast<CompletionContainer<short>*>(aRequestCompletionPair.second)->Complete(aStatusReply.GetStatus());
								break;
							}
							case WONMsg::RoutingRegisterClientReply:
							{
								MMsgRoutingRegisterClientReply aRegisterClientReply;
								FillMsgFromResult(aRegisterClientReply, result);
								RegisterClientResult aRegisterClientResult(aRegisterClientReply);
								reinterpret_cast<CompletionContainer<const RegisterClientResult&>*>(aRequestCompletionPair.second)->Complete(aRegisterClientResult);
								break;
							}
							case WONMsg::RoutingCreateGroupReply:
							{
								MMsgRoutingCreateGroupReply aCreateGroupReply;
								FillMsgFromResult(aCreateGroupReply, result);
								CreateGroupResult aCreateGroupResult(aCreateGroupReply);
								reinterpret_cast<CompletionContainer<const CreateGroupResult&>*>(aRequestCompletionPair.second)->Complete(aCreateGroupResult);
								break;
							}
							case WONMsg::RoutingGetClientListReply:
							{
								MMsgRoutingGetClientListReply aGetClientListReply;
								FillMsgFromResult(aGetClientListReply, result);
								switch (aRequestCompletionPair.first)
								{
									case WONMsg::RoutingGetClientList:
									case WONMsg::RoutingGetMembersOfGroup:
									{
										GetClientListResult aGetClientListResult(aGetClientListReply);
										reinterpret_cast<CompletionContainer<const GetClientListResult&>*>(aRequestCompletionPair.second)->Complete(aGetClientListResult);
										break;
									}
									case WONMsg::RoutingGetClientInfo:
									{
										GetClientInfoResult aGetClientInfoResult(aGetClientListReply);
										reinterpret_cast<CompletionContainer<const GetClientInfoResult&>*>(aRequestCompletionPair.second)->Complete(aGetClientInfoResult);
										break;
									}
									default:
										_ASSERT(false);
								}
								break;
							}
							case WONMsg::RoutingGetGroupListReply:
							{
								MMsgRoutingGetGroupListReply aGetGroupListReply;
								FillMsgFromResult(aGetGroupListReply, result);
								GetGroupListResult aGetGroupListResult(aGetGroupListReply);
								reinterpret_cast<CompletionContainer<const GetGroupListResult&>*>(aRequestCompletionPair.second)->Complete(aGetGroupListResult);
								break;
							}
							case WONMsg::RoutingGetSimpleClientListReply:
							{
								MMsgRoutingGetSimpleClientListReply aGetSimpleClientListReply;
								FillMsgFromResult(aGetSimpleClientListReply, result);
								GetSimpleClientListResult aGetSimpleClientListResult(aGetSimpleClientListReply);
								reinterpret_cast<CompletionContainer<const GetSimpleClientListResult&>*>(aRequestCompletionPair.second)->Complete(aGetSimpleClientListResult);
								break;
							}
							case WONMsg::RoutingReadDataObjectReply:
							{
								MMsgRoutingReadDataObjectReply aReadDataObjectReply;
								FillMsgFromResult(aReadDataObjectReply, result);
								ReadDataObjectResult aReadDataObjectResult(aReadDataObjectReply);
								reinterpret_cast<CompletionContainer<const ReadDataObjectResult&>*>(aRequestCompletionPair.second)->Complete(aReadDataObjectResult);
								break;
							}
						}
					}

					delete aRequestCompletionPair.second;
				}
			}
			else if (that->IsAsyncMessage(aMessageType))
			{
				Handler handler = that->mAsyncMsgHandlerVector[aMessageType];
//				that->mRequestCompletionListCrit.Enter();
				if (handler)
					(that->*handler)(result);
//				that->mRequestCompletionListCrit.Leave();
			}
		}
	}

	// Note: According to the Creator (aka Colen), result.msg might be NULL if:
	//  o the timeout expired
	//  o the AuthSocket received an invalid message (encrypted when not handling encryption, or vice versa)

	// initiate a new recv that calls this routine again on completion
	if (!result.closed)
		that->mSocketP->RecvBaseMsgEx(NULL, true, -1, true, ReceiveLoop, that);

	delete result.msg;
}

// RoutingServerClient::Register
// Sends an MMsgRoutingRegisterClient request.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::Register(const ClientName& theClientNameR, const std::string& thePasswordR, bool becomeHost, bool becomeSpectator, bool joinChat, const CompletionContainer<const RegisterClientResult&>& completion)
{ Register(theClientNameR, WONCommon::StringToWString(thePasswordR), becomeHost, becomeSpectator, joinChat, completion); }

void RoutingServerClient::Register(const ClientName& theClientNameR, const Password& thePasswordR, bool becomeHost, bool becomeSpectator, bool joinChat, const CompletionContainer<const RegisterClientResult&>& completion)
{
	if (mEnableShortcuts && mClientId != 0) {
		completion.Complete(RegisterClientResult(WONMsg::StatusRouting_ClientAlreadyRegistered));
		return;
	}
	
	mClientName = theClientNameR; 

	MMsgRoutingRegisterClient aRegisterClientMsg;
	aRegisterClientMsg.SetClientName(theClientNameR);
	aRegisterClientMsg.SetPassword(thePasswordR);
	aRegisterClientMsg.SetTryingToBecomeHost(becomeHost);
	aRegisterClientMsg.SetBecomeSpectator(becomeSpectator);
	aRegisterClientMsg.SetSetupChat(joinChat);

	SendMessageToServer(aRegisterClientMsg, WONMsg::RoutingRegisterClient, new CompletionContainer<const RegisterClientResult&>(completion));
}

RoutingServerClient::ClientId RoutingServerClient::GetClientId() const
{ return mClientId; }

bool RoutingServerClient::IsRegistered() const
{ return mClientId != 0; }

// RoutingServerClient::Disconnect
// Sends an MMsgRoutingDisconnectClient request.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::Disconnect(bool isPermanent, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeClient);
		return;
	}
	
	MMsgRoutingDisconnectClient aDisconnectClientMsg;
	aDisconnectClientMsg.SetIsPermanent(isPermanent);
	SendMessageToServer(aDisconnectClientMsg, WONMsg::RoutingDisconnectClient, new CompletionContainer<short>(completion));
}

// RoutingServerClient::Reconnect
// Sends an MMsgRoutingReconnectClient request.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::Reconnect(bool getMissedMessages, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_ReconnectImpossible);
		return;
	}
	
	MMsgRoutingReconnectClient aReconnectClientMsg;
	aReconnectClientMsg.SetClientId(mClientId);
	aReconnectClientMsg.SetWantMissedMessages(getMissedMessages);
	SendMessageToServer(aReconnectClientMsg, WONMsg::RoutingReconnectClient, new CompletionContainer<short>(completion));
}

// RoutingServerClient::GetClientList
// Sends an MMsgRoutingGetClientList request.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::GetClientList(bool requestAuthInfo, bool requestMuteFlag, bool requestModeratorFlag, const CompletionContainer<const GetClientListResult&>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(*(new GetClientListResult(WONMsg::StatusRouting_MustBeClient)));
		return;
	}
	
	MMsgRoutingGetClientList aGetClientListMsg;
	aGetClientListMsg.SetAuthInfoRequested(requestAuthInfo);
	aGetClientListMsg.SetMuteFlagRequested(requestMuteFlag);
	aGetClientListMsg.SetModeratorFlagRequested(requestModeratorFlag);
	SendMessageToServer(aGetClientListMsg, WONMsg::RoutingGetClientList, new CompletionContainer<const GetClientListResult&>(completion));
}

// RoutingServerClient::GetClientInfo
// Sends an MMsgRoutingGetClientInfo request.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::GetClientInfo(RoutingServerClient::ClientId theClientId, bool requestAuthInfo, bool requestMuteFlag, bool requestModeratorFlag, const CompletionContainer<const GetClientInfoResult&>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(GetClientInfoResult(WONMsg::StatusRouting_MustBeClient));
		return;
	}
	
	MMsgRoutingGetClientInfo aGetClientInfoMsg;
	aGetClientInfoMsg.SetClientId(theClientId);
	aGetClientInfoMsg.SetAuthInfoRequested(requestAuthInfo);
	aGetClientInfoMsg.SetMuteFlagRequested(requestMuteFlag);
	aGetClientInfoMsg.SetModeratorFlagRequested(requestModeratorFlag);
	SendMessageToServer(aGetClientInfoMsg, WONMsg::RoutingGetClientInfo, new CompletionContainer<const GetClientInfoResult&>(completion));
}

// RoutingServerClient::GetGroupList
// Sends an MMsgRoutingGetGroupList request.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::GetGroupList(const CompletionContainer<const GetGroupListResult&>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(GetGroupListResult(WONMsg::StatusRouting_MustBeClient));
		return;
	}
	
	MMsgRoutingGetGroupList aGetGroupListMsg;
	SendMessageToServer(aGetGroupListMsg, WONMsg::RoutingGetGroupList, new CompletionContainer<const GetGroupListResult&>(completion));
}

// RoutingServerClient::GetMembersOfGroup
// Sends an MMsgRoutingGetMembersOfGroup request.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::GetMembersOfGroup(GroupId theGroupId, bool requestAuthInfo, bool requestMuteFlag, bool requestModeratorFlag, const CompletionContainer<const GetClientListResult&>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(GetClientListResult(WONMsg::StatusRouting_MustBeClient));
		return;
	}
	
	MMsgRoutingGetMembersOfGroup aGetMembersOfGroupMsg;
	aGetMembersOfGroupMsg.SetGroupId(theGroupId);
	aGetMembersOfGroupMsg.SetAuthInfoRequested(requestAuthInfo);
	aGetMembersOfGroupMsg.SetMuteFlagRequested(requestMuteFlag);
	aGetMembersOfGroupMsg.SetModeratorFlagRequested(requestModeratorFlag);
	SendMessageToServer(aGetMembersOfGroupMsg, WONMsg::RoutingGetMembersOfGroup, new CompletionContainer<const GetClientListResult&>(completion));
}

// RoutingServerClient::GetSimpleClientList
// Sends an MMsgRoutingGetSimpleClientList.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::GetSimpleClientList(unsigned char theClientListType, const CompletionContainer<const GetSimpleClientListResult&>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(GetSimpleClientListResult(WONMsg::StatusRouting_MustBeClient));
		return;
	}
	
	MMsgRoutingGetSimpleClientList aGetSimpleClientListMsg;
	aGetSimpleClientListMsg.SetClientListType(theClientListType);
	SendMessageToServer(aGetSimpleClientListMsg, WONMsg::RoutingGetSimpleClientList, new CompletionContainer<const GetSimpleClientListResult&>(completion));
}

// RoutingServerClient::BroadcastData
// Sends an MMsgRoutingSendDataBroadcast request.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::BroadcastData(const Message& theMessageR, bool shouldSendReply, const CompletionContainer<short>& completion)
{ BroadcastData(RawBuffer(theMessageR.mBufP, theMessageR.mBufLen), shouldSendReply, completion); }

void RoutingServerClient::BroadcastData(const RawBuffer& theMessageR, bool shouldSendReply, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeClient);
		return;
	}
	
	MMsgRoutingSendDataBroadcast aSendDataBroadcastMsg;
	aSendDataBroadcastMsg.SetData(theMessageR);
	aSendDataBroadcastMsg.SetShouldSendReply(shouldSendReply);	

	if (shouldSendReply)
		SendMessageToServer(aSendDataBroadcastMsg, WONMsg::RoutingSendDataBroadcast, new CompletionContainer<short>(completion));
	else
		SendMessageToServer(aSendDataBroadcastMsg);
}

// RoutingServerClient::SendData
// Sends an MMsgRoutingSendData request.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::SendData(const ClientOrGroupId theRecipients[], unsigned short theNumRecipients, bool flagIncludeExclude, const Message& theMessageR, bool shouldSendReply, const CompletionContainer<short>& completion)
{
	SendData(theRecipients, theNumRecipients, flagIncludeExclude, RawBuffer(theMessageR.mBufP, theMessageR.mBufLen), shouldSendReply, completion);
}
void RoutingServerClient::SendData(const ClientOrGroupId theRecipients[], unsigned short theNumRecipients, bool flagIncludeExclude, const RawBuffer& theMessageR, bool shouldSendReply, const CompletionContainer<short>& completion)
{
	std::list<ClientOrGroupId> aRecipientList;
	for (int i = 0; i < theNumRecipients; ++i)
		aRecipientList.push_back(*(theRecipients + i));

	SendData(aRecipientList, flagIncludeExclude, theMessageR, shouldSendReply, completion);
}
void RoutingServerClient::SendData(const std::list<ClientOrGroupId>& theRecipientListR, bool flagIncludeExclude, const RawBuffer& theMessageR, bool shouldSendReply, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeClient);
		return;
	}
	
	MMsgRoutingSendData aSendDataMsg;
	aSendDataMsg.SetIncludeExcludeFlag(flagIncludeExclude);
	aSendDataMsg.SetData(theMessageR);
	aSendDataMsg.SetShouldSendReply(shouldSendReply);
	aSendDataMsg.SetAddresseeList(theRecipientListR);
	
	if (shouldSendReply)
		SendMessageToServer(aSendDataMsg, WONMsg::RoutingSendData, new CompletionContainer<short>(completion));
	else
		SendMessageToServer(aSendDataMsg);
}

// RoutingServerClient::SendDataMultiple
// Sends an MMsgRoutingSendDataMultiple request.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::SendDataMultiple(const ClientOrGroupId theRecipients[], unsigned short theNumRecipients, bool flagIncludeExclude, const Message theMessages[], unsigned short theNumMessages, bool shouldSendReply, const CompletionContainer<short>& completion)
{
	std::list<RawBuffer> aMessageList;
	int i;
	for (i = 0; i < theNumMessages; ++i)
		aMessageList.push_back(RawBuffer((theMessages + i)->mBufP, (theMessages + i)->mBufLen));
	
	std::list<ClientOrGroupId> aRecipientList;
	for (i = 0; i < theNumRecipients; ++i)
		aRecipientList.push_back(*(theRecipients + i));

	SendDataMultiple(aRecipientList, flagIncludeExclude, aMessageList, shouldSendReply, completion);
}

void RoutingServerClient::SendDataMultiple(const std::list<ClientOrGroupId>& theRecipientListR, bool flagIncludeExclude, const std::list<RawBuffer>& theMessageListR, bool shouldSendReply, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeClient);
		return;
	}
	
	MMsgRoutingSendDataMultiple aSendDataMultipleMsg;
	aSendDataMultipleMsg.SetAddresseeList(theRecipientListR);
	aSendDataMultipleMsg.SetIncludeExcludeFlag(flagIncludeExclude);
	aSendDataMultipleMsg.SetMessageList(theMessageListR);
	aSendDataMultipleMsg.SetShouldSendReply(shouldSendReply);	

	if (shouldSendReply)
		SendMessageToServer(aSendDataMultipleMsg, WONMsg::RoutingSendDataMultiple, new CompletionContainer<short>(completion));
	else
		SendMessageToServer(aSendDataMultipleMsg);
}

// RoutingServerClient::BroadcastChat
// Sends an MMsgRoutingSendChat request as a broadcast.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::BroadcastChat(const Message& theMessageR, WONMsg::ChatType theChatType, bool shouldSendReply, const CompletionContainer<short>& completion)
{ BroadcastChat(RawBuffer(theMessageR.mBufP, theMessageR.mBufLen), theChatType, shouldSendReply, completion); }

void RoutingServerClient::BroadcastChat(const string& theMessageR, bool shouldSendReply, const CompletionContainer<short>& completion)
{ BroadcastChat(RawBuffer(reinterpret_cast<const unsigned char*>(theMessageR.data()), theMessageR.size()), WONMsg::CHATTYPE_ASCII, shouldSendReply, completion); }

void RoutingServerClient::BroadcastChat(const wstring& theMessageR, bool shouldSendReply, const CompletionContainer<short>& completion)
{
	std::wstring tmpMsg(theMessageR);
	makeLittleEndianWString(tmpMsg);
	BroadcastChat(RawBuffer(reinterpret_cast<const unsigned char*>(tmpMsg.data()), tmpMsg.size()*2), WONMsg::CHATTYPE_UNICODE, shouldSendReply, completion);
}

void RoutingServerClient::BroadcastChat(const RawBuffer& theMessageR, WONMsg::ChatType theChatType, bool shouldSendReply, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeClient);
		return;
	}
	
	MMsgRoutingSendChat aSendChatMsg;
	aSendChatMsg.SetData(theMessageR);
	aSendChatMsg.SetChatType(theChatType);
	aSendChatMsg.SetIncludeExcludeFlag(false); // exclude everyone
	aSendChatMsg.SetShouldSendReply(shouldSendReply);	

	if (shouldSendReply)
		SendMessageToServer(aSendChatMsg, WONMsg::RoutingSendChat, new CompletionContainer<short>(completion));
	else
		SendMessageToServer(aSendChatMsg);
}

// RoutingServerClient:WhisperChat
// Sends an MMsgRoutingSendChat request as a broadcast.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::WhisperChat(const ClientOrGroupId theRecipients[], unsigned short theNumRecipients, bool flagIncludeExclude, const Message& theMessageR, WONMsg::ChatType theChatType, bool shouldSendReply, const CompletionContainer<short>& completion)
{
	std::list<ClientOrGroupId> aRecipientList;
	for (int i = 0; i < theNumRecipients; ++i)
		aRecipientList.push_back(*(theRecipients + i));

	WhisperChat(aRecipientList, flagIncludeExclude, RawBuffer(theMessageR.mBufP, theMessageR.mBufLen), theChatType, shouldSendReply, completion);
}

void RoutingServerClient::WhisperChat(const std::list<ClientOrGroupId>& theRecipientListR, bool flagIncludeExclude, const string& theMessageR, bool shouldSendReply, const CompletionContainer<short>& completion)
{
	WhisperChat(theRecipientListR, flagIncludeExclude, RawBuffer(reinterpret_cast<const unsigned char*>(theMessageR.data()), theMessageR.size()), WONMsg::CHATTYPE_ASCII, shouldSendReply, completion);
}

void RoutingServerClient::WhisperChat(const std::list<ClientOrGroupId>& theRecipientListR, bool flagIncludeExclude, const wstring& theMessageR, bool shouldSendReply, const CompletionContainer<short>& completion)
{
	std::wstring tmpMsg(theMessageR);
	makeLittleEndianWString(tmpMsg);
	WhisperChat(theRecipientListR, flagIncludeExclude, RawBuffer(reinterpret_cast<const unsigned char*>(tmpMsg.data()), tmpMsg.size()*2), WONMsg::CHATTYPE_UNICODE, shouldSendReply, completion);
}

void RoutingServerClient::WhisperChat(const std::list<ClientOrGroupId>& theRecipientListR, bool flagIncludeExclude, const RawBuffer& theMessageR, WONMsg::ChatType theChatType, bool shouldSendReply, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeClient);
		return;
	}
	
	MMsgRoutingSendChat aSendChatMsg;
	aSendChatMsg.SetData(theMessageR);
	aSendChatMsg.SetChatType(theChatType);
	aSendChatMsg.SetIncludeExcludeFlag(flagIncludeExclude);
	aSendChatMsg.SetAddresseeList(theRecipientListR);
	aSendChatMsg.SetShouldSendReply(shouldSendReply);
	
	if (shouldSendReply)
		SendMessageToServer(aSendChatMsg, WONMsg::RoutingSendChat, new CompletionContainer<short>(completion));
	else
		SendMessageToServer(aSendChatMsg);
}

// RoutingServerClient::IgnoreClient
// Adds the specified ClientName (theClientNameR) to the list of ClientNames that are ignored.
// If the specified client is currently in the room, adds their current ClientId to the set
// of ignored ids.
void RoutingServerClient::IgnoreClient(const ClientName& theClientNameR)
{
	mIgnoredClientNames.insert(theClientNameR);
	ClientNameToIdMap::const_iterator itr = mClientNameToIdMap.find(theClientNameR);
	if (itr != mClientNameToIdMap.end())
		mIgnoredClientIds.insert(itr->second);
}

// RoutingServerClient::UnignoreClient
// Removes the specified clientname (theClientNameR) from the list of ClientNames that are ignored.
// If the specified client is currently in the room, removes their current ClientId from the set
// of ignored ids.
void RoutingServerClient::UnignoreClient(const ClientName& theClientNameR)
{
	mIgnoredClientNames.erase(theClientNameR);
	ClientNameToIdMap::const_iterator itr = mClientNameToIdMap.find(theClientNameR);
	if (itr != mClientNameToIdMap.end())
		mIgnoredClientIds.erase(itr->second);
}

void RoutingServerClient::JoinChat(const CompletionContainer<short>& completion)
{ AddToGroup(mClientId, GROUPID_CHAT, completion); }
void RoutingServerClient::LeaveChat(const CompletionContainer<short>& completion)
{ RemoveFromGroup(mClientId, GROUPID_CHAT, completion); }

// RoutingServerClient::OpenRegistration
// Sends an MMsgRoutingOpenRegistration request.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::OpenRegistration(const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeHost);
		return;
	}
	
	MMsgRoutingOpenRegistration aOpenRegistrationMsg;
	SendMessageToServer(aOpenRegistrationMsg, WONMsg::RoutingOpenRegistration, new CompletionContainer<short>(completion));
}

// RoutingServerClient::CloseRegistration
// Sends an MMsgRoutingCloseRegistration request.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::CloseRegistration(const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeHost);
		return;
	}
	
	MMsgRoutingCloseRegistration aCloseRegistrationMsg;
	SendMessageToServer(aCloseRegistrationMsg, WONMsg::RoutingCloseRegistration, new CompletionContainer<short>(completion));
}

// RoutingServerClient::SetPassword
// Sends an MMsgRoutingSetPassword request.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::SetPassword(const Password& thePasswordR, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeHost);
		return;
	}
	
	MMsgRoutingSetPassword aSetPasswordMsg;
	aSetPasswordMsg.SetNewPassword(thePasswordR);
	SendMessageToServer(aSetPasswordMsg, WONMsg::RoutingSetPassword, new CompletionContainer<short>(completion));
}

// RoutingServerClient::BecomeModerator
// Sends an MMsgRoutingBecomeModerator request.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::BecomeModerator(bool flagOnOrOff, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeClient);
		return;
	}
	
	MMsgRoutingBecomeModerator aBecomeModeratorMsg;
	aBecomeModeratorMsg.SetOnOrOffFlag(flagOnOrOff);
	SendMessageToServer(aBecomeModeratorMsg, WONMsg::RoutingBecomeModerator, new CompletionContainer<short>(completion));
}

// RoutingServerClient::BootClient
// Sends an MMsgRoutingBootClient request.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::BootClient(RoutingServerClient::ClientId theClientId, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeHostOrModerator);
		return;
	}
	
	MMsgRoutingBootClient aBootClientMsg;
	aBootClientMsg.SetClientId(theClientId);
	SendMessageToServer(aBootClientMsg, WONMsg::RoutingBootClient, new CompletionContainer<short>(completion));
}

// RoutingServerClient::BanClient/UnbanClient
// Sends an MMsgRoutingBanClient request.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::BanClientId(ClientId theClientId, const CompletionContainer<short>& completion)
{ BanOrUnbanClientId(true, theClientId, completion); }
void RoutingServerClient::UnbanClientId(ClientId theClientId, const CompletionContainer<short>& completion)
{ BanOrUnbanClientId(false, theClientId, completion); }

void RoutingServerClient::BanClientName(const ClientName& theClientNameR, const CompletionContainer<short>& completion)
{ BanOrUnbanClientName(true, theClientNameR, completion); }
void RoutingServerClient::UnbanClientName(const ClientName& theClientNameR, const CompletionContainer<short>& completion)
{ BanOrUnbanClientName(false, theClientNameR, completion); }

void RoutingServerClient::BanWONUserId(unsigned long theWONUserId, const CompletionContainer<short>& completion)
{ BanOrUnbanWONUserId(true, theWONUserId, completion); }
void RoutingServerClient::UnbanWONUserId(unsigned long theWONUserId, const CompletionContainer<short>& completion)
{ BanOrUnbanWONUserId(false, theWONUserId, completion); }

void RoutingServerClient::BanOrUnbanClientId(bool banOrUnban, ClientId theClientId, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeHostOrModerator);
		return;
	}
	
	MMsgRoutingBanClient aBanClientMsg;
    aBanClientMsg.SetOnOrOffFlag(banOrUnban);
	aBanClientMsg.SetContentType(WONMsg::CONTENTTYPE_CLIENTID);
	aBanClientMsg.SetClientId(theClientId);
	SendMessageToServer(aBanClientMsg, WONMsg::RoutingBanClient, new CompletionContainer<short>(completion));
}

void RoutingServerClient::BanOrUnbanClientName(bool banOrUnban, const ClientName& theClientNameR, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeHostOrModerator);
		return;
	}
	
	MMsgRoutingBanClient aBanClientMsg;
    aBanClientMsg.SetOnOrOffFlag(banOrUnban);
	aBanClientMsg.SetContentType(WONMsg::CONTENTTYPE_CLIENTNAME);
	aBanClientMsg.SetClientName(theClientNameR);
	SendMessageToServer(aBanClientMsg, WONMsg::RoutingBanClient, new CompletionContainer<short>(completion));
}

void RoutingServerClient::BanOrUnbanWONUserId(bool banOrUnban, long theWONUserId, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeHostOrModerator);
		return;
	}
	
	MMsgRoutingBanClient aBanClientMsg;
    aBanClientMsg.SetOnOrOffFlag(banOrUnban);
	aBanClientMsg.SetContentType(WONMsg::CONTENTTYPE_WONUSERID);
	aBanClientMsg.SetWONUserId(theWONUserId);
	SendMessageToServer(aBanClientMsg, WONMsg::RoutingBanClient, new CompletionContainer<short>(completion));
}

// RoutingServerClient::InviteClient
// Sends an MMsgRoutingInviteClient request.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::InviteClient(const ClientName& theClientNameR, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeHost);
		return;
	}
	
	MMsgRoutingInviteClient aInviteClientMsg;
    aInviteClientMsg.SetOnOrOffFlag(true);
	aInviteClientMsg.SetClientName(theClientNameR);
	SendMessageToServer(aInviteClientMsg, WONMsg::RoutingInviteClient, new CompletionContainer<short>(completion));
}

// RoutingServerClient::UninviteClient
// Sends an MMsgRoutingUninviteClient request.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::UninviteClient(const ClientName& theClientNameR, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeHost);
		return;
	}
	
	MMsgRoutingInviteClient aInviteClientMsg;
    aInviteClientMsg.SetOnOrOffFlag(false);
	aInviteClientMsg.SetClientName(theClientNameR);
	SendMessageToServer(aInviteClientMsg, WONMsg::RoutingInviteClient, new CompletionContainer<short>(completion));
}

// RoutingServerClient::MuteClient
// Sends an MMsgRoutingMuteClient request.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::MuteClient(RoutingServerClient::ClientId theClientId, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeHostOrModerator);
		return;
	}
	
	MMsgRoutingMuteClient aMuteClientMsg;
	aMuteClientMsg.SetOnOrOffFlag(true);
	aMuteClientMsg.SetClientId(theClientId);
	SendMessageToServer(aMuteClientMsg, WONMsg::RoutingMuteClient, new CompletionContainer<short>(completion));
}

// RoutingServerClient::UnmuteClient
// Sends an MMsgRoutingUnmuteClient request.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::UnmuteClient(RoutingServerClient::ClientId theClientId, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeHostOrModerator);
		return;
	}
	
	MMsgRoutingMuteClient aUnmuteClientMsg;
	aUnmuteClientMsg.SetOnOrOffFlag(false);
	aUnmuteClientMsg.SetClientId(theClientId);
	SendMessageToServer(aUnmuteClientMsg, WONMsg::RoutingMuteClient, new CompletionContainer<short>(completion));
}

// RoutingServerClient::AddSuccessor
// Sends an MMsgRoutingAddSuccessor request.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::AddSuccessor(RoutingServerClient::ClientId theClientId, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeHostOrSuccessor);
		return;
	}
	
	MMsgRoutingAddSuccessor aAddSuccessorMsg;
	aAddSuccessorMsg.SetClientId(theClientId);
	SendMessageToServer(aAddSuccessorMsg, WONMsg::RoutingAddSuccessor, new CompletionContainer<short>(completion));
}

// RoutingServerClient::RemoveSuccessor
// Sends an MMsgRoutingRemoveSuccessor request.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::RemoveSuccessor(RoutingServerClient::ClientId theClientId, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeHostOrSuccessor);
		return;
	}
	
	MMsgRoutingRemoveSuccessor aRemoveSuccessorMsg;
	aRemoveSuccessorMsg.SetClientId(theClientId);
	SendMessageToServer(aRemoveSuccessorMsg, WONMsg::RoutingRemoveSuccessor, new CompletionContainer<short>(completion));
}

// RoutingServerClient::CreateGroup
// Sends an MMsgRoutingCreateGroup request.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::CreateGroup(const GroupName& theGroupNameR, bool isPublic, bool announceChanges, const RoutingServerClient::ClientId* theInitialMembersP, unsigned short theNumInitialMembers, const CompletionContainer<const CreateGroupResult&>& completion)
{ 
	std::list<RoutingServerClient::ClientId> aInitialMembersList;
	for (int i = 0; i < theNumInitialMembers; ++i)
		aInitialMembersList.push_back(*(theInitialMembersP + i));

	CreateGroup(theGroupNameR, isPublic, announceChanges, aInitialMembersList, completion);
}
void RoutingServerClient::CreateGroup(const GroupName& theGroupNameR, bool isPublic, bool announceChanges, const std::list<RoutingServerClient::ClientId>& theInitialMembers, const CompletionContainer<const CreateGroupResult&>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(CreateGroupResult(WONMsg::StatusRouting_MustBeHost));
		return;
	}
	
	MMsgRoutingCreateGroup aCreateGroupMsg;
	aCreateGroupMsg.SetGroupName(theGroupNameR);
	aCreateGroupMsg.SetIsPublic(isPublic);
	aCreateGroupMsg.SetAnnounceGroupChanges(announceChanges);
	aCreateGroupMsg.SetClientList(theInitialMembers);
	SendMessageToServer(aCreateGroupMsg, WONMsg::RoutingCreateGroup, new CompletionContainer<const CreateGroupResult&>(completion));
}

// RoutingServerClient::DeleteGroup
// Sends an MMsgRoutingDeleteGroup request.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::DeleteGroup(GroupId theGroupId, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeHost);
		return;
	}
	
	MMsgRoutingDeleteGroup aDeleteGroupMsg;
	aDeleteGroupMsg.SetGroupId(theGroupId);
	SendMessageToServer(aDeleteGroupMsg, WONMsg::RoutingDeleteGroup, new CompletionContainer<short>(completion));
}

// RoutingServerClient::AddToGroup
// Sends an MMsgRoutingAddToGroup request.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::AddToGroup(RoutingServerClient::ClientId theClientId, GroupId theGroupId, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeClient);
		return;
	}
	
	MMsgRoutingAddToGroup aAddToGroupMsg;
	aAddToGroupMsg.SetClientId(theClientId);
	aAddToGroupMsg.SetGroupId(theGroupId);
	SendMessageToServer(aAddToGroupMsg, WONMsg::RoutingAddToGroup, new CompletionContainer<short>(completion));
}

// RoutingServerClient::RemoveFromGroup
// Sends an MMsgRoutingRemoveFromGroup request.  See the Routing Server spec for
// more details on this message.
void RoutingServerClient::RemoveFromGroup(RoutingServerClient::ClientId theClientId, GroupId theGroupId, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeClient);
		return;
	}
	
	MMsgRoutingRemoveFromGroup aRemoveFromGroupMsg;
	aRemoveFromGroupMsg.SetClientId(theClientId);
	aRemoveFromGroupMsg.SetGroupId(theGroupId);
	SendMessageToServer(aRemoveFromGroupMsg, WONMsg::RoutingRemoveFromGroup, new CompletionContainer<short>(completion));
}

void RoutingServerClient::CreateDataObject(ClientOrGroupId theLinkId, const RawBuffer& theDataTypeR, ClientOrGroupId theOwnerId, unsigned short theLifespan, const RawBuffer& theDataR, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeClient);
		return;
	}
	
	MMsgRoutingCreateDataObject aCreateDataObjectMsg;
	aCreateDataObjectMsg.SetLinkId(theLinkId);
	aCreateDataObjectMsg.SetDataType(theDataTypeR);
	aCreateDataObjectMsg.SetOwnerId(theOwnerId);
	aCreateDataObjectMsg.SetLifespan(theLifespan);
	aCreateDataObjectMsg.SetData(theDataR);
	SendMessageToServer(aCreateDataObjectMsg, WONMsg::RoutingCreateDataObject, new CompletionContainer<short>(completion));
}

void RoutingServerClient::DeleteDataObject(ClientOrGroupId theLinkId, const RawBuffer& theDataTypeR, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeClient);
		return;
	}
	
	MMsgRoutingDeleteDataObject aDeleteDataObjectMsg;
	aDeleteDataObjectMsg.SetLinkId(theLinkId);
	aDeleteDataObjectMsg.SetDataType(theDataTypeR);
	SendMessageToServer(aDeleteDataObjectMsg, WONMsg::RoutingDeleteDataObject, new CompletionContainer<short>(completion));
}

void RoutingServerClient::ModifyDataObject(ClientOrGroupId theLinkId, const RawBuffer& theDataTypeR, unsigned short theOffset, bool isInsert, const RawBuffer& theDataR, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeClient);
		return;
	}
	
	MMsgRoutingModifyDataObject aModifyDataObjectMsg;
	aModifyDataObjectMsg.SetLinkId(theLinkId);
	aModifyDataObjectMsg.SetDataType(theDataTypeR);
	aModifyDataObjectMsg.SetOffset(theOffset);
	aModifyDataObjectMsg.SetIsInsert(isInsert);
	aModifyDataObjectMsg.SetData(theDataR);
	SendMessageToServer(aModifyDataObjectMsg, WONMsg::RoutingModifyDataObject, new CompletionContainer<short>(completion));
}

void RoutingServerClient::ReplaceDataObject(ClientOrGroupId theLinkId, const RawBuffer& theDataTypeR, const RawBuffer& theDataR, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeClient);
		return;
	}
	
	MMsgRoutingReplaceDataObject aReplaceDataObjectMsg;
	aReplaceDataObjectMsg.SetLinkId(theLinkId);
	aReplaceDataObjectMsg.SetDataType(theDataTypeR);
	aReplaceDataObjectMsg.SetData(theDataR);
	SendMessageToServer(aReplaceDataObjectMsg, WONMsg::RoutingReplaceDataObject, new CompletionContainer<short>(completion));
}

void RoutingServerClient::RenewDataObject(ClientOrGroupId theLinkId, const RawBuffer& theDataTypeR, unsigned short theLifespan, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeClient);
		return;
	}
	
	MMsgRoutingRenewDataObject aRenewDataObjectMsg;
	aRenewDataObjectMsg.SetLinkId(theLinkId);
	aRenewDataObjectMsg.SetDataType(theDataTypeR);
	aRenewDataObjectMsg.SetNewLifespan(theLifespan);
	SendMessageToServer(aRenewDataObjectMsg, WONMsg::RoutingRenewDataObject, new CompletionContainer<short>(completion));
}

void RoutingServerClient::ReadDataObject(ClientOrGroupId theLinkId, const RawBuffer& theDataTypeR, bool flagExactOrRecursive, bool flagGroupOrMembers, const CompletionContainer<const ReadDataObjectResult&>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(ReadDataObjectResult(WONMsg::StatusRouting_MustBeClient));
		return;
	}
	
	MMsgRoutingReadDataObject aReadDataObjectMsg;
	aReadDataObjectMsg.SetLinkId(theLinkId);
	aReadDataObjectMsg.SetDataType(theDataTypeR);
	aReadDataObjectMsg.SetExactOrRecursiveFlag(flagExactOrRecursive);
	aReadDataObjectMsg.SetGroupOrMembersFlag(flagGroupOrMembers);
	SendMessageToServer(aReadDataObjectMsg, WONMsg::RoutingReadDataObject, new CompletionContainer<const ReadDataObjectResult&>(completion));
}

void RoutingServerClient::SubscribeDataObject(ClientOrGroupId theLinkId, const RawBuffer& theDataTypeR, bool flagExactOrRecursive, bool flagGroupOrMembers, const CompletionContainer<const ReadDataObjectResult&>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(ReadDataObjectResult(WONMsg::StatusRouting_MustBeClient));
		return;
	}
	
	MMsgRoutingSubscribeDataObject aSubscribeDataObjectMsg;
	aSubscribeDataObjectMsg.SetLinkId(theLinkId);
	aSubscribeDataObjectMsg.SetDataType(theDataTypeR);
	aSubscribeDataObjectMsg.SetExactOrRecursiveFlag(flagExactOrRecursive);
	aSubscribeDataObjectMsg.SetGroupOrMembersFlag(flagGroupOrMembers);
	SendMessageToServer(aSubscribeDataObjectMsg, WONMsg::RoutingSubscribeDataObject, new CompletionContainer<const ReadDataObjectResult&>(completion));
}

void RoutingServerClient::UnsubscribeDataObject(ClientOrGroupId theLinkId, const RawBuffer& theDataTypeR, bool flagExactOrRecursive, bool flagGroupOrMembers, const CompletionContainer<short>& completion)
{
	if (mEnableShortcuts && mClientId == 0) {
		completion.Complete(WONMsg::StatusRouting_MustBeClient);
		return;
	}
	
	MMsgRoutingUnsubscribeDataObject aUnsubscribeDataObjectMsg;
	aUnsubscribeDataObjectMsg.SetLinkId(theLinkId);
	aUnsubscribeDataObjectMsg.SetDataType(theDataTypeR);
	aUnsubscribeDataObjectMsg.SetExactOrRecursiveFlag(flagExactOrRecursive);
	aUnsubscribeDataObjectMsg.SetGroupOrMembersFlag(flagGroupOrMembers);
	SendMessageToServer(aUnsubscribeDataObjectMsg, WONMsg::RoutingUnsubscribeDataObject, new CompletionContainer<short>(completion));
}

void RoutingServerClient::IsUserPresent(const wstring& theUserName, bool caseSensitive, bool shouldSendNACK, unsigned short theTag, const CompletionContainer<bool>& completion)
{
	MMsgCommIsUserPresent aIsUserPresentMsg;
	aIsUserPresentMsg.SetUserName(theUserName);
	aIsUserPresentMsg.SetCaseInsensitive(!caseSensitive);
	aIsUserPresentMsg.SetShouldSendNack(shouldSendNACK);
	aIsUserPresentMsg.SetTag(theTag);
	SendMessageToServer(aIsUserPresentMsg, WONMsg::MiniCommIsUserPresent, new CompletionContainer<bool>(completion));
}

void RoutingServerClient::GetNumUsers(unsigned short theTag, const CompletionContainer<const GetNumUsersResult&>& completion)
{
	MMsgCommGetNumUsers aGetNumUsersMsg;
	aGetNumUsersMsg.SetTag(theTag);
	SendMessageToServer(aGetNumUsersMsg, WONMsg::MiniCommGetNumUsers, new CompletionContainer<const GetNumUsersResult&>(completion));
}


//
// Reply Handlers
//

/*
// RoutingServerClient::HandleAcceptUser
// Called when an MMsgRoutingAcceptUser request is received.
void RoutingServerClient::HandleAcceptUser(const TMsgSocket::RecvBaseMsgResult& result)
{
	MMsgRoutingAcceptUser aAcceptUserMsg;
	FillMsgFromResult(aAcceptUserMsg, result);
	
	OnClientJoinAttempt(aAcceptUserMsg.GetUserName());
	if (!mClientJoinAttemptCatcher.empty())
		mClientJoinAttemptCatcher.Complete(aAcceptUserMsg.GetUserName());
}
*/
// RoutingServerClient::HandleClientChange
// Called when an MMsgRoutingClientChange notification is received.
void RoutingServerClient::HandleClientChange(const TMsgSocket::RecvBaseMsgResult& result)
{
	MMsgRoutingClientChange aClientChangeMsg;
	FillMsgFromResult(aClientChangeMsg, result);

	if (aClientChangeMsg.DidClientLeave())
	{
		ClientMap::iterator itr = mClientMap.find(aClientChangeMsg.GetClientId());
		if (itr != mClientMap.end())
		{
			mClientNameToIdMap.erase(itr->second.mClientName);
			mClientMap.erase(itr);
		}

		OnClientLeave(aClientChangeMsg.GetClientId());
		if (!mClientLeaveCatcher.empty())
			mClientLeaveCatcher.Complete(ClientIdWithReason(aClientChangeMsg.GetClientId(), aClientChangeMsg.GetReason()));
	}
	else
	{
		OnClientEnter(aClientChangeMsg.GetClientId());
		if (!mClientEnterCatcher.empty())
			mClientEnterCatcher.Complete(ClientIdWithReason(aClientChangeMsg.GetClientId(), aClientChangeMsg.GetReason()));
	}
}

// RoutingServerClient::HandleClientChangeEx
// Called when an MMsgRoutingClientChangeEx notification is received.
void RoutingServerClient::HandleClientChangeEx(const TMsgSocket::RecvBaseMsgResult& result)
{
	MMsgRoutingClientChangeEx aClientChangeExMsg;
	FillMsgFromResult(aClientChangeExMsg, result);

	if (aClientChangeExMsg.DidClientLeave())
	{
		// NOTE: This should never happen.  ClientChangeEx messages should only be sent when clients enter
		mClientMap.erase(aClientChangeExMsg.GetClientId());
		mClientNameToIdMap.erase(aClientChangeExMsg.GetClientName());

		OnClientLeave(aClientChangeExMsg.GetClientId());
		if (!mClientLeaveCatcher.empty())
			mClientLeaveCatcher.Complete(ClientIdWithReason(aClientChangeExMsg.GetClientId(), aClientChangeExMsg.GetReason()));
	}
	else
	{
		WONMsg::MMsgRoutingGetClientListReply::ClientData aClientData;
		aClientData.mClientId   = aClientChangeExMsg.GetClientId();
		aClientData.mClientName = aClientChangeExMsg.GetClientName();
		if (aClientChangeExMsg.IncludesIPs())
			aClientData.mIPAddress = aClientChangeExMsg.GetIPAddress();
		if (aClientChangeExMsg.IncludesAuthInfo())
		{
			aClientData.mWONUserId   = aClientChangeExMsg.GetWONUserId();
			aClientData.mCommunityId = aClientChangeExMsg.GetCommunityId();
			aClientData.mTrustLevel  = aClientChangeExMsg.GetTrustLevel();
		}
		if (aClientChangeExMsg.IncludesModeratorFlag())
			aClientData.mIsModerator = aClientChangeExMsg.IsModerator();
		if (aClientChangeExMsg.IncludesMuteFlag())
			aClientData.mIsMuted = aClientChangeExMsg.IsMuted();

		mClientMap.insert(ClientMapPair(aClientChangeExMsg.GetClientId(), aClientData));
		mClientNameToIdMap.insert(ClientNameToIdMapPair(aClientChangeExMsg.GetClientName(), aClientChangeExMsg.GetClientId()));
		if (IsClientIgnored(aClientChangeExMsg.GetClientName()))
			mIgnoredClientIds.insert(aClientChangeExMsg.GetClientId());

		OnClientEnter(aClientChangeExMsg.GetClientId(), aClientChangeExMsg.GetClientName(), aClientChangeExMsg.GetIPAddress(), aClientChangeExMsg.GetWONUserId(), aClientChangeExMsg.GetCommunityId(), aClientChangeExMsg.GetTrustLevel());
		if (!mClientEnterExCatcher.empty())
			mClientEnterExCatcher.Complete(ClientDataWithReason(aClientData, aClientChangeExMsg.GetReason()));
	}
}

// RoutingServerClient::HandleHostChange
// Called when an MMsgRoutingHostChange notification is received.
void RoutingServerClient::HandleHostChange(const TMsgSocket::RecvBaseMsgResult& result)
{
	MMsgRoutingHostChange aHostChangeMsg;
	FillMsgFromResult(aHostChangeMsg, result);

	mHostId = aHostChangeMsg.GetClientId();
	
	OnHostChange(mHostId);
	if (!mHostChangeCatcher.empty())
		mHostChangeCatcher.Complete(mHostId);
}

// RoutingServerClient::HandleSpectatorCount
// Called when an MMsgRoutingSpectatorCount notification is received.
void RoutingServerClient::HandleSpectatorCount(const TMsgSocket::RecvBaseMsgResult& result)
{
	MMsgRoutingSpectatorCount aSpectatorCountMsg;
	FillMsgFromResult(aSpectatorCountMsg, result);

	OnSpectatorCount(aSpectatorCountMsg.GetSpectatorCount());
	if (!mSpectatorCountCatcher.empty())
		mSpectatorCountCatcher.Complete(aSpectatorCountMsg.GetSpectatorCount());
}

// RoutingServerClient::HandleGroupChangeEx
// Called when an MMsgRoutingGroupChangeEx notification is received.
void RoutingServerClient::HandleGroupChangeEx(const TMsgSocket::RecvBaseMsgResult& result)
{
	MMsgRoutingGroupChangeEx aGroupChangeExMsg;
	FillMsgFromResult(aGroupChangeExMsg, result);

	if (aGroupChangeExMsg.DidClientLeave())
	{
		// NOTE: This should never happen.  ClientChangeEx messages should only be sent when clients enter
//		mClientMap.erase(aGroupChangeMsg.GetClientId());
		OnGroupLeave(aGroupChangeExMsg.GetGroupId(), aGroupChangeExMsg.GetClientId());
		if (!mGroupLeaveCatcher.empty())
			mGroupLeaveCatcher.Complete(GroupIdAndClientIdWithReason(aGroupChangeExMsg.GetGroupId(), aGroupChangeExMsg.GetClientId(), aGroupChangeExMsg.GetReason()));
	}
	else
	{
		WONMsg::MMsgRoutingGetClientListReply::ClientData aClientData;
		aClientData.mClientId   = aGroupChangeExMsg.GetClientId();
		aClientData.mClientName = aGroupChangeExMsg.GetClientName();
		if (aGroupChangeExMsg.IncludesIPs())
			aClientData.mIPAddress = aGroupChangeExMsg.GetIPAddress();
		if (aGroupChangeExMsg.IncludesAuthInfo())
		{
			aClientData.mWONUserId   = aGroupChangeExMsg.GetWONUserId();
			aClientData.mCommunityId = aGroupChangeExMsg.GetCommunityId();
			aClientData.mTrustLevel  = aGroupChangeExMsg.GetTrustLevel();
		}
		if (aGroupChangeExMsg.IncludesModeratorFlag())
			aClientData.mIsModerator = aGroupChangeExMsg.IsModerator();
		if (aGroupChangeExMsg.IncludesMuteFlag())
			aClientData.mIsMuted = aGroupChangeExMsg.IsMuted();

		OnGroupEnter(aGroupChangeExMsg.GetGroupId(), aGroupChangeExMsg.GetClientId(), aGroupChangeExMsg.GetClientName(), aGroupChangeExMsg.GetIPAddress(), aGroupChangeExMsg.GetWONUserId(), aGroupChangeExMsg.GetCommunityId(), aGroupChangeExMsg.GetTrustLevel());
		if (!mGroupEnterExCatcher.empty())
			mGroupEnterExCatcher.Complete(GroupIdAndClientDataWithReason(aGroupChangeExMsg.GetGroupId(), aClientData, aGroupChangeExMsg.GetReason()));
		if (IsClientIgnored(aGroupChangeExMsg.GetClientName()))
			mIgnoredClientIds.insert(aGroupChangeExMsg.GetClientId());
	}
}

// RoutingServerClient::HandleGroupChange
// Called when an MMsgRoutingGroupChange notification is received.
void RoutingServerClient::HandleGroupChange(const TMsgSocket::RecvBaseMsgResult& result)
{
	MMsgRoutingGroupChange aGroupChangeMsg;
	FillMsgFromResult(aGroupChangeMsg, result);

	if (aGroupChangeMsg.DidClientLeave())
	{
//		mClientMap.erase(aGroupChangeMsg.GetClientId());
		OnGroupLeave(aGroupChangeMsg.GetGroupId(), aGroupChangeMsg.GetClientId());
		if (!mGroupLeaveCatcher.empty())
			mGroupLeaveCatcher.Complete(GroupIdAndClientIdWithReason(aGroupChangeMsg.GetGroupId(), aGroupChangeMsg.GetClientId(), aGroupChangeMsg.GetReason()));
	}
	else
	{
		OnGroupEnter(aGroupChangeMsg.GetGroupId(), aGroupChangeMsg.GetClientId());
		if (!mGroupEnterCatcher.empty())
			mGroupEnterCatcher.Complete(GroupIdAndClientIdWithReason(aGroupChangeMsg.GetGroupId(), aGroupChangeMsg.GetClientId(), aGroupChangeMsg.GetReason()));
	}
}

// RoutingServerClient::HandleGroupSpectatorCount
// Called when an MMsgRoutingGroupSpectatorCount notification is received.
void RoutingServerClient::HandleGroupSpectatorCount(const TMsgSocket::RecvBaseMsgResult& result)
{
	MMsgRoutingGroupSpectatorCount aGroupSpectatorCountMsg;
	FillMsgFromResult(aGroupSpectatorCountMsg, result);

	OnGroupSpectatorCount(aGroupSpectatorCountMsg.GetGroupId(), aGroupSpectatorCountMsg.GetSpectatorCount());
	if (!mGroupSpectatorCountCatcher.empty())
		mGroupSpectatorCountCatcher.Complete(GroupIdAndSpectatorCount(aGroupSpectatorCountMsg.GetGroupId(), aGroupSpectatorCountMsg.GetSpectatorCount()));
}

void RoutingServerClient::HandleBecomeModerator(const TMsgSocket::RecvBaseMsgResult& result)
{
	MMsgRoutingBecomeModerator aBecomeModeratorMsg;
	FillMsgFromResult(aBecomeModeratorMsg, result);

	OnBecomeModerator(aBecomeModeratorMsg.GetClientId(), aBecomeModeratorMsg.GetOnOrOffFlag());

	if (!mBecomeModeratorCatcher.empty())
	{
		ClientIdWithFlag aClientIdWithFlag(aBecomeModeratorMsg.GetClientId(), aBecomeModeratorMsg.GetOnOrOffFlag());
		mBecomeModeratorCatcher.Complete(aClientIdWithFlag);
	}
}

void RoutingServerClient::HandleMuteClient(const TMsgSocket::RecvBaseMsgResult& result)
{
	MMsgRoutingMuteClient aMuteClientMsg;
	FillMsgFromResult(aMuteClientMsg, result);

	OnMuteClient(aMuteClientMsg.GetClientId(), aMuteClientMsg.GetOnOrOffFlag());

	if (!mMuteClientCatcher.empty())
	{
		ClientIdWithFlag aClientIdWithFlag(aMuteClientMsg.GetClientId(), aMuteClientMsg.GetOnOrOffFlag());
		mMuteClientCatcher.Complete(aClientIdWithFlag);
	}
}

void RoutingServerClient::HandleCreateDataObject(const TMsgSocket::RecvBaseMsgResult& result)
{
	MMsgRoutingCreateDataObject aCreateDataObjectMsg;
	FillMsgFromResult(aCreateDataObjectMsg, result);

	OnDataObjectCreated();

	if (!mDataObjectCreationCatcher.empty())
	{
		DataObjectWithLifespan aDataObject(aCreateDataObjectMsg.GetLinkId(), aCreateDataObjectMsg.GetOwnerId(), aCreateDataObjectMsg.GetDataType(), aCreateDataObjectMsg.GetData(), aCreateDataObjectMsg.GetLifespan());
		mDataObjectCreationCatcher.Complete(aDataObject);
	}
}

void RoutingServerClient::HandleDeleteDataObject(const TMsgSocket::RecvBaseMsgResult& result)
{
	MMsgRoutingDeleteDataObject aDeleteDataObjectMsg;
	FillMsgFromResult(aDeleteDataObjectMsg, result);

	OnDataObjectDeleted();

	if (!mDataObjectDeletionCatcher.empty())
	{
		DataObject aDataObject(aDeleteDataObjectMsg.GetLinkId(), aDeleteDataObjectMsg.GetDataType());
		mDataObjectDeletionCatcher.Complete(aDataObject);
	}
}

void RoutingServerClient::HandleReplaceDataObject(const TMsgSocket::RecvBaseMsgResult& result)
{
	MMsgRoutingReplaceDataObject aReplaceDataObjectMsg;
	FillMsgFromResult(aReplaceDataObjectMsg, result);

	OnDataObjectReplaced();

	if (!mDataObjectReplacementCatcher.empty())
	{
		DataObject aDataObject(aReplaceDataObjectMsg.GetLinkId(), aReplaceDataObjectMsg.GetDataType(), aReplaceDataObjectMsg.GetData());
		mDataObjectReplacementCatcher.Complete(aDataObject);
	}
}

void RoutingServerClient::HandleKeepAlive(const TMsgSocket::RecvBaseMsgResult& result)
{
	OnKeepAlive();
	if (!mKeepAliveCatcher.empty())
		mKeepAliveCatcher.Complete();
}

void RoutingServerClient::HandleModifyDataObject(const TMsgSocket::RecvBaseMsgResult& result)
{
	MMsgRoutingModifyDataObject aModifyDataObjectMsg;
	FillMsgFromResult(aModifyDataObjectMsg, result);

	OnDataObjectModified();

	if (!mDataObjectModificationCatcher.empty())
	{
		DataObjectModification aDataObjectModification(aModifyDataObjectMsg.GetLinkId(), aModifyDataObjectMsg.GetDataType(), aModifyDataObjectMsg.GetData(), aModifyDataObjectMsg.IsInsert(), aModifyDataObjectMsg.GetOffset());
		mDataObjectModificationCatcher.Complete(aDataObjectModification);
	}
}

void RoutingServerClient::HandlePeerData(const TMsgSocket::RecvBaseMsgResult& result)
{
	MMsgRoutingPeerData aPeerDataMsg;
	FillMsgFromResult(aPeerDataMsg, result);

	OnDataFromPeer();

	MessageWithClientId aMessage(aPeerDataMsg.GetClientId(), aPeerDataMsg.GetData().data(), aPeerDataMsg.GetData().size());
	if (!mPeerDataCatcher.empty())
		mPeerDataCatcher.Complete(aMessage);
}

void RoutingServerClient::HandlePeerChat(const TMsgSocket::RecvBaseMsgResult& result)
{
	MMsgRoutingPeerChat aPeerChatMsg;
	FillMsgFromResult(aPeerChatMsg, result);

	if (IsClientIgnored(aPeerChatMsg.GetClientId()))
		return;

	OnChatFromPeer();

	if (!mRawPeerChatCatcher.empty())
	{
		RawChatMessage aMessage(aPeerChatMsg);
		mRawPeerChatCatcher.Complete(aMessage);
	}

	switch (aPeerChatMsg.GetChatType())
	{
		case WONMsg::CHATTYPE_ASCII:
		case WONMsg::CHATTYPE_ASCII_EMOTE:
			if (!mASCIIPeerChatCatcher.empty())
			{
				ASCIIChatMessage aMessage(aPeerChatMsg);
				mASCIIPeerChatCatcher.Complete(aMessage);
			}
			else if (!mUnicodePeerChatCatcher.empty())
			{
				WONString aChatData((char*)aPeerChatMsg.GetData().data());
				aPeerChatMsg.SetData(RawBuffer((unsigned char*)aChatData.GetTitanUnicodeCString()));
				aPeerChatMsg.SetChatType(aPeerChatMsg.GetChatType() == WONMsg::CHATTYPE_ASCII ? WONMsg::CHATTYPE_UNICODE : WONMsg::CHATTYPE_UNICODE_EMOTE);
				UnicodeChatMessage aMessage(aPeerChatMsg);
				mUnicodePeerChatCatcher.Complete(aMessage);
			}
			break;
		case WONMsg::CHATTYPE_UNICODE:
		case WONMsg::CHATTYPE_UNICODE_EMOTE:
			if (!mUnicodePeerChatCatcher.empty())
			{
				UnicodeChatMessage aMessage(aPeerChatMsg);
				mUnicodePeerChatCatcher.Complete(aMessage);
			}
			else if (!mASCIIPeerChatCatcher.empty())
			{
				WONString aChatData((wchar*)aPeerChatMsg.GetData().data(), aPeerChatMsg.GetData().size() / 2);
				aPeerChatMsg.SetData((unsigned char*)aChatData.GetAsciiCString());
				aPeerChatMsg.SetChatType(aPeerChatMsg.GetChatType() == WONMsg::CHATTYPE_UNICODE ? WONMsg::CHATTYPE_ASCII : WONMsg::CHATTYPE_ASCII_EMOTE);
				ASCIIChatMessage aMessage(aPeerChatMsg);
				mASCIIPeerChatCatcher.Complete(aMessage);
			}
			break;
	}
}

void RoutingServerClient::HandlePeerDataMultiple(const TMsgSocket::RecvBaseMsgResult& result)
{
	MMsgRoutingPeerDataMultiple aPeerDataMultipleMsg;
	FillMsgFromResult(aPeerDataMultipleMsg, result);

	MMsgRoutingPeerDataMultiple::MessageList::const_iterator itr = aPeerDataMultipleMsg.GetMessageList().begin();
	for ( ; itr != aPeerDataMultipleMsg.GetMessageList().end(); ++itr)
	{
		OnDataFromPeer();
		
		if (!mPeerDataCatcher.empty())
		{
			MessageWithClientId aMessage(itr->mClientId, itr->mData.data(), itr->mData.size());
			mPeerDataCatcher.Complete(aMessage);
		}
	}
}

// RoutingServerClient::HandleGetClientInfoReply
// Called when an MMsgRoutingGetClientListReply is received in response to an MMsgRoutingGetClientInfo request.
void RoutingServerClient::HandleGetClientInfoReply(const TMsgSocket::RecvBaseMsgResult& result)
{
	HandleGetClientListReply(result);
}

// RoutingServerClient::HandleGetClientListReply
// Called when an MMsgRoutingGetClientListReply is received in response to an MMsgRoutingGetClientList request.
void RoutingServerClient::HandleGetClientListReply(const TMsgSocket::RecvBaseMsgResult& result)
{
	MMsgRoutingGetClientListReply aGetClientListReply;
	FillMsgFromResult(aGetClientListReply, result);

	MMsgRoutingGetClientListReply::ClientList::const_iterator itr = aGetClientListReply.GetClientList().begin();
	for (; itr != aGetClientListReply.GetClientList().end(); ++itr)
	{
		WONMsg::MMsgRoutingGetClientListReply::ClientData aClientData;
		aClientData.mClientId   = itr->mClientId;
		aClientData.mClientName = itr->mClientName;
		if (aGetClientListReply.IncludesIPs())
			aClientData.mIPAddress = itr->mIPAddress;
		if (aGetClientListReply.IncludesAuthInfo())
		{
			aClientData.mWONUserId   = itr->mWONUserId;
			aClientData.mCommunityId = itr->mCommunityId;
			aClientData.mTrustLevel  = itr->mTrustLevel;
		}

		mClientMap.insert(ClientMapPair(itr->mClientId, aClientData));
		mClientNameToIdMap.insert(ClientNameToIdMapPair(itr->mClientName, itr->mClientId));
	}
}

// RoutingServerClient::HandleRegisterClientReply
// Called when an MMsgRoutingRegisterClientReply is received in response to an MMsgRoutingRegisterClient request.
void RoutingServerClient::HandleRegisterClientReply(const TMsgSocket::RecvBaseMsgResult& result)
{
	MMsgRoutingRegisterClientReply aRegisterClientReply;
	FillMsgFromResult(aRegisterClientReply, result);

	// if the registration was successful, record the ClientId
	if (aRegisterClientReply.GetStatus() == WONMsg::StatusCommon_Success)
		mClientId = aRegisterClientReply.GetClientId();
}

//
// PRIVATE METHODS
//
/*
// RoutingServerClient::AcceptUser
// Called by the public AcceptUser and RejectUser methods to send an MMsgRoutingAcceptUserReply
// to the server.
void RoutingServerClient::AcceptUser(bool accept, const UserName& theUserNameR, const wstring& theCommentR)
{
	MMsgRoutingAcceptUserReply aAcceptUserReply;
	aAcceptUserReply.SetAcceptOrRejectFlag(accept);
	aAcceptUserReply.SetUserName(theUserNameR);
	aAcceptUserReply.SetComment(theCommentR);
	SendMessageToServer(aAcceptUserReply);
}
*/
// RoutingServerClient::FillMsgFromResult
// Appends the data in result.msgLength to the buffer in theMsg, and then unpacks the data.
// Normally called with an uninitialized BaseMessage object.
void RoutingServerClient::FillMsgFromResult(WONMsg::BaseMessage& theMsg, const TMsgSocket::RecvBaseMsgResult& result)
{
	theMsg.AppendBytes(result.msg->GetDataLen(), result.msg->GetDataPtr());
	try
	{ 
		theMsg.Unpack(); 
	}
	catch (WONMsg::BadMsgException&)
	{ 
		// FIX: do something useful here
		_ASSERT(false); 
	}
}

// RoutingServerClient::InitMessageHandlers
// Sets up the message dispatch tables.
void RoutingServerClient::InitMessageHandlers()
{
	// prepare the reply handler vector
	mReplyHandlerVector.clear();
	mReplyHandlerVector.resize((int)WONMsg::RoutingLargestValueInUse + 1);

	// add the data
	mReplyHandlerVector[WONMsg::RoutingGetClientInfo]     	  = &RoutingServerClient::HandleGetClientInfoReply;
	mReplyHandlerVector[WONMsg::RoutingGetClientList]     	  = &RoutingServerClient::HandleGetClientListReply;
	mReplyHandlerVector[WONMsg::RoutingRegisterClient]    	  = &RoutingServerClient::HandleRegisterClientReply;
	
	// prepare the asynchronous message handler vector
	mAsyncMsgHandlerVector.clear();
	mAsyncMsgHandlerVector.resize((int)WONMsg::RoutingLargestValueInUse + 1);

	// add the data
//	mAsyncMsgHandlerVector[WONMsg::RoutingAcceptUser]          = &RoutingServerClient::HandleAcceptUser;
	mAsyncMsgHandlerVector[WONMsg::RoutingBecomeModerator]     = &RoutingServerClient::HandleBecomeModerator;
	mAsyncMsgHandlerVector[WONMsg::RoutingClientChange]        = &RoutingServerClient::HandleClientChange;
	mAsyncMsgHandlerVector[WONMsg::RoutingClientChangeEx]      = &RoutingServerClient::HandleClientChangeEx;
	mAsyncMsgHandlerVector[WONMsg::RoutingCreateDataObject]    = &RoutingServerClient::HandleCreateDataObject;
	mAsyncMsgHandlerVector[WONMsg::RoutingDeleteDataObject]    = &RoutingServerClient::HandleDeleteDataObject;
	mAsyncMsgHandlerVector[WONMsg::RoutingGroupChange]         = &RoutingServerClient::HandleGroupChange;
	mAsyncMsgHandlerVector[WONMsg::RoutingGroupChangeEx]       = &RoutingServerClient::HandleGroupChangeEx;
	mAsyncMsgHandlerVector[WONMsg::RoutingGroupSpectatorCount] = &RoutingServerClient::HandleGroupSpectatorCount;
	mAsyncMsgHandlerVector[WONMsg::RoutingHostChange]          = &RoutingServerClient::HandleHostChange;
	mAsyncMsgHandlerVector[WONMsg::RoutingKeepAlive]           = &RoutingServerClient::HandleKeepAlive;
	mAsyncMsgHandlerVector[WONMsg::RoutingModifyDataObject]    = &RoutingServerClient::HandleModifyDataObject;
	mAsyncMsgHandlerVector[WONMsg::RoutingMuteClient]          = &RoutingServerClient::HandleMuteClient;
	mAsyncMsgHandlerVector[WONMsg::RoutingPeerChat]            = &RoutingServerClient::HandlePeerChat;
	mAsyncMsgHandlerVector[WONMsg::RoutingPeerData]            = &RoutingServerClient::HandlePeerData;
	mAsyncMsgHandlerVector[WONMsg::RoutingPeerDataMultiple]    = &RoutingServerClient::HandlePeerDataMultiple;
	mAsyncMsgHandlerVector[WONMsg::RoutingReplaceDataObject]   = &RoutingServerClient::HandleReplaceDataObject;
	mAsyncMsgHandlerVector[WONMsg::RoutingSpectatorCount]      = &RoutingServerClient::HandleSpectatorCount;
}

// RoutingServerClient::IsAsyncMessage
// Returns true if the specified message is sent to clients asynchronously during
// connections.  False otherwise.
bool RoutingServerClient::IsAsyncMessage(unsigned char theMessageType)
{
	switch (theMessageType)
	{
		case WONMsg::RoutingAcceptClient:
		case WONMsg::RoutingBecomeModerator:
		case WONMsg::RoutingClientChange:
		case WONMsg::RoutingClientChangeEx:
		case WONMsg::RoutingCreateDataObject:
		case WONMsg::RoutingDeleteDataObject:
		case WONMsg::RoutingGroupChange:
		case WONMsg::RoutingGroupChangeEx:
		case WONMsg::RoutingGroupSpectatorCount:
		case WONMsg::RoutingHostChange:
		case WONMsg::RoutingKeepAlive:
		case WONMsg::RoutingModifyDataObject:
		case WONMsg::RoutingMuteClient:
		case WONMsg::RoutingPeerChat:
		case WONMsg::RoutingPeerData:
		case WONMsg::RoutingPeerDataMultiple:
		case WONMsg::RoutingReplaceDataObject:
		case WONMsg::RoutingSpectatorCount:
			return true;
		default:
			return false;
	}
}

// RoutingServerClient::IsReplyMessage
// Returns true if the specified message (assumed to be a Routing Server message type)
// is sent by the server as a reply.  False otherwise.
bool RoutingServerClient::IsReplyMessage(unsigned char theMessageType)
{
	switch (theMessageType)
	{
		case WONMsg::RoutingCreateGroupReply:
		case WONMsg::RoutingGetClientListReply:
		case WONMsg::RoutingGetGroupListReply:
		case WONMsg::RoutingGetSimpleClientListReply:
		case WONMsg::MiniCommGetNumUsersReply:
		case WONMsg::MiniCommIsUserPresentReply:
		case WONMsg::RoutingReadDataObjectReply:
		case WONMsg::RoutingRegisterClientReply:
		case WONMsg::RoutingStatusReply:
			return true;
		default:
			return false;
	}
}

// RoutingServerClient::SendMessageToServer
// Sends the specified message to the Routing Server.  If a reply is expected, theMessageType must
// be filled in to keep the client in sync.
void RoutingServerClient::SendMessageToServer(const WONMsg::BaseMessage& theMsgR, unsigned char theMessageType, CompletionContainerBase* completion)
{
	if (theMessageType && completion)
	{
		mRequestCompletionListCrit.Enter();
		mRequestCompletionList.push_back(RequestCompletionPair(theMessageType, completion));
		mRequestCompletionListCrit.Leave();
	}
	const_cast<WONMsg::BaseMessage&>(theMsgR).Pack();

	// NOTE: Access violation because mSocketP is NULL?  Did you try to Register before Connect?  Call Connect first.
	mSocketP->SendRawMsg(theMsgR.GetDataLen(), const_cast<void*>(theMsgR.GetDataPtr()), true, -1, true);
}

// Ignore File Format:
// <namecount>: 4 bytes: number of users in list
// namecount of the following:
// <namesize>: 2 bytes: number of Unicode chars in name
// <name>: namesize Unicode characters
//
// Example (fields are described as <fieldname:size = data>:
// <namecount:4 = 2><namesize:2 = 4><name:4 = erik><namesize:2 = 6><name:6 = austin>

// RoutingServerClient::LoadIgnoredUsersList
// Reads in the binary file (IGNOREFILE) containing the list of usernames that
// the local user has ignored.
void RoutingServerClient::LoadIgnoredUsersList()
{
	FILE* aIgnoreFileH = fopen(IGNOREFILE, "rb");

	if (aIgnoreFileH)
	{
		// read in "namecount" field
		unsigned long namecount;
		fread(&namecount, sizeof(namecount), 1, aIgnoreFileH);

		// read in the list of names
		const unsigned int aNameBufSize = 100;
		unsigned char aNameBuf[aNameBufSize];
		ClientName aClientName;
		for (int iName = 0; iName < namecount; ++iName)
		{
			aClientName.erase();

			unsigned short namesize;
			fread(&namesize, sizeof(namesize), 1, aIgnoreFileH);
			
			while (namesize)
			{
				unsigned int aReadSize = namesize < aNameBufSize ? namesize : aNameBufSize;
				fread(aNameBuf, sizeof(wchar_t), aReadSize, aIgnoreFileH);
				aClientName.append(aNameBuf, aReadSize);
				namesize -= aReadSize;
			}

			mIgnoredClientNames.insert(aClientName);
		}

		fclose(aIgnoreFileH);
	}
}

// RoutingServerClient::SaveIgnoredUsersList
// Writes out a binary file (IGNOREFILE) containing the list of usernames that
// the local user has ignored.
void RoutingServerClient::SaveIgnoredUsersList() const
{
	FILE* aIgnoreFileH = fopen(IGNOREFILE, "wb");
	
	// write out "namecount" field
	unsigned long namecount = mIgnoredClientNames.size();
	fwrite(&namecount, sizeof(namecount), 1, aIgnoreFileH);

	// write out the list of names
	for (ClientNameSet::const_iterator itr = mIgnoredClientNames.begin(); itr != mIgnoredClientNames.end(); ++itr)
	{
		unsigned short namesize = itr->size();
		fwrite(&namesize, sizeof(namesize), 1, aIgnoreFileH);
		fwrite(itr->data(), sizeof(wchar_t), namesize, aIgnoreFileH);
	}

	fclose(aIgnoreFileH);
}

// RoutingServerClient::IsUserIgnored(ClientId)
// Returns true if the client with the specified id is currently being ignored.
bool RoutingServerClient::IsClientIgnored(WONMsg::ClientId theClientId) const
{ return mIgnoredClientIds.find(theClientId) != mIgnoredClientIds.end(); }
// RoutingServerClient::IsClientIgnored(ClientName&)
// Returns true if the client with the specified name is currently being ignored.
bool RoutingServerClient::IsClientIgnored(const ClientName& theClientNameR) const
{ return mIgnoredClientNames.find(theClientNameR) != mIgnoredClientNames.end(); }
