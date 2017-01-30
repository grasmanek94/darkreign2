#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingStatusReply.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingStatusReply;
};

MMsgRoutingStatusReply::MMsgRoutingStatusReply(void) : 
	RoutingServerMessage(),
	mStatus(0)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingStatusReply);
}

MMsgRoutingStatusReply::~MMsgRoutingStatusReply(void)
{}

MMsgRoutingStatusReply::MMsgRoutingStatusReply(const MMsgRoutingStatusReply& theMsgR) :
    RoutingServerMessage(theMsgR),
	mStatus(theMsgR.mStatus)
{}

MMsgRoutingStatusReply::MMsgRoutingStatusReply(const RoutingServerMessage& theMsgR, bool doUnpack) : 
    RoutingServerMessage(theMsgR)
{
	if (doUnpack)
		Unpack();
}

MMsgRoutingStatusReply& MMsgRoutingStatusReply::operator =(const MMsgRoutingStatusReply& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
		mStatus = theMsgR.mStatus;
	}

	return *this;
}

void MMsgRoutingStatusReply::Dump(std::ostream& os) const
{
	RoutingServerMessage::Dump(os);	
	
	os << "  Status: " << mStatus << endl;
}

void* MMsgRoutingStatusReply::Pack(void)
{
	WTRACE("MMsgRoutingStatusReply::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingStatusReply);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingStatusReply::Pack Appending message data");

	// append the status
	AppendShort(mStatus);

	return GetDataPtr();
}

void MMsgRoutingStatusReply::Unpack(void)
{
	WTRACE("MMsgRoutingStatusReply::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingStatusReply)
	{
		WDBG_AH("MMsgRoutingStatusReply::Unpack Not a RoutingStatusReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingStatusReply message.");
	}

	WDBG_LL("MMsgRoutingStatusReply::Unpack Reading message data");

	// read in the status
	mStatus = ReadShort();
}