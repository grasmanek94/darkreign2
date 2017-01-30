#ifndef _CROUTINGAPI_H_
#define _CROUTINGAPI_H_

#include "CRoutingAPITypes.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct unnamed_HWONROUTINGSESSION { int unused; }* HWONROUTINGSESSION;

///////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
HWONROUTINGSESSION WONRoutingCreateSession();
HWONROUTINGSESSION WONRoutingCreateSessionEx(BOOL useTCP, BOOL enableShortcuts, unsigned char theLengthFieldSize);
void WONRoutingCloseSessionHandle(HWONROUTINGSESSION theSession);		 

///////////////////////////////////////////////////////////////////////////////
// Connecting/Reconnecting/Disconnecting
WONError WONRoutingConnect(HWONROUTINGSESSION theSession, const WONIPAddress* theServerAddress, HWONAUTH hAuth, long theTimeout);
WONError WONRoutingReConnect(HWONROUTINGSESSION theSession, const WONIPAddress* theServerAddress, HWONAUTH hAuth, long theTimeout);

void WONRoutingConnectAsync(HWONROUTINGSESSION theSession, const WONIPAddress* theServerAddress, HWONAUTH hAuth, long theTimeout, WONRoutingErrorCallback theCallback, void *theUserParam);
void WONRoutingReConnectAsync(HWONROUTINGSESSION theSession, const WONIPAddress* theServerAddress, HWONAUTH hAuth, long theTimeout, WONRoutingErrorCallback theCallback, void *theUserParam);

void  WONRoutingCloseConnection(HWONROUTINGSESSION theSession);

///////////////////////////////////////////////////////////////////////////////
// Registration
void WONRoutingRegisterA(HWONROUTINGSESSION theSession, const char* theClientName, const char* thePassword, BOOL becomeSpectator, BOOL becomeHost, BOOL joinChat, WONRoutingRegisterClientCallback theCallback, void *theUserParam);
void WONRoutingRegisterW(HWONROUTINGSESSION theSession, const wchar_t* theClientName, const wchar_t* thePassword, BOOL becomeSpectator, BOOL becomeHost, BOOL joinChat, WONRoutingRegisterClientCallback theCallback, void *theUserParam);
void WONRoutingDeregister(HWONROUTINGSESSION theSession, BOOL isPermanent, WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingReregister(HWONROUTINGSESSION theSession, BOOL getMissedMessages, WONRoutingStatusCallback theCallback, void *theUserParam);
WONRoutingClientId WONRoutingGetClientId(HWONROUTINGSESSION theSession);
BOOL WONRoutingIsRegistered(HWONROUTINGSESSION theSession);

///////////////////////////////////////////////////////////////////////////////
// Client lists
void WONRoutingGetClientList(HWONROUTINGSESSION theSession, BOOL requestAuthInfo,  BOOL requestMuteFlag, BOOL requestModeratorFlag, WONRoutingClientListCallback theCallback, void *theUserParam);
void WONRoutingGetMembersOfGroup(HWONROUTINGSESSION theSession, WONRoutingGroupId theGroupId, BOOL requestAuthInfo, BOOL requestMuteFlag, BOOL requestModeratorFlag, WONRoutingClientListCallback theCallback, void *theUserParam);
void WONRoutingGetClientInfo(HWONROUTINGSESSION theSession, WONRoutingClientId theClientId, BOOL requestAuthInfo, BOOL requestMuteFlag, BOOL requestModeratorFlag, WONRoutingClientInfoCallback theCallback, void *theUserParam);
void WONRoutingGetGroupList(HWONROUTINGSESSION theSession, WONRoutingGroupListCallback theCallback, void *theUserParam);

///////////////////////////////////////////////////////////////////////////////
// Send raw data
void WONRoutingBroadcastData(HWONROUTINGSESSION theSession, const void *theData, unsigned short theDataLen);
void WONRoutingSendData(HWONROUTINGSESSION theSession, const WONRoutingClientOrGroupId theRecipients[], unsigned short theNumRecipients, BOOL flagIncludeExclude, const void *theData, unsigned short theDataLen);
void WONRoutingSendDataMultiple(HWONROUTINGSESSION theSession, const WONRoutingClientOrGroupId theRecipients[], unsigned short theNumRecipients, BOOL flagIncludeExclude, const WONRoutingMessage theMessages[], unsigned short theNumMessages);

void WONRoutingBroadcastDataAndGetResult(HWONROUTINGSESSION theSession, const void *theData, unsigned short theDataLen, WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingSendDataAndGetResult(HWONROUTINGSESSION theSession, const WONRoutingClientOrGroupId theRecipients[], unsigned short theNumRecipients, BOOL flagIncludeExclude, const void *theData, unsigned short theDataLen, WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingSendDataMultipleAndGetResult(HWONROUTINGSESSION theSession, const WONRoutingClientOrGroupId theRecipients[], unsigned short theNumRecipients, BOOL flagIncludeExclude, const WONRoutingMessage theMessages[], unsigned short theNumMessages, WONRoutingStatusCallback theCallback, void *theUserParam);

///////////////////////////////////////////////////////////////////////////////
// Send chat
void WONRoutingBroadcastChatA(HWONROUTINGSESSION theSession, const char *theChat, BOOL emote);
void WONRoutingBroadcastChatW(HWONROUTINGSESSION theSession, const wchar_t *theChat, BOOL emote);
void WONRoutingSendChatA(HWONROUTINGSESSION theSession, const WONRoutingClientOrGroupId theRecipients[], unsigned short theNumRecipients, BOOL flagIncludeExclude, const char *theChat);
void WONRoutingSendChatW(HWONROUTINGSESSION theSession, const WONRoutingClientOrGroupId theRecipients[], unsigned short theNumRecipients, BOOL flagIncludeExclude, const wchar_t *theChat);

void WONRoutingBroadcastChatAndGetResultA(HWONROUTINGSESSION theSession, const char *theChat, BOOL emote,WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingBroadcastChatAndGetResultW(HWONROUTINGSESSION theSession, const wchar_t *theChat, BOOL emote,WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingSendChatAndGetResultA(HWONROUTINGSESSION theSession, const WONRoutingClientOrGroupId theRecipients[], unsigned short theNumRecipients, BOOL flagIncludeExclude, const char *theChat,WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingSendChatAndGetResultW(HWONROUTINGSESSION theSession, const WONRoutingClientOrGroupId theRecipients[], unsigned short theNumRecipients, BOOL flagIncludeExclude, const wchar_t *theChat,WONRoutingStatusCallback theCallback, void *theUserParam);

///////////////////////////////////////////////////////////////////////////////
// Ignoring
void WONRoutingIgnoreClientA(HWONROUTINGSESSION theSession, const char* theClientName);
void WONRoutingIgnoreClientW(HWONROUTINGSESSION theSession, const wchar_t* theClientName);
void WONRoutingUnignoreClientA(HWONROUTINGSESSION theSession, const char* theClientName);
void WONRoutingUnignoreClientW(HWONROUTINGSESSION theSession, const wchar_t* theClientName);

///////////////////////////////////////////////////////////////////////////////
// Administrative
void WONRoutingOpenRegistration(HWONROUTINGSESSION theSession);
void WONRoutingCloseRegistration(HWONROUTINGSESSION theSession);
void WONRoutingSetPasswordA(HWONROUTINGSESSION theSession, const char *thePassword);
void WONRoutingSetPasswordW(HWONROUTINGSESSION theSession, const wchar_t *thePassword);
void WONRoutingBecomeModerator(HWONROUTINGSESSION theSession, BOOL flagOnOrOff); 
void WONRoutingBootClient(HWONROUTINGSESSION theSession, WONRoutingClientId theClientId);
void WONRoutingBanClientA(HWONROUTINGSESSION theSession, const char *theClientName);
void WONRoutingBanClientW(HWONROUTINGSESSION theSession, const wchar_t *theClientName);
void WONRoutingUnbanClientA(HWONROUTINGSESSION theSession,const char *theClientName);
void WONRoutingUnbanClientW(HWONROUTINGSESSION theSession,const wchar_t *theClientName);
void WONRoutingInviteClientA(HWONROUTINGSESSION theSession,const char *theClientName);
void WONRoutingInviteClientW(HWONROUTINGSESSION theSession,const wchar_t *theClientName);
void WONRoutingUninviteClientA(HWONROUTINGSESSION theSession,const char *theClientName);
void WONRoutingUninviteClientW(HWONROUTINGSESSION theSession,const wchar_t *theClientName);
void WONRoutingMuteClient(HWONROUTINGSESSION theSession,WONRoutingClientId theClientId);
void WONRoutingUnmuteClient(HWONROUTINGSESSION theSession,WONRoutingClientId theClientId);
void WONRoutingAddSuccessor(HWONROUTINGSESSION theSession,WONRoutingClientId theClientId);
void WONRoutingRemoveSuccessor(HWONROUTINGSESSION theSession,WONRoutingClientId theClientId);

void WONRoutingOpenRegistrationAndGetResult(HWONROUTINGSESSION theSession, WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingCloseRegistrationAndGetResult(HWONROUTINGSESSION theSession, WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingSetPasswordAndGetResultA(HWONROUTINGSESSION theSession, const char *thePassword, WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingSetPasswordAndGetResultW(HWONROUTINGSESSION theSession, const wchar_t *thePassword, WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingBecomeModeratorAndGetResult(HWONROUTINGSESSION theSession, BOOL flagOnOrOff, WONRoutingStatusCallback theCallback, void *theUserParam); 
void WONRoutingBootClientAndGetResult(HWONROUTINGSESSION theSession, WONRoutingClientId theClientId,WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingBanClientAndGetResultA(HWONROUTINGSESSION theSession, const char *theClientName, WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingBanClientAndGetResultW(HWONROUTINGSESSION theSession, const wchar_t *theClientName, WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingUnbanClientAndGetResultA(HWONROUTINGSESSION theSession,const char *theClientName, WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingUnbanClientAndGetResultW(HWONROUTINGSESSION theSession,const wchar_t *theClientName, WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingInviteClientAndGetResultA(HWONROUTINGSESSION theSession,const char *theClientName, WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingInviteClientAndGetResultW(HWONROUTINGSESSION theSession,const wchar_t *theClientName, WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingUninviteClientAndGetResultA(HWONROUTINGSESSION theSession,const char *theClientName, WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingUninviteClientAndGetResultW(HWONROUTINGSESSION theSession,const wchar_t *theClientName, WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingMuteClientAndGetResult(HWONROUTINGSESSION theSession,WONRoutingClientId theClientId, WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingUnmuteClientAndGetResult(HWONROUTINGSESSION theSession,WONRoutingClientId theClientId, WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingAddSuccessorAndGetResult(HWONROUTINGSESSION theSession,WONRoutingClientId theClientId, WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingRemoveSuccessorAndGetResult(HWONROUTINGSESSION theSession,WONRoutingClientId theClientId, WONRoutingStatusCallback theCallback, void *theUserParam);

///////////////////////////////////////////////////////////////////////////////
// Group management
void WONRoutingCreateGroup(HWONROUTINGSESSION theSession, const char *theGroupName, BOOL isPublic, BOOL announceChanges, const WONRoutingClientId theInitialMembers[], unsigned short theNumInitialMembers);
void WONRoutingDeleteGroup(HWONROUTINGSESSION theSession, WONRoutingGroupId theGroupId);
void WONRoutingAddToGroup(HWONROUTINGSESSION theSession, WONRoutingClientId theClientId, WONRoutingGroupId theGroupId);
void WONRoutingRemoveFromGroup(HWONROUTINGSESSION theSession, WONRoutingClientId theClientId, WONRoutingGroupId theGroupId);

void WONRoutingCreateGroupAndGetResult(HWONROUTINGSESSION theSession, const char *theGroupName, BOOL isPublic, BOOL announceChanges, const WONRoutingClientId theInitialMembers[], unsigned short theNumInitialMembers, WONRoutingCreateGroupCallback theCallback, void *theUserParam);
void WONRoutingDeleteGroupAndGetResult(HWONROUTINGSESSION theSession, WONRoutingGroupId theGroupId, WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingAddToGroupAndGetResult(HWONROUTINGSESSION theSession, WONRoutingClientId theClientId, WONRoutingGroupId theGroupId, WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingRemoveFromGroupAndGetResult(HWONROUTINGSESSION theSession, WONRoutingClientId theClientId, WONRoutingGroupId theGroupId, WONRoutingStatusCallback theCallback, void *theUserParam);

///////////////////////////////////////////////////////////////////////////////
// Dataobjects
void WONRoutingCreateDataObject(HWONROUTINGSESSION theSession, WONRoutingClientOrGroupId theLinkId, const void *theDataType, unsigned short theDataTypeLen, WONRoutingClientOrGroupId theOwnerId, unsigned short theLifespan, const void *theData, unsigned short theDataLen);
void WONRoutingDeleteDataObject(HWONROUTINGSESSION theSession, WONRoutingClientOrGroupId theLinkId, const void *theDataType, unsigned short theDataTypeLen);
void WONRoutingModifyDataObject(HWONROUTINGSESSION theSession, WONRoutingClientOrGroupId theLinkId, const void *theDataType, unsigned short theDataTypeLen, unsigned short theOffset, BOOL isInsert, const void *theData, unsigned short theDataLen);
void WONRoutingReplaceDataObject(HWONROUTINGSESSION theSession, WONRoutingClientOrGroupId theLinkId, const void *theDataType, unsigned short theDataTypeLen, const void *theData, unsigned short theDataLen);
void WONRoutingRenewDataObject(HWONROUTINGSESSION theSession, WONRoutingClientOrGroupId theLinkId, const void *theDataType, unsigned short theDataTypeLen, unsigned short theLifespan);
void WONRoutingUnsubscribeDataObject(HWONROUTINGSESSION theSession, WONRoutingClientOrGroupId theLinkId, const void *theDataType, unsigned short theDataTypeLen, BOOL flagExactOrRecursive, BOOL flagGroupOrMembers);

void WONRoutingCreateDataObjectAndGetResult(HWONROUTINGSESSION theSession, WONRoutingClientOrGroupId theLinkId, const void *theDataType, unsigned short theDataTypeLen, WONRoutingClientOrGroupId theOwnerId, unsigned short theLifespan, const void *theData, unsigned short theDataLen,WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingDeleteDataObjectAndGetResult(HWONROUTINGSESSION theSession, WONRoutingClientOrGroupId theLinkId, const void *theDataType, unsigned short theDataTypeLen,WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingModifyDataObjectAndGetResult(HWONROUTINGSESSION theSession, WONRoutingClientOrGroupId theLinkId, const void *theDataType, unsigned short theDataTypeLen, unsigned short theOffset, BOOL isInsert, const void *theData, unsigned short theDataLen,WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingReplaceDataObjectAndGetResult(HWONROUTINGSESSION theSession, WONRoutingClientOrGroupId theLinkId, const void *theDataType, unsigned short theDataTypeLen, const void *theData, unsigned short theDataLen,WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingRenewDataObjectAndGetResult(HWONROUTINGSESSION theSession, WONRoutingClientOrGroupId theLinkId, const void *theDataType, unsigned short theDataTypeLen, unsigned short theLifespan,WONRoutingStatusCallback theCallback, void *theUserParam);
void WONRoutingUnsubscribeDataObjectAndGetResult(HWONROUTINGSESSION theSession, WONRoutingClientOrGroupId theLinkId, const void *theDataType, unsigned short theDataTypeLen, BOOL flagExactOrRecursive, BOOL flagGroupOrMembers,WONRoutingStatusCallback theCallback, void *theUserParam);

void WONRoutingReadDataObject(HWONROUTINGSESSION theSession, WONRoutingClientOrGroupId theLinkId, const void *theDataType, unsigned short theDataTypeLen, BOOL flagExactOrRecursive, BOOL flagGroupOrMembers, WONRoutingReadDataObjectCallback theCallback, void *theUserParam);
void WONRoutingSubscribeDataObject(HWONROUTINGSESSION theSession, WONRoutingClientOrGroupId theLinkId, const void *theDataType, unsigned short theDataTypeLen, BOOL flagExactOrRecursive, BOOL flagGroupOrMembers, WONRoutingReadDataObjectCallback theCallback, void *theUserParam);

///////////////////////////////////////////////////////////////////////////////
// User Queries
void WONRoutingIsUserPresentA(HWONROUTINGSESSION theSession, const char* theUserName, BOOL caseSensitive, BOOL shouldSendNACK,WONRoutingBoolCallback theCallback, void *theUserParam);
void WONRoutingIsUserPresentW(HWONROUTINGSESSION theSession, const wchar_t* theUserName, BOOL caseSensitive, BOOL shouldSendNACK, WONRoutingBoolCallback theCallback, void *theUserParam);
void WONRoutingGetNumUsers(HWONROUTINGSESSION theSession,WONRoutingGetNumUsersCallback theCallback, void *theUserParam);

///////////////////////////////////////////////////////////////////////////////
// Unsolicited Data Handlers
void WONRoutingInstallClientJoinAttemptCallback(HWONROUTINGSESSION theSession, WONRoutingClientNameCallback theCallback, void *theUserParam);      
void WONRoutingInstallClientEnterCallback(HWONROUTINGSESSION theSession, WONRoutingClientIdWithReasonCallback theCallback, void *theUserParam);     
void WONRoutingInstallClientEnterExCallback(HWONROUTINGSESSION theSession, WONRoutingClientDataWithReasonCallback theCallback, void *theUserParam); 
void WONRoutingInstallClientLeaveCallback(HWONROUTINGSESSION theSession, WONRoutingClientIdWithReasonCallback theCallback, void *theUserParam);     
void WONRoutingInstallSpectatorCountCallback(HWONROUTINGSESSION theSession, WONRoutingSpectatorCallback theCallback, void *theUserParam);                       
void WONRoutingInstallGroupEnterCallback(HWONROUTINGSESSION theSession, WONRoutingGroupIdAndClientIdWithReasonCallback theCallback, void *theUserParam);      
void WONRoutingInstallGroupEnterExCallback(HWONROUTINGSESSION theSession, WONRoutingGroupIdAndClientDataWithReasonCallback theCallback, void *theUserParam);  
void WONRoutingInstallGroupLeaveCallback(HWONROUTINGSESSION theSession, WONRoutingGroupIdAndClientIdWithReasonCallback theCallback, void *theUserParam);      
void WONRoutingInstallGroupSpectatorCountCallback(HWONROUTINGSESSION theSession, WONRoutingGroupIdAndSpectatorCountCallback theCallback, void *theUserParam); 
void WONRoutingInstallMuteClientCallback(HWONROUTINGSESSION theSession, WONRoutingClientIdWithFlagCallback theCallback, void *theUserParam);                   
void WONRoutingInstallBecomeModeratorCallback(HWONROUTINGSESSION theSession, WONRoutingClientIdWithFlagCallback theCallback, void *theUserParam);              
void WONRoutingInstallHostChangeCallback(HWONROUTINGSESSION theSession, WONRoutingClientIdCallback theCallback, void *theUserParam);                                 
void WONRoutingInstallDataObjectCreationCallback(HWONROUTINGSESSION theSession, WONRoutingDataObjectWithLifespanCallback theCallback, void *theUserParam);    
void WONRoutingInstallDataObjectDeletionCallback(HWONROUTINGSESSION theSession, WONRoutingDataObjectCallback theCallback, void *theUserParam);                
void WONRoutingInstallDataObjectModificationCallback(HWONROUTINGSESSION theSession, WONRoutingDataObjectModificationCallback theCallback, void *theUserParam);
void WONRoutingInstallDataObjectReplacementCallback(HWONROUTINGSESSION theSession, WONRoutingDataObjectCallback theCallback, void *theUserParam);             
void WONRoutingInstallKeepAliveCallback(HWONROUTINGSESSION theSession, WONRoutingVoidCallback theCallback, void *theUserParam);                                      
void WONRoutingInstallPeerDataCallback(HWONROUTINGSESSION theSession, WONRoutingMessageWithClientIdCallback theCallback, void *theUserParam);                 
void WONRoutingInstallRawPeerChatCallback(HWONROUTINGSESSION theSession, WONRoutingRawChatMessageCallback theCallback, void *theUserParam);                   
void WONRoutingInstallASCIIPeerChatCallback(HWONROUTINGSESSION theSession, WONRoutingASCIIChatMessageCallback theCallback, void *theUserParam);               
void WONRoutingInstallUnicodePeerChatCallback(HWONROUTINGSESSION theSession, WONRoutingUnicodeChatMessageCallback theCallback, void *theUserParam);           
void WONRoutingInstallReconnectFailureCallback(HWONROUTINGSESSION theSession, WONRoutingVoidCallback theCallback, void *theUserParam);                               

///////////////////////////////////////////////////////////////////////////////
// Copy routines for structs
void CopyWONRoutingMessage(WON_CONST WONRoutingMessage* theSrcP, WONRoutingMessage* theDestP);
void CopyWONRoutingRegisterClientResult(WON_CONST WONRoutingRegisterClientResult* theSrcP, WONRoutingRegisterClientResult* theDestP);
void CopyWONRoutingClientData(WON_CONST WONRoutingClientData* theSrcP, WONRoutingClientData* theDestP);
void CopyWONRoutingClientListResult(WON_CONST WONRoutingClientListResult* theSrcP, WONRoutingClientListResult* theDestP);
void CopyWONRoutingClientInfoResult(WON_CONST WONRoutingClientInfoResult* theSrcP, WONRoutingClientInfoResult* theDestP);
void CopyWONRoutingGroupData(WON_CONST WONRoutingGroupData* theSrcP, WONRoutingGroupData* theDestP);
void CopyWONRoutingGroupListResult(WON_CONST WONRoutingGroupListResult* theSrcP, WONRoutingGroupListResult* theDestP);
void CopyWONRoutingCreateGroupResult(WON_CONST WONRoutingCreateGroupResult* theSrcP, WONRoutingCreateGroupResult* theDestP);
void CopyWONRoutingDataObject(WON_CONST WONRoutingDataObject* theSrcP, WONRoutingDataObject* theDestP);
void CopyWONRoutingDataObjectWithLifespan(WON_CONST WONRoutingDataObjectWithLifespan* theSrcP, WONRoutingDataObjectWithLifespan* theDestP);
void CopyWONRoutingDataObjectModification(WON_CONST WONRoutingDataObjectModification* theSrcP, WONRoutingDataObjectModification* theDestP);
void CopyWONRoutingReadDataObjectResult(WON_CONST WONRoutingReadDataObjectResult* theSrcP, WONRoutingReadDataObjectResult* theDestP);
void CopyWONRoutingGetNumUsersResult(WON_CONST WONRoutingGetNumUsersResult* theSrcP, WONRoutingGetNumUsersResult* theDestP);
void CopyWONRoutingClientNameResult(WON_CONST WONRoutingClientNameResult* theSrcP, WONRoutingClientNameResult* theDestP);
void CopyWONRoutingClientIdWithReason(WON_CONST WONRoutingClientIdWithReason* theSrcP, WONRoutingClientIdWithReason* theDestP);
void CopyWONRoutingGroupIdAndClientIdWithReason(WON_CONST WONRoutingGroupIdAndClientIdWithReason* theSrcP, WONRoutingGroupIdAndClientIdWithReason* theDestP);
void CopyWONRoutingClientDataWithReason(WON_CONST WONRoutingClientDataWithReason* theSrcP, WONRoutingClientDataWithReason* theDestP);
void CopyWONRoutingGroupIdAndClientDataWithReason(WON_CONST WONRoutingGroupIdAndClientDataWithReason* theSrcP, WONRoutingGroupIdAndClientDataWithReason* theDestP);
void CopyWONRoutingGroupIdAndSpectatorCount(WON_CONST WONRoutingGroupIdAndSpectatorCount* theSrcP, WONRoutingGroupIdAndSpectatorCount* theDestP);
void CopyWONRoutingClientIdWithFlag(WON_CONST WONRoutingClientIdWithFlag* theSrcP, WONRoutingClientIdWithFlag* theDestP);
void CopyWONRoutingMessageWithClientId(WON_CONST WONRoutingMessageWithClientId* theSrcP, WONRoutingMessageWithClientId* theDestP);
void CopyWONRoutingChatMessageBase(WON_CONST WONRoutingChatMessageBase* theSrcP, WONRoutingChatMessageBase* theDestP);
void CopyWONRoutingRawChatMessage(WON_CONST WONRoutingRawChatMessage* theSrcP, WONRoutingRawChatMessage* theDestP);
void CopyWONRoutingASCIIChatMessage(WON_CONST WONRoutingASCIIChatMessage* theSrcP, WONRoutingASCIIChatMessage* theDestP);
void CopyWONRoutingUnicodeChatMessage(WON_CONST WONRoutingUnicodeChatMessage* theSrcP, WONRoutingUnicodeChatMessage* theDestP);

///////////////////////////////////////////////////////////////////////////////
// Delete routines for structs
void DeleteWONRoutingMessage(WON_CONST WONRoutingMessage* theSrcP);
void DeleteWONRoutingRegisterClientResult(WON_CONST WONRoutingRegisterClientResult* theSrcP);
void DeleteWONRoutingClientData(WON_CONST WONRoutingClientData* theSrcP);
void DeleteWONRoutingClientListResult(WON_CONST WONRoutingClientListResult* theSrcP);
void DeleteWONRoutingClientInfoResult(WON_CONST WONRoutingClientInfoResult* theSrcP);
void DeleteWONRoutingGroupData(WON_CONST WONRoutingGroupData* theSrcP);
void DeleteWONRoutingGroupListResult(WON_CONST WONRoutingGroupListResult* theSrcP);
void DeleteWONRoutingCreateGroupResult(WON_CONST WONRoutingCreateGroupResult* theSrcP);
void DeleteWONRoutingDataObject(WON_CONST WONRoutingDataObject* theSrcP);
void DeleteWONRoutingDataObjectWithLifespan(WON_CONST WONRoutingDataObjectWithLifespan* theSrcP);
void DeleteWONRoutingDataObjectModification(WON_CONST WONRoutingDataObjectModification* theSrcP);
void DeleteWONRoutingReadDataObjectResult(WON_CONST WONRoutingReadDataObjectResult* theSrcP);
void DeleteWONRoutingGetNumUsersResult(WON_CONST WONRoutingGetNumUsersResult* theSrcP);
void DeleteWONRoutingClientNameResult(WON_CONST WONRoutingClientNameResult* theSrcP);
void DeleteWONRoutingClientIdWithReason(WON_CONST WONRoutingClientIdWithReason* theSrcP);
void DeleteWONRoutingGroupIdAndClientIdWithReason(WON_CONST WONRoutingGroupIdAndClientIdWithReason* theSrcP);
void DeleteWONRoutingClientDataWithReason(WON_CONST WONRoutingClientDataWithReason* theSrcP);
void DeleteWONRoutingGroupIdAndClientDataWithReason(WON_CONST WONRoutingGroupIdAndClientDataWithReason* theSrcP);
void DeleteWONRoutingGroupIdAndSpectatorCount(WON_CONST WONRoutingGroupIdAndSpectatorCount* theSrcP);
void DeleteWONRoutingClientIdWithFlag(WON_CONST WONRoutingClientIdWithFlag* theSrcP);
void DeleteWONRoutingMessageWithClientId(WON_CONST WONRoutingMessageWithClientId* theSrcP);
void DeleteWONRoutingChatMessageBase(WON_CONST WONRoutingChatMessageBase* theSrcP);
void DeleteWONRoutingRawChatMessage(WON_CONST WONRoutingRawChatMessage* theSrcP);
void DeleteWONRoutingASCIIChatMessage(WON_CONST WONRoutingASCIIChatMessage* theSrcP);
void DeleteWONRoutingUnicodeChatMessage(WON_CONST WONRoutingUnicodeChatMessage* theSrcP);

#ifdef __cplusplus
}
#endif

#endif