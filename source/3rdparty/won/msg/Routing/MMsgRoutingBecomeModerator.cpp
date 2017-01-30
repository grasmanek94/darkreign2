#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingBecomeModerator.h"

namespace {
	using WONMsg::RoutingServerClientIdFlagMessage;
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingBecomeModerator;
};

MMsgRoutingBecomeModerator::MMsgRoutingBecomeModerator(bool flagOnOrOff) :
	RoutingServerClientIdFlagMessage(flagOnOrOff)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingBecomeModerator);
}

MMsgRoutingBecomeModerator::~MMsgRoutingBecomeModerator(void)
{}

MMsgRoutingBecomeModerator::MMsgRoutingBecomeModerator(const MMsgRoutingBecomeModerator& theMsgR) :
    RoutingServerClientIdFlagMessage(theMsgR)
{}

MMsgRoutingBecomeModerator::MMsgRoutingBecomeModerator(const RoutingServerMessage& theMsgR) : 
    RoutingServerClientIdFlagMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingBecomeModerator& MMsgRoutingBecomeModerator::operator =(const MMsgRoutingBecomeModerator& theMsgR)
{
	if (this != &theMsgR)
		RoutingServerClientIdFlagMessage::operator=(theMsgR);

	return *this;
}

void* MMsgRoutingBecomeModerator::Pack(void)
{
	WTRACE("MMsgRoutingBecomeModerator::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingBecomeModerator);
	RoutingServerClientIdFlagMessage::Pack();

	return GetDataPtr();
}

void MMsgRoutingBecomeModerator::Unpack(void)
{
	WTRACE("MMsgRoutingBecomeModerator::Unpack");
	RoutingServerClientIdFlagMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingBecomeModerator)
	{
		WDBG_AH("MMsgRoutingBecomeModerator::Unpack Not a RoutingBecomeModerator message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingBecomeModerator message.");
	}
}