#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "RoutingServerClientIdFlgMsg.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::RoutingServerFlagMessage;
	using WONMsg::RoutingServerClientIdFlagMessage;
};

RoutingServerClientIdFlagMessage::RoutingServerClientIdFlagMessage(bool flagOnOrOff) :
	RoutingServerFlagMessage(flagOnOrOff),
	mClientId(0)
{}

RoutingServerClientIdFlagMessage::~RoutingServerClientIdFlagMessage(void)
{}

RoutingServerClientIdFlagMessage::RoutingServerClientIdFlagMessage(const RoutingServerClientIdFlagMessage& theMsgR) :
    RoutingServerFlagMessage(theMsgR),
	mClientId(theMsgR.mClientId)
{}

RoutingServerClientIdFlagMessage::RoutingServerClientIdFlagMessage(const RoutingServerMessage& theMsgR) : 
    RoutingServerFlagMessage(theMsgR)
{
	Unpack();
}

RoutingServerClientIdFlagMessage& RoutingServerClientIdFlagMessage::operator =(const RoutingServerClientIdFlagMessage& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerFlagMessage::operator=(theMsgR);
	    mClientId  = theMsgR.mClientId;
	}

	return *this;
}

void RoutingServerClientIdFlagMessage::Dump(std::ostream& os) const
{
	RoutingServerFlagMessage::Dump(os);	
	
	os << "  ClientId: " << mClientId << endl;
}

void* RoutingServerClientIdFlagMessage::Pack(void)
{
	WTRACE("RoutingServerClientIdFlagMessage::Pack");
	RoutingServerFlagMessage::Pack();

	WDBG_LL("RoutingServerClientIdFlagMessage::Pack Appending message data");

	AppendClientId(mClientId);

	return GetDataPtr();
}

void RoutingServerClientIdFlagMessage::Unpack(void)
{
	WTRACE("RoutingServerClientIdFlagMessage::Unpack");
	RoutingServerFlagMessage::Unpack();

	WDBG_LL("RoutingServerClientIdFlagMessage::Unpack Reading message data");

	mClientId    = ReadClientId();
}