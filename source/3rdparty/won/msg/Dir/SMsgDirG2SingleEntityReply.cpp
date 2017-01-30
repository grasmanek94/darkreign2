// SMsgDirG2SingleEntityReply.h

// Directory single entity reply.  Reply for query request that can return only
// one entity.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "msg/ServerStatus.h"
#include "SMsgTypesDir.h"
#include "DirEntity.h"
#include "SMsgDirG2SingleEntityReply.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2EntityBase;
	using WONMsg::SMsgDirG2SingleEntityReply;
};


// ** Constructors / Destructor

// Default ctor
SMsgDirG2SingleEntityReply::SMsgDirG2SingleEntityReply(void) :
	SMsgDirG2EntityBase(),
	mStatus(WONMsg::StatusCommon_Success),
	mFlags(WONMsg::GF_NOFLAGS),
	mEntity()
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2SingleEntityReply);
}


// SmallMessage ctor
SMsgDirG2SingleEntityReply::SMsgDirG2SingleEntityReply(const SmallMessage& theMsgR) :
	SMsgDirG2EntityBase(theMsgR),
	mStatus(WONMsg::StatusCommon_Success),
	mFlags(WONMsg::GF_NOFLAGS),
	mEntity()
{
	Unpack();
}


// Copy ctor
SMsgDirG2SingleEntityReply::SMsgDirG2SingleEntityReply(const SMsgDirG2SingleEntityReply& theMsgR) :
	SMsgDirG2EntityBase(theMsgR),
	mStatus(theMsgR.mStatus),
	mFlags(theMsgR.mFlags),
	mEntity(theMsgR.mEntity)
{}


// Destructor
SMsgDirG2SingleEntityReply::~SMsgDirG2SingleEntityReply(void)
{}


// ** Public Methods **

// Assignment operator
SMsgDirG2SingleEntityReply&
SMsgDirG2SingleEntityReply::operator=(const SMsgDirG2SingleEntityReply& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
		SMsgDirG2EntityBase::operator=(theMsgR);
		mStatus    = theMsgR.mStatus;
		mFlags     = theMsgR.mFlags;
		mEntity    = theMsgR.mEntity;
	}
	return *this;
}


// SMsgDirG2SingleEntityReply::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2SingleEntityReply::Pack(void)
{
	WTRACE("SMsgDirG2SingleEntityReply::Pack");
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2SingleEntityReply);
	SMsgDirG2EntityBase::Pack();

	WDBG_LL("SMsgDirG2SingleEntityReply::Pack Appending message data");
	AppendShort(static_cast<short>(mStatus));

	// Append flags, force AddDataObject flag if there are data types
	if (mStatus >= 0)
	{
		if (mDataTypes.empty())
			AppendLong(mFlags);
		else
			AppendLong(mFlags | GF_ADDDATAOBJECTS);

		PackEntity(mEntity);
	}

	return GetDataPtr();
}


// SMsgDirG2SingleEntityReply::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2SingleEntityReply::Unpack(void)
{
	WTRACE("SMsgDirG2SingleEntityReply::Unpack");
	SMsgDirG2EntityBase::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    (GetMessageType() != WONMsg::DirG2SingleEntityReply))
	{
		WDBG_AH("SMsgDirG2SingleEntityReply::Unpack Not a DirG2SingleEntityReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2SingleEntityReply message.");
	}

	WDBG_LL("SMsgDirG2SingleEntityReply::Unpack Reading message data");
	mStatus = static_cast<WONMsg::ServerStatus>(static_cast<short>(ReadShort()));

	if (mStatus >= 0)
	{
		mFlags = ReadLong();
		UnpackEntity(mEntity);
	}
}
