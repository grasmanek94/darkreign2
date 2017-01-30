#ifndef ROUTINGAPITYPES_H
#define ROUTINGAPITYPES_H

typedef std::basic_string<unsigned char> ClientName;
typedef std::string GroupName;
typedef std::wstring Password;
typedef unsigned short ClientId;
typedef unsigned short GroupId;
typedef unsigned short ClientOrGroupId;

enum { GROUPID_ALLUSERS          =0,
       GROUPID_ALLGROUPS         =1,
       GROUPID_ALLUSERSANDGROUPS =2,
       GROUPID_NOOWNER           =3,
       GROUPID_CHAT              =4 };

enum { CLIENTLISTTYPE_COMPLETE = 0,
	   CLIENTLISTTYPE_BANNED   = 1,
	   CLIENTLISTTYPE_INVITED  = 2,
	   CLIENTLISTTYPE_MUTED    = 3 };

struct ClientIdWithFlag {
	ClientId mClientId;
	bool     mFlagOnOrOff;

	ClientIdWithFlag() : mClientId(0), mFlagOnOrOff(true) {}
	ClientIdWithFlag(ClientId theClientId, bool flagOnOrOff) : mClientId(theClientId), mFlagOnOrOff(flagOnOrOff) {}
};

struct ClientIdWithReason {
	ClientId      mClientId;
	unsigned char mReason;

	ClientIdWithReason() : mClientId(0), mReason(0) {}
	ClientIdWithReason(ClientId theClientId, unsigned char theReason) : mClientId(theClientId), mReason(theReason) {}
};

struct ClientDataWithReason : public WONMsg::MMsgRoutingGetClientListReply::ClientData {
	unsigned char mReason;

	ClientDataWithReason() : mReason(0) {}
	ClientDataWithReason(const WONMsg::MMsgRoutingGetClientListReply::ClientData& theClientDataR, unsigned char theReason) : WONMsg::MMsgRoutingGetClientListReply::ClientData(theClientDataR), mReason(theReason) {}
};

struct DataObject {
	ClientOrGroupId      mLinkId;
	ClientOrGroupId      mOwnerId;
	WONCommon::RawBuffer mDataType;
	WONCommon::RawBuffer mData;

	DataObject() : mLinkId(0), mOwnerId(0) {}
	DataObject(ClientOrGroupId theLinkId, const WONCommon::RawBuffer& theDataTypeR) : mLinkId(theLinkId), mOwnerId(0), mDataType(theDataTypeR) {}
	DataObject(ClientOrGroupId theLinkId, const WONCommon::RawBuffer& theDataTypeR, const WONCommon::RawBuffer& theDataR) : mLinkId(theLinkId), mOwnerId(0), mDataType(theDataTypeR), mData(theDataR) {}
	DataObject(ClientOrGroupId theLinkId, ClientOrGroupId theOwnerId, const WONCommon::RawBuffer& theDataTypeR, const WONCommon::RawBuffer& theDataR) : mLinkId(theLinkId), mOwnerId(theOwnerId), mDataType(theDataTypeR), mData(theDataR) {}
};

struct DataObjectWithLifespan : public DataObject {
	unsigned short  mLifespan;

	DataObjectWithLifespan() : mLifespan(0) {}
	DataObjectWithLifespan(ClientOrGroupId theLinkId, const WONCommon::RawBuffer& theDataTypeR) : DataObject(theLinkId, theDataTypeR), mLifespan(0) {}
	DataObjectWithLifespan(ClientOrGroupId theLinkId, const WONCommon::RawBuffer& theDataTypeR, const WONCommon::RawBuffer& theDataR) : DataObject(theLinkId, theDataTypeR, theDataR), mLifespan(0) {}
	DataObjectWithLifespan(ClientOrGroupId theLinkId, ClientOrGroupId theOwnerId, const WONCommon::RawBuffer& theDataTypeR, const WONCommon::RawBuffer& theDataR, unsigned short theLifespan) : DataObject(theLinkId, theOwnerId, theDataTypeR, theDataR), mLifespan(theLifespan) {}
};

struct DataObjectModification : public DataObject {
	bool           mIsInsert;
	unsigned short mOffset;

	DataObjectModification(bool isInsert =false, unsigned short theOffset =0) : mIsInsert(isInsert), mOffset(theOffset) {}
	DataObjectModification(ClientOrGroupId theLinkId, const WONCommon::RawBuffer& theDataTypeR, bool isInsert =false, unsigned short theOffset =0) : DataObject(theLinkId, theDataTypeR), mIsInsert(isInsert), mOffset(theOffset) {}
	DataObjectModification(ClientOrGroupId theLinkId, const WONCommon::RawBuffer& theDataTypeR, const WONCommon::RawBuffer& theDataR, bool isInsert =false, unsigned short theOffset =0) : DataObject(theLinkId, theDataTypeR, theDataR), mIsInsert(isInsert), mOffset(theOffset) {}
	DataObjectModification(ClientOrGroupId theLinkId, ClientOrGroupId theOwnerId, const WONCommon::RawBuffer& theDataTypeR, const WONCommon::RawBuffer& theDataR, bool isInsert =false, unsigned short theOffset =0) : DataObject(theLinkId, theOwnerId, theDataTypeR, theDataR), mIsInsert(isInsert), mOffset(theOffset) {}
};

// FIX: can we use the one in MMsgRoutingGetGroupList.h???
struct GroupData {
	GroupId   mGroupId;
	GroupName mGroupName;
	bool      mIsPublic;
};

struct GroupIdAndSpectatorCount {
	GroupId        mGroupId;
	unsigned short mSpectatorCount;

	GroupIdAndSpectatorCount() : mGroupId(0), mSpectatorCount(0) {}
	GroupIdAndSpectatorCount(GroupId theGroupId, unsigned short theSpectatorCount) : mGroupId(theGroupId), mSpectatorCount(theSpectatorCount) {}
};

struct GroupIdAndClientIdWithReason : ClientIdWithReason {
	GroupId mGroupId;

	GroupIdAndClientIdWithReason() : mGroupId(0) {}
	GroupIdAndClientIdWithReason(GroupId theGroupId, ClientId theClientId, unsigned char theReason) : mGroupId(theGroupId), ClientIdWithReason(theClientId, theReason) {}
};

struct GroupIdAndClientDataWithReason : ClientDataWithReason {
	GroupId mGroupId;

	GroupIdAndClientDataWithReason() : mGroupId(0) {}
	GroupIdAndClientDataWithReason(GroupId theGroupId, const WONMsg::MMsgRoutingGetClientListReply::ClientData& theClientDataR, unsigned char theReason) : mGroupId(theGroupId), ClientDataWithReason(theClientDataR, theReason) {}
};

struct Message {
	const unsigned char* mBufP;
	unsigned short mBufLen;

	Message() : mBufP(NULL), mBufLen(0) {}
	Message(const unsigned char* theBufP, unsigned short theBufLen) : mBufP(theBufP), mBufLen(theBufLen) {}
};

struct MessageWithClientId : public Message {
	ClientId mClientId;

	MessageWithClientId() : mClientId(0) {}
	MessageWithClientId(ClientId theClientId, const unsigned char* theBufP, unsigned short theBufLen) : Message(theBufP, theBufLen), mClientId(theClientId) {}
};

struct ChatMessageBase {
	ClientId mSenderId;
	WONMsg::ChatType mChatType;
	bool mIncludeExcludeFlag;
	std::vector<ClientOrGroupId> mRecipientIds;

	ChatMessageBase() : mSenderId(0), mChatType(WONMsg::CHATTYPE_UNKNOWN) {}
	ChatMessageBase(const WONMsg::MMsgRoutingPeerChat& thePeerChatMsgR);

	bool IsWhisper() const { return mIncludeExcludeFlag == true || mRecipientIds.size() != 0; }
};

struct RawChatMessage : public ChatMessageBase {
	const unsigned char* mBufP;
	unsigned short mBufLen;

	RawChatMessage() : mBufP(NULL), mBufLen(0) {}
	RawChatMessage(const WONMsg::MMsgRoutingPeerChat& thePeerChatMsgR);
};

struct ASCIIChatMessage : public ChatMessageBase {
	std::string mData;

	ASCIIChatMessage() {}
	ASCIIChatMessage(const WONMsg::MMsgRoutingPeerChat& thePeerChatMsgR);
};

struct UnicodeChatMessage : public ChatMessageBase {
	std::wstring mData;

	UnicodeChatMessage() {}
	UnicodeChatMessage(const WONMsg::MMsgRoutingPeerChat& thePeerChatMsgR);
};

struct CreateGroupResult {
	short   mStatus;
	GroupId mGroupId;

	CreateGroupResult() : mStatus(0), mGroupId(0) {}
	explicit CreateGroupResult(short theStatus) : mStatus(theStatus), mGroupId(0) {}
	explicit CreateGroupResult(const WONMsg::MMsgRoutingCreateGroupReply& theReplyR);
};

struct ClientDataResultBase {
	short mStatus;
	bool  mIncludesIPs;
	bool  mIncludesAuthInfo;
	bool  mIncludesMuteFlag;
	bool  mIncludesModeratorFlag;

	ClientDataResultBase() : mStatus(32000), mIncludesIPs(false), mIncludesAuthInfo(false), mIncludesMuteFlag(false), mIncludesModeratorFlag(false) {}
	explicit ClientDataResultBase(short theStatus) : mStatus(theStatus), mIncludesIPs(false), mIncludesAuthInfo(false), mIncludesMuteFlag(false), mIncludesModeratorFlag(false) {}
	explicit ClientDataResultBase(const WONMsg::MMsgRoutingGetClientListReply& theReplyR);
	bool IncludesIPs() const { return mIncludesIPs; }
	bool IncludesAuthInfo() const { return mIncludesAuthInfo; }
	bool IncludesMuteFlag() const { return mIncludesMuteFlag; }
	bool IncludesModeratorFlag() const { return mIncludesModeratorFlag; }
};

// FIX: derive from ClientData?
struct GetClientInfoResult : public ClientDataResultBase {
	WONMsg::MMsgRoutingGetClientListReply::ClientData mClientData;

	GetClientInfoResult() {}
	explicit GetClientInfoResult(short theStatus) : ClientDataResultBase(theStatus) {}
	explicit GetClientInfoResult(const WONMsg::MMsgRoutingGetClientListReply& theReplyR);
};

struct GetClientListResult : public ClientDataResultBase {
	unsigned short mNumSpectators;
	std::list<WONMsg::MMsgRoutingGetClientListReply::ClientData> mClientList;

	GetClientListResult() {}
	explicit GetClientListResult(short theStatus) : ClientDataResultBase(theStatus) {}
	explicit GetClientListResult(const WONMsg::MMsgRoutingGetClientListReply& theReplyR);
};

struct GetGroupListResult {
	short                mStatus;
	std::list<GroupData> mGroupList;

	GetGroupListResult() : mStatus(32000) {}
	explicit GetGroupListResult(short theStatus) : mStatus(theStatus) {}
	explicit GetGroupListResult(const WONMsg::MMsgRoutingGetGroupListReply& theReplyR);
};

struct GetSimpleClientListResult {
	short                    mStatus;
	std::list<ClientId>      mClientIdList;
	std::list<ClientName>    mClientNameList;
	std::list<unsigned long> mWONUserIdList;

	GetSimpleClientListResult() : mStatus(32000) {}
	explicit GetSimpleClientListResult(short theStatus) : mStatus(theStatus) {}
	explicit GetSimpleClientListResult(const WONMsg::MMsgRoutingGetSimpleClientListReply& theReplyR);
};

struct GetNumUsersResult {
	unsigned short mNumActiveUsers;
	unsigned short mServerCapacity;
	explicit GetNumUsersResult(unsigned short theNumActiveUsers =0, unsigned short theServerCapacity =0) : mNumActiveUsers(theNumActiveUsers), mServerCapacity(theServerCapacity) {}
	explicit GetNumUsersResult(const WONMsg::MMsgCommGetNumUsersReply& theReplyR);
};
	
struct ReadDataObjectResult {
	short                 mStatus;
	std::list<DataObject> mDataObjectList;

	ReadDataObjectResult() : mStatus(32000) {}
	explicit ReadDataObjectResult(short theStatus) : mStatus(theStatus) {}
	explicit ReadDataObjectResult(const WONMsg::MMsgRoutingReadDataObjectReply& theReplyR);
};

struct RegisterClientResult {
	short      mStatus;
	ClientName mHostName;
	wstring    mHostComment;
	ClientId   mClientId;

	RegisterClientResult() : mStatus(0), mClientId(0) {}
	RegisterClientResult(short theStatus, const ClientName& theHostNameR =gEmptyRawBuffer, const wstring& theHostCommentR =gEmptyWString, ClientId theClientId =0) : mStatus(theStatus), mHostName(theHostNameR), mHostComment(theHostCommentR), mClientId(theClientId) {}
	explicit RegisterClientResult(const WONMsg::MMsgRoutingRegisterClientReply& theReplyR);
};

#endif // ROUTINGAPITYPES_H