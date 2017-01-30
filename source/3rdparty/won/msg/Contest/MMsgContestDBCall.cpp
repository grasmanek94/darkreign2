#include "MMsgContestDBCall.h"
#include "msg/MServiceTypes.h"

using namespace WONMsg;


MMsgContestDBCall::MMsgContestDBCall()
	:	mData(0), mDataLen(0)
{
	SetServiceType(MiniContestServer);
	SetMessageType(ContestDBCall);
}


MMsgContestDBCall::MMsgContestDBCall(const MiniMessage& theMsgR, bool doUnpack)
	:	MiniMessage(theMsgR), mData(0), mDataLen(0)
{
	if (doUnpack)
		Unpack();
	else
	{
		SetServiceType(MiniContestServer);
		SetMessageType(ContestDBCall);
	}
}



MMsgContestDBCall::MMsgContestDBCall(const MMsgContestDBCall& toCopy)
	:	MiniMessage(toCopy), mProcNum(toCopy.mProcNum),
		mGameGUID(toCopy.mGameGUID), mDataLen(toCopy.mDataLen),
		mUserId(toCopy.mUserId)
{
	mData = 0;
	if (toCopy.mData)
	{
		mData = new unsigned char[mDataLen];
		if (mData)
			memcpy(mData, toCopy.mData, mDataLen);
		else
			mDataLen = 0;
	}
}


MMsgContestDBCall& MMsgContestDBCall::operator=(const MMsgContestDBCall& toCopy)
{
	MiniMessage::operator=(toCopy);

	if (mData)
		delete[] (unsigned char*)mData;

	mProcNum = toCopy.mProcNum;
	mGameGUID = toCopy.mGameGUID;
	mDataLen = toCopy.mDataLen;
		
	mData = 0;
	if (toCopy.mData)
	{
		mData = new unsigned char[mDataLen];
		if (mData)
			memcpy(mData, toCopy.mData, mDataLen);
		else
			mDataLen = 0;
	}
	
	return *this;
}


MMsgContestDBCall::~MMsgContestDBCall(void)
{
	if (mData)
		delete[] (unsigned char*)mData;
}


TRawMsg* MMsgContestDBCall::Duplicate(void) const
{
	return new MMsgContestDBCall(*this);
}


void MMsgContestDBCall::Unpack()
{
	MiniMessage::Unpack(); //header

	mUserId = ReadLong();
	mProcNum = ReadShort();
	
	const void* guidPtr = ReadBytes(16);
	memcpy(&mGameGUID, guidPtr, 16);
	
	if (mData)
	{
		delete[] (unsigned char*)mData;
		mData = 0;
	}
	mDataLen = BytesLeftToRead();
	if (mDataLen)
	{
		const void* dataPtr = ReadBytes(mDataLen);
		mData = new unsigned char[mDataLen];
		if (mData)
			memcpy(mData, dataPtr, mDataLen);
		else
			mDataLen = 0;
	}
}


void* MMsgContestDBCall::Pack()
{
	SetServiceType(MiniContestServer);
	SetMessageType(ContestDBCall);

	MiniMessage::Pack(); //header

	AppendLong(mUserId);
	AppendShort(mProcNum);
	AppendBytes(16, &mGameGUID);
	AppendBytes(mDataLen, mData);

	return GetDataPtr();
}


void MMsgContestDBCall::SetData(const void* data, unsigned short size)
{
	if (mData)
	{
		delete[] (unsigned char*)mData;
		mData = 0;
		mDataLen = 0;
	}

	if (data && size)
	{
		mData = new unsigned char[size];
		if (mData)
		{
			memcpy(mData, data, size);
			mDataLen = size;
		}
	}
}



MMsgContestDBCallReply::MMsgContestDBCallReply()
	:	mData(0), mDataLen(0)
{
	SetServiceType(MiniContestServer);
	SetMessageType(ContestDBCallReply);
}


MMsgContestDBCallReply::MMsgContestDBCallReply(const MiniMessage& theMsgR, bool doUnpack)
	:	MiniMessage(theMsgR), mData(0), mDataLen(0)
{
	if (doUnpack)
		Unpack();
	else
	{
		SetServiceType(MiniContestServer);
		SetMessageType(ContestDBCallReply);
	}
}


MMsgContestDBCallReply::MMsgContestDBCallReply(const MMsgContestDBCallReply& toCopy)
	:	MiniMessage(toCopy), mStatus(toCopy.mStatus),
		mDataLen(toCopy.mDataLen)
{
	mData = 0;
	if (toCopy.mData)
	{
		mData = new unsigned char[mDataLen];
		if (mData)
			memcpy(mData, toCopy.mData, mDataLen);
		else
			mDataLen = 0;
	}
}


MMsgContestDBCallReply& MMsgContestDBCallReply::operator=(const MMsgContestDBCallReply& toCopy)
{
	MiniMessage::operator=(toCopy);

	if (mData)
		delete[] (unsigned char*)mData;

	mStatus = toCopy.mStatus;
	mDataLen = toCopy.mDataLen;
		
	mData = 0;
	if (toCopy.mData)
	{
		mData = new unsigned char[mDataLen];
		if (mData)
			memcpy(mData, toCopy.mData, mDataLen);
		else
			mDataLen = 0;
	}
	
	return *this;
}


MMsgContestDBCallReply::~MMsgContestDBCallReply(void)
{
	if (mData)
		delete[] (unsigned char*)mData;
}


TRawMsg* MMsgContestDBCallReply::Duplicate(void) const
{
	return new MMsgContestDBCallReply(*this);
}


void MMsgContestDBCallReply::Unpack()
{
	MiniMessage::Unpack(); //header

	mStatus = (ServerStatus)ReadShort();
		
	if (mData)
	{
		delete[] (unsigned char*)mData;
		mData = 0;
	}
	mDataLen = BytesLeftToRead();
	if (mDataLen)
	{
		const void* dataPtr = ReadBytes(mDataLen);
		mData = new unsigned char[mDataLen];
		if (mData)
			memcpy(mData, dataPtr, mDataLen);
		else
			mDataLen = 0;
	}
}


void* MMsgContestDBCallReply::Pack()
{
	SetServiceType(MiniContestServer);
	SetMessageType(ContestDBCallReply);

	MiniMessage::Pack(); //header

	AppendShort(mStatus);
	AppendBytes(mDataLen, mData);

	return GetDataPtr();
}


void MMsgContestDBCallReply::SetData(const void* data, unsigned short size)
{
	if (mData)
	{
		delete[] (unsigned char*)mData;
		mData = 0;
		mDataLen = 0;
	}

	if (data && size)
	{
		mData = new unsigned char[size];
		if (mData)
		{
			memcpy(mData, data, size);
			mDataLen = size;
		}
	}
}


