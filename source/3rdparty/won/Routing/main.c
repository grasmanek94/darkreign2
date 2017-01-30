#include <stdio.h>
#include <conio.h>

#include "Routing/CRoutingAPI.h"

//#include <windows.h>

HWONROUTINGSESSION mSession;
HWONAUTH mAuth;
HANDLE mEvent;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void PrintRaw(const char *theData, int theDataLen)
{
	int i;

	printf("[");
	for(i=0; i<theDataLen; i++)
		printf("%x ",theData[i]);

	printf("] ");
	for(i=0; i<theDataLen; i++)
	{
		if(isprint(theData[i]))
			printf("%c",theData[i]);
		else
			printf(".");
	}

	printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void PrintClientData(WONRoutingClientData *theData)
{
	printf("ClientId: %d\n",theData->mClientId);
	printf("ClientNameA: %s\n",theData->mClientNameA);
	wprintf(L"ClientNameW: %s\n",theData->mClientNameW);
	printf("IPAddress: %d\n",theData->mIPAddress);
	printf("WONUserId: %d\n",theData->mWONUserId);
	printf("CommunityId: %d\n",theData->mCommunityId);
	printf("TrustLevet: %d\n",theData->mTrustLevel);
	printf("IsModerator: %d\n",theData->mIsModerator);
	printf("IsMuted: %d\n",theData->mIsMuted);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void PrintDataObject(WONRoutingDataObject *theObject)
{
	printf("OwnerId: %d\n",theObject->mOwnerId);
	printf("LinkId: %d\n",theObject->mLinkId);
	printf("DataType: "); PrintRaw(theObject->mDataType,theObject->mDataTypeLen);
	printf("Data: "); PrintRaw(theObject->mData,theObject->mDataLen);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void PrintChatMessageBase(WONRoutingChatMessageBase *theMsg)
{
	int i;

	printf("SenderId: %d\n",theMsg->mSenderId);
	printf("ChatType: %d\n",theMsg->mChatType);
	printf("IncludeExclude: %d\n",theMsg->mIncludeExcludeFlag);
	printf("Recipients: \n");

	for(i=0; i<theMsg->mNumRecipientIds; i++)
		printf("RecipientId: %d\n",theMsg->mRecipientIds[i]);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingVoidCallback(void *theUserParam)
{
	printf("Void Callback: %s\n",(char*)theUserParam);
	SetEvent(mEvent); printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingErrorCallback(WONError theError, void *theUserParam)
{
	printf("Error Callback: %s \n",(char*)theUserParam);
	printf("Error = %s\n",WONErrorToString(theError));
	SetEvent(mEvent); printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingStatusCallback(short theStatus, void *theUserParam)
{
	printf("Status Callback: %s\n",(char*)theUserParam);
	printf("Status = %d\n",theStatus);
	SetEvent(mEvent); printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingBoolCallback(BOOL theBool, void *theUserParam)
{
	printf("Bool Callback: %s\n",(char*)theUserParam);
	printf("Bool = %s\n",theBool?"True":"False");
	SetEvent(mEvent); printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingSpectatorCallback(unsigned short theCount, void *theUserParam)
{
	printf("Spectator Callback: %s\n",(char*)theUserParam);
	printf("Count = %d\n",theCount);
	SetEvent(mEvent); printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingClientIdCallback(WONRoutingClientId theId, void *theUserParam)
{
	printf("ClientId Callback: %s\n",(char*)theUserParam);
	printf("Id: %d\n",theId);
	SetEvent(mEvent); printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingRegisterClientCallback(WONRoutingRegisterClientResult *theResult, void *theUserParam)
{
	printf("RegisterClient Callback: %s\n",(char*)theUserParam);
	printf("Status: %d\n",theResult->mStatus);
	printf("ClientId: %d\n",theResult->mClientId);
	printf("HostNameA: %s\n",theResult->mHostNameA);
	printf("HostCommentA: %s\n",theResult->mHostCommentA);
	wprintf(L"HostNameW: %s\n",theResult->mHostNameW);
	wprintf(L"HostCommentW: %s\n",theResult->mHostCommentW);

	if(theResult->mStatus!=0)
		exit(0);

	SetEvent(mEvent); printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingClientListCallback(WONRoutingClientListResult *theList, void *theUserParam)
{
	int i;

	printf("ClientList Callback: %s\n",(char*)theUserParam);

	printf("Status: %d\n",theList->mStatus);
	printf("IncludesIPs: %d\n",theList->mIncludesIPs);
	printf("IncludesAuthInfo: %d\n", theList->mIncludesAuthInfo);
	printf("IncludesMuteFlag: %d\n", theList->mIncludesMuteFlag);
	printf("IncludesModeratorFlag: %d\n",theList->mIncludesModeratorFlag);
	printf("NumSpectators: %d\n",theList->mNumSpectators);
	printf("Clients:\n\n");

	for(i=0; i<theList->mNumClients; i++)
	{
		PrintClientData(&theList->mClients[i]);
		printf("\n");
	}

	SetEvent(mEvent); printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingClientInfoCallback(WONRoutingClientInfoResult *theInfo, void *theUserParam)
{
	printf("ClientInfo Callback: %s\n",(char*)theUserParam);

	printf("Status: %d\n",theInfo->mStatus);
	printf("IncludesIPs: %d\n",theInfo->mIncludesIPs);
	printf("IncludesAuthInfo: %d\n", theInfo->mIncludesAuthInfo);
	printf("IncludesMuteFlag: %d\n", theInfo->mIncludesMuteFlag);
	printf("IncludesModeratorFlag: %d\n",theInfo->mIncludesModeratorFlag);

	PrintClientData(&theInfo->mClient);

	SetEvent(mEvent); printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingGroupListCallback(WONRoutingGroupListResult *theList, void *theUserParam)
{
	int i;

	printf("GroupList Callback: %s\n",(char*)theUserParam);

	printf("Status: %d\n",theList->mStatus);
	printf("Groups:\n\n");

	for(i=0; i<theList->mNumGroups; i++)
	{
		WONRoutingGroupData *aGroup = &theList->mGroups[i];
		printf("GroupId: %d\n",aGroup->mGroupId);
		printf("GroupName: %s\n",aGroup->mGroupName);
		printf("IsPublic: %d\n",aGroup->mIsPublic);
		printf("\n");
	}

	SetEvent(mEvent); printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingCreateGroupCallback(WONRoutingCreateGroupResult *theResult, void *theUserParam)
{
	printf("CreateGroup Callback: %s\n",(char*)theUserParam);

	printf("Status: %d\n",theResult->mStatus);
	printf("GroupId: %d\n",theResult->mGroupId);
	SetEvent(mEvent); printf("\n");
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingReadDataObjectCallback(WONRoutingReadDataObjectResult *theObjects, void *theUserParam)
{
	unsigned int i;

	printf("ReadDataObject Callback: %s\n",(char*)theUserParam);
	printf("Status: %d\n",theObjects->mStatus);
	printf("Objects:\n\n");
	for(i=0; i<theObjects->mNumObjects; i++)
	{
		PrintDataObject(&theObjects->mObjects[i]);
		printf("\n");
	}
	SetEvent(mEvent); printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingGetNumUsersCallback(WONRoutingGetNumUsersResult *theResult, void *theUserParam)
{
	printf("GetNumUsers Callback: %s\n",(char*)theUserParam);
	printf("Cur Users: %d\n",theResult->mNumActiveUsers);
	printf("Max Users: %d\n",theResult->mServerCapacity);
	SetEvent(mEvent); printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingClientNameCallback(WONRoutingClientNameResult *theResult, void *theUserParam)
{
	printf("ClientName Callback: %s\n",(char*)theUserParam);
	printf("ClientNameA: ",theResult->mNameA);
	wprintf(L"ClientNameW: ",theResult->mNameW);
	SetEvent(mEvent); printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingClientIdWithReasonCallback(WONRoutingClientIdWithReason *theReason, void *theUserParam)
{
	printf("ClientIdWithReason Callback: %s\n",(char*)theUserParam);
	printf("ClientId: %d\n",theReason->mClientId);
	printf("Reason: %d\n",theReason->mReason);
	SetEvent(mEvent); printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingGroupIdAndClientIdWithReasonCallback(WONRoutingGroupIdAndClientIdWithReason *theReason, void *theUserParam)
{
	printf("GroupIdAndClientIdWithReason Callback: %s\n",(char*)theUserParam);
	printf("GroupId: %d\n",theReason->mGroupId);
	printf("ClientId: %d\n",theReason->mClientId);
	printf("Reason: %d\n",theReason->mReason);
	SetEvent(mEvent); printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingClientDataWithReasonCallback(WONRoutingClientDataWithReason *theData, void *theUserParam)
{
	printf("ClientDataWithReason Callback: %s\n",(char*)theUserParam);
	printf("Reason: %d",theData->mReason);
	PrintClientData(&theData->mData);
	SetEvent(mEvent); printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingGroupIdAndClientDataWithReasonCallback(WONRoutingGroupIdAndClientDataWithReason *theData, void *theUserParam)
{
	printf("GroupIdAndClientDataWithReason Callback: %s\n",(char*)theUserParam);
	printf("Reason: %d\n",theData->mReason);
	printf("GroupId: %d\n",theData->mGroupId);
	PrintClientData(&theData->mData);
	SetEvent(mEvent); printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingGroupIdAndSpectatorCountCallback(WONRoutingGroupIdAndSpectatorCount *theData, void *theUserParam)
{
	printf("GroupIdAndSpectatorCount Callback: %s\n",(char*)theUserParam);
	printf("GroupId: %d\n",theData->mGroupId);
	printf("Count: %d\n",theData->mSpectatorCount);
	SetEvent(mEvent); printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingClientIdWithFlagCallback(WONRoutingClientIdWithFlag *theData, void *theUserParam)
{
	printf("ClientIdWithFlag Callback: %s\n",(char*)theUserParam);
	printf("ClientId: %d\n",theData->mClientId);
	printf("Flag: %d\n",theData->mFlagOnOrOff);
	SetEvent(mEvent); printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingDataObjectCallback(WONRoutingDataObject *theData, void *theUserParam)
{
	printf("DataObject Callback: %s\n",(char*)theUserParam);
	PrintDataObject(theData);
	SetEvent(mEvent); printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingDataObjectWithLifespanCallback(WONRoutingDataObjectWithLifespan *theData, void *theUserParam)
{
	printf("DataObjectWithLifespan Callback: %s\n",(char*)theUserParam);
	printf("Lifespan: %d\n",theData->mLifespan);
	PrintDataObject(&theData->mObject);
	SetEvent(mEvent); printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingDataObjectModificationCallback(WONRoutingDataObjectModification *theData, void *theUserParam)
{
	printf("DataObjectModification Callback: %s\n",(char*)theUserParam);
	printf("IsInsert: %d\n",theData->mIsInsert);
	printf("Offset: %d\n",theData->mOffset);
	PrintDataObject(&theData->mObject);
	SetEvent(mEvent); printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingMessageWithClientIdCallback(WONRoutingMessageWithClientId *theMsg, void *theUserParam)
{
	printf("MessageWithClientId Callback: %s\n",(char*)theUserParam);
	printf("ClientId: %d\n",theMsg->mClientId);
	printf("Data: "); PrintRaw(theMsg->mData,theMsg->mDataLen);
	SetEvent(mEvent); printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingRawChatMessageCallback(WONRoutingRawChatMessage *theMsg, void *theUserParam)
{
	printf("RawChatMessage Callback: %s\n",(char*)theUserParam);
	PrintChatMessageBase(&theMsg->mBaseData);
	printf("Msg: "); PrintRaw(theMsg->mData,theMsg->mDataLen);
	SetEvent(mEvent); printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingASCIIChatMessageCallback(WONRoutingASCIIChatMessage *theMsg, void *theUserParam)
{
	printf("ASCIIChatMessage Callback: %s\n",(char*)theUserParam);
	PrintChatMessageBase(&theMsg->mBaseData);
	printf("Chat: %s\n",theMsg->mChat);
	SetEvent(mEvent); printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MyRoutingUnicodeChatMessageCallback(WONRoutingUnicodeChatMessage *theMsg, void *theUserParam)
{
	printf("UnicodeChatMessage Callback: %s\n",(char*)theUserParam);
	PrintChatMessageBase(&theMsg->mBaseData);
	wprintf(L"Chat: %s\n",theMsg->mChat);
	SetEvent(mEvent); printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

BOOL GetCertificate(const char *theName, const char *theCommunity, const char *thePassword)
{
	WONIPAddress anAddr;
	WONError anError;
	WONIPAddressSetFromString(&anAddr, "master1.west.won.net:15200");


	mAuth = WONAuthLoginA(&anError, &anAddr, 1, theName, theCommunity, thePassword, "", 10000);
	if(mAuth==NULL)
	{
		printf("Error authenticating: %s\n",WONErrorToString(anError));
		return FALSE;
	}
	else
		return TRUE;
}

void main()
{
	WONIPAddress anAddr;
	WONError anError;

	WONInitialize();
	mEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	WONAuthLoadVerifierKeyFromRegistry("SOFTWARE\\WON\\Titan", "AuthVerifierKey");
	if(!GetCertificate("Brian123","WON","suck"))
		return;

	mSession = WONRoutingCreateSession();

	WONIPAddressSetFromString(&anAddr,"localhost:8200");
	anError = WONRoutingConnect(mSession, &anAddr, mAuth, 10000);
	printf("Connection result: %s\n",WONErrorToString(anError));
	if(anError!=WONError_Success)
		return;

	WONRoutingRegisterA(mSession,"Brian123","",FALSE,FALSE,TRUE,MyRoutingRegisterClientCallback,"RegisterA");
	WaitForSingleObject(mEvent, INFINITE);

	WONRoutingGetNumUsers(mSession,MyRoutingGetNumUsersCallback,"GetNumUsers");
	WONRoutingIsUserPresentA(mSession,"test",TRUE,TRUE,MyRoutingBoolCallback,"Good IsUserPresent");
	WONRoutingIsUserPresentA(mSession,"test2",TRUE,TRUE,MyRoutingBoolCallback,"Good IsUserPresent");
	WONRoutingGetClientList(mSession,TRUE,TRUE,TRUE,MyRoutingClientListCallback,"GetClientList");
	WONRoutingGetMembersOfGroup(mSession,4,TRUE,TRUE,TRUE,MyRoutingClientListCallback,"GetChatGroup");
	WONRoutingGetMembersOfGroup(mSession,5,TRUE,TRUE,TRUE,MyRoutingClientListCallback,"GetBadGroup");
	WONRoutingGetGroupList(mSession,MyRoutingGroupListCallback,"GetGroupList");
	WONRoutingSubscribeDataObject(mSession,2,"",0,FALSE,TRUE,MyRoutingReadDataObjectCallback,"SubscribeDataObject");
	WONRoutingBroadcastChatA(mSession,"YO YO YO!!!",FALSE);
	WONRoutingBroadcastChatAndGetResultA(mSession,"YO YO YO!!!",FALSE,MyRoutingStatusCallback, "Send Chat");


	WONRoutingInstallClientJoinAttemptCallback(mSession, MyRoutingClientNameCallback, "Join Attempt");      
	WONRoutingInstallClientEnterCallback(mSession, MyRoutingClientIdWithReasonCallback, "Client Enter");     
	WONRoutingInstallClientEnterExCallback(mSession, MyRoutingClientDataWithReasonCallback, "Client Enter Ex"); 
	WONRoutingInstallClientLeaveCallback(mSession, MyRoutingClientIdWithReasonCallback, "Client Leave");     
	WONRoutingInstallSpectatorCountCallback(mSession, MyRoutingSpectatorCallback, "Spectator Count");                       
	WONRoutingInstallGroupEnterCallback(mSession, MyRoutingGroupIdAndClientIdWithReasonCallback, "Group Enter");      
	WONRoutingInstallGroupEnterExCallback(mSession, MyRoutingGroupIdAndClientDataWithReasonCallback, "Group Enter Ex");  
	WONRoutingInstallGroupLeaveCallback(mSession, MyRoutingGroupIdAndClientIdWithReasonCallback, "Group Leave");      
	WONRoutingInstallGroupSpectatorCountCallback(mSession, MyRoutingGroupIdAndSpectatorCountCallback, "Group Spectator Count"); 
	WONRoutingInstallMuteClientCallback(mSession, MyRoutingClientIdWithFlagCallback, "Mute Client");                   
	WONRoutingInstallBecomeModeratorCallback(mSession, MyRoutingClientIdWithFlagCallback, "Become Moderator");              
	WONRoutingInstallHostChangeCallback(mSession, MyRoutingClientIdCallback, "Host Change");                                 
	WONRoutingInstallDataObjectCreationCallback(mSession, MyRoutingDataObjectWithLifespanCallback, "Data Object Creation");    
	WONRoutingInstallDataObjectDeletionCallback(mSession, MyRoutingDataObjectCallback, "Data Object Deletion");                
	WONRoutingInstallDataObjectModificationCallback(mSession, MyRoutingDataObjectModificationCallback, "Data Object Modification");
	WONRoutingInstallDataObjectReplacementCallback(mSession, MyRoutingDataObjectCallback, "Data Object Replacement");             
	WONRoutingInstallKeepAliveCallback(mSession, MyRoutingVoidCallback, "Keep Alive");                                      
	WONRoutingInstallPeerDataCallback(mSession, MyRoutingMessageWithClientIdCallback, "Peer Data");                 
	WONRoutingInstallRawPeerChatCallback(mSession, MyRoutingRawChatMessageCallback, "Raw Peer Chat");                   
	WONRoutingInstallASCIIPeerChatCallback(mSession, MyRoutingASCIIChatMessageCallback, "ASCII Peer Chat");               
	WONRoutingInstallUnicodePeerChatCallback(mSession, MyRoutingUnicodeChatMessageCallback, "Unicode Peer Chat");           
	WONRoutingInstallReconnectFailureCallback(mSession, MyRoutingVoidCallback, "Reconnect Failure");                               
	

	getch();

	WONRoutingCloseSessionHandle(mSession);
	WONAuthCloseHandle(mAuth);

	WONTerminate();
	
}