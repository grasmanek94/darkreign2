// TMsgDirFindService.h

// Directory Find Service Message class.  Locates a service is a specified
// directory.  Service may have a parital key specified and search may be
// recursive or not.

// Directory Find Service Reply Message class.  Contains data from the
// Directory Server in response to a find service request.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "msg/ServerStatus.h"
#include "TMsgTypesDir.h"
#include "DirServerEntry.h"
#include "DirServerSKMasks.h"
#include "TMsgDirFindService.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::ServerStatus;
	using WONMsg::DirServerEntry;
	using WONMsg::DirServerEntryVector;
	using WONMsg::TMsgDirFindService;
	using WONMsg::TMsgDirFindServiceReply;
};


// ** TMsgDirFindService **

// ** Constructors / Destructor

// Default ctor
TMsgDirFindService::TMsgDirFindService(void) :
	TMsgDirServiceBase(),
	mFindMask(SKMASK_ALL),
	mRecursive(false)
{
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirFindService);
}


// TMessage ctor
TMsgDirFindService::TMsgDirFindService(const TMessage& theMsgR) :
	TMsgDirServiceBase(theMsgR),
	mFindMask(SKMASK_ALL),
	mRecursive(false)
{
	Unpack();
}


// Copy ctor
TMsgDirFindService::TMsgDirFindService(const TMsgDirFindService& theMsgR) :
	TMsgDirServiceBase(theMsgR),
	mFindMask(theMsgR.mFindMask),
	mRecursive(theMsgR.mRecursive)
{}


// Destructor
TMsgDirFindService::~TMsgDirFindService(void)
{}


// ** Public Methods

// Assignment operator
TMsgDirFindService&
TMsgDirFindService::operator=(const TMsgDirFindService& theMsgR)
{
	TMessage::operator=(theMsgR);
	mFindMask  = theMsgR.mFindMask;
	mRecursive = theMsgR.mRecursive;
	return *this;
}


// TMsgDirFindService::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgDirFindService::Pack(void)
{
	WTRACE("TMsgDirFindService::Pack");
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirFindService);
	TMsgDirServiceBase::Pack();

	WDBG_LL("TMsgDirFindService::Pack Appending message data");
	AppendByte(mFindMask);
	AppendByte(mRecursive ? 1 : 0);

	return GetDataPtr();
}


// TMsgDirFindService::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgDirFindService::Unpack(void)
{
	WTRACE("TMsgDirFindService::Unpack");
	TMsgDirServiceBase::Unpack();

	if ((GetServiceType() != WONMsg::DirServer) ||
	    (GetMessageType() != WONMsg::DirFindService))
	{
		WDBG_AH("TMsgDirFindService::Unpack Not a DirFindService message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirFindService message.");
	}

	WDBG_LL("TMsgDirFindService::Unpack Reading message data");
	mFindMask  = ReadByte();
	mRecursive = (ReadByte() == 0 ? false : true);
}


// ** TMsgDirFindServiceReply **

// ** Constructors / Destructor

// Default ctor
TMsgDirFindServiceReply::TMsgDirFindServiceReply(void) :
	TMessage(),
	mStatus(WONMsg::StatusCommon_Success),
	mService()
{
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirFindServiceReply);
	mService.mType = DirServerEntry::EntryTypeService;
}


// TMessage ctor
TMsgDirFindServiceReply::TMsgDirFindServiceReply(const TMessage& theMsgR) :
	TMessage(theMsgR),
	mStatus(WONMsg::StatusCommon_Success),
	mService()
{
	Unpack();
}


// Copy ctor
TMsgDirFindServiceReply::TMsgDirFindServiceReply(const TMsgDirFindServiceReply& theMsgR) :
	TMessage(theMsgR),
	mStatus(theMsgR.mStatus),
	mService(theMsgR.mService)
{}


// Destructor
TMsgDirFindServiceReply::~TMsgDirFindServiceReply(void)
{}


// ** Public Methods **

// Assignment operator
TMsgDirFindServiceReply&
TMsgDirFindServiceReply::operator=(const TMsgDirFindServiceReply& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
		TMessage::operator=(theMsgR);
		mStatus  = theMsgR.mStatus;
		mService = theMsgR.mService;
	}
	return *this;
}


// TMsgDirFindServiceReply::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgDirFindServiceReply::Pack(void)
{
	WTRACE("TMsgDirFindServiceReply::Pack");
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirFindServiceReply);
	TMessage::Pack();

	WDBG_LL("TMsgDirFindServiceReply::Pack Appending message data");
	AppendShort(static_cast<short>(mStatus));

	// Service info only sent if status is ok
	if (mStatus == WONMsg::StatusCommon_Success)
	{
		Append_PW_STRING(mService.mPath);
		Append_PW_STRING(mService.mName);
		Append_PW_STRING(mService.mDisplayName);
		Append_PW_STRING(mService.mVersion);
		Append_PW_STRING(mService.mProtoName);
		Append_PW_STRING(mService.mProtoVersion);
		Append_PW_STRING(mService.mNetAddress);
		AppendLong(mService.mLifespan);
		AppendLong(mService.mCreated);
		AppendShort(mService.mBlobLen);
		AppendBytes(mService.mBlobLen, mService.mBlob);
	}

	return GetDataPtr();
}


// TMsgDirFindServiceReply::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgDirFindServiceReply::Unpack(void)
{
	WTRACE("TMsgDirFindServiceReply::Unpack");
	TMessage::Unpack();

	if ((GetServiceType() != WONMsg::DirServer) ||
	    (GetMessageType() != WONMsg::DirFindServiceReply))
	{
		WDBG_AH("TMsgDirFindServiceReply::Unpack Not a DirFindServiceReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirFindServiceReply message.");
	}

	WDBG_LL("TMsgDirFindServiceReply::Unpack Reading message data");
	mStatus = static_cast<ServerStatus>(static_cast<short>(ReadShort()));

	// Service data only present if status is success
	if (mStatus == WONMsg::StatusCommon_Success)
	{
		mService.mType = DirServerEntry::EntryTypeService;
		ReadWString(mService.mPath);
		ReadWString(mService.mName);
		ReadWString(mService.mDisplayName);
		ReadWString(mService.mVersion);
		ReadWString(mService.mProtoName);
		ReadWString(mService.mProtoVersion);
		ReadWString(mService.mNetAddress);
		mService.mLifespan = ReadLong();
		mService.mCreated  = ReadLong();

		unsigned short aLen = ReadShort();
		mService.SetBlob(ReadBytes(aLen), aLen);
	}
}
