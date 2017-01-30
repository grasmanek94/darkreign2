// TMsgCommStatusReply.h

// Common Generic Reply class.  Returns a status (short) value.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "msg/ServerStatus.h"
#include "TMsgTypesComm.h"
#include "TMsgCommStatusReply.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgCommStatusReply;
};


// ** Constructors / Destructor

// Default ctor
TMsgCommStatusReply::TMsgCommStatusReply(void) :
	TMessage(),
	mStatus(WONMsg::StatusCommon_Success),
	mData()
{
	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommStatusReply);
}


// TMessage ctor
TMsgCommStatusReply::TMsgCommStatusReply(const TMessage& theMsgR) :
	TMessage(theMsgR),
	mStatus(WONMsg::StatusCommon_Success),
	mData()
{
	Unpack();
}


// Copy ctor
TMsgCommStatusReply::TMsgCommStatusReply(const TMsgCommStatusReply& theMsgR) :
	TMessage(theMsgR),
	mStatus(theMsgR.mStatus),
	mData(theMsgR.mData)
{}


// Destructor
TMsgCommStatusReply::~TMsgCommStatusReply(void)
{}


// ** Public Methods

// Assignment operator
TMsgCommStatusReply&
TMsgCommStatusReply::operator=(const TMsgCommStatusReply& theMsgR)
{
	TMessage::operator=(theMsgR);
	mStatus = theMsgR.mStatus;
	mData   = theMsgR.mData;
	return *this;
}


// TMsgCommStatusReply::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgCommStatusReply::Pack(void)
{
	WTRACE("TMsgCommStatusReply::Pack");
	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommStatusReply);
	TMessage::Pack();

	WDBG_LL("TMsgCommStatusReply::Pack Appending message data");
	AppendShort(static_cast<short>(mStatus));

	// Add data only if not empty
	if (! mData.empty())
	{
		AppendShort(mData.size());
		AppendBytes(mData.size(), mData.data());
	}

	return GetDataPtr();
}


// TMsgCommStatusReply::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgCommStatusReply::Unpack(void)
{
	WTRACE("TMsgCommStatusReply::Unpack");
	TMessage::Unpack();

	if ((GetServiceType() != WONMsg::CommonService) ||
	    (GetMessageType() != WONMsg::CommStatusReply))
	{
		WDBG_AH("TMsgCommStatusReply::Unpack Not a CommStatusReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a CommStatusReply message.");
	}

	WDBG_LL("TMsgCommStatusReply::Unpack Reading message data");
	mStatus = static_cast<WONMsg::ServerStatus>(static_cast<short>(ReadShort()));

	// Read data only if there is more to read
	if (BytesLeftToRead() > 0)
	{
		unsigned short aLen = ReadShort();
		mData.assign(reinterpret_cast<const unsigned char*>(ReadBytes(aLen)), aLen);
	}
	else
		mData.erase();
}
