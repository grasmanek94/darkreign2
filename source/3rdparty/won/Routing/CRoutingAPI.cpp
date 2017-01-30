#pragma warning(disable:4786)

#ifdef _LINUX
# define __USE_GNU
# include <wchar.h>
#endif // _LINUX
#include "RoutingAPI.h"
#include "RoutingAPITypes.h"
#include "msg/Routing/RoutingServerTypes.h"
#include "CRoutingAPI.h"


using namespace WONAPI;
using namespace WONMsg;
using namespace std;
using namespace WONCommon;
//using WONAPI::RoutingServerClient::RegisterClientResult;
//using WONAPI::RoutingServerClient::GetClientListResult;
//using WONAPI::RoutingServerClient::GetClientInfoResult;
//using WONAPI::RoutingServerClient::GetGroupListResult;
//using WONAPI::RoutingServerClient::CreateGroupResult;
//using WONAPI::RoutingServerClient::ReadDataObjectResult;
//using WONAPI::RoutingServerClient::GetNumUsersResult;
//using WONAPI::RoutingServerClient::ClientIdWithReason;
//using WONAPI::RoutingServerClient::GroupIdAndClientIdWithReason;
//using WONAPI::RoutingServerClient::ChatMessageBase;
//using WONAPI::RoutingServerClient::GroupIdAndClientDataWithReason;
//using WONAPI::RoutingServerClient::GroupIdAndSpectatorCount;
//using WONAPI::RoutingServerClient::ClientDataWithReason;
//using WONAPI::RoutingServerClient::ClientIdWithFlag;
//using WONAPI::RoutingServerClient::DataObjectWithLifespan;
//using WONAPI::RoutingServerClient::DataObjectModification;
//using WONAPI::RoutingServerClient::ClientId;
//using WONAPI::RoutingServerClient::GroupId;
//using WONAPI::RoutingServerClient::GroupData;
//using WONAPI::RoutingServerClient::ClientName;
//using WONAPI::RoutingServerClient::Message;
//using WONAPI::RoutingServerClient::MessageWithClientId;
//using WONAPI::RoutingServerClient::ChatMessageBase;
//using WONAPI::RoutingServerClient::RawChatMessage;
//using WONAPI::RoutingServerClient::ASCIIChatMessage;
//using WONAPI::RoutingServerClient::UnicodeChatMessage;
//using WONAPI::RoutingServerClient::ClientDataResultBase;

//using WONMsg::MMsgRoutingGetClientListReply::ClientData;

struct CallbackStruct
{
	void *callback;
	void *userParam;

	CallbackStruct(void *theCallback, void *theUserParam) : callback(theCallback), userParam(theUserParam) {}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template <class T> 
static void CopyClientDataResultBase(T &theCopyTo, const RoutingServerClient::ClientDataResultBase& theCopyFrom)
{
	theCopyTo.mStatus = theCopyFrom.mStatus;
	theCopyTo.mIncludesIPs = theCopyFrom.mIncludesIPs?TRUE:FALSE;
	theCopyTo.mIncludesAuthInfo = theCopyFrom.mIncludesAuthInfo?TRUE:FALSE;
	theCopyTo.mIncludesMuteFlag = theCopyFrom.mIncludesMuteFlag?TRUE:FALSE;
	theCopyTo.mIncludesModeratorFlag = theCopyFrom.mIncludesModeratorFlag?TRUE:FALSE;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void CopyChatMessageBase(WONRoutingChatMessageBase &theCopyTo, const RoutingServerClient::ChatMessageBase &theCopyFrom)
{
	theCopyTo.mSenderId = theCopyFrom.mSenderId;
	theCopyTo.mChatType = (WONRoutingChatType)theCopyFrom.mChatType;
	theCopyTo.mIncludeExcludeFlag = theCopyFrom.mIncludeExcludeFlag?TRUE:FALSE;
	theCopyTo.mNumRecipientIds = theCopyFrom.mRecipientIds.size();
	theCopyTo.mRecipientIds = theCopyFrom.mRecipientIds.begin();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void CopyDataObject(WONRoutingDataObject &theCopyTo, const WONAPI::RoutingServerClient::DataObject &theCopyFrom)
{
	theCopyTo.mLinkId = theCopyFrom.mLinkId;
	theCopyTo.mOwnerId = theCopyFrom.mOwnerId;
	theCopyTo.mData = (const char*)theCopyFrom.mData.data();
	theCopyTo.mDataLen = theCopyFrom.mData.length();
	theCopyTo.mDataType = (const char*)theCopyFrom.mDataType.data();
	theCopyTo.mDataTypeLen = theCopyFrom.mDataType.length();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void CopyClientData(WONRoutingClientData &theCopyTo, const MMsgRoutingGetClientListReply::ClientData &theCopyFrom)
{
	theCopyTo.mClientId = theCopyFrom.mClientId;
	
	WONString aClientNameW((const wchar*)theCopyFrom.mClientName.data(),theCopyFrom.mClientName.length()/2);

	
	wchar_t* tmpClientNameW = (wchar_t*)malloc((aClientNameW.length() + 1) * sizeof(wchar_t));
	wcsncpy(tmpClientNameW, aClientNameW.GetUnicodeCString(), aClientNameW.length() + 1);
	theCopyTo.mClientNameW = tmpClientNameW;
	theCopyTo.mClientNameA = strdup(aClientNameW.GetAsciiCString());
	theCopyTo.mIPAddress = theCopyFrom.mIPAddress;
	theCopyTo.mWONUserId = theCopyFrom.mWONUserId;
	theCopyTo.mCommunityId = theCopyFrom.mCommunityId;
	theCopyTo.mTrustLevel = theCopyFrom.mTrustLevel;
	theCopyTo.mIsModerator = theCopyFrom.mIsModerator?TRUE:FALSE;
	theCopyTo.mIsMuted = theCopyFrom.mIsMuted?TRUE:FALSE;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void FreeClientData(WONRoutingClientData &theData)
{
	free((void*)theData.mClientNameW);
	free((void*)theData.mClientNameA);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static ClientName GetClientName(const char *theName)
{
	wstring aClientNameW = StringToWString(theName);
	int aLen = aClientNameW.length();

	return ClientName((unsigned char*)aClientNameW.data(),aLen*2);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static ClientName GetClientName(const wchar_t *theName)
{
	WONString aClientName(theName);
	return ClientName((const unsigned char*)aClientName.GetTitanUnicodeCString(), aClientName.GetTitanUnicodeString().size() * 2);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//Callbacks
static void VoidCompletion(CallbackStruct theStruct)
{
	if(theStruct.callback!=NULL)
		((WONRoutingVoidCallback)theStruct.callback)(theStruct.userParam);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void ErrorCompletion(Error theError, CallbackStruct theStruct)
{
	if(theStruct.callback!=NULL)
		((WONRoutingErrorCallback)theStruct.callback)(theError,theStruct.userParam);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void StatusCompletion(short theStatus, CallbackStruct theStruct)
{
	if(theStruct.callback!=NULL)
		((WONRoutingStatusCallback)theStruct.callback)(theStatus,theStruct.userParam);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void SpectatorCountCompletion(unsigned short theCount, CallbackStruct theStruct)
{
	if(theStruct.callback!=NULL)
		((WONRoutingClientIdCallback)theStruct.callback)(theCount,theStruct.userParam);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void ClientIdCompletion(ClientId theId, CallbackStruct theStruct)
{
	if(theStruct.callback!=NULL)
		((WONRoutingClientIdCallback)theStruct.callback)(theId,theStruct.userParam);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void BoolCompletion(bool theBool, CallbackStruct theStruct)
{
	if(theStruct.callback!=NULL)
		((WONRoutingBoolCallback)theStruct.callback)(theBool?TRUE:FALSE,theStruct.userParam);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void RegisterClientResultCompletion(const RoutingServerClient::RegisterClientResult &theResult, CallbackStruct theStruct)
{
	if(theStruct.callback==NULL)
		return;

	WONRoutingRegisterClientResult aResult;
	aResult.mClientId = theResult.mClientId;

	wchar_t* tmpHostCommentW = (wchar_t*)malloc((theResult.mHostComment.length() + 1) * sizeof(wchar_t));
	wcsncpy(tmpHostCommentW, theResult.mHostComment.data(), theResult.mHostComment.length() + 1);
	aResult.mHostCommentW = tmpHostCommentW;
	
	string aHostCommentA = WStringToString(theResult.mHostComment);
	aResult.mHostCommentA = aHostCommentA.c_str();

	aResult.mHostNameA = (const char*)theResult.mHostName.c_str();
	wstring aHostNameW = StringToWString(aResult.mHostNameA);

	wchar_t* tmpHostNameW = (wchar_t*)malloc((theResult.mHostName.length() + 1) * sizeof(wchar_t));
	wcsncpy(tmpHostNameW, aHostNameW.data(), aHostNameW.length() + 1);
	aResult.mHostNameW = tmpHostNameW;

	aResult.mStatus = theResult.mStatus;

	((WONRoutingRegisterClientCallback)theStruct.callback)(&aResult,theStruct.userParam);

	free(tmpHostCommentW);
	free(tmpHostNameW);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void GetClientListCompletion(const RoutingServerClient::GetClientListResult& theResult, CallbackStruct theStruct)
{
	if(theStruct.callback==NULL) // save some computation
		return;

	WONRoutingClientListResult aResult;
	CopyClientDataResultBase(aResult,theResult);

	aResult.mNumSpectators = theResult.mNumSpectators;

	aResult.mNumClients = theResult.mClientList.size();
	if(aResult.mNumClients>0)
		aResult.mClients = new WONRoutingClientData[aResult.mNumClients];
	else
		aResult.mClients = NULL;

	std::list<MMsgRoutingGetClientListReply::ClientData>::const_iterator anItr = theResult.mClientList.begin();
	int i=0;
	while(anItr!=theResult.mClientList.end())
	{
		CopyClientData(aResult.mClients[i],*anItr);

		anItr++;
		i++;
	}

	((WONRoutingClientListCallback)theStruct.callback)(&aResult,theStruct.userParam);

	for(i=0; i<aResult.mNumClients; i++)
		FreeClientData(aResult.mClients[i]);

	delete [] aResult.mClients;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void GetClientInfoCompletion(const RoutingServerClient::GetClientInfoResult& theResult, CallbackStruct theStruct)
{
	if(theStruct.callback==NULL)
		return;

	WONRoutingClientInfoResult aResult;
	CopyClientDataResultBase(aResult,theResult);
	CopyClientData(aResult.mClient,theResult.mClientData);

	((WONRoutingClientInfoCallback)theStruct.callback)(&aResult,theStruct.userParam);
	FreeClientData(aResult.mClient);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void GetGroupListCompletion(const RoutingServerClient::GetGroupListResult& theResult, CallbackStruct theStruct)
{
	if(theStruct.callback==NULL)
		return;

	WONRoutingGroupListResult aResult;
	aResult.mStatus = theResult.mStatus;
	aResult.mNumGroups = theResult.mGroupList.size();
	
	if(aResult.mNumGroups==0)
		aResult.mGroups = NULL;
	else
		aResult.mGroups = new WONRoutingGroupData[aResult.mNumGroups];

	std::list<RoutingServerClient::GroupData>::const_iterator anItr = theResult.mGroupList.begin();
	int i=0;
	while(anItr!=theResult.mGroupList.end())
	{
		aResult.mGroups[i].mGroupId = anItr->mGroupId;
		aResult.mGroups[i].mGroupName = anItr->mGroupName.c_str();
		aResult.mGroups[i].mIsPublic = anItr->mIsPublic ? TRUE : FALSE;

		anItr++;
		i++;
	}

	((WONRoutingGroupListCallback)theStruct.callback)(&aResult,theStruct.userParam);

	delete [] aResult.mGroups;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void CreateGroupCompletion(const RoutingServerClient::CreateGroupResult& theResult, CallbackStruct theStruct)
{
	if(theStruct.callback==NULL)
		return;

	WONRoutingCreateGroupResult aResult;
	aResult.mGroupId = theResult.mGroupId;
	aResult.mStatus = theResult.mStatus;

	((WONRoutingCreateGroupCallback)theStruct.callback)(&aResult,theStruct.userParam);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void ReadDataObjectCompletion(const RoutingServerClient::ReadDataObjectResult& theResult, CallbackStruct theStruct)
{
	if(theStruct.callback==NULL)
		return;

	WONRoutingReadDataObjectResult aResult;

	aResult.mStatus = theResult.mStatus;
	aResult.mNumObjects = theResult.mDataObjectList.size();

	if(aResult.mNumObjects==0)
		aResult.mObjects = NULL;
	else 
		aResult.mObjects = new WONRoutingDataObject[aResult.mNumObjects];


	std::list<WONAPI::RoutingServerClient::DataObject>::const_iterator anItr = theResult.mDataObjectList.begin();
	int i=0;
	while(anItr!=theResult.mDataObjectList.end())
	{
		CopyDataObject(aResult.mObjects[i],*anItr);
		anItr++;
		i++;
	}

	((WONRoutingReadDataObjectCallback)theStruct.callback)(&aResult,theStruct.userParam);

	delete [] aResult.mObjects;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void DataObjectCompletion(const RoutingServerClient::DataObject &theData, CallbackStruct theStruct)
{
	if(theStruct.callback==NULL)
		return;

	WONRoutingDataObject aData;
	CopyDataObject(aData,theData);

	((WONRoutingDataObjectCallback)theStruct.callback)(&aData,theStruct.userParam);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void DataObjectWithLifespanCompletion(const RoutingServerClient::DataObjectWithLifespan &theData, CallbackStruct theStruct)
{
	if(theStruct.callback==NULL)
		return;

	WONRoutingDataObjectWithLifespan aData;
	CopyDataObject(aData.mObject,theData);
	aData.mLifespan = theData.mLifespan;

	((WONRoutingDataObjectWithLifespanCallback)theStruct.callback)(&aData,theStruct.userParam);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void DataObjectModificationCompletion(const RoutingServerClient::DataObjectModification &theData, CallbackStruct theStruct)
{
	if(theStruct.callback==NULL)
		return;

	WONRoutingDataObjectModification aData;
	CopyDataObject(aData.mObject,theData);
	aData.mIsInsert = theData.mIsInsert?TRUE:FALSE;
	aData.mOffset = theData.mOffset;

	((WONRoutingDataObjectModificationCallback)theStruct.callback)(&aData,theStruct.userParam);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void GetNumUsersCompletion(const RoutingServerClient::GetNumUsersResult& theResult, CallbackStruct theStruct)
{
	if(theStruct.callback==NULL)
		return;

	WONRoutingGetNumUsersResult aResult;
	aResult.mNumActiveUsers = theResult.mNumActiveUsers;
	aResult.mServerCapacity = theResult.mServerCapacity;

	((WONRoutingGetNumUsersCallback)theStruct.callback)(&aResult,theStruct.userParam);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void ClientNameCompletion(const RoutingServerClient::ClientName& theName, CallbackStruct theStruct)
{
	if(theStruct.callback==NULL)
		return;

	WONString aClientNameW((const wchar*)theName.c_str(),theName.length()/2);
	string aClientNameA = aClientNameW.GetAsciiString();

	WONRoutingClientNameResult aResult;
	aResult.mNameA = aClientNameA.c_str();

	wchar_t* tmpNameW = (wchar_t*)malloc((aClientNameW.length() + 1) * sizeof(wchar_t));
	wcsncpy(tmpNameW, aClientNameW.GetUnicodeCString(), aClientNameW.length() + 1);
	aResult.mNameW = tmpNameW;

	((WONRoutingClientNameCallback)theStruct.callback)(&aResult,theStruct.userParam);

	free(tmpNameW);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void ClientIdWithReasonCompletion(const RoutingServerClient::ClientIdWithReason& theReason, CallbackStruct theStruct)
{
	if(theStruct.callback==NULL)
		return;

	WONRoutingClientIdWithReason aReason;

	aReason.mClientId = theReason.mClientId;
	aReason.mReason = theReason.mReason;

	((WONRoutingClientIdWithReasonCallback)theStruct.callback)(&aReason,theStruct.userParam);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void ClientDataWithReasonCompletion(const RoutingServerClient::ClientDataWithReason& theReason, CallbackStruct theStruct)
{
	if(theStruct.callback==NULL)
		return;

	WONRoutingClientDataWithReason aReason;
	aReason.mReason = theReason.mReason;
	CopyClientData(aReason.mData, theReason);

	((WONRoutingClientDataWithReasonCallback)theStruct.callback)(&aReason,theStruct.userParam);
	FreeClientData(aReason.mData);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void GroupIdAndClientDataWithReasonCompletion(const RoutingServerClient::GroupIdAndClientDataWithReason& theReason, CallbackStruct theStruct)
{
	if(theStruct.callback==NULL)
		return;

	WONRoutingGroupIdAndClientDataWithReason aReason;
	aReason.mReason = theReason.mReason;
	aReason.mGroupId = theReason.mGroupId;
	CopyClientData(aReason.mData, theReason);

	((WONRoutingGroupIdAndClientDataWithReasonCallback)theStruct.callback)(&aReason,theStruct.userParam);
	FreeClientData(aReason.mData);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void GroupIdAndClientIdWithReasonCompletion(const RoutingServerClient::GroupIdAndClientIdWithReason &theReason, CallbackStruct theStruct)
{
	if(theStruct.callback==NULL)
		return;

	WONRoutingGroupIdAndClientIdWithReason aReason;
	aReason.mGroupId = theReason.mGroupId;
	aReason.mClientId = theReason.mClientId;
	aReason.mReason = theReason.mReason;

	((WONRoutingGroupIdAndClientIdWithReasonCallback)theStruct.callback)(&aReason,theStruct.userParam);

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void GroupIdAndSpectatorCountCompletion(const RoutingServerClient::GroupIdAndSpectatorCount &theCount, CallbackStruct theStruct)
{
	if(theStruct.callback==NULL)
		return;

	WONRoutingGroupIdAndSpectatorCount aCount;
	aCount.mGroupId = theCount.mGroupId;
	aCount.mSpectatorCount = theCount.mSpectatorCount;

	((WONRoutingGroupIdAndSpectatorCountCallback)theStruct.callback)(&aCount,theStruct.userParam);

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void ClientIdWithFlagCompletion(const RoutingServerClient::ClientIdWithFlag &theFlag, CallbackStruct theStruct)
{
	if(theStruct.callback==NULL)
		return;

	WONRoutingClientIdWithFlag aFlag;
	aFlag.mClientId = theFlag.mClientId;
	aFlag.mFlagOnOrOff = theFlag.mFlagOnOrOff;

	((WONRoutingClientIdWithFlagCallback)theStruct.callback)(&aFlag,theStruct.userParam);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void MessageWithClientIdCompletion(const RoutingServerClient::MessageWithClientId &theMsg, CallbackStruct theStruct)
{
	if(theStruct.callback==NULL)
		return;

	WONRoutingMessageWithClientId aMsg;
	aMsg.mClientId = theMsg.mClientId;
	aMsg.mData = (const char*)theMsg.mBufP;
	aMsg.mDataLen = theMsg.mBufLen;

	((WONRoutingMessageWithClientIdCallback)theStruct.callback)(&aMsg,theStruct.userParam);

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void RawChatMessageCompletion(const RoutingServerClient::RawChatMessage &theMsg, CallbackStruct theStruct)
{
	if(theStruct.callback==NULL)
		return;

	WONRoutingRawChatMessage aMsg;
	aMsg.mData = (const char*)theMsg.mBufP;
	aMsg.mDataLen = theMsg.mBufLen;

	CopyChatMessageBase(aMsg.mBaseData,theMsg);

	((WONRoutingRawChatMessageCallback)theStruct.callback)(&aMsg,theStruct.userParam);

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void ASCIIChatMessageCompletion(const RoutingServerClient::ASCIIChatMessage &theMsg, CallbackStruct theStruct)
{
	if(theStruct.callback==NULL)
		return;

	WONRoutingASCIIChatMessage aMsg;
	aMsg.mChat = (const char*)theMsg.mData.c_str();

	CopyChatMessageBase(aMsg.mBaseData,theMsg);

	((WONRoutingASCIIChatMessageCallback)theStruct.callback)(&aMsg,theStruct.userParam);

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void UnicodeChatMessageCompletion(const RoutingServerClient::UnicodeChatMessage &theMsg, CallbackStruct theStruct)
{
	if(theStruct.callback==NULL)
		return;

	WONRoutingUnicodeChatMessage aMsg;

	wchar_t* tmpChat = (wchar_t*)malloc((theMsg.mData.length() + 1) * sizeof(wchar_t));
	wcsncpy(tmpChat, theMsg.mData.data(), theMsg.mData.length() + 1);
	aMsg.mChat = tmpChat;

	CopyChatMessageBase(aMsg.mBaseData,theMsg);

	((WONRoutingUnicodeChatMessageCallback)theStruct.callback)(&aMsg,theStruct.userParam);
	
	free(tmpChat);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//Construction/Destruction

HWONROUTINGSESSION WONRoutingCreateSession()
{
	return (HWONROUTINGSESSION)new RoutingServerClient(true,true,false,2);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

HWONROUTINGSESSION WONRoutingCreateSessionEx(BOOL useTCP, 
											 BOOL enableShortcuts,
                                             BOOL activateIgnoring,
											 unsigned char theLengthFieldSize)
{
	return (HWONROUTINGSESSION)new RoutingServerClient(useTCP?true:false,enableShortcuts?true:false,activateIgnoring?true:false,theLengthFieldSize);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingCloseSessionHandle(HWONROUTINGSESSION theSession)
{
	delete (RoutingServerClient*)theSession;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Connecting/Disconnecting/Reconnecting

WONError WONRoutingConnect(HWONROUTINGSESSION theSession, 
						   const WONIPAddress* theServerAddress, 
						   HWONAUTH hAuth, 
						   long theTimeout)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	IPSocket::Address anAddr(theServerAddress->ipAddress,theServerAddress->portNum);
	return 
		aClient->Connect(anAddr, (Identity*)hAuth, false, theTimeout, false);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingConnectAsync(HWONROUTINGSESSION theSession, 
							const WONIPAddress* theServerAddress, 
							HWONAUTH hAuth, long theTimeout, 
							WONRoutingErrorCallback theCallback,
							void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;


	IPSocket::Address anAddr(theServerAddress->ipAddress,theServerAddress->portNum);

	aClient->ConnectEx(anAddr, 
					  (Identity*)hAuth, 
					  false, 
					  theTimeout, 
					  true, 
					  ErrorCompletion, 
					  CallbackStruct((void*)theCallback, theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

WONError WONRoutingReConnect(HWONROUTINGSESSION theSession, 
							 const WONIPAddress* theServerAddress, 
							 HWONAUTH hAuth, 
							 long theTimeout)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;
	IPSocket::Address anAddr(theServerAddress->ipAddress,theServerAddress->portNum);

	return 
		aClient->Connect(anAddr, (Identity*)hAuth, true, theTimeout, false);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingReConnectAsync(HWONROUTINGSESSION theSession, 
							  const WONIPAddress* theServerAddress, 
							  HWONAUTH hAuth, 
							  long theTimeout, 
							  WONRoutingErrorCallback theCallback,
							  void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	IPSocket::Address anAddr(theServerAddress->ipAddress,theServerAddress->portNum);
	
	aClient->ConnectEx(anAddr, 
					  (Identity*)hAuth, 
					  true, 
					  theTimeout, 
					  true, 
					  ErrorCompletion, 
					  CallbackStruct((void*)theCallback, theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


void  WONRoutingCloseConnection(HWONROUTINGSESSION theSession)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->Close();
	
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Registration


void WONRoutingRegisterA(HWONROUTINGSESSION theSession, 
							 const char* theClientName, 
							 const char* thePassword, 
							 BOOL becomeSpectator, 
							 BOOL becomeHost, 
							 BOOL joinChat, 
							 WONRoutingRegisterClientCallback theCallback,
							 void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->RegisterEx(GetClientName(theClientName), 
					  thePassword, 
					  becomeHost?true:false, 
					  becomeSpectator?true:false, 
					  joinChat?true:false, 
					  RegisterClientResultCompletion,
					  CallbackStruct((void*)theCallback, theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingRegisterW(HWONROUTINGSESSION theSession, 
							 const wchar_t* theClientName, 
							 const wchar_t* thePassword, 
							 BOOL becomeSpectator, 
							 BOOL becomeHost, 
							 BOOL joinChat, 
							 WONRoutingRegisterClientCallback theCallback,
							 void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->RegisterEx(GetClientName(theClientName), 
					  thePassword, 
					  becomeHost?true:false, 
					  becomeSpectator?true:false, 
					  joinChat?true:false, 
					  RegisterClientResultCompletion,
					  CallbackStruct((void*)theCallback, theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingDeregister(HWONROUTINGSESSION theSession, 
							   BOOL isPermanent, 
							   WONRoutingStatusCallback theCallback,
							   void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	
	aClient->DisconnectEx(isPermanent?true:false,
						  StatusCompletion,
						  CallbackStruct((void*)theCallback, theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingReregister(HWONROUTINGSESSION theSession, 
							   BOOL getMissedMessages, 
							   WONRoutingStatusCallback theCallback,
							   void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->ReconnectEx(getMissedMessages?true:false,
					     StatusCompletion,
					     CallbackStruct((void*)theCallback, theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

WONRoutingClientId WONRoutingGetClientId(HWONROUTINGSESSION theSession)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	return aClient->GetClientId();

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

BOOL WONRoutingIsRegistered(HWONROUTINGSESSION theSession)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	return aClient->IsRegistered() ? TRUE : FALSE;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Client lists

void WONRoutingGetClientList(HWONROUTINGSESSION theSession,
								  BOOL requestAuthInfo,  
								  BOOL requestMuteFlag, 
								  BOOL requestModeratorFlag, 
								  WONRoutingClientListCallback theCallback, 
								  void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->GetClientListEx(requestAuthInfo?true:false,
					  		 requestMuteFlag?true:false,
							 requestModeratorFlag?true:false,
							 GetClientListCompletion,
							 CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingGetMembersOfGroup(HWONROUTINGSESSION theSession,
									  WONRoutingGroupId theGroupId, 
									  BOOL requestAuthInfo, 
									  BOOL requestMuteFlag, 
									  BOOL requestModeratorFlag, 
									  WONRoutingClientListCallback theCallback, 
									  void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->GetMembersOfGroupEx(theGroupId,
								 requestAuthInfo?true:false,
								 requestMuteFlag?true:false,
								 requestModeratorFlag?true:false,
								 GetClientListCompletion,
								 CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingGetClientInfo(HWONROUTINGSESSION theSession, 
 								  WONRoutingClientId theClientId, 
								  BOOL requestAuthInfo, 
								  BOOL requestMuteFlag, 
								  BOOL requestModeratorFlag, 
								  WONRoutingClientInfoCallback theCallback, 
								  void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;
	
	aClient->GetClientInfoEx(theClientId,
							 requestAuthInfo?true:false,
							 requestMuteFlag?true:false,
							 requestModeratorFlag?true:false,
							 GetClientInfoCompletion,
							 CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingGetGroupList(HWONROUTINGSESSION theSession, 
								 WONRoutingGroupListCallback theCallback, 
								 void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->GetGroupListEx(GetGroupListCompletion,CallbackStruct((void*)theCallback,theUserParam));
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Send raw data
void WONRoutingBroadcastData(HWONROUTINGSESSION theSession, 
							 const void *theData, 
							 unsigned short theDataLen)
{
	WONRoutingBroadcastDataAndGetResult(theSession,theData,theDataLen,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingSendData(HWONROUTINGSESSION theSession,
						const WONRoutingClientOrGroupId theRecipients[], 
						unsigned short theNumRecipients, 
						BOOL flagIncludeExclude, 
						const void *theData, 
						unsigned short theDataLen)
{
	WONRoutingSendDataAndGetResult(theSession,theRecipients,theNumRecipients,flagIncludeExclude,theData,theDataLen,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingSendDataMultiple(HWONROUTINGSESSION theSession, 
								const WONRoutingClientOrGroupId theRecipients[], 
								unsigned short theNumRecipients, 
								BOOL flagIncludeExclude, 
								const WONRoutingMessage theMessages[], 
								unsigned short theNumMessages)
{
	WONRoutingSendDataMultipleAndGetResult(theSession,theRecipients,theNumRecipients,flagIncludeExclude,theMessages,theNumMessages,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingBroadcastDataAndGetResult(HWONROUTINGSESSION theSession, 
										 const void *theData, 
										 unsigned short theDataLen, 
										 WONRoutingStatusCallback theCallback, 
										 void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	RawBuffer aBuf((const unsigned char*)theData,theDataLen);
	aClient->BroadcastDataEx(aBuf,true,StatusCompletion,CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingSendDataAndGetResult(HWONROUTINGSESSION theSession, 
									const WONRoutingClientOrGroupId theRecipients[], 
									unsigned short theNumRecipients, 
									BOOL flagIncludeExclude, 
									const void *theData, 
									unsigned short theDataLen, 
									WONRoutingStatusCallback theCallback, 
									void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	
	RawBuffer aBuf((const unsigned char*)theData,theDataLen);
	aClient->SendDataEx(theRecipients,
						theNumRecipients,
						flagIncludeExclude?true:false,
						aBuf,
						true,
						StatusCompletion,
						CallbackStruct((void*)theCallback,theUserParam));
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingSendDataMultipleAndGetResult(HWONROUTINGSESSION theSession, 
											const WONRoutingClientOrGroupId theRecipients[], 
											unsigned short theNumRecipients, 
											BOOL flagIncludeExclude, 
											const WONRoutingMessage theMessages[], 
											unsigned short theNumMessages, 
											WONRoutingStatusCallback theCallback, 
											void *theUserParam)
{
	if(theNumMessages==0)
		return;

	RoutingServerClient *aClient = (RoutingServerClient*)theSession;


	RoutingServerClient::Message *aMessage = new RoutingServerClient::Message[theNumMessages];
	for(int i=0; i<theNumMessages; i++)
	{
		aMessage[i].mBufLen = theMessages[i].mDataLen;
		aMessage[i].mBufP = (const unsigned char*)theMessages[i].mData;
	}

	aClient->SendDataMultipleEx(theRecipients,
							    theNumRecipients,
								flagIncludeExclude?true:false,
								aMessage,
								theNumMessages,
								true,
								StatusCompletion,
								CallbackStruct((void*)theCallback,theUserParam));
	delete [] aMessage;

}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Send chat

void WONRoutingBroadcastChatA(HWONROUTINGSESSION theSession, const char *theChat, BOOL emote)
{
	WONRoutingBroadcastChatAndGetResultA(theSession,theChat,emote,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingBroadcastChatW(HWONROUTINGSESSION theSession, const wchar_t *theChat, BOOL emote)
{
	WONRoutingBroadcastChatAndGetResultW(theSession,theChat,emote,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingSendChatA(HWONROUTINGSESSION theSession, 
						 const WONRoutingClientOrGroupId theRecipients[],
						 unsigned short theNumRecipients,
						 BOOL flagIncludeExclude,
						 const char *theChat)
{
	WONRoutingSendChatAndGetResultA(theSession,theRecipients,theNumRecipients,flagIncludeExclude,theChat,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingSendChatW(HWONROUTINGSESSION theSession, 
						 const WONRoutingClientOrGroupId theRecipients[],
						 unsigned short theNumRecipients,
						 BOOL flagIncludeExclude,
						 const wchar_t *theChat)
{
	WONRoutingSendChatAndGetResultW(theSession,theRecipients,theNumRecipients,flagIncludeExclude,theChat,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingBroadcastChatAndGetResultA(HWONROUTINGSESSION theSession, 
										  const char *theChat, 
										  BOOL emote, 
										  WONRoutingStatusCallback theCallback, 
										  void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	RoutingServerClient::Message aMessage;
	aMessage.mBufLen = strlen(theChat);
	aMessage.mBufP = (const unsigned char*)theChat;

	aClient->BroadcastChatEx(aMessage, 
							 emote?WONMsg::CHATTYPE_ASCII_EMOTE:WONMsg::CHATTYPE_ASCII,
							 true,
							 StatusCompletion,
							 CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingBroadcastChatAndGetResultW(HWONROUTINGSESSION theSession, 
										  const wchar_t *theChat, 
										  BOOL emote,
										  WONRoutingStatusCallback theCallback,
										  void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	RoutingServerClient::Message aMessage;
	aMessage.mBufLen = wcslen(theChat)*2;
	aMessage.mBufP = (const unsigned char*)theChat;

	aClient->BroadcastChatEx(aMessage, 
						     emote?WONMsg::CHATTYPE_UNICODE_EMOTE:WONMsg::CHATTYPE_UNICODE,
						     true,
						     StatusCompletion,
						     CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingSendChatAndGetResultA(HWONROUTINGSESSION theSession, 
						 const WONRoutingClientOrGroupId theRecipients[],
						 unsigned short theNumRecipients,
						 BOOL flagIncludeExclude,
						 const char *theChat,
						 WONRoutingStatusCallback theCallback,
						 void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	RoutingServerClient::Message aMessage;
	aMessage.mBufLen = strlen(theChat);
	aMessage.mBufP = (const unsigned char*)theChat;

	aClient->WhisperChatEx(theRecipients, 
						   theNumRecipients, 
						   flagIncludeExclude?true:false,
						   aMessage, 
						   WONMsg::CHATTYPE_ASCII,
						   true,
						   StatusCompletion,
						   CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingSendChatAndGetResultW(HWONROUTINGSESSION theSession, 
						 const WONRoutingClientOrGroupId theRecipients[],
						 unsigned short theNumRecipients,
						 BOOL flagIncludeExclude,
						 const wchar_t *theChat,
						 WONRoutingStatusCallback theCallback,
						 void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	RoutingServerClient::Message aMessage;
	aMessage.mBufLen = wcslen(theChat)*2;
	aMessage.mBufP = (const unsigned char*)theChat;

	aClient->WhisperChatEx(theRecipients,
						   theNumRecipients, 
						   flagIncludeExclude?true:false,
						   aMessage, 
						   WONMsg::CHATTYPE_UNICODE,
						   true,
						   StatusCompletion,
						   CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Ignoring
void WONRoutingIgnoreClientA(HWONROUTINGSESSION theSession, const char* theClientName)
{ 
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;
	aClient->IgnoreClient((const unsigned char*)theClientName); 
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WONRoutingIgnoreClientW(HWONROUTINGSESSION theSession, const wchar_t* theClientName)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;
	aClient->IgnoreClient(RawBuffer((unsigned char*)theClientName, wcslen(theClientName) * 2));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WONRoutingUnignoreClientA(HWONROUTINGSESSION theSession, const char* theClientName)
{ 
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;
	aClient->UnignoreClient((const unsigned char*)theClientName); 
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WONRoutingUnignoreClientW(HWONROUTINGSESSION theSession, const wchar_t* theClientName)
{ 
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;
	aClient->UnignoreClient(RawBuffer((unsigned char*)theClientName, wcslen(theClientName) * 2));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Administrative
void WONRoutingOpenRegistration(HWONROUTINGSESSION theSession)
{
	WONRoutingOpenRegistrationAndGetResult(theSession,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingCloseRegistration(HWONROUTINGSESSION theSession)
{
	WONRoutingCloseRegistrationAndGetResult(theSession,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingSetPasswordA(HWONROUTINGSESSION theSession, const char *thePassword)
{
	WONRoutingSetPasswordAndGetResultA(theSession,thePassword,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingSetPasswordW(HWONROUTINGSESSION theSession, const wchar_t *thePassword)
{
	WONRoutingSetPasswordAndGetResultW(theSession,thePassword,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingBecomeModerator(HWONROUTINGSESSION theSession, BOOL flagOnOrOff)
{
	WONRoutingBecomeModeratorAndGetResult(theSession,flagOnOrOff,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingBootClient(HWONROUTINGSESSION theSession, WONRoutingClientId theClientId)
{
	WONRoutingBootClientAndGetResult(theSession, theClientId,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingBanClientA(HWONROUTINGSESSION theSession, const char *theClientName)
{
	WONRoutingBanClientAndGetResultA(theSession,theClientName,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingBanClientW(HWONROUTINGSESSION theSession, const wchar_t *theClientName)
{
	WONRoutingBanClientAndGetResultW(theSession,theClientName,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingUnbanClientA(HWONROUTINGSESSION theSession,const char *theClientName)
{
	WONRoutingUnbanClientAndGetResultA(theSession,theClientName,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingUnbanClientW(HWONROUTINGSESSION theSession,const wchar_t *theClientName)
{
	WONRoutingUnbanClientAndGetResultW(theSession,theClientName,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingInviteClientA(HWONROUTINGSESSION theSession,const char *theClientName)
{
	WONRoutingInviteClientAndGetResultA(theSession,theClientName,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingInviteClientW(HWONROUTINGSESSION theSession,const wchar_t *theClientName)
{
	WONRoutingInviteClientAndGetResultW(theSession,theClientName,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingUninviteClientA(HWONROUTINGSESSION theSession,const char *theClientName)
{
	WONRoutingUninviteClientAndGetResultA(theSession,theClientName,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingUninviteClientW(HWONROUTINGSESSION theSession,const wchar_t *theClientName)
{
	WONRoutingUninviteClientAndGetResultW(theSession,theClientName,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingMuteClient(HWONROUTINGSESSION theSession,WONRoutingClientId theClientId)
{
	WONRoutingMuteClientAndGetResult(theSession,theClientId,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingUnmuteClient(HWONROUTINGSESSION theSession,WONRoutingClientId theClientId)
{
	WONRoutingUnmuteClientAndGetResult(theSession,theClientId,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingAddSuccessor(HWONROUTINGSESSION theSession,WONRoutingClientId theClientId)
{
	WONRoutingAddSuccessorAndGetResult(theSession,theClientId,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingRemoveSuccessor(HWONROUTINGSESSION theSession,WONRoutingClientId theClientId)
{
	WONRoutingRemoveSuccessorAndGetResult(theSession,theClientId,NULL,NULL);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingOpenRegistrationAndGetResult(HWONROUTINGSESSION theSession, WONRoutingStatusCallback theCallback, void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->OpenRegistrationEx(StatusCompletion,CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingCloseRegistrationAndGetResult(HWONROUTINGSESSION theSession, WONRoutingStatusCallback theCallback, void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->CloseRegistrationEx(StatusCompletion,CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingSetPasswordAndGetResultA(HWONROUTINGSESSION theSession, 
										const char *thePassword, 
										WONRoutingStatusCallback theCallback, 
										void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->SetPasswordEx(StringToWString(thePassword),StatusCompletion,CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingSetPasswordAndGetResultW(HWONROUTINGSESSION theSession, 
										const wchar_t *thePassword, 
										WONRoutingStatusCallback theCallback, 
										void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->SetPasswordEx(thePassword,StatusCompletion,CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingBecomeModeratorAndGetResult(HWONROUTINGSESSION theSession, 
										   BOOL flagOnOrOff, 
										   WONRoutingStatusCallback theCallback, 
										   void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->BecomeModeratorEx(flagOnOrOff?true:false,StatusCompletion,CallbackStruct((void*)theCallback,theUserParam));

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingBootClientAndGetResult(HWONROUTINGSESSION theSession, 
									  WONRoutingClientId theClientId,
									  WONRoutingStatusCallback theCallback, 
									  void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->BootClientEx(theClientId,StatusCompletion,CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingBanClientAndGetResultA(HWONROUTINGSESSION theSession, 
									  const char *theClientName, 
									  WONRoutingStatusCallback theCallback, 
									  void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->BanClientNameEx(GetClientName(theClientName),StatusCompletion,CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingBanClientAndGetResultW(HWONROUTINGSESSION theSession, 
									  const wchar_t *theClientName, 
									  WONRoutingStatusCallback theCallback, 
									  void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->BanClientNameEx(GetClientName(theClientName),StatusCompletion,CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingUnbanClientAndGetResultA(HWONROUTINGSESSION theSession,
										const char *theClientName, 
										WONRoutingStatusCallback theCallback, 
										void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->UnbanClientNameEx(GetClientName(theClientName),StatusCompletion,CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingUnbanClientAndGetResultW(HWONROUTINGSESSION theSession,
										const wchar_t *theClientName, 
										WONRoutingStatusCallback theCallback, 
										void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->UnbanClientNameEx(GetClientName(theClientName),StatusCompletion,CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingInviteClientAndGetResultA(HWONROUTINGSESSION theSession,
										 const char *theClientName, 
										 WONRoutingStatusCallback theCallback, 
										 void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->InviteClientEx(GetClientName(theClientName),StatusCompletion,CallbackStruct((void*)theCallback,theUserParam));
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingInviteClientAndGetResultW(HWONROUTINGSESSION theSession,
										 const wchar_t *theClientName, 
										 WONRoutingStatusCallback theCallback, 
										 void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->InviteClientEx(GetClientName(theClientName),StatusCompletion,CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingUninviteClientAndGetResultA(HWONROUTINGSESSION theSession,
										   const char *theClientName, 
										   WONRoutingStatusCallback theCallback, 
										   void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->UninviteClientEx(GetClientName(theClientName),StatusCompletion,CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingUninviteClientAndGetResultW(HWONROUTINGSESSION theSession,
										   const wchar_t *theClientName, 
										   WONRoutingStatusCallback theCallback, 
										   void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->UninviteClientEx(GetClientName(theClientName),StatusCompletion,CallbackStruct((void*)theCallback,theUserParam));
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingMuteClientAndGetResult(HWONROUTINGSESSION theSession,
									  WONRoutingClientId theClientId, 
									  WONRoutingStatusCallback theCallback, 
									  void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->MuteClientEx(theClientId,StatusCompletion,CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingUnmuteClientAndGetResult(HWONROUTINGSESSION theSession,WONRoutingClientId theClientId, WONRoutingStatusCallback theCallback, void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->UnmuteClientEx(theClientId,StatusCompletion,CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingAddSuccessorAndGetResult(HWONROUTINGSESSION theSession,WONRoutingClientId theClientId, WONRoutingStatusCallback theCallback, void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->AddSuccessorEx(theClientId,StatusCompletion,CallbackStruct((void*)theCallback,theUserParam));

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingRemoveSuccessorAndGetResult(HWONROUTINGSESSION theSession,WONRoutingClientId theClientId, WONRoutingStatusCallback theCallback, void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->RemoveSuccessorEx(theClientId,StatusCompletion,CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Group management

void WONRoutingCreateGroup(HWONROUTINGSESSION theSession, 
						   const char *theGroupName, 
						   BOOL isPublic, 
						   BOOL announceChanges, 
						   const WONRoutingClientId theInitialMembers[], 
						   unsigned short theNumInitialMembers)
{
	WONRoutingCreateGroupAndGetResult(theSession,theGroupName,isPublic,announceChanges,theInitialMembers,theNumInitialMembers,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingDeleteGroup(HWONROUTINGSESSION theSession, WONRoutingGroupId theGroupId)
{
	WONRoutingDeleteGroupAndGetResult(theSession,theGroupId,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingAddToGroup(HWONROUTINGSESSION theSession, 
						  WONRoutingClientId theClientId, 
						  WONRoutingGroupId theGroupId)
{
	WONRoutingAddToGroupAndGetResult(theSession,theClientId,theGroupId,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingRemoveFromGroup(HWONROUTINGSESSION theSession, 
							   WONRoutingClientId theClientId, 
							   WONRoutingGroupId theGroupId)
{
	WONRoutingRemoveFromGroupAndGetResult(theSession,theClientId,theGroupId,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingCreateGroupAndGetResult(HWONROUTINGSESSION theSession, 
									   const char *theGroupName, 
									   BOOL isPublic, 
									   BOOL announceChanges, 
									   const WONRoutingClientId theInitialMembers[], 
									   unsigned short theNumInitialMembers, 
									   WONRoutingCreateGroupCallback theCallback, 
									   void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->CreateGroupEx(theGroupName,
						   isPublic?true:false,
						   announceChanges?true:false,
						   theInitialMembers,
						   theNumInitialMembers,
						   CreateGroupCompletion,
						   CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingDeleteGroupAndGetResult(HWONROUTINGSESSION theSession, 
									   WONRoutingGroupId theGroupId, 
									   WONRoutingStatusCallback theCallback, 
									   void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->DeleteGroupEx(theGroupId,StatusCompletion,CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingAddToGroupAndGetResult(HWONROUTINGSESSION theSession, 
									  WONRoutingClientId theClientId, 
									  WONRoutingGroupId theGroupId, 
									  WONRoutingStatusCallback theCallback, 
									  void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->AddToGroupEx(theClientId,theGroupId,StatusCompletion,CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingRemoveFromGroupAndGetResult(HWONROUTINGSESSION theSession, 
										   WONRoutingClientId theClientId, 
										   WONRoutingGroupId theGroupId, 
										   WONRoutingStatusCallback theCallback, 
										   void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->RemoveFromGroupEx(theClientId,theGroupId,StatusCompletion,CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Dataobjects

void WONRoutingCreateDataObject(HWONROUTINGSESSION theSession, 
								WONRoutingClientOrGroupId theLinkId, 
								const void *theDataType, 
								unsigned short theDataTypeLen, 
								WONRoutingClientOrGroupId theOwnerId, 
								unsigned short theLifespan, 
								const void *theData, 
								unsigned short theDataLen)
{
	WONRoutingCreateDataObjectAndGetResult(theSession,theLinkId,theDataType,theDataTypeLen,theOwnerId,theLifespan,theData,theDataLen,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingDeleteDataObject(HWONROUTINGSESSION theSession, 
								WONRoutingClientOrGroupId theLinkId, 
								const void *theDataType, 
								unsigned short theDataTypeLen)
{
	WONRoutingDeleteDataObjectAndGetResult(theSession, theLinkId,theDataType,theDataTypeLen,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingModifyDataObject(HWONROUTINGSESSION theSession, 
								WONRoutingClientOrGroupId theLinkId, 
								const void *theDataType, 
								unsigned short theDataTypeLen, 
								unsigned short theOffset, 
								BOOL isInsert, 
								const void *theData, 
								unsigned short theDataLen)
{
	WONRoutingModifyDataObjectAndGetResult(theSession,theLinkId,theDataType,theDataTypeLen,theOffset,isInsert,theData,theDataLen,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingReplaceDataObject(HWONROUTINGSESSION theSession, 
								 WONRoutingClientOrGroupId theLinkId, 
								 const void *theDataType, 
								 unsigned short theDataTypeLen, 
								 const void *theData, 
								 unsigned short theDataLen)
{
	WONRoutingReplaceDataObjectAndGetResult(theSession,theLinkId,theDataType,theDataTypeLen,theData,theDataLen,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingRenewDataObject(HWONROUTINGSESSION theSession, 
							   WONRoutingClientOrGroupId theLinkId, 
							   const void *theDataType, 
							   unsigned short theDataTypeLen, 
							   unsigned short theLifespan)
{
	WONRoutingRenewDataObjectAndGetResult(theSession,theLinkId,theDataType,theDataTypeLen,theLifespan,NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingUnsubscribeDataObject(HWONROUTINGSESSION theSession, 
									 WONRoutingClientOrGroupId theLinkId, 
									 const void *theDataType, 
									 unsigned short theDataTypeLen, 
									 BOOL flagExactOrRecursive, 
									 BOOL flagGroupOrMembers)
{
	WONRoutingUnsubscribeDataObjectAndGetResult(theSession,theLinkId,theDataType,theDataTypeLen,flagExactOrRecursive,flagGroupOrMembers,NULL,NULL);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingCreateDataObjectAndGetResult(HWONROUTINGSESSION theSession, 
											WONRoutingClientOrGroupId theLinkId, 
											const void *theDataType, 
											unsigned short theDataTypeLen, 
											WONRoutingClientOrGroupId theOwnerId, 
											unsigned short theLifespan, 
											const void *theData, 
											unsigned short theDataLen,
											WONRoutingStatusCallback theCallback, 
											void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;
	
	aClient->CreateDataObjectEx(theLinkId,
								RawBuffer((unsigned char*)theDataType,theDataTypeLen),
								theOwnerId,
								theLifespan,
								RawBuffer((unsigned char*)theData,theDataLen),
								StatusCompletion,
								CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingDeleteDataObjectAndGetResult(HWONROUTINGSESSION theSession, 
											WONRoutingClientOrGroupId theLinkId, 
											const void *theDataType, 
											unsigned short theDataTypeLen,
											WONRoutingStatusCallback theCallback, 
											void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->DeleteDataObjectEx(theLinkId,
								RawBuffer((unsigned char*)theDataType,theDataTypeLen),
								StatusCompletion,
								CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingModifyDataObjectAndGetResult(HWONROUTINGSESSION theSession, 
											WONRoutingClientOrGroupId theLinkId, 
											const void *theDataType, 
											unsigned short theDataTypeLen, 
											unsigned short theOffset, 
											BOOL isInsert, 
											const void *theData, 
											unsigned short theDataLen,
											WONRoutingStatusCallback theCallback,
											void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->ModifyDataObjectEx(theLinkId,
								RawBuffer((unsigned char*)theDataType,theDataTypeLen),
								theOffset,
								isInsert?true:false,
								RawBuffer((unsigned char*)theData,theDataLen),
								StatusCompletion,
								CallbackStruct((void*)theCallback,theUserParam));
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingReplaceDataObjectAndGetResult(HWONROUTINGSESSION theSession, 
											 WONRoutingClientOrGroupId theLinkId, 
											 const void *theDataType, 
											 unsigned short theDataTypeLen, 
											 const void *theData, 
											 unsigned short theDataLen,
											 WONRoutingStatusCallback theCallback, 
											 void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->ReplaceDataObjectEx(theLinkId,
								 RawBuffer((unsigned char*)theDataType,theDataTypeLen),
								 RawBuffer((unsigned char*)theData, theDataLen),
								 StatusCompletion,
								 CallbackStruct((void*)theCallback,theUserParam));

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingRenewDataObjectAndGetResult(HWONROUTINGSESSION theSession, 
										   WONRoutingClientOrGroupId theLinkId, 
										   const void *theDataType, 
										   unsigned short theDataTypeLen, 
										   unsigned short theLifespan,
										   WONRoutingStatusCallback theCallback, 
										   void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->RenewDataObjectEx(theLinkId,
							   RawBuffer((unsigned char*)theDataType,theDataTypeLen),
							   theLifespan,
							   StatusCompletion,
							   CallbackStruct((void*)theCallback,theUserParam));
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingUnsubscribeDataObjectAndGetResult(HWONROUTINGSESSION theSession, 
												 WONRoutingClientOrGroupId theLinkId, 
												 const void *theDataType, 
												 unsigned short theDataTypeLen, 
												 BOOL flagExactOrRecursive, 
												 BOOL flagGroupOrMembers,
												 WONRoutingStatusCallback theCallback, 
												 void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->UnsubscribeDataObjectEx(theLinkId,
									RawBuffer((unsigned char*)theDataType,theDataTypeLen),
									flagExactOrRecursive?true:false,
									flagGroupOrMembers?true:false,
									StatusCompletion,
									CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingReadDataObject(HWONROUTINGSESSION theSession, 
							  WONRoutingClientOrGroupId theLinkId, 
							  const void *theDataType, 
							  unsigned short theDataTypeLen, 
							  BOOL flagExactOrRecursive, 
							  BOOL flagGroupOrMembers, 
							  WONRoutingReadDataObjectCallback theCallback,
							  void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->ReadDataObjectEx(theLinkId,
							  RawBuffer((unsigned char*)theDataType,theDataTypeLen),
							  flagExactOrRecursive?true:false,
							  flagGroupOrMembers?true:false,
							  ReadDataObjectCompletion,
							  CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingSubscribeDataObject(HWONROUTINGSESSION theSession, 
								   WONRoutingClientOrGroupId theLinkId, 
								   const void *theDataType, 
								   unsigned short theDataTypeLen, 
								   BOOL flagExactOrRecursive, 
								   BOOL flagGroupOrMembers, 
								   WONRoutingReadDataObjectCallback theCallback, 
								   void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->SubscribeDataObjectEx(theLinkId,
							  RawBuffer((unsigned char*)theDataType,theDataTypeLen),
							  flagExactOrRecursive?true:false,
							  flagGroupOrMembers?true:false,
							  ReadDataObjectCompletion,
							  CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// User Queries
void WONRoutingIsUserPresentA(HWONROUTINGSESSION theSession, 
							  const char* theUserName, 
							  BOOL caseSensitive, 
							  BOOL shouldSendNACK, 
							  WONRoutingBoolCallback theCallback, 
							  void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->IsUserPresentEx(StringToWString(theUserName),
							 caseSensitive?true:false,
							 shouldSendNACK?true:false,
							 0,
							 BoolCompletion,
							 CallbackStruct((void*)theCallback,theUserParam));
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WONRoutingIsUserPresentW(HWONROUTINGSESSION 
							  theSession, 
							  const wchar_t* theUserName, 
							  BOOL caseSensitive, 
							  BOOL shouldSendNACK, 
							  WONRoutingBoolCallback theCallback, 
							  void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->IsUserPresentEx(theUserName,
							 caseSensitive?true:false,
							 shouldSendNACK?true:false,
							 0,
							 BoolCompletion,
							 CallbackStruct((void*)theCallback,theUserParam));

}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingGetNumUsers(HWONROUTINGSESSION theSession, 
						   WONRoutingGetNumUsersCallback theCallback, 
						   void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->GetNumUsersEx(0,GetNumUsersCompletion,CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Unsolicited Data Handlers
void WONRoutingInstallClientJoinAttemptCallback(HWONROUTINGSESSION theSession, 
												WONRoutingClientNameCallback theCallback, 
												void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->InstallClientJoinAttemptCatcherEx(ClientNameCompletion, CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingInstallClientEnterCallback(HWONROUTINGSESSION theSession, 
										  WONRoutingClientIdWithReasonCallback theCallback, 
										  void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->InstallClientEnterCatcherEx(ClientIdWithReasonCompletion, CallbackStruct((void*)theCallback,theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingInstallClientEnterExCallback(HWONROUTINGSESSION theSession, 
											WONRoutingClientDataWithReasonCallback theCallback, 
											void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->InstallClientEnterExCatcherEx(ClientDataWithReasonCompletion, CallbackStruct((void*)theCallback, theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingInstallClientLeaveCallback(HWONROUTINGSESSION theSession, 
										  WONRoutingClientIdWithReasonCallback theCallback, 
										  void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->InstallClientLeaveCatcherEx(ClientIdWithReasonCompletion, CallbackStruct((void*)theCallback, theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingInstallSpectatorCountCallback(HWONROUTINGSESSION theSession, 
											 WONRoutingSpectatorCallback theCallback, 
											 void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->InstallSpectatorCountCatcherEx(SpectatorCountCompletion, CallbackStruct((void*)theCallback, theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingInstallGroupEnterCallback(HWONROUTINGSESSION theSession, 
										 WONRoutingGroupIdAndClientIdWithReasonCallback theCallback, 
										 void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->InstallGroupEnterCatcherEx(GroupIdAndClientIdWithReasonCompletion, CallbackStruct((void*)theCallback, theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingInstallGroupEnterExCallback(HWONROUTINGSESSION theSession, 
										   WONRoutingGroupIdAndClientDataWithReasonCallback theCallback, 
										   void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->InstallGroupEnterExCatcherEx(GroupIdAndClientDataWithReasonCompletion, CallbackStruct((void*)theCallback, theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingInstallGroupLeaveCallback(HWONROUTINGSESSION theSession, 
										 WONRoutingGroupIdAndClientIdWithReasonCallback theCallback, 
										 void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->InstallGroupLeaveCatcherEx(GroupIdAndClientIdWithReasonCompletion, CallbackStruct((void*)theCallback, theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingInstallGroupSpectatorCountCallback(HWONROUTINGSESSION theSession, 
												  WONRoutingGroupIdAndSpectatorCountCallback theCallback, 
												  void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->InstallGroupSpectatorCountCatcherEx(GroupIdAndSpectatorCountCompletion, CallbackStruct((void*)theCallback, theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingInstallMuteClientCallback(HWONROUTINGSESSION theSession, 
										 WONRoutingClientIdWithFlagCallback theCallback, 
										 void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->InstallMuteClientCatcherEx(ClientIdWithFlagCompletion, CallbackStruct((void*)theCallback, theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingInstallBecomeModeratorCallback(HWONROUTINGSESSION theSession, 
											  WONRoutingClientIdWithFlagCallback theCallback, 
											  void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->InstallBecomeModeratorCatcherEx(ClientIdWithFlagCompletion, CallbackStruct((void*)theCallback, theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingInstallHostChangeCallback(HWONROUTINGSESSION theSession, 
										 WONRoutingClientIdCallback theCallback, 
										 void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->InstallHostChangeCatcherEx(ClientIdCompletion, CallbackStruct((void*)theCallback, theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingInstallDataObjectCreationCallback(HWONROUTINGSESSION theSession, 
												 WONRoutingDataObjectWithLifespanCallback theCallback, 
												 void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->InstallDataObjectCreationCatcherEx(DataObjectWithLifespanCompletion, CallbackStruct((void*)theCallback, theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingInstallDataObjectDeletionCallback(HWONROUTINGSESSION theSession, 
												 WONRoutingDataObjectCallback theCallback,
												 void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->InstallDataObjectDeletionCatcherEx(DataObjectCompletion, CallbackStruct((void*)theCallback, theUserParam));
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingInstallDataObjectModificationCallback(HWONROUTINGSESSION theSession, 
													 WONRoutingDataObjectModificationCallback theCallback, 
													 void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->InstallDataObjectModificationCatcherEx(DataObjectModificationCompletion, CallbackStruct((void*)theCallback, theUserParam));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingInstallDataObjectReplacementCallback(HWONROUTINGSESSION theSession, 
													WONRoutingDataObjectCallback theCallback, 
													void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->InstallDataObjectReplacementCatcherEx(DataObjectCompletion, CallbackStruct((void*)theCallback, theUserParam));
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingInstallKeepAliveCallback(HWONROUTINGSESSION theSession, 
										WONRoutingVoidCallback theCallback, 
										void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->InstallKeepAliveCatcherEx(VoidCompletion, CallbackStruct((void*)theCallback, theUserParam));	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingInstallPeerDataCallback(HWONROUTINGSESSION theSession, 
									   WONRoutingMessageWithClientIdCallback theCallback, 
									   void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->InstallPeerDataCatcherEx(MessageWithClientIdCompletion, CallbackStruct((void*)theCallback, theUserParam));	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingInstallRawPeerChatCallback(HWONROUTINGSESSION theSession, 
										  WONRoutingRawChatMessageCallback theCallback, 
										  void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->InstallRawPeerChatCatcherEx(RawChatMessageCompletion, CallbackStruct((void*)theCallback, theUserParam));	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingInstallASCIIPeerChatCallback(HWONROUTINGSESSION theSession, 
											WONRoutingASCIIChatMessageCallback theCallback, 
											void *theUserParam)
{	
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->InstallASCIIPeerChatCatcherEx(ASCIIChatMessageCompletion, CallbackStruct((void*)theCallback, theUserParam));	
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingInstallUnicodePeerChatCallback(HWONROUTINGSESSION theSession, 
											  WONRoutingUnicodeChatMessageCallback theCallback, 
											  void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->InstallUnicodePeerChatCatcherEx(UnicodeChatMessageCompletion, CallbackStruct((void*)theCallback, theUserParam));	

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WONRoutingInstallReconnectFailureCallback(HWONROUTINGSESSION theSession, 
											   WONRoutingVoidCallback theCallback, 
											   void *theUserParam)
{
	RoutingServerClient *aClient = (RoutingServerClient*)theSession;

	aClient->InstallReconnectFailureCatcherEx(VoidCompletion, CallbackStruct((void*)theCallback, theUserParam));	
}

///////////////////////////////////////////////////////////////////////////////
// Copy routines for structs
void CopyWONRoutingMessage(WON_CONST WONRoutingMessage* theSrcP, WONRoutingMessage* theDestP)
{
	theDestP->mDataLen = theSrcP->mDataLen;
	theDestP->mData = malloc(theSrcP->mDataLen);
	memcpy(theDestP->mData, theSrcP->mData, theSrcP->mDataLen);
}

void CopyWONRoutingRegisterClientResult(WON_CONST WONRoutingRegisterClientResult* theSrcP, WONRoutingRegisterClientResult* theDestP)
{
	theDestP->mStatus = theSrcP->mStatus;
	theDestP->mClientId = theSrcP->mClientId;
	theDestP->mHostNameW = wcsdup(theSrcP->mHostNameW);
	theDestP->mHostCommentW = wcsdup(theSrcP->mHostCommentW);
	theDestP->mHostNameA = strdup(theSrcP->mHostNameA);
	theDestP->mHostCommentA = strdup(theSrcP->mHostCommentA);
}

void CopyWONRoutingClientData(WON_CONST WONRoutingClientData* theSrcP, WONRoutingClientData* theDestP)
{
	memcpy(theDestP, theSrcP, sizeof(WONRoutingClientData));
	theDestP->mClientNameW = wcsdup(theSrcP->mClientNameW);
	theDestP->mClientNameA = strdup(theSrcP->mClientNameA);
}

void CopyWONRoutingClientListResult(WON_CONST WONRoutingClientListResult* theSrcP, WONRoutingClientListResult* theDestP)
{
	memcpy(theDestP, theSrcP, sizeof(WONRoutingClientListResult));
	theDestP->mClients = (WONRoutingClientData*)malloc(theSrcP->mNumClients * sizeof(WONRoutingClientData));
	for (int i = 0; i < theSrcP->mNumClients; ++i)
		CopyWONRoutingClientData(&theSrcP->mClients[i], &theDestP->mClients[i]);
}

void CopyWONRoutingClientInfoResult(WON_CONST WONRoutingClientInfoResult* theSrcP, WONRoutingClientInfoResult* theDestP)
{
	memcpy(theDestP, theSrcP, sizeof(WONRoutingClientInfoResult));
	CopyWONRoutingClientData(&theSrcP->mClient, &theDestP->mClient);
}

void CopyWONRoutingGroupData(WON_CONST WONRoutingGroupData* theSrcP, WONRoutingGroupData* theDestP)
{
	memcpy(theDestP, theSrcP, sizeof(WONRoutingGroupData));
	theDestP->mGroupName = strdup(theSrcP->mGroupName);
}

void CopyWONRoutingGroupListResult(WON_CONST WONRoutingGroupListResult* theSrcP, WONRoutingGroupListResult* theDestP)
{
	memcpy(theDestP, theSrcP, sizeof(WONRoutingGroupListResult));
	for (int i = 0; i < theSrcP->mNumGroups; ++i)
		CopyWONRoutingGroupData(&theSrcP->mGroups[i], &theDestP->mGroups[i]);
}

void CopyWONRoutingCreateGroupResult(WON_CONST WONRoutingCreateGroupResult* theSrcP, WONRoutingCreateGroupResult* theDestP)
{
	memcpy(theDestP, theSrcP, sizeof(WONRoutingCreateGroupResult));
}

void CopyWONRoutingDataObject(WON_CONST WONRoutingDataObject* theSrcP, WONRoutingDataObject* theDestP)
{
	memcpy(theDestP, theSrcP, sizeof(WONRoutingDataObject));
	theDestP->mData = (const char*)malloc(theSrcP->mDataLen);
	memcpy((void*)theDestP->mData, theSrcP->mData, theSrcP->mDataLen);
	theDestP->mDataType = (const char*)malloc(theSrcP->mDataTypeLen);
	memcpy((void*)theDestP->mDataType, theSrcP->mDataType, theSrcP->mDataTypeLen);
}

void CopyWONRoutingDataObjectWithLifespan(WON_CONST WONRoutingDataObjectWithLifespan* theSrcP, WONRoutingDataObjectWithLifespan* theDestP)
{
	CopyWONRoutingDataObject(&theSrcP->mObject, &theDestP->mObject);
	theDestP->mLifespan = theSrcP->mLifespan;
}

void CopyWONRoutingDataObjectModification(WON_CONST WONRoutingDataObjectModification* theSrcP, WONRoutingDataObjectModification* theDestP)
{
	CopyWONRoutingDataObject(&theSrcP->mObject, &theDestP->mObject);
	theDestP->mIsInsert = theSrcP->mIsInsert;
	theDestP->mOffset = theSrcP->mOffset;
}

void CopyWONRoutingReadDataObjectResult(WON_CONST WONRoutingReadDataObjectResult* theSrcP, WONRoutingReadDataObjectResult* theDestP)
{
	memcpy(theDestP, theSrcP, sizeof(WONRoutingReadDataObjectResult));
	for (int i = 0; i < theSrcP->mNumObjects; ++i)
		CopyWONRoutingDataObject(&theSrcP->mObjects[i], &theDestP->mObjects[i]);
}

void CopyWONRoutingGetNumUsersResult(WON_CONST WONRoutingGetNumUsersResult* theSrcP, WONRoutingGetNumUsersResult* theDestP)
{
	memcpy(theDestP, theSrcP, sizeof(WONRoutingGetNumUsersResult));
}

void CopyWONRoutingClientNameResult(WON_CONST WONRoutingClientNameResult* theSrcP, WONRoutingClientNameResult* theDestP)
{
	theDestP->mNameA = strdup(theSrcP->mNameA);
	theDestP->mNameW = wcsdup(theSrcP->mNameW);
}

void CopyWONRoutingClientIdWithReason(WON_CONST WONRoutingClientIdWithReason* theSrcP, WONRoutingClientIdWithReason* theDestP)
{
	memcpy(theDestP, theSrcP, sizeof(WONRoutingClientIdWithReason));
}

void CopyWONRoutingGroupIdAndClientIdWithReason(WON_CONST WONRoutingGroupIdAndClientIdWithReason* theSrcP, WONRoutingGroupIdAndClientIdWithReason* theDestP)
{
	memcpy(theDestP, theSrcP, sizeof(WONRoutingGroupIdAndClientIdWithReason));
}

void CopyWONRoutingClientDataWithReason(WON_CONST WONRoutingClientDataWithReason* theSrcP, WONRoutingClientDataWithReason* theDestP)
{
	theDestP->mReason = theSrcP->mReason;
	CopyWONRoutingClientData(&theSrcP->mData, &theDestP->mData);
}

void CopyWONRoutingGroupIdAndClientDataWithReason(WON_CONST WONRoutingGroupIdAndClientDataWithReason* theSrcP, WONRoutingGroupIdAndClientDataWithReason* theDestP)
{
	memcpy(theDestP, theSrcP, sizeof(WONRoutingGroupIdAndClientDataWithReason));
	CopyWONRoutingClientData(&theSrcP->mData, &theDestP->mData);
}

void CopyWONRoutingGroupIdAndSpectatorCount(WON_CONST WONRoutingGroupIdAndSpectatorCount* theSrcP, WONRoutingGroupIdAndSpectatorCount* theDestP)
{
	memcpy(theDestP, theSrcP, sizeof(WONRoutingGroupIdAndSpectatorCount));
}

void CopyWONRoutingClientIdWithFlag(WON_CONST WONRoutingClientIdWithFlag* theSrcP, WONRoutingClientIdWithFlag* theDestP)
{
	memcpy(theDestP, theSrcP, sizeof(WONRoutingClientIdWithFlag));
}

void CopyWONRoutingMessageWithClientId(WON_CONST WONRoutingMessageWithClientId* theSrcP, WONRoutingMessageWithClientId* theDestP)
{
	memcpy(theDestP, theSrcP, sizeof(WONRoutingMessageWithClientId));
	theDestP->mData = (const char*)malloc(theSrcP->mDataLen);
	memcpy((void*)theDestP->mData, theSrcP->mData, theSrcP->mDataLen);
}

void CopyWONRoutingChatMessageBase(WON_CONST WONRoutingChatMessageBase* theSrcP, WONRoutingChatMessageBase* theDestP)
{
	memcpy(theDestP, theSrcP, sizeof(WONRoutingChatMessageBase));
	theDestP->mRecipientIds = (const unsigned short*)malloc(theSrcP->mNumRecipientIds * sizeof(WONRoutingClientOrGroupId));
	memcpy((void*)theDestP->mRecipientIds, theSrcP->mRecipientIds, theSrcP->mNumRecipientIds * sizeof(WONRoutingClientOrGroupId));
}

void CopyWONRoutingRawChatMessage(WON_CONST WONRoutingRawChatMessage* theSrcP, WONRoutingRawChatMessage* theDestP)
{
	CopyWONRoutingChatMessageBase(&theSrcP->mBaseData, &theDestP->mBaseData);
	theDestP->mDataLen = theSrcP->mDataLen;
	theDestP->mData = (const char*)malloc(theSrcP->mDataLen);
	memcpy((void*)theDestP->mData, theSrcP->mData, theSrcP->mDataLen);
}

void CopyWONRoutingASCIIChatMessage(WON_CONST WONRoutingASCIIChatMessage* theSrcP, WONRoutingASCIIChatMessage* theDestP)
{
	CopyWONRoutingChatMessageBase(&theSrcP->mBaseData, &theDestP->mBaseData);
	theDestP->mChat = strdup(theSrcP->mChat);
}

void CopyWONRoutingUnicodeChatMessage(WON_CONST WONRoutingUnicodeChatMessage* theSrcP, WONRoutingUnicodeChatMessage* theDestP)
{
	CopyWONRoutingChatMessageBase(&theSrcP->mBaseData, &theDestP->mBaseData);
	theDestP->mChat = wcsdup(theSrcP->mChat);
}

///////////////////////////////////////////////////////////////////////////////
// Delete routines for structs
void DeleteWONRoutingMessage(WON_CONST WONRoutingMessage* theSrcP)
{
	free(theSrcP->mData);
}

void DeleteWONRoutingRegisterClientResult(WON_CONST WONRoutingRegisterClientResult* theSrcP)
{
	free(const_cast<wchar_t*>(theSrcP->mHostNameW));
	free(const_cast<wchar_t*>(theSrcP->mHostCommentW));
	free(const_cast<char*>(theSrcP->mHostNameA));
	free(const_cast<char*>(theSrcP->mHostCommentA));
}

void DeleteWONRoutingClientData(WON_CONST WONRoutingClientData* theSrcP)
{
	free(const_cast<wchar_t*>(theSrcP->mClientNameW));
	free(const_cast<char*>(theSrcP->mClientNameA));
}

void DeleteWONRoutingClientListResult(WON_CONST WONRoutingClientListResult* theSrcP)
{
	for (int i = 0; i < theSrcP->mNumClients; ++i)
		DeleteWONRoutingClientData(&theSrcP->mClients[i]);
	free(theSrcP->mClients);
}

void DeleteWONRoutingClientInfoResult(WON_CONST WONRoutingClientInfoResult* theSrcP)
{
	DeleteWONRoutingClientData(&theSrcP->mClient);
}

void DeleteWONRoutingGroupData(WON_CONST WONRoutingGroupData* theSrcP)
{
	free(const_cast<char*>(theSrcP->mGroupName));
}

void DeleteWONRoutingGroupListResult(WON_CONST WONRoutingGroupListResult* theSrcP)
{
	for (int i = 0; i < theSrcP->mNumGroups; ++i)
		DeleteWONRoutingGroupData(&theSrcP->mGroups[i]);
}

void DeleteWONRoutingCreateGroupResult(WON_CONST WONRoutingCreateGroupResult* theSrcP)
{
}

void DeleteWONRoutingDataObject(WON_CONST WONRoutingDataObject* theSrcP)
{
	free(const_cast<char*>(theSrcP->mData));
	free(const_cast<char*>(theSrcP->mDataType));
}

void DeleteWONRoutingDataObjectWithLifespan(WON_CONST WONRoutingDataObjectWithLifespan* theSrcP)
{
	DeleteWONRoutingDataObject(&theSrcP->mObject);
}

void DeleteWONRoutingDataObjectModification(WON_CONST WONRoutingDataObjectModification* theSrcP)
{
	DeleteWONRoutingDataObject(&theSrcP->mObject);
}

void DeleteWONRoutingReadDataObjectResult(WON_CONST WONRoutingReadDataObjectResult* theSrcP)
{
	for (int i = 0; i < theSrcP->mNumObjects; ++i)
		DeleteWONRoutingDataObject(&theSrcP->mObjects[i]);
}

void DeleteWONRoutingGetNumUsersResult(WON_CONST WONRoutingGetNumUsersResult* theSrcP)
{
}

void DeleteWONRoutingClientNameResult(WON_CONST WONRoutingClientNameResult* theSrcP)
{
	free(const_cast<char*>(theSrcP->mNameA));
	free(const_cast<wchar_t*>(theSrcP->mNameW));
}

void DeleteWONRoutingClientIdWithReason(WON_CONST WONRoutingClientIdWithReason* theSrcP)
{
}

void DeleteWONRoutingGroupIdAndClientIdWithReason(WON_CONST WONRoutingGroupIdAndClientIdWithReason* theSrcP)
{
}

void DeleteWONRoutingClientDataWithReason(WON_CONST WONRoutingClientDataWithReason* theSrcP)
{
	DeleteWONRoutingClientData(&theSrcP->mData);
}

void DeleteWONRoutingGroupIdAndClientDataWithReason(WON_CONST WONRoutingGroupIdAndClientDataWithReason* theSrcP)
{
	DeleteWONRoutingClientData(&theSrcP->mData);
}

void DeleteWONRoutingGroupIdAndSpectatorCount(WON_CONST WONRoutingGroupIdAndSpectatorCount* theSrcP)
{
}

void DeleteWONRoutingClientIdWithFlag(WON_CONST WONRoutingClientIdWithFlag* theSrcP)
{
}

void DeleteWONRoutingMessageWithClientId(WON_CONST WONRoutingMessageWithClientId* theSrcP)
{
	free(const_cast<char*>(theSrcP->mData));
}

void DeleteWONRoutingChatMessageBase(WON_CONST WONRoutingChatMessageBase* theSrcP)
{
	free(const_cast<WONRoutingClientOrGroupId*>(theSrcP->mRecipientIds));
}

void DeleteWONRoutingRawChatMessage(WON_CONST WONRoutingRawChatMessage* theSrcP)
{
	DeleteWONRoutingChatMessageBase(&theSrcP->mBaseData);
	free(const_cast<char*>(theSrcP->mData));
}

void DeleteWONRoutingASCIIChatMessage(WON_CONST WONRoutingASCIIChatMessage* theSrcP)
{
	DeleteWONRoutingChatMessageBase(&theSrcP->mBaseData);
	free(const_cast<char*>(theSrcP->mChat));
}

void DeleteWONRoutingUnicodeChatMessage(WON_CONST WONRoutingUnicodeChatMessage* theSrcP)
{
	DeleteWONRoutingChatMessageBase(&theSrcP->mBaseData);
	free(const_cast<wchar_t*>(theSrcP->mChat));
}
