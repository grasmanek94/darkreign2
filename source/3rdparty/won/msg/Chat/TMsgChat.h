#ifndef _TITAN_CHAT_SERVER_MESSAGES_H_
#define _TITAN_CHAT_SERVER_MESSAGES_H_

#pragma warning(disable:4786) // Or STL will generate ugly warnings.
#include <map>
#include <set>
#include <deque>

#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"
#include "ChatTypes.h"


namespace WONMsg
{

enum MsgTypeChat
{
	// Chat Server messages
	Chat_UserJoin				= 0,	// Received (sent from peer slave)
	Chat_UserJoinReply			= 1,	// Sent	(received from peer master)
	Chat_UserJoined				= 2,	// Sent	(received from peer master)
	Chat_UserPresent			= 3,	// Received (sent from peer slave)
	Chat_UserPresentReply		= 4,	// Sent	(received from peer master)
	Chat_UserLeft				= 5,	// Symmetrical
	Chat_ChatDataMessage		= 6,	// Symmetrical
	Chat_SimpleChatDataMessage	= 7,	// Symmetrical
	Chat_UserDataMessage		= 8,	// Symmetrical
	Chat_ReqUserData			= 9,	// Symmetrical
	Chat_RemoveUserData			= 10,	// Symmetrical
	Chat_UserDataMessageReply	= 11,	// Sent	(received from peer master)
	Chat_RemoveUserDataReply	= 12,	// Sent (received from peer master)
	Chat_UnsubUserData			= 13,	// Received (never sent)
	Chat_ChangeName				= 14,	// Symmetrical
	Chat_ChangeNameReply		= 15,	// Sent	(received from peer master)
	Chat_UserLurk				= 16,	// Received (sent from peer slave)
	Chat_UserLurkReply			= 17,	// Sent	(received from peer master)
	Chat_UsersHere				= 18,	// Sent (received from peer master)
	Chat_SubscribeUserData		= 19,	// Symmetrical
	Chat_LurkerCount			= 20,	// Sent

	// Last Message type.  Don't use
	Chat_ChatMessageMax
};


typedef short ChatStatus;


class TMsgChat_UserJoin : public TMessage
{
private:
	wstring	mUserName;
	bool	mUserPresent;
	string  mServerPassword;

public:
	TMsgChat_UserJoin();
	TMsgChat_UserJoin(const TMessage& Tmsg) : TMessage(Tmsg)	{ Unpack(); }
	TMsgChat_UserJoin(const TMsgChat_UserJoin& toCopy);
	TMsgChat_UserJoin& operator=(const TMessage& Tmsg)			{ TMessage::operator=(Tmsg); Unpack(); return *this; }
	TMsgChat_UserJoin& operator=(const TMsgChat_UserJoin& toCopy);
	virtual TRawMsg* Duplicate() const;

	void Unpack();
	void* Pack();

	void GetUserName(wstring& name) const			{ name = mUserName; }
	bool GetUserPresent() const						{ return mUserPresent; }
	void GetServerPassword(string& serverpw) const	{ serverpw = mServerPassword; }

	void SetUserName(const wstring& userName)		{ mUserName = userName; }
	void SetUserPresent(bool userPresent)			{ mUserPresent = userPresent; }
	void SetServerPassword(const string& serverpw)	{ mServerPassword = serverpw; }
};


class TMsgChat_UserJoinReply : public TMessage
{
private:
	WONChat::UserID	mUserID;
	ChatStatus		mStatus;

public:
	TMsgChat_UserJoinReply();
	TMsgChat_UserJoinReply(const TMessage& Tmsg) : TMessage(Tmsg)	{ Unpack(); }
	TMsgChat_UserJoinReply(const TMsgChat_UserJoinReply& toCopy);
	TMsgChat_UserJoinReply& operator=(const TMessage& Tmsg)			{ TMessage::operator=(Tmsg); Unpack(); return *this; }
	TMsgChat_UserJoinReply& operator=(const TMsgChat_UserJoinReply& toCopy);
	virtual TRawMsg* Duplicate() const;

	void Unpack();
	void* Pack();

	WONChat::UserID	GetUserID() const		{ return mUserID; }
	ChatStatus		GetStatus() const		{ return mStatus; }

	void SetUserID(WONChat::UserID userID)	{ mUserID = userID; }
	void SetStatus(ChatStatus status)		{ mStatus = status; }
};


class TMsgChat_UserJoined : public TMessage
{
public:
	class UserInfo
	{
	public:
		std::wstring userName;
		WONChat::UserID	userID;
	};
private:
	std::deque<UserInfo*> usersJoined;

public:
	TMsgChat_UserJoined();
	TMsgChat_UserJoined(const TMessage& Tmsg) : TMessage(Tmsg)		{ Unpack(); }
	TMsgChat_UserJoined(const TMsgChat_UserJoined& toCopy);
	TMsgChat_UserJoined& operator=(const TMessage& Tmsg)			{ Drain(); TMessage::operator=(Tmsg); Unpack(); return *this; }
	TMsgChat_UserJoined& operator=(const TMsgChat_UserJoined& Tmsg);
	virtual TRawMsg* Duplicate() const;
	~TMsgChat_UserJoined(void)										{ Drain(); };

	void Unpack();
	void* Pack();

	void Drain(void);
	void Set(const std::deque<UserInfo*>& joiningUsers);
	void Get(std::deque<UserInfo*>& joiningUsers);

	void Add(WONChat::UserID userID, const std::wstring& userName);
	bool Pop(WONChat::UserID& userID, wstring& userName);
	bool GetAtIndex(unsigned short index, WONChat::UserID& userID, wstring& userName);
	unsigned short count(void) const				{ return usersJoined.size(); };			
	bool empty(void) const							{ return usersJoined.empty(); };
};


class TMsgChat_UsersHere : public TMessage
{
public:
	class UserInfo
	{
	public:
		std::wstring userName;
		WONChat::UserID	userID;
	};
private:
	std::deque<UserInfo*> usersHere;
public:
	TMsgChat_UsersHere();
	TMsgChat_UsersHere(const TMessage& Tmsg) : TMessage(Tmsg)		{ Unpack(); }
	TMsgChat_UsersHere(const TMsgChat_UsersHere& toCopy);
	TMsgChat_UsersHere& operator=(const TMessage& Tmsg)				{ Drain(); TMessage::operator=(Tmsg); Unpack(); return *this; }
	TMsgChat_UsersHere& operator=(const TMsgChat_UsersHere& Tmsg);
	virtual TRawMsg* Duplicate() const;
	~TMsgChat_UsersHere(void)										{ Drain(); };

	void Unpack();
	void* Pack();

	void Drain(void);
	void Set(const std::deque<UserInfo*>& users);
	void Get(std::deque<UserInfo*>& users);

	void Add(WONChat::UserID userID, const std::wstring& userName);
	bool Pop(WONChat::UserID& userID, wstring& userName);
	bool GetAtIndex(unsigned short index, WONChat::UserID& userID, wstring& userName);
	unsigned short count(void) const				{ return usersHere.size(); };			
	bool empty(void) const							{ return usersHere.empty(); };
};


class TMsgChat_UserPresent : public TMessage
{
private:
	WONChat::UserID	mUserID;
public:
	TMsgChat_UserPresent();
	TMsgChat_UserPresent(const TMessage& Tmsg) : TMessage(Tmsg) { Unpack(); }
	TMsgChat_UserPresent(const TMsgChat_UserPresent& toCopy);
	TMsgChat_UserPresent& operator=(const TMessage& Tmsg)		{ TMessage::operator=(Tmsg); Unpack(); return *this; }
	TMsgChat_UserPresent& operator=(const TMsgChat_UserPresent& toCopy);
	virtual TRawMsg* Duplicate() const;

	void Unpack();
	void* Pack();

	WONChat::UserID	GetUserID() const		{ return mUserID; }
	void SetUserID(WONChat::UserID userID)	{ mUserID = userID; }
};


class TMsgChat_UserPresentReply : public TMessage
{
private:
	ChatStatus	mStatus;

public:
	TMsgChat_UserPresentReply();
	TMsgChat_UserPresentReply(const TMessage& Tmsg) : TMessage(Tmsg)	{ Unpack(); }
	TMsgChat_UserPresentReply(const TMsgChat_UserPresentReply& toCopy);
	TMsgChat_UserPresentReply& operator=(const TMessage& Tmsg)			{ TMessage::operator=(Tmsg); Unpack(); return *this; }
	TMsgChat_UserPresentReply& operator=(const TMsgChat_UserPresentReply& toCopy);
	virtual TRawMsg* Duplicate() const;

	void Unpack();
	void* Pack();

	ChatStatus	GetStatus() const		{ return mStatus; }
	void SetStatus(ChatStatus status)	{ mStatus = status; }
};


class TMsgChat_UserLeft : public TMessage
{
private:
	WONChat::UserIDList usersThatLeft;

public:
	TMsgChat_UserLeft();
	TMsgChat_UserLeft(const TMessage& Tmsg) : TMessage(Tmsg)	{ Unpack(); }
	TMsgChat_UserLeft(const TMsgChat_UserLeft& toCopy);
	TMsgChat_UserLeft& operator=(const TMessage& Tmsg)			{ TMessage::operator=(Tmsg); Unpack(); return *this; }
	TMsgChat_UserLeft& operator=(const TMsgChat_UserLeft& toCopy);
	virtual TRawMsg* Duplicate() const;

	void Unpack();
	void* Pack();

	void Drain(void)						{ usersThatLeft.clear(); };
	void Get(WONChat::UserIDList& userList)	{ userList = usersThatLeft; };
	void Set(WONChat::UserIDList userList)	{ usersThatLeft = userList; };

	void Add(WONChat::UserID userID);
	bool Pop(WONChat::UserID& userID);
	unsigned short count(void) const		{ return usersThatLeft.size(); };
	bool empty(void) const					{ return usersThatLeft.empty(); };
};


class TMsgChat_ChatDataMessage : public TMessage
{
public:
	class ChatDataMessageInfo
	{
	public:
		bool							exclusiveUserList;
		WONChat::UserIDList				userList;
		std::deque<WONChat::ChatData*>	messages;
	};
private:
	TMessage							mSaveMsg;
	WONChat::UserID						mOriginUserID;
	std::deque<ChatDataMessageInfo*>	batch;
public:
	TMsgChat_ChatDataMessage();
	TMsgChat_ChatDataMessage(const TMessage& Tmsg) : TMessage(Tmsg)		{ Unpack(); }
	TMsgChat_ChatDataMessage(const TMsgChat_ChatDataMessage& toCopy);
	TMsgChat_ChatDataMessage& operator=(const TMessage& Tmsg)			{ Drain(); TMessage::operator=(Tmsg); Unpack(); return *this; }
	TMsgChat_ChatDataMessage& operator=(const TMsgChat_ChatDataMessage& toCopy);
	virtual TRawMsg* Duplicate() const;
	~TMsgChat_ChatDataMessage()											{ Drain(); }

	void Unpack();
	void* Pack();

	void Drain();

	void SetOriginUserID(WONChat::UserID origID)	{ mOriginUserID = origID; }
	void Set(WONChat::UserID origID, const std::deque<ChatDataMessageInfo*>& theBatch);
	void Set(const std::deque<ChatDataMessageInfo*>& theBatch);
	WONChat::UserID GetOriginUserID() const			{ return mOriginUserID; }
	void Get(WONChat::UserID& origID, std::deque<ChatDataMessageInfo*>& theBatch);
	void Get(std::deque<ChatDataMessageInfo*>& theBatch);

	void Add(bool exclusiveUserList, WONChat::UserIDList userList, std::deque<WONChat::ChatData*> messages);
	bool Pop(bool& exclusiveUserList, WONChat::UserIDList& userList, std::deque<WONChat::ChatData*>& messages);

	bool GetAtIndex(unsigned short index, bool& exclusiveUserList, WONChat::UserIDList& userList, std::deque<WONChat::ChatData*>& messages);
	// !! IMPORTANT !! -	DAMN VC5 STL BUG!!!
	//	TMsgChat_ChatDataMessage will take ownership of ChatData objects, but not
	//		ChatData->chatData.  The later will be copied during Pack().

	unsigned short count(void) const				{ return batch.size(); };
	bool empty(void) const							{ return batch.empty(); };
};


// WARNING: does not copy data until Pack() !
class TMsgChat_SimpleChatDataMessage : public TMessage
{
private:
	TMessage				mSaveMsg;
	WONChat::UserID			mOriginUserID;
	WONChat::ChatDataType	mChatDataType;
	unsigned short			mChatDataSize;
	const void*				mChatData;

public:
	TMsgChat_SimpleChatDataMessage();
	TMsgChat_SimpleChatDataMessage(const TMessage& Tmsg) : TMessage(Tmsg)	{ Unpack(); }
	TMsgChat_SimpleChatDataMessage(const TMsgChat_SimpleChatDataMessage& toCopy);
	TMsgChat_SimpleChatDataMessage& operator=(const TMessage& Tmsg)			{ TMessage::operator=(Tmsg); Unpack(); return *this; }
	TMsgChat_SimpleChatDataMessage& operator=(const TMsgChat_SimpleChatDataMessage& toCopy);
	virtual TRawMsg* Duplicate() const;

	void Unpack();
	void* Pack();

	WONChat::UserID			GetOriginUserID() const				{ return mOriginUserID; }
	WONChat::ChatDataType	GetChatDataType() const				{ return mChatDataType; }
	unsigned short			GetChatDataSize() const				{ return mChatDataSize; };
	const void*				GetChatData() const					{ return mChatData; }

	void SetOriginUserID(WONChat::UserID userID)				{ mOriginUserID = userID; }
	void SetChatDataType(WONChat::ChatDataType chatDataType)	{ mChatDataType = chatDataType; }
	void SetChatData(const void* chatData, unsigned short size)	{ mChatData = chatData; mChatDataSize = size; }
};


class TMsgChat_UserDataMessage : public TMessage
{
public:
	class UserDataInfo
	{
	public:
		WONChat::UserID			userID;
		WONChat::UserDataType	userDataType;
		unsigned short			userDataSize;
		const void*				userData;
	};
private:
	TMessage					mSaveMsg;
	std::deque<UserDataInfo*>	userDataElements;
public:
	TMsgChat_UserDataMessage();
	TMsgChat_UserDataMessage(const TMessage& Tmsg) : TMessage(Tmsg)	{ Unpack(); }
	TMsgChat_UserDataMessage(const TMsgChat_UserDataMessage& toCopy);
	TMsgChat_UserDataMessage& operator=(const TMessage& Tmsg)		{ Drain(); TMessage::operator=(Tmsg); Unpack(); return *this; }
	TMsgChat_UserDataMessage& operator=(const TMsgChat_UserDataMessage& toCopy);
	virtual TRawMsg* Duplicate() const;
	~TMsgChat_UserDataMessage()										{ Drain(); };

	void Unpack();
	void* Pack();

	void Drain(void );
	void Set(const std::deque<UserDataInfo*>& userDataElems);
	void Get(std::deque<UserDataInfo*>&userDataElems);

	void Add(WONChat::UserID userID, WONChat::UserDataType userDataType, unsigned short userDataSize, const void* userData);
	bool Pop(WONChat::UserID& userID, WONChat::UserDataType& userDataType, unsigned short& userDataSize, const void*& userData);
	bool GetAtIndex(unsigned short index, WONChat::UserID& userID, WONChat::UserDataType& userDataType, unsigned short& userDataSize, const void*& userData);
	unsigned short count(void) const					{ return userDataElements.size(); };
	bool empty(void) const								{ return userDataElements.empty(); };
};


class TMsgChat_ReqUserData : public TMessage
{
private:
	WONChat::UserID			mUserID;
	WONChat::UserDataType	mUserDataType;

public:
	TMsgChat_ReqUserData();
	TMsgChat_ReqUserData(const TMessage& Tmsg) : TMessage(Tmsg)	{ Unpack(); }
	TMsgChat_ReqUserData(const TMsgChat_ReqUserData& toCopy);
	TMsgChat_ReqUserData& operator=(const TMessage& Tmsg)		{ TMessage::operator=(Tmsg); Unpack(); return *this; }
	TMsgChat_ReqUserData& operator=(const TMsgChat_ReqUserData& toCopy);
	virtual TRawMsg* Duplicate() const;

	void Unpack();
	void* Pack();

	WONChat::UserID			GetUserID() const					{ return mUserID; }
	WONChat::UserDataType	GetUserDataType() const				{ return mUserDataType; }

	void SetUserID(WONChat::UserID userID)						{ mUserID = userID; }
	void SetUserDataType(WONChat::UserDataType userDataType)	{ mUserDataType = userDataType; }
};


class TMsgChat_RemoveUserData : public TMessage
{
private:
	WONChat::UserID			mUserID;
	WONChat::UserDataType	mUserDataType;

public:
	TMsgChat_RemoveUserData();
	TMsgChat_RemoveUserData(const TMessage& Tmsg) : TMessage(Tmsg)	{ Unpack(); }
	TMsgChat_RemoveUserData(const TMsgChat_RemoveUserData& Tmsg);
	TMsgChat_RemoveUserData& operator=(const TMessage& Tmsg)		{ TMessage::operator=(Tmsg); Unpack(); return *this; }
	TMsgChat_RemoveUserData& operator=(const TMsgChat_RemoveUserData& Tmsg);
	virtual TRawMsg* Duplicate() const;

	void Unpack();
	void* Pack();

	WONChat::UserID			GetUserID() const					{ return mUserID; }
	WONChat::UserDataType	GetUserDataType() const				{ return mUserDataType; }

	void SetUserID(WONChat::UserID userID)						{ mUserID = userID; }
	void SetUserDataType(WONChat::UserDataType userDataType)	{ mUserDataType = userDataType; }
};

class TMsgChat_UserDataMessageReply : public TMessage
{
private:
	std::deque<ChatStatus> batchStatus;	

public:
	TMsgChat_UserDataMessageReply();
	TMsgChat_UserDataMessageReply(const TMessage& Tmsg) : TMessage(Tmsg)	{ Unpack(); }
	TMsgChat_UserDataMessageReply(const TMsgChat_UserDataMessageReply& toCopy);
	TMsgChat_UserDataMessageReply& operator=(const TMessage& Tmsg)			{ TMessage::operator=(Tmsg); Unpack(); return *this; }
	TMsgChat_UserDataMessageReply& operator=(const TMsgChat_UserDataMessageReply& toCopy);
	virtual TRawMsg* Duplicate() const;

	void Unpack();
	void* Pack();

	void Set(std::deque<ChatStatus>		statuses)	{ batchStatus = statuses; };
	void Get(std::deque<ChatStatus>&	statuses)	{ statuses = batchStatus; };

	void Add(ChatStatus status);
	bool Pop(ChatStatus& status);
	bool GetAtIndex(unsigned short index, ChatStatus& status);
	unsigned short count(void) const				{ return batchStatus.size(); };
	bool empty(void) const							{ return batchStatus.empty(); };
};


class TMsgChat_RemoveUserDataReply : public TMessage
{
private:
	ChatStatus mStatus;

public:
	TMsgChat_RemoveUserDataReply();
	TMsgChat_RemoveUserDataReply(const TMessage& Tmsg) : TMessage(Tmsg)	{ Unpack(); }
	TMsgChat_RemoveUserDataReply(const TMsgChat_RemoveUserDataReply& toCopy);
	TMsgChat_RemoveUserDataReply& operator=(const TMessage& Tmsg)		{ TMessage::operator=(Tmsg); Unpack(); return *this; }
	TMsgChat_RemoveUserDataReply& operator=(const TMsgChat_RemoveUserDataReply& toCopy);
	virtual TRawMsg* Duplicate() const;

	void Unpack();
	void* Pack();

	ChatStatus GetStatus() const		{ return mStatus; }
	void SetStatus(ChatStatus status)	{ mStatus = status; }
};


class TMsgChat_UnsubUserData : public TMessage
{
private:
	bool					mAssociated;
	WONChat::UserDataType	mUserDataType;

public:
	TMsgChat_UnsubUserData();
	TMsgChat_UnsubUserData(const TMessage& Tmsg) : TMessage(Tmsg)	{ Unpack(); }
	TMsgChat_UnsubUserData(const TMsgChat_UnsubUserData& toCopy);
	TMsgChat_UnsubUserData& operator=(const TMessage& Tmsg)			{ TMessage::operator=(Tmsg); Unpack(); return *this; }
	TMsgChat_UnsubUserData& operator=(const TMsgChat_UnsubUserData& toCopy);
	virtual TRawMsg* Duplicate() const;

	void Unpack();
	void* Pack();

	bool GetAssociated() const									{ return mAssociated; }
	WONChat::UserDataType GetUserDataType() const				{ return mUserDataType; }

	void SetAssociated(bool assoc)								{ mAssociated = assoc; }
	void SetUserDataType(WONChat::UserDataType userDataType)	{ mUserDataType = userDataType; }
};


class TMsgChat_ChangeName : public TMessage
{
private:
	WONChat::UserID	mUserID;
	wstring			mName;

public:
	TMsgChat_ChangeName();
	TMsgChat_ChangeName(const TMessage& Tmsg) : TMessage(Tmsg)	{ Unpack(); }
	TMsgChat_ChangeName(const TMsgChat_ChangeName& toCopy);
	TMsgChat_ChangeName& operator=(const TMessage& Tmsg)		{ TMessage::operator=(Tmsg); Unpack(); return *this; }
	TMsgChat_ChangeName& operator=(const TMsgChat_ChangeName& toCopy);
	virtual TRawMsg* Duplicate() const;

	void Unpack();
	void* Pack();

	WONChat::UserID GetUserID() const							{ return mUserID; }
	void GetName(wstring& name) const							{ name = mName; }

	void SetUserID(WONChat::UserID userID)						{ mUserID = userID; }
	void SetName(const wstring& name)							{ mName = name; }
};


class TMsgChat_ChangeNameReply : public TMessage
{
private:
	ChatStatus mStatus;

public:
	TMsgChat_ChangeNameReply();
	TMsgChat_ChangeNameReply(const TMessage& Tmsg) : TMessage(Tmsg)	{ Unpack(); }
	TMsgChat_ChangeNameReply(const TMsgChat_ChangeNameReply& toCopy);
	TMsgChat_ChangeNameReply& operator=(const TMessage& Tmsg)		{ TMessage::operator=(Tmsg); Unpack(); return *this; }
	TMsgChat_ChangeNameReply& operator=(const TMsgChat_ChangeNameReply& toCopy);
	virtual TRawMsg* Duplicate() const;

	void Unpack();
	void* Pack();

	ChatStatus GetStatus() const									{ return mStatus; }
	void SetStatus(ChatStatus status)								{ mStatus = status; }
};


class TMsgChat_UserLurk : public TMessage
{
private:
	WONChat::UserID	mUserID;
public:
	TMsgChat_UserLurk();
	TMsgChat_UserLurk(const TMessage& Tmsg) : TMessage(Tmsg)	{ Unpack(); }
	TMsgChat_UserLurk(const TMsgChat_UserLurk& toCopy);
	TMsgChat_UserLurk& operator=(const TMessage& Tmsg)			{ TMessage::operator=(Tmsg); Unpack(); return *this; }
	TMsgChat_UserLurk& operator=(const TMsgChat_UserLurk& toCopy);
	virtual TRawMsg* Duplicate() const;

	void Unpack();
	void* Pack();

	WONChat::UserID	GetUserID() const		{ return mUserID; }
	void SetUserID(WONChat::UserID userID)	{ mUserID = userID; }

};


class TMsgChat_UserLurkReply : public TMessage
{
private:
	ChatStatus	mStatus;

public:
	TMsgChat_UserLurkReply();
	TMsgChat_UserLurkReply(const TMessage& Tmsg) : TMessage(Tmsg)	{ Unpack(); }
	TMsgChat_UserLurkReply(const TMsgChat_UserLurkReply& toCopy);
	TMsgChat_UserLurkReply& operator=(const TMessage& Tmsg)			{ TMessage::operator=(Tmsg); Unpack(); return *this; }
	TMsgChat_UserLurkReply& operator=(const TMsgChat_UserLurkReply& toCopy);
	virtual TRawMsg* Duplicate() const;

	void Unpack();
	void* Pack();

	ChatStatus	GetStatus() const		{ return mStatus; }
	void SetStatus(ChatStatus status)	{ mStatus = status; }
};


class TMsgChat_SubscribeUserData : public TMessage
{
private:
	bool					mAssociated;
	WONChat::UserDataType	mUserDataType;

public:
	TMsgChat_SubscribeUserData();
	TMsgChat_SubscribeUserData(const TMessage& Tmsg) : TMessage(Tmsg)	{ Unpack(); }
	TMsgChat_SubscribeUserData(const TMsgChat_SubscribeUserData& toCopy);
	TMsgChat_SubscribeUserData& operator=(const TMessage& Tmsg)			{ TMessage::operator=(Tmsg); Unpack(); return *this; }
	TMsgChat_SubscribeUserData& operator=(const TMsgChat_SubscribeUserData& toCopy);
	virtual TRawMsg* Duplicate() const;

	void Unpack();
	void* Pack();

	bool GetAssociated() const									{ return mAssociated; }
	WONChat::UserDataType GetUserDataType() const				{ return mUserDataType; }

	void SetAssociated(bool assoc)								{ mAssociated = assoc; }
	void SetUserDataType(WONChat::UserDataType userDataType)	{ mUserDataType = userDataType; }
};


class TMsgChat_LurkerCount : public TMessage
{
private:
	long mLurkerCount;
public:
	TMsgChat_LurkerCount();
	TMsgChat_LurkerCount(const TMessage& Tmsg) : TMessage(Tmsg) { Unpack(); }
	TMsgChat_LurkerCount(const TMsgChat_LurkerCount& toCopy);
	TMsgChat_LurkerCount& operator=(const TMessage& Tmsg)		{ TMessage::operator=(Tmsg); Unpack(); return *this; }
	TMsgChat_LurkerCount& operator=(const TMsgChat_LurkerCount& toCopy);
	virtual TRawMsg* Duplicate() const;

	void Unpack();
	void* Pack();

	long GetLurkerCount() const				{ return mLurkerCount; }
	void SetLurkerCount(long lurkerCount)	{ mLurkerCount = lurkerCount; }

};


};

#endif
