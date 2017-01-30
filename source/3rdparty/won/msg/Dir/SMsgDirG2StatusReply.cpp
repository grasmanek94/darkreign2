// SMsgDirG2StatusReply.h

// Directory generation 2 Generic Reply class.  Returns the status of a
// request made to the Directory Server.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "msg/ServerStatus.h"
#include "SMsgTypesDir.h"
#include "SMsgDirG2StatusReply.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2StatusReply;
};


// ** Constructors / Destructor

// Default ctor
SMsgDirG2StatusReply::SMsgDirG2StatusReply(WONMsg::ServerStatus theStatus) :
	SmallMessage(),
	mStatus(theStatus),
	mNetAddr()
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2StatusReply);
}


// SmallMessage ctor
SMsgDirG2StatusReply::SMsgDirG2StatusReply(const SmallMessage& theMsgR) :
	SmallMessage(theMsgR),
	mStatus(WONMsg::StatusCommon_Success),
	mNetAddr()
{
	Unpack();
}


// Copy ctor
SMsgDirG2StatusReply::SMsgDirG2StatusReply(const SMsgDirG2StatusReply& theMsgR) :
	SmallMessage(theMsgR),
	mStatus(theMsgR.mStatus),
	mNetAddr(theMsgR.mNetAddr)
{}


// Destructor
SMsgDirG2StatusReply::~SMsgDirG2StatusReply(void)
{}


// ** Public Methods

// Assignment operator
SMsgDirG2StatusReply&
SMsgDirG2StatusReply::operator=(const SMsgDirG2StatusReply& theMsgR)
{
	SmallMessage::operator=(theMsgR);
	mStatus  = theMsgR.mStatus;
	mNetAddr = theMsgR.mNetAddr;
	return *this;
}


// SMsgDirG2StatusReply::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2StatusReply::Pack(void)
{
	WTRACE("SMsgDirG2StatusReply::Pack");
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2StatusReply);
	SmallMessage::Pack();

	WDBG_LL("SMsgDirG2StatusReply::Pack Appending message data");
	AppendShort(static_cast<short>(mStatus));

	// Add net address if set
	if (! mNetAddr.empty())
	{
		AppendByte(mNetAddr.size());
		AppendBytes(mNetAddr.size(), mNetAddr.data());
	}

	return GetDataPtr();
}


// SMsgDirG2StatusReply::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2StatusReply::Unpack(void)
{
	WTRACE("SMsgDirG2StatusReply::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    (GetMessageType() != WONMsg::DirG2StatusReply))
	{
		WDBG_AH("SMsgDirG2StatusReply::Unpack Not a DirG2StatusReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2StatusReply message.");
	}

	WDBG_LL("SMsgDirG2StatusReply::Unpack Reading message data");
	mStatus = static_cast<WONMsg::ServerStatus>(static_cast<short>(ReadShort()));

	// Read Netaddress if needed
	mNetAddr.erase();
	if (BytesLeftToRead() > 0)
	{
		WDBG_LL("SMsgDirG2StatusReply::Unpack Reading address");
		unsigned int aLen = ReadByte();
		if (aLen > 0)
			mNetAddr.assign(reinterpret_cast<const unsigned char*>(ReadBytes(aLen)), aLen);
	}
}
