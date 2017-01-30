#ifndef __CROUTINGAPITYPES_H__
#define __CROUTINGAPITYPES_H__

#include "wondll.h"

typedef unsigned short WONRoutingClientId;
typedef unsigned short WONRoutingGroupId;
typedef unsigned short WONRoutingClientOrGroupId;

typedef enum 
{
	GROUPID_ALLUSERS          =0,
    GROUPID_ALLGROUPS         =1,
    GROUPID_ALLUSERSANDGROUPS =2,
    GROUPID_NOOWNER           =3,
    GROUPID_CHAT              =4 
};

typedef enum 
{ 
	CHATTYPE_UNKNOWN = 0, 
	CHATTYPE_ASCII = 1, 
	CHATTYPE_ASCII_EMOTE = 2, 
	CHATTYPE_UNICODE = 3, 
	CHATTYPE_UNICODE_EMOTE = 4 
} WONRoutingChatType ;

typedef struct
{
	void *mData;
	unsigned short mDataLen;
} WONRoutingMessage;

typedef struct  
{
	short  mStatus;
	const wchar_t* mHostNameW;
	const wchar_t* mHostCommentW;
	const char* mHostNameA;
	const char* mHostCommentA;
	WONRoutingClientId   mClientId;
} WONRoutingRegisterClientResult;


typedef struct
{
	WONRoutingClientId mClientId;
	const wchar_t* mClientNameW;
	const char* mClientNameA;

	unsigned long  mIPAddress;
	unsigned long  mWONUserId;
	unsigned long  mCommunityId;
	unsigned short mTrustLevel;
	BOOL           mIsModerator;
	BOOL           mIsMuted;
} WONRoutingClientData;


typedef struct
{
	short mStatus;
	BOOL  mIncludesIPs;
	BOOL  mIncludesAuthInfo;
	BOOL  mIncludesMuteFlag;
	BOOL  mIncludesModeratorFlag;

	unsigned short mNumSpectators;
	unsigned short mNumClients;
	WONRoutingClientData *mClients;
} WONRoutingClientListResult;

typedef struct
{
	short mStatus;
	BOOL  mIncludesIPs;
	BOOL  mIncludesAuthInfo;
	BOOL  mIncludesMuteFlag;
	BOOL  mIncludesModeratorFlag;

	WONRoutingClientData mClient;
} WONRoutingClientInfoResult;

typedef struct 
{
	WONRoutingGroupId   mGroupId;
	const char* mGroupName;
	BOOL mIsPublic;
} WONRoutingGroupData;

typedef struct
{
	short mStatus;
	unsigned short mNumGroups;
	WONRoutingGroupData *mGroups;

} WONRoutingGroupListResult;

typedef struct 
{
	short   mStatus;
	WONRoutingGroupId mGroupId;
} WONRoutingCreateGroupResult;

typedef struct
{
	WONRoutingClientOrGroupId      mLinkId;
	WONRoutingClientOrGroupId      mOwnerId;

	const char *mData;
	unsigned short mDataLen;

	const char *mDataType;
	unsigned short mDataTypeLen;
} WONRoutingDataObject;

typedef struct  
{
	WONRoutingDataObject mObject;
	unsigned short  mLifespan;
} WONRoutingDataObjectWithLifespan;

typedef struct 
{
	WONRoutingDataObject mObject;

	BOOL           mIsInsert;
	unsigned short mOffset;
} WONRoutingDataObjectModification;

typedef struct 
{
	short                 mStatus;
	unsigned long mNumObjects;
	WONRoutingDataObject *mObjects;
} WONRoutingReadDataObjectResult;

typedef struct 
{
	unsigned short mNumActiveUsers;
	unsigned short mServerCapacity;
} WONRoutingGetNumUsersResult;

typedef struct
{
	const char*    mNameA;
	const wchar_t* mNameW;
} WONRoutingClientNameResult;

typedef struct  
{
	WONRoutingClientId mClientId;
	unsigned char mReason;
} WONRoutingClientIdWithReason;

typedef struct  
{
	WONRoutingGroupId mGroupId;
	WONRoutingClientId mClientId;
	unsigned char mReason;
} WONRoutingGroupIdAndClientIdWithReason;

typedef struct 
{
	unsigned char mReason;
	WONRoutingClientData mData;
} WONRoutingClientDataWithReason;

typedef struct 
{
	WONRoutingGroupId mGroupId;
	unsigned char mReason;
	WONRoutingClientData mData;
} WONRoutingGroupIdAndClientDataWithReason;

typedef struct 
{
	WONRoutingGroupId mGroupId;
	unsigned short    mSpectatorCount;
} WONRoutingGroupIdAndSpectatorCount;

typedef struct 
{
	WONRoutingClientId mClientId;
	BOOL mFlagOnOrOff;
} WONRoutingClientIdWithFlag;

typedef struct 
{
	const char *mData;
	unsigned short mDataLen;
	WONRoutingClientId mClientId;
} WONRoutingMessageWithClientId;

typedef struct
{
	WONRoutingClientId mSenderId;
	WONRoutingChatType mChatType;
	BOOL mIncludeExcludeFlag;
	const WONRoutingClientOrGroupId *mRecipientIds;
	unsigned short mNumRecipientIds;
} WONRoutingChatMessageBase;

typedef struct
{
	WONRoutingChatMessageBase mBaseData;
	const char* mData;
	unsigned short mDataLen;
} WONRoutingRawChatMessage;

typedef struct  
{
	WONRoutingChatMessageBase mBaseData;
	const char *mChat;
} WONRoutingASCIIChatMessage;

typedef struct 
{
	WONRoutingChatMessageBase mBaseData;
	const wchar_t *mChat;
} WONRoutingUnicodeChatMessage;


typedef void(*WONRoutingVoidCallback)(void *theUserParam);
typedef void(*WONRoutingErrorCallback)(WONError theError, void *userParam);
typedef void(*WONRoutingStatusCallback)(short theStatus, void *userParam);
typedef void(*WONRoutingBoolCallback)(BOOL theBool, void *userParam);
typedef void(*WONRoutingSpectatorCallback)(unsigned short theCount, void *theUserParam);
typedef void(*WONRoutingClientIdCallback)(WONRoutingClientId theId, void *theUserParam);
typedef void(*WONRoutingRegisterClientCallback)(WONRoutingRegisterClientResult *theResult, void *userParam); 
typedef void(*WONRoutingClientListCallback)(WONRoutingClientListResult *theList, void *userParam);
typedef void(*WONRoutingClientInfoCallback)(WONRoutingClientInfoResult *theInfo, void *userParam);
typedef void(*WONRoutingGroupListCallback)(WONRoutingGroupListResult *theList, void *userParam);
typedef void(*WONRoutingCreateGroupCallback)(WONRoutingCreateGroupResult *theResult, void *userParam);
typedef void(*WONRoutingReadDataObjectCallback)(WONRoutingReadDataObjectResult *theObjects, void *userParam);
typedef void(*WONRoutingGetNumUsersCallback)(WONRoutingGetNumUsersResult *theResult, void *userParam);
typedef void(*WONRoutingClientNameCallback)(WONRoutingClientNameResult *theResult, void *userParam);
typedef void(*WONRoutingClientIdWithReasonCallback)(WONRoutingClientIdWithReason *theReason, void *userParam);
typedef void(*WONRoutingGroupIdAndClientIdWithReasonCallback)(WONRoutingGroupIdAndClientIdWithReason *theReason, void *userParam);
typedef void(*WONRoutingClientDataWithReasonCallback)(WONRoutingClientDataWithReason *theData, void *userParam);
typedef void(*WONRoutingGroupIdAndClientDataWithReasonCallback)(WONRoutingGroupIdAndClientDataWithReason *theData, void *userParam);
typedef void(*WONRoutingGroupIdAndSpectatorCountCallback)(WONRoutingGroupIdAndSpectatorCount *theData, void *theUserParam);
typedef void(*WONRoutingClientIdWithFlagCallback)(WONRoutingClientIdWithFlag *theData, void *theParam);
typedef void(*WONRoutingDataObjectCallback)(WONRoutingDataObject *theData, void *theParam);
typedef void(*WONRoutingDataObjectWithLifespanCallback)(WONRoutingDataObjectWithLifespan *theData, void *theParam);
typedef void(*WONRoutingDataObjectModificationCallback)(WONRoutingDataObjectModification *theData, void *theParam);

typedef void(*WONRoutingMessageWithClientIdCallback)(WONRoutingMessageWithClientId *theMsg, void *theParam);
typedef void(*WONRoutingRawChatMessageCallback)(WONRoutingRawChatMessage *theMsg, void *theParam);
typedef void(*WONRoutingASCIIChatMessageCallback)(WONRoutingASCIIChatMessage *theMsg, void *theParam);
typedef void(*WONRoutingUnicodeChatMessageCallback)(WONRoutingUnicodeChatMessage *theMsg, void *theParam);
#endif
