#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingOpenRegistration.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingOpenRegistration;
};

MMsgRoutingOpenRegistration::MMsgRoutingOpenRegistration(void) : RoutingServerMessage()
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingOpenRegistration);
}

MMsgRoutingOpenRegistration::~MMsgRoutingOpenRegistration(void)
{}

MMsgRoutingOpenRegistration::MMsgRoutingOpenRegistration(const MMsgRoutingOpenRegistration& theMsgR) :
    RoutingServerMessage(theMsgR)
{}

MMsgRoutingOpenRegistration::MMsgRoutingOpenRegistration(const RoutingServerMessage& theMsgR) : 
    RoutingServerMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingOpenRegistration& MMsgRoutingOpenRegistration::operator =(const MMsgRoutingOpenRegistration& theMsgR)
{
	if (this != &theMsgR)
		RoutingServerMessage::operator=(theMsgR);

	return *this;
}

void* MMsgRoutingOpenRegistration::Pack(void)
{
	WTRACE("MMsgRoutingOpenRegistration::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingOpenRegistration);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingOpenRegistration::Pack Appending message data");

	return GetDataPtr();
}

void MMsgRoutingOpenRegistration::Unpack(void)
{
	WTRACE("MMsgRoutingOpenRegistration::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingOpenRegistration)
	{
		WDBG_AH("MMsgRoutingOpenRegistration::Unpack Not a RoutingOpenRegistration message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingOpenRegistration message.");
	}

	WDBG_LL("MMsgRoutingOpenRegistration::Unpack Reading message data");
}