#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "RoutingSrvClientNameFlagMsg.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::RoutingServerFlagMessage;
	using WONMsg::RoutingServerClientNameFlagMessage;
};

RoutingServerClientNameFlagMessage::RoutingServerClientNameFlagMessage(bool flagOnOrOff) :
	RoutingServerFlagMessage(flagOnOrOff)
{}

RoutingServerClientNameFlagMessage::~RoutingServerClientNameFlagMessage(void)
{}

RoutingServerClientNameFlagMessage::RoutingServerClientNameFlagMessage(const RoutingServerClientNameFlagMessage& theMsgR) :
    RoutingServerFlagMessage(theMsgR),
	mClientName(theMsgR.mClientName)
{}

RoutingServerClientNameFlagMessage::RoutingServerClientNameFlagMessage(const RoutingServerMessage& theMsgR) : 
    RoutingServerFlagMessage(theMsgR)
{
	Unpack();
}

RoutingServerClientNameFlagMessage& RoutingServerClientNameFlagMessage::operator =(const RoutingServerClientNameFlagMessage& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerFlagMessage::operator=(theMsgR);
	    mClientName  = theMsgR.mClientName;
	}

	return *this;
}

void RoutingServerClientNameFlagMessage::Dump(std::ostream& os) const
{
	RoutingServerFlagMessage::Dump(os);	
	
	os << "  ClientName: " << mClientName << endl;
}

void* RoutingServerClientNameFlagMessage::Pack(void)
{
	WTRACE("RoutingServerClientNameFlagMessage::Pack");
	RoutingServerFlagMessage::Pack();

	WDBG_LL("RoutingServerClientNameFlagMessage::Pack Appending message data");

	AppendClientName(mClientName);

	return GetDataPtr();
}

void RoutingServerClientNameFlagMessage::Unpack(void)
{
	WTRACE("RoutingServerClientNameFlagMessage::Unpack");
	RoutingServerFlagMessage::Unpack();

	WDBG_LL("RoutingServerClientNameFlagMessage::Unpack Reading message data");

	ReadClientName(mClientName);
}