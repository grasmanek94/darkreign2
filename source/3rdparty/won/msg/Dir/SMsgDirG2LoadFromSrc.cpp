// SMsgDirG2LoadFromSrc

// DirectoryServer Load From Source message.  Requests directory tree of a source
// Directory Server.


#include "common/won.h"
#include "common/DataObject.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesDir.h"
#include "DirG2Flags.h"
#include "SMsgDirG2LoadFromSrc.h"

// Private namespace for using, types, and constants
namespace {
	using WONCommon::DataObject;
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2EntityListBase;
	using WONMsg::SMsgDirG2LoadFromSrc;
	using WONMsg::SMsgDirG2LoadFromSrcReply;

	// Current getflags
	const unsigned long gGetFlags = (
		// Decomposition
		WONMsg::GF_DECOMPROOT      | WONMsg::GF_DECOMPSERVICES | WONMsg::GF_DECOMPSUBDIRS |
		WONMsg::GF_DECOMPRECURSIVE |

		// Common
		WONMsg::GF_ADDTYPE    | WONMsg::GF_ADDDISPLAYNAME | WONMsg::GF_ADDCREATED     |
		WONMsg::GF_ADDTOUCHED | WONMsg::GF_ADDLIFESPAN    | WONMsg::GF_ADDUIDS        |
		WONMsg::GF_ADDDOTYPE  | WONMsg::GF_ADDDODATA      | WONMsg::GF_ADDDATAOBJECTS |
		WONMsg::GF_ADDACLS    |

		// Directory
		WONMsg::GF_DIRADDPATH | WONMsg::GF_DIRADDNAME | WONMsg::GF_DIRADDVISIBLE |

		// Service
		WONMsg::GF_SERVADDPATH | WONMsg::GF_SERVADDNAME | WONMsg::GF_SERVADDNETADDR
	);

	// Old get flags used before mGetFlags member added
	const unsigned long gOldFlags = (
		// Decomposition
		WONMsg::GF_DECOMPROOT      | WONMsg::GF_DECOMPSERVICES | WONMsg::GF_DECOMPSUBDIRS |
		WONMsg::GF_DECOMPRECURSIVE |

		// Common
		WONMsg::GF_ADDTYPE    | WONMsg::GF_ADDDISPLAYNAME | WONMsg::GF_ADDCREATED |
		WONMsg::GF_ADDTOUCHED | WONMsg::GF_ADDLIFESPAN    | WONMsg::GF_ADDDOTYPE  |
		WONMsg::GF_ADDDODATA  | WONMsg::GF_ADDDATAOBJECTS | WONMsg::GF_ADDACLS    |

		// Directory
		WONMsg::GF_DIRADDPATH | WONMsg::GF_DIRADDNAME | WONMsg::GF_DIRADDVISIBLE |

		// Service
		WONMsg::GF_SERVADDPATH | WONMsg::GF_SERVADDNAME | WONMsg::GF_SERVADDNETADDR
	);
};


// ** SMsgDirG2LoadFromSrc **

// ** Constructors / Destructor **

// Default ctor
SMsgDirG2LoadFromSrc::SMsgDirG2LoadFromSrc(void) :
	SmallMessage(),
	mGetFlags(gGetFlags)
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2LoadFromSrc);
}


// SmallMessage ctor
SMsgDirG2LoadFromSrc::SMsgDirG2LoadFromSrc(const SmallMessage& theMsgR) :
	SmallMessage(theMsgR),
	mGetFlags(gGetFlags)
{
	Unpack();
}


// Copy ctor
SMsgDirG2LoadFromSrc::SMsgDirG2LoadFromSrc(const SMsgDirG2LoadFromSrc& theMsgR) :
	SmallMessage(theMsgR),
	mGetFlags(theMsgR.mGetFlags)
{}


// Destructor
SMsgDirG2LoadFromSrc::~SMsgDirG2LoadFromSrc(void)
{}


// ** Public Methods **

// Assignment operator
SMsgDirG2LoadFromSrc&
SMsgDirG2LoadFromSrc::operator=(const SMsgDirG2LoadFromSrc& theMsgR)
{
	SmallMessage::operator=(theMsgR);
	mGetFlags = theMsgR.mGetFlags;
	return *this;
}


// SMsgDirG2LoadFromSrc::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2LoadFromSrc::Pack(void)
{
	WTRACE("SMsgDirG2LoadFromSrc::Pack");
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2LoadFromSrc);

	// Pack data
	SmallMessage::Pack();
	AppendLong(mGetFlags);

	return GetDataPtr();
}


// SMsgDirG2LoadFromSrc::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2LoadFromSrc::Unpack(void)
{
	WTRACE("SMsgDirG2LoadFromSrc::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    (GetMessageType() != WONMsg::DirG2LoadFromSrc))
	{
		WDBG_AH("SMsgDirG2LoadFromSrc::Unpack Not a DirG2LoadFromSrc message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2LoadFromSrc message.");
	}

	// Unpack data
	mGetFlags = (BytesLeftToRead() > 0 ? ReadLong() : gOldFlags);
}


// ** SMsgDirG2LoadFromSrcReply **

// ** Constructors / Destructor

// Default ctor
SMsgDirG2LoadFromSrcReply::SMsgDirG2LoadFromSrcReply(void) :
	SMsgDirG2EntityListBase(),
	mGetFlags(gGetFlags)
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2LoadFromSrcReply);
}


// SmallMessage ctor
SMsgDirG2LoadFromSrcReply::SMsgDirG2LoadFromSrcReply(const SmallMessage& theMsgR) :
	SMsgDirG2EntityListBase(theMsgR),
	mGetFlags(gGetFlags)
{
	Unpack();
}


// Copy ctor
SMsgDirG2LoadFromSrcReply::SMsgDirG2LoadFromSrcReply(const SMsgDirG2LoadFromSrcReply& theMsgR) :
	SMsgDirG2EntityListBase(theMsgR),
	mGetFlags(theMsgR.mGetFlags)
{}


// Destructor
SMsgDirG2LoadFromSrcReply::~SMsgDirG2LoadFromSrcReply(void)
{}


// ** Public Methods

// Assignment operator
SMsgDirG2LoadFromSrcReply&
SMsgDirG2LoadFromSrcReply::operator=(const SMsgDirG2LoadFromSrcReply& theMsgR)
{
	SMsgDirG2EntityListBase::operator=(theMsgR);
	return *this;
}


// SMsgDirG2LoadFromSrcReply::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2LoadFromSrcReply::Pack(void)
{
	WTRACE("SMsgDirG2LoadFromSrcReply::Pack");
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2LoadFromSrcReply);
	SMsgDirG2EntityListBase::Pack();

	WDBG_LL("SMsgDirG2LoadFromSrcReply::Pack Appending message data");
	AppendLong(mGetFlags);
	PackSequence();
	PackEntities();

	return GetDataPtr();
}


// SMsgDirG2LoadFromSrcReply::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2LoadFromSrcReply::Unpack(void)
{
	WTRACE("SMsgDirG2LoadFromSrcReply::Unpack");
	SMsgDirG2EntityListBase::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    ((GetMessageType() != WONMsg::DirG2LoadFromSrcReply) &&
	     (GetMessageType() != WONMsg::DirG2LoadFromSrcReplyObsolete)))
	{
		WDBG_AH("SMsgDirG2LoadFromSrcReply::Unpack Not a DirG2LoadFromSrcReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2LoadFromSrcReply message.");
	}

	WDBG_LL("SMsgDirG2LoadFromSrcReply::Unpack Reading message data");
	mGetFlags = (GetMessageType() != WONMsg::DirG2LoadFromSrcReplyObsolete ? ReadLong() : gOldFlags);
	UnpackSequence();
	UnpackEntities();
}
