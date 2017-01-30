// SMsgFirewallStatusReply.h

// Common Generic Reply class.  Returns a status (short) value.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "msg/ServerStatus.h"
#include "SMsgFirewallStatusReply.h"
#include "SMsgTypesFirewall.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgFirewallStatusReply;
};


// ** Constructors / Destructor

// Default ctor
SMsgFirewallStatusReply::SMsgFirewallStatusReply(void) :
	SmallMessage(),
	mStatus(WONMsg::StatusCommon_Success)
{
	SetServiceType(WONMsg::SmallFirewallDetector);
	SetMessageType(WONMsg::FirewallStatusReply);
}


// SmallMessage ctor
SMsgFirewallStatusReply::SMsgFirewallStatusReply(const SmallMessage& theMsgR) :
	SmallMessage(theMsgR),
	mStatus(WONMsg::StatusCommon_Success)
{
	Unpack();
}


// Copy ctor
SMsgFirewallStatusReply::SMsgFirewallStatusReply(const SMsgFirewallStatusReply& theMsgR) :
	SmallMessage(theMsgR),
	mStatus(theMsgR.mStatus)
{}


// Destructor
SMsgFirewallStatusReply::~SMsgFirewallStatusReply(void)
{}


// ** Public Methods

// Assignment operator
SMsgFirewallStatusReply&
SMsgFirewallStatusReply::operator=(const SMsgFirewallStatusReply& theMsgR)
{
	SmallMessage::operator=(theMsgR);
	mStatus = theMsgR.mStatus;
	return *this;
}


// SMsgFirewallStatusReply::Pack
// Virtual method from SmallMessage.  Packs data into message buffer and
// sets the new message length.
void*
SMsgFirewallStatusReply::Pack(void)
{
	WTRACE("SMsgFirewallStatusReply::Pack");
	SetServiceType(WONMsg::SmallFirewallDetector);
	SetMessageType(WONMsg::FirewallStatusReply);
	SmallMessage::Pack();

	WDBG_LL("SMsgFirewallStatusReply::Pack Appending message data");
	AppendShort(static_cast<short>(mStatus));

	return GetDataPtr();
}


// SMsgFirewallStatusReply::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgFirewallStatusReply::Unpack(void)
{
	WTRACE("SMsgFirewallStatusReply::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallFirewallDetector) ||
	    (GetMessageType() != WONMsg::FirewallStatusReply))
	{
		WDBG_AH("SMsgFirewallStatusReply::Unpack Not a FirewallStatusReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a FirewallStatusReply message.");
	}

	WDBG_LL("SMsgFirewallStatusReply::Unpack Reading message data");
	mStatus = static_cast<WONMsg::ServerStatus>(static_cast<short>(ReadShort()));
}
