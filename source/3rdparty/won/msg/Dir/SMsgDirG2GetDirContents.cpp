// SMsgDirG2GetDirectoryContents.cpp

// DirectoryServer get directory contents message.  Requests fixed set of fields
// for services and subdirs of a specified path.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "msg/ServerStatus.h"
#include "SMsgTypesDir.h"
#include "DirEntity.h"
#include "SMsgDirG2GetDirContents.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2QueryBase;
	using WONMsg::SMsgDirG2EntityListBase;
	using WONMsg::SMsgDirG2GetDirectoryContents;
	using WONMsg::SMsgDirG2GetDirectoryContentsReply;
};

// Class constants
const unsigned long SMsgDirG2GetDirectoryContentsReply::gGetFlags =
	(WONMsg::GF_DECOMPSERVICES | WONMsg::GF_DECOMPSUBDIRS | WONMsg::GF_ADDTYPE       |
	 WONMsg::GF_ADDDISPLAYNAME | WONMsg::GF_DIRADDNAME    | WONMsg::GF_SERVADDNAME   |
	 WONMsg::GF_SERVADDNETADDR);


// ** SMsgDirG2GetDirectoryContents **

// ** Constructors / Destructor **

// Default ctor
SMsgDirG2GetDirectoryContents::SMsgDirG2GetDirectoryContents(void) :
	SMsgDirG2QueryBase(KT_DIRECTORY)
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2GetDirectoryContents);
}


// SmallMessage ctor
SMsgDirG2GetDirectoryContents::SMsgDirG2GetDirectoryContents(const SmallMessage& theMsgR) :
	SMsgDirG2QueryBase(theMsgR)
{
	Unpack();
}


// Copy ctor
SMsgDirG2GetDirectoryContents::SMsgDirG2GetDirectoryContents(const SMsgDirG2GetDirectoryContents& theMsgR) :
	SMsgDirG2QueryBase(theMsgR)
{}


// Destructor
SMsgDirG2GetDirectoryContents::~SMsgDirG2GetDirectoryContents(void)
{}


// ** Public Methods **

// Assignment operator
SMsgDirG2GetDirectoryContents&
SMsgDirG2GetDirectoryContents::operator=(const SMsgDirG2GetDirectoryContents& theMsgR)
{
	SMsgDirG2QueryBase::operator=(theMsgR);
	mPath = theMsgR.mPath;
	return *this;
}


// SMsgDirG2GetDirectoryContents::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2GetDirectoryContents::Pack(void)
{
	WTRACE("SMsgDirG2GetDirectoryContents::Pack");
	SetKeyType(KT_DIRECTORY);
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2GetDirectoryContents);
	SMsgDirG2QueryBase::Pack();

	PackKey(*this);
	return GetDataPtr();
}


// SMsgDirG2GetDirectoryContents::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2GetDirectoryContents::Unpack(void)
{
	WTRACE("SMsgDirG2GetDirectoryContents::Unpack");
	SetKeyType(KT_DIRECTORY);
	SMsgDirG2QueryBase::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    (GetMessageType() != WONMsg::DirG2GetDirectoryContents))
	{
		WDBG_AH("SMsgDirG2GetDirectoryContents::Unpack Not a DirG2GetDirectoryContents message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2GetDirectoryContents message.");
	}

	UnpackKey(*this);
}


// ** SMsgDirG2GetDirectoryContentsReply **

// ** Constructors / Destructor

// Default ctor
SMsgDirG2GetDirectoryContentsReply::SMsgDirG2GetDirectoryContentsReply(void) :
	SMsgDirG2EntityListBase(),
	mStatus(WONMsg::StatusCommon_Success)
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2GetDirectoryContentsReply);
}


// SmallMessage ctor
SMsgDirG2GetDirectoryContentsReply::SMsgDirG2GetDirectoryContentsReply(const SmallMessage& theMsgR) :
	SMsgDirG2EntityListBase(theMsgR),
	mStatus(WONMsg::StatusCommon_Success)
{
	Unpack();
}


// Copy ctor
SMsgDirG2GetDirectoryContentsReply::SMsgDirG2GetDirectoryContentsReply(const SMsgDirG2GetDirectoryContentsReply& theMsgR) :
	SMsgDirG2EntityListBase(theMsgR),
	mStatus(theMsgR.mStatus)
{}


// Destructor
SMsgDirG2GetDirectoryContentsReply::~SMsgDirG2GetDirectoryContentsReply(void)
{}


// ** Public Methods

// Assignment operator
SMsgDirG2GetDirectoryContentsReply&
SMsgDirG2GetDirectoryContentsReply::operator=(const SMsgDirG2GetDirectoryContentsReply& theMsgR)
{
	if (this != &theMsgR)  // protect against a = a
	{
		SMsgDirG2EntityListBase::operator=(theMsgR);
		mStatus = theMsgR.mStatus;
	}
	return *this;
}


// SMsgDirG2GetDirectoryContentsReply::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2GetDirectoryContentsReply::Pack(void)
{
	WTRACE("SMsgDirG2GetDirectoryContentsReply::Pack");
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2GetDirectoryContentsReply);
	SMsgDirG2EntityListBase::Pack();

	WDBG_LL("SMsgDirG2GetDirectoryContentsReply::Pack Appending message data");
	AppendShort(static_cast<short>(mStatus));

	PackSequence();
	PackEntities();

	return GetDataPtr();
}


// SMsgDirG2GetDirectoryContentsReply::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2GetDirectoryContentsReply::Unpack(void)
{
	WTRACE("SMsgDirG2GetDirectoryContentsReply::Unpack");
	SMsgDirG2EntityListBase::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    (GetMessageType() != WONMsg::DirG2GetDirectoryContentsReply))
	{
		WDBG_AH("SMsgDirG2GetDirectoryContentsReply::Unpack Not a DirG2GetDirectoryContentsReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2GetDirectoryContentsReply message.");
	}

	WDBG_LL("SMsgDirG2GetDirectoryContentsReply::Unpack Reading message data");
	mStatus = static_cast<WONMsg::ServerStatus>(static_cast<short>(ReadShort()));

	UnpackSequence();
	UnpackEntities();
}


unsigned long
SMsgDirG2GetDirectoryContentsReply::ComputePackSize() const
{
	return (SMsgDirG2EntityListBase::ComputePackSize() + sizeof(mStatus));
}
