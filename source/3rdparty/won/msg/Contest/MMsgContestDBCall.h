#ifndef __MMSGCONTESTDBCALL_H__
#define __MMSGCONTESTDBCALL_H__


#include "msg/TMessage.h"
#include "msg/ServerStatus.h"
#include "MMsgTypesContest.h"


namespace WONMsg {


class MMsgContestDBCall : public MiniMessage
{
public:
	MMsgContestDBCall(void);
	explicit MMsgContestDBCall(const MiniMessage& theMsgR, bool doUnpack = true);
	MMsgContestDBCall(const MMsgContestDBCall& theMsgR);

	virtual ~MMsgContestDBCall(void);

	MMsgContestDBCall& operator=(const MMsgContestDBCall& theMsgR);

	// Virtual Duplicate from BaseMessage
	virtual TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);

	// Member access
	unsigned long GetUserId(void) const					{ return mUserId; }
	void SetUserId(unsigned long userId)				{ mUserId = userId; }

	short GetProcedureNum(void) const					{ return mProcNum; }
	void SetProcedureNum(short theProcNum)				{ mProcNum = theProcNum; }

	GUID GetGameGUID(void) const						{ return mGameGUID; }
	void SetGameGUID(const GUID& theGUID)				{ mGameGUID = theGUID; }

	const void* GetData() const							{ return mData; }
	unsigned short GetDataLen() const					{ return mDataLen; }
	void SetData(const void* data, unsigned short size);

protected:
	unsigned long mUserId;
	short mProcNum;
	GUID mGameGUID;
	void* mData;
	unsigned short mDataLen;
};



class MMsgContestDBCallReply : public MiniMessage
{
public:
	MMsgContestDBCallReply(void);
	explicit MMsgContestDBCallReply(const MiniMessage& theMsgR, bool doUnpack = true);
	MMsgContestDBCallReply(const MMsgContestDBCallReply& theMsgR);

	virtual ~MMsgContestDBCallReply(void);

	MMsgContestDBCallReply& operator=(const MMsgContestDBCallReply& theMsgR);

	// Virtual Duplicate from BaseMessage
	virtual TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);

	// Member access
	ServerStatus GetStatus(void) const					{ return mStatus; }
	void SetStatus(ServerStatus theStatus)				{ mStatus = theStatus; }

	const void* GetData() const							{ return mData; }
	unsigned short GetDataLen() const					{ return mDataLen; }
	void SetData(const void* data, unsigned short size);

protected:
	ServerStatus mStatus;
	void* mData;
	unsigned short mDataLen;
};




};  // Namespace WONMsg

#endif

