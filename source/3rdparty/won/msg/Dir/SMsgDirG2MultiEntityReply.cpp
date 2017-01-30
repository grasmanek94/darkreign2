// SMsgDirG2MultiEntityReply.cpp

// Directory multi entity reply.  Reply for query request that can return one or
// more entities.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "msg/ServerStatus.h"
#include "SMsgTypesDir.h"
#include "DirEntity.h"
#include "SMsgDirG2MultiEntityReply.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2EntityListBase;
	using WONMsg::SMsgDirG2MultiEntityReply;
};


// ** Constructors / Destructor

// Default ctor
SMsgDirG2MultiEntityReply::SMsgDirG2MultiEntityReply(void) :
	SMsgDirG2EntityListBase(),
	mStatus(WONMsg::StatusCommon_Success),
	mFlags(WONMsg::GF_NOFLAGS)
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2MultiEntityReply);
}


// SmallMessage ctor
SMsgDirG2MultiEntityReply::SMsgDirG2MultiEntityReply(const SmallMessage& theMsgR) :
	SMsgDirG2EntityListBase(theMsgR),
	mStatus(WONMsg::StatusCommon_Success),
	mFlags(WONMsg::GF_NOFLAGS)
{
	Unpack();
}


// Copy ctor
SMsgDirG2MultiEntityReply::SMsgDirG2MultiEntityReply(const SMsgDirG2MultiEntityReply& theMsgR) :
	SMsgDirG2EntityListBase(theMsgR),
	mStatus(theMsgR.mStatus),
	mFlags(theMsgR.mFlags)
{}


// Destructor
SMsgDirG2MultiEntityReply::~SMsgDirG2MultiEntityReply(void)
{}


// ** Public Methods

// Assignment operator
SMsgDirG2MultiEntityReply&
SMsgDirG2MultiEntityReply::operator=(const SMsgDirG2MultiEntityReply& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
		SMsgDirG2EntityListBase::operator=(theMsgR);
		mStatus = theMsgR.mStatus;
		mFlags  = theMsgR.mFlags;
	}
	return *this;
}


// SMsgDirG2MultiEntityReply::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2MultiEntityReply::Pack(void)
{
	WTRACE("SMsgDirG2MultiEntityReply::Pack");
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2MultiEntityReply);
	SMsgDirG2EntityListBase::Pack();

	WDBG_LL("SMsgDirG2MultiEntityReply::Pack Appending message data");
	AppendShort(static_cast<short>(mStatus));
	PackSequence();

	// Append flags, force AddDataObject flag if there are data types
	if (mDataTypes.empty())
		AppendLong(mFlags);
	else
		AppendLong(mFlags | GF_ADDDATAOBJECTS);

	PackEntities();
	return GetDataPtr();
}


// SMsgDirG2MultiEntityReply::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2MultiEntityReply::Unpack(void)
{
	WTRACE("SMsgDirG2MultiEntityReply::Unpack");
	SMsgDirG2EntityListBase::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    (GetMessageType() != WONMsg::DirG2MultiEntityReply))
	{
		WDBG_AH("SMsgDirG2MultiEntityReply::Unpack Not a DirG2MultiEntityReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2MultiEntityReply message.");
	}

	WDBG_LL("SMsgDirG2MultiEntityReply::Unpack Reading message data");
	mStatus = static_cast<WONMsg::ServerStatus>(static_cast<short>(ReadShort()));
	UnpackSequence();
	mFlags  = ReadLong();

	UnpackEntities();
}


unsigned long
SMsgDirG2MultiEntityReply::ComputePackSize() const
{
	return (SMsgDirG2EntityListBase::ComputePackSize() + sizeof(mStatus) + sizeof(mFlags));
}
